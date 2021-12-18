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
#include "menu.h"

#define KEY_SPACE 32
#define DIM_NAVICELLA 6
#define DIM_NEMICO 3
#define maxx 80 /* Numero di colonne dello schermo */
#define maxy 24 /* Numero di righe dello schermo */
#define NAVE 0
#define MISSILE_1 1
#define MISSILE_2 2

/* Struttura per la comunicazione tra figli e padre */
typedef struct {
    int n; /* soggetto che invia il dato: vespa o nave_player */
    int x; /* coordinata x */
    int y; /* coordinata y */
} Pos;

_Noreturn void vespa(int pipeout);
_Noreturn void nave_player(int pipeout);
void AreaGioco(int pipein);

int M;
//int maxx, maxy;

char *nave[DIM_NAVICELLA]= {" ▟█▛▀▀",
                            "▟██▙  ",
                            "▗▟█▒▙▖",
                            "▝▜█▒▛▘",
                            "▜██▛  ",
                            " ▜█▙▄▄"
};

char *nemico_lv1[DIM_NEMICO]={"▀█▙",
                              "▒█ ",
                              "▄█▛"
};

char *nemico_lv2[DIM_NEMICO]={" △ "
                              "◁ ◊"
                              " ▽ "
};
/*
▀
▄
▗
▖
▘
▝
▙
▚
▛
▜
▞
▟
█
  	◀
*/


int main() {
    StatoCorrente status;
    status = MENU;
    M = 10;

    while (TRUE) {
        switch (status) {
            case GIOCA:
                status = gioco();
                break;
            case MENU:
                status = menu();
                break;
            case ESCI:
                return 0;
                break;
        }
        clear();
        refresh();
    }
}

StatoCorrente gioco() {
    int filedes[2];
    int pid_navicella;
    int i;
    pid_t pid_nemici = 1;
    Pos *array_pos_nemici = NULL;
    signal(SIGCHLD, SIG_IGN);

    setlocale(LC_ALL, "");
    initscr(); /* inizializzazione dello schermo */
    noecho(); /* i caratteri corrispondenti ai tasti premuti non saranno visualizzati sullo schermo del terminale */
    srand(time(NULL)); /* inizializziamo il generatore di numeri random */
    curs_set(0); /* nasconde il cursore */

    start_color();
    init_pair(0, COLOR_BLACK, COLOR_BLACK); // per cancellare
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // per scrivere

    bkgd(COLOR_PAIR(1));

    if (pipe(filedes) == -1) { //inizializzazione della pipe con AreaGioco degli errori
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }

    pid_navicella = fork(); //generazione di un secondo processo figlio per la nave_player
    switch (pid_navicella) {
        case -1:
            perror("Errore nell'esecuzione della fork.");
            _exit(1);
        case 0:
            prctl(PR_SET_NAME, (unsigned long) "Navicella");
            mvprintw(maxy / 2, maxx / 2, "%s", nave);
            close(filedes[0]); /* chiusura del descrittore di lettura (standard input)*/
            nave_player(
                    filedes[1]); /* il secondo processo figlio invoca la funzione nave_player passandogli la pipe in scrittura*/
        default:    //processo padre
            array_pos_nemici = (Pos *) malloc(sizeof(Pos) * M);
            int temp;
            //for (i = 0; i < M; i++) {ù
            i = 0;
            pid_nemici = fork();
            if (pid_nemici == -1) {
                perror("Errore nell'esecuzione della fork.");
                _exit(1);
            } else if (pid_nemici == 0) {
                char nome[10];
                sprintf(nome, "Nemico_%d", i);
                prctl(PR_SET_NAME, (unsigned long) nome);
                array_pos_nemici[i].y = 1 + (i % 5) * (DIM_NEMICO + 1);
                temp = (int) (i / 5) + 1;
                array_pos_nemici[i].x = maxx - temp * DIM_NEMICO;
                array_pos_nemici[i].n = 11 + i;
                write(filedes[1], &array_pos_nemici[i], sizeof(array_pos_nemici[i]));
                exit(0);
            }
            if (pid_nemici != 0) {
                close(filedes[1]); /* chiusura del descrittore di scrittura (standard output)*/
                AreaGioco(filedes[0]); /* il processo padre invoca la funzione di AreaGioco */
                // }
            }
    }
    /* siamo usciti dalla funzione di AreaGioco e vengono terminati i 2 processi figli e ripristinato il normale modo operativo dello schermo */
    kill(pid_navicella, 1);
    clear();
    attron(COLOR_PAIR(1));
    printw("   _____              __  __   ______    ____   __      __  ______   _____    \n"
           "  / ____|     /\\     |  \\/  | |  ____|  / __ \\  \\ \\    / / |  ____| |  __ \\   \n"
           " | |  __     /  \\    | \\  / | | |__    | |  | |  \\ \\  / /  | |__    | |__) |  \n"
           " | | |_ |   / /\\ \\   | |\\/| | |  __|   | |  | |   \\ \\/ /   |  __|   |  _  /   \n"
           " | |__| |  / ____ \\  | |  | | | |____  | |__| |    \\  /    | |____  | | \\ \\   \n"
           "  \\_____| /_/    \\_\\ |_|  |_| |______|  \\____/      \\/     |______| |_|  \\_\\");
    refresh();
    sleep(500);
    endwin();
    free(array_pos_nemici);
    return MENU;
}



