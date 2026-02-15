#include "gamelib.h"

// --- VARIABILI GLOBALI (Statiche) ---
static struct Zona_mondoreale* prima_zona_mondoreale = NULL;
static struct Zona_soprasotto* prima_zona_soprasotto = NULL;
static struct Giocatore* giocatori[4] = {NULL, NULL, NULL, NULL};

static int numero_giocatori = 0;
static int gioco_impostato = 0; 
static int undici_scelto = 0;   

// --- PROTOTIPI DELLE FUNZIONI STATICHE ---
static void pulisci_buffer();
static int lancia_dado(int facce);
static void pulisci_memoria();
static int conta_zone();
static int conta_demotorzone();
static void svuota_mappa();
static const char* nome_zona(Tipo_zona t);
static const char* nome_nemico(Tipo_nemico n);
static const char* nome_oggetto(Tipo_oggetto o);
static void genera_mappa();
static void inserisci_zona();
static void cancella_zona();
static void stampa_mappa();
static void chiudi_mappa();
static void menu_creazione_mappa();
static int presenza_nemico(struct Giocatore* g);
static void avanza(struct Giocatore* g, int* ha_avanzato);
static void indietreggia(struct Giocatore* g);
static void cambia_mondo(struct Giocatore* g, int* ha_avanzato);
static void passa(int* turno_finito);
static void combatti(struct Giocatore* g);
static void stampa_giocatore(struct Giocatore* g);
static void stampa_zona(struct Giocatore* g);
static void raccogli_oggetto(struct Giocatore* g);
static void utilizza_oggetto(struct Giocatore* g);

// --- UTILITY ---
static void pulisci_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static int lancia_dado(int facce) {
    return (rand() % facce) + 1;
}

static void pulisci_memoria() {
    for (int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            free(giocatori[i]);
            giocatori[i] = NULL;
        }
    }
    numero_giocatori = 0;
    undici_scelto = 0;

    struct Zona_mondoreale* corrente_mr = prima_zona_mondoreale;
    while (corrente_mr != NULL) {
        struct Zona_mondoreale* temp = corrente_mr;
        corrente_mr = corrente_mr->avanti;
        free(temp);
    }
    prima_zona_mondoreale = NULL;

    struct Zona_soprasotto* corrente_ss = prima_zona_soprasotto;
    while (corrente_ss != NULL) {
        struct Zona_soprasotto* temp = corrente_ss;
        corrente_ss = corrente_ss->avanti;
        free(temp);
    }
    prima_zona_soprasotto = NULL;

    gioco_impostato = 0;
    printf("Memoria pulita correttamente.\n");
}

// --- FUNZIONI PUBBLICHE (main) ---
void termina_gioco() {
    pulisci_memoria();
    printf("\nGrazie per aver giocato a Cosestrane!\n");
    printf("Uscita in corso...\n");
    exit(0);
}

void crediti() {
    printf("\n---------------- CREDITI ----------------\n");
    printf("Sviluppato da: Samuel Vasalie, Matricola: 381587\n");
    printf("Esame di Programmazione Procedurale 2025/2026\n");
    printf("Progetto: OCCHINZ - COSE STRANE\n");
    printf("-----------------------------------------\n");
}

