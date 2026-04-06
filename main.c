#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"


static void print_separator(void)
{
    printf("\n========================================================\n");
}


/* print_freq_descending — Stampa i caratteri in ordine di frequenza decrescente. */
static void print_freq_descending(const char *text, int length)
{
    int freq[ASCII_SIZE] = {0};

    /* Conteggio frequenze */
    for (int i = 0; i < length; i++) {
        freq[(unsigned char)text[i]]++;
    }

    /* Contiamo quanti caratteri distinti ci sono */
    int distinct = 0;
    for (int i = 0; i < ASCII_SIZE; i++) {
        if (freq[i] > 0) distinct++;
    }

    printf("%-20s %-10s %-10s\n", "Carattere", "ASCII", "Frequenza");
    printf("%-20s %-10s %-10s\n", "---------", "-----", "---------");

    /* Stampiamo in ordine decrescente di frequenza */
    for (int printed = 0; printed < distinct; printed++) {
        /* Trova il carattere con la frequenza massima rimanente */
        int max_freq = 0;
        int max_char = -1;

        for (int i = 0; i < ASCII_SIZE; i++) {
            if (freq[i] > max_freq) {
                max_freq = freq[i];
                max_char = i; //salvo il codice ascii del carattere con la frequenza maggiore
            }
        }
        
        if (max_char == -1) break;

        /* Stampa il carattere: se e' stampabile lo mostra, altrimenti
         * mostra una descrizione (es. "SPAZIO" per ' ', "TAB" per '\t') */
        if (max_char == ' ') {
            printf("%-20s %-10d %-10d\n", "(spazio)", max_char, max_freq);
        } else if (max_char == '\n') {
            printf("%-20s %-10d %-10d\n", "(a capo)", max_char, max_freq);
        } else if (max_char == '\t') {
            printf("%-20s %-10d %-10d\n", "(tab)", max_char, max_freq);
        } else if (max_char >= 33 && max_char <= 126) {
            /* Carattere ASCII stampabile */
            char display[4];
            display[0] = '\'';
            display[1] = (char)max_char;
            display[2] = '\'';
            display[3] = '\0';
            printf("%-20s %-10d %-10d\n", display, max_char, max_freq);
        } else {
            printf("(ASCII %-3d)          %-10d %-10d\n", max_char, max_char, max_freq);
        }

        /* Azzeriamo per non ritrovarlo al prossimo giro */
        freq[max_char] = 0;
    }
}


int main(void)
{

    char text[4096];

    printf("Inserisci il testo da comprimere (min 80 caratteri, min 8 distinti):\n> ");
    if (fgets(text, sizeof(text), stdin) == NULL) {
        printf("Errore nella lettura dell'input.\n");
        return 1;
    }

    /* Rimuoviamo il '\n' finale inserito da fgets quando l'utente preme Invio */
    int length = strlen(text);
    if (length > 0 && text[length - 1] == '\n') {
        text[length - 1] = '\0';
        length--;
    }

    /* Verifica requisiti minimi */
    if (length < 80) {
        printf("Errore: il testo deve essere lungo almeno 80 caratteri (inseriti: %d).\n", length);
        return 1;
    }

    /* Contiamo i caratteri distinti per la verifica */
    {
        int check_freq[ASCII_SIZE] = {0};
        int distinct = 0;
        for (int i = 0; i < length; i++) {
            check_freq[(unsigned char)text[i]]++;
        }
        for (int i = 0; i < ASCII_SIZE; i++) {
            if (check_freq[i] > 0) distinct++;
        }
        if (distinct < 8) {
            printf("Errore: servono almeno 8 caratteri distinti (trovati: %d).\n", distinct);
            return 1;
        }
    }

    printf("Testo di test (%d caratteri):\n\"%s\"\n", length, text);


    /* SCENARIO 1 — Analisi delle frequenze */
    print_separator();
    printf("SCENARIO 1 - Analisi delle frequenze\n");
    print_separator();

    MinHeap *heap1 = build_heap(text, length);
    printf("\nCaratteri distinti trovati: %d\n", heap1->size);
    printf("Caratteri totali nel testo: %d\n\n", length);

    print_freq_descending(text, length);

    /* Per lo scenario 1, liberiamo heap e nodi. I nodi nella heap non fanno parte di un albero, quindi li liberiamo manualmente. */
    for (int i = 0; i < heap1->size; i++) {
        free(heap1->data[i]);
    }
    free_heap(heap1);


    /* SCENARIO 2 — Albero e codici di Huffman */
    print_separator();
    printf("SCENARIO 2 - Albero di Huffman e codici binari\n");
    print_separator();

    MinHeap *heap2 = build_heap(text, length);
    HuffNode *root = build_tree(heap2);
    free_heap(heap2);  /* la heap e' vuota dopo build_tree */

    /* Tabella dei codici: 256 stringhe (una per ogni possibile carattere ASCII),
     * ciascuna lunga al massimo 256 caratteri. */
    char codes[ASCII_SIZE][MAX_CODE_LEN];
    int lengths[ASCII_SIZE];

    build_codes(root, codes, lengths);

    printf("\n%-15s %-10s %-10s %-30s %-10s\n",
           "Carattere", "ASCII", "Frequenza", "Codice Huffman", "Bit");
    printf("%-15s %-10s %-10s %-30s %-10s\n",
           "---------", "-----", "---------", "--------------", "---");

    /* Calcoliamo le frequenze per stamparle nella tabella */
    int freq[ASCII_SIZE] = {0};
    for (int i = 0; i < length; i++) {
        freq[(unsigned char)text[i]]++;
    }

    for (int i = 0; i < ASCII_SIZE; i++) {
        if (lengths[i] > 0) {
            if (i == ' ') {
                printf("%-15s %-10d %-10d %-30s %-10d\n",
                    "(spazio)", i, freq[i], codes[i], lengths[i]);
            } else if (i >= 33 && i <= 126) {
                char display[4];
                display[0] = '\'';
                display[1] = (char)i;
                display[2] = '\'';
                display[3] = '\0';
                printf("%-15s %-10d %-10d %-30s %-10d\n",
                       display, i, freq[i], codes[i], lengths[i]);
            } else {
                printf("(ASCII %-3d)    %-10d %-10d %-30s %-10d\n",
                       i, i, freq[i], codes[i], lengths[i]);
            }
        }
    }


    /* SCENARIO 3 — Compressione */
    print_separator();
    printf("SCENARIO 3 - Compressione (encode)\n");
    print_separator();

    char *encoded = encode(text, codes);

    printf("\nStringa di bit codificata:\n%s\n", encoded);
    printf("\nLunghezza stringa di bit: %d caratteri\n", (int)strlen(encoded));


    /* SCENARIO 4 — Decompressione e verifica */
    print_separator();
    printf("SCENARIO 4 - Decompressione e verifica\n");
    print_separator();

    char *decoded = decode(root, encoded);

    printf("\nTesto decodificato:\n\"%s\"\n", decoded);
    printf("\nTesto originale:\n\"%s\"\n", text);

    printf("\nConfronto carattere per carattere: ");
    if (strcmp(text, decoded) == 0) {
        printf("DECODIFICA CORRETTA\n");
    } else {
        printf("ERRORE\n");
    }


    /* Pulizia della memoria */
    free(encoded);
    free(decoded);
    free_tree(root);

    printf("\nMemoria liberata correttamente. Programma terminato.\n");

    return 0;
}
