#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define TOKEN_SIZE 16
#define COUNT_SIZE 64
#define SERIALSIZE 16

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define token_t unsigned _BitInt(TOKEN_SIZE)
#define count_t unsigned _BitInt(COUNT_SIZE)
#define probi_t __fp16
#define VOCAB_SIZE 65535

/* Trie node */
typedef struct PrefixNode {
    count_t count;
    struct HashMapNode *children; /* HashMap: token_t --> PrefixNode */
} PrefixNode;

/* HashMap implemented as AVL tree */
typedef struct HashMapNode {
    token_t key;
    PrefixNode *value;
    struct HashMapNode *left, *right;
    uint16_t height; /* from leaf upwards */
} HashMapNode;

typedef HashMapNode *HashMap;

/* Top-r distribution */
typedef struct yMulti_i {
    token_t token_id;
    count_t count;
    probi_t prob;
} yMulti_i;

HashMap newHashMapEntry(uint16_t key, PrefixNode *value);

PrefixNode *newPrefixNode(); 

int16_t heightAVLTree(HashMap node); 

int16_t getHeightDiffAVLTree(HashMap node);

HashMap leftRotateAVLTree(HashMap root); 

HashMap rightRotateAVLTree(HashMap root);

HashMap insertIntoHashMap(HashMap root, token_t key, PrefixNode *value); 

PrefixNode *findKeyInHashMap(HashMap root, token_t key); 

void deletePrefixNode(PrefixNode *node);

void deleteHashMap(HashMap map);

PrefixNode *insertPrefix(PrefixNode *root, token_t *prefix, uint16_t prefix_len);

void populateDistri(yMulti_i *dist, HashMap map);

void swap(yMulti_i *a, yMulti_i *b);

void heapify(yMulti_i dist[], uint64_t n, uint64_t i);

void buildHeap(yMulti_i dist[], uint64_t n);

void topRHashMap(HashMap map, uint16_t r, count_t normalizer, yMulti_i *out);

void printRecursiveHashMapInOrder(HashMap map, uint16_t depth);