void imposta_gioco() {
    if (gioco_impostato || giocatori[0] != NULL) {
        printf("Rilevata una partita precedente. Pulizia in corso...\n");
        pulisci_memoria();
    }

    printf("\n--- IMPOSTAZIONE GIOCO ---\n");
    do {
        printf("Inserisci il numero di giocatori (1-4): ");
        if (scanf("%d", &numero_giocatori) != 1) {
            numero_giocatori = 0;
            pulisci_buffer();
        } else {
            pulisci_buffer();
        }
        if (numero_giocatori < 1 || numero_giocatori > 4) {
            printf("Errore: Il numero deve essere compreso tra 1 e 4.\n");
        }
    } while (numero_giocatori < 1 || numero_giocatori > 4);

    for (int i = 0; i < numero_giocatori; i++) {
        giocatori[i] = (struct Giocatore*) malloc(sizeof(struct Giocatore));
        if (giocatori[i] == NULL) {
            printf("Errore critico: Memoria insufficiente per allocare il giocatore %d.\n", i + 1);
            exit(1);
        }

        printf("\n--- Configurazione Giocatore %d ---\n", i + 1);
        printf("Inserisci il nome del giocatore: ");
        if (fgets(giocatori[i]->nome, 30, stdin) != NULL) {
            size_t len = strlen(giocatori[i]->nome);
            if (len > 0 && giocatori[i]->nome[len - 1] == '\n') {
                giocatori[i]->nome[len - 1] = '\0';
            }
        }

        giocatori[i]->attacco_psichico = lancia_dado(20);
        giocatori[i]->difesa_psichica = lancia_dado(20);
        giocatori[i]->fortuna = lancia_dado(20);
        giocatori[i]->mondo = 0;
        giocatori[i]->stato = vivo;
        giocatori[i]->pos_mondoreale = NULL;
        giocatori[i]->pos_soprasotto = NULL;
        for(int k=0; k<3; k++) giocatori[i]->zaino[k] = nessun_oggetto;

        int scelta_classe = 0;
        int conferma = 0;

        do {
            printf("\nStatistiche attuali:\n");
            printf("- Attacco Psichico: %d\n", giocatori[i]->attacco_psichico);
            printf("- Difesa Psichica:  %d\n", giocatori[i]->difesa_psichica);
            printf("- Fortuna:          %d\n", giocatori[i]->fortuna);

            printf("\nScegli una modifica:\n");
            printf("1. Nessuna modifica (Mantieni questi valori)\n");
            printf("2. Attacco +3, Difesa -3\n");
            printf("3. Difesa +3, Attacco -3\n");
            if (undici_scelto == 0) {
                printf("4. Diventa 'Undici VirgolaCinque' (+4 Att/+4 Dif/-7 Fortuna)\n");
            }

            printf(">> ");
            scanf("%d", &scelta_classe);
            pulisci_buffer();
            conferma = 1;

            switch (scelta_classe) {
                case 1: printf("Hai scelto di mantenere le statistiche.\n"); break;
                case 2:
                    giocatori[i]->attacco_psichico += 3;
                    if(giocatori[i]->difesa_psichica >= 3) giocatori[i]->difesa_psichica -= 3;
                    else giocatori[i]->difesa_psichica = 0;
                    break;
                case 3:
                    giocatori[i]->difesa_psichica += 3;
                    if(giocatori[i]->attacco_psichico >= 3) giocatori[i]->attacco_psichico -= 3;
                    else giocatori[i]->attacco_psichico = 0;
                    break;
                case 4:
                    if (undici_scelto == 0) {
                        giocatori[i]->attacco_psichico += 4;
                        giocatori[i]->difesa_psichica += 4;
                        if(giocatori[i]->fortuna >= 7) giocatori[i]->fortuna -= 7;
                        else giocatori[i]->fortuna = 0;
                        char nuovo_nome[50];
                        snprintf(nuovo_nome, 50, "Undici_%s", giocatori[i]->nome);
                        strncpy(giocatori[i]->nome, nuovo_nome, 30);
                        undici_scelto = 1;
                        printf("Sei diventato Undici VirgolaCinque!\n");
                    } else {
                        printf("Scelta non valida.\n");
                        conferma = 0;
                    }
                    break;
                default:
                    printf("Scelta non valida.\n");
                    conferma = 0;
            }
        } while (conferma == 0);
    }
    printf("\nGiocatori creati. Ora procediamo alla creazione della mappa.\n");
    menu_creazione_mappa(); 
}

// --- FUNZIONI DI SUPPORTO PER LA MAPPA ---
static int conta_zone() {
    int count = 0;
    struct Zona_mondoreale* cur = prima_zona_mondoreale;
    while (cur != NULL) { count++; cur = cur->avanti; }
    return count;
}

