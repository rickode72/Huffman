#include <stdio.h>    
#include <stdlib.h>   
#include <string.h>   
#include "huffman.h"


/* swap — Scambia due puntatori nell'array della heap. */
static void swap(HuffNode **a, HuffNode **b)
{
    HuffNode *temp = *a;
    *a = *b;
    *b = temp;
}


static void heapify_down(MinHeap *h, int i)
{
    int smallest = i;            /* ipotizziamo che i sia il piu' piccolo */
    int left  = 2 * i + 1;      /* indice del figlio sinistro */
    int right = 2 * i + 2;      /* indice del figlio destro */

    /* Se il figlio sinistro esiste E ha frequenza minore, aggiorna smallest */
    if (left < h->size && h->data[left]->freq < h->data[smallest]->freq) {
        smallest = left;
    }

    /* Se il figlio destro esiste E ha frequenza minore, aggiorna smallest */
    if (right < h->size && h->data[right]->freq < h->data[smallest]->freq) {
        smallest = right;
    }

    /* Se smallest non e' piu' i, c'e' un figlio piu' piccolo:
     * scambiamo e continuiamo a scendere ricorsivamente */
    if (smallest != i) {
        swap(&h->data[i], &h->data[smallest]);
        heapify_down(h, smallest); /* chiamata ricorsiva sul figlio scambiato */
    }
}


static void heapify_up(MinHeap *h, int i)
{
    /* Finche' non siamo alla radice E il padre ha frequenza maggiore */
    while (i > 0) {
        int parent = (i - 1) / 2;  /* indice del padre */

        if (h->data[i]->freq < h->data[parent]->freq) {
            swap(&h->data[i], &h->data[parent]);
            i = parent;  /* continuiamo a risalire dalla posizione del padre */
        } else {
            break;  /* il nodo e' nella posizione corretta */
        }
    }
}



MinHeap* create_heap(int capacity)
{
    MinHeap *h = (MinHeap *)malloc(sizeof(MinHeap));
    h->data = (HuffNode **)malloc(capacity * sizeof(HuffNode *));
    h->size = 0;
    h->capacity = capacity;
    return h;
}


void heap_insert(MinHeap *h, HuffNode *node)
{
    /* Se l'array e' pieno raddoppiamo la capacita' */
    if (h->size == h->capacity) {
        h->capacity *= 2;
        h->data = (HuffNode **)realloc(h->data, h->capacity * sizeof(HuffNode *));
    }

    /* Inseriamo il nodo alla fine */
    h->data[h->size] = node;
    h->size++;

    /* Facciamo risalire il nodo per ripristinare la proprieta' heap */
    heapify_up(h, h->size - 1);
}


HuffNode* extract_min(MinHeap *h)
{
    if (h->size == 0) {
        return NULL;
    }

    HuffNode *min_node = h->data[0];        /* salviamo il minimo */
    h->data[0] = h->data[h->size - 1];     /* l'ultimo va in cima */
    h->size--;                               /* un elemento in meno */
    heapify_down(h, 0);                      /* ripristiniamo la proprieta' */

    return min_node;
}



void free_heap(MinHeap *h)
{
    if (h != NULL) {
        free(h->data);  /* libera l'array di puntatori */
        free(h);         /* libera la struttura MinHeap */
    }
}



/* create_leaf — Funzione di supporto: crea un nodo foglia. */
static HuffNode* create_leaf(unsigned char ch, int freq)
{
    HuffNode *node = (HuffNode *)malloc(sizeof(HuffNode));
    node->ch    = ch;
    node->freq  = freq;
    node->left  = NULL;
    node->right = NULL;
    return node;
}


