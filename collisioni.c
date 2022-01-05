/**
 * In questo file sorgente vengono gestite le collisioni tra i vari oggetti del gioco e vengono definite le
 * funzioni @collisione_missili_bombe, @collisione_bombe_navicella e @collisione_missili_nemici
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
    /// collissione tra missili e bombe
    /** Scorre tutto l'array dei missili e delle bombe ancora in vita**/
    for (i = 0; i < MAX_MISSILI; i++) {
        if (missili[i].pid != 0) {
            for (j = 0; j < M; j++) {
                if (bombe_nem[j].pid != 0 && bombe_nem[j].id == ID_BOMBA) {
                    /** Se la posizione di un missile coincide con quella di una bomba nemica**/
                    if (missili[i].pos.x == bombe_nem[j].pos.x && missili[i].pos.y == bombe_nem[j].pos.y){
                        //uccido il missile colliso
                        kill(missili[i].pid, SIGKILL);
                        missili[i].vite = 0;


                        kill(bombe_nem[j].pid, SIGKILL);  //uccido la bomba collisa
                        bombe_nem[j].vite = 0;
                        attron(COLOR_PAIR(0));
                        /* Cancellazione del missile colliso dallo schermo*/
                        mvaddstr(missili[i].old_pos.y, missili[i].old_pos.x, "  ");
                        mvaddstr(missili[i].pos.y, missili[i].pos.x, "  ");
                        /* Cancellazione della bomba collisa dallo schermo*/
                        mvaddstr(bombe_nem[j].pos.y, bombe_nem[j].pos.x, "  ");
                        mvaddstr(bombe_nem[j].old_pos.y, bombe_nem[j].old_pos.x, "  ");
                        attroff(COLOR_PAIR(0));
                        missili[i] = init; // la locazione del missile colliso viene reinizializzata con @init
                        bombe_nem[j] = init; // la locazione della bomba collisa viene reinizializzata con @init
                    }
                }
            }
        }
    }
}
/**
 * La funzione @collisione_bombe_navicella gestisce la collisione tra la navicella e le bombe
 * generate dalle navicelle nemiche. I suoi paramentri in ingresso sono:
 * @param navicella - È una struttura @Oggetto che contiene tutti i dati relativi alla navicella del giocatore.
 * @param bombe_nem - È un array di strutture @Oggetto che contiene tutti i dati di ogni singola bomba generata dalle
 *                    navi nemiche che si trovano i prima fila.
 */
void collisione_bombe_navicella(Oggetto *navicella, Oggetto *bombe_nem) {
    int i, j, k; // indici cicli for
    ///collisione tra bombe e navicella player
    /** Scorre tutto l'array delle bombe e verifica che le bombe siano ancora in vita **/
    for (i = 0; i < M; i++) {
        if (bombe_nem[i].pid != 0) {
            for (j = 0; j < DIM_NAVICELLA; ++j) {
                for (k = 0; k < DIM_NAVICELLA; ++k) {
                    if (j > 0 && j < 5) {
                        k = 5;
                    }
                    /* Se la posizione di una bomba coincide con quella della navicella del giocatore*/
                    if (bombe_nem[i].pos.x == navicella->pos.x + k && bombe_nem[i].pos.y == navicella->pos.y + j) {
                        kill(bombe_nem[i].pid, SIGKILL);  // uccido la bomba collisa
                        beep();                           // Genera un effetto sonoro che indica la collisione
                        navicella->vite--; // decrementa le vite della nave player
                        /* Cancellazione della bomba collisa dallo schermo*/
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

/**
 * La funzione @collisione_missili_bombe gestisce la collisione tra i missili generati dalla navicella e le bombe
 * generate dalle navicelle nemiche. I suoi paramentri in ingresso sono:
 * @param missili - È un array di strutture @Oggetto che contiene tutti i dati di ogni singolo missile generato dalla
 *                  navicella del giocatore.
 * @param enemies - È un array di strutture @Oggetto che contiene tutti i dati di ogni singola navicella nemica.
 * @param primafila - È l'indirizzo di una variabile che contiene un @_Bool
 */
void collisione_missili_nemici(Oggetto *enemies, Oggetto *missili, _Bool *primafila) {
    int i, j, k, l; // indici cicli for
    ///collisione tra Missili e Nemici
    for (i = 0; i < M; i++) {
        if (enemies[i].pid != 0) {
            for (j = 0; j < MAX_MISSILI; j++) {
                if (missili[j].pid != 0) {
                    for (k = 0; k < DIM_NEMICO; ++k) {
                        for (l = 0; l < DIM_NEMICO; ++l) {
                            /** Se la posizione di un missile coincide con quella di una navicella nemica**/
                            if (missili[j].pos.x == enemies[i].pos.x + l && missili[j].pos.y == enemies[i].pos.y + k) {
                                kill(missili[j].pid, SIGKILL); // uccido il processo della navicella nemica distrutta
                                missili[j].vite = 0;
                                enemies[i].vite -= 1;          // decrementa le vite della navicella nemica
                                // consente alla navicella nemica che la segue a sparare le bombe
                                if ((i - num_righe) >= 0 && enemies[i].vite == 0){
                                    primafila[i-num_righe] = TRUE;
                                }
                                /* Cacellazione del missile colliso */
                                attron(COLOR_PAIR(0));
                                mvaddstr(missili[j].pos.y, missili[j].pos.x, " ");
                                attroff(COLOR_PAIR(0));
                                missili[j] = init; // la locazione del missile colliso viene rinizializzato con @init
                                if (enemies[i].vite == 0){
                                    enemies[i] = init; // la locazione della navicella ditrutta viene rinizializzata con @init
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
