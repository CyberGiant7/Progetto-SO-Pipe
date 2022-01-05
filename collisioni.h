#ifndef PROGETTO_SO___THREAD_COLLISIONI_H
#define PROGETTO_SO___THREAD_COLLISIONI_H
/**
 * In questa libreria vengono dichiare le funzioni che gestiscono le collisioni, la funzione stampa nemico
 **/
#include "macro.h"

void collisione_missili_nemici(Oggetto *enemies, Oggetto *missili, _Bool *primafila);
void collisione_missili_bombe(Oggetto *missili, Oggetto *bombe_nem);
void collisione_bombe_navicella(Oggetto *navicella, Oggetto *bombe_nem);

///Dichiarazione della funzione stampaNemico definita in
extern void stampaNemico(Oggetto *enemy);

#endif //PROGETTO_SO___THREAD_COLLISIONI_H
