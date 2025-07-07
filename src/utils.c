#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

void trim_whitespace(char *str) {
    if (str == NULL) {
        return;
    }

    // Salta spazi iniziali
    char *start = str;
    while (isspace((unsigned char)*start)) {
        start++;
    }

    // Sposta i caratteri nella stringa originale
    if (start != str) {
        char *p = str;
        while ((*p++ = *start++)) ;
    }

    // Se stringa vuota (tutti spazi), ritorna
    if (*str == '\0') {
        return;
    }

    // Rimuovi spazi finali
    char *end = str;
    while (*end) end++; // Trova la fine della stringa
    end--;

    while (end > str && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0'; // Nuova fine stringa.
}

char *string_append(char *dest, const char *src) {
    if (!src) return dest;
    if (!dest) { // Creo destinazione in caso fosse NULL in input
        dest = malloc(strlen(src) + 1);
        if (dest) strcpy(dest, src);
        return dest;
    }
    size_t old_len = strlen(dest);
    size_t src_len = strlen(src);
    char *new_dest = realloc(dest, old_len + src_len + 1); // Crea spazio per ospitare la nuova stringa
    if (!new_dest) {
        free(dest);
        return NULL;
    }
    strcpy(new_dest + old_len, src); // Aggiungo la seconda parte della stringa
    return new_dest;
}

char *read_line(FILE *fp) {
    if (!fp) return NULL;
    char *buffer = NULL;
    size_t buffer_size = 0;
    size_t length = 0;
    
    while (1) { // Prosegue fino a EOF, newline, o errore
        int c = fgetc(fp);
        if (c == EOF) {
            if (length == 0) return NULL;
            break;
        }
        
        // Aggiorna la dimensione del buffer
        if (length + 2 >= buffer_size) { // Spazio per il prossimo carattere e anche per il null terminator
            size_t new_size = buffer_size ? buffer_size * 2 : 128; // Raddoppia dimensione ogni volta (Ottima prassi per evitare chiamate frequenti)
            char *new_buffer = realloc(buffer, new_size);
            if (!new_buffer) {
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
            buffer_size = new_size;
        }
        
        buffer[length++] = c;
        if (c == '\n') break;
    }

    buffer[length] = '\0'; // Null terminator sicuro
    return buffer;
}


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

float float_abs(float val) {
    return val >= 0.0 ? val : -val;
}

void complex_print(complex c, char endchar) {
    if (!endchar) endchar = '\0';
    char sep = c.im >= 0.0 ? '+' : '-'; 
    printf("%.5f%ci%.5f%c", c.re, sep, float_abs(c.im), endchar);
}

void complex_vec_print(complex *vec, size_t dim) {
    printf("[");
    for (size_t i = 0; i < dim; i++) {
        complex_print(vec[i], 0x0);
        if (i != dim-1) printf(", ");
    }
    printf("]\n");
}