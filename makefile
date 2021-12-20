progetto: main.o menu.o
	gcc -o progetto main.o menu.o -lncursesw
	rm *.o

main.o: menu.h main.c
	if ! dpkg -l | grep libncursesw5-dev -c >>/dev/null; then sudo apt-get install libncursesw5-dev; fi
	if ! dpkg -l | grep libncurses5-dev -c >>/dev/null; then sudo apt-get install libncurses5-dev; fi
	gcc -std=c99 -c main.c

menu.o: menu.h menu.c
	gcc -std=c99 -c  menu.c

