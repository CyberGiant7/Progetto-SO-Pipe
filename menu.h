#ifndef PROGETTO_SO___THREAD_AREA_MENU_H
#define PROGETTO_SO___THREAD_AREA_MENU_H
#include "macro.h"

/// Macro per il menu iniziale
#define MENU_ELEM 3             // Numero di elementi del menu principale
#define DEFAULT_MAXX 100        // Risoluzione di default (colonne)
#define DEFAULT_MAXY 30         // Risoluzione di default (righe)
#define ALTEZZA_TITOLO 13       // Caratteri altezza titolo
#define LARGHEZZA_TITOLO 65     // Caratteri larghezza titolo
#define ALTEZZA_GIOCA 2         // Caratteri altezza gioca
#define LARGHEZZA_GIOCA 17      // Caratteri larghezza gioca
#define ALTEZZA_OPZIONI 2       // Caratteri altezza opzioni
#define LARGHEZZA_OPZIONI 23    // Caratteri larghezza opzioni
#define ALTEZZA_ESCI 2          // Caratteri altezza esci
#define LARGHEZZA_ESCI 12       // Caratteri larghezza esci
#define MENUWIN_X 70            // Larghezza default della finestra del menu iniziale
#define MENUWIN_Y 20            // Altezza default della finestra del menu iniziale

/// Macro per le opzioni
#define NUMERO_OPZIONI 4            // Numero di opzioni modificabili
#define NUMERO_RISOLUZIONI 5        // Numero di risoluzioni modificabili
#define NUMERO_SELEZIONI_NEMICI 12  // Numero di valori di nemici selezionabili
#define NUMERO_VELOCITA_MISSILI 3   // Numero di opzioni per la modifica della velocità missili

/// Enumerazione che contiene le varie risoluzioni selezionabili
typedef enum {R_80_X_24, R_100_X_30, R_100_X_45, R_150_X_45, R_170_X_51} Risoluzioni;

/// Definizione delle funzioni
extern void stampaNemico(Oggetto enemy, int vite);
extern void stampaNavicella(Oggetto navicella);
extern void stampaMissile(Oggetto *missile);
extern void stampaBomba(Oggetto *bomba);
StatoCorrente area_menu();

StatoCorrente menu();
extern StatoCorrente gioco();
StatoCorrente opzioni();


#endif //PROGETTO_SO___THREAD_AREA_MENU_H
