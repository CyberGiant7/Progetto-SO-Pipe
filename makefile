progetto_processi: main.o menu.o collisioni.o generazione_e_movimento.o stampa.o
	gcc -o progetto_processi main.o menu.o collisioni.o generazione_e_movimento.o stampa.o -lncursesw

main.o: menu.h stampa.h generazione_e_movimento.h collisioni.h main.c
	gcc -std=c99 -c main.c

menu.o: menu.h menu.c
	gcc -std=c99 -c  menu.c

collisioni.o: macro.h collisioni.h collisioni.c
	gcc -std=c99 -c  collisioni.c

generazione_e_movimento.o: macro.h generazione_e_movimento.h generazione_e_movimento.c
	gcc -std=c99 -c  generazione_e_movimento.c

stampa.o: macro.h stampa.h stampa.c
	gcc -std=c99 -c  stampa.c

clean:
	rm *.o

depencies:
	if ! dpkg -l | grep libncursesw5-dev -c >>/dev/null; then echo "osboxes.org" | sudo -S apt-get -y install libncursesw5-dev; fi
