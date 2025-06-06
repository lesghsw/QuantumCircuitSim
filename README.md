## Progetto di SOII, Università la sapienza.

### Come usare il programma:

Da linea di comando eseguire il programma,
come primo parametro inserire il percorso (relativo al programma)
del file di init, e come secondo quello del file circuito.

I file init hanno la seguente formattazione:

```
#qubits 1

#init [0.5+i0.5, 0.5-i0.5]
```

Mentre i file circuito hanno la seguente formattazione:

```
#define X [(0, 1) (1, 0)]

#define I [(1, 0) (0, 1)]

#define Y [(0, -i) (i, 0)]

...

#circ Y X I ...
```

**In entrambi i casi non importa l'ordine delle direttive.**

**Nei file verranno ignorate tutte le righe che non iniziano con una direttiva.**

**In caso di errore verrà stampato a schermo la posizione nel file**
**(se è un errore nei file input) e la possibile causa.**

Invece in caso di successo verrà stampato a schermo il risultato, ovvero
lo stato finale del circuito.

-----------------

## Regole di formattazione

### Ovunque si possa inserire un numero questo può essere reale, immaginario o complesso.

### I numeri complessi e immaginari seguono la stessa formattazione per la parte immaginaria,ovvero 'i' come prefisso del coeffieciente

### I coefficienti possono essere scritti in notazione scientifica

**In linea generale i numeri sono scritti nella forma (a+ib)**

esempi di numeri validi:
```
i2
-i
i
12
1.23e2
-1+i2
-1.02e1-i3.2e2
```

-----------------

## makefile

### make
Con make viene compilato il programma, pronto per essere usato
### make clean
Make clean rimuove il file generato