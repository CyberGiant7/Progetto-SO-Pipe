#ifndef PROGETTO_SO__PIPE_GENERAZIONE_E_MOVIMENTO_H
#define PROGETTO_SO__PIPE_GENERAZIONE_E_MOVIMENTO_H

#include "macro.h"

/// Dichiarazione delle funzioni di movimento e generazione
void genera_missili(int *missili_vivi, Oggetto *rockets, int *pipe1, arg_from_navicella *argFromNavicella);
void genera_bomba(Oggetto *bombe_nem, int i, Oggetto *enemies, int *pipe1, int *pipe_to_bombe);
void nave_player(int pipeout);
void Enemy(int pipeout,Oggetto *enemy);
void missili(int pipeout, Oggetto *missile);
void bombe(int pipeout, Oggetto *bomba);

#endif //PROGETTO_SO__PIPE_GENERAZIONE_E_MOVIMENTO_H
