#include "gamelib.h"

int main() {
    int scelta = 0;
    int c; // Variabile di supporto per svuotare il buffer

    // Inizializza il generatore di numeri casuali utilizzando il tempo attuale
    // Funzione da chiamare una volta sola nel programma
    time_t t;
    srand((unsigned) time(&t));

    printf("\n============================================\n");
    printf("        BENVENUTO A OCCHINZ: COSESTRANE       \n");
    printf("============================================\n");

    do {
        // Stampa del menu principale
        printf("\n--- MENU PRINCIPALE ---\n");
        printf("1. Imposta gioco\n");
        printf("2. Gioca\n");
        printf("3. Termina gioco\n");
        printf("4. Visualizza i crediti\n");
        printf(">> ");

        // Controllo dell'input: se scanf non legge un numero, assegniamo un valore non valido
        if (scanf("%d", &scelta) != 1) {
            scelta = 0; 
        }

        // Svuota il buffer della tastiera per evitare problemi con input errati o "a capo" rimasti
        while ((c = getchar()) != '\n' && c != EOF);

        // Switch per gestire la scelta dell'utente
        switch (scelta) {
            case 1:
                imposta_gioco();
                break;
            case 2:
                gioca();
                break;
            case 3:
                termina_gioco();
                break;
            case 4:
                crediti();
                break;
            default:
                // Se il comando non è 1, 2, 3 o 4
                printf("\nAttenzione: Comando sbagliato! Inserisci un numero tra 1 e 4.\n");
                break;
        }

    } while (scelta != 3); // Il ciclo continua finché non si sceglie 3 (Termina gioco)

    return 0;
}
