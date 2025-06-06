#ifndef PARSER_H
#define PARSER_H

#include "complex.h"

/// @brief Parser di numeri reali
/// @param str Stringa da cui prendere il numero
/// @param out Puntatore al double in cui salvare il numero
/// @return EXIT_FAILURE o EXIT_SUCCESS
int parse_real(const char *str, double *out);

/// @brief Parser di numeri immaginari, il numero immaginario in input deve essere del formato ib (b può essere 0).
/// @param str Stringa da cui prendere il numero
/// @param out Puntatore al complex in cui salvare il numero
/// @return EXIT_FAILURE o EXIT_SUCCESS
int parse_imag(const char *str, complex *out);

/// @brief Parser di numeri immaginari, il numero complesso in input deve essere del formato a+ib (a e b possono essere 0).
/// @param str Stringa da cui prendere il numero
/// @param out Puntatore al complex in cui salvare il numero
/// @return EXIT_FAILURE o EXIT_SUCCESS
int parse_complex(const char *str, complex *out);

#endif