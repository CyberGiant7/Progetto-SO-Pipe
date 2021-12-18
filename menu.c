#include "menu.h"

char *gioca[9]={"██████████████████████████████████████████████",
                "██░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░██",
                "██░░░░████░░██████░░██████░░░░████░░██████░░██",
                "██░░██░░░░░░░░██░░░░██░░██░░██░░░░░░██░░██░░██",
                "██░░██░░██░░░░██░░░░██░░██░░██░░░░░░██████░░██",
                "██░░██░░██░░░░██░░░░██░░██░░██░░░░░░██░░██░░██",
                "██░░░░████░░██████░░██████░░░░████░░██░░██░░██",
                "██░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░██",
                "██████████████████████████████████████████████"};

char *esci[9]={"██████████████████████████████████████",
               "██░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░██",
               "██░░██████░░░░████░░░░████░░██████░░██",
               "██░░██░░░░░░██░░░░░░██░░░░░░░░██░░░░██",
               "██░░██████░░░░██░░░░██░░░░░░░░██░░░░██",
               "██░░██░░░░░░░░░░██░░██░░░░░░░░██░░░░██",
               "██░░██████░░████░░░░░░████░░██████░░██",
               "██░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░██",
               "██████████████████████████████████████"};

_Noreturn void scelta_fun(int pipeout);
StatoCorrente area_menu(int pipein);

StatoCorrente menu(){
    system("printf '\033[8;24;80t'");
    setlocale(LC_ALL, "");

    StatoCorrente status;
    int filedes[2];
    int pid_scelta_fun;
    signal(SIGCHLD, SIG_IGN);

    initscr(); /* inizializzazione dello schermo */
    noecho(); /* i caratteri corrispondenti ai tasti premuti non saranno visualizzati sullo schermo del terminale */
    curs_set(0);
    resize_term(24,80);

    start_color();
    init_pair(0, COLOR_BLACK, COLOR_BLACK); //per cancellare
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    refresh();


    if (pipe(filedes) == -1) { //inizializzazione della pipe con AreaGioco degli errori
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (fcntl(filedes[0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);

    pid_scelta_fun = fork();
    switch (pid_scelta_fun) {
        case -1:
            perror("Errore nell'esecuzione della fork.");
            exit(1);
        case 0:
            close(filedes[0]); /* chiusura del descrittore di lettura (standard input)*/
            scelta_fun(filedes[1]); /* il primo processo figlio invoca la funzione contadino passandogli la pipe in scrittura*/
        default: //processo padre
            close(filedes[1]); /* chiusura del descrittore di scrittura (standard output)*/
            status = area_menu(filedes[0]);
    }
    switch (status) {
        case GIOCA:
            kill(pid_scelta_fun, 1);
            return status;
            break;
        case ESCI:
            kill(pid_scelta_fun, 1);
            return status;
            break;
    }
}

_Noreturn void scelta_fun(int pipeout) {
    int scelta = 1;
    write(pipeout, &scelta, sizeof(scelta));
    keypad(stdscr, TRUE);
    while(TRUE) {
        int c = getch();
        switch(c) {
            case KEY_UP:
                if(scelta == 1)
                    scelta = MENU_ELEM;
                else
                    scelta--;
                break;
            case KEY_DOWN:
                if(scelta == MENU_ELEM)
                    scelta = 1;
                else
                    scelta++;
                break;
            case 10:    //tasto enter
                scelta = -1;
                break;
            default:
                break;
        }
        write(pipeout, &scelta, sizeof(scelta));
    }
}

StatoCorrente area_menu(int pipein) {
    int valore_letto, valore_scorso = 1;
    int i;
    _Bool active = TRUE;
    clear();
    read(pipein, &valore_letto, sizeof(valore_letto)); /* leggo dalla pipe */
    for (i = 0; i < 9; ++i) {
        attron(COLOR_PAIR(2));
        mvprintw(i, 0, "%s\n", gioca[i]);
    }
    for (i = 0; i < 9; ++i) {
        attron(COLOR_PAIR(2));
        mvprintw(i + 10, 0, "%s\n", esci[i]);
    }
    refresh();
    while (TRUE) {
        clrtoeol();
        read(pipein, &valore_letto, sizeof(valore_letto)); /* leggo dalla pipe */
        if (valore_scorso != valore_letto) {
            if (valore_scorso == 1) {
                for (i = 0; i < 9; ++i) {
                    attron(COLOR_PAIR(2));
                    mvprintw(i, 0, "%s\n", gioca[i]);
                }
            } else if (valore_scorso == 2) {
                for (i = 0; i < 9; ++i) {
                    attron(COLOR_PAIR(2));
                    mvprintw(i + 10, 0, "%s\n", esci[i]);
                }
            }
        }

        system("printf '\033[8;24;80t'");
        if (active)
            active = FALSE;
        else
            active = TRUE;

        if (valore_letto == 1) {
            if (active) {
                for (i = 0; i < 9; ++i) {
                    attron(COLOR_PAIR(1));
                    mvprintw(i, 0, "%s\n", gioca[i]);
                }
            } else {
                for (i = 0; i < 9; ++i) {
                    attron(COLOR_PAIR(2));
                    mvprintw(i, 0, "%s\n", gioca[i]);
                }
            }
        } else if (valore_letto == 2) {
            if (active) {
                for (i = 0; i < 9; ++i) {
                    attron(COLOR_PAIR(1));
                    mvprintw(i + 10, 0, "%s\n", esci[i]);
                }
            } else {
                for (i = 0; i < 9; ++i) {
                    attron(COLOR_PAIR(2));
                    mvprintw(i + 10, 0, "%s\n", esci[i]);
                }
            }
        } else if (valore_letto == -1) {//caso enter
            switch (valore_scorso) {
                case 1: //caso Gioca
                    return(GIOCA);
                    break;
                case 2: //caso Esci
                    return(ESCI);
                    break;
            }
        }
        curs_set(0);
        refresh();
        valore_scorso = valore_letto;
        usleep(500000);
    }
}