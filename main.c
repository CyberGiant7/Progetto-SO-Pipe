#include "menu.h"
#include "collisioni.h"
#include "stampa.h"
#include "generazione_e_movimento.h"

#define KEY_SPACE 32
#define DIM_NAVICELLA 6
#define DIM_NEMICO 3
#define NAVE 0  // identificatore posizione navicella
#define MISSILE_1 1 // identificatore posizione Missile 1
#define MISSILE_2 2 // identificatore posizione Missile 2
#define MAX_MISSILI 10 // Numero (pari) massimo di missili sparabili contemporaneamente






_Bool AreaGioco(int *pipe1, int pipe_from_navicella, Oggetto *enemies);

int M = 24; //Numero nemici
int num_righe = 6; //numero di nemici in ogni colonna
int mov_verticale = 6;
int velocita_missili = 100000;
const Oggetto init = {0, 0, {0, 0}, {0, 0}, 0, 0}; //Costante utilizzata per inizializzare la struttura OGGETTO


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
            WIN = AreaGioco(fd1, pipe_from_navicella[0], enemies); /* il processo padre invoca la funzione di AreaGioco */
    }
    /* siamo usciti dalla funzione di AreaGioco e vengono terminati i 2 processi figli e ripristinato il normale modo operativo dello schermo */
    kill(pid_navicella, 1);

    if (WIN) {
        clear();
        for (i = 0; i < 6; ++i) {
            mvprintw((maxy - 5) / 2 + i, (maxx - 58) / 2, vittoria[i]);     // Stampa vittoria
        }
    } else {
        clear();
        for (i = 0; i < 6; ++i) {
            mvprintw((maxy - 6) / 2 + i, (maxx - 69) / 2, gameover[i]);     // Stampa game over
        }
    }
    sleep(5);
    endwin();
    free(enemies);
    free(pipe_enemies);
    free(pid_nemici);
    return MENU;
}

_Bool AreaGioco(int *pipe1, int pipe_from_navicella, Oggetto *enemies) {
    clear();
    int i, nemici_vivi;
    _Bool collision = false, WIN = false;
    Oggetto valore_letto = init;
    int missili_vivi = 0;
    int missili_morti = 0;
    Oggetto rockets[MAX_MISSILI], navicella_temp;
    Oggetto *bombe_nem = (Oggetto *) calloc(M, sizeof(Oggetto));
    InfoToNemici *message_to_nemici = (InfoToNemici *) calloc(M, sizeof(InfoToNemici));
    arg_from_navicella argFromNavicella;
    int count = 0;

    int *pipe_to_bombe = (int *)malloc(M * 2 * sizeof(int));
    for (i = 0; i < M; i++) {
        if (pipe(&pipe_to_bombe[i * 2 + 0]) == -1) { //area gioco -> nemici
            perror("Errore nella creazione della pipe!");
            _exit(1);
        }
    }

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
    navicella_temp.vite=3;
    do {
        /// Lettura da tutti i processi figli
        read(pipe1[0], &valore_letto, sizeof(valore_letto)); /* leggo dalla pipe */
        read(pipe_from_navicella, &argFromNavicella, sizeof(arg_from_navicella));
        navicella_temp.pos = argFromNavicella.navicella.pos;
        navicella_temp.old_pos = argFromNavicella.navicella.old_pos;
        navicella_temp.index = argFromNavicella.navicella.index;
        navicella_temp.pid = argFromNavicella.navicella.pid;
        navicella_temp.id = argFromNavicella.navicella.id;
        stampaNavicella(argFromNavicella.navicella);



        if(argFromNavicella.sparo && missili_vivi < MAX_MISSILI) {
            genera_missili(&missili_vivi, rockets, pipe1, &argFromNavicella);
        }

        /// Stampa dei vari oggetti
        switch (valore_letto.id) {
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
                if (rockets[valore_letto.index].vite != 0) {
                    stampaMissile(&valore_letto);
                }
                if(valore_letto.vite == 0){
                    rockets[valore_letto.index] = init;
                } else{
                    rockets[valore_letto.index].pos = valore_letto.pos;
                    rockets[valore_letto.index].old_pos = valore_letto.old_pos;
                    rockets[valore_letto.index].index = valore_letto.index;
                    rockets[valore_letto.index].pid = valore_letto.pid;
                    rockets[valore_letto.index].id = valore_letto.id;
                }
                break;
            case ID_MISSILE2:
                if (rockets[valore_letto.index].vite != 0) {
                    stampaMissile(&valore_letto);
                }
                if(valore_letto.vite == 0){
                    rockets[valore_letto.index] = init;
                } else{
                    rockets[valore_letto.index].pos = valore_letto.pos;
                    rockets[valore_letto.index].old_pos = valore_letto.old_pos;
                    rockets[valore_letto.index].index = valore_letto.index;
                    rockets[valore_letto.index].pid = valore_letto.pid;
                    rockets[valore_letto.index].id = valore_letto.id;
                }
                break;
            case ID_BOMBA:
                if(valore_letto.pos.y == 0 && valore_letto.pos.x == 0){
                    write(pipe_to_bombe[valore_letto.index * 2 + 1], &enemies[valore_letto.index], sizeof(Oggetto));
                    count++;
                }
                if (bombe_nem[valore_letto.index].vite != 0 && bombe_nem[valore_letto.index].pos.y != -1) {
                    stampaBomba(&valore_letto);
                }
                bombe_nem[valore_letto.index].pos = valore_letto.pos;
                bombe_nem[valore_letto.index].old_pos = valore_letto.old_pos;
                bombe_nem[valore_letto.index].index = valore_letto.index;
                bombe_nem[valore_letto.index].pid = valore_letto.pid;
                bombe_nem[valore_letto.index].id = valore_letto.id;

                if (bombe_nem[valore_letto.index].pos.x < 0) {
                    bombe_nem[valore_letto.index].vite = 0;
                    kill(bombe_nem[valore_letto.index].pid, SIGKILL);
                }
                if (bombe_nem[valore_letto.index].vite == 0){
                    bombe_nem[valore_letto.index] = init;
                }
                break;
        }

        /// generazione bomba

        for (i = 0; i < M; ++i) {
            if (primafila[i] == TRUE) {         // se siamo in primafila
                genera_bomba(bombe_nem, i, enemies, pipe1, pipe_to_bombe);
            }
        }


        for (i = 0, missili_morti = 0; i < MAX_MISSILI; ++i) {
            if (rockets[i].vite == 0) {     // se il missile è morto
                missili_morti++;
            }
        }
        if (missili_morti == MAX_MISSILI) { // quando muore tutta l'ondata di missili si potrà tornare a sparare
            missili_vivi = 0;
        }


        /// Controllo delle varie collisioni
        collisione_missili_bombe(rockets, bombe_nem);
        collisione_bombe_navicella(&navicella_temp, bombe_nem);
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
        if (navicella_temp.vite == 0) {
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
        mvprintw(0, 0, "VITE: %d, nemici: %d, count %d", navicella_temp.vite, nemici_vivi, count);
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


