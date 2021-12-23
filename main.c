#include "menu.h"

#define KEY_SPACE 32
#define DIM_NAVICELLA 6
#define DIM_NEMICO 3
#define maxx 80 /* Numero di colonne dello schermo */
#define maxy 24 /* Numero di righe dello schermo */
#define NAVE 0  // identificatore posizione navicella
#define MISSILE_1 1 // identificatore posizione Missile 1
#define MISSILE_2 2 // identificatore posizione Missile 2
#define MAX_MISSILI 10 // Numero (pari) massimo di missili sparabili contemporaneamente

#define START_ID_MISSILI 200 // DA 200 a 300  identificatori per i missili
#define END_ID_MISSILI 300
#define START_ID_NEMICI 11 // DA 11 a 100   identificatori Nemici
#define END_ID_NEMICI 100
#define START_ID_BOMBE_NEMICHE 101 // Da 101 a 150  identinficatori Bombe nemiche
#define END_ID_BOMBE_NEMICHE 150


/* Struttura per la comunicazione tra figli e padre */
typedef struct {
    int id; /* soggetto che invia il dato: vespa o nave_player */
    int x; /* coordinata x */
    int y; /* coordinata y */
    int vite;
    int pid;
} Pos;

const Pos init = {0,0,0,0,0}; //Costante utilizzata per inizializzare la struttura pos
_Noreturn void nave_player(int pipeout, int pipein);
void AreaGioco(int pipein, int pipeout, Pos *array_nemici);

int M; //Numero nemici
int num_missili = 0;

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

char *nemico_lv2[DIM_NEMICO]={" △ ",
                              "◁ ◊",
                              " ▽ "
};
char *nemico_lv3[DIM_NEMICO]= {"♿♿ ",
                               "♿♿ ",
                               "♿♿ "
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
    M = 20;

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
        }
        clear();
        refresh();
    }
}

