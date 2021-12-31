#define _POSIX_SOURCE
#define _DEFAULT_SOURCE
#ifndef PROGETTO_SO__PIPE_MENU_H
#define PROGETTO_SO__PIPE_MENU_H

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


#define MENU_ELEM 3
#define DEFAULT_MAXX 100
#define DEFAULT_MAXY 30
#define DIM_TITOLO 13


#define NUMERO_OPZIONI 4
#define NUMERO_RISOLUZIONI 5
#define NUMERO_SELEZIONI_NEMICI 12
#define MENUWIN_X 70
#define MENUWIN_Y 20

typedef enum {GIOCA, MENU, OPZIONI, ESCI} StatoCorrente;
typedef enum {R_80_X_24, R_100_X_30, R_100_X_45, R_150_X_45, R_170_X_51} Risoluzioni;


typedef struct {
    int x; /* coordinata x */
    int y; /* coordinata y */
} Pos;
typedef enum{ID_NAVICELLA, ID_NEMICO, ID_MISSILE, ID_BOMBA} IdOggetto;

typedef struct {
    int index;         // Indice dell'oggetto
    IdOggetto id;            // Identificatore dell'entità che invia i dati
    Pos pos;           // Posizione attuale dell'oggetto
    Pos old_pos;       // Posizione precedente dell'oggetto
    pid_t pid;              // Pid del processo proprietario
    int vite;
} Oggetto;

typedef struct{
    int vite;
    _Bool primafila;
} InfoToNemici;


StatoCorrente menu();
extern StatoCorrente gioco();
extern int M;
extern int num_righe;
extern int mov_verticale;
extern void stampaNemico(Oggetto nemico, int vite);
extern void stampaNavicella(Oggetto navicella);
extern void stampaMiss_bomb(Oggetto bomb_miss);
extern int velocita_missili;
StatoCorrente opzioni();

#endif //PROGETTO_SO__PIPE_MENU_H
