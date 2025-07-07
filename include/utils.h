#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include "complex.h"

/// @brief Funzione per rimuovere spazi iniziali e finali da una stringa
/// @param str stringa da modificare in-place
void trim_whitespace(char *str);

/// @brief Funzione che unisce due stringhe
/// @param dest Stringa di partenza
/// @param src  Stringa da aggiungere alla fine della stringa di partenza
/// @return Puntatore al nuovo indirizzo di memoria in cui è contenuta la stringa (malloc usato, caller must free)
char *string_append(char *dest, const char *src);

/// @brief Funzione che legge una riga con dimensione variabile
/// @param fp File da cui leggere la riga
/// @return Puntatore al buffer in cui verrà salvata la riga di testo (NULL in caso di errore) (realloc usato, caller must free)
char *read_line(FILE *fp);

/// @brief Funzione per eseguire l'operazione "matrice X vettore"
/// @param M Array di complessi (verra' trattato come una matrice)
/// @param in_vec Array di complessi (vettore colonna)
/// @param out_vec Puntatore all'array dove verrà salvato il risultato
/// @param dim Numero di righe == Numero di colonne della matrice
void matvec_mul(const complex *M, const complex *in_vec, complex *out_vec, int dim);

/// @brief Funzione per calcolare il valore assoluto di un float
/// @param val Numero di cui verra' calcolato il valore assoluto
/// @return Float contenente il valore assoluto
float float_abs(float val);

/// @brief Stampa a schermo un numero complesso
/// @param c Numero da stampare
/// @param endchar Carattere con cui terminare la stampa ('\0' di default)
void complex_print(complex c, char endchar);

/// @brief Stampa a schermo un vettore di numeri complessi ("[c0, c1, c2, c3 , ... , c(dim-1)]")
void complex_vec_print(complex *vec, size_t dim);

#endif