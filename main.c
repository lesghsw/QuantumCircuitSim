#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h> // Per fabs()
#include <string.h>

#define true  1
#define false 0

typedef unsigned char bool;

typedef struct {
    double re;
    double im;
} complex;

complex complex_add(complex c1, complex c2) {
    complex c;
    c.re = c1.re + c2.re;
    c.im = c1.im + c2.im;
    return c;
}

complex complex_mul(complex c1, complex c2) {
    complex c;
    c.re = c1.re * c2.re - c1.im * c2.im;
    c.im = c1.re * c2.im + c1.im * c2.re;
    return c;
}

/*
Stampa a schermo un numero complesso
evita di scrivere segni e/o coefficienti se non necessario

es: +i1 -> i
*/
void complex_print(complex c) {
    if (c.re == 0.0 && c.im == 0.0) {
        printf("0\n");
    }
    if (c.re != 0.0 && c.im == 0.0) {
        printf("%.10g\n", c.re);
    }
    if (c.re == 0.0 && c.im != 0.0) {
        if (c.im >= 0.0)
            printf("i");
        else
            printf("-i");
        
        if (fabs(c.im) == 1.0)
            printf("\n");
        else
            printf("%.10g\n", fabs(c.im));
    }
    if (c.re != 0.0 && c.im != 0.0) {
        char sep = c.im >= 0.0 ? '+' : '-'; 
        if (fabs(c.im) == 1.0)
            printf("%.10g%ci\n", c.re, sep);
        else
            printf("%.10g%ci%.10g\n", c.re, sep, fabs(c.im));
    }
    fflush(stdout);
}

typedef struct {
    char *name;
    complex *matrix; 
} gate;

// Funzione per eseguire l'operazione "matrice X vettore"
void matvec_mul(const complex *M, const complex *in_vec, complex *out_vec, int dim) {
    for (int i = 0; i < dim; i++) {
        complex sum = {0.0, 0.0};
        for (int j = 0; j < dim; j++) {
            complex prod = complex_mul(M[i * dim + j], in_vec[j]);
            sum = complex_add(sum, prod);
        }
        out_vec[i] = sum;
    }
}

void free_gates(gate *gates, int n_gates) {
    if (!gates) return;
    for (int i = 0; i < n_gates; i++) {
        free(gates[i].name);
        free(gates[i].matrix);
    }
    free(gates);
}

int parse_real(const char *str, double *out) {
    if (str == NULL || *str == '\0') return 1;
    errno = 0;
    char *end_ptr;
    double res = strtod(str, &end_ptr);

    if (end_ptr == str || *end_ptr != '\0') return 1;
    if (errno != 0) return 1;

    *out = res;
    return 0;
}

int parse_imag(const char *str, complex *out) {
    if (str == NULL || *str == '\0') return 1;

    const char *p_str = str; // Crea un secondo puntatore cosi' che potra' essere incrementato

    // Salva il segno e salta al carattere successivo se il segno è esplicito
    double sign = 1.0;
    if (*p_str == '+')
        p_str++;
    else if (*p_str == '-') {
        sign = -1.0;
        p_str++;
    }

    // Controlla se dopo il segno e' presente "i" (Se non lo e' ritorna 0)
    if (*p_str != 'i') return 1;
    p_str++;

    // Solo "i", ritorna subito con risultato valido.
    if (*p_str == '\0') {
        out->re = 0.0;
        out->im = 1.0*sign;
        return 0;
    }

    double parsed_coeff;
    if(parse_real(p_str, &parsed_coeff)) return 1;
    out->re = 0.0;
    out->im = parsed_coeff*sign;

    return 0;
}

// Il numero complesso in input deve essere del formato a+ib (a e b possono essere 0).
int parse_complex(const char *str, complex *out) {
    if (str == NULL || *str == '\0') return 1;

    // Prima cosa controlliamo se c'e' una i
    const char *p_i = strchr(str, 'i');
    if (p_i == NULL) {
        out->im = 0.0;
        return parse_real(str, &out->re);
    }

    int len = (int)strlen(str);
    // Valore base di zero, poiche' non è un valore valido
    // ci permette di verificare il successo della ricerca
    int split_index = 0;

    for (int i = 1; i < len; i++) {
        // Controllo se il segno è preceduto da una 'e', così da evitare falsi positivi
        // causati dalla notazione scientifica.
        if ((str[i] == '+' || str[i] == '-') && !(str[i-1] == 'e' || str[i-1] == 'E')) {
            split_index = i;
        }
    }

    // Non è stato trovato l'indice del separatore, è un numero immaginario
    if (split_index == 0)
        return parse_imag(str, out);
    // Trovato l'indice del separatore, quindi si procede a separare la stringa in due.
    else {
        // Separazione parte reale
        int re_len = split_index;
        char *re_str = malloc(re_len + 1);
        if (!re_str)
            return 1;

        strncpy(re_str, str, re_len);
        re_str[re_len] = '\0';
        
        // Parte reale al parser
        double temp_re;
        if (parse_real(re_str, &temp_re)) {
            free(re_str);
            return 1;
        }
        free(re_str);

        // Separazione parte immaginaria
        int im_len = len - split_index;
        char *im_str = malloc(im_len + 1);

        strncpy(im_str, str + split_index, im_len);
        im_str[im_len] = '\0';

        // Parte immaginaria al parser
        if (parse_imag(im_str, out)) {
            free(im_str);
            return 1;
        }
        free(im_str);

        out->re = temp_re;
    }

    return 0;
}

