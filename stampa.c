#include "stampa.h"

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
//    if (bomba->pos.x != -1 && bomba->pos.y != -1) {
    attron(COLOR_PAIR(0));
    mvprintw(bomba->old_pos.y, bomba->old_pos.x, " "); // cancella la stampa dell'ultima posizione della bomba
    attroff(COLOR_PAIR(0));
//    }
    /* Stampa la bomba nella posizione corrente*/
    attron(COLOR_PAIR(7));
    mvprintw(bomba->pos.y, bomba->pos.x, "¤"); ///♿ ⟢ ⁂ ꗇ ꗈ 💣 🚀 卐 ◌́ ◌͂ ✝
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