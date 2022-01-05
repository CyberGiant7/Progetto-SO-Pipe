# Progetto_SO__Pipe
Versione con i processi

Per la versione con i processi abbiamo strutturato il progetto utilizzando la funzione AreaGioco che è collegata tramite pipes ai processi figli che generano le coordinate della navicella del player, dei suoi missili, dei nemici e delle loro bombe. Da area Gioco vengono gestite tutte le collisioni e le stampe dei vari oggetti di gioco.

Versione con i thread

Per la versione con i thread abbiamo strutturato il progetto utilizzando la funzione AreaGioco che è collegata tramite variabili condivise con i thread che generano le coordinate della navicella del player, dei suoi missili, dei nemici e delle loro bombe. Ogni qual volta si accede a queste aree di memoria condivise utilizziamo dei mutex, per evitare problemi di race condition. Da area Gioco vengono gestite tutte le collisioni e le stampe dei vari oggetti di gioco.

In aggiunta alla specifiche abbiamo implementato:

Un menu principale, da cui si può iniziare il gioco, uscire o entrare nel menu opzioni.
Un menu opzioni, in cui si possono modificare il numero dei nemici, la risoluzione e la velocità di missili e bombe nemiche. Se viene cambiata la risoluzione cambia anche il numero di nemici generabili.
I suoni: in caso di vittoria, gameover, morte di un nemico, movimento nel menu e colpo ricevuto.
La collisione tra i missili del player e le bombe nemiche.
La generazione di un punteggio. Generato in base al numero di nemici uccisi e bonus vite.


Nota:
Nel makefile è presente anche un opzione da utilizzare in caso ncurses non sia installato nella macchina virtuale. Questo commando è "make dependencies". Inoltre è fortemente sconsigliato modificare manualmente la risoluzione del terminale.

MADE by Troddio & CyberGiant7
