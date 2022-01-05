/**
 * In questo file sorgente vengono definite le funzioni @collisione_missili_bombe, @collisione_bombe_navicella e
 * @collisione_missili_nemici
 */
#include "collisioni.h"
/**
 * La funzione @collisione_missili_bombe gestisce la collisione tra i missili generati dalla navicella e le bombe
 * generate dalle navicelle nemiche. I suoi paramentri in ingresso sono:
 * @param missili - È un array di strutture @Oggetto che contiene tutti i dati di ogni singolo missile generato dalla
 *                  navicella del giocatore.
 * @param bombe_nem - È un array di strutture @Oggetto che contiene tutti i dati di ogni singola bomba generata dalle
 *                    navi nemiche che si trovano i prima fila.
 */
void collisione_missili_bombe(Oggetto *missili, Oggetto *bombe_nem){
    int i, j; //indici cicli for
    pthread_mutex_lock(&mtx_missili); ///blocca il mutex @mtx_missili
    pthread_mutex_lock(&mtx_bombe); ///blocca il mutex @mtx_bombe
    /// collissione tra missili e bombe
    /** Scorre tutto l'array dei missili e delle bombe ancora in vita**/
    for (i = 0; i < MAX_MISSILI; i++) {
        if (missili[i].thread_id != 0 && missili[i].vite != 0) {
            for (j = 0; j < M; j++) {
                if (bombe_nem[j].thread_id != 0 && bombe_nem[j].vite != 0) {
                    /** Se la posizione di un missile coincide con quella di una bomba nemica**/
                    if (missili[i].pos.x == bombe_nem[j].pos.x && missili[i].pos.y == bombe_nem[j].pos.y){

                        pthread_cancel(missili[i].thread_id); //uccide il thread del missile colliso
                        missili[i].vite = 0;
                        pthread_cancel(bombe_nem[j].thread_id); //uccide il thread della bomba collisa
                        bombe_nem[j].vite = 0;
                        /* Cancellazione del missile colliso dallo schermo*/
                        attron(COLOR_PAIR(0));
                        mvaddstr(missili[i].old_pos.y, missili[i].old_pos.x, " ");
                        mvaddstr(missili[i].pos.y, missili[i].pos.x, " ");
                        attroff(COLOR_PAIR(0));
                        missili[i] = init; // la locazione del missile colliso viene rinizializzata con @init
                    }
                }
            }
        }
    }
    pthread_mutex_unlock(&mtx_missili); ///sblocca il mutex @mtx_missili
    pthread_mutex_unlock(&mtx_bombe); ///sblocca il mutex @mtx_bombe
}
/**
 * La funzione @collisione_bombe_navicella gestisce la collisione tra la navicella e le bombe
 * generate dalle navicelle nemiche. I suoi paramentri in ingresso sono:
 * @param navicella - È una struttura Oggetto che contiene tutti i dati relativi alla navicella del giocatore.
 * @param bombe_nem - È un array di strutture Oggetto che contiene tutti i dati di ogni singola bomba generata dalle
 *                    navi nemiche che si trovano i prima fila.
 */
void collisione_bombe_navicella(Oggetto *navicella, Oggetto *bombe_nem){
    int i,j,k; // indici cicli for
    pthread_mutex_lock(&mtx_nave); ///blocca il mutex @mtx_nave
    pthread_mutex_lock(&mtx_bombe); ///blocca il mutex @mtx_bombe

    ///collisione tra bombe e navicella player
    /** Scorre tutto l'array delle bombe e verifica che le bombe siano ancora in vita **/
    for (i = 0; i < M; i++) {
        if (bombe_nem[i].thread_id != 0) {
            for (j = 0; j < DIM_NAVICELLA; ++j) {
                for (k = 0; k < DIM_NAVICELLA; ++k) {
                    if (j > 0 && j<5){
                        k = 5;
                    }
                    /* Se la posizione di una bomba coincide con quella della navicella del giocatore*/
                    if(bombe_nem[i].pos.x == navicella->pos.x + k && bombe_nem[i].pos.y == navicella->pos.y + j){
                        pthread_cancel(bombe_nem[i].thread_id); //Uccide il thread della bomba collisa
                        beep(); // Genera un effetto sonoro che indica la collisione
                        bombe_nem[i].vite = 0;
                        navicella->vite--;
                    }
                }
            }
        }
    }
    pthread_mutex_unlock(&mtx_nave); ///sblocca il mutex @mtx_nave
    pthread_mutex_unlock(&mtx_bombe); ///sblocca il mutex @mtx_bombe
}
/**
 * La funzione @collisione_missili_bombe gestisce la collisione tra i missili generati dalla navicella e le bombe
 * generate dalle navicelle nemiche. I suoi paramentri in ingresso sono:
 * @param missili - È un array di strutture @Oggetto che contiene tutti i dati di ogni singolo missile generato dalla
 *                  navicella del giocatore.
 * @param enemies - È un array di strutture @Oggetto che contiene tutti i dati di ogni singola navicella nemica.
 * @param primafila - È l'indirizzo di una variabile che contiene un @_Bool
 */
void collisione_missili_nemici(Oggetto *enemies, Oggetto *missili, _Bool *primafila) {
    pthread_mutex_lock(&mtx_nemici); ///blocca il mutex @mtx_nemici
    pthread_mutex_lock(&mtx_missili); ///blocca il mutex @mtx_missili
    int i, j, k, l; // indici cicli for
    ///collisione tra Missili e Nemici
    /** Scorre tutto l'array dei nemici e dei missili ancora in vita**/
    for (i = 0; i < M; i++) {
        if (enemies[i].vite != 0) {
            for (j = 0; j < MAX_MISSILI; j++) {
                if (missili[j].vite != 0) {
                    for (k = 0; k < DIM_NEMICO; ++k) {
                        for (l = 0; l < DIM_NEMICO; ++l) {
                            /** Se la posizione di un missile coincide con quella di una navicella nemica**/
                            if (missili[j].pos.x == enemies[i].pos.x + l && missili[j].pos.y == enemies[i].pos.y + k) {
                                pthread_cancel(missili[j].thread_id); // uccide il thread del missile colliso
                                missili[j].vite = 0;
                                enemies[i].vite -= 1; // decrementa le vite della navicella nemica
                                /** Se la navicella nemica ha perso tutte le vite **/
                                if ((i - num_righe) >= 0 && enemies[i].vite == 0){
                                    primafila[i-num_righe] = TRUE; // consente alla navicella nemica che la segue a sparare le bombe**/
                                }
                                /* Cacellazione del missile colliso */
                                attron(COLOR_PAIR(0));
                                mvaddstr(missili[j].old_pos.y, missili[j].old_pos.x, " ");
                                attroff(COLOR_PAIR(0));
                                missili[j] = init; // la locazione del missile colliso viene rinizializzato con @init
                                /* Se la navicella nemica colpita ha terminato le vite */
                                if (enemies[i].vite == 0){
                                    pthread_cancel(enemies[i].thread_id); // uccide il thread della navicella nemica
                                    stampaNemico(&enemies[i]); //cancello la navicella nemica distrutta
                                    enemies[i] = init; // la locazione della navicella ditrutta viene rinizializzata con @init
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    pthread_mutex_unlock(&mtx_nemici); ///sblocca il mutex @mtx_nemici
    pthread_mutex_unlock(&mtx_missili); ///sblocca il mutex @mtx_missili
}
