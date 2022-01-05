#include "generazione_movimenti.h"
/**
 * La funzione Enemy elimina le navicelle nemiche, inoltre,
 * inizializza e aggiorna la posizione della navicella.
 * @param arg - È un puntatore generico. In questo caso viene passata una struttura Oggetto
 * @return - void*
 */
void* Enemy(void* arg) {
    int j = 0, l = 0, temp; // inizializzazione variabili temporanee
    int status = 0; //
    /** Dichiarazione strutture temporanee per aggiornare i valori dopo lo spostamento **/
    Oggetto *enemy, temp_enemy;
    enemy = (Oggetto *) arg;
    /* Inizializzazione parziale dei valori della navicella nemica */
    pthread_mutex_lock(&mtx_nemici); ///blocca il mutex @mtx_nemici
    enemy->vite = 3;
    enemy->old_pos.x = -1;
    enemy->old_pos.y = -1;
    enemy->id = ID_NEMICO;
    temp_enemy = *enemy;

    pthread_mutex_unlock(&mtx_nemici); ///sblocca il mutex @mtx_nemici
    while (true) {
        /* Aggiorna i valori delle coordinate della navicella per farla muovere */
        temp_enemy.pos.y = (1 + (temp_enemy.index % num_righe) * (DIM_NEMICO + 1)) + j;
        temp = (int) (temp_enemy.index / num_righe) + 1;
        temp_enemy.pos.x = maxx - temp * (DIM_NEMICO + 1) - l;

        /* Se la navicella nemica è morta*/
        if (temp_enemy.vite == 0) {
            pthread_mutex_lock(&mtx_nemici); ///blocca il mutex @mtx_nemici
            enemy->pos = temp_enemy.pos; // salva la posizione attuale
            enemy->old_pos = temp_enemy.old_pos; // salva la posizione precedente
            pthread_mutex_unlock(&mtx_nemici); ///blocca il mutex @mtx_nemici
            pthread_exit(NULL); // termina il thread corrente
        } else {
            pthread_mutex_lock(&mtx_nemici); ///blocca il mutex @mtx_nemici
            enemy->pos = temp_enemy.pos; // salva la posizione attuale
            enemy->old_pos = temp_enemy.old_pos; // salva la posizione precedente
            temp_enemy.vite = enemy->vite; // aggiorna le vite
            pthread_mutex_unlock(&mtx_nemici); ///blocca il mutex @mtx_nemici
        }


        /* Spostamento orizzontale navicelle nemiche */
        if (j % 8 == 0) {
            l++;
        }

        usleep(300000); //velocità spostamento nemici
        /* Spostamenti verticali navicelle nemiche*/
        if (j <= mov_verticale && status == 0) {
            j++; // verso su
            if (j == mov_verticale)
                status = 1;
        } else if (j >= 0 && status == 1) {
            j--; //verso giù
            if (j == 0)
                status = 0;
        }
        temp_enemy.old_pos = temp_enemy.pos; // salva l'ultima posizione della navicella nemica
    }
}
/**
 * La funzione nave_player consente al giocatore di spostare con movimenti verticali la navicella, inoltre,
 * consente di abilitare lo sparo per la generazione dei missili.
 * @param arg - È un puntatore generico. In questo caso viene passata una struttura Oggetto
 * @return - void*
 */
void* nave_player(void *arg) {
    /// Definisce l'argomento passato in input alla funzione
    arg_to_navicella *argToNavicella;
    argToNavicella = (arg_to_navicella *) arg;
    /** Dichiarazione strutture temporanee per aggiornare i valori dopo lo spostamento e per
     * abilitare lo sparo dei missili **/
    Oggetto *navicella = argToNavicella->navicella;
    _Bool *sparo = &argToNavicella->sparo;

    keypad(stdscr, TRUE); // abilita l'input da tastiera
    while (TRUE) {
        int c = getch();
        // Controlla il tasto premuto dal giocatore
        switch (c) {
            /// se ha premuto la freccia verso su
            case KEY_UP:
                pthread_mutex_lock(&mtx_nave); ///blocca il mutex @mtx_nave
                /* impedisce alla nave di sovrascrivere la prima riga dello schermo */
                if (navicella->pos.y > 1)
                    navicella->pos.y -= 1; // decrementa le y della nave facendola spostare verso l'alto
                pthread_mutex_unlock(&mtx_nave); ///sblocca il mutex @mtx_nave
                break;
            /// se ha premuto la freccia verso giù
            case KEY_DOWN:
                /* impedisce alla nave di uscire dalla parte inferiore dello schermo */
                pthread_mutex_lock(&mtx_nave); ///blocca il mutex @mtx_nave
                if (navicella->pos.y < maxy - DIM_NAVICELLA)
                    navicella->pos.y += 1; // incrementa le y della nave facendola spostare verso il basso
                pthread_mutex_unlock(&mtx_nave); ///sblocca il mutex @mtx_nave
                break;
            /// se ha premuto la barra spaziatrice
            case KEY_SPACE:
                pthread_mutex_lock(&mtx_nave); ///blocca il mutex @mtx_nave
                *sparo = true; // abilita lo sparo
                pthread_mutex_unlock(&mtx_nave); ///sblocca il mutex @mtx_nave
                break;
        }
    }
}
/**
 * La funzione missili genera un movimento in diagonale verso l'alto o verso il basso in base a quale
 * missile gli viene passato in input
 * @param arg - È un puntatore generico. In questo caso viene passata una struttura Oggetto
 * @return - void*
 */