MinHeap* build_heap(const char *text, int length)
{
    /* Array di conteggio di frequenze*/
    int freq_array[ASCII_SIZE] = {0};

    for (int i = 0; i < length; i++) {
        freq_array[(unsigned char)text[i]]++;
    }

    int distinct_chars = 0;
    for (int i = 0; i < ASCII_SIZE; i++) {
        if (freq_array[i] > 0) {
            distinct_chars++;
        }
    }

    /* Creiamo la heap e inseriamo un nodo per ogni carattere. */
    MinHeap *heap = create_heap(distinct_chars);

    for (int i = 0; i < ASCII_SIZE; i++) {
        if (freq_array[i] > 0) {
            HuffNode *leaf = create_leaf((unsigned char)i, freq_array[i]);
            heap_insert(heap, leaf);
        }
    }

    return heap;
}



HuffNode* build_tree(MinHeap *heap)
{
    /* Caso speciale: se la heap ha un solo nodo (testo con un solo
     * carattere distinto), l'albero e' gia' completo: la radice
     * e' quel singolo nodo foglia. */

    /* Ciclo greedy: finche' ci sono almeno 2 nodi nella heap */
    while (heap->size > 1) {
        /* Estraiamo i due nodi con frequenza minima */
        HuffNode *left  = extract_min(heap);  /* il piu' piccolo */
        HuffNode *right = extract_min(heap);  /* il secondo piu' piccolo */

        /* Creiamo un nodo interno che li unisce.
         * Il carattere '\0' indica che non e' una foglia.
         * La frequenza e' la somma delle frequenze dei figli. */
        HuffNode *internal = (HuffNode *)malloc(sizeof(HuffNode));
        internal->ch    = '\0';
        internal->freq  = left->freq + right->freq;
        internal->left  = left;   /* sottoalbero sinistro = bit 0 */
        internal->right = right;  /* sottoalbero destro   = bit 1 */

        /* Reinseriamo il nodo interno nella heap */
        heap_insert(heap, internal);
    }

    /* L'unico nodo rimasto e' la radice dell'albero */
    return extract_min(heap);
}


/* build_codes_recursive — Funzione ricorsiva di supporto per build_codes. */

static void build_codes_recursive(HuffNode *node, char codes[][MAX_CODE_LEN], int lengths[], char *buffer, int depth) //depth = profondità attuale ( = lunghezza del codice accumulato), lentgh = array della lunghezza dei codici
{
    if (node == NULL) {
        return;
    }

    /* Se e' una foglia: abbiamo trovato un carattere */
    if (node->left == NULL && node->right == NULL) {
        buffer[depth] = '\0';  /* terminiamo la stringa */

        /* Copiamo il percorso accumulato nella tabella dei codici */
        strcpy(codes[node->ch], buffer);
        lengths[node->ch] = depth;
        return;
    }

    /* Scendiamo a sinistra: aggiungiamo '0' al percorso */
    buffer[depth] = '0';
    build_codes_recursive(node->left, codes, lengths, buffer, depth + 1);

    /* Scendiamo a destra: aggiungiamo '1' al percorso */
    buffer[depth] = '1';
    build_codes_recursive(node->right, codes, lengths, buffer, depth + 1);
}


/* Genera la tabella dei codici binari. */
void build_codes(HuffNode *root, char codes[][MAX_CODE_LEN], int lengths[])
{
    /* Inizializziamo tutto a zero */
    for (int i = 0; i < ASCII_SIZE; i++) {
        codes[i][0] = '\0';
        lengths[i] = 0;
    }

    if (root == NULL) {
        return;
    }

    /* Caso speciale: la radice E' una foglia (un solo carattere distinto).
     * L'albero non ha rami, quindi non c'e' percorso da codificare.
     * Per convenzione assegniamo il codice "0". */
    if (root->left == NULL && root->right == NULL) {
        codes[root->ch][0] = '0';
        codes[root->ch][1] = '\0';
        lengths[root->ch] = 1;
        return;
    }

    /* Caso normale: lanciamo la DFS ricorsiva */
    char buffer[MAX_CODE_LEN];
    build_codes_recursive(root, codes, lengths, buffer, 0);
}


