
#ifndef HUFFMAN_H
#define HUFFMAN_H

#define ASCII_SIZE    256 //numero totale di caratteri disponibili
#define MAX_CODE_LEN  256 //lunghezza massimo teorica di un codice di huffman

typedef struct HuffNode {
    unsigned char ch;       /* carattere ASCII (0-255), significativo solo nelle foglie */
    int freq;               /* frequenza: conteggio se foglia, somma figli se interno */
    struct HuffNode *left;  /* figlio sinistro — percorso bit 0 */
    struct HuffNode *right; /* figlio destro   — percorso bit 1 */
} HuffNode;

typedef struct {
    HuffNode **data;  /* array dinamico di puntatori a HuffNode */
    int size;         /* numero di elementi presenti (0..capacity) */
    int capacity;     /* dimensione allocata dell'array */
} MinHeap;


MinHeap* create_heap(int capacity);

void heap_insert(MinHeap *h, HuffNode *node);

HuffNode* extract_min(MinHeap *h);

void free_heap(MinHeap *h);

MinHeap* build_heap(const char *text, int length);

HuffNode* build_tree(MinHeap *heap);

void build_codes(HuffNode *root, char codes[][MAX_CODE_LEN], int lengths[]);

char* encode(const char *text, char codes[][MAX_CODE_LEN]);

char* decode(HuffNode *root, const char *bits);

void free_tree(HuffNode *root);

#endif /* HUFFMAN_H */