void nave_player(int pipeout) {
    int pid_missile1;
    int pid_missile2;
    Pos pos_missile1, pos_missile2, temp_missile;
    Pos pos_navicella;

    pos_navicella.n = NAVE;
    pos_navicella.x = 1;
    pos_navicella.y = maxy / 2;

    pos_missile1.n = MISSILE_1;
    pos_missile2.n = MISSILE_2;


    write(pipeout, &pos_navicella, sizeof(pos_navicella));

    signal(SIGCHLD, SIG_IGN);

    keypad(stdscr, TRUE);
    while (1) {
        nodelay(stdscr, 1);
        timeout(500);
        int c = getch();
        switch (c) {
            case KEY_UP:
                if (pos_navicella.y > 1)
                    pos_navicella.y -= 1;
                write(pipeout, &pos_navicella, sizeof(pos_navicella));
                break;
            case KEY_DOWN:
                if (pos_navicella.y < maxy - DIM_NAVICELLA)
                    pos_navicella.y += 1;
                write(pipeout, &pos_navicella, sizeof(pos_navicella));
                break;
            case KEY_SPACE:
                pos_missile1.y = pos_navicella.y + (DIM_NAVICELLA / 2);
                pos_missile1.x = pos_navicella.x + DIM_NAVICELLA;
                pos_missile2.y = pos_navicella.y + (DIM_NAVICELLA / 2);
                pos_missile2.x = pos_navicella.x + DIM_NAVICELLA;
                pid_missile1 = fork(); //generazione processo
                switch (pid_missile1) {
                    case -1:
                        perror("Errore nell'esecuzione della fork.");
                        exit(1);
                    case 0:
                        prctl(PR_SET_NAME, (unsigned long) "Missile 1");
                        /* funzione che genera coordinate missile 1*/
                        while (TRUE) {
                            pos_missile1.y -= 1;
                            pos_missile1.x += 4;
                            write(pipeout, &pos_missile1, sizeof(pos_missile1));
                            if (pos_missile1.x > maxx || pos_missile1.y <= 1)
                                exit(1);
                            usleep(1000000);
                        }
                        break;
                    default: //processo padre
                        pid_missile2 = fork(); //generazione di un secondo processo figlio per la nave_player
                        switch (pid_missile2) {
                            case -1:
                                perror("Errore nell'esecuzione della fork.");
                                exit(1);
                            case 0:
                                prctl(PR_SET_NAME, (unsigned long) "Missile 2");
                                while (TRUE) {
                                    pos_missile2.y += 1;
                                    pos_missile2.x += 4;
                                    write(pipeout, &pos_missile2, sizeof(pos_missile2));
                                    if (pos_missile2.x > maxx || pos_missile2.y >= maxy)
                                        exit(1);
                                    usleep(1000000);
                                }
                                break;
                            default:
                                break;
                        }
                }
                break;
        }
    }
}


void AreaGioco(int pipein) {
    refresh();
    int vite = 1000;
    int i, j;
    _Bool collision = false;
    Pos navicella, missile1, missile2, valore_letto;
    navicella.x = -1;

    while (true) {
        move(0, 1);
        clrtoeol();
        printw("VITE: %d", vite);
        read(pipein, &valore_letto, sizeof(valore_letto)); /* leggo dalla pipe */
        if (valore_letto.n == NAVE) {
            if (navicella.x >= 0) { /* cancello la 'vecchia' posizione della navicella */
                attron(COLOR_PAIR(0));
                for (i = 0; i < DIM_NAVICELLA; i++) {
                    mvprintw(navicella.y + i, navicella.x, "      ");
                }
                attron(COLOR_PAIR(1));
                for (i = 0; i < DIM_NAVICELLA; i++) {
                    mvprintw(valore_letto.y + i, valore_letto.x, nave[i]);
                }
            }
            navicella = valore_letto;
        } else if (valore_letto.n == MISSILE_1) {
            attron(COLOR_PAIR(0));
            mvprintw(missile1.y, missile1.x, " ");
            attron(COLOR_PAIR(1));
            mvprintw(valore_letto.y, valore_letto.x, ">");
            missile1 = valore_letto;
        } else if (valore_letto.n == MISSILE_2) {
            attron(COLOR_PAIR(0));
            mvprintw(missile2.y, missile2.x, " ");
            attron(COLOR_PAIR(1));
            mvprintw(valore_letto.y, valore_letto.x, "<");
            missile2 = valore_letto;
        }
        if (valore_letto.n == 11){
            for (j = 0; j < DIM_NEMICO; j++) {
                mvprintw(valore_letto.y, valore_letto.x, nemico_lv1[j]);
            }
        }

        /*for(i=0; i < M; i++) {
            if (valore_letto.n >= 11 && valore_letto.n <= 11 + M) {
                attron(COLOR_PAIR(1));
                for (j = 0; j < DIM_NEMICO; j++) {
                    mvprintw(valore_letto.y, valore_letto.x, nemico_lv1[j]);
                }
            }
        }*/
        /* visualizzo l'oggetto nella posizione aggiornata */
        //if (vespa.x == navicella.x + DIM_NAVICELLA - 1 && vespa.y == navicella.y + i)
        /*for (i = 0; i < DIM_NAVICELLA; i++) {
            if ((vespa.x == navicella.x + i && vespa.y == navicella.y)
                || (vespa.x == navicella.x + i && vespa.y == navicella.y + DIM_NAVICELLA - 1)
                || ) {
                vite -= 3;
                if (navicella.x < maxx) navicella.x += 1; else navicella.x -= 1;
                if (navicella.y < maxy) navicella.y += 1; else navicella.y -= 1;
            }
            if (valore_letto.c == '#' && valore_letto.y != 0 || valore_letto.x != 1) {

            }*/

        curs_set(0);
        refresh();
        if (vite == 0)
            collision = true;
    }
}