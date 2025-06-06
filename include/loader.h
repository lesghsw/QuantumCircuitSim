#ifndef LOADER_H
#define LOADER_H

#include "complex.h"
#include "gate.h"

/// @brief Carica i dati dei qubit e del vettore da file
/// @param filename Nome del file da cui leggere
/// @param n_qubits Puntatore all'int in cui salvare il numero di qubits
/// @param out_vec  Puntatore all'array di complessi in cui salvare il vettore letto da file
/// @return EXIT_FAILURE o EXIT_SUCCESS
/// malloc utilizzato internamente per "out", "Caller must free"
int load_qubits_init(const char *filename, int *n_qubits, complex **out_vec);

/// @brief Carica i dati del circuito e dei gate da file
/// @param filename Nome del file da cui leggere
/// @param n_gates_out Puntatore all'int in cui salvare il numero di gate caricati
/// @param circuit_out Puntatore al circuito (gate*)
/// @param n_qubits Numero di qubits (Serve per la dimensione delle matrici)
/// @return EXIT_FAILURE o EXIT_SUCCESS
/// malloc utilizzato internamente per "circuit_out", "Caller must free"
/// @see free_circuit()
int load_gates_circ(const char *filename, int *n_gates_out, gate **circuit_out, const int n_qubits);

#endif