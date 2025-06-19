#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "loader.h"
#include "parser.h"

int load_qubits_init(const char *filename, int *n_qubits, complex **out_vec) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        return EXIT_FAILURE;
    }

    char line[1024];
    int qubits = 0;
    unsigned char done_qubits = 0, done_init = 0; // Unsigned char come bool

    char *init_buf = NULL; // Buffer per il contenuto di #init (esclusi '[' e ']')

    int idx_line = 1; // Indice linea, serve per gli errori
    while (fgets(line, sizeof(line), fp)) {
        if (!done_qubits && strncmp(line, "#qubits ", 8) == 0) {
            char *num_start = line + 7;
            
            errno = 0;
            char *endptr = NULL;
            qubits = strtol(num_start, &endptr, 10); // strtol così da usare errno

            if (errno != 0 || endptr == num_start || (*endptr != '\0' && *endptr != '\n')) {
                fprintf(stderr, "Errore in %s, riga %d: Parsing numero fallito (%s)\n", filename, idx_line, strerror(errno));
                if (init_buf) free(init_buf);
                fclose(fp);
                return EXIT_FAILURE;
            }
            if (qubits < 1 || qubits > 30) {
                fprintf(stderr, "Errore in %s, riga %d: Numero qubits non valido (0<x<31)\n", filename, idx_line);
                if (init_buf) free(init_buf);
                fclose(fp);
                return EXIT_FAILURE;
            }
            
            done_qubits = 1;
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

        idx_line++;
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
    unsigned char done_circ = 0;

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

            // Controllo se il nome e' gia' stato preso
            for (int i = 0; i < n_gates; i++) {
                if (strcmp(circuit[i].name, gate_name) == 0) {
                    fprintf(stderr, "Errore in %s, riga %d: Esiste gia' un gate con lo stesso nome (%s)\n", filename, idx_line, gate_name);
                    free(gate_name);
                    free_circuit(circuit, n_gates);
                    fclose(fp);
                    return EXIT_FAILURE;
                }
            }

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

            char *check_end_mat = strchr(p_mat_buf, '('); // Controllo per righe in piu'
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
            done_circ = 1;
        }
        idx_line++;
    }

    fclose(fp);

    if (!done_circ) {
        fprintf(stderr, "Errore in %s: Circuito non trovato\n", filename);
        free_circuit(circuit, n_gates);
        return EXIT_FAILURE;
    }

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
                    tmp               = circuit[idx_tkn];
                    circuit[idx_tkn]  = circuit[idx_gate];
                    circuit[idx_gate] = tmp;
                }
                break;
            }
        }
        if (idx_gate == n_gates) { // Se non e' stato trovato il gate tra quelli caricati errore.
            fprintf(stderr, "Errore in %s: Gate inesistente (%s)\n", filename, tkn_gate);
            free_circuit(circuit, n_gates);
            return EXIT_FAILURE;
        }
        tkn_gate = strtok(NULL, " ");
        idx_tkn++;
    }

    if (idx_tkn != n_gates) {
        fprintf(stderr, "Errore in %s: Uno o piu' gate non sono specificati in #circ\n", filename);
        free_circuit(circuit, n_gates);
        return EXIT_FAILURE;
    }

    *circuit_out = circuit; // Out del circuito già sortato
    *n_gates_out = n_gates; // Out del numero di gates

    return EXIT_SUCCESS;
}