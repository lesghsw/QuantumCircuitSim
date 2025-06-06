#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "parser.h"

int parse_real(const char *str, double *out) {
    if (str == NULL || *str == '\0') {
        return EXIT_FAILURE;
    }
    errno = 0;
    char *end_ptr;
    double res = strtod(str, &end_ptr);

    if (end_ptr == str || *end_ptr != '\0') {
        return EXIT_FAILURE;
    }
    if (errno != 0) {
        return EXIT_FAILURE;
    };

    *out = res;
    return EXIT_SUCCESS;
}

int parse_imag(const char *str, complex *out) {
    if (str == NULL || *str == '\0') {
        return EXIT_FAILURE;
    }

    const char *p_str = str; // Crea un secondo puntatore cosi' che potra' essere incrementato

    // Salva il segno e salta al carattere successivo se il segno e' esplicito
    double sign = 1.0;
    if (*p_str == '+')
        p_str++;
    else if (*p_str == '-') {
        sign = -1.0;
        p_str++;
    }

    // Controlla se dopo il segno e' presente "i" (Se non lo e' ritorna 1)
    if (*p_str != 'i') {
        return EXIT_FAILURE;
    }
    p_str++;

    // Solo "i", ritorna subito con risultato valido.
    if (*p_str == '\0') {
        out->re = 0.0;
        out->im = 1.0*sign;
        return EXIT_SUCCESS;
    }

    double parsed_coeff;
    if(parse_real(p_str, &parsed_coeff)) return EXIT_FAILURE;
    out->re = 0.0;
    out->im = parsed_coeff*sign;

    return EXIT_SUCCESS;
}

int parse_complex(const char *str, complex *out) {
    if (str == NULL || *str == '\0') {
        return EXIT_FAILURE;
    }

    // Se non c'e' una 'i' e' un numero reale
    const char *p_i = strchr(str, 'i');
    if (p_i == NULL) {
        if (parse_real(str, &out->re)) {
            return EXIT_FAILURE;
        }  
        else {
            out->im = 0.0;
            return EXIT_SUCCESS;
        }
    }

    int len = (int)strlen(str);

    // Valore base di zero, poiche' non e' un valore valido
    // ci permette di verificare il successo della ricerca
    int split_index = 0;

    for (int i = 1; i < len; i++) {
        // Controllo se il segno e' preceduto da una 'e', così da evitare falsi positivi
        // causati dalla notazione scientifica.
        if ((str[i] == '+' || str[i] == '-') && !(str[i-1] == 'e' || str[i-1] == 'E')) {
            split_index = i;
        }
    }

    // Non e' stato trovato l'indice del separatore, e' un numero immaginario
    if (split_index == 0)
        return parse_imag(str, out);
    // Trovato l'indice del separatore, quindi si procede a separare la stringa in re e im.
    else {
        // Separazione parte reale
        int re_len = split_index;
        char *re_str = malloc(re_len + 1);
        if (!re_str) {
            return EXIT_FAILURE;
        }

        strncpy(re_str, str, re_len);
        re_str[re_len] = '\0';
        
        // Parte reale al parser
        double temp_re;
        if (parse_real(re_str, &temp_re)) {
            free(re_str);
            return EXIT_FAILURE;
        }
        free(re_str);

        // Separazione parte immaginaria
        int im_len = len - split_index;
        char *im_str = malloc(im_len + 1);

        strncpy(im_str, str + split_index, im_len);
        im_str[im_len] = '\0';

        // Parte immaginaria al parser
        if (parse_imag(im_str, out)) {
            free(im_str);
            return EXIT_FAILURE;
        }
        free(im_str);

        out->re = temp_re;
    }

    return EXIT_SUCCESS;
}