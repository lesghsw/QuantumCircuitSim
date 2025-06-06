#include <stdlib.h>
#include "gate.h"

void free_circuit(gate *circuit, int n_gates) {
    if (!circuit) return;
    for (int i = 0; i < n_gates; i++) {
        free(circuit[i].name);
        free(circuit[i].matrix);
    }
    free(circuit);
}