static int conta_demotorzone() {
    int count = 0;
    struct Zona_soprasotto* cur = prima_zona_soprasotto;
    while (cur != NULL) {
        if (cur->nemico == demotorzone) count++;
        cur = cur->avanti;
    }
    return count;
}

static void svuota_mappa() {
    struct Zona_mondoreale* cur_mr = prima_zona_mondoreale;
    while (cur_mr != NULL) {
        struct Zona_mondoreale* temp = cur_mr;
        cur_mr = cur_mr->avanti;
        free(temp);
    }
    prima_zona_mondoreale = NULL;

    struct Zona_soprasotto* cur_ss = prima_zona_soprasotto;
    while (cur_ss != NULL) {
        struct Zona_soprasotto* temp = cur_ss;
        cur_ss = cur_ss->avanti;
        free(temp);
    }
    prima_zona_soprasotto = NULL;
}

static const char* nome_zona(Tipo_zona t) {
    const char* nomi[] = {"Bosco", "Scuola", "Laboratorio", "Caverna", "Strada", 
                          "Giardino", "Supermercato", "Centrale Elettrica", 
                          "Deposito Abbandonato", "Stazione Polizia"};
    return nomi[t];
}

static const char* nome_nemico(Tipo_nemico n) {
    const char* nomi[] = {"Nessun Nemico", "Billi", "Democane", "Demotorzone"};
    return nomi[n];
}

static const char* nome_oggetto(Tipo_oggetto o) {
    const char* nomi[] = {"Nessun Oggetto", "Bicicletta", "Maglietta Fuocoinferno", 
                          "Bussola", "Schitarrata Metallica"};
    return nomi[o];
}

// --- CREAZIONE E GESTIONE MAPPA ---
static void genera_mappa() {
    svuota_mappa();
    struct Zona_mondoreale* ultimo_mr = NULL;
    struct Zona_soprasotto* ultimo_ss = NULL;
    int pos_demotorzone = rand() % 15;

    for (int i = 0; i < 15; i++) {
        struct Zona_mondoreale* nuovo_mr = (struct Zona_mondoreale*) malloc(sizeof(struct Zona_mondoreale));
        struct Zona_soprasotto* nuovo_ss = (struct Zona_soprasotto*) malloc(sizeof(struct Zona_soprasotto));

        Tipo_zona tipo_casuale = (Tipo_zona)(rand() % 10);
        nuovo_mr->tipo = tipo_casuale;
        nuovo_ss->tipo = tipo_casuale;
        nuovo_mr->oggetto = (Tipo_oggetto)(rand() % 5);
        nuovo_mr->nemico = (Tipo_nemico)(rand() % 3); 

        if (i == pos_demotorzone) nuovo_ss->nemico = demotorzone;
        else {
            int r = rand() % 2; 
            nuovo_ss->nemico = (r == 0) ? nessun_nemico : democane;
        }

        nuovo_mr->link_soprasotto = nuovo_ss;
        nuovo_ss->link_mondoreale = nuovo_mr;
        nuovo_mr->avanti = NULL;
        nuovo_ss->avanti = NULL;

        if (prima_zona_mondoreale == NULL) {
            nuovo_mr->indietro = NULL;
            nuovo_ss->indietro = NULL;
            prima_zona_mondoreale = nuovo_mr;
            prima_zona_soprasotto = nuovo_ss;
        } else {
            nuovo_mr->indietro = ultimo_mr;
            nuovo_ss->indietro = ultimo_ss;
            ultimo_mr->avanti = nuovo_mr;
            ultimo_ss->avanti = nuovo_ss;
        }
        ultimo_mr = nuovo_mr;
        ultimo_ss = nuovo_ss;
    }
    printf("\nGenerazione completata: 15 zone create con successo!\n");
}

