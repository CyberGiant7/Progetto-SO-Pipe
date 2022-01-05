#include "collisioni.h"

void collisione_missili_bombe(Oggetto *missili, Oggetto *bombe_nem){
    int i, j;
    /// collissione tra missili e bombe
    for (i = 0; i < MAX_MISSILI; i++) {
        if (missili[i].pid != 0) {
            for (j = 0; j < M; j++) {
                if (bombe_nem[j].pid != 0 && bombe_nem[j].id == ID_BOMBA) {
                    if (missili[i].pos.x == bombe_nem[j].pos.x && missili[i].pos.y == bombe_nem[j].pos.y){
                        //uccido il missile colliso
                        kill(missili[i].pid, SIGKILL);
                        missili[i].vite = 0;

                        //uccido la bomba collisa
                        kill(bombe_nem[j].pid, SIGKILL);
                        bombe_nem[j].vite = 0;
                        attron(COLOR_PAIR(0));
                        mvaddstr(missili[i].old_pos.y, missili[i].old_pos.x, "  ");
                        mvaddstr(missili[i].pos.y, missili[i].pos.x, "  ");
                        mvaddstr(bombe_nem[j].pos.y, bombe_nem[j].pos.x, "  ");
                        mvaddstr(bombe_nem[j].old_pos.y, bombe_nem[j].old_pos.x, "  ");
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
