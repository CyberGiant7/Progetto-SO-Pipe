#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <locale.h>

#define DIM_NAVICELLA 6
#define DIM_NEMICO 3
#define PASSO 1 /* Entita spostamento del vespa */
#define maxx 80 /* Numero di colonne dello schermo */
#define maxy 24 /* Numero di righe dello schermo */
/* Struttura per la comunicazione tra figli e padre */
struct pos {
    char c; /* soggetto che invia il dato: vespa o nave_player */
    int x; /* coordinata x */
    int y; /* coordinata y */
};

_Noreturn void vespa(int pipeout);
_Noreturn void nave_player(int pipeout);
void AreaGioco(int pipein);
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
*/

int main() {
    int filedes[2];
    int pid_vespa;
    int pid_contadino;

    setlocale(LC_ALL, "");
    initscr(); /* inizializzazione dello schermo */
    noecho(); /* i caratteri corrispondenti ai tasti premuti non saranno visualizzati sullo schermo del terminale */
    srand(time(NULL)); /* inizializziamo il generatore di numeri random */
    curs_set(0); /* nasconde il cursore */

    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);  /* Colore vespa */
    init_pair(2, COLOR_WHITE, COLOR_BLACK);   /* Colore nave_player */
    init_pair(3, COLOR_RED, COLOR_BLACK);   /* Colore trappola */
    init_pair(4, COLOR_BLACK, COLOR_BLACK);
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
            close(filedes[0]); /* chiusura del descrittore di lettura (standard input)*/
            vespa(filedes[1]); /* il primo processo figlio invoca la funzione nave_player passandogli la pipe in scrittura*/
        default: //processo padre
            pid_contadino = fork(); //generazione di un secondo processo figlio per la nave_player
            switch (pid_contadino) {
                case -1:
                    perror("Errore nell'esecuzione della fork.");
                    _exit(1);
                case 0:
                    mvprintw(maxy / 2, maxx / 2, "%s", nave);
                    close(filedes[0]); /* chiusura del descrittore di lettura (standard input)*/
                    nave_player(
                            filedes[1]); /* il secondo processo figlio invoca la funzione nave_player passandogli la pipe in scrittura*/
                default:    //processo padre
                    close(filedes[1]); /* chiusura del descrittore di scrittura (standard output)*/
                    AreaGioco(filedes[0]); /* il processo padre invoca la funzione di AreaGioco */
            }
    }
    /* siamo usciti dalla funzione di AreaGioco e vengono terminati i 2 processi figli e ripristinato il normale modo operativo dello schermo */
    kill(pid_vespa, 1);
    kill(pid_contadino, 1);
    clear();
    printw("   _____              __  __   ______    ____   __      __  ______   _____    \n"
           "  / ____|     /\\     |  \\/  | |  ____|  / __ \\  \\ \\    / / |  ____| |  __ \\   \n"
           " | |  __     /  \\    | \\  / | | |__    | |  | |  \\ \\  / /  | |__    | |__) |  \n"
           " | | |_ |   / /\\ \\   | |\\/| | |  __|   | |  | |   \\ \\/ /   |  __|   |  _  /   \n"
           " | |__| |  / ____ \\  | |  | | | |____  | |__| |    \\  /    | |____  | | \\ \\   \n"
           "  \\_____| /_/    \\_\\ |_|  |_| |______|  \\____/      \\/     |______| |_|  \\_\\");
    refresh();
    sleep(100);
    endwin();
    return 0;
}

_Noreturn void vespa(int pipeout) {
    struct pos pos_vespa;
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

_Noreturn void nave_player(int pipeout) {
    struct pos pos_navicella;
    pos_navicella.c='#';
    pos_navicella.x= 1;
    pos_navicella.y= maxy / 2;
    write(pipeout, &pos_navicella, sizeof(pos_navicella));
    keypad(stdscr, TRUE);
    while(1) {
        int c = getch();
        switch(c) {
            case KEY_UP:
                if(pos_navicella.y > 1)
                    pos_navicella.y-=1;
                break;
            case KEY_DOWN:
                if(pos_navicella.y < maxy - DIM_NAVICELLA)
                    pos_navicella.y+=1;
                break;
        }
        write(pipeout, &pos_navicella, sizeof(pos_navicella));
    }
}

void AreaGioco (int pipein) {
    int vite = 3;
    int i;
    _Bool collision = false;
    struct pos vespa, navicella, valore_letto, trap1, trap2, trap3;
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
                attron(COLOR_PAIR(4));
                for (i = 0; i < DIM_NAVICELLA; i++) {
                    mvprintw(navicella.y + i, navicella.x, "      ");
                }
                attron(COLOR_PAIR(2));
                for (i = 0; i < DIM_NAVICELLA; i++) {
                    mvprintw(valore_letto.y+i, valore_letto.x, nave[i]);
                }
            }
            navicella = valore_letto;
        }
        /* visualizzo l'oggetto nella posizione aggiornata */
        if(vespa.x == navicella.x && vespa.y == navicella.y) {
            vite--;
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