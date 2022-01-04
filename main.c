#include "menu.h"

#define KEY_SPACE 32
#define DIM_NAVICELLA 6
#define DIM_NEMICO 3
#define NAVE 0  // identificatore posizione navicella
#define MISSILE_1 1 // identificatore posizione Missile 1
#define MISSILE_2 2 // identificatore posizione Missile 2
#define MAX_MISSILI 10 // Numero (pari) massimo di missili sparabili contemporaneamente




const Oggetto init = {0, 0, {0, 0}, {0, 0}, 0, 0}; //Costante utilizzata per inizializzare la struttura OGGETTO
void nave_player(int pipeout);
_Bool AreaGioco(int *pipe, int pipe_from_navicella, int* pipe_to_nemici, Oggetto *enemies);
void Enemy(int pipeout,Oggetto *enemy);
void missili(int pipeout, Oggetto *missile);
void bombe(int pipeout, Oggetto *bomba);




void collisione_missili_bombe(Oggetto *missili, Oggetto *bombe_nem);
void collisione_bombe_navicella(Oggetto *navicella, Oggetto *bombe_nem);
void collisione_missili_nemici(Oggetto *enemies, Oggetto *missili, _Bool *primafila);

int M = 24; //Numero nemici
int num_righe = 6; //numero di nemici in ogni colonna
int mov_verticale = 6;
int velocita_missili = 100000;




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
char *nemico_lv3[DIM_NEMICO]= {" ☠ ",
                               "☠ ☠", ///○ ◙ ❍ ◚
                               " ☠ "};

char *vittoria[6]={"██╗  ██╗ █████╗ ██╗ ██╗   ██╗██╗███╗  ██╗████████╗ █████╗ ",
                   "██║  ██║██╔══██╗██║ ██║   ██║██║████╗ ██║╚══██╔══╝██╔══██╗",
                   "███████║███████║██║ ╚██╗ ██╔╝██║██╔██╗██║   ██║   ██║  ██║",
                   "██╔══██║██╔══██║██║  ╚████╔╝ ██║██║╚████║   ██║   ██║  ██║",
                   "██║  ██║██║  ██║██║   ╚██╔╝  ██║██║ ╚███║   ██║   ╚█████╔╝",
                   "╚═╝  ╚═╝╚═╝  ╚═╝╚═╝    ╚═╝   ╚═╝╚═╝  ╚══╝   ╚═╝    ╚════╝ "};

char *gameover[6]= {" ██████╗  █████╗ ███╗   ███╗███████╗ █████╗ ██╗   ██╗███████╗██████╗ ",
                    "██╔════╝ ██╔══██╗████╗ ████║██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗",
                    "██║  ██╗ ███████║██╔████╔██║█████╗  ██║  ██║╚██╗ ██╔╝█████╗  ██████╔╝",
                    "██║  ╚██╗██╔══██║██║╚██╔╝██║██╔══╝  ██║  ██║ ╚████╔╝ ██╔══╝  ██╔══██╗",
                    "╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗╚█████╔╝  ╚██╔╝  ███████╗██║  ██║",
                    " ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝ ╚════╝    ╚═╝   ╚══════╝╚═╝  ╚═╝"};

extern int maxx;
extern int maxy;

int main() {
    StatoCorrente status;
    status = MENU;

    while (TRUE) {
        switch (status) {
            case GIOCA:
                status = gioco();
                break;
            case MENU:
                status = menu();
                break;
            case OPZIONI:
                status = opzioni();
                break;
            case ESCI:
                endwin();
                return 0;

        }
        clear();
        refresh();
    }
}

