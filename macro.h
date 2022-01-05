#ifndef PROGETTO_SO__PIPE_MACRO_H
#define PROGETTO_SO__PIPE_MACRO_H
#define _DEFAULT_SOURCE
/**
 * In questo file vengono dichiarate tutte variabili globali, strutture, enumerazioni e macro utilizzate in tutti i
 * files.
 */

#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
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


#define MENU_ELEM 3
#define DEFAULT_MAXX 100
#define DEFAULT_MAXY 30
#define DIM_TITOLO 13


#define NUMERO_OPZIONI 4
#define NUMERO_RISOLUZIONI 5
#define NUMERO_SELEZIONI_NEMICI 12
#define MENUWIN_X 70
#define MENUWIN_Y 20

/// Struttura per salvare le coordinate
typedef struct {
    int x; /* coordinata x */
    int y; /* coordinata y */
} Pos;

/// Enumerazione per gestire gli ID dei diversi oggetti
typedef enum{ID_NAVICELLA, ID_NEMICO, ID_MISSILE1, ID_MISSILE2, ID_BOMBA} IdOggetto;

typedef struct {
    int index;         // Indice dell'oggetto
    IdOggetto id;            // Identificatore dell'entità che invia i dati
    Pos pos;           // Posizione attuale dell'oggetto
    Pos old_pos;       // Posizione precedente dell'oggetto
    pid_t pid;              // Pid del processo proprietario
    int vite;
} Oggetto;

typedef struct {
    Oggetto navicella;
    _Bool sparo;
} arg_from_navicella;

typedef struct{
    int vite;
    _Bool primafila;
} InfoToNemici;


/// Enumerazione per gestire i vari cambi di contesto
typedef enum {GIOCA, OPZIONI, ESCI, MENU} StatoCorrente;

/// Dichiarazione di variabili globali
extern int num_righe;       // Numero di nemici che verranno stampati in ogni colonna
extern int mov_verticale;   // Numero di spostamenti che i nemici effettuano prima di sbattere nel bordo

extern int maxx;            // Risoluzione dello schermo (numero di colonne)
extern int maxy;            // Risoluzione dello schermo (numero di righe)
extern const Oggetto init;  // Costante utilizzata per inizializzare le strutture Oggetto comodamente
extern int velocita_missili;// Velocità dei missili (valore inserito in un usleep)
extern int M;               // Numero di nemici generati

StatoCorrente menu();
extern StatoCorrente gioco();
StatoCorrente opzioni();


#endif //PROGETTO_SO__PIPE_MACRO_H

