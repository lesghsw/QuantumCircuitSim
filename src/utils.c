#include <stdio.h>
#include "utils.h"

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

    if (c.re == 0.0 && c.im == 0.0) {
        printf("0%c", endchar);
    }
    if (c.re != 0.0 && c.im == 0.0) {
        printf("%.10g%c", c.re, endchar);
    }
    if (c.re == 0.0 && c.im != 0.0) {
        if (c.im >= 0.0)
            printf("i");
        else
            printf("-i");
        
        // Evita di scrivere il coefficiente se non è necessario
        if (float_abs(c.im) == 1.0)
            printf("%c", endchar);
        else
            printf("%.10g%c", float_abs(c.im), endchar);
    }
    if (c.re != 0.0 && c.im != 0.0) {
        char sep = c.im >= 0.0 ? '+' : '-'; 
        // Evita di scrivere il coefficiente se non è necessario
        if (float_abs(c.im) == 1.0)
            printf("%.10g%ci%c", c.re, sep, endchar);
        else
            printf("%.10g%ci%.10g%c", c.re, sep, float_abs(c.im), endchar);
    }
}

void complex_vec_print(complex *vec, size_t dim) {
    printf("[");
    for (size_t i = 0; i < dim; i++) {
        complex_print(vec[i], 0x0);
        if (i != dim-1) printf(", ");
    }
    printf("]\n");
}