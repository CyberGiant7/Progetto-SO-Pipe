#ifndef PROGETTO_SO__PIPE_COLLISIONI_H
#define PROGETTO_SO__PIPE_COLLISIONI_H
/**
 * In questa libreria vengono dichiarate le funzioni che gestiscono le collisioni, la funzione stampa nemico
 **/
#include "macro.h"

/// Dichiarazione funzioni per le collisioni
void genera_missili(int *missili_vivi, Oggetto *rockets, int *pipe1, arg_from_navicella *argFromNavicella);
void collisione_missili_bombe(Oggetto *missili, Oggetto *bombe_nem);
void collisione_bombe_navicella(Oggetto *navicella, Oggetto *bombe_nem);
void collisione_missili_nemici(Oggetto *enemies, Oggetto *missili, _Bool *primafila);

#endif //PROGETTO_SO__PIPE_COLLISIONI_H
