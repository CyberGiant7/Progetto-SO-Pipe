#include "menu.h"

char *gioca[2]={"█▀▀ █ █▀█ █▀▀ ▄▀█",
                "█▄█ █ █▄█ █▄▄ █▀█"};

char *esci[2]={"█▀▀ █▀ █▀▀ █",
               "██▄ ▄█ █▄▄ █"};

char *opzioni_stampa[2]={"█▀█ █▀█ ▀█ █ █▀█ █▄ █ █",
                         "█▄█ █▀▀ █▄ █ █▄█ █ ▀█ █"};


char *titolo[DIM_TITOLO]={"               ██████╗██████╗  █████╗  █████╗ ███████╗",
                          "              ██╔════╝██╔══██╗██╔══██╗██╔══██╗██╔════╝",
                          "              ╚█████╗ ██████╔╝███████║██║  ╚═╝█████╗  ",
                          "               ╚═══██╗██╔═══╝ ██╔══██║██║  ██╗██╔══╝  ",
                          "              ██████╔╝██║     ██║  ██║╚█████╔╝███████╗",
                          "              ╚═════╝ ╚═╝     ╚═╝  ╚═╝ ╚════╝ ╚══════╝",
                          "                                                                 ",
                          "██████╗ ███████╗███████╗███████╗███╗  ██╗██████╗ ███████╗██████╗ ",
                          "██╔══██╗██╔════╝██╔════╝██╔════╝████╗ ██║██╔══██╗██╔════╝██╔══██╗",
                          "██║  ██║█████╗  █████╗  █████╗  ██╔██╗██║██║  ██║█████╗  ██████╔╝",
                          "██║  ██║██╔══╝  ██╔══╝  ██╔══╝  ██║╚████║██║  ██║██╔══╝  ██╔══██╗",
                          "██████╔╝███████╗██║     ███████╗██║ ╚███║██████╔╝███████╗██║  ██║",
                          "╚═════╝ ╚══════╝╚═╝     ╚══════╝╚═╝  ╚══╝╚═════╝ ╚══════╝╚═╝  ╚═╝"};

StatoCorrente area_menu();
int sottomenu(WINDOW* menuwin, char *scelte[], int num_scelte);

int maxx = DEFAULT_MAXX; /* Numero di colonne dello schermo */
int maxy = DEFAULT_MAXY; /* Numero di righe dello schermo */

StatoCorrente menu() {
    int i;
    char parametro[25];


    StatoCorrente status;
    Oggetto fake_navicella = {0, ID_NAVICELLA, {1, maxy/2}, {0, 0}, 0, 0};
    Oggetto fake_nemico1 = {0, ID_NEMICO, {maxx-4, (maxy/2)}, {0, 0}, 0, 0};
    Oggetto fake_nemico2 = {0, ID_NEMICO, {maxx-6, (maxy/2+4)}, {0, 0}, 0, 0};
    Oggetto fake_missile1 = {0, ID_MISSILE, {8, maxy/2 +3}, {0, 0}, 0, 0};
    Oggetto fake_missile2 = {0, ID_MISSILE, {10, maxy/2 +1}, {0, 0}, 0, 0};
    Oggetto fake_bomba = {0, ID_BOMBA, {maxx-7, (maxy/2)+1}, {0, 0}, 0, 0};
    setlocale(LC_ALL, "");
    initscr(); /* inizializzazione dello schermo */
    noecho(); /* i caratteri corrispondenti ai tasti premuti non saranno visualizzati sullo schermo del terminale */
    curs_set(0);

    sprintf(parametro, "printf '\033[8;%d;%dt'", maxy, maxx);
    system(parametro);
    resize_term(maxy, maxx);

    //inizializzazione colori
    start_color();
    init_pair(0, COLOR_BLACK, COLOR_BLACK); // per cancellare
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // per scrivere
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
    init_pair(7, COLOR_RED, COLOR_BLACK);

    for (i = 0; i < DIM_TITOLO; ++i) {
        attron(COLOR_PAIR(1));
        mvprintw(i, (maxx - 65) / 2, "%s\n", titolo[i]);
        attroff(COLOR_PAIR(1));
    }
    stampaNavicella(fake_navicella);
    stampaMiss_bomb(fake_missile1);
    stampaMiss_bomb(fake_missile2);
    stampaNemico(fake_nemico1, 3);
    stampaNemico(fake_nemico2, 3);
    stampaMiss_bomb(fake_bomba);

    refresh();


    status = area_menu();
    return status;
}