static void inserisci_zona() {
    int pos;
    int num_zone = conta_zone();
    
    printf("\n--- INSERISCI NUOVA ZONA ---\n");
    printf("Zone attuali: %d. Inserisci la posizione (da 0 a %d): ", num_zone, num_zone);
    if (scanf("%d", &pos) != 1) { pos = -1; }
    pulisci_buffer();

    if (pos < 0 || pos > num_zone) {
        printf("Posizione non valida! Operazione annullata.\n");
        return;
    }

    struct Zona_mondoreale* nuovo_mr = (struct Zona_mondoreale*) malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nuovo_ss = (struct Zona_soprasotto*) malloc(sizeof(struct Zona_soprasotto));

    nuovo_mr->tipo = (Tipo_zona)(rand() % 10);
    nuovo_ss->tipo = nuovo_mr->tipo;

    int scelta_ogg, scelta_nemico_mr, scelta_nemico_ss;
    do {
        printf("Scegli Oggetto MR (0=Nessuno, 1=Bici, 2=Maglietta, 3=Bussola, 4=Chitarra): ");
        if (scanf("%d", &scelta_ogg) != 1) { scelta_ogg = -1; pulisci_buffer(); }
        else { pulisci_buffer(); }
    } while (scelta_ogg < 0 || scelta_ogg > 4);
    nuovo_mr->oggetto = (Tipo_oggetto)scelta_ogg;

    do {
        printf("Scegli Nemico MR (0=Nessuno, 1=Billi, 2=Democane): ");
        if (scanf("%d", &scelta_nemico_mr) != 1) { scelta_nemico_mr = -1; pulisci_buffer(); }
        else { pulisci_buffer(); }
    } while (scelta_nemico_mr < 0 || scelta_nemico_mr > 2);
    nuovo_mr->nemico = (Tipo_nemico)scelta_nemico_mr;

    do {
        printf("Scegli Nemico SS (0=Nessuno, 2=Democane, 3=Demotorzone): ");
        if (scanf("%d", &scelta_nemico_ss) != 1) { scelta_nemico_ss = -1; pulisci_buffer(); }
        else { pulisci_buffer(); }
    } while (scelta_nemico_ss != 0 && scelta_nemico_ss != 2 && scelta_nemico_ss != 3);
    nuovo_ss->nemico = (Tipo_nemico)scelta_nemico_ss;

    nuovo_mr->link_soprasotto = nuovo_ss;
    nuovo_ss->link_mondoreale = nuovo_mr;

    if (pos == 0) {
        nuovo_mr->indietro = NULL;
        nuovo_ss->indietro = NULL;
        nuovo_mr->avanti = prima_zona_mondoreale;
        nuovo_ss->avanti = prima_zona_soprasotto;
        if (prima_zona_mondoreale != NULL) {
            prima_zona_mondoreale->indietro = nuovo_mr;
            prima_zona_soprasotto->indietro = nuovo_ss;
        }
        prima_zona_mondoreale = nuovo_mr;
        prima_zona_soprasotto = nuovo_ss;
    } else {
        struct Zona_mondoreale* cur_mr = prima_zona_mondoreale;
        struct Zona_soprasotto* cur_ss = prima_zona_soprasotto;
        for (int i = 0; i < pos - 1; i++) {
            cur_mr = cur_mr->avanti;
            cur_ss = cur_ss->avanti;
        }
        nuovo_mr->avanti = cur_mr->avanti;
        nuovo_ss->avanti = cur_ss->avanti;
        nuovo_mr->indietro = cur_mr;
        nuovo_ss->indietro = cur_ss;
        if (cur_mr->avanti != NULL) {
            cur_mr->avanti->indietro = nuovo_mr;
            cur_ss->avanti->indietro = nuovo_ss;
        }
        cur_mr->avanti = nuovo_mr;
        cur_ss->avanti = nuovo_ss;
    }
    printf("\nZona inserita in posizione %d!\n", pos);
}

