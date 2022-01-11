#include "stampa.h"

/// Inizializzazione grafica della navicella del giocatore
char *nave[DIM_NAVICELLA]= {" ▟█▛▀▀",
                            "▟██▙  ",
                            "▗▟█▒▙▖",
                            "▝▜█▒▛▘",
                            "▜██▛  ",
                            " ▜█▙▄▄"};
/// Inizializzazione grafica della navicella nemica di livello 1
char *nemico_lv1[DIM_NEMICO]={"▀█▙",
                              "▒█ ",
                              "▄█▛"};
/// Inizializzazione grafica della navicella nemica di livello 2
char *nemico_lv2[DIM_NEMICO]={" △ ",
                              "◁ ◊",
                              " ▽ "};
/// Inizializzazione grafica della navicella nemica di livello 3
char *nemico_lv3[DIM_NEMICO]= {" ☠ ",
                               "☠ ☠",
                               " ☠ "};
/**
 *  La funzione @stampaNavicella stampa la navicella del giocatore nella posizione corrente e cancella dallo schermo
 *  la stampa relativa alla posizione precedente in cui si trovava la navicella. Il suo parametro in ingresso è:
 * @param navicella - È una struttura @Oggetto che contiene tutti i dati relativi alla navicella del giocatore.
 */
void stampaNavicella(Oggetto navicella) {
    int i;
    /* cancello la 'vecchia' posizione della navicella */
    if (navicella.old_pos.x >= 0) {
        attron(COLOR_PAIR(BLACK));
        for (i = 0; i < DIM_NAVICELLA; i++) {
            mvaddstr(navicella.old_pos.y + i, navicella.old_pos.x, "      ");
        }
        attroff(COLOR_PAIR(BLACK));
    }
    attron(COLOR_PAIR(GREEN));
    for (i = 0; i < DIM_NAVICELLA; i++) {
        if (i <= 1 || i >= 4) {
            attron(COLOR_PAIR(BLUE));
            mvaddstr(navicella.pos.y + i, navicella.pos.x, nave[i]);
            attroff(COLOR_PAIR(BLUE));
        } else {
            attron(COLOR_PAIR(GREEN));
            mvaddstr(navicella.pos.y + i, navicella.pos.x, nave[i]);
            attroff(COLOR_PAIR(GREEN));
        }
    }
}
/**
 *  La funzione @stampaNemico stampa le navicelle nemiche nella posizione corrente e cancella dallo schermo
 *  la stampa relativa alla posizione in cui si trovavano precedentemente. Il suo parametro in ingresso è:
 * @param enemy - È una struttura @Oggetto che contiene tutti i dati relativi alla singola navicella nemica.
 * @param vite - è il numero di vite del nemico da stampare. In base al numero delle vite viene fatta una stampa diversa
 */
void stampaNemico(Oggetto nemico, int vite){
    int i;  // indice ciclo for
    // cancello il nemico nella posizione precedente
    attron(COLOR_PAIR(BLACK));
    for (i = 0; i < DIM_NEMICO; i++) {
        mvprintw(nemico.old_pos.y + i, nemico.old_pos.x, "   ");
    }
    attroff(COLOR_PAIR(BLACK));
    switch (vite) {
        case 3:// Se ha tre vite stampa nemico livello 1
            for (i = 0; i < DIM_NEMICO; i++) {
                if (i == 1) {
                    attron(COLOR_PAIR(MAGENTA));
                    mvprintw(nemico.pos.y + i, nemico.pos.x, nemico_lv1[i]);
                    attroff(COLOR_PAIR(MAGENTA));
                } else {
                    attron(COLOR_PAIR(RED));
                    mvprintw(nemico.pos.y + i, nemico.pos.x, nemico_lv1[i]);
                    attroff(COLOR_PAIR(RED));
                }
            }
            break;
        case 2: // Se ha due vite stampa nemico livello 2
            attron(COLOR_PAIR(CYAN));
            for (i = 0; i < DIM_NEMICO; i++) {
                mvprintw(nemico.pos.y + i, nemico.pos.x, nemico_lv2[i]);
            }
            attroff(COLOR_PAIR(WHITE));
            break;
        case 1: // Se ha una vita stampa nemico livello 3
            attron(COLOR_PAIR(CYAN));
            for (i = 0; i < DIM_NEMICO; i++) {
                mvprintw(nemico.pos.y + i, nemico.pos.x, nemico_lv3[i]);
            }
            attroff(COLOR_PAIR(WHITE));
            break;
        case 0: // Se ha zero vite cancella la navicella nemica
            attron(COLOR_PAIR(BLACK));
            for (i = 0; i < DIM_NEMICO; i++) {
                mvprintw(nemico.pos.y + i, nemico.pos.x, "   ");
            }
            attroff(COLOR_PAIR(BLACK));
            break;
    }
}
void stampaBomba(Oggetto *bomba) {
    // cancella la stampa dell'ultima posizione della bomba
    attron(COLOR_PAIR(BLACK));
    mvprintw(bomba->old_pos.y, bomba->old_pos.x, " ");
    attroff(COLOR_PAIR(BLACK));

    /* Stampa la bomba nella posizione corrente*/
    attron(COLOR_PAIR(RED));
    mvprintw(bomba->pos.y, bomba->pos.x, "¤");
    attroff(COLOR_PAIR(RED));
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
        attron(COLOR_PAIR(BLACK));
        mvprintw(missile->old_pos.y, missile->old_pos.x, " ");
        attroff(COLOR_PAIR(BLACK));
    }
    attron(COLOR_PAIR(YELLOW));
    mvprintw(missile->pos.y, missile->pos.x, "⟢");
    attroff(COLOR_PAIR(YELLOW));
}