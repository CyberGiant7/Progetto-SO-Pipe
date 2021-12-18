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
#define PASSO 1 /* Entita spostamento del vespa */
#define maxx 80 /* Numero di colonne dello schermo */
#define maxy 24 /* Numero di righe dello schermo */
/* Struttura per la comunicazione tra figli e padre */
typedef struct {
    char c; /* soggetto che invia il dato: vespa o nave_player */
    int x; /* coordinata x */
    int y; /* coordinata y */
} Pos;

_Noreturn void vespa(int pipeout);
_Noreturn void nave_player(int pipeout);
void AreaGioco(int pipein);
Pos missili(Pos pos_navicella);
//int maxx, maxy;

char *nave[DIM_NAVICELLA]= {" ▟█▛▀▀",
                            "▟██▙",
                            "▗▟█▒▙▖",
                            "▝▜█▒▛▘",
                            "▜██▛",
                            " ▜█▙▄▄"
};

char *nemico_lv1[DIM_NEMICO]={"▀█▙"
                              "█▒ "
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

    while(TRUE) {
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

StatoCorrente gioco(){
    int filedes[2];
    int pid_vespa;
    int pid_navicella;
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

    //getmaxyx(stdscr, maxy, maxx);
    pid_vespa = fork(); //generazione processo figlio per il vespa
    switch (pid_vespa) {
        case -1:
            perror("Errore nell'esecuzione della fork.");
            exit(1);
        case 0:
            prctl(PR_SET_NAME, (unsigned long) "Vespa");
            close(filedes[0]); /* chiusura del descrittore di lettura (standard input)*/
            vespa(filedes[1]); /* il primo processo figlio invoca la funzione nave_player passandogli la pipe in scrittura*/
        default: //processo padre
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
                    prctl(PR_SET_NAME, (unsigned long) "Area_gioco");
                    close(filedes[1]); /* chiusura del descrittore di scrittura (standard output)*/
                    AreaGioco(filedes[0]); /* il processo padre invoca la funzione di AreaGioco */
            }
    }
    /* siamo usciti dalla funzione di AreaGioco e vengono terminati i 2 processi figli e ripristinato il normale modo operativo dello schermo */
    kill(pid_vespa, 1);
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
    return MENU;
}

_Noreturn void vespa(int pipeout) {
    Pos pos_vespa;
    long int r;
    int dx,dy;
    pos_vespa.x=1;
    pos_vespa.y=1;
    pos_vespa.c='^';
    write(pipeout, &pos_vespa, sizeof(pos_vespa));
    while(TRUE) {
        r=random();// impostazione dello spostamento x
        if(r%2)
            dx=PASSO;
        else
            dx=-PASSO;
        if(pos_vespa.x + dx < 1 || pos_vespa.x + dx > maxx)
            dx=-dx;
        pos_vespa.x+=dx;
        r=random();// impostazione dello spostamento y
        if(r%2)
            dy = PASSO;
        else
            dy = -PASSO;
        refresh();
        if(pos_vespa.y + dy < 1 || pos_vespa.y + dy > getmaxy(stdscr)-1)
            dy = -dy;
        pos_vespa.y += dy;
        write(pipeout, &pos_vespa, sizeof(pos_vespa));
        usleep(1000000);
    }
}

void nave_player(int pipeout) {
    int filedes[2];
    int pid_missile1;
    int pid_missile2;
    Pos pos_missile1, pos_missile2, temp_missile;
    Pos pos_navicella;

    pos_navicella.c = '#';
    pos_navicella.x = 1;
    pos_navicella.y = maxy / 2;

    pos_missile1.c = '1';
    pos_missile2.c = '2';


    write(pipeout, &pos_navicella, sizeof(pos_navicella));

    signal(SIGCHLD, SIG_IGN);

    if (pipe(filedes) == -1) { //inizializzazione della pipe con AreaGioco degli errori
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (fcntl(filedes[0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);

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
                        close(filedes[0]); /* chiusura del descrittore di lettura (standard input)*/
                        /* funzione che genera coordinate missile 1*/
                        while (TRUE) {
                            pos_missile1.y -= 1;
                            pos_missile1.x += 4;
                            write(filedes[1], &pos_missile1, sizeof(pos_missile1));
                            if (pos_missile1.x > maxx || pos_missile1.y <= 0)
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
                                close(filedes[0]); /* chiusura del descrittore di lettura (standard input)*/
                                while (TRUE) {
                                    pos_missile2.y += 1;
                                    pos_missile2.x += 4;
                                    write(filedes[1], &pos_missile2, sizeof(pos_missile2));
                                    if (pos_missile2.x > maxx || pos_missile2.y >= maxy)
                                        exit(1);
                                    usleep(1000000);
                                }
                                break;
                            default:
                                close(filedes[1]);
                                break;
                        }
                }
                break;
        }
        read(filedes[0], &temp_missile, sizeof(temp_missile)); /* leggo dalla pipe la posizione di missile 1/2*/
        write(pipeout, &temp_missile, sizeof(temp_missile));
        write(pipeout, &pos_navicella, sizeof(pos_navicella));
    }
}

Pos missili(Pos pos_navicella) {
    int filedes[2];
    int pid_missile1;
    int pid_missile2;
    Pos pos_missile1, pos_missile2, miss_temp;

    pos_missile1.c = '1';
    pos_missile1.y = pos_navicella.y + DIM_NAVICELLA/2;
    pos_missile1.x = pos_navicella.x + DIM_NAVICELLA;

    pos_missile2.c = '2';
    pos_missile2.y = pos_navicella.y + (DIM_NAVICELLA/2)+1;
    pos_missile2.x = pos_navicella.x + DIM_NAVICELLA;

    signal(SIGCHLD, SIG_IGN);

    if (pipe(filedes) == -1) { //inizializzazione della pipe con AreaGioco degli errori
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (fcntl(filedes[0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);

    pid_missile1 = fork(); //generazione processo
    switch (pid_missile1) {
        case -1:
            perror("Errore nell'esecuzione della fork.");
            exit(1);
        case 0:
            close(filedes[0]); /* chiusura del descrittore di lettura (standard input)*/
            /* funzione che genera coordinate missile 1*/
            while(TRUE){
                pos_missile1.y -= 1;
                pos_missile1.x += 4;
                /*if (pos_missile1.x > maxx || pos_missile1.y <= 0)
                    kill(pid_missile1, 1);*/
                write(filedes[1], &pos_missile1, sizeof(pos_missile1));
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
                    close(filedes[0]); /* chiusura del descrittore di lettura (standard input)*/
                    while(TRUE){
                        pos_missile2.y += 1;
                        pos_missile2.x -= 4;
                        /*if (pos_missile2.x > maxx || pos_missile2.y >= maxy)
                            kill(pid_missile2, 1);*/
                        write(filedes[1], &pos_missile2, sizeof(pos_missile2));
                        usleep(1000000);
                    }
                    break;
                default:
                    close(filedes[1]);
                    while(TRUE) {
                        read(filedes[0], &miss_temp,
                             sizeof(miss_temp)); /* leggo dalla pipe la posizione di missile 1/2*/
                        return miss_temp; //restituisco alla funzione nave_player
                    }
                    break;
            }
    }
}

void AreaGioco (int pipein) {
    int vite = 3;
    int i;
    _Bool collision = false;
    Pos vespa, navicella, missile1, missile2, valore_letto;
    vespa.x = -1;
    navicella.x = -1;

    while(!collision){
        //getmaxyx(stdscr, maxy, maxx);
        move(0,1);
        clrtoeol();
        printw("VITE: %d, MAXY = %d, MAXX = %d, vespa.y = %d", vite, getmaxy(stdscr), maxx,vespa.y);
        read(pipein, &valore_letto, sizeof(valore_letto)); /* leggo dalla pipe */
        if (valore_letto.c == '^') {
            if (vespa.x >= 0) { /* cancello la 'vecchia' posizione del vespa */
                mvaddch(vespa.y, vespa.x, ' ');
            }
            mvprintw(valore_letto.y,valore_letto.x,"^");
            vespa = valore_letto;
        } else if (valore_letto.c == '#'){
            if (navicella.x >= 0) { /* cancello la 'vecchia' posizione della navicella */
                attron(COLOR_PAIR(0));
                for (i = 0; i < DIM_NAVICELLA; i++) {
                    mvprintw(navicella.y + i, navicella.x, "      ");
                }
                attron(COLOR_PAIR(1));
                for (i = 0; i < DIM_NAVICELLA; i++) {
                    mvprintw(valore_letto.y+i, valore_letto.x, nave[i]);
                }
            }
            navicella = valore_letto;
        } else if (valore_letto.c == '1'){
            attron(COLOR_PAIR(0));
            mvprintw(missile1.y, missile1.x , " ");
            attron(COLOR_PAIR(1));
            mvprintw(valore_letto.y, valore_letto.x , ">");
            missile1 = valore_letto;
        } else if (valore_letto.c == '2'){
            attron(COLOR_PAIR(0));
            mvprintw(missile2.y, missile2.x , " ");
            attron(COLOR_PAIR(1));
            mvprintw(valore_letto.y, valore_letto.x , "<");
            missile2 = valore_letto;
        }
        /* visualizzo l'oggetto nella posizione aggiornata */
        if(vespa.x == navicella.x && vespa.y == navicella.y) {
            vite-=3;
            if(navicella.x < maxx) navicella.x += 1; else navicella.x -= 1;
            if(navicella.y < maxy) navicella.y += 1; else navicella.y -= 1;
        }
        if(valore_letto.c=='#' && valore_letto.y != 0 || valore_letto.x != 1){

        }
        curs_set(0);
        refresh();
        if (vite == 0)
            collision = true;
    }
}