static void cancella_zona() {
    int num_zone = conta_zone();
    if (num_zone == 0) {
        printf("\nLa mappa e' vuota!\n");
        return;
    }

    int pos;
    printf("\nInserisci la posizione da cancellare (da 0 a %d): ", num_zone - 1);
    if (scanf("%d", &pos) != 1) { pos = -1; }
    pulisci_buffer();

    if (pos < 0 || pos >= num_zone) {
        printf("Posizione non valida!\n");
        return;
    }

    struct Zona_mondoreale* cur_mr = prima_zona_mondoreale;
    struct Zona_soprasotto* cur_ss = prima_zona_soprasotto;

    for (int i = 0; i < pos; i++) {
        cur_mr = cur_mr->avanti;
        cur_ss = cur_ss->avanti;
    }

    if (cur_mr->indietro == NULL) {
        prima_zona_mondoreale = cur_mr->avanti;
        prima_zona_soprasotto = cur_ss->avanti;
        if (prima_zona_mondoreale != NULL) {
            prima_zona_mondoreale->indietro = NULL;
            prima_zona_soprasotto->indietro = NULL;
        }
    } else {
        cur_mr->indietro->avanti = cur_mr->avanti;
        cur_ss->indietro->avanti = cur_ss->avanti;
    }

    if (cur_mr->avanti != NULL) {
        cur_mr->avanti->indietro = cur_mr->indietro;
        cur_ss->avanti->indietro = cur_ss->indietro;
    }

    free(cur_mr);
    free(cur_ss);
    printf("\nZona %d cancellata!\n", pos);
}

static void stampa_mappa() {
    if (prima_zona_mondoreale == NULL) {
        printf("La mappa e' vuota.\n");
        return;
    }

    int scelta;
    printf("\nQuale mappa stampare?\n1. Mondo Reale\n2. Soprasotto\n>> ");
    scanf("%d", &scelta);
    pulisci_buffer();

    int i = 0;
    if (scelta == 1) {
        printf("\n--- MAPPA MONDO REALE ---\n");
        struct Zona_mondoreale* cur = prima_zona_mondoreale;
        while (cur != NULL) {
            printf("[%d] %s | Nemico: %s | Oggetto: %s\n", 
                   i, nome_zona(cur->tipo), nome_nemico(cur->nemico), nome_oggetto(cur->oggetto));
            cur = cur->avanti;
            i++;
        }
    } else if (scelta == 2) {
        printf("\n--- MAPPA SOPRASOTTO ---\n");
        struct Zona_soprasotto* cur = prima_zona_soprasotto;
        while (cur != NULL) {
            printf("[%d] %s | Nemico: %s\n", i, nome_zona(cur->tipo), nome_nemico(cur->nemico));
            cur = cur->avanti;
            i++;
        }
    } else {
        printf("Scelta non valida.\n");
    }
}

static void chiudi_mappa() {
    int num_zone = conta_zone();
    int num_demo = conta_demotorzone();

    if (num_zone < 15) {
        printf("\nErrore: La mappa deve avere almeno 15 zone (sono %d).\n", num_zone);
    } else if (num_demo != 1) {
        printf("\nErrore: Ci deve essere esattamente UN Demotorzone nel Soprasotto (sono %d).\n", num_demo);
    } else {
        gioco_impostato = 1;
        for(int i=0; i<numero_giocatori; i++) {
            giocatori[i]->pos_mondoreale = prima_zona_mondoreale;
            giocatori[i]->pos_soprasotto = prima_zona_soprasotto;
            giocatori[i]->mondo = 0; 
        }
        printf("\nMappa validata e chiusa con successo!\n");
    }
}

static void menu_creazione_mappa() {
    int scelta = 0;
    do {
        printf("\n--- MENU CREAZIONE MAPPA ---\n");
        printf("Zone attuali: %d | Demotorzone: %d\n", conta_zone(), conta_demotorzone());
        printf("1. Genera Mappa Casuale\n");
        printf("2. Inserisci Zona (Manuale)\n");
        printf("3. Cancella Zona (Manuale)\n");
        printf("4. Stampa Mappa\n");
        printf("5. Chiudi Mappa\n");
        printf(">> ");
        
        if (scanf("%d", &scelta) != 1) { scelta = 0; }
        pulisci_buffer();

        switch (scelta) {
            case 1: genera_mappa(); break;
            case 2: inserisci_zona(); break;
            case 3: cancella_zona(); break;
            case 4: stampa_mappa(); break;
            case 5: chiudi_mappa(); break;
            default: printf("Comando non valido.\n");
        }
    } while (scelta != 5 || gioco_impostato == 0); 
}