void* missili(void* arg) {
    /// Definisce l'argomento passato in input alla funzione
    Oggetto *missile, temp_missile;
    missile = (Oggetto *) arg;
    int i = 0; // indice contatore
    while (TRUE) {
        pthread_mutex_lock(&mtx_missili); ///blocca il mutex @mtx_missili
        temp_missile = *missile; //variabile temporanea
        pthread_mutex_unlock(&mtx_missili); ///sblocca il mutex @mtx_missili
        // Ogni 20 spostamenti orizzontali
        if (i % 20 == 0) {
            if (temp_missile.id == ID_MISSILE2) {
                temp_missile.pos.y += 1; // MISSILE2 si sposta verso il basso
            } else if (temp_missile.id == ID_MISSILE1) {
                temp_missile.pos.y -= 1; // MISSILE1 si sposta verso l'alto
            }
        }
        // Sposta il missile di una posione da sinistra verso destra
        temp_missile.pos.x += 1;
        i++;
        // Se uno dei due missili raggiunge i bordi della finestra di gioco
        if ((temp_missile.pos.x > maxx || temp_missile.pos.y >= maxy || temp_missile.pos.y < 0) &&
            (temp_missile.id == ID_MISSILE2 || temp_missile.id == ID_MISSILE1)) {
            temp_missile.vite = 0; // set il missile
            pthread_mutex_lock(&mtx_missili); ///blocca il mutex @mtx_missili
            *missile = init; // rinizializzazione del missile morto
            pthread_mutex_unlock(&mtx_missili); ///sblocca il mutex @mtx_missili
            pthread_exit(NULL);
        }
        pthread_mutex_lock(&mtx_missili); ///blocca il mutex @mtx_missili
        *missile = temp_missile; // aggiorna i valori del missile
        pthread_mutex_unlock(&mtx_missili); ///sblocca il mutex @mtx_missili
        usleep(velocita_missili); //regola velocità missili
    }
}
/**
 * La funzione missili genera un movimento in diagonale verso l'alto o verso il basso in base a quale
 * missile gli viene passato in imput
 * @param arg - È un puntatore generico. In questo caso viene passata una struttura Oggetto
 * @return - void*
 */
void* bombe(void* arg) {
    Oggetto *bomba, temp_bomba;
    arg_to_bomba *argToBomba;
    argToBomba = (arg_to_bomba*) arg;
    bomba = argToBomba->bomba;

    if (bomba->index % 4 == 0) {
        sleep(1);
    } else if (bomba->index % 2 == 1) {
        sleep(3);
    } else if (bomba->index % 2 == 0) {
        sleep(5);
    }
    pthread_mutex_lock(&mtx_bombe); ///blocca il mutex @mtx_bombe
    bomba->id = ID_BOMBA;
    pthread_mutex_lock(&mtx_nemici); ///blocca il mutex @mtx_nemici
    bomba->pos.x = argToBomba->nemico->pos.x-1;
    bomba->pos.y = argToBomba->nemico->pos.y + (DIM_NEMICO/2);
    pthread_mutex_unlock(&mtx_nemici); ///sblocca il mutex @mtx_nemici
    bomba->old_pos.x = -1;
    bomba->old_pos.y = -1;
    bomba->vite = 1;
    pthread_mutex_unlock(&mtx_bombe); ///sblocca il mutex @mtx_bombe

    pthread_mutex_lock(&mtx_bombe); ///blocca il mutex @mtx_bombe
    temp_bomba = *bomba;
    pthread_mutex_unlock(&mtx_bombe); ///sblocca il mutex @mtx_bombe

    while (TRUE) {
        pthread_mutex_lock(&mtx_bombe); ///blocca il mutex @mtx_bombe
        temp_bomba.vite = bomba->vite;
        pthread_mutex_unlock(&mtx_bombe); ///sblocca il mutex @mtx_bombe
        temp_bomba.pos.x -= 1;

        if (temp_bomba.pos.x < 0 || temp_bomba.pos.y >= maxy || temp_bomba.pos.y < 0) {
            temp_bomba.vite = 0;
            pthread_mutex_lock(&mtx_bombe); ///blocca il mutex @mtx_bombe
            *bomba = temp_bomba;
            pthread_mutex_unlock(&mtx_bombe); ///sblocca il mutex @mtx_bombe
            pthread_exit(NULL);
        } else {
            pthread_mutex_lock(&mtx_bombe); ///blocca il mutex @mtx_bombe
            *bomba = temp_bomba;
            pthread_mutex_unlock(&mtx_bombe); ///sblocca il mutex @mtx_bombe
            temp_bomba.old_pos = temp_bomba.pos;
            usleep(velocita_missili); //regola velocità missili
        }
    }
}
