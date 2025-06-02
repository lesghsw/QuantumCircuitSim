#include <stdlib.h>
#include <stdio.h>

typedef struct {
    double re;
    double im;
} complex;

complex complex_add(complex c1, complex c2) {
    complex c;
    c.re = c1.re + c2.re;
    c.im = c1.im + c2.im;
    return c;
}

complex complex_mul(complex c1, complex c2) {
    complex c;
    c.re = c1.re * c2.re - c1.im * c2.im;
    c.im = c1.re * c2.im + c1.im * c2.re;
    return c;
}

void complex_print(complex c) {
    printf("(%.10g,%.10g)", c.re, c.im);
}

typedef struct {
    char *name;
    complex *matrix; 
} gate;

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

void free_gates(gate *gates, int n_gates) {
    if (!gates) return;
    for (int i = 0; i < n_gates; i++) {
        free(gates[i].name);
        free(gates[i].matrix);
    }
    free(gates);
}

complex parse_complex(const char *str) {

}

// TODO: Do after completing complex parser
int read_qbits_init(const char *filename, int *n_qubits, complex **init_state) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Errore: impossibile aprire il file %s\n", filename);
        return -1;
    }
}

int main(void) {

    return 0;
}