StatoCorrente gioco() {
    int fd1[2];
    int fd2[2];
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
    init_color(COLOR_GREEN_L, 748, 999, 0); //verde fiamma
    init_color(COLOR_ORANGE_L, 999, 725, 0); // arancione fiamma
    init_color(COLOR_PURPLE_L, 525, 196, 670); // viola fiamma
    init_color(COLOR_GREEN_WATER, 0, 999, 889); // Verde acqua fiamma
    init_pair(0, COLOR_BLACK, COLOR_BLACK); // per cancellare
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // per scrivere
    init_pair(2, COLOR_GREEN_L, COLOR_BLACK);
    init_pair(3, COLOR_ORANGE_L, COLOR_BLACK);
    init_pair(4, COLOR_PURPLE_L, COLOR_BLACK);
    init_pair(5, COLOR_GREEN_WATER, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);

    bkgd(COLOR_PAIR(1));

    if (pipe(fd1) == -1) { //inizializzazione della pipe con AreaGioco degli errori
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (pipe(fd2) == -1) { //inizializzazione della pipe con AreaGioco degli errori
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (fcntl(fd2[0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);

    pid_navicella = fork(); //generazione di un secondo processo figlio per la nave_player
    switch (pid_navicella) {
        case -1:
            perror("Errore nell'esecuzione della fork.");
            _exit(1);
        case 0:
            prctl(PR_SET_NAME, (unsigned long) "Navicella");
            mvprintw(maxy / 2, maxx / 2, "%s", nave);
            close(fd1[0]); /* chiusura del descrittore di lettura (standard input)*/
            close(fd2[1]); /* chiusura del descrittore di scrittura (standard output)*/
            nave_player(fd1[1], fd2[0]); /* il secondo processo figlio invoca la funzione nave_player passandogli la pipe in scrittura*/
        default:    //processo padre
            array_pos_nemici = (Pos *) calloc(M, sizeof(Pos));
            int temp, j=0, status = 0;
            for (i = 0; i < M; i++) {
                pid_nemici = fork();
                if (pid_nemici == -1) {
                    perror("Errore nell'esecuzione della fork.");
                    _exit(1);
                } else if (pid_nemici == 0) {
                    int vite;
                    char nome[10];
                    sprintf(nome, "Nemico_%d", i);
                    prctl(PR_SET_NAME, (unsigned long) nome);

                    array_pos_nemici[i].vite = 3;
                    array_pos_nemici[i].pid = getpid();
                    while (true) {
                        if(array_pos_nemici[i].vite > 0) {
                            array_pos_nemici[i].y = (1 + (i % 5) * (DIM_NEMICO + 1)) + j;
                            temp = (int) (i / 5) + 1;
                            array_pos_nemici[i].x = maxx - temp * (DIM_NEMICO + 1);
                            array_pos_nemici[i].id = START_ID_NEMICI + i;
                            write(fd1[1], &array_pos_nemici[i], sizeof(array_pos_nemici[i]));
                            usleep(1000000);
                        } else {
                            exit(1);
                        }
                        if(j <= 4 && status == 0){
                            j++;
                            if(j == 4)
                                status = 1;
                        } else if (j >= 0 && status == 1){
                            j--;
                            if(j == 0)
                                status = 0;
                        }
                    }
                } else if (pid_nemici != 0) {
                    continue;
                }
            }
            prctl(PR_SET_NAME, (unsigned long) "Area_gioco");
            close(fd1[1]); /* chiusura del descrittore di scrittura (standard output)*/
            close(fd2[0]); /* chiusura del descrittore di lettura (standard input)*/
            AreaGioco(fd1[0], fd2[1], array_pos_nemici); /* il processo padre invoca la funzione di AreaGioco */
    }
    /* siamo usciti dalla funzione di AreaGioco e vengono terminati i 2 processi figli e ripristinato il normale modo operativo dello schermo */
    kill(pid_navicella, 1);
    clear();
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



void nave_player(int pipeout, int pipein) {
    int filedes[2];
    int pid_missile1;
    int pid_missile2;
    int i = 0, j = 0;
    Pos pos_missili[MAX_MISSILI];
    Pos pos_missile1, pos_missile2, temp_missile;
    Pos pos_navicella;
    _Bool blocco_sparo = false;

    pos_navicella.id = NAVE;
    pos_navicella.x = 1;
    pos_navicella.y = (maxy - DIM_NAVICELLA) / 2;

    for (i = 0; i < MAX_MISSILI; i++) {
        pos_missili[i].x = 0;
        pos_missili[i].y = 0;
        pos_missili[i].id = 0;
        pos_missili[i].vite = 0;
        pos_missili[i].pid = 0;
    }

    if (pipe(filedes) == -1) { //inizializzazione della pipe con AreaGioco degli errori
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (fcntl(filedes[0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);

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
                if (num_missili <= MAX_MISSILI - 2) {
                    num_missili += 2;
                    pos_missile1.y = pos_navicella.y + (DIM_NAVICELLA / 2);
                    pos_missile1.x = pos_navicella.x + DIM_NAVICELLA;
                    pos_missile2.y = pos_navicella.y + (DIM_NAVICELLA / 2);
                    pos_missile2.x = pos_navicella.x + DIM_NAVICELLA;

                    pos_missile1.id = START_ID_MISSILI + num_missili - 2;
                    pos_missile1.vite = 1;
                    pos_missile2.id = START_ID_MISSILI + num_missili - 1;
                    pos_missile2.vite = 1;

                    i = 0;
                    pid_missile1 = fork(); //generazione processo
                    switch (pid_missile1) {
                        case -1:
                            perror("Errore nell'esecuzione della fork.");
                            exit(1);
                        case 0:
                            prctl(PR_SET_NAME, (unsigned long) "Missile 1");
                            /* funzione che genera coordinate missile 1*/
                            i = 0;
                            pos_missile1.vite = 1;
                            pos_missile1.pid = getpid();
                            while (TRUE) {
                                if (i % 7 == 0) {
                                    pos_missile1.y -= 1;
                                }
                                pos_missile1.x += 1;
                                write(pipeout, &pos_missile1, sizeof(pos_missile1));
                                write(filedes[1], &pos_missile1, sizeof(pos_missile1));
                                i++;
                                if (pos_missile1.x > maxx || pos_missile1.y <= 0) {
                                    pos_missile1.vite = 0;
                                    write(filedes[1], &pos_missile1, sizeof(pos_missile1));
                                    exit(1);
                                }
                                usleep(100000); //velocità missile

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
                                    i = 0;
                                    pos_missile2.vite = 1;
                                    pos_missile2.pid = getpid();
                                    while (TRUE) {
                                        if (i % 7 == 0) {
                                            pos_missile2.y += 1;
                                        }
                                        pos_missile2.x += 1;
                                        write(pipeout, &pos_missile2, sizeof(pos_missile2));
                                        write(filedes[1], &pos_missile2, sizeof(pos_missile2));
                                        i++;
                                        if (pos_missile2.x > maxx || pos_missile2.y >= maxy) {
                                            pos_missile2.vite = 0;
                                            write(filedes[1], &pos_missile2, sizeof(pos_missile2));
                                            exit(1);
                                        }
                                        usleep(100000); //regola velocità missili
                                    }
                                    break;
                                default:
                                    break;
                            }
                            break;
                    }
                }
                break;
        }
        for (i = 0; i <= MAX_MISSILI * 100; i++) {
            read(filedes[0], &temp_missile, sizeof(temp_missile));
            if ((temp_missile.id - START_ID_MISSILI) >= 0 &&
                (temp_missile.id - START_ID_MISSILI) < MAX_MISSILI) {
                pos_missili[temp_missile.id - START_ID_MISSILI].vite = temp_missile.vite;
            }
        }
        for (i = 0; i <= MAX_MISSILI * 100; i++) {
            read(pipein, &temp_missile, sizeof(temp_missile));
            if ((temp_missile.id - START_ID_MISSILI) >= 0 &&
                (temp_missile.id - START_ID_MISSILI) < MAX_MISSILI) {
                pos_missili[temp_missile.id - START_ID_MISSILI].vite = temp_missile.vite;
            }
        }
        for (i = 0, j = 0; i < MAX_MISSILI; i++) {
            if (pos_missili[i].vite == 0) {
                j++;
            }
        }

        if (num_missili == MAX_MISSILI && j == MAX_MISSILI) {
            num_missili = 0;
        }
        //mvprintw(2,0,"num_missili= %d, j= %d", num_missili, j);
    }
}





void AreaGioco(int pipein, int pipeout, Pos *pos_nemici) {
    int vite = 1000;
    int i, j, k;
    _Bool collision = false;
    Pos navicella, valore_letto;
    Pos missili[MAX_MISSILI];
    navicella.x = -1;

    for (i = 0; i < MAX_MISSILI; i++) {
        missili[i] = init;
    }

    for (i = 0; i < DIM_NAVICELLA; i++) {
        if (i <= 1 || i >= 4) {
            attron(COLOR_PAIR(6));
            mvaddstr((maxy - DIM_NAVICELLA) / 2 + i, 1, nave[i]);
            attroff(COLOR_PAIR(6));
        } else {
            attron(COLOR_PAIR(2));
            mvaddstr((maxy - DIM_NAVICELLA) / 2 + i, 1, nave[i]);
            attroff(COLOR_PAIR(2));
        }

    }

    while (true) {
        /*if(getmaxx(stdscr)!= 80 || getmaxy(stdscr)!= 24){
            system("printf '\033[8;24;80t'");
            resize_term(24,80);
            clear();
            refresh();
        }*/
        read(pipein, &valore_letto, sizeof(valore_letto)); /* leggo dalla pipe */
        if (valore_letto.id == NAVE) {
            if (navicella.x >= 0) { /* cancello la 'vecchia' posizione della navicella */
                attron(COLOR_PAIR(0));
                for (i = 0; i < DIM_NAVICELLA; i++) {
                    mvaddstr(navicella.y + i, navicella.x, "      ");
                }
                attroff(COLOR_PAIR(0));
                attron(COLOR_PAIR(2));
                for (i = 0; i < DIM_NAVICELLA; i++) {
                    if(i<=1 || i>=4) {
                        attron(COLOR_PAIR(6));
                        mvaddstr(valore_letto.y + i, valore_letto.x, nave[i]);
                        attroff(COLOR_PAIR(6));
                    }else{
                        attron(COLOR_PAIR(2));
                        mvaddstr(valore_letto.y + i, valore_letto.x, nave[i]);
                        attroff(COLOR_PAIR(2));
                    }
                }

            }
            navicella = valore_letto;
        } else if (valore_letto.id >= START_ID_MISSILI && valore_letto.id <= END_ID_MISSILI) {
            if (missili[valore_letto.id - START_ID_MISSILI].id != 0) {
                attron(COLOR_PAIR(0));
                mvprintw(missili[valore_letto.id - START_ID_MISSILI].y, missili[valore_letto.id - START_ID_MISSILI].x,
                         " ");
                attroff(COLOR_PAIR(0));
            }
            attron(COLOR_PAIR(3));
            mvprintw(valore_letto.y, valore_letto.x, "✝"); ///♿ ⟢ ⁂ ꗇ ꗈ 💣 🚀 卐 ◌́ ◌͂ ۩
            attroff(COLOR_PAIR(3));
            missili[valore_letto.id - START_ID_MISSILI] = valore_letto;
        } else if (valore_letto.id >= START_ID_NEMICI && valore_letto.id <= END_ID_NEMICI) {
            if (pos_nemici[valore_letto.id - START_ID_NEMICI].id != 0) {
                attron(COLOR_PAIR(0));
                for (j = 0; j < DIM_NEMICO; j++) {
                    mvprintw(pos_nemici[valore_letto.id - START_ID_NEMICI].y + j,
                             pos_nemici[valore_letto.id - START_ID_NEMICI].x, "   ");
                }
                attroff(COLOR_PAIR(0));
            }
            if (pos_nemici[valore_letto.id - START_ID_NEMICI].vite == 3) {
                attron(COLOR_PAIR(4));
                for (j = 0; j < DIM_NEMICO; j++) {
                    mvprintw(valore_letto.y + j, valore_letto.x, nemico_lv1[j]);
                }
                attroff(COLOR_PAIR(4));
            } else if (pos_nemici[valore_letto.id - START_ID_NEMICI].vite == 2) {
                attron(COLOR_PAIR(5));
                for (j = 0; j < DIM_NEMICO; j++) {
                    mvprintw(valore_letto.y + j, valore_letto.x, nemico_lv2[j]);
                }
                attroff(COLOR_PAIR(1));
            }else if(pos_nemici[valore_letto.id - START_ID_NEMICI].vite == 1){
                attron(COLOR_PAIR(5));
                for (j = 0; j < DIM_NEMICO; j++) {
                    mvprintw(valore_letto.y + j, valore_letto.x, nemico_lv3[j]);
                }
                attroff(COLOR_PAIR(1));
            }

            if (pos_nemici[valore_letto.id - START_ID_NEMICI].pid == 0) {
                pos_nemici[valore_letto.id - START_ID_NEMICI] = valore_letto;
            } else {
                pos_nemici[valore_letto.id - START_ID_NEMICI].x = valore_letto.x;
                pos_nemici[valore_letto.id - START_ID_NEMICI].y = valore_letto.y;
                pos_nemici[valore_letto.id - START_ID_NEMICI].id = valore_letto.id;
                pos_nemici[valore_letto.id - START_ID_NEMICI].pid = valore_letto.pid;
            }
        }

        for (i = 0; i < M; i++) {
            if (pos_nemici[i].id != 0) {
                for (j = 0; j < MAX_MISSILI; j++) {
                    if (missili[j].id != 0) {
                        if (pos_nemici[i].x == missili[j].x && pos_nemici[i].y == missili[j].y
                            || pos_nemici[i].x + 1 == missili[j].x && pos_nemici[i].y == missili[j].y
                            || pos_nemici[i].x + 2 == missili[j].x && pos_nemici[i].y == missili[j].y
                            || pos_nemici[i].x == missili[j].x && pos_nemici[i].y + 1 == missili[j].y
                            || pos_nemici[i].x == missili[j].x && pos_nemici[i].y + 2 == missili[j].y
                            || pos_nemici[i].x + 1 == missili[j].x && pos_nemici[i].y + 2 == missili[j].y
                            || pos_nemici[i].x + 2 == missili[j].x && pos_nemici[i].y + 2 == missili[j].y) {
                            /*for (k = 0; k < DIM_NEMICO; ++k) {
                                for (int l = 0; l < DIM_NEMICO; ++l) {*/
                            /*if (missili[j].x == pos_nemici[i].x + k + l &&
                                missili[j].y == pos_nemici[i].y + k + l) {*/
                            kill(missili[j].pid, SIGKILL);
                            missili[j].vite = 0;
                            write(pipeout, &missili[j], sizeof(missili[j]));
                            pos_nemici[i].vite--;
                            //mvprintw(1,0, "pos_nemici[%d].vite = %d", i, pos_nemici[i].vite);
                            if (pos_nemici[i].vite == 0) {
                                kill(pos_nemici[i].pid, SIGKILL);
                                attron(COLOR_PAIR(0));
                                for (int m = 0; m < DIM_NEMICO; m++) {
                                    mvaddstr(pos_nemici[i].y + m, pos_nemici[i].x, "   ");
                                }
                                attroff(COLOR_PAIR(0));
                                pos_nemici[i] = init;
                            }
                            attron(COLOR_PAIR(0));
                            mvaddstr(missili[j].y, missili[j].x, " ");
                            attroff(COLOR_PAIR(0));
                            missili[j] = init;
                            /*}
                        }*/
                        }
                    }
                }
            }
        }

        move(0, 0);
        clrtoeol();
        mvprintw(0, 0, "VITE: %d", vite);
        refresh();
        /*if (vite == 0) {
            collision = true;
        }*/
    }
}