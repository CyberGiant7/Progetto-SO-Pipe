#define _POSIX_SOURCE
#define _DEFAULT_SOURCE
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
#include <sys/prctl.h> // momentanea, per dare nomi ai processi


#define MENU_ELEM 2

typedef enum {GIOCA, MENU, ESCI} StatoCorrente;

StatoCorrente menu();
extern StatoCorrente gioco();

#endif //PROGETTO_SO__PIPE_MENU_H
