## Progetto di SOII, Università la sapienza.

### Descrizione dei file:

La cartella "include" contiene tutti gli header, mentre "src" tutti i file source.

**gate.h** crea una struttura dati per salvare i gate, contiene anche una funzione per liberare 
la memoria allocata nei circuiti (circuit = (gate*))

**complex.h** crea una struttura dati per contenere numeri complessi
e anche delle funzioni che permettono di
aggiungere e moltiplicare i numeri complessi tra loro

**loader.h** contiene le funzioni utili ha leggere e caricare i file in input.

**parser.h** contiene le funzioni utili a convertire una string in numero complesso.

**utils.h** contiene funzioni di utilità generiche del progetto.

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

**In caso di errore all'interno del file di input esso verrà segnalatospecificando posizione e ragione del problema.**

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
-1.02e-1-i3.2e2
```

-----------------

## makefile

### Con make viene compilato il programma, pronto per essere usato
### Make clean rimuove il file generato