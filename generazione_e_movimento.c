/**
 * In questo file vengono definite le funzioni per la gestione dei movimenti dei vari oggetti di gioco
 */
#include "generazione_e_movimento.h"

/**
 * La funzione Enemy inizializza, elimina una navicella nemica e aggiorna la sua posizione.
 * I suoi paramentri in ingresso sono:
 * @param pipeout - È il file descriptor aperto in scrittura
 * @param enemy - È un array di strutture @Oggetto che contiene tutti i dati di ogni singola navicella nemica.
 * @return - void*
 */
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
        /* Aggiorna i valori delle coordinate della navicella per farla muovere */
        enemy->pos.y = (1 + (enemy->index % num_righe) * (DIM_NEMICO + 1)) + j;
        temp = (int) (enemy->index / num_righe) + 1;
        enemy->pos.x = maxx - temp * (DIM_NEMICO + 1) - l;
        write(pipeout, enemy, sizeof(Oggetto)); // scrive sulla pipe
        /* Spostamento orizzontale navicelle nemiche */
        if (j % 8 == 0) { // ogni 8 spostamenti in verticale avanza
            l++;
        }
        usleep(300000); //velocità spostamento nemici
        /* Spostamenti verticali navicelle nemiche*/
        if (j <= mov_verticale && status == 0) {
            j++; // verso giù
            if (j == mov_verticale)
                status = 1; // inverte il senso dello spostamento verticale
        } else if (j >= 0 && status == 1) {
            j--; //verso su
            if (j == 0)
                status = 0; // inverte il senso dello spostamento verticale
        }
        enemy->old_pos = enemy->pos; // salva l'ultima posizione della navicella nemica
    }
}
/**
 * La funzione nave_player consente al giocatore di spostare con movimenti verticali la navicella, inoltre,
 * consente di abilitare lo sparo per la generazione dei missili.
 * I suoi paramentri in ingresso sono:
 * @param pipeout - È il file descriptor aperto in scrittura
 * @return - void*
 */
void nave_player(int pipeout) {
    arg_from_navicella argFromNavicella;
    /** Inizializzazione campi nave player**/
    argFromNavicella.navicella.id = ID_NAVICELLA;
    argFromNavicella.navicella.pos.x = 1;
    argFromNavicella.navicella.pos.y = (maxy - DIM_NAVICELLA) / 2;
    argFromNavicella.navicella.vite = 3;
    argFromNavicella.navicella.index = 0;
    argFromNavicella.navicella.pid = getpid();
    argFromNavicella.navicella.old_pos.x = 1;
    argFromNavicella.navicella.old_pos.y = (maxy - DIM_NAVICELLA) / 2;

    argFromNavicella.sparo = FALSE;

    write(pipeout, &argFromNavicella, sizeof(argFromNavicella)); // scrive sulla pipe

    keypad(stdscr, TRUE); // abilita l'input da tastiera
    while (TRUE) {
        int c = getch();
        // Controlla il tasto premuto dal giocatore
        switch (c) {
            /// se ha premuto la freccia verso su
            case KEY_UP:
                /* impedisce alla nave di sovrascrivere la prima riga dello schermo */
                if (argFromNavicella.navicella.pos.y > 1)
                    argFromNavicella.navicella.pos.y -= 1; // decrementa le y della nave facendola spostare verso l'alto
                break;
            /// se ha premuto la freccia verso giù
            case KEY_DOWN:
                /* impedisce alla nave di uscire dalla parte inferiore dello schermo */
                if (argFromNavicella.navicella.pos.y < maxy - DIM_NAVICELLA)
                    argFromNavicella.navicella.pos.y += 1; // incrementa le y della nave facendola spostare verso il basso
                break;
            /// se ha premuto la barra spaziatrice
            case KEY_SPACE:
                argFromNavicella.sparo = TRUE; // abilita lo sparo
                break;
        }
        write(pipeout, &argFromNavicella, sizeof(argFromNavicella)); // scrive sulla pipe
        argFromNavicella.sparo = FALSE;  // disabilita lo sparo
        argFromNavicella.navicella.old_pos = argFromNavicella.navicella.pos; // salva l'ultima posizione della nave player
    }
}
/**
 * La funzione missili genera il processo di un missile e inizializza i suoi valori
 * I suoi paramentri in ingresso sono:
 * @param missili_vivi - È un puntatore a una variabile che contiene il numero di missili vivi
 * @param rockets - È un struttura Oggetto per inizializzare tutti i valori del missile
 * @param pipe1 - È un file descriptor generico
 * @param argFromNavicella - È la stuttura Oggetto nave player
 */
