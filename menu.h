#ifndef PROGETTO_SO__PIPE_MENU_H
#define PROGETTO_SO__PIPE_MENU_H

#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <locale.h>
#include <fcntl.h>


#define MENU_ELEM 2

typedef enum {GIOCA, MENU, ESCI} StatoCorrente;

StatoCorrente menu();
extern StatoCorrente gioco();

#endif //PROGETTO_SO__PIPE_MENU_H