// --- AZIONI DI GIOCO ---
static int presenza_nemico(struct Giocatore* g) {
    if (g->mondo == 0) return (g->pos_mondoreale->nemico != nessun_nemico);
    else return (g->pos_soprasotto->nemico != nessun_nemico);
}

static void avanza(struct Giocatore* g, int* ha_avanzato) {
    if (*ha_avanzato) {
        printf("\nHai gia' usato la tua mossa di avanzamento!\n");
        return;
    }
    if (presenza_nemico(g)) {
        printf("\nImpossibile avanzare: sconfiggi prima il nemico!\n");
        return;
    }

    if (g->mondo == 0) {
        if (g->pos_mondoreale->avanti != NULL) {
            g->pos_mondoreale = g->pos_mondoreale->avanti;
            g->pos_soprasotto = g->pos_soprasotto->avanti;
            *ha_avanzato = 1;
            printf("\nAvanzi in: %s\n", nome_zona(g->pos_mondoreale->tipo));
        } else printf("\nSei all'ultima zona!\n");
    } else {
        if (g->pos_soprasotto->avanti != NULL) {
            g->pos_soprasotto = g->pos_soprasotto->avanti;
            g->pos_mondoreale = g->pos_mondoreale->avanti;
            *ha_avanzato = 1;
            printf("\nAvanzi in: %s\n", nome_zona(g->pos_soprasotto->tipo));
        } else printf("\nSei all'ultima zona!\n");
    }
}

static void indietreggia(struct Giocatore* g) {
    if (presenza_nemico(g)) {
        printf("\nImpossibile indietreggiare: sconfiggi prima il nemico!\n");
        return;
    }

    if (g->mondo == 0) {
        if (g->pos_mondoreale->indietro != NULL) {
            g->pos_mondoreale = g->pos_mondoreale->indietro;
            g->pos_soprasotto = g->pos_soprasotto->indietro;
            printf("\nSei indietreggiato.\n");
        } else printf("\nSei alla prima zona!\n");
    } else {
        if (g->pos_soprasotto->indietro != NULL) {
            g->pos_soprasotto = g->pos_soprasotto->indietro;
            g->pos_mondoreale = g->pos_mondoreale->indietro;
            printf("\nSei indietreggiato.\n");
        } else printf("\nSei alla prima zona!\n");
    }
}

static void cambia_mondo(struct Giocatore* g, int* ha_avanzato) {
    if (g->mondo == 0) {
        if (*ha_avanzato) {
            printf("\nHai gia' avanzato in questo turno!\n");
            return;
        }
        if (presenza_nemico(g)) {
            printf("\nImpossibile usare il portale con un nemico presente!\n");
            return;
        }
        g->mondo = 1;
        *ha_avanzato = 1;
        printf("\n*BZZZ* Sei entrato nel Soprasotto!\n");
    } else {
        printf("\nTenti la fuga (Serve D20 < %d)...\n", g->fortuna);
        int dado = lancia_dado(20);
        printf("Hai rollato: %d\n", dado);
        if (dado < g->fortuna) {
            g->mondo = 0;
            printf("Fuga riuscita! Sei nel Mondo Reale.\n");
        } else {
            printf("Fuga fallita!\n");
        }
    }
}

static void passa(int* turno_finito) {
    printf("\nHai passato il turno.\n");
    *turno_finito = 1;
}

static void stampa_giocatore(struct Giocatore* g) {
    printf("\n--- INFO GIOCATORE ---\n");
    printf("Nome: %s\n", g->nome);
    printf("Dimensione: %s\n", g->mondo == 0 ? "Mondo Reale" : "Soprasotto");
    printf("Attacco: %d | Difesa(HP): %d | Fortuna: %d\n", g->attacco_psichico, g->difesa_psichica, g->fortuna);
    printf("Zaino: [%s, %s, %s]\n", nome_oggetto(g->zaino[0]), nome_oggetto(g->zaino[1]), nome_oggetto(g->zaino[2]));
}