void genera_missili(int *missili_vivi, Oggetto *rockets, int *pipe1, arg_from_navicella *argFromNavicella){
    for (int temp = *missili_vivi + 2; *missili_vivi < temp; *missili_vivi += 1) {
        /** Inizializzazione i valori del missile **/
        rockets[*missili_vivi].pid = fork(); //generazione processo
        rockets[*missili_vivi].pos.x = argFromNavicella->navicella.pos.x + DIM_NAVICELLA;
        rockets[*missili_vivi].pos.y = argFromNavicella->navicella.pos.y + (DIM_NAVICELLA / 2);
        rockets[*missili_vivi].old_pos.x = -1;
        rockets[*missili_vivi].old_pos.y = -1;
        rockets[*missili_vivi].vite = 1;
        rockets[*missili_vivi].index = *missili_vivi;
        // Controlla l'esito della fork
        switch (rockets[*missili_vivi].pid) {
            /// Se non è stata effettuata
            case -1:
                perror("Errore nell'esecuzione della fork.");
                exit(1); // termina il processo corrente
            /// Se è stata effettuata con successo caso: processo figlio
            case 0:
                // Assegna il nome al processo
                prctl(PR_SET_NAME, (unsigned long) "Missile 1");
                close(pipe1[0]); // chiude la pipe in lettura
                // Inizializza l'id del missile
                if (*missili_vivi % 2 == 0)
                    rockets[*missili_vivi].id = ID_MISSILE1;
                else
                    rockets[*missili_vivi].id = ID_MISSILE2;
                system("aplay -q Suoni/missili.wav &");
                rockets[*missili_vivi].pid = getpid();
                missili(pipe1[1], &rockets[*missili_vivi]);
                break;
            /// Se è stata effettuata con successo caso: processo padre
            default:
                argFromNavicella->sparo = false; // disabilita lo sparo
                break;
        }
    }
}
/**
 * La funzione missili genera un movimento in diagonale verso l'alto o verso il basso in base a quale
 * missile gli viene passato in input
 * @param pipeout - È il file descriptor aperto in scrittura
 * @param missile - È una struttura Oggetto contiene tutti i valori relativi a un missile
 * @return - void*
 */
void missili(int pipeout, Oggetto *missile){
    int i = 0; // varaiabile contattore
    while (TRUE) {
        if (i % 20 == 0) {
            if (missile->id == ID_MISSILE2) {
                missile->pos.y += 1; // MISSILE2 si sposta verso il basso
            } else if (missile->id == ID_MISSILE1) {
                missile->pos.y -= 1; // MISSILE1 si sposta verso l'alto
            }
            i=0; 
        }
        missile->pos.x += 1; // Sposta il missile di una posizione da sinistra verso destra
        write(pipeout, missile, sizeof(Oggetto)); // scrive sulla pipe
        i++;
        // Se uno dei due missili raggiunge i bordi della finestra di gioco
        if ((missile->pos.x > maxx || missile->pos.y >= maxy || missile->pos.y < 0)) {
            missile->vite = 0; // impostiamo le vite a zero
            write(pipeout, missile, sizeof(Oggetto)); // scrive sulla pipe
            exit(1); // termina il processo corrente 
        }
        missile->old_pos = missile->pos; // salva l'ultima posizione del missile
        usleep(velocita_missili); //regola velocità missili
    }
}
/**
 * La funzione genera_bomba genera il processo di una bomba e inizializza i suoi valori
 * I suoi paramentri in ingresso sono:
 * @param bomba_nem - È un array di strutture Oggetto e contiene i valori di ogni singola bomba
 * @param i - È l'indice dell'array 
 * @param pipe1 - È un file descriptor generico
 * @param pipe_to_bombe - È un file descriptor aperto in lettura
 */
void genera_bomba(Oggetto *bombe_nem, int i, Oggetto *enemies, int *pipe1, int *pipe_to_bombe){
    if (bombe_nem[i].vite == 0 && bombe_nem[i].pid == 0 && enemies[i].pid != 0) { // se la bomba sparata precedentemente è morta
        /// Inizializzo parzialmente la nuova bomba
        bombe_nem[i].pid = fork();
        bombe_nem[i].vite = 1;
        // Controlla l'esito della fork
        switch (bombe_nem[i].pid) {
            /// Se non è stata effettuata
            case -1:
                perror("Errore nell'esecuzione della fork.");
                exit(1); // termina il processo corrente
            /// Se è stata effettuata con successo caso: processo figlio
            case 0:
                close(pipe1[0]); // chiude la pipe in lettura
                /** Inizializzazione i valori della bomba **/
                bombe_nem[i].index = i;
                bombe_nem[i].id = ID_BOMBA;
                bombe_nem[i].old_pos.x = -1;
                bombe_nem[i].old_pos.y = -1;
                bombe_nem[i].pid = getpid();
                if (bombe_nem[i].index % 4 == 0) {
                    sleep(1);
                } else if (bombe_nem[i].index % 2 == 1) {
                    sleep(3);
                } else if (bombe_nem[i].index % 2 == 0) {
                    sleep(5);
                }
                write(pipe1[1], &bombe_nem[i], sizeof(Oggetto)); // scrive sulla pipe
                close(pipe_to_bombe[i * 2 + 1]); // chiude la pipe in scrittura
                read(pipe_to_bombe[i * 2 + 0], &enemies[i], sizeof(Oggetto)); // legge dalla pipe
                bombe_nem[i].pos.x = enemies[i].pos.x - 1;
                bombe_nem[i].pos.y = enemies[i].pos.y + (DIM_NEMICO / 2);
                bombe(pipe1[1], &bombe_nem[i]);
                break;
            /// Se è stata effettuata con successo caso: processo padre
            default:
                break;
        }
    }
}
/**
 * La funzione bombe genera le coordinate per produrre un movimento rettilineo da destra verso sinistra
 * @param pipeout - È un file descriptor aperto i lettura
 * @param bomba - È un struttura Oggetto e contiene le coordinate della bomba
 * @return - void*
 */
void bombe(int pipeout, Oggetto *bomba) {
    char nome[10];
    /* Assegna il nome al processo seguendo l'ordine di generazione delle bombe*/
    sprintf(nome, "Bomba %d", bomba->index);
    prctl(PR_SET_NAME, (unsigned long) nome);

    while (TRUE) {
        bomba->pos.x -= 1; // Movimento verso sinistra
        write(pipeout, bomba, sizeof(Oggetto)); // scrive sulla pipe
        bomba->old_pos = bomba->pos; // salva l'ultima posizione della bomba
        usleep(velocita_missili); //regola velocità missili
    }
}