/* Codifica il testo in una stringa di bit. */
char* encode(const char *text, char codes[][MAX_CODE_LEN])
{
    int text_len = strlen(text);

    /* Calcoliamo quanti bit servono in totale */
    int total_bits = 0;
    for (int i = 0; i < text_len; i++) {
        total_bits += strlen(codes[(unsigned char)text[i]]);
    }

    /* Allochiamo la stringa di bit (+1 per il terminatore '\0') */
    char *encoded = (char *)malloc(total_bits + 1);
    encoded[0] = '\0';  /* iniziamo con stringa vuota */

    /* Per ogni carattere, appendiamo il suo codice binario */
    int pos = 0;
    for (int i = 0; i < text_len; i++) {
        const char *code = codes[(unsigned char)text[i]]; //puntatore alla stringa, la stringa è il codice di huffman del carattere
        int code_len = strlen(code);
        /* Copiamo il codice nella posizione corrente */
        for (int j = 0; j < code_len; j++) {
            encoded[pos++] = code[j]; //code[j] accede al carattere in posizione j, si poteva anche scrivere "*(code + j)" o "*code" con "code++" nel for
        }

    }
    encoded[pos] = '\0';  /* terminiamo la stringa */

    /* Calcoliamo e stampiamo le statistiche di compressione */
    int original_bits = text_len * 8;  /* ASCII usa 8 bit per carattere */
    double ratio = (double)total_bits / original_bits * 100.0;

    printf("\n--- Statistiche di compressione ---\n");
    printf("Bit Huffman:   %d\n", total_bits);
    printf("Bit originali: %d (= %d caratteri x 8 bit)\n", original_bits, text_len);
    printf("Rapporto:      %.2f%%\n", ratio);

    return encoded;
}


/* Decodifica una stringa di bit in testo. */
char* decode(HuffNode *root, const char *bits)
{
    int bits_len = strlen(bits);

    /* Allocazione iniziale del buffer di output */
    int capacity = 256;
    char *decoded = (char *)malloc(capacity);
    int pos = 0;

    /* Caso speciale: albero con un solo nodo (un solo carattere distinto).
     * Ogni bit '0' corrisponde a una emissione del carattere. */
    if (root->left == NULL && root->right == NULL) {
        for (int i = 0; i < bits_len; i++) {
            /* Espandi il buffer se necessario */
            if (pos >= capacity - 1) {
                capacity *= 2;
                decoded = (char *)realloc(decoded, capacity);
            }
            decoded[pos++] = root->ch;
        }
        decoded[pos] = '\0';
        return decoded;
    }

    /* Caso normale: navigazione dell'albero bit per bit */
    HuffNode *current = root;  /* partiamo dalla radice */

    for (int i = 0; i < bits_len; i++) {
        /* Scendiamo in base al bit */
        
        if (bits[i] == '0') {
            current = current->left;   /* bit 0 = sinistra */
        } else {
            current = current->right;  /* bit 1 = destra */
        }

        /* Se siamo arrivati a una foglia, emettiamo il carattere */
        if (current->left == NULL && current->right == NULL) {
            /* Espandi il buffer se necessario */
            if (pos >= capacity - 1) {
                capacity *= 2;
                decoded = (char *)realloc(decoded, capacity);
            }
            decoded[pos++] = current->ch; 
            current = root;  /* ricominciamo dalla radice */
        }
    }

    decoded[pos] = '\0';  /* terminiamo la stringa */
    return decoded;
}

/* Libera ricorsivamente tutti i nodi dell'albero. */
void free_tree(HuffNode *root)
{
    if (root == NULL) {
        return;
    }

    free_tree(root->left);   /* libera tutto il sottoalbero sinistro */
    free_tree(root->right);  /* libera tutto il sottoalbero destro */
    free(root);               /* ora possiamo liberare il nodo corrente */
}