StatoCorrente area_menu() {
    int i;
    prctl(PR_SET_NAME, (unsigned long) "Menu");




    WINDOW *menuwin = newwin(11, maxx/2, 13, (maxx /2)/ 2);
    box(menuwin, 0, 0);
    refresh();
    wrefresh(menuwin);

    keypad(menuwin, TRUE);
    char *scelte[MENU_ELEM] = {"GIOCA",
                               "OPZIONI",
                               "ESCI"};
    int scelta;
    int selezione = 0;

    while (TRUE) {
        wmove(menuwin, 0, 1);
        for (i = 0; i < MENU_ELEM; ++i) {
            if (i == selezione)
                wattron(menuwin, COLOR_PAIR(3));
            switch (i) {
                case 0:
                    for (int j = 0; j < 2; ++j) {
                        mvwprintw(menuwin, 1+j, (maxx/2 - 17)/2, gioca[j]);
                    }
                    break;
                case 1:
                    for (int j = 0; j < 2; ++j) {
                        mvwprintw(menuwin, 4+j, (maxx/2 - 23)/2, opzioni_stampa[j]);
                    }
                    break;
                case 2:
                    for (int j = 0; j < 2; ++j) {
                        mvwprintw(menuwin, 7+j, (maxx/2 - 12)/2, esci[j]);
                    }
                    break;

            }
            wattroff(menuwin, COLOR_PAIR(3));
        }
        refresh();
        wrefresh(menuwin);
        scelta = wgetch(menuwin);
        switch (scelta) {
            case KEY_UP:
                selezione--;
                if (selezione == -1)
                    selezione = 0;
                break;
            case KEY_DOWN:
                selezione++;
                if (selezione == MENU_ELEM)
                    selezione = MENU_ELEM - 1;
                break;
            default:
                break;
        }
        if (scelta == 10) //caso enter
            break;
    }
    switch (selezione) {
        case 0: //caso Gioca
            return GIOCA;
            break;
        case 1: // caso Opzioni
            return OPZIONI;
            break;
        case 2: //caso Esci
            return ESCI;
            break;
    }
}

#define NUMERO_VELOCITA_MISSILI 3

