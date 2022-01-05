/**
 * In questo file vengono definite le funzioni per la gestione dei menu e delle varie impostazioni.
 */

#include "menu.h"

/// Inizializzazione delle varie stampe
char *gioca[2]={"█▀▀ █ █▀█ █▀▀ ▄▀█",
                "█▄█ █ █▄█ █▄▄ █▀█"};

char *esci[2]={"█▀▀ █▀ █▀▀ █",
               "██▄ ▄█ █▄▄ █"};

char *opzioni_stampa[2]={"█▀█ █▀█ ▀█ █ █▀█ █▄ █ █",
                         "█▄█ █▀▀ █▄ █ █▄█ █ ▀█ █"};


char *titolo[ALTEZZA_TITOLO]={"               ██████╗██████╗  █████╗  █████╗ ███████╗",
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

/// Definizione funzioni
int sottomenu(WINDOW* menuwin, char *scelte[], int num_scelte);

/// inizializzazione variabili globali
int maxx = DEFAULT_MAXX; /* Numero di colonne dello schermo */
int maxy = DEFAULT_MAXY; /* Numero di righe dello schermo */

/**
 * Funzione che serve per gestire il movimento tra i vari contesti tramite un apposito menu
 * @return Il prossimo contesto a cui accedere
 */
StatoCorrente menu() {
    /// Dichiarazione delle variabili
    int i;                  // Variabile contatore
    char parametro[25];     // Stringa per cambiare la risoluzione del terminale
    StatoCorrente status;   // Valore che verrà restituito

    /// Inizializzazione di variabili Oggetto puramente estetiche
    Oggetto fake_navicella = {0, ID_NAVICELLA, {1, maxy/2}, {0, 0}, 0, 0};
    Oggetto fake_nemico1 = {0, ID_NEMICO, {maxx-4, (maxy/2)}, {0, 0}, 0, 3};
    Oggetto fake_nemico2 = {0, ID_NEMICO, {maxx-6, (maxy/2+4)}, {0, 0}, 0, 3};
    Oggetto fake_missile1 = {0, ID_MISSILE1, {8, maxy/2 +3}, {0, 0}, 0, 0};
    Oggetto fake_missile2 = {0, ID_MISSILE1, {10, maxy/2 +1}, {0, 0}, 0, 0};
    Oggetto fake_bomba = {0, ID_BOMBA, {maxx-7, (maxy/2)+1}, {0, 0}, 0, 0};
    Oggetto fake_bomba2 = {0, ID_BOMBA, {maxx-14, (maxy/2)+5}, {0, 0}, 0, 0};



    setlocale(LC_ALL, "");      // per poter utilizzare caratteri unicode
    initscr();                  // inizializzazione dello schermo
    noecho();                   // i caratteri corrispondenti ai tasti premuti non saranno visualizzati sullo schermo del terminale
    curs_set(0);                // nasconde il cursore

    /// modifica della risoluzione del terminale
    sprintf(parametro, "printf '\033[8;%d;%dt'", maxy, maxx);
    system(parametro);
    resize_term(maxy, maxx);
    usleep(100000);

    ///inizializzazione colori
    start_color();
    init_pair(BLACK, COLOR_BLACK, COLOR_BLACK); // per cancellare
    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK); // per scrivere
    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(RED, COLOR_RED, COLOR_BLACK);

    /// Stampa iniziale
    for (i = 0; i < ALTEZZA_TITOLO; ++i) {
        attron(COLOR_PAIR(1));
        mvprintw(i, (maxx - LARGHEZZA_TITOLO) / 2, "%s\n", titolo[i]);
        attroff(COLOR_PAIR(1));
    }
    stampaNavicella(fake_navicella);
    stampaMissile(&fake_missile1);
    stampaMissile(&fake_missile2);
    stampaNemico(fake_nemico1, 3);
    stampaNemico(fake_nemico2, 3);
    stampaBomba(&fake_bomba);
    stampaBomba(&fake_bomba2);
    refresh();


    status = area_menu();
    endwin();
    return status;
}

/**
 * Funzione per gestire i movimenti tra i vari campi presenti nel menu e il conseguente accesso ad essi
 * @return Stato di uscita, ossia il contesto che deve essere aperto una volta usciti dal menu
 */