static void stampa_zona(struct Giocatore* g) {
    printf("\n--- INFO ZONA ---\n");
    if (g->mondo == 0) {
        printf("Mondo Reale | Zona: %s | Nemico: %s | Oggetto: %s\n", 
            nome_zona(g->pos_mondoreale->tipo), nome_nemico(g->pos_mondoreale->nemico), nome_oggetto(g->pos_mondoreale->oggetto));
    } else {
        printf("Soprasotto | Zona: %s | Nemico: %s\n", 
            nome_zona(g->pos_soprasotto->tipo), nome_nemico(g->pos_soprasotto->nemico));
    }
}

static void raccogli_oggetto(struct Giocatore* g) {
    if (g->mondo == 1) {
        printf("\nNiente oggetti nel Soprasotto!\n");
        return;
    }
    if (g->pos_mondoreale->nemico != nessun_nemico) {
        printf("\nSconfiggi prima il nemico!\n");
        return;
    }
    if (g->pos_mondoreale->oggetto == nessun_oggetto) {
        printf("\nNessun oggetto qui.\n");
        return;
    }
    
    for (int i = 0; i < 3; i++) {
        if (g->zaino[i] == nessun_oggetto) {
            g->zaino[i] = g->pos_mondoreale->oggetto;
            printf("\nRaccolto: %s!\n", nome_oggetto(g->pos_mondoreale->oggetto));
            g->pos_mondoreale->oggetto = nessun_oggetto;
            return;
        }
    }
    printf("\nZaino pieno!\n");
}

static void utilizza_oggetto(struct Giocatore* g) {
    printf("\nScegli oggetto da usare (1-3, 0=Annulla):\n");
    for (int i = 0; i < 3; i++) printf("%d. %s\n", i + 1, nome_oggetto(g->zaino[i]));
    
    int scelta;
    if (scanf("%d", &scelta) != 1) { scelta = 0; pulisci_buffer(); return; }
    pulisci_buffer();
    
    if (scelta >= 1 && scelta <= 3 && g->zaino[scelta - 1] != nessun_oggetto) {
        Tipo_oggetto ogg = g->zaino[scelta - 1];
        switch (ogg) {
            case bicicletta: g->fortuna += 5; printf("Bici usata! Fortuna +5.\n"); break;
            case maglietta_fuocoinferno: g->difesa_psichica += 10; printf("Maglietta usata! Difesa +10.\n"); break;
            case bussola: g->attacco_psichico += 3; g->fortuna += 3; printf("Bussola usata! Attacco +3, Fortuna +3.\n"); break;
            case schitarrata_metallica: g->attacco_psichico += 10; printf("Chitarra usata! Attacco +10.\n"); break;
            default: break;
        }
        g->zaino[scelta - 1] = nessun_oggetto; 
    }
}

