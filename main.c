#include "menu.h"
#include "stampa.h"
#include "generazione_e_movimento.h"
#include "collisioni.h"

_Bool AreaGioco(int *pipe1, int pipe_from_navicella, Oggetto *enemies);

/// Inizializzazione di variabili globali
int M = 24;                     // Numero default di nemici
int num_righe = 6;              // Numero default di nemici in ogni colonna
int mov_verticale = 6;          // Numero di spostamenti verticali default
int velocita_missili = 100000;  // Velocità missile default
const Oggetto init = {0, 0, {0, 0}, {0, 0}, 0, 0}; // Costante per inizializzare le strutture Oggetto comodamente
int punteggio_f; //punteggio finale

/// Inizializzazione stampe
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


int main() {
    StatoCorrente status;
    status = MENU;      // Variabile che conterrà il contesto di esecuzione corrente

    /// Gestione cambiamento di stati tra i vari contesti
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

/**
 * Funzione che fa eseguire il gioco
 * @return contesto in cui dovrà tornare una volta terminata la funzione
 */
StatoCorrente gioco() {
    /// Dichiarazione variabili
    int i;                      // Variabile contatore
    _Bool WIN;                  // Contiene se si vince o si perde
    int main_pipe[2];
    int pipe_from_navicella[2];
    pid_t pid_navicella;
    Oggetto *enemies = (Oggetto *) calloc(M, sizeof(Oggetto));

    setlocale(LC_ALL, "");      // per poter utilizzare caratteri unicode
    initscr();                  // inizializzazione dello schermo
    noecho();                   // i caratteri corrispondenti ai tasti premuti non saranno visualizzati sullo schermo del terminale
    curs_set(0);                // nasconde il cursore

    /// Inizializzazione dei colori
    start_color();
    init_pair(BLACK, COLOR_BLACK, COLOR_BLACK); // per cancellare
    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK); // per scrivere
    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(RED, COLOR_RED, COLOR_BLACK);

    bkgd(COLOR_PAIR(WHITE));

    /// inizializzazione delle pipes
    if (pipe(main_pipe) == -1) { //nemici, bombe, missili -> area gioco
        perror("Errore nella creazione della pipe!");
        _exit(1);}
    if (pipe(pipe_from_navicella) == -1) { // navicella -> area gioco
        perror("Errore nella creazione della pipe!");
        _exit(1);}
    if (fcntl(pipe_from_navicella[0], F_SETFL, O_NONBLOCK) < 0) //pipe non bloccante
        exit(2);

    /// Generazione dei vari processi
    /// Generazione di processo figlio per la nave_player
    pid_navicella = fork();
    switch (pid_navicella) {
        case -1:
            perror("Errore nell'esecuzione della fork.");
            _exit(1);
        case 0:
            prctl(PR_SET_NAME, (unsigned long) "Navicella");
            close(pipe_from_navicella[0]);          // chiusura del descrittore di lettura (standard input)*
            nave_player(pipe_from_navicella[1]);    // Passiamo a nave_player la pipe in scrittura
        default:
            /// Generazione dei processi dei nemici
            for (i = 0; i < M;  i++) {
                switch (enemies[i].pid = fork()) {
                    case -1:
                        perror("Errore nell'esecuzione della fork.");
                        _exit(1);
                    case 0:
                        close(main_pipe[0]);
                        enemies[i].index = i;   // Assegnamendo dell'indice
                        Enemy(main_pipe[1], &enemies[i]);
                        break;
                    default:
                        continue;
                }
            }
            prctl(PR_SET_NAME, (unsigned long) "Area_gioco");   // Per dare un nome al processo
            close(pipe_from_navicella[1]); // chiusura del descrittore di lettura (standard input)
            WIN = AreaGioco(main_pipe, pipe_from_navicella[0], enemies); // Il processo padre invoca AreaGioco
    }
    // Siamo usciti dalla funzione di AreaGioco
    // Stampiamo il punteggio e se il player ha vinto o perso
    if (WIN) {
        clear();
        system("aplay -q Suoni/win.wav &");
        for (i = 0; i < 6; ++i) {
            mvprintw((maxy - 5) / 2 + i, (maxx - 58) / 2, vittoria[i]);     // Stampa vittoria
        }
    } else {
        clear();
        system("aplay -q Suoni/gameover.wav &");
        for (i = 0; i < 6; ++i) {
            mvprintw((maxy - 6) / 2 + i, (maxx - 69) / 2, gameover[i]);     // Stampa game over
        }
    }
    mvprintw((maxy- 5)/2 + i+2, (maxx-13)/2, "SCORE: %d", punteggio_f);
    refresh();
    sleep(5);
    endwin();

    /// Liberazione delle risorse allocate dinamicamente
    free(enemies);
    return MENU;
}

