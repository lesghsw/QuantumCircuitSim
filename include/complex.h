#ifndef COMPLEX_H
#define COMPLEX_H

/// @brief Definizione di complex (numero complesso, con parte reale e parte immaginaria double)
typedef struct {
    double re;
    double im;
} complex;

/// @brief Funzione che addiziona due numeri complessi
/// @param c1 Numero complesso
/// @param c2 Numero complesso
/// @return Nuovo numero complesso contenente la somma
complex complex_add(complex c1, complex c2);

/// @brief Funzione che moltiplica due numeri complessi
/// @param c1 Numero complesso
/// @param c2 Numero complesso
/// @return Nuovo numero complesso contenente il risultato
complex complex_mul(complex c1, complex c2);

#endif