StatoCorrente opzioni() {
    int i;
    clear();
    WINDOW *menuwin = newwin(MENUWIN_Y, MENUWIN_X, 1, (maxx - MENUWIN_X) / 2);
    box(menuwin, 0, 0);
    refresh();
    wrefresh(menuwin);

    keypad(menuwin, TRUE);
    char *scelte[NUMERO_OPZIONI] = {"\u2B05 Indietro",
                                    "Nemici",
                                    "Risoluzione",
                                    "Missili"};

    char *resolutions[NUMERO_RISOLUZIONI] = {"80  x 24",
                                             "100 x 30 (default)",
                                             "100 x 45",
                                             "150 x 45",
                                             "170 x 51"};

    char *velocitaMissili[NUMERO_VELOCITA_MISSILI] = {"LENTO (default)",
                                                      "MEDIO",
                                                      "VELOCE"};

    char **enemy_count = (char**) malloc(sizeof(char*) * NUMERO_SELEZIONI_NEMICI);
    for (i = 0; i < NUMERO_SELEZIONI_NEMICI; ++i) {
        enemy_count[i] = (char*) malloc(sizeof(char)*20);
    }

    char parametro[25];
    Risoluzioni risoluzione;
    int case_nemici = (M/num_righe) -1;
    int scelta;
    int selezione = 0;


    while (TRUE) {
        for (i = 0; i < NUMERO_SELEZIONI_NEMICI; ++i) {
            if (i == 3){
                sprintf(enemy_count[i], "%d (default)", (i + 1)*num_righe);
            }
            else
                sprintf(enemy_count[i], "%d", (i + 1)*num_righe);
        }
        wmove(menuwin, 0, 1);
        for (i = 0; i < NUMERO_OPZIONI; ++i) {
            if (i == selezione)
                wattron(menuwin, A_REVERSE);
            wprintw(menuwin, "%s──", scelte[i]);
            wattroff(menuwin, A_REVERSE);
        }
        scelta = wgetch(menuwin);
        switch (scelta) {
            case KEY_LEFT:
                selezione--;
                if (selezione == -1)
                    selezione = 0;
                break;
            case KEY_RIGHT:
                selezione++;
                if (selezione == NUMERO_OPZIONI)
                    selezione = NUMERO_OPZIONI - 1;
                break;
            default:
                break;
        }
        for (i = 0; i < MENUWIN_Y - 2; ++i) {
            mvwprintw(menuwin, i + 1, 1, "                                                                ");
        }
        switch (selezione) {
            case 0:
                mvwprintw(menuwin, 1, 1, "Torna al menu principale");
                break;
            case 1:
                mvwprintw(menuwin, 1, 1, "Per inserire il numero dei nemici");
                break;
            case 2:
                mvwprintw(menuwin, 1, 1, "Per cambiare la risoluzione");
                mvwprintw(menuwin, 2, 1, "Cambiando la risoluzione cambiano il numero di nemici inseribili");
                break;
            case 3:
                mvwprintw(menuwin, 1, 1, "Per selezionare la velocità dei missili amici e nemici");

        }
        if (scelta == 10) //caso enter
            break;
    }
    switch (selezione) {
        case 0:
            for (i = 0; i < NUMERO_SELEZIONI_NEMICI; ++i) {
                free (enemy_count[i]);
            }
            free(enemy_count);
            return MENU;
            break;
        case 1: // caso scelta numero nemici
            case_nemici = sottomenu(menuwin, enemy_count, NUMERO_SELEZIONI_NEMICI);
            M = (case_nemici + 1) * num_righe;

            for (i = 0; i < NUMERO_SELEZIONI_NEMICI; ++i) {
                free (enemy_count[i]);
            }
            free(enemy_count);
            return OPZIONI;
            break;
        case 2:
            risoluzione = sottomenu(menuwin, resolutions, NUMERO_RISOLUZIONI);
            switch (risoluzione) {
                case R_80_X_24:
                    maxx = 80;
                    maxy = 24;
                    num_righe = 5;
                    mov_verticale = 4;
                    M = (case_nemici + 1) * num_righe;
                    break;
                case R_100_X_30:
                    maxx = 100;
                    maxy = 30;
                    num_righe = 6;
                    mov_verticale = 6;
                    M = (case_nemici + 1) * num_righe;
                    break;
                case R_100_X_45:
                    maxx = 100;
                    maxy = 45;
                    num_righe = 9;
                    mov_verticale = 9;
                    M = (case_nemici + 1) * num_righe;
                    break;
                case R_150_X_45:
                    maxx = 150;
                    maxy = 45;
                    num_righe = 9;
                    mov_verticale = 9;
                    M = (case_nemici + 1) * num_righe;
                    break;
                case R_170_X_51:
                    maxx = 170;
                    maxy = 45;
                    num_righe = 9;
                    mov_verticale = 9;
                    M = (case_nemici + 1) * num_righe;
                    break;
            }
            sprintf(parametro, "printf '\033[8;%d;%dt'", maxy, maxx);
            system(parametro);
            resize_term(maxy, maxx);

            for (i = 0; i < NUMERO_SELEZIONI_NEMICI; ++i) {
                free (enemy_count[i]);
            }
            free(enemy_count);
            return OPZIONI;
            break;
        case 3:
            switch (sottomenu(menuwin, velocitaMissili, NUMERO_VELOCITA_MISSILI)) {
                case 0:
                    velocita_missili = 100000;
                    break;
                case 1:
                    velocita_missili = 50000;
                    break;
                case 2:
                    velocita_missili = 10000;
                    break;
            }
            break;
    }
}

int sottomenu(WINDOW* menuwin, char *scelte[], int num_scelte){
    Risoluzioni selezione = 0;
    int scelta = 0, i;
    for (i = 0; i < MENUWIN_Y - 2; ++i) {
        mvwprintw(menuwin, i + 1, 1, "                                                                ");
    }
    while (TRUE) {
        wmove(menuwin, 0, 1);
        for (i = 0; i < num_scelte; ++i) {
            if (i == selezione)
                wattron(menuwin, A_REVERSE);
            mvwprintw(menuwin,i+1,1, "%s", scelte[i]);
            wattroff(menuwin, A_REVERSE);
        }
        scelta = wgetch(menuwin);
        switch (scelta) {
            case KEY_UP:
                selezione--;
                if (selezione == -1)
                    selezione = 0;
                break;
            case KEY_DOWN:
                selezione++;
                if (selezione == num_scelte)
                    selezione = num_scelte - 1;
                break;
            default:
                break;
        }
        for (i = 0; i < MENUWIN_Y - 2; ++i) {
            mvwprintw(menuwin, i + 1, 1, "                              ");
        }
        if (scelta == 10) //caso enter
            break;
    }
    return selezione;
}