StatoCorrente area_menu() {
    /// Dichiarazione variabili
    int i;                      // Variabile contatore
    StatoCorrente stato_uscita; // Variabile da restituire
    int scelta;                 // Contiene la scelta selezionata
    int selezione = 0;          // Indica in quale campo del menu ci si trova

    WINDOW *menuwin = newwin(11, maxx/2, 13, (maxx /2)/ 2); //creazione di una finestra che contiene il menu
    box(menuwin, 0, 0);
    refresh();
    wrefresh(menuwin);

    keypad(menuwin, TRUE);      // Abilitazione dei tasti freccia

    while (TRUE) {
        wmove(menuwin, 0, 1);
        /// stampa dei campi del menu
        for (i = 0; i < MENU_ELEM; ++i) {
            if (i == selezione)                         // se ci troviamo nel campo attualmente selezionato
                wattron(menuwin, COLOR_PAIR(YELLOW));   // il campo stampato è colorato
            switch (i) {
                case GIOCA:
                    for (int j = 0; j < ALTEZZA_GIOCA; ++j) {
                        mvwprintw(menuwin, 1+j, (maxx/2 - LARGHEZZA_GIOCA)/2, gioca[j]);
                    }
                    break;
                case OPZIONI:
                    for (int j = 0; j < ALTEZZA_OPZIONI; ++j) {
                        mvwprintw(menuwin, 4+j, (maxx/2 - LARGHEZZA_OPZIONI)/2, opzioni_stampa[j]);
                    }
                    break;
                case ESCI:
                    for (int j = 0; j < ALTEZZA_ESCI; ++j) {
                        mvwprintw(menuwin, 7+j, (maxx/2 - LARGHEZZA_ESCI)/2, esci[j]);
                    }
                    break;
                default:
                    break;
            }
            wattroff(menuwin, COLOR_PAIR(YELLOW));
        }
        refresh();
        wrefresh(menuwin);
        scelta = wgetch(menuwin);   // aspettiamo l'input
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
        system("aplay -q Suoni/menu.wav &");
        if (scelta == 10) //caso enter
            break;
    }
    switch (selezione) { // In base alla selezione assegna lo stato di uscita
        case GIOCA:
            stato_uscita = GIOCA;
            break;
        case OPZIONI:
            stato_uscita = OPZIONI;
            break;
        case ESCI:
            stato_uscita = ESCI;
            break;
    }
    delwin(menuwin);
    return stato_uscita;
}


/**
 * Funzione per cambiare alcuni parametri del gioco, come la risoluzione, il numero di nemici e la velocità di missili
 * e bombe. Nel caso in cui venga cambiata la risoluzione, viene cambiato automaticamente anche il numero di nemici.
 * @return Stato di uscita dalla funzione, ossia il contesto che deve essere aperto una volta usciti dalle opzioni
 */
