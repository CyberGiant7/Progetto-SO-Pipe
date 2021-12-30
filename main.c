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



const Oggetto init = {0, 0, {0, 0}, {0, 0}, 0, 0}; //Costante utilizzata per inizializzare la struttura OGGETTO
_Noreturn void nave_player(int pipeout, int pipein);
_Bool AreaGioco(int pipein, int pipe_to_navicella, int* pipe_to_nemici, Oggetto *enemies);
void Enemy(int pipein, int pipeout, Oggetto enemy);
void missili(int pipe_to_grandpa, int pipe_to_dad, int tipo, Oggetto *missile);


void stampaMiss_bomb(Oggetto bomb_miss);
void stampaNemico(Oggetto nemico, int vite);
void stampaNavicella(Oggetto navicella);

void collisione_missili_bombe(Oggetto *missili, Oggetto *bombe_nem, int pipe_to_navicella,int *pipe_to_nemici);
void collisione_bombe_navicella(Oggetto navicella, Oggetto *bombe_nem, int *pipe_to_nemici);
void collisione_missili_nemici(Oggetto *enemies, Oggetto *missili, int pipe_to_navicella, int *pipe_to_nemici);

int M; //Numero nemici


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
char *nemico_lv3[DIM_NEMICO]= {" ☠  ",
                               "☠ ☠", ///○ ◙ ❍ ◚
                               " ☠"
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
    int i, l, n;
    int fd1[2];
    int fd2[2];
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

/** Questi colori non sono supportati nella VM**/
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
    init_pair(7, COLOR_RED, COLOR_BLACK);

    bkgd(COLOR_PAIR(1));

    if (pipe(fd1) == -1) { //nemici, navicella -> area gioco
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (pipe(fd2) == -1) { //area gioco -> navicella
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (fcntl(fd2[0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);

    for (i = 0; i < M; i++) {
        if (pipe(&pipe_enemies[i * 2 + 0]) == -1) { //area gioco -> nemici
            perror("Errore nella creazione della pipe!");
            _exit(1);
        }
        if (fcntl(pipe_enemies[i * 2 + 1], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
            exit(2);
        if (fcntl(pipe_enemies[i * 2 + 0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
            exit(2);
    }


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
            enemies = (Oggetto *) calloc(M, sizeof(Oggetto));
            for (i = 0; i < M; i++) {
                switch (pid_nemici[i] = fork()) {
                    case -1:
                        perror("Errore nell'esecuzione della fork.");
                        _exit(1);
                    case 0:
                        close(pipe_enemies[i * 2 + 1]); /* chiusura del descrittore di scrittura (standard output)*/
                        enemies[i].index = i;
                        enemies[i].id = ID_NEMICO;
                        Enemy(pipe_enemies[i * 2 + 0], fd1[1], enemies[i]);
                        break;
                    default:
                        continue;
                }
            }
            prctl(PR_SET_NAME, (unsigned long) "Area_gioco");
            close(fd1[1]); /* chiusura del descrittore di scrittura (standard output)*/
            close(fd2[0]); /* chiusura del descrittore di lettura (standard input)*/
            WIN = AreaGioco(fd1[0], fd2[1], pipe_enemies, enemies); /* il processo padre invoca la funzione di AreaGioco */
    }
    /* siamo usciti dalla funzione di AreaGioco e vengono terminati i 2 processi figli e ripristinato il normale modo operativo dello schermo */
    kill(pid_navicella, 1);
    if(WIN){
        clear();
        system("printf '\033[8;24;160t'");
        resize_term(24,160);
        sleep(10);
        printw("\n"
               "██   ██  █████  ██     ██    ██ ██ ███    ██ ████████  ██████      ██████   ██████  ██████   ██████  ██████      ██████  ██  ██████  \n"
               "██   ██ ██   ██ ██     ██    ██ ██ ████   ██    ██    ██    ██     ██   ██ ██    ██ ██   ██ ██      ██    ██     ██   ██ ██ ██    ██ \n"
               "███████ ███████ ██     ██    ██ ██ ██ ██  ██    ██    ██    ██     ██████  ██    ██ ██████  ██      ██    ██     ██   ██ ██ ██    ██ \n"
               "██   ██ ██   ██ ██      ██  ██  ██ ██  ██ ██    ██    ██    ██     ██      ██    ██ ██   ██ ██      ██    ██     ██   ██ ██ ██    ██ \n"
               "██   ██ ██   ██ ██       ████   ██ ██   ████    ██     ██████      ██       ██████  ██   ██  ██████  ██████      ██████  ██  ██████  \n"
               "                                                                                                                                     \n"
               "                                                                                                                                     ");
        refresh();
    }else {
        clear();
        printw("   _____              __  __   ______    ____   __      __  ______   _____    \n"
               "  / ____|     /\\     |  \\/  | |  ____|  / __ \\  \\ \\    / / |  ____| |  __ \\   \n"
               " | |  __     /  \\    | \\  / | | |__    | |  | |  \\ \\  / /  | |__    | |__) |  \n"
               " | | |_ |   / /\\ \\   | |\\/| | |  __|   | |  | |   \\ \\/ /   |  __|   |  _  /   \n"
               " | |__| |  / ____ \\  | |  | | | |____  | |__| |    \\  /    | |____  | | \\ \\   \n"
               "  \\_____| /_/    \\_\\ |_|  |_| |______|  \\____/      \\/     |______| |_|  \\_\\");
        refresh();
    }
    sleep(500);
    endwin();
    free(enemies);
    return MENU;
}

void Enemy(int pipein, int pipeout, Oggetto enemy){
    int j = 0, l=0, temp, k;
    int status = 0;
    char nome[10];
    int random, random2;
    int fd1[2];
    _Bool shoot;
    _Bool spara;
    pid_t pid_bomba;
    Oggetto dummy, bomba, bomba_temp;
    sprintf(nome, "Nemico_%d", enemy.index);
    prctl(PR_SET_NAME, (unsigned long) nome);
    k=17;//3 quasi ok

    bomba = init;
    bomba.id = ID_BOMBA;

    enemy.vite = 3;
    enemy.pid = getpid();
    enemy.old_pos.x = -1;
    enemy.old_pos.y = -1;
    dummy.vite = 3;
    if (pipe(fd1) == -1) { //bombe -> nemici
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (fcntl(fd1[0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);
    if (fcntl(fd1[1], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);


    while (true) {
        enemy.pos.y = (1 + (enemy.index % 5) * (DIM_NEMICO + 1)) + j;
        temp = (int) (enemy.index / 5) + 1;
        enemy.pos.x = maxx - temp * (DIM_NEMICO + 1) - l;
        enemy.id = ID_NEMICO;
        bomba.pos.x = enemy.pos.x-1;
        bomba.pos.y = enemy.pos.y+1;
        bomba_temp.vite = 0;

        if (enemy.vite == 0) {
            write(pipeout, &enemy, sizeof(Oggetto));
            exit(1);
        } else {
            write(pipeout, &enemy, sizeof(Oggetto));
        }
        read(pipein, &dummy, sizeof(Oggetto));
        enemy.vite = dummy.vite;

        /*Spostamenti navicelle*/
        if (j % 8 == 0) {
            l++;
        }

        usleep(300000); //velocità spostamento nemici
        if (j <= 4 && status == 0) {
            j++;
            if (j == 4)
                status = 1;
        } else if (j >= 0 && status == 1) {
            j--;
            if (j == 0)
                status = 0;
        }

        for (int i = 0; i < 100; ++i) {
            read(fd1[0], &bomba_temp, sizeof(Oggetto));
        }

        if(bomba_temp.vite == 1){
            spara = false;
        } else{
            if(bomba_temp.pid!= 0) {
                kill(bomba_temp.pid, SIGKILL);
                spara = true;
            }
        }


        random = rand()%1001; //genera numero casuale
        /*generazione bombe*/
        if(spara==TRUE) {
            if (random % 10 == 0) {
//                if (((enemy.index + k) % 4) == 0 && !shoot) {
                    shoot = TRUE;
                    pid_bomba = fork(); //generazione processo
                    switch (pid_bomba) {
                        case -1:
                            perror("Errore nell'esecuzione della fork.");
                            exit(1);
                        case 0:
                            sprintf(nome, "Bomber %d", enemy.index);
                            prctl(PR_SET_NAME, (unsigned long) nome);
                            close(fd1[0]);
                            bomba.vite=1;
                            bomba.index = enemy.index;
                            write(fd1[1], &bomba, sizeof(Oggetto));
                            if(enemy.index %4 == 0){
                                sleep(5);
                            } else if (enemy.index %2== 1){
                                sleep(10);
                            } else if (enemy.index %2 == 0) {
                                sleep(15);
                            }
                            missili(pipeout, fd1[1], 3, &bomba);
                            break;
                        default: //processo padre
                            break;
                    }
//                } else {
//                    if (k < M) {
//                        k += 3;
//                        shoot = FALSE;
//                    } else {
//                        k = 5;//3 quasi ok von k=5 ancora meglio
//                        shoot = FALSE;
//                    }
//                }
            }
        }
        enemy.old_pos = enemy.pos;
    }
}


void nave_player(int pipeout, int pipein) {
    int filedes[2];
    pid_t pid_missile1, pid_missile2;
    int i = 0, j = 0;
    int num_missili = 0;
    Oggetto array_missili[MAX_MISSILI];
    Oggetto missile1, missile2, temp_missile;
    Oggetto navicella;
    _Bool blocco_sparo = false;


    navicella.id = ID_NAVICELLA;
    navicella.pos.x = 1;
    navicella.pos.y = (maxy - DIM_NAVICELLA) / 2;
    navicella.vite = 3;
    navicella.index = 0;
    navicella.pid = getpid();
    navicella.old_pos.x = -1;
    navicella.old_pos.y = -1;

    //inizializzo a zero l'array di array_missili
    for (i = 0; i < MAX_MISSILI; i++) {
        array_missili[i] = init;
    }

    for (i = 0; i < DIM_NAVICELLA; i++) {
        move(navicella.pos.y + i, 0);
        clrtoeol();
    }
    refresh();


    if (pipe(filedes) == -1) { //inizializzazione della pipe con AreaGioco degli errori
        perror("Errore nella creazione della pipe!");
        _exit(1);
    }
    if (fcntl(filedes[0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);

    write(pipeout, &navicella, sizeof(navicella));

    signal(SIGCHLD, SIG_IGN);

    keypad(stdscr, TRUE);
    while (TRUE) {
        nodelay(stdscr, 1);
        timeout(500);
        int c = getch();
        switch (c) {
            case KEY_UP:
                if (navicella.pos.y > 1)
                    navicella.pos.y -= 1;
                write(pipeout, &navicella, sizeof(navicella));
                break;
            case KEY_DOWN:
                if (navicella.pos.y < maxy - DIM_NAVICELLA)
                    navicella.pos.y += 1;
                write(pipeout, &navicella, sizeof(navicella));
                break;
            case KEY_SPACE:
                if (num_missili <= MAX_MISSILI - 2) {
                    num_missili += 2;
                    missile1.pos.y = navicella.pos.y + (DIM_NAVICELLA / 2);
                    missile1.pos.x = navicella.pos.x + DIM_NAVICELLA;
                    missile2.pos.y = navicella.pos.y + (DIM_NAVICELLA / 2);
                    missile2.pos.x = navicella.pos.x + DIM_NAVICELLA;

                    missile1.id = ID_MISSILE;
                    missile1.index = num_missili - 2;
                    missile2.id = ID_MISSILE;
                    missile2.index = num_missili - 1;

                    i = 0;
                    pid_missile1 = fork(); //generazione processo
                    switch (pid_missile1) {
                        case -1:
                            perror("Errore nell'esecuzione della fork.");
                            exit(1);
                        case 0:
                            prctl(PR_SET_NAME, (unsigned long) "Missile 1");
                            close(filedes[0]);
                            missili(pipeout, filedes[1], 1, &missile1);
                            break;
                        default: //processo padre
                            pid_missile2 = fork(); //generazione di un secondo processo figlio per la nave_player
                            switch (pid_missile2) {
                                case -1:
                                    perror("Errore nell'esecuzione della fork.");
                                    exit(1);
                                case 0:
                                    prctl(PR_SET_NAME, (unsigned long) "Missile 2");
                                    close(filedes[0]);
                                    missili(pipeout, filedes[1], 2, &missile2);
                                    break;
                                default:
                                    break;
                            }
                            break;
                    }
                }
                break;
        }
        //prima legge dai figli missili
        for (i = 0; i <= MAX_MISSILI * 100; i++) {
            read(filedes[0], &temp_missile, sizeof(temp_missile));
            if ((temp_missile.index) >= 0 && (temp_missile.index) < MAX_MISSILI) {
                array_missili[temp_missile.index] = temp_missile;
            }
        }
        //poi legge dal padre area gioco
        for (i = 0; i <= MAX_MISSILI * 100; i++) {
            read(pipein, &temp_missile, sizeof(temp_missile));
            if ((temp_missile.index) >= 0 && (temp_missile.index) < MAX_MISSILI) {
                array_missili[temp_missile.index].vite = temp_missile.vite;
            }
        }
//        clear();
        for (i = 0, j = 0; i < MAX_MISSILI; i++) {
            if (array_missili[i].vite == 0) {
                j++;
            }
//            mvprintw(i+1,0,"array_missili[%d].vite= %d, j= %d", i,array_missili[i].vite, j);
//            refresh();
        }


        if (num_missili == MAX_MISSILI && j == MAX_MISSILI) {
            num_missili = 0;
        }
//        clear();
//        mvprintw(1,0,"num_missili= %d, j= %d", num_missili, j);
//        refresh();

        navicella.old_pos = navicella.pos;
    }
}

void missili(int pipe_to_grandpa, int pipe_to_dad, int tipo, Oggetto *missile){
    int i = 0;
    missile->vite = 1;
    missile->pid = getpid();
    while (TRUE) {
        /*if(tipo == 3){
             clear();
             mvprintw(0,0,"vite: %d\n"
                          "pid: %d\n"
                          "id: %d\n"
                          "x: : %d\n"
                          "y: : %d\n", missile->vite, missile->pid, missile->id, missile->pos.x, missile->pos.y);
             refresh();
        }*/
        if (i % 20 == 0) {
            if (tipo == 2) {
                missile->pos.y += 1;

            } else if (tipo == 1) {
                missile->pos.y -= 1;

            }
        }
        if(tipo == 2 || tipo == 1){
            missile->pos.x += 1;
        }
        if(tipo == 3){
            missile->pos.x -= 1;
        }
        write(pipe_to_grandpa, missile, sizeof(Oggetto));
        write(pipe_to_dad, missile, sizeof(Oggetto));
        i++;
        if ((missile->pos.x > maxx || missile->pos.y >= maxy || missile->pos.y < 0) && (tipo == 2 || tipo == 1)) {
            missile->vite = 0;
            write(pipe_to_dad, missile, sizeof(Oggetto));
            exit(1);
        }else if((missile->pos.x < 0 || missile->pos.y >= maxy || missile->pos.y < 0) && tipo == 3){
            missile->vite = 0;
            write(pipe_to_dad, missile, sizeof(Oggetto));
            exit(1);
        }
        missile->old_pos = missile->pos;
        usleep(100000); //regola velocità missili
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
_Bool AreaGioco(int pipein, int pipe_to_navicella,int *pipe_to_nemici, Oggetto *enemies) {
    int i, j, k, l;
    _Bool collision = false, WIN = false;
    Oggetto navicella, valore_letto = init;
    Oggetto missili[MAX_MISSILI];
    Oggetto *bombe_nem = (Oggetto *) calloc(M, sizeof(Oggetto));
    navicella.pos.x = -1;
    navicella.vite = 3;

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
    for (i = 0; i < M; ++i) {
        enemies[i].vite = 3;
    }

    do {
        /// Lettura da tutti i processi figli
        read(pipein, &valore_letto, sizeof(valore_letto)); /* leggo dalla pipe */

        /// Stampa dei vari oggetti
        switch (valore_letto.id) {
            case ID_NAVICELLA:
                stampaNavicella(valore_letto);
                navicella.pos = valore_letto.pos;
                navicella.old_pos = valore_letto.old_pos;
                navicella.index = valore_letto.index;
                navicella.pid = valore_letto.pid;
                navicella.id = valore_letto.id;
                break;
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
            case ID_MISSILE:
                stampaMiss_bomb(valore_letto);
                missili[valore_letto.index] = valore_letto;
                break;
            case ID_BOMBA:
                stampaMiss_bomb(valore_letto);
                bombe_nem[valore_letto.index] = valore_letto;
                break;
        }
        /// Controllo delle varie collisioni
        collisione_missili_bombe(missili, bombe_nem, pipe_to_navicella, pipe_to_nemici);
        collisione_bombe_navicella(navicella, bombe_nem, pipe_to_nemici);
        collisione_missili_nemici(enemies, missili, pipe_to_navicella, pipe_to_nemici);

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
        for (i = 0, j = 0; i < M; i++) {
            if (enemies[i].vite == 0) {
                j++;
            }
        }
        if (j == 2) {
            WIN = TRUE;
            collision = true;
        }
        move(0, 0);
        clrtoeol();
        mvprintw(0, 0, "VITE: %d, j: %d", navicella.vite, j);
        refresh();
    } while (!collision);
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
void stampaMiss_bomb(Oggetto bomb_miss) {
    if (bomb_miss.pid != 0) {
        attron(COLOR_PAIR(0));
        mvprintw(bomb_miss.old_pos.y, bomb_miss.old_pos.x, " ");
        attroff(COLOR_PAIR(0));
    }
    if (bomb_miss.id == ID_MISSILE)
        attron(COLOR_PAIR(3));
    else
        attron(COLOR_PAIR(7));

    mvprintw(bomb_miss.pos.y, bomb_miss.pos.x, "⟢"); ///♿ ⟢ ⁂ ꗇ ꗈ 💣 🚀 卐 ◌́ ◌͂ ✝

    if (bomb_miss.id == ID_MISSILE)
        attroff(COLOR_PAIR(3));
    else
        attroff(COLOR_PAIR(7));
}

void collisione_missili_bombe(Oggetto *missili, Oggetto *bombe_nem, int pipe_to_navicella,int *pipe_to_nemici){
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
                        write(pipe_to_navicella, &missili[i], sizeof(missili[i]));
                        //uccido la bomba collisa
                        kill(bombe_nem[j].pid, SIGKILL);
                        bombe_nem[j].vite = 0;
//                            write(pipe_to_nemici[i * 2 + 1], &bombe_nem[j], sizeof(Oggetto));
                        attron(COLOR_PAIR(0));
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
void collisione_bombe_navicella(Oggetto navicella, Oggetto *bombe_nem, int *pipe_to_nemici){
    int i,j,k;
    ///collisione tra bombe e navicella player
    for (i = 0; i < M; i++) {
        if (bombe_nem[i].pid != 0) {
            for (j = 0; j < DIM_NAVICELLA; ++j) {
                for (k = 0; k < DIM_NAVICELLA; ++k) {
                    if (j > 0 && j<5){
                        k = 5;
                    }
                    if(bombe_nem[i].pos.x == navicella.pos.x + k && bombe_nem[i].pos.y == navicella.pos.y + j){
//                          uccido la bomba collisa
                        kill(bombe_nem[i].pid, SIGKILL);

//                            write(pipe_to_nemici[i * 2 + 1], &bombe_nem[i], sizeof(Oggetto));

                        navicella.vite--;
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
void collisione_missili_nemici(Oggetto *enemies, Oggetto *missili, int pipe_to_navicella, int *pipe_to_nemici) {
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
                                write(pipe_to_navicella, &missili[j], sizeof(missili[j]));
                                enemies[i].vite -= 3;
                                write(pipe_to_nemici[i * 2 + 1], &enemies[i], sizeof(Oggetto));
//                                    clear();
//                                    mvprintw(1, 0, "enemies[%d].vite = %d", i, enemies[i].vite);
//                                    refresh();
                                attron(COLOR_PAIR(0));
                                mvaddstr(missili[j].pos.y, missili[j].pos.x, " ");
                                attroff(COLOR_PAIR(0));
                                missili[j] = init;
                                enemies[i] = init;
                            }
                        }
                    }
                }
            }
        }
    }
}