StatoCorrente gioco() {
    int i, l, n;
    int fd1[2];
    int pipe_from_navicella[2];
    int *pipe_enemies = (int *)malloc(M * 2 * sizeof(int));
    int pid_navicella;
    _Bool WIN;
    l = 0, n = 0;
    pid_t *pid_nemici = (pid_t *) calloc(M, sizeof(pid_t));
    Oggetto *enemies = NULL;
    signal(SIGCHLD, SIG_IGN);
    setlocale(LC_ALL, "");
    initscr(); /* inizializzazione dello schermo */
    noecho(); /* i caratteri corrispondenti ai tasti premuti non saranno visualizzati sullo schermo del terminale */
    srand(time(NULL)); /* inizializziamo il generatore di numeri random */
    curs_set(0); /* nasconde il cursore */


    start_color();
    init_pair(0, COLOR_BLACK, COLOR_BLACK); // per cancellare
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // per scrivere
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
    init_pair(7, COLOR_RED, COLOR_BLACK);

    bkgd(COLOR_PAIR(1));

    if (pipe(fd1) == -1) { //nemici, navicella -> area gioco
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }

    if (pipe(pipe_from_navicella) == -1) { // navicella -> area gioco
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (fcntl(pipe_from_navicella[0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);


    pid_navicella = fork(); //generazione di un secondo processo figlio per la nave_player
    switch (pid_navicella) {
        case -1:
            perror("Errore nell'esecuzione della fork.");
            _exit(1);
        case 0:
            prctl(PR_SET_NAME, (unsigned long) "Navicella");
            close(pipe_from_navicella[0]); /* chiusura del descrittore di lettura (standard input)*/
            nave_player(pipe_from_navicella[1]); /* il secondo processo figlio invoca la funzione nave_player passandogli la pipe in scrittura*/
        default:    //processo padre
            enemies = (Oggetto *) calloc(M, sizeof(Oggetto));
            for (i = 0; i < M;  i++) {
                switch (pid_nemici[i] = fork()) {
                    case -1:
                        perror("Errore nell'esecuzione della fork.");
                        _exit(1);
                    case 0:
                        close(pipe_enemies[i * 2 + 1]); /* chiusura del descrittore di scrittura (standard output)*/
                        enemies[i].index = i;
                        Enemy(fd1[1], &enemies[i]);
                        break;
                    default:
                        continue;
                }
            }
            prctl(PR_SET_NAME, (unsigned long) "Area_gioco");
            close(pipe_from_navicella[1]); /* chiusura del descrittore di lettura (standard input)*/
            WIN = AreaGioco(fd1, pipe_from_navicella[0], pipe_enemies, enemies); /* il processo padre invoca la funzione di AreaGioco */
    }
    /* siamo usciti dalla funzione di AreaGioco e vengono terminati i 2 processi figli e ripristinato il normale modo operativo dello schermo */
    kill(pid_navicella, 1);
    if(WIN){
        clear();
        for (i = 0; i < 6; ++i) {
            mvprintw((maxy- 5)/2 + i, (maxx-58)/2, vittoria[i]);
        }
        refresh();
    }else {
        clear();
        for (i = 0; i < 5; ++i) {
            mvprintw((maxy- 6)/2 + i, (maxx-69)/2, gameover[i]);
        }
        refresh();
    }
    sleep(5);
    endwin();
    free(enemies);
    free(pipe_enemies);
    free(pid_nemici);
    return MENU;
}

void Enemy(int pipeout,Oggetto *enemy){
    /// Inizializzazione variabili
    int j = 0, l = 0;   // Variabili contatori spostamento
    int temp;           // per il calcolo della posizione
    int status = 0;     // Per cambiare la direzione dello spostamento
    char nome[10];

    /// Diamo un nome al processo
    sprintf(nome, "Nemico_%d", enemy->index);
    prctl(PR_SET_NAME, (unsigned long) nome);

    /// Inizializzazione parziale dei valori della navicella nemica
    enemy->vite = 3;
    enemy->old_pos.x = -1;
    enemy->old_pos.y = -1;
    enemy->id = ID_NEMICO;
    enemy->pid = getpid();

    while (true) {
        enemy->pos.y = (1 + (enemy->index % num_righe) * (DIM_NEMICO + 1)) + j;
        temp = (int) (enemy->index / num_righe) + 1;
        enemy->pos.x = maxx - temp * (DIM_NEMICO + 1) - l;
        write(pipeout, enemy, sizeof(Oggetto));
        /*Spostamenti navicelle*/
        if (j % 8 == 0) {
            l++;
        }
        usleep(300000); //velocità spostamento nemici
        if (j <= mov_verticale && status == 0) {
            j++;
            if (j == mov_verticale)
                status = 1;
        } else if (j >= 0 && status == 1) {
            j--;
            if (j == 0)
                status = 0;
        }
        enemy->old_pos = enemy->pos;
    }
}

//void* nave_player(void *arg) {
//    /// Definisce l'argomento passato in input alla funzione
//    arg_to_navicella *argToNavicella;
//    argToNavicella = (arg_to_navicella *) arg;
//    /** Dichiarazione strutture temporanee per aggiornare i valori dopo lo spostamento e per
//     * abilitare lo sparo dei missili **/
//    Oggetto *navicella = argToNavicella->navicella;
//

//
//    keypad(stdscr, TRUE); // abilita l'input da tastiera
//    while (TRUE) {
//        int c = getch();
//        // Controlla il tasto premuto dal giocatore
//        switch (c) {
//            /// se ha premuto la freccia verso su
//            case KEY_UP:
//                pthread_mutex_lock(&mtx_nave); ///blocca il mutex @mtx_nave
//                /* impedisce alla nave di sovrascrivere la prima riga dello schermo */
//                if (navicella->pos.y > 1)
//                    navicella->pos.y -= 1; // decrementa le y della nave facendola spostare verso l'alto
//                pthread_mutex_unlock(&mtx_nave); ///sblocca il mutex @mtx_nave
//                break;
//                /// se ha premuto la freccia verso giù
//            case KEY_DOWN:
//                /* impedisce alla nave di uscire dalla parte inferiore dello schermo */
//                pthread_mutex_lock(&mtx_nave); ///blocca il mutex @mtx_nave
//                if (navicella->pos.y < maxy - DIM_NAVICELLA)
//                    navicella->pos.y += 1; // incrementa le y della nave facendola spostare verso il basso
//                pthread_mutex_unlock(&mtx_nave); ///sblocca il mutex @mtx_nave
//                break;
//                /// se ha premuto la barra spaziatrice
//            case KEY_SPACE:
//                pthread_mutex_lock(&mtx_nave); ///blocca il mutex @mtx_nave
//                *sparo = true; // abilita lo sparo
//                pthread_mutex_unlock(&mtx_nave); ///sblocca il mutex @mtx_nave
//                break;
//        }
//    }
//}
void nave_player(int pipeout) {
    arg_from_navicella argFromNavicella;

    argFromNavicella.navicella.id = ID_NAVICELLA;
    argFromNavicella.navicella.pos.x = 1;
    argFromNavicella.navicella.pos.y = (maxy - DIM_NAVICELLA) / 2;
    argFromNavicella.navicella.vite = 3;
    argFromNavicella.navicella.index = 0;
    argFromNavicella.navicella.pid = getpid();
    argFromNavicella.navicella.old_pos.x = -1;
    argFromNavicella.navicella.old_pos.y = -1;

    argFromNavicella.sparo = FALSE;

    write(pipeout, &argFromNavicella, sizeof(argFromNavicella));

//    nodelay(stdscr, 1);
//    timeout(3);
    keypad(stdscr, TRUE);
    while (TRUE) {
        int c = getch();
        switch (c) {
            case KEY_UP:
                if (argFromNavicella.navicella.pos.y > 1)
                    argFromNavicella.navicella.pos.y -= 1;
                break;
            case KEY_DOWN:
                if (argFromNavicella.navicella.pos.y < maxy - DIM_NAVICELLA)
                    argFromNavicella.navicella.pos.y += 1;
                break;
            case KEY_SPACE:
                argFromNavicella.sparo = TRUE;
                break;
        }
        write(pipeout, &argFromNavicella, sizeof(argFromNavicella));
        argFromNavicella.sparo = FALSE;
        argFromNavicella.navicella.old_pos = argFromNavicella.navicella.pos;
    }
}

// if (num_missili <= MAX_MISSILI - 2) {
//                    num_missili += 2;
//                    missile1.pos.y = navicella.pos.y + (DIM_NAVICELLA / 2);
//                    missile1.pos.x = navicella.pos.x + DIM_NAVICELLA;
//                    missile2.pos.y = navicella.pos.y + (DIM_NAVICELLA / 2);
//                    missile2.pos.x = navicella.pos.x + DIM_NAVICELLA;
//
//                    missile1.id = ID_MISSILE;
//                    missile1.index = num_missili - 2;
//                    missile2.id = ID_MISSILE;
//                    missile2.index = num_missili - 1;
//
//                    i = 0;
//
//                    }
//                }
//                break;
//for (i = 0, j = 0; i < MAX_MISSILI; i++) {
//            if (array_missili[i].vite == 0) {
//                j++;
//            }
//  if (num_missili == MAX_MISSILI && j == MAX_MISSILI) {
//            num_missili = 0;
//        }


void missili(int pipeout, Oggetto *missile){
    int i = 0;
    while (TRUE) {
        if (i % 20 == 0) {
            if (missile->id == ID_MISSILE2) {
                missile->pos.y += 1;
            } else if (missile->id == ID_MISSILE1) {
                missile->pos.y -= 1;
            }
        }
        missile->pos.x += 1;
        write(pipeout, missile, sizeof(Oggetto));
        i++;
        if ((missile->pos.x > maxx || missile->pos.y >= maxy || missile->pos.y < 0)) {
            missile->vite = 0;
            write(pipeout, missile, sizeof(Oggetto));
            exit(1);
        }
        missile->old_pos = missile->pos;
        usleep(velocita_missili); //regola velocità missili
    }
}

void bombe(int pipeout, Oggetto *bomba) {
    char nome[10];
    sprintf(nome, "Bomba %d", bomba->index);
    prctl(PR_SET_NAME, (unsigned long) nome);
    while (TRUE) {
        bomba->pos.x -= 1;
        write(pipeout, bomba, sizeof(Oggetto));
        if (bomba->pos.x < 0 || bomba->pos.y >= maxy || bomba->pos.y < 0) {
            *bomba = init;
            write(pipeout, bomba, sizeof(Oggetto));
            exit(1);
        }
        bomba->old_pos = bomba->pos;
        usleep(velocita_missili); //regola velocità missili
    }
}


/**
 * Funzione per gestire la stampa, le collisioni e la coordinazione di tutti i figli
 * @param pipein pipe per ricevere i valori dai figli
 * @param pipe_to_navicella pipe per inviare i valori alla navicella player
 * @param pipe_to_nemici
 * @param enemies
 * @return Il valore vittoria, vero se si vince e falso se si perde
 */
_Bool AreaGioco(int *pipe, int pipe_from_navicella,int *pipe_to_nemici, Oggetto *enemies) {
    clear();
    int i, nemici_vivi;
    _Bool collision = false, WIN = false;
    Oggetto navicella, valore_letto = init;
    int missili_vivi = 0;
    int missili_morti = 0;
    Oggetto rockets[MAX_MISSILI];
    Oggetto *bombe_nem = (Oggetto *) calloc(M, sizeof(Oggetto));
    InfoToNemici *message_to_nemici = (InfoToNemici *) calloc(M, sizeof(InfoToNemici));
    navicella.pos.x = -1;
    navicella.vite = 3;
    arg_from_navicella argFromNavicella;
    for (i = 0; i < MAX_MISSILI; i++) {
        rockets[i] = init;
    }

    _Bool *primafila = (_Bool *) calloc(M, sizeof(_Bool));


    /// Inizializzazione primafila
    // Primafila è utilizzato per garantire che siano solo i nemici della prima colonna a sinistra a sparare e non
    // quelli che stanno dietro. Successivamente, appena morirà un nemico della prima fila, a quello dietro verrà
    // impostato il suo valore primafila a true.
    for (i = 0; i < M; ++i) {
        if (i >= (((M / num_righe) - 1) * num_righe) && i < M) {    // Se è in prima fila
            primafila[i] = TRUE;
        } else {
            primafila[i] = FALSE;
        }
    }

    for (i = 0; i < M; ++i) {
        enemies[i].vite = 3;
    }
    do {
        /// Lettura da tutti i processi figli
        read(pipe[0], &valore_letto, sizeof(valore_letto)); /* leggo dalla pipe */
        read(pipe_from_navicella, &argFromNavicella, sizeof(arg_from_navicella));
        stampaNavicella(argFromNavicella.navicella);

        if(argFromNavicella.sparo && missili_vivi < MAX_MISSILI) {
            for (int temp = missili_vivi + 2; missili_vivi < temp; missili_vivi++) {
                rockets[missili_vivi].pid = fork(); //generazione processo
                switch (rockets[missili_vivi].pid) {
                    case -1:
                        perror("Errore nell'esecuzione della fork.");
                        exit(1);
                    case 0:
                        prctl(PR_SET_NAME, (unsigned long) "Missile 1");
                        close(pipe[0]);
                        if (missili_vivi % 2 == 0)
                            rockets[missili_vivi].id = ID_MISSILE1;
                        else
                            rockets[missili_vivi].id = ID_MISSILE2;
                        rockets[missili_vivi].pos.x = argFromNavicella.navicella.pos.x + DIM_NAVICELLA;
                        rockets[missili_vivi].pos.y = argFromNavicella.navicella.pos.y + (DIM_NAVICELLA / 2);
                        rockets[missili_vivi].old_pos.x = -1;
                        rockets[missili_vivi].old_pos.y = -1;
                        rockets[missili_vivi].vite = 1;
                        rockets[missili_vivi].index = missili_vivi;
                        rockets[missili_vivi].pid = getpid();
                        missili(pipe[1], &rockets[missili_vivi]);
                        break;
                    default: //processo padre
                        argFromNavicella.sparo = false;
                        break;
                }
            }
        }

        /// Stampa dei vari oggetti
        switch (valore_letto.id) {
//            case ID_NAVICELLA:
//                stampaNavicella(valore_letto);
//                navicella.pos = valore_letto.pos;
//                navicella.old_pos = valore_letto.old_pos;
//                navicella.index = valore_letto.index;
//                navicella.pid = valore_letto.pid;
//                navicella.id = valore_letto.id;
//                // if sparo = true
//                // switch
//                // figlio missili (pos navicella )
//                // caso genitore break
//                break;
            case ID_NEMICO:
                stampaNemico(valore_letto, enemies[valore_letto.index].vite);
                if (enemies[valore_letto.index].vite > 0) {
                    enemies[valore_letto.index].pos = valore_letto.pos;
                    enemies[valore_letto.index].old_pos = valore_letto.old_pos;
                    enemies[valore_letto.index].index = valore_letto.index;
                    enemies[valore_letto.index].pid = valore_letto.pid;
                    enemies[valore_letto.index].id = valore_letto.id;
                } else if (enemies[valore_letto.index].vite == 0) {
                    enemies[valore_letto.index] = init;
                }
                break;
            case ID_MISSILE1:
                stampaMissile(&valore_letto);
                rockets[valore_letto.index] = valore_letto;
                break;
            case ID_MISSILE2:
                stampaMissile(&valore_letto);
                rockets[valore_letto.index] = valore_letto;
                break;
            case ID_BOMBA:
                stampaBomba(&valore_letto);
                bombe_nem[valore_letto.index] = valore_letto;
                break;
        }

        for (i = 0, missili_morti = 0; i < missili_vivi; ++i) {
            if (rockets[i].vite == 0) {     // se il missile è morto
                missili_morti++;
            }
        }
        if (missili_morti == MAX_MISSILI) { // quando muore tutta l'ondata di missili si potrà tornare a sparare
            missili_vivi = 0;
        }
        /// generazione bomba
        for (i = 0; i < M; ++i) {
            if (primafila[i] == TRUE) {         // se siamo in primafila
                if (bombe_nem[i].vite == 0) { // se la bomba sparata precedentemente è morta
                    /// Inizializzo parzialmente la nuova bomba
                    switch (bombe_nem[i].pid = fork()) {
                        case -1:
                            perror("Errore nell'esecuzione della fork.");
                            exit(1);
                        case 0:
                            close(pipe[0]);
                            bombe_nem[i].vite = 1;
                            bombe_nem[i].index = i;
                            bombe_nem[i].id = ID_BOMBA;
                            bombe_nem[i].old_pos.x = -1;
                            bombe_nem[i].old_pos.y = -1;
                            bombe_nem[i].pid = getpid();
//                            if (bombe_nem[i].index % 4 == 0) {
//                                sleep(1);
//                            } else if (bombe_nem[i].index % 2 == 1) {
//                                sleep(3);
//                            } else if (bombe_nem[i].index % 2 == 0) {
//                                sleep(5);
//                            }
                            bombe_nem[i].pos.x = enemies[i].pos.x - 1;
                            bombe_nem[i].pos.y = enemies[i].pos.y + (DIM_NEMICO / 2);
                            bombe(pipe[1], &bombe_nem[i]);
                            break;
                        default: //processo padre
                            break;
                    }
                }
            }
        }

        /// Controllo delle varie collisioni
//        collisione_missili_bombe(rockets, bombe_nem);
//        collisione_bombe_navicella(&navicella, bombe_nem);
        collisione_missili_nemici(enemies, rockets, primafila);

        /// Collisione tra navicella e nemici
        for (i = 0; i < M; i++) {
            if (enemies[i].pos.x == DIM_NAVICELLA + 2) {
                collision = TRUE;
                WIN = FALSE;
                break;
            }
        }
        ///Controllo vite del player
        if (navicella.vite == 0) {
            WIN = FALSE;
            collision = TRUE;
        }
        ///Controllo vite nemici
        nemici_vivi = 0;
        for (i = 0; i < M; ++i) {
            if (enemies[i].vite != 0) {
                nemici_vivi++;
            }
        }
        if (nemici_vivi == 0) {
            WIN = TRUE;
            collision = true;
        }
        move(0, 0);
        clrtoeol();
        mvprintw(0, 0, "VITE: %d, nemici: %d", navicella.vite, nemici_vivi);
        refresh();
    } while (!collision);

    for (i = 0; i < M; ++i) {
        if(enemies[i].pid != 0){
            kill(enemies[i].pid, SIGKILL);
        }
    }
    free(bombe_nem);
    free(message_to_nemici);
    return WIN;
}

void stampaNavicella(Oggetto navicella) {
    int i;
    /* cancello la 'vecchia' posizione della navicella */
    if (navicella.old_pos.x >= 0) {
        attron(COLOR_PAIR(0));
        for (i = 0; i < DIM_NAVICELLA; i++) {
            mvaddstr(navicella.old_pos.y + i, navicella.old_pos.x, "      ");
        }
        attroff(COLOR_PAIR(0));
    }
    attron(COLOR_PAIR(2));
    for (i = 0; i < DIM_NAVICELLA; i++) {
        if (i <= 1 || i >= 4) {
            attron(COLOR_PAIR(6));
            mvaddstr(navicella.pos.y + i, navicella.pos.x, nave[i]);
            attroff(COLOR_PAIR(6));
        } else {
            attron(COLOR_PAIR(2));
            mvaddstr(navicella.pos.y + i, navicella.pos.x, nave[i]);
            attroff(COLOR_PAIR(2));
        }
    }
}
void stampaNemico(Oggetto nemico, int vite){
    int i;
    if (nemico.pid != 0) {
        attron(COLOR_PAIR(0));
        for (i = 0; i < DIM_NEMICO; i++) {
            mvprintw(nemico.old_pos.y + i, nemico.old_pos.x, "   ");
        }
        attroff(COLOR_PAIR(0));
    }
    switch (vite) {
        case 3:
            for (i = 0; i < DIM_NEMICO; i++) {
                if (i == 1) {
                    attron(COLOR_PAIR(4));
                    mvprintw(nemico.pos.y + i, nemico.pos.x, nemico_lv1[i]);
                    attroff(COLOR_PAIR(4));
                } else {
                    attron(COLOR_PAIR(7));
                    mvprintw(nemico.pos.y + i, nemico.pos.x, nemico_lv1[i]);
                    attroff(COLOR_PAIR(7));
                }
            }
            break;
        case 2:
            attron(COLOR_PAIR(5));
            for (i = 0; i < DIM_NEMICO; i++) {
                mvprintw(nemico.pos.y + i, nemico.pos.x, nemico_lv2[i]);
            }
            attroff(COLOR_PAIR(1));
            break;
        case 1:
            attron(COLOR_PAIR(5));
            for (i = 0; i < DIM_NEMICO; i++) {
                mvprintw(nemico.pos.y + i, nemico.pos.x, nemico_lv3[i]);
            }
            attroff(COLOR_PAIR(1));
            break;
        case 0:
            attron(COLOR_PAIR(0));
            for (i = 0; i < DIM_NEMICO; i++) {
                mvprintw(nemico.pos.y + i, nemico.pos.x, "   ");
            }
            attroff(COLOR_PAIR(0));
            break;
    }
}
void stampaBomba(Oggetto *bomba) {
    // Se la bomba è all'interno dalla finestra di gioco
    if (bomba->pos.x != -1 && bomba->pos.y != -1) {
        attron(COLOR_PAIR(0));
        mvprintw(bomba->old_pos.y, bomba->old_pos.x, " "); // cancella la stampa dell'ultima posizione della bomba
        attroff(COLOR_PAIR(0));
    }
    /* Stampa la bomba nella posizione corrente*/
    attron(COLOR_PAIR(7));
    mvprintw(bomba->pos.y, bomba->pos.x, "⁂"); ///♿ ⟢ ⁂ ꗇ ꗈ 💣 🚀 卐 ◌́ ◌͂ ✝
    attroff(COLOR_PAIR(7));
}
/**
 * La funzione @stampaMissile stampa i missili nella posizione corrente e cancella dallo schermo
 *  la stampa relativa alla posizione in cui si trovavano precedentemente. Il suo parametro in ingresso è:
 * @param missile - È una struttura @Oggetto che contiene tutti i dati del singolo missile generato dalla
 *                  navicella del giocatore.
 */
void stampaMissile(Oggetto *missile) {
    /* Se il missile è all'interno dalla finestra di gioco*/
    if (missile->pos.x != -1 && missile->pos.y != -1) {
        attron(COLOR_PAIR(0));
        mvprintw(missile->old_pos.y, missile->old_pos.x, " ");
        attroff(COLOR_PAIR(0));
    }
    attron(COLOR_PAIR(3));
    mvprintw(missile->pos.y, missile->pos.x, "⟢"); ///♿ ⟢ ⁂ ꗇ ꗈ 💣 🚀 卐 ◌́ ◌͂ ✝
    attroff(COLOR_PAIR(3));
}
void collisione_missili_bombe(Oggetto *missili, Oggetto *bombe_nem){
    int i, j;
    /// collissione tra missili e bombe
    for (i = 0; i < MAX_MISSILI; i++) {
        if (missili[i].pid != 0) {
            for (j = 0; j < M; j++) {
                if (bombe_nem[j].pid != 0) {
                    if (missili[i].pos.x == bombe_nem[j].pos.x && missili[i].pos.y == bombe_nem[j].pos.y){
                        //uccido il missile colliso
                        kill(missili[i].pid, SIGKILL);
                        missili[i].vite = 0;
                        //uccido la bomba collisa
                        kill(bombe_nem[j].pid, SIGKILL);
                        bombe_nem[j].vite = 0;
                        attron(COLOR_PAIR(0));
                        mvaddstr(missili[i].old_pos.y, missili[i].old_pos.x, " ");
                        mvaddstr(missili[i].pos.y, missili[i].pos.x, " ");
                        mvaddstr(bombe_nem[j].pos.y, bombe_nem[j].pos.x, " ");
                        attroff(COLOR_PAIR(0));
                        missili[i] = init;
                        bombe_nem[j] = init;
                    }
                }
            }
        }
    }
}

void collisione_bombe_navicella(Oggetto *navicella, Oggetto *bombe_nem) {
    int i, j, k;
    ///collisione tra bombe e navicella player
    for (i = 0; i < M; i++) {
        if (bombe_nem[i].pid != 0) {
            for (j = 0; j < DIM_NAVICELLA; ++j) {
                for (k = 0; k < DIM_NAVICELLA; ++k) {
                    if (j > 0 && j < 5) {
                        k = 5;
                    }
                    if (bombe_nem[i].pos.x == navicella->pos.x + k && bombe_nem[i].pos.y == navicella->pos.y + j) {
//                          uccido la bomba collisa
                        kill(bombe_nem[i].pid, SIGKILL);
                        beep();
                        navicella->vite--;
                        attron(COLOR_PAIR(0));
                        mvaddstr(bombe_nem[i].pos.y, bombe_nem[i].pos.x, " ");
                        attroff(COLOR_PAIR(0));
                        bombe_nem[i] = init;
                    }
                }
            }
        }
    }
}
void collisione_missili_nemici(Oggetto *enemies, Oggetto *missili, _Bool *primafila) {
    int i, j, k, l;
    ///collisione tra Missili e Nemici
    for (i = 0; i < M; i++) {
        if (enemies[i].pid != 0) {
            for (j = 0; j < MAX_MISSILI; j++) {
                if (missili[j].pid != 0) {
                    for (k = 0; k < DIM_NEMICO; ++k) {
                        for (l = 0; l < DIM_NEMICO; ++l) {
                            if (missili[j].pos.x == enemies[i].pos.x + l && missili[j].pos.y == enemies[i].pos.y + k) {
                                kill(missili[j].pid, SIGKILL);
                                missili[j].vite = 0;

                                enemies[i].vite -= 1;
                                if ((i - num_righe) >= 0 && enemies[i].vite == 0){
                                    primafila[i-num_righe] = TRUE;
                                }
                                attron(COLOR_PAIR(0));
                                mvaddstr(missili[j].pos.y, missili[j].pos.x, " ");
                                attroff(COLOR_PAIR(0));
                                missili[j] = init;
                                if (enemies[i].vite == 0){
                                    enemies[i] = init;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}