StatoCorrente opzioni() {
    /// Dichiarazione e inizializzazione variabili
    int i;                      // Variabile contatore
    StatoCorrente stato_uscita; // Variabile da restituire
    char parametro[25];         // Per cambiare la risoluzione
    Risoluzioni risoluzione;    // Contiene le diverse scelte delle risoluzioni
    int scelta;                 // Contiene la scelta selezionata
    int selezione = 0;          // Indica in quale campo del menu ci si trova
    int case_nemici = (M/num_righe) -1; // Contiene le diverse scelte dei numeri di nemici

    /// Array di stringhe da stampare nelle opzioni
    char *scelte[NUMERO_OPZIONI] = {"⬅ Indietro", "Nemici", "Risoluzione", "Missili"};
    char *resolutions[NUMERO_RISOLUZIONI] = {"80  x 24", "100 x 30 (default)","100 x 45","150 x 45", "170 x 51"};
    char *velocitaMissili[NUMERO_VELOCITA_MISSILI] = {"LENTO (default)", "MEDIO", "VELOCE"};

    clear();
    WINDOW *menuwin = newwin(MENUWIN_Y, MENUWIN_X, 1, (maxx - MENUWIN_X) / 2);  // Inizializzo una nuova finestra
    box(menuwin, 0, 0);
    refresh();
    wrefresh(menuwin);

    keypad(menuwin, TRUE);      // Abilitazione dei tasti freccia

    // Array di stringhe dinamico per adattare la stampa del numero di nemici (che cambia)
    char **enemy_count = (char**) malloc(sizeof(char*) * NUMERO_SELEZIONI_NEMICI);
    for (i = 0; i < NUMERO_SELEZIONI_NEMICI; ++i) {
        enemy_count[i] = (char*) malloc(sizeof(char)*20);
        if (i == 3)
            sprintf(enemy_count[i], "%d (default)", (i + 1)*num_righe);
        else
            sprintf(enemy_count[i], "%d", (i + 1)*num_righe);
    }

    while (TRUE) {
        wmove(menuwin, 0, 1);
        for (i = 0; i < NUMERO_OPZIONI; ++i) {  // stampa campi opzione
            if (i == selezione)                 // se ci troviamo nel campo attualmente selezionato
                wattron(menuwin, A_REVERSE);    // il campo stampato viene colorato
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
        system("aplay -q Suoni/menu.wav &");
        for (i = 0; i < MENUWIN_Y - 2; ++i) {
            mvwprintw(menuwin, i + 1, 1, "                                                                ");
        }
        switch (selezione) {    // Stampa descrizione campi opzione
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
        case 0: // caso indietro
            stato_uscita = MENU; // torniamo al menu
            break;
        case 1: // caso scelta numero nemici
            case_nemici = sottomenu(menuwin, enemy_count, NUMERO_SELEZIONI_NEMICI); // entriamo nel sottomenu per i nemici
            M = (case_nemici + 1) * num_righe;  // modifichiamo la variabile globale M
            stato_uscita = OPZIONI;             // torniamo alle opzioni
            break;
        case 2:
            risoluzione = sottomenu(menuwin, resolutions, NUMERO_RISOLUZIONI);  // entriamo nel sottomenu per cambiare la risoluzione
            switch (risoluzione) { // In base al valore restituito dal sottomenu modifichiamo la risoluzione
                case R_80_X_24:
                    maxx = 80;
                    maxy = 24;
                    num_righe = 5;      // numero di nemici in ogni colonna
                    mov_verticale = 4;  // Numero di spostamenti che i nemici effettuano prima di sbattere nel bordo
                    break;
                case R_100_X_30:
                    maxx = 100;
                    maxy = 30;
                    num_righe = 6;      // numero di nemici in ogni colonna
                    mov_verticale = 6;  // Numero di spostamenti che i nemici effettuano prima di sbattere nel bordo
                    break;
                case R_100_X_45:
                    maxx = 100;
                    maxy = 45;
                    num_righe = 9;      // numero di nemici in ogni colonna
                    mov_verticale = 9;  // Numero di spostamenti che i nemici effettuano prima di sbattere nel bordo
                    break;
                case R_150_X_45:
                    maxx = 150;
                    maxy = 45;
                    num_righe = 9;      // numero di nemici in ogni colonna
                    mov_verticale = 9;  // Numero di spostamenti che i nemici effettuano prima di sbattere nel bordo
                    break;
                case R_170_X_51:
                    maxx = 170;
                    maxy = 45;
                    num_righe = 9;      // numero di nemici in ogni colonna
                    mov_verticale = 9;  // Numero di spostamenti che i nemici effettuano prima di sbattere nel bordo
                    break;
            }
            M = (case_nemici + 1) * num_righe; // aggiorno il numero di nemici per adattarlo alla risoluzione
            sprintf(parametro, "printf '\033[8;%d;%dt'", maxy, maxx);
            system(parametro);          // Cambio la risoluzione del terminale
            resize_term(maxy, maxx);    // Cambio la risoluzione della finestra di ncurses
            usleep(100000);
            stato_uscita =  OPZIONI;    // Imposto la variabile di ritorno per tornare nelle opzioni
            break;
        case 3: // caso Missili
            switch (sottomenu(menuwin, velocitaMissili, NUMERO_VELOCITA_MISSILI)) { // cambio la velocità dei missili
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
            stato_uscita = OPZIONI;
            break;
    }
    /// Dealloco le risorse allocate dinamicamente
    for (i = 0; i < NUMERO_SELEZIONI_NEMICI; ++i) {
        free (enemy_count[i]);
    }
    free(enemy_count);
    delwin(menuwin);    // Chiudo la finestra del menu
    return stato_uscita;
}

/// Funzione per stampare le varie scelte delle varie opzioni
/// \param menuwin      finestra su cui stampare le scelte
/// \param scelte       scelte da stampare
/// \param num_scelte   numero di scelte da stampare
/// \return             scelta selezionata
int sottomenu(WINDOW* menuwin, char *scelte[], int num_scelte){
    /// Inizializzazione delle variabili
    int i;              // Variabile contatore
    int selezione = 0;  // Contiene la scelta selezionata
    int scelta;         // Indica in quale campo del menu ci si trova
    for (i = 0; i < MENUWIN_Y - 2; ++i) {// cancello le precedenti stampe
        mvwprintw(menuwin, i + 1, 1, "                                                                ");
    }

    while (TRUE) {
        wmove(menuwin, 0, 1);
        for (i = 0; i < num_scelte; ++i) {  // stampa le scelte
            if (i == selezione)             // se ci troviamo nel campo attualmente selezionato
                wattron(menuwin, A_REVERSE);// il campo stampato viene colorato
            mvwprintw(menuwin,i+1,1, "%s", scelte[i]);
            wattroff(menuwin, A_REVERSE);
        }
        scelta = wgetch(menuwin);
        switch (scelta) {   // movimento tra le varie scelte
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
        system("aplay -q Suoni/menu.wav &");
        if (scelta == 10) //caso enter
            break;
    }
    for (i = 0; i < MENUWIN_Y - 2; ++i) { // cancellazione delle scelte stampate
        mvwprintw(menuwin, i + 1, 1, "                              ");
    }
    return selezione; // Restituisco la scelta selezionata
}