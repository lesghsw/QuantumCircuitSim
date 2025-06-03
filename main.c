#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>

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

/*
Stampa a schermo un numero complesso
evita di scrivere segni e/o coefficienti se non necessario

es: +i1 -> i
*/
void complex_print(complex c) {
    if (c.re == 0.0 && c.im == 0.0) {
        printf("0\n");
    }
    if (c.re != 0.0 && c.im == 0.0) {
        printf("%.10g\n", c.re);
    }
    if (c.re == 0.0 && c.im != 0.0) {
        if (c.im >= 0.0)
            printf("i");
        else
            printf("-i");
        
        if (fabs(c.im) == 1.0)
            printf("\n");
        else
            printf("%.10g\n", fabs(c.im));
    }
    if (c.re != 0.0 && c.im != 0.0) {
        char sep = c.im >= 0.0 ? '+' : '-'; 
        if (fabs(c.im) == 1.0)
            printf("%.10g%ci\n", c.re, sep);
        else
            printf("%.10g%ci%.10g\n", c.re, sep, fabs(c.im));
    }
    fflush(stdout);
}

typedef struct {
    char *name;
    complex *matrix; 
} gate;

// Funzione per eseguire l'operazione "matrice X vettore"
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

int parse_real(const char *str, double *out) {
    if (str == NULL || *str == '\0') return 1;
    errno = 0;
    char *end_ptr;
    double res = strtod(str, &end_ptr);

    if (end_ptr == str) return 1;
    if (errno != 0) return 1;

    *out = res;
    return 0;
}

int parse_imag(const char *str, complex *out) {
    if (str == NULL || *str == '\0') return 1;

    const char *p_str = str; // Crea un secondo puntatore cosi' che potra' essere incrementato
    out->re = 0.0; // La parte reale è sempre 0

    // Salva il segno e salta al carattere successivo se il segno è esplicito
    double sign = 1.0;
    if (*p_str == '+')
        p_str++;
    else if (*p_str == '-') {
        sign = -1.0;
        p_str++;
    }

    // Controlla se dopo il segno e' presente "i" (Se non lo e' ritorna 0)
    if (*p_str != 'i') return 1;
    p_str++;

    // Solo "i", ritorna subito con risultato valido.
    if (*p_str == '\0') {
        out->im = 1.0*sign;
        return 0;
    }

    double parsed_coeff;
    if(parse_real(p_str, &parsed_coeff)) return 1;
    out->im = parsed_coeff*sign;

    return 0;
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
    return 0;
}

int main(void) {

    return 0;
}