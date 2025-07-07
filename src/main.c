#include <stdlib.h>
#include <stdio.h>
#include "complex.h"
#include "gate.h"
#include "parser.h"
#include "loader.h"
#include "utils.h"

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
        fprintf(stderr, "Errore caricando il file %s\n", init_file);
        return EXIT_FAILURE;
    }

    // Carica numero di gate e circuit
    int n_gates;
    gate *circuit;

    if(load_gates_circ(circ_file, &n_gates, &circuit, n_qubits)) {
        fprintf(stderr, "Errore caricando il file %s\n", circ_file);
        free(vec);
        return EXIT_FAILURE;
    }

    size_t dim = 1UL << n_qubits;
    complex *t_vec = malloc(dim * sizeof(complex)); // Array temp di supporto per la moltiplicazione
    if (!t_vec) {
        perror("Allocazione memoria fallita");
        free_circuit(circuit, n_gates);
        free(vec);
        return EXIT_FAILURE;
    }

    // Moltiplico secondo l'ordine dato in input
    for (int i = 0; i < n_gates; i++) {
        matvec_mul(circuit[i].matrix, vec, t_vec, dim);

        for (size_t j = 0; j < dim; j++)
            vec[j] = t_vec[j];
    }

    // Stampa in stdout dello stato finale
    complex_vec_print(vec, dim);

    free(t_vec);
    free_circuit(circuit, n_gates);
    free(vec);
    fflush(stdout);

    return EXIT_SUCCESS;
}