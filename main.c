#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h> // Per fabs()
#include <string.h>

/// @brief Definizione del tipo bool per codice più leggibile
typedef unsigned char bool;

#ifndef true
    #define true  1
#endif
#ifndef false
    #define false 0
#endif

/// @brief Definizione di complex (numero complesso, con parte reale e parte immaginaria double)
typedef struct {
    double re;
    double im;
} complex;

/// @brief Funzione che addiziona due numeri complessi
/// @param c1 Numero complesso
/// @param c2 Numero complesso
/// @return Nuovo numero complesso contenente la somma
complex complex_add(complex c1, complex c2) {
    complex c;
    c.re = c1.re + c2.re;
    c.im = c1.im + c2.im;
    return c;
}

/// @brief Funzione che addiziona due numeri complessi
/// @param c1 Numero complesso
/// @param c2 Numero complesso
/// @return Nuovo numero complesso contenente il risultato
complex complex_mul(complex c1, complex c2) {
    complex c;
    c.re = c1.re * c2.re - c1.im * c2.im;
    c.im = c1.re * c2.im + c1.im * c2.re;
    return c;
}

/// @brief Stampa a schermo un numero complesso
/// @param c Numero da stampare
/// @param endchar Carattere con cui terminare la stampa ('\0' di default)
void complex_print(complex c, char endchar) {
    if (!endchar) endchar = '\0';

    if (c.re == 0.0 && c.im == 0.0) {
        printf("0%c", endchar);
    }
    if (c.re != 0.0 && c.im == 0.0) {
        printf("%.10g%c", c.re, endchar);
    }
    if (c.re == 0.0 && c.im != 0.0) {
        if (c.im >= 0.0)
            printf("i");
        else
            printf("-i");
        
        // Evita di scrivere il coefficiente se non è necessario
        if (fabs(c.im) == 1.0)
            printf("%c", endchar);
        else
            printf("%.10g%c", fabs(c.im), endchar);
    }
    if (c.re != 0.0 && c.im != 0.0) {
        char sep = c.im >= 0.0 ? '+' : '-'; 
        // Evita di scrivere il coefficiente se non è necessario
        if (fabs(c.im) == 1.0)
            printf("%.10g%ci%c", c.re, sep, endchar);
        else
            printf("%.10g%ci%.10g%c", c.re, sep, fabs(c.im), endchar);
    }
}

/// @brief Struttura dati per contenere i gate
typedef struct {
    char *name;
    complex *matrix; 
} gate;

/// @brief Funzione per liberare i gate di un circuito e il circuito stesso
/// @param circuit Puntatore al circuito (Array di gate) 
/// @param n_gates Numero di gate da liberare
void free_circuit(gate *circuit, int n_gates) {
    if (!circuit) return;
    for (int i = 0; i < n_gates; i++) {
        free(circuit[i].name);
        free(circuit[i].matrix);
    }
    free(circuit);
}

/// @brief Funzione per eseguire l'operazione "matrice X vettore"
/// @param M Array di complessi (verra' trattato come una matrice)
/// @param in_vec Array di complessi (vettore colonna)
/// @param out_vec Puntatore all'array dove verrà salvato il risultato
/// @param dim Numero di righe == Numero di colonne della matrice
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

/// @brief Parser di numeri reali
/// @param str Stringa da cui prendere il numero
/// @param out Puntatore al double in cui salvare il numero
/// @return EXIT_FAILURE o EXIT_SUCCESS
int parse_real(const char *str, double *out) {
    if (str == NULL || *str == '\0') {
        fprintf(stderr, "Errore nel parser, stringa vuota\n");
        return EXIT_FAILURE;
    }
    errno = 0;
    char *end_ptr;
    double res = strtod(str, &end_ptr);

    if (end_ptr == str || *end_ptr != '\0') {
        fprintf(stderr, "Errore nel parser, Numero non trovato o malformato\n");
        return EXIT_FAILURE;
    }
    if (errno != 0) {
        perror("Errore nel parser:\n");
        return EXIT_FAILURE;
    };

    *out = res;
    return EXIT_SUCCESS;
}