/** Funzione principale che gestisce tutto il gioco. Da qua vengono gestite le stampe di tutti gli oggetti, le collisioni,
 * la generazione dei processi delle bombe e dei missili, e la loro corretta terminazione.
 * @param pipe1                 Pipe principale, riceve i valori dai nemici, dalle bombe e dai missili
 * @param pipe_from_navicella   Pipe che riceve i valori dalla navicella
 * @param enemies               Array che contiene i dati dei nemici
 * @return                      Valore di vittoria
 */
_Bool AreaGioco(int *pipe1, int pipe_from_navicella, Oggetto *enemies) {
    /// Dichiarazione delle variabili
    int i, j = 0;                       // Variabili contatore
    int nemici_vivi;                    // Contatore di nemici vivi
    Oggetto rockets[MAX_MISSILI];       // Array dei dati dei missili amici sparati
    arg_from_navicella argFromNavicella;// Variabile per salvare i dati letti dalla pipe che legge da navicella
    Oggetto navicella_temp;             // Variabile usata per gestire le vite di navicella

    /// Inizializzazione variabili
    int punteggio = M * 100;            // Punteggio del gioco
    int missili_vivi = 0;               // Contatore di missili vivi
    int missili_morti = MAX_MISSILI;    // Contatore di missili morti
    _Bool collision = false;            // Per verificare le collisioni con navicella
    _Bool WIN = false;
    Oggetto valore_letto = init;        // Variabile per salvare i dati letti dalla pipe principale

    /// Inizializzazione variabili dinamiche
    Oggetto *bombe_nem = (Oggetto *) calloc(M, sizeof(Oggetto));    // Array dei dati delle bombe
    _Bool *primafila = (_Bool *) calloc(M, sizeof(_Bool));          // Array dei dati di primafila
    int *pipe_to_bombe = (int *)malloc(M * 2 * sizeof(int));        // Array di file descriptor

    /// Inizializzazione delle pipe per comunicare con le bombe
    for (i = 0; i < M; i++) {
        if (pipe(&pipe_to_bombe[i * 2 + 0]) == -1) { // Area gioco -> bombe
            perror("Errore nella creazione della pipe!");
            _exit(1);
        }
    }

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
    /// Inizializzazione a zero dell'array di missili
    for (i = 0; i < MAX_MISSILI; ++i) {
        rockets[i] = init;
    }

    /// Inizializzazione delle vite dei nemici e della navicella
    for (i = 0; i < M; ++i) {
        enemies[i].vite = 3;
    }
    navicella_temp.vite=3;

    clear();
    refresh();
    do {
        /// Lettura da tutti i processi figli
        read(pipe1[0], &valore_letto, sizeof(valore_letto)); // leggo dalla pipe principale
        read(pipe_from_navicella, &argFromNavicella, sizeof(arg_from_navicella)); // leggo dalla pipe di navicella

        // Salvo tutti i dati (tranne le vite) della navicella in navicella_temp
        navicella_temp.pos = argFromNavicella.navicella.pos;
        navicella_temp.old_pos = argFromNavicella.navicella.old_pos;
        navicella_temp.index = argFromNavicella.navicella.index;
        navicella_temp.pid = argFromNavicella.navicella.pid;
        navicella_temp.id = argFromNavicella.navicella.id;

        // Stampo la navicella
        stampaNavicella(argFromNavicella.navicella);

        // Se è stato premuto space e i missili già sparati sono di meno del massimo allora genero i nuovi due missili
        if(argFromNavicella.sparo && missili_vivi < MAX_MISSILI) {
            genera_missili(&missili_vivi, rockets, pipe1, &argFromNavicella);
        }

        /// Stampa dei vari oggetti
        switch (valore_letto.id) {
            case ID_NEMICO:
                /// Stampo il nemico
                stampaNemico(valore_letto, enemies[valore_letto.index].vite);
                /// Salvo nell'array tutti i dati (tranne le vite) del nemico letto
                if (enemies[valore_letto.index].vite > 0) {
                    enemies[valore_letto.index].pos = valore_letto.pos;
                    enemies[valore_letto.index].old_pos = valore_letto.old_pos;
                    enemies[valore_letto.index].index = valore_letto.index;
                    enemies[valore_letto.index].pid = valore_letto.pid;
                    enemies[valore_letto.index].id = valore_letto.id;
                    // se il nemico è morto lo reinizializzo
                } else if (enemies[valore_letto.index].vite == 0) {
                    enemies[valore_letto.index] = init;
                }
                break;
            case ID_MISSILE1:
                // Se il missile di cui ho ricevuto precedentemente dati è vivo, lo stampo (questo controllo viene
                // effettuato perchè esso potrebbe essere già morto in una collisione, in tal caso non lo stampo)
                if (rockets[valore_letto.index].vite != 0) {
                    stampaMissile(&valore_letto);
                }
                // Se il missile di cui ho ricevuto i dati è morto lo reinizializzo
                if(valore_letto.vite == 0){
                    rockets[valore_letto.index] = init;
                } else{ // Altrimenti salvo nell'array tutti i dati (tranne le vite) del missile letto
                    rockets[valore_letto.index].pos = valore_letto.pos;
                    rockets[valore_letto.index].old_pos = valore_letto.old_pos;
                    rockets[valore_letto.index].index = valore_letto.index;
                    rockets[valore_letto.index].pid = valore_letto.pid;
                    rockets[valore_letto.index].id = valore_letto.id;
                }
                break;
            case ID_MISSILE2:
                // Se il missile di cui ho ricevuto precedentemente dati è vivo, lo stampo (questo controllo viene
                // effettuato perchè esso potrebbe essere già morto in una collisione, in tal caso non lo stampo)
                if (rockets[valore_letto.index].vite != 0) {
                    stampaMissile(&valore_letto);
                }
                // Se il missile di cui ho ricevuto i dati è morto lo reinizializzo
                if(valore_letto.vite == 0){
                    rockets[valore_letto.index] = init;
                } else{ // Altrimenti salvo nell'array tutti i dati (tranne le vite) del missile letto
                    rockets[valore_letto.index].pos = valore_letto.pos;
                    rockets[valore_letto.index].old_pos = valore_letto.old_pos;
                    rockets[valore_letto.index].index = valore_letto.index;
                    rockets[valore_letto.index].pid = valore_letto.pid;
                    rockets[valore_letto.index].id = valore_letto.id;
                }
                break;
            case ID_BOMBA:
                // Se il valore letto è pos(0,0) vuol dire che sono stati ricevuti dalla pipe dei dati della
                // non ancora inizializzata. Se succede allora comunichiamo a tale bomba le coordinate attuali
                // del nemico da cui deve partire tale bomba che verrà conseguentemente inizializzata con queste
                // coordinate. Grazie a questo possiamo far partire la bomba nella posizione corretta
                if(valore_letto.pos.y == 0 && valore_letto.pos.x == 0){
                    write(pipe_to_bombe[valore_letto.index * 2 + 1], &enemies[valore_letto.index], sizeof(Oggetto));
                }
                // Se la bomba di cui ho ricevuto precedentemente dati è viva, la stampo (questo controllo viene
                // effettuato perchè essa potrebbe essere già morta in una collisione, in tal caso non la stampo)
                if (bombe_nem[valore_letto.index].vite != 0) {
                    stampaBomba(&valore_letto);
                }
                //salvo nell'array tutti i dati (tranne le vite) della bomba letta
                bombe_nem[valore_letto.index].pos = valore_letto.pos;
                bombe_nem[valore_letto.index].old_pos = valore_letto.old_pos;
                bombe_nem[valore_letto.index].index = valore_letto.index;
                bombe_nem[valore_letto.index].pid = valore_letto.pid;
                bombe_nem[valore_letto.index].id = valore_letto.id;

                // Se la bomba esce fuori dall'area di gioco uccidiamo il processo
                if (bombe_nem[valore_letto.index].pos.x < 0) {
                    bombe_nem[valore_letto.index].vite = 0;
                    kill(bombe_nem[valore_letto.index].pid, SIGKILL);
                }
                // Se le vite della bomba sono zero reinizializziamo i dati della bomba nell'array
                if (bombe_nem[valore_letto.index].vite == 0){
                    bombe_nem[valore_letto.index] = init;
                }
                break;
        }

        /// Generazione bomba
        for (i = 0; i < M; ++i) {
            if (primafila[i] == TRUE) {         // se siamo in primafila
                genera_bomba(bombe_nem, i, enemies, pipe1, pipe_to_bombe);
            }
        }

        /// Gestione missili sparabili contemporaneamente
        for (i = 0, missili_morti = 0; i < MAX_MISSILI; ++i) {
            if (rockets[i].vite == 0) {     // se il missile è morto
                missili_morti++;
            }
        }
        if (missili_morti == MAX_MISSILI) { // Quando muore tutta l'ondata di missili si potrà tornare a sparare
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
        mvprintw(0, 0, "VITE: %d, SCORE: %d", navicella_temp.vite, (punteggio - (nemici_vivi * 100)));
        punteggio_f = (punteggio - (nemici_vivi * 100)) + (navicella_temp.vite * 200);
        refresh();
    } while (!collision);

    /// Uccisione dei thread dei nemici e delle bombe ancora in vita
    for (i = 0; i < M; ++i) {
        if (enemies[i].pid != 0) {
            kill(enemies[i].pid, SIGKILL);
        }
        if (bombe_nem[i].pid != 0) {
            kill(bombe_nem[i].pid, SIGKILL);
        }
    }
    /// Uccisione dei thread dei missili ancora in vita
    for (i = 0; i < MAX_MISSILI; i++) {
        if (rockets[i].pid != 0) {
            kill(rockets[i].pid, SIGKILL);
        }
    }
    /// Uccisione thread della navicella
    kill(navicella_temp.pid, SIGKILL);

    /// Deallocazione delle risorse allocate dinamicamente
    free(bombe_nem);
    free(primafila);
    free(pipe_to_bombe);

    return WIN;
}