// malloc utilizzato internamente per "out", "Caller must free"
int load_qbits_init(const char *filename, int *n_qubits, complex **out) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Errore: impossibile aprire il file %s\n", filename);
        return 1;
    }

    char line[1024];
    int qubits = 0;
    bool done_qubits = false, done_init = false;

    char *init_buf; // Buffer per il contenuto di #init (esclusi '[' e ']')

    while (fgets(line, sizeof(line), fp)) {
        if (!done_qubits && strncmp(line, "#qubits ", 8) == 0) {
            char *num_start = line + 7;
            
            errno = 0;
            char *endptr = NULL;
            qubits = strtol(num_start, &endptr, 10); // strtol così da usare errno

            if (qubits < 1 || qubits > 30 || errno != 0 || endptr == num_start || (*endptr != '\0' && *endptr != '\n')) {
                fclose(fp);
                return 1;
            }
            
            done_qubits = true;
        }

        if (!done_init && strncmp(line, "#init ", 6) == 0) {
            // Prendo input tra parentesi quadre
            char *lbr = strchr(line, '[');
            char *rbr = strchr(line, ']');
            if (!lbr || !rbr || rbr < lbr) {
                fclose(fp);
                return 1;
            }

            size_t len = rbr - lbr - 1;

            init_buf = malloc(len + 1); // Allocazione di memoria per il buffer
            if (!init_buf) {
                fclose(fp);
                return 1;
            }

            strncpy(init_buf, lbr + 1, len); // Copia la parte tra parentesi quadre
            init_buf[len] = '\0';
            done_init = 1;
        }

        // Finisce prime di EOF in caso abbia trovato tutti i dati
        if (done_qubits && done_init)
            break;
    }

    fclose(fp);

    // Controllo se ci sono tutti i dati
    if (!done_qubits || !done_init)
        return 1;

    size_t dim = 1 << qubits; // Dimensione vettore 2^n
    size_t idx = 0;

    complex *vec = malloc(dim * sizeof(complex)); // Allocazione vettore finale

    // Manda al parser tutto cio' che è tra le virgole, senza spazi iniziali
    char *tkn = strtok(init_buf, ",");
    while (tkn != NULL) {
        while (*tkn == ' ') tkn++;
        
        if (parse_complex(tkn, &vec[idx])) {
            free(vec);
            free(init_buf);
            return 1;
        }

        idx++;
        tkn = strtok(NULL, ",");
    }

    free(init_buf);

    if (idx != dim) {
        free(vec);
        return 1;
    }

    *out = vec;
    *n_qubits = qubits;

    return 0;
}

int read_gates_circ(const char *filename, int *n_gates_out, gate **circuit_out, const int n_qubits) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Errore: impossibile aprire il file %s\n", filename);
        return 1;
    }

    int n_gates = 0;
    gate *circuit = malloc(1 * sizeof(gate));
    if (!circuit) {
        fclose(fp);
        return 1;
    }
    char *mat_buf, *row_buf;
    size_t dim = 1 << n_qubits;
    char line[2048];
    char circ_in[2048];

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "#define ", 8) == 0) {
            gate t_gate;

            char* gate_name;
            char* name_start = line + 7;
            while (*name_start == ' ') name_start++;
            char *lbr = strchr(line, '[');
            char *rbr = strchr(line, ']');
            if (!lbr || !rbr || rbr < lbr) {
                free_gates(circuit, n_gates);
                fclose(fp);
                return 1;
            }
            char *name_end = lbr - 1; // name_end parte da lbr - 1 e toglie spazi a destra
            while (*name_end == ' ') name_end--;

            int name_len = name_end - name_start + 1;
            if (name_len > 7) { // Nome troppo lungo
                free_gates(circuit, n_gates);
                fclose(fp);
                return 1;
            }

            gate_name = malloc(name_len + 1);
            if (!gate_name) {
                free_gates(circuit, n_gates);
                fclose(fp);
                return 1;
            }

            strncpy(gate_name, name_start, name_len); // Copia il nome
            gate_name[name_len] = '\0';
            t_gate.name = gate_name;

            size_t len = rbr - lbr - 1;

            mat_buf = malloc(len + 1); // Allocazione di memoria per il buffer
            if (!mat_buf) {
                free(gate_name);
                free_gates(circuit, n_gates);
                fclose(fp);
                return 1;
            }

            strncpy(mat_buf, lbr + 1, len); // Copia la parte tra parentesi quadre
            mat_buf[len] = '\0';

            // Dopo aver acquisito i contenuti tra parentesi quadre
            // si prendono le righe della matrice (Parentesi tonde)

            // Alloca memoria per la matrice, nella quale verranno inseriti i numeri
            // mentre verranno isolati da ogni riga
            complex *t_mat = malloc(dim * dim * sizeof(complex));
            if (!t_mat) {
                free(mat_buf);
                free(gate_name);
                free_gates(circuit, n_gates);
                fclose(fp);
                return 1;
            }
            int mat_idx = 0; // Posizione all'interno della matrice finale

            int row_idx = 0;
            char *p_mat_buf = mat_buf; // Nuovo puntatore a mat_buf da poter manipolare
            while (row_idx < dim) { // Limite "duro", dato che sappiamo quante righe aspettiamo

                char *row_lbr = strchr(p_mat_buf, '(');
                char *row_rbr = strchr(p_mat_buf, ')');
                if (!row_lbr || !row_rbr || row_rbr < row_lbr) {
                    free(t_mat);
                    free(mat_buf);
                    free(gate_name);
                    free_gates(circuit, n_gates);
                    fclose(fp);
                    return 1;
                }

                size_t row_len = row_rbr - row_lbr - 1;

                row_buf = malloc(row_len + 1); // Allocazione di memoria per il buffer
                if (!row_buf) {
                    free(t_mat);
                    free(mat_buf);
                    free(gate_name);
                    free_gates(circuit, n_gates);
                    fclose(fp);
                    return 1;
                }

                strncpy(row_buf, row_lbr + 1, row_len); // Copia la parte tra parentesi tonde
                row_buf[row_len] = '\0';

                // Manda al parser tutto cio' che è tra le virgole, come per init
                // mettendole in un array (t_mat) che verra' assegnato al gate come sua matrice

                char *tkn = strtok(row_buf, ",");
                while (tkn != NULL) {
                    while (*tkn == ' ') tkn++;

                    if (parse_complex(tkn, &t_mat[mat_idx])) {
                        free(row_buf);
                        free(t_mat);
                        free(mat_buf);
                        free(gate_name);
                        free_gates(circuit, n_gates);
                        fclose(fp);
                        return 1;
                    }
                    
                    mat_idx++;
                    tkn = strtok(NULL, ",");
                }

                if (mat_idx % dim != 0) {
                    free(row_buf);
                    free(t_mat);
                    free(mat_buf);
                    free(gate_name);
                    free_gates(circuit, n_gates);
                    fclose(fp);
                    return 1;
                }


                p_mat_buf += row_len + 2; // Sposto di row_len e aggiungo le parentedi tonde alla lunghezza
                row_idx++;
                free(row_buf);
            }
            t_gate.matrix = t_mat;
            if (n_gates > 0) {
                gate *new_circuit = realloc(circuit, (n_gates + 1) * sizeof(gate)); // Aumento memoria circuito di 1 gate alla volta.
                if (!new_circuit) {
                    free(t_mat);
                    free(mat_buf);
                    free(gate_name);
                    free_gates(circuit, n_gates);
                    fclose(fp);
                    return 1;
                }
                circuit = new_circuit;
            }
            circuit[n_gates] = t_gate;
            n_gates++;

            free(mat_buf);
            if (row_idx != dim) {
                free_gates(circuit, n_gates);
                fclose(fp);
                return 1;
            }
        }
        else if (strncmp(line, "#circ ", 6) == 0) {
            char *circ_start = line + 5; // Puntatore da manipolare
            while (*circ_start == ' ') circ_start++;

            strncpy(circ_in, circ_start, strlen(circ_start));
            circ_in[strlen(circ_start) - 1] = '\0';
        }
    }

    fclose(fp);

    *n_gates_out = n_gates;

    // Gate nell'ordine giusto
    int idx_tkn = 0;
    int idx_gate;
    gate tmp;
    char *tkn_gate = strtok(circ_in, " "); // Prendo i gate uno alla volta dall'input e li cerco nel circuito caricato
    while (tkn_gate != NULL) {
        for (idx_gate = idx_tkn; idx_gate < n_gates; idx_gate++) {
            if (strcmp(circuit[idx_gate].name, tkn_gate) == 0) {
                // Trovato il gate corrispondente metto in ordine
                if (idx_gate != idx_tkn) {
                    tmp      = circuit[idx_tkn];
                    circuit[idx_tkn] = circuit[idx_gate];
                    circuit[idx_gate] = tmp;
                }
                break;
            }
        }
        if (idx_gate == n_gates) { // Se non è stato trovato il gate tra quelli caricati errore.
            free_gates(circuit, n_gates);
            return 1;
        }
        tkn_gate = strtok(NULL, " ");
        idx_tkn++;
    }

    *circuit_out = circuit; // out del circuito già sortato

    return 0;
}

int main(void) {

    return 0;
}