static void combatti(struct Giocatore* g) {
    Tipo_nemico* nemico_ptr = (g->mondo == 0) ? &(g->pos_mondoreale->nemico) : &(g->pos_soprasotto->nemico);
    if (*nemico_ptr == nessun_nemico) {
        printf("\nNessun nemico qui.\n");
        return;
    }
    
    printf("\n--- COMBATTIMENTO CONTRO: %s ---\n", nome_nemico(*nemico_ptr));
    int hp_nemico = 0, atk_nemico = 0;
    switch (*nemico_ptr) {
        case billi: hp_nemico = 20; atk_nemico = 5; break;
        case democane: hp_nemico = 40; atk_nemico = 10; break;
        case demotorzone: hp_nemico = 80; atk_nemico = 15; break;
        default: break;
    }

    while (hp_nemico > 0 && g->stato == vivo) {
        printf("\nTuoi HP(Difesa): %d | HP Nemico: %d\n", g->difesa_psichica, hp_nemico);
        printf("1. Attacca | 2. Usa Oggetto | 3. Fuga\n>> ");
        
        int azione;
        if (scanf("%d", &azione) != 1) { azione = 0; pulisci_buffer(); }
        pulisci_buffer();

        if (azione == 2) { utilizza_oggetto(g); continue; } 
        else if (azione == 3) {
            if (lancia_dado(20) < g->fortuna) {
                printf("Fuga riuscita!\n"); return;
            } else printf("Fuga fallita!\n");
        } else if (azione == 1) {
            int attacco = lancia_dado(20) + g->attacco_psichico;
            printf("Attacchi con %d danni!\n", attacco);
            hp_nemico -= attacco;
        }

        if (hp_nemico > 0) {
            int atk_subito = lancia_dado(20) + atk_nemico;
            if (lancia_dado(20) < (g->fortuna / 2)) {
                printf("Schivato!\n");
            } else {
                printf("Subisci %d danni!\n", atk_subito);
                if (atk_subito >= g->difesa_psichica) {
                    g->difesa_psichica = 0;
                    g->stato = morto;
                    printf("\n*** SEI MORTO! ***\n");
                } else g->difesa_psichica -= atk_subito;
            }
        }
    }

    if (g->stato == vivo && hp_nemico <= 0) {
        printf("\nHai sconfitto il %s!\n", nome_nemico(*nemico_ptr));
        if (*nemico_ptr == demotorzone) {
            printf("\n*** HAI VINTO LA PARTITA! ***\n");
            termina_gioco();
        }
        if (lancia_dado(100) <= 50) *nemico_ptr = nessun_nemico;
    }
}

// --- LOOP PRINCIPALE ---
void gioca() {
    if (!gioco_impostato || numero_giocatori == 0) {
        printf("\nErrore: Imposta e chiudi la mappa prima!\n");
        return;
    }

    int vittoria = 0, round = 1;
    printf("\n*** INIZIO PARTITA ***\n");

    while (!vittoria) {
        int ordine[4], attivi = 0;
        for (int i = 0; i < numero_giocatori; i++) {
            if (giocatori[i]->stato == vivo) { ordine[attivi] = i; attivi++; }
        }

        if (attivi == 0) {
            printf("\n*** TUTTI MORTI. GAME OVER. ***\n");
            break;
        }

        for (int i = 0; i < attivi; i++) {
            int r = i + rand() % (attivi - i);
            int temp = ordine[i]; ordine[i] = ordine[r]; ordine[r] = temp;
        }

        printf("\n--- ROUND %d ---\n", round);
        for (int i = 0; i < attivi; i++) {
            struct Giocatore* g_corr = giocatori[ordine[i]];
            if (g_corr->stato == morto) continue;

            int turno_finito = 0, ha_avanzato = 0;
            do {
                int scelta;
                printf("\n--- Turno di %s (%s) ---\n", g_corr->nome, (g_corr->mondo == 0) ? "Mondo Reale" : "Soprasotto");
                printf("1. Avanza | 2. Indietreggia | 3. Cambia Mondo | 4. Combatti\n");
                printf("5. Info Giocatore | 6. Info Zona | 7. Raccogli Oggetto | 8. Usa Oggetto | 9. Passa\n>> ");
                
                if (scanf("%d", &scelta) != 1) { scelta = 0; }
                pulisci_buffer();

                switch (scelta) {
                    case 1: avanza(g_corr, &ha_avanzato); break;
                    case 2: indietreggia(g_corr); break;
                    case 3: cambia_mondo(g_corr, &ha_avanzato); break;
                    case 4: combatti(g_corr); break;
                    case 5: stampa_giocatore(g_corr); break;
                    case 6: stampa_zona(g_corr); break;
                    case 7: raccogli_oggetto(g_corr); break;
                    case 8: utilizza_oggetto(g_corr); break;
                    case 9: passa(&turno_finito); break;
                    default: printf("Scelta non valida!\n"); break;
                }
            } while (!turno_finito && g_corr->stato == vivo);
        }
        round++;
    }
}