/// @brief Parser di numeri immaginari, il numero immaginario in input deve essere del formato ib (b può essere 0).
/// @param str Stringa da cui prendere il numero
/// @param out Puntatore al complex in cui salvare il numero
/// @return EXIT_FAILURE o EXIT_SUCCESS
int parse_imag(const char *str, complex *out) {
    if (str == NULL || *str == '\0') {
        fprintf(stderr, "Errore nel parser, stringa vuota\n");
        return EXIT_FAILURE;
    }

    const char *p_str = str; // Crea un secondo puntatore cosi' che potra' essere incrementato

    // Salva il segno e salta al carattere successivo se il segno e' esplicito
    double sign = 1.0;
    if (*p_str == '+')
        p_str++;
    else if (*p_str == '-') {
        sign = -1.0;
        p_str++;
    }

    // Controlla se dopo il segno e' presente "i" (Se non lo e' ritorna 1)
    if (*p_str != 'i') {
        fprintf(stderr, "Errore nel parser, atteso numero immaginario, 'i' non trovata (%s)\n", str);
        return EXIT_FAILURE;
    }
    p_str++;

    // Solo "i", ritorna subito con risultato valido.
    if (*p_str == '\0') {
        out->re = 0.0;
        out->im = 1.0*sign;
        return EXIT_SUCCESS;
    }

    double parsed_coeff;
    if(parse_real(p_str, &parsed_coeff)) return EXIT_FAILURE;
    out->re = 0.0;
    out->im = parsed_coeff*sign;

    return EXIT_SUCCESS;
}

/// @brief Parser di numeri immaginari, il numero complesso in input deve essere del formato a+ib (a e b possono essere 0).
/// @param str Stringa da cui prendere il numero
/// @param out Puntatore al complex in cui salvare il numero
/// @return EXIT_FAILURE o EXIT_SUCCESS
int parse_complex(const char *str, complex *out) {
    if (str == NULL || *str == '\0') {
        fprintf(stderr, "Errore nel parser, stringa vuota\n");
        return EXIT_FAILURE;
    }

    // Se non c'e' una 'i' e' un numero reale
    const char *p_i = strchr(str, 'i');
    if (p_i == NULL) {
        if (parse_real(str, &out->re)) {
            return EXIT_FAILURE;
        }  
        else {
            out->im = 0.0;
            return EXIT_SUCCESS;
        }
    }

    int len = (int)strlen(str);

    // Valore base di zero, poiche' non e' un valore valido
    // ci permette di verificare il successo della ricerca
    int split_index = 0;

    for (int i = 1; i < len; i++) {
        // Controllo se il segno e' preceduto da una 'e', così da evitare falsi positivi
        // causati dalla notazione scientifica.
        if ((str[i] == '+' || str[i] == '-') && !(str[i-1] == 'e' || str[i-1] == 'E')) {
            split_index = i;
        }
    }

    // Non e' stato trovato l'indice del separatore, e' un numero immaginario
    if (split_index == 0)
        return parse_imag(str, out);
    // Trovato l'indice del separatore, quindi si procede a separare la stringa in re e im.
    else {
        // Separazione parte reale
        int re_len = split_index;
        char *re_str = malloc(re_len + 1);
        if (!re_str) {
            perror("Errore nel parser: ");
            return EXIT_FAILURE;
        }

        strncpy(re_str, str, re_len);
        re_str[re_len] = '\0';
        
        // Parte reale al parser
        double temp_re;
        if (parse_real(re_str, &temp_re)) {
            free(re_str);
            return EXIT_FAILURE;
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
            return EXIT_FAILURE;
        }
        free(im_str);

        out->re = temp_re;
    }

    return EXIT_SUCCESS;
}

/// @brief Carica i dati dei qubit e del vettore da file
/// @param filename Nome del file da cui leggere
/// @param n_qubits Puntatore all'int in cui salvare il numero di qubits
/// @param out_vec  Puntatore all'array di complessi in cui salvare il vettore letto da file
/// @return EXIT_FAILURE o EXIT_SUCCESS
/// malloc utilizzato internamente per "out", "Caller must free"
int load_qubits_init(const char *filename, int *n_qubits, complex **out_vec) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        return EXIT_FAILURE;
    }

    char line[1024];
    int qubits = 0;
    bool done_qubits = false, done_init = false;

    char *init_buf = NULL; // Buffer per il contenuto di #init (esclusi '[' e ']')

    int idx_line = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (!done_qubits && strncmp(line, "#qubits ", 8) == 0) {
            char *num_start = line + 7;
            
            errno = 0;
            char *endptr = NULL;
            qubits = strtol(num_start, &endptr, 10); // strtol così da usare errno

            if (qubits < 1 || qubits > 30) {
                fprintf(stderr, "Errore in %s, riga %d: Numero qubits non valido (0<x<31)\n", filename, idx_line);
                if (init_buf) free(init_buf);
                fclose(fp);
                return EXIT_FAILURE;
            }
            if (errno != 0 || endptr == num_start || (*endptr != '\0' && *endptr != '\n')) {
                fprintf(stderr, "Errore in %s, riga %d: Parsing numero fallito (%s)\n", filename, idx_line, strerror(errno));
                if (init_buf) free(init_buf);
                fclose(fp);
                return EXIT_FAILURE;
            }
            
            done_qubits = true;
        }

        if (!done_init && strncmp(line, "#init ", 6) == 0) {
            // Prendo input tra parentesi quadre
            char *lbr = strchr(line, '[');
            char *rbr = strchr(line, ']');
            if (!lbr || !rbr || rbr < lbr) {
                fprintf(stderr, "Errore in %s, riga %d: Parentesi quadre malformate\n", filename, idx_line);
                fclose(fp);
                return EXIT_FAILURE;
            }

            size_t len = rbr - lbr - 1;

            init_buf = malloc(len + 1); // Allocazione di memoria per il buffer
            if (!init_buf) {
                perror("Allocazione memoria fallita");
                fclose(fp);
                return EXIT_FAILURE;
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
    if (!done_qubits) {
        fprintf(stderr, "Errore in %s: Numero qubits mancante\n", filename);
        free(init_buf);
        return EXIT_FAILURE;
    }
    if (!done_init) {
        fprintf(stderr, "Errore in %s: Vettore init mancante\n", filename);
        free(init_buf);
        return EXIT_FAILURE;
    }

    size_t dim = 1 << qubits; // Dimensione vettore 2^n
    size_t idx = 0;

    complex *vec = malloc(dim * sizeof(complex)); // Allocazione vettore finale

    // Manda al parser tutto cio' che e' tra le virgole, senza spazi iniziali
    char *tkn = strtok(init_buf, ",");
    while (tkn != NULL) {
        if (idx == dim) {
            fprintf(stderr, "Errore in %s, riga %d: Elementi di #init superiori al necessario\n", filename, idx_line);
            free(vec);
            free(init_buf);
            return EXIT_FAILURE;
        }
        
        while (*tkn == ' ') tkn++;
        
        if (parse_complex(tkn, &vec[idx])) {
            fprintf(stderr, "Errore in %s, riga %d: Parsing numero fallito (%s)\n", filename, idx_line, tkn);
            free(vec);
            free(init_buf);
            return EXIT_FAILURE;
        }
        idx++;
        tkn = strtok(NULL, ",");
    }

    free(init_buf);

    if (idx < dim) {
        fprintf(stderr, "Errore in %s, riga %d: Elementi di #init inferiori al necessario\n", filename, idx_line);
        free(vec);
        return EXIT_FAILURE;
    }

    *out_vec = vec;
    *n_qubits = qubits;

    return EXIT_SUCCESS;
}

/// @brief Carica i dati del circuito e dei gate da file
/// @param filename Nome del file da cui leggere
/// @param n_gates_out Puntatore all'int in cui salvare il numero di gate caricati
/// @param circuit_out Puntatore al circuito (gate*)
/// @param n_qubits Numero di qubits (Serve per la dimensione delle matrici)
/// @return EXIT_FAILURE o EXIT_SUCCESS
/// malloc utilizzato internamente per "circuit_out", "Caller must free"
/// @see free_circuit()
int load_gates_circ(const char *filename, int *n_gates_out, gate **circuit_out, const int n_qubits) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        return EXIT_FAILURE;
    }

    int n_gates = 0;
    gate *circuit = malloc(1 * sizeof(gate));
    if (!circuit) {
        perror("Allocazione memoria fallita");
        fclose(fp);
        return EXIT_FAILURE;
    }
    char *mat_buf, *row_buf;
    size_t dim = 1 << n_qubits;
    char line[2048];
    char circ_in[2048];
    int idx_line = 1; // Indice linea, serve per gli errori
    bool done_circ = false;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "#define ", 8) == 0) {
            gate t_gate;

            char* gate_name;
            char* name_start = line + 7;
            while (*name_start == ' ') name_start++;
            char *lbr = strchr(line, '[');
            char *rbr = strchr(line, ']');
            if (!lbr || !rbr || rbr < lbr) {
                fprintf(stderr, "Errore in %s, riga %d: Parentesi quadre malformate\n", filename, idx_line);
                free_circuit(circuit, n_gates);
                fclose(fp);
                return EXIT_FAILURE;
            }
            char *name_end = name_start; // name_end parte da name_start e va verso destra fino al primo spazio
            while (*name_end != ' ') name_end++;

            // int è abbastanza per contenere il risultato anche se dovrebbe essere un long
            int name_len = name_end - name_start;
            if (name_len > 15) { // Nome troppo lungo
                fprintf(stderr, "Errore in %s, riga %d: Lunghezza massima nome gate 15 caratteri\n", filename, idx_line);
                free_circuit(circuit, n_gates);
                fclose(fp);
                return EXIT_FAILURE;
            }

            gate_name = malloc(name_len + 1);
            if (!gate_name) {
                perror("Allocazione memoria fallita");
                free_circuit(circuit, n_gates);
                fclose(fp);
                return EXIT_FAILURE;
            }

            strncpy(gate_name, name_start, name_len); // Copia il nome
            gate_name[name_len] = '\0';
            t_gate.name = gate_name;

            size_t len = rbr - lbr - 1;

            mat_buf = malloc(len + 1); // Allocazione di memoria per il buffer
            if (!mat_buf) {
                perror("Allocazione memoria fallita");
                free(gate_name);
                free_circuit(circuit, n_gates);
                fclose(fp);
                return EXIT_FAILURE;
            }

            strncpy(mat_buf, lbr + 1, len); // Copia la parte tra parentesi quadre
            mat_buf[len] = '\0';

            // Dopo aver acquisito i contenuti tra parentesi quadre
            // si prendono le righe della matrice (Parentesi tonde)

            // Alloca memoria per la matrice, nella quale verranno inseriti i numeri
            // mentre verranno isolati da ogni riga
            complex *t_mat = malloc(dim * dim * sizeof(complex));
            if (!t_mat) {
                perror("allocazione memoria fallita");
                free(mat_buf);
                free(gate_name);
                free_circuit(circuit, n_gates);
                fclose(fp);
                return EXIT_FAILURE;
            }
            size_t mat_idx = 0; // Posizione all'interno della matrice finale

            size_t row_idx = 0;
            char *p_mat_buf = mat_buf; // Nuovo puntatore a mat_buf da poter manipolare
            while (row_idx < dim) { // Limite "duro", dato che sappiamo quante righe aspettiamo

                char *row_lbr = strchr(p_mat_buf, '(');
                char *row_rbr = strchr(p_mat_buf, ')');
                if (!row_lbr || !row_rbr || row_rbr < row_lbr) {
                    if (!row_lbr && !row_rbr) fprintf(stderr, "Errore in %s, riga %d: Il numero di righe dalla matrice e' inferiore al necessario\n", filename, idx_line);
                    else fprintf(stderr, "Errore in %s, riga %d: Parentesi tonde malformate\n", filename, idx_line);
                    free(t_mat);
                    free(mat_buf);
                    free(gate_name);
                    free_circuit(circuit, n_gates);
                    fclose(fp);
                    return EXIT_FAILURE;
                }

                size_t row_len = row_rbr - row_lbr - 1;

                row_buf = malloc(row_len + 1); // Allocazione di memoria per il buffer
                if (!row_buf) {
                    perror("Allocazione memoria fallita");
                    free(t_mat);
                    free(mat_buf);
                    free(gate_name);
                    free_circuit(circuit, n_gates);
                    fclose(fp);
                    return EXIT_FAILURE;
                }

                strncpy(row_buf, row_lbr + 1, row_len); // Copia la parte tra parentesi tonde
                row_buf[row_len] = '\0';

                // Manda al parser tutto cio' che e' tra le virgole, come per init
                // mettendole in un array (t_mat) che verra' assegnato al gate come sua matrice

                char *tkn = strtok(row_buf, ",");
                while (tkn != NULL) {
                    while (*tkn == ' ') tkn++;

                    if (parse_complex(tkn, &t_mat[mat_idx])) {
                        fprintf(stderr, "Errore in %s, riga %d, (%s): Parsing numero fallito\n", filename, idx_line, tkn);
                        free(row_buf);
                        free(t_mat);
                        free(mat_buf);
                        free(gate_name);
                        free_circuit(circuit, n_gates);
                        fclose(fp);
                        return EXIT_FAILURE;
                    }
                    
                    mat_idx++;
                    tkn = strtok(NULL, ",");
                }

                if (mat_idx % dim != 0) {
                    fprintf(stderr, "Errore in %s, riga %d: Numero errato di componenti della riga\n", filename, idx_line);
                    free(row_buf);
                    free(t_mat);
                    free(mat_buf);
                    free(gate_name);
                    free_circuit(circuit, n_gates);
                    fclose(fp);
                    return EXIT_FAILURE;
                }


                p_mat_buf += row_len + 3; // Sposto di row_len e aggiungo le parentesi tonde e lo spazio alla lunghezza
                row_idx++;
                free(row_buf);
            }

            t_gate.matrix = t_mat;

            if (n_gates > 0) {
                gate *new_circuit = realloc(circuit, (n_gates + 1) * sizeof(gate)); // Aumento memoria circuito di 1 gate alla volta.
                if (!new_circuit) {
                    perror("Allocazione memoria fallita");
                    free(t_mat);
                    free(mat_buf);
                    free(gate_name);
                    free_circuit(circuit, n_gates);
                    fclose(fp);
                    return EXIT_FAILURE;
                }
                circuit = new_circuit;
            }
            circuit[n_gates] = t_gate;
            n_gates++;

            char *check_end_mat = strchr(p_mat_buf, '('); // Check if there were other rows (error)
            free(mat_buf);
            if (check_end_mat) {
                fprintf(stderr, "Errore in %s, riga %d: Il numero di righe dalla matrice e' superiore al necessario\n", filename, idx_line);
                free_circuit(circuit, n_gates);
                fclose(fp);
                return EXIT_FAILURE;
            }
        }
        else if (!done_circ && strncmp(line, "#circ ", 6) == 0) {
            char *circ_start = line + 5; // Puntatore da manipolare
            while (*circ_start == ' ') circ_start++;

            strncpy(circ_in, circ_start, strlen(circ_start));
            circ_in[strlen(circ_start) - 1] = '\0';
            done_circ = true;
        }
        idx_line++;
    }

    fclose(fp);

    if (!done_circ) {
        fprintf(stderr, "Errore in %s: Circuito non trovato\n", filename);
        free_circuit(circuit, n_gates);
        return EXIT_FAILURE;
    }

    // ate nell'ordine giusto
    int idx_tkn = 0;
    int idx_gate;
    gate tmp;
    char *tkn_gate = strtok(circ_in, " "); // Prendo i gate uno alla volta dall'input e li cerco nel circuito caricato
    while (tkn_gate != NULL) {
        for (idx_gate = idx_tkn; idx_gate < n_gates; idx_gate++) {
            if (strcmp(circuit[idx_gate].name, tkn_gate) == 0) {
                // Trovato il gate corrispondente metto in ordine
                if (idx_gate != idx_tkn) {
                    tmp               = circuit[idx_tkn];
                    circuit[idx_tkn]  = circuit[idx_gate];
                    circuit[idx_gate] = tmp;
                }
                break;
            }
        }
        if (idx_gate == n_gates) { // Se non e' stato trovato il gate tra quelli caricati errore.
            fprintf(stderr, "Errore in %s, riga %d: Gate inesistente (%s)\n", filename, idx_line, tkn_gate);
            free_circuit(circuit, n_gates);
            return EXIT_FAILURE;
        }
        tkn_gate = strtok(NULL, " ");
        idx_tkn++;
    }

    *circuit_out = circuit; // Out del circuito già sortato
    *n_gates_out = n_gates; // Out del numero di gates

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <init_file> <circuit_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *init_file = argv[1], *circ_file = argv[2];

    // Carica qubits e vec
    int n_qubits;
    complex *vec;

    if (load_qubits_init(init_file, &n_qubits, &vec)) {
        return EXIT_FAILURE;
    }

    // Carica numero di gate e circuit
    int n_gates;
    gate *circuit;

    if(load_gates_circ(circ_file, &n_gates, &circuit, n_qubits)) {
        free(vec);
        return EXIT_FAILURE;
    }

    size_t dim = 1 << n_qubits;
    complex *t_vec = malloc(dim * sizeof(complex)); // Array temp di supporto per la moltiplicazione

    // Moltiplico secondo l'ordine dato in input
    for (int i = 0; i < n_gates; i++) {
        matvec_mul(circuit[i].matrix, vec, t_vec, dim);

        for (size_t j = 0; j < dim; j++)
            vec[j] = t_vec[j];
    }

    // Stampa in stdout dello stato finale
    printf("[");
    for (size_t i = 0; i < dim; i++) {
        complex_print(vec[i], 0x0);
        if (i != dim-1) printf(", ");
    }
    printf("]\n");

    free_circuit(circuit, n_gates);
    free(vec);
    free(t_vec);
    fflush(stdout);

    return EXIT_SUCCESS;
}