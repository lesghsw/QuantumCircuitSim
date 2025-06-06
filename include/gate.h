#ifndef GATE_H
#define GATE_H

#include "complex.h"

/// @brief Struttura dati per contenere i gate
typedef struct {
    char *name;
    complex *matrix; 
} gate;

/// @brief Funzione per liberare i gate di un circuito e il circuito stesso
/// @param circuit Puntatore al circuito (Array di gate) 
/// @param n_gates Numero di gate da liberare
void free_circuit(gate *circuit, int n_gates);

#endif