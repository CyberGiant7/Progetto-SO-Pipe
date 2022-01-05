#include "generazione_e_movimento.h"

void Enemy(int pipeout,Oggetto *enemy){
    /// Inizializzazione variabili
    int j = 0, l = 0;   // Variabili contatori spostamento
    int temp;           // per il calcolo della posizione
    int status = 0;     // Per cambiare la direzione dello spostamento
    char nome[10];

    /// Diamo un nome al processo
    sprintf(nome, "Nemico_%d", enemy->index);
    prctl(PR_SET_NAME, (unsigned long) nome);

    /// Inizializzazione parziale dei valori della navicella nemica
    enemy->vite = 3;
    enemy->old_pos.x = -1;
    enemy->old_pos.y = -1;
    enemy->id = ID_NEMICO;
    enemy->pid = getpid();

    while (true) {
        enemy->pos.y = (1 + (enemy->index % num_righe) * (DIM_NEMICO + 1)) + j;
        temp = (int) (enemy->index / num_righe) + 1;
        enemy->pos.x = maxx - temp * (DIM_NEMICO + 1) - l;
        write(pipeout, enemy, sizeof(Oggetto));
        /*Spostamenti navicelle*/
        if (j % 1 == 0) {
            l++;
        }
        usleep(300000); //velocità spostamento nemici
        if (j <= mov_verticale && status == 0) {
            j++;
            if (j == mov_verticale)
                status = 1;
        } else if (j >= 0 && status == 1) {
            j--;
            if (j == 0)
                status = 0;
        }
        enemy->old_pos = enemy->pos;
    }
}

void nave_player(int pipeout) {
    arg_from_navicella argFromNavicella;

    argFromNavicella.navicella.id = ID_NAVICELLA;
    argFromNavicella.navicella.pos.x = 1;
    argFromNavicella.navicella.pos.y = (maxy - DIM_NAVICELLA) / 2;
    argFromNavicella.navicella.vite = 3;
    argFromNavicella.navicella.index = 0;
    argFromNavicella.navicella.pid = getpid();
    argFromNavicella.navicella.old_pos.x = 1;
    argFromNavicella.navicella.old_pos.y = (maxy - DIM_NAVICELLA) / 2;

    argFromNavicella.sparo = FALSE;

    write(pipeout, &argFromNavicella, sizeof(argFromNavicella));

    keypad(stdscr, TRUE);
    while (TRUE) {
        int c = getch();
        switch (c) {
            case KEY_UP:
                if (argFromNavicella.navicella.pos.y > 1)
                    argFromNavicella.navicella.pos.y -= 1;
                break;
            case KEY_DOWN:
                if (argFromNavicella.navicella.pos.y < maxy - DIM_NAVICELLA)
                    argFromNavicella.navicella.pos.y += 1;
                break;
            case KEY_SPACE:
                argFromNavicella.sparo = TRUE;
                break;
        }
        write(pipeout, &argFromNavicella, sizeof(argFromNavicella));
        argFromNavicella.sparo = FALSE;
        argFromNavicella.navicella.old_pos = argFromNavicella.navicella.pos;
    }
}

void genera_missili(int *missili_vivi, Oggetto *rockets, int *pipe1, arg_from_navicella *argFromNavicella){
    for (int temp = *missili_vivi + 2; *missili_vivi < temp; *missili_vivi += 1) {
        rockets[*missili_vivi].pid = fork(); //generazione processo
        rockets[*missili_vivi].pos.x = argFromNavicella->navicella.pos.x + DIM_NAVICELLA;
        rockets[*missili_vivi].pos.y = argFromNavicella->navicella.pos.y + (DIM_NAVICELLA / 2);
        rockets[*missili_vivi].old_pos.x = -1;
        rockets[*missili_vivi].old_pos.y = -1;
        rockets[*missili_vivi].vite = 1;
        rockets[*missili_vivi].index = *missili_vivi;
        switch (rockets[*missili_vivi].pid) {
            case -1:
                perror("Errore nell'esecuzione della fork.");
                exit(1);
            case 0:
                prctl(PR_SET_NAME, (unsigned long) "Missile 1");
                close(pipe1[0]);
                if (*missili_vivi % 2 == 0)
                    rockets[*missili_vivi].id = ID_MISSILE1;
                else
                    rockets[*missili_vivi].id = ID_MISSILE2;

                rockets[*missili_vivi].pid = getpid();
                missili(pipe1[1], &rockets[*missili_vivi]);
                break;
            default: //processo padre
                argFromNavicella->sparo = false;
                break;
        }
    }
}

void missili(int pipeout, Oggetto *missile){
    int i = 0;
    while (TRUE) {
        if (i % 20 == 0) {
            if (missile->id == ID_MISSILE2) {
                missile->pos.y += 1;
            } else if (missile->id == ID_MISSILE1) {
                missile->pos.y -= 1;
            }
        }
        missile->pos.x += 1;
        write(pipeout, missile, sizeof(Oggetto));
        i++;
        if ((missile->pos.x > maxx || missile->pos.y >= maxy || missile->pos.y < 0)) {
            missile->vite = 0;
            write(pipeout, missile, sizeof(Oggetto));
            exit(1);
        }
        missile->old_pos = missile->pos;
        usleep(velocita_missili); //regola velocità missili
    }
}

void genera_bomba(Oggetto *bombe_nem, int i, Oggetto *enemies, int *pipe1, int *pipe_to_bombe){
    if (bombe_nem[i].vite == 0 && bombe_nem[i].pid == 0 && enemies[i].pid != 0) { // se la bomba sparata precedentemente è morta
        /// Inizializzo parzialmente la nuova bomba
        bombe_nem[i].pid = fork();
        bombe_nem[i].vite = 1;
        switch (bombe_nem[i].pid) {
            case -1:
                perror("Errore nell'esecuzione della fork.");
                exit(1);
            case 0:
                close(pipe1[0]);
                bombe_nem[i].index = i;
                bombe_nem[i].id = ID_BOMBA;
                bombe_nem[i].old_pos.x = -1;
                bombe_nem[i].old_pos.y = -1;
                bombe_nem[i].pid = getpid();
                if (bombe_nem[i].index % 4 == 0) {
                    sleep(1);
                } else if (bombe_nem[i].index % 2 == 1) {
                    sleep(3);
                } else if (bombe_nem[i].index % 2 == 0) {
                    sleep(5);
                }
                write(pipe1[1], &bombe_nem[i], sizeof(Oggetto));
                close(pipe_to_bombe[i * 2 + 1]);
                read(pipe_to_bombe[i * 2 + 0], &enemies[i], sizeof(Oggetto));
                bombe_nem[i].pos.x = enemies[i].pos.x - 1;
                bombe_nem[i].pos.y = enemies[i].pos.y + (DIM_NEMICO / 2);
                bombe(pipe1[1], &bombe_nem[i]);
                break;
            default: //processo padre
                break;
        }
    }
}

void bombe(int pipeout, Oggetto *bomba) {
    char nome[10];
    sprintf(nome, "Bomba %d", bomba->index);
    prctl(PR_SET_NAME, (unsigned long) nome);

    while (TRUE) {
        bomba->pos.x -= 1;
        write(pipeout, bomba, sizeof(Oggetto));
        bomba->old_pos = bomba->pos;
        usleep(velocita_missili); //regola velocità missili
    }
}