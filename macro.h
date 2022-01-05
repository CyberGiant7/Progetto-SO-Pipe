/**
 * In questo file vengono dichiarate tutte variabili globali, strutture, enumerazioni e macro utilizzate in tutti i
 * files.
 */

#ifndef PROGETTO_SO__PIPE_MACRO_H
#define PROGETTO_SO__PIPE_MACRO_H
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <locale.h>
#include <fcntl.h>
#include <sys/prctl.h> // momentanea, per dare nomi ai processi


/// Colori
#define BLACK 0
#define WHITE 1
#define GREEN 2
#define YELLOW 3
#define MAGENTA 4
#define CYAN 5
#define BLUE 6
#define RED 7

#define KEY_SPACE 32        // Id tasto space
#define DIM_NAVICELLA 6     // Dimensione navicella (caratteri)
#define DIM_NEMICO 3        // Dimensione nemici    (caratteri)
#define MAX_MISSILI 10      // Numero (pari) massimo di missili sparabili contemporaneamente


/// Struttura per salvare le coordinate
typedef struct {
    int x; /* coordinata x */
    int y; /* coordinata y */
} Pos;

/// Enumerazione per gestire gli ID dei diversi oggetti
typedef enum{ID_NAVICELLA, ID_NEMICO, ID_MISSILE1, ID_MISSILE2, ID_BOMBA} IdOggetto;

/// Struttura per descrivere i vari oggetti del gioco
typedef struct {
    int index;          // Indice dell'oggetto
    IdOggetto id;       // Identificatore dell'entità che invia i dati
    Pos pos;            // Posizione attuale dell'oggetto
    Pos old_pos;        // Posizione precedente dell'oggetto (utile per cancellare)
    pid_t pid;          // Pid del processo proprietario
    int vite;           // Numero di vite dell'oggetto
} Oggetto;

/// Enumerazione per gestire i vari cambi di contesto
typedef enum {GIOCA, OPZIONI, ESCI, MENU} StatoCorrente;

/// Struttura per comunicare tramite la pipe della navicella
typedef struct {
    Oggetto navicella;
    _Bool sparo;
} arg_from_navicella;


/// Dichiarazione di variabili globali
extern int num_righe;       // Numero di nemici che verranno stampati in ogni colonna
extern int mov_verticale;   // Numero di spostamenti che i nemici effettuano prima di sbattere nel bordo

extern int maxx;            // Risoluzione dello schermo (numero di colonne)
extern int maxy;            // Risoluzione dello schermo (numero di righe)
extern const Oggetto init;  // Costante utilizzata per inizializzare le strutture Oggetto comodamente
extern int velocita_missili;// Velocità dei missili (valore inserito in un usleep)
extern int M;               // Numero di nemici generati

#endif //PROGETTO_SO__PIPE_MACRO_H

