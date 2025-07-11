#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "loader.h"
#include "parser.h"
#include "utils.h"

int load_qubits_init(const char *filename, int *n_qubits, complex **out_vec) {
    FILE *fp = fopen(filename, "r");
    int qubits = 0;
    unsigned char done_qubits = 0, done_init = 0;
    char *init_buf = NULL;
    int idx_line = 1;
    char *line = NULL;

    if (!fp) {
        perror(filename);
        return EXIT_FAILURE;
    }

    while ((line = read_line(fp)) != NULL) {
        if (!done_qubits && strncmp(line, "#qubits ", 8) == 0) {
            char *num_start = line + 7;
            errno = 0;
            char *endptr = NULL;
            qubits = strtol(num_start, &endptr, 10);

            if (errno != 0 || endptr == num_start || (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')) {
                fprintf(stderr, "Errore in %s, riga %d: Parsing numero fallito (%s)\n", filename, idx_line, strerror(errno));
                if (init_buf) free(init_buf);
                free(line);
                fclose(fp);
                return EXIT_FAILURE;
            }
            if (qubits < 1 || qubits > 30) {
                fprintf(stderr, "Errore in %s, riga %d: Numero qubits non valido (0<x<31)\n", filename, idx_line);
                if (init_buf) free(init_buf);
                free(line);
                fclose(fp);
                return EXIT_FAILURE;
            }
            done_qubits = 1;
        }

        if (!done_init && strncmp(line, "#init ", 6) == 0) {
            char *lbr = strchr(line, '[');
            char *rbr = strchr(line, ']');
            if (!lbr || !rbr || rbr < lbr) {
                fprintf(stderr, "Errore in %s, riga %d: Parentesi quadre malformate\n", filename, idx_line);
                free(line);
                fclose(fp);
                return EXIT_FAILURE;
            }

            size_t len = rbr - lbr - 1;
            init_buf = malloc(len + 1);
            if (!init_buf) {
                perror("Allocazione memoria fallita");
                free(line);
                fclose(fp);
                return EXIT_FAILURE;
            }
            strncpy(init_buf, lbr + 1, len);
            init_buf[len] = '\0';
            done_init = 1;
        }

        free(line);
        if (done_qubits && done_init) break;
        idx_line++;
    }
    fclose(fp);

    if (!done_qubits) {
        fprintf(stderr, "Errore in %s: Numero qubits mancante\n", filename);
        if (init_buf) free(init_buf);
        return EXIT_FAILURE;
    }
    if (!done_init) {
        fprintf(stderr, "Errore in %s: Vettore init mancante\n", filename);
        free(init_buf);
        return EXIT_FAILURE;
    }

    size_t dim = 1 << qubits;
    size_t idx = 0;
    complex *vec = malloc(dim * sizeof(complex));
    if (!vec) {
        perror("Allocazione memoria fallita");
        free(init_buf);
        return EXIT_FAILURE;
    }

    char *tkn = strtok(init_buf, ",");
    while (tkn) {
        if (idx == dim) {
            fprintf(stderr, "Errore in %s: Elementi di #init superiori al necessario\n", filename);
            free(vec);
            free(init_buf);
            return EXIT_FAILURE;
        }
        trim_whitespace(tkn);
        if (parse_complex(tkn, &vec[idx])) {
            fprintf(stderr, "Errore in %s: Parsing numero fallito (%s)\n", filename, tkn);
            free(vec);
            free(init_buf);
            return EXIT_FAILURE;
        }
        idx++;
        tkn = strtok(NULL, ",");
    }
    free(init_buf);

    if (idx < dim) {
        fprintf(stderr, "Errore in %s: Elementi di #init inferiori al necessario\n", filename);
        free(vec);
        return EXIT_FAILURE;
    }

    *out_vec = vec;
    *n_qubits = qubits;
    return EXIT_SUCCESS;
}

int load_gates_circ(const char *filename, int *n_gates_out, gate **circuit_out, const int n_qubits) {
    FILE *fp = NULL;
    int ret = EXIT_FAILURE;
    char *line = NULL;
    char *block = NULL;
    char *mat_buf = NULL;
    char *row_buf = NULL;
    gate *circuit = NULL;
    char *circ_in = NULL;
    int n_gates = 0;
    int idx_line = 1;
    char *gate_name = NULL;
    size_t dim = 1UL << n_qubits;
    
    fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        goto cleanup;
    }

    while ((line = read_line(fp)) != NULL) {
        if (strncmp(line, "#define ", 8) == 0) {
            block = string_append(NULL, line);
            if (!block) {
                perror("Allocazione memoria fallita");
                goto cleanup;
            }

            // Trova le parentesi quadre (Anche su più righe!)
            char *open_bracket = strchr(block, '[');
            while (!open_bracket) {
                char *next_line = read_line(fp);
                if (!next_line) break;
                idx_line++;
                block = string_append(block, next_line);
                free(next_line);
                if (!block) {
                    perror("Allocazione memoria fallita");
                    goto cleanup;
                }
                open_bracket = strchr(block, '[');
            }

            if (!open_bracket) {
                fprintf(stderr, "Errore in %s, riga %d: Parentesi quadre mancanti\n", filename, idx_line);
                goto cleanup;
            }

            char *close_bracket = strchr(open_bracket, ']');
            while (!close_bracket) {
                char *next_line = read_line(fp);
                if (!next_line) break;
                idx_line++;
                block = string_append(block, next_line);
                free(next_line);
                if (!block) {
                    perror("Allocazione memoria fallita");
                    goto cleanup;
                }
                open_bracket = strchr(block, '[');
                close_bracket = strchr(open_bracket, ']');
            }

            if (!close_bracket) {
                fprintf(stderr, "Errore in %s, riga %d: Parentesi quadre non chiuse\n", filename, idx_line);
                goto cleanup;
            }

            // Estrai il nome del gate
            char *name_start = block + 8;
            while (isspace((unsigned char)*name_start)) name_start++;
            char *name_end = name_start;
            while (*name_end && !isspace((unsigned char)*name_end)) name_end++;
            int name_len = name_end - name_start;
            if (name_len > 15) {
                fprintf(stderr, "Errore in %s, riga %d: Nome gate troppo lungo\n", filename, idx_line);
                goto cleanup;
            }

            gate_name = malloc(name_len + 1);
            if (!gate_name) {
                perror("Allocazione memoria fallita");
                goto cleanup;
            }
            strncpy(gate_name, name_start, name_len);
            gate_name[name_len] = '\0';

            // Controlla per duplicati nei gate
            for (int i = 0; i < n_gates; i++) {
                if (strcmp(circuit[i].name, gate_name) == 0) {
                    fprintf(stderr, "Errore in %s, riga %d: Gate duplicato (%s)\n", filename, idx_line, gate_name);
                    goto cleanup;
                }
            }

            // Estrai la stringa contenente la matrice
            size_t len = close_bracket - open_bracket - 1;
            mat_buf = malloc(len + 1);
            if (!mat_buf) {
                perror("Allocazione memoria fallita");
                goto cleanup;
            }
            strncpy(mat_buf, open_bracket + 1, len);
            mat_buf[len] = '\0';
            
            // Libera subito block
            free(block);
            block = NULL;

            // Inizia il parsing della matrice
            complex *t_mat = malloc(dim * dim * sizeof(complex));
            if (!t_mat) {
                perror("Allocazione memoria fallita");
                goto cleanup;
            }

            size_t mat_idx = 0;
            char *p_mat_buf = mat_buf;
            for (size_t row_idx = 0; row_idx < dim; row_idx++) {
                char *row_lbr = strchr(p_mat_buf, '(');
                char *row_rbr = strchr(p_mat_buf, ')');
                if (!row_lbr || !row_rbr || row_rbr < row_lbr) {
                    fprintf(stderr, "Errore in %s, riga %d: Parentesi tonde malformate\n", filename, idx_line);
                    free(t_mat);
                    goto cleanup;
                }

                size_t row_len = row_rbr - row_lbr - 1;
                row_buf = malloc(row_len + 1);
                if (!row_buf) {
                    perror("Allocazione memoria fallita");
                    free(t_mat);
                    goto cleanup;
                }
                strncpy(row_buf, row_lbr + 1, row_len);
                row_buf[row_len] = '\0';

                char *tkn = strtok(row_buf, ",");
                while (tkn) {
                    trim_whitespace(tkn); // Il parser accetta solo input sanificato
                    if (parse_complex(tkn, &t_mat[mat_idx])) {
                        fprintf(stderr, "Errore in %s, riga %d: Parsing fallito (%s)\n", filename, idx_line, tkn);
                        free(row_buf);
                        free(t_mat);
                        row_buf = NULL;
                        goto cleanup;
                    }
                    mat_idx++;
                    tkn = strtok(NULL, ",");
                }
                free(row_buf);
                row_buf = NULL;
                p_mat_buf = row_rbr + 1;
            }
            
            // Libera subito il buffer della matrice
            free(mat_buf);
            mat_buf = NULL;

            // Crea nuovo gate
            gate t_gate;
            t_gate.name = gate_name;
            t_gate.matrix = t_mat;
            gate_name = NULL;  // Trasferita la proprieta' a t_gate

            // Aggiungi al circuito
            gate *new_circuit = realloc(circuit, (n_gates + 1) * sizeof(gate));
            if (!new_circuit) {
                perror("Allocazione memoria fallita");
                free(t_gate.name);
                free(t_gate.matrix);
                goto cleanup;
            }
            circuit = new_circuit;
            circuit[n_gates++] = t_gate;
        }
        else if (!circ_in && strncmp(line, "#circ ", 6) == 0) {
            // Estrai stringa circuito
            char *circ_start = line + 5;
            while (isspace((unsigned char)*circ_start)) circ_start++;
            circ_in = strdup(circ_start);
            if (!circ_in) {
                perror("Allocazione memoria fallita");
                goto cleanup;
            }
        }
        free(line);
        line = NULL;
        idx_line++;
    }

    // Inizia validazione e costruzione del circuito finale
    if (!circ_in) {
        fprintf(stderr, "Errore in %s: Circuito mancante\n", filename);
        goto cleanup;
    }

    // Estrae i nomi dei gate (tokens) e costruisce il circuito
    int n_circ = 0;
    char **tokens = NULL;
    gate *new_circuit = NULL;
    char *saveptr;
    char *token = strtok_r(circ_in, " ", &saveptr);
    while (token) {
        char **new_tokens = realloc(tokens, (n_circ + 1) * sizeof(char *));
        if (!new_tokens) {
            perror("Allocazione memoria fallita");
            goto circuit_cleanup;
        }
        tokens = new_tokens;
        tokens[n_circ] = strdup(token);
        if (!tokens[n_circ]) {
            perror("Allocazione memoria fallita");
            goto circuit_cleanup;
        }
        n_circ++;
        token = strtok_r(NULL, " ", &saveptr);
    }

    // Costruzione effettiva
    new_circuit = malloc(n_circ * sizeof(gate));
    if (!new_circuit) {
        perror("Allocazione memoria fallita");
        goto circuit_cleanup;
    }
    memset(new_circuit, 0, n_circ * sizeof(gate)); // Inizializzo a 0

    size_t mat_size = dim * dim * sizeof(complex);
    for (int i = 0; i < n_circ; i++) {
        int found = 0;
        for (int j = 0; j < n_gates; j++) {
            if (strcmp(circuit[j].name, tokens[i]) == 0) {
                // Duplica gate name
                new_circuit[i].name = strdup(circuit[j].name);
                if (!new_circuit[i].name) {
                    perror("Allocazione memoria fallita");
                    goto circuit_cleanup;
                }

                // Duplica matrice
                new_circuit[i].matrix = malloc(mat_size);
                if (!new_circuit[i].matrix) {
                    perror("Allocazione memoria fallita");
                    free(new_circuit[i].name);
                    new_circuit[i].name = NULL;
                    goto circuit_cleanup;
                }
                memcpy(new_circuit[i].matrix, circuit[j].matrix, mat_size);
                found = 1;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "Errore in %s: Gate non definito (%s)\n", filename, tokens[i]);
            goto circuit_cleanup;
        }
    }

    // In caso di successo trasferisco la proprieta'
    *n_gates_out = n_circ;
    *circuit_out = new_circuit;
    ret = EXIT_SUCCESS;

// Pulizia specifica della parte sulla costruzione dei circuiti
circuit_cleanup:
    if (tokens) {
        for (int i = 0; i < n_circ; i++) free(tokens[i]);
        free(tokens);
    }
    if (ret != EXIT_SUCCESS && new_circuit) {
        for (int i = 0; i < n_circ; i++) {
            if (new_circuit[i].name) free(new_circuit[i].name);
            if (new_circuit[i].matrix) free(new_circuit[i].matrix);
        }
        free(new_circuit);
    }

// Pulizia principale
cleanup:
    if (fp) fclose(fp);
    if (line) free(line);
    if (block) free(block);
    if (mat_buf) free(mat_buf);
    if (row_buf) free(row_buf);
    if (circ_in) free(circ_in);
    if (gate_name) free(gate_name);
    
    // Libera il circuito originale (Tutto ciò che serviva è stato duplicato)
    if (circuit) {
        for (int i = 0; i < n_gates; i++) {
            if (circuit[i].name) free(circuit[i].name);
            if (circuit[i].matrix) free(circuit[i].matrix);
        }
        free(circuit);
    }
    return ret;
}