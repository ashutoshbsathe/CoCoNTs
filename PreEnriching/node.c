#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "node.h"

HashMap newHashMapEntry(token_t key, PrefixNode *value) {
    HashMapNode *ret = (HashMapNode *) malloc(sizeof(HashMapNode));
    ret->key = key;
    ret->value = value;
    ret->height = 1;
    ret->left = NULL;
    ret->right = NULL;
    return ret;
}

PrefixNode *newPrefixNode() {
    PrefixNode *ret = (PrefixNode *) malloc(sizeof(PrefixNode));
    ret->count = 0;
    ret->children = NULL;
    return ret;
}

void deletePrefixNode(PrefixNode *node) {
    if(node == NULL) {
        return;
    }
    deleteHashMap(node->children);
    free(node);
    return;
}

void deleteHashMap(HashMap map) {
    if(map == NULL) {
        return;
    }
    deletePrefixNode(map->value);
    deleteHashMap(map->left);
    deleteHashMap(map->right);
    free(map);
    return;
}

int16_t heightAVLTree(HashMap node) {
    if(node == NULL) {
        return 0;
    }
    return node->height;
}

int16_t getHeightDiffAVLTree(HashMap node) {
    if(node == NULL) {
        return 0;
    }
    return heightAVLTree(node->left) - heightAVLTree(node->right);
}

HashMap leftRotateAVLTree(HashMap root) {
    int16_t left_height, right_height;
    HashMapNode *right = root->right, *left = right->left;

    right->left = root;
    root->right = left;

    left_height = heightAVLTree(root->left);
    right_height = heightAVLTree(root->right);
    root->height = 1 + MAX(left_height, right_height);

    left_height = heightAVLTree(right->left);
    right_height = heightAVLTree(right->right);
    right->height = 1 + MAX(left_height, right_height);

    return right;
}

HashMap rightRotateAVLTree(HashMap root) {
    int16_t left_height, right_height;
    HashMapNode *left = root->left, *right = left->right;

    left->right = root;
    root->left = right;

    left_height = heightAVLTree(root->left);
    right_height = heightAVLTree(root->right);
    root->height = 1 + MAX(left_height, right_height);

    left_height = heightAVLTree(left->left);
    right_height = heightAVLTree(left->right);
    left->height = 1 + MAX(left_height, right_height);

    return left;
}

HashMap insertIntoHashMap(HashMap root, token_t key, PrefixNode *value) {
    int16_t left_height, right_height, height_diff;
    if(root == NULL) {
        return newHashMapEntry(key, value);
    }

    if(key < root->key) {
        root->left = insertIntoHashMap(root->left, key, value);
    }
    else if(key > root->key) {
        root->right = insertIntoHashMap(root->right, key, value);
    }
    else {
        /* don't add new node, caller's responsibility to free `value` */
        return root;
    }

    /* Update heights */
    left_height = heightAVLTree(root->left);
    right_height = heightAVLTree(root->right);
    root->height = 1 + MAX(left_height, right_height);

    height_diff = getHeightDiffAVLTree(root);
    if(height_diff > 1) {
        if(key < root->left->key) {
            return rightRotateAVLTree(root);
        }
        else if(key > root->left->key) {
            root->left = leftRotateAVLTree(root->left);
            return rightRotateAVLTree(root);
        } 
    }

    if(height_diff < -1) {
        if(key > root->right->key) {
            return leftRotateAVLTree(root);
        }
        else if(key < root->right->key) {
            root->right = rightRotateAVLTree(root->right);
            return leftRotateAVLTree(root);
        }
    }
    return root;
}

PrefixNode *findKeyInHashMap(HashMap root, token_t key) {
    if(root == NULL) {
        return NULL;
    }
    else if(root->key > key) {
        return findKeyInHashMap(root->left, key);
    }
    else if(root->key < key) {
        return findKeyInHashMap(root->right, key);
    }
    else {
        return root->value;
    }
}

PrefixNode *insertPrefix(PrefixNode *root, token_t *prefix, uint16_t prefix_len) {
    token_t token;
    PrefixNode *curr_node = root, *next_node = NULL;
    curr_node->count += 1;
    for(uint16_t i = 0; i < prefix_len; i++) {
        token = prefix[i];
        next_node = findKeyInHashMap(curr_node->children, token);
        if(next_node == NULL) {
            next_node = newPrefixNode();
            curr_node->children = insertIntoHashMap(curr_node->children, token, next_node);
        }
        next_node->count += 1;
        curr_node = next_node;
    }
    return root;
}

void populateDistri(yMulti_i *dist, HashMap map) {
    if(map == NULL) {
        return;
    }
    populateDistri(dist, map->left);
    if(debug) {
        printf("Setting %d count to %d\n", map->key, map->value->count);
    }
    dist[map->key].count = map->value->count;
    populateDistri(dist, map->right);
    return;
}

void swap(yMulti_i *a, yMulti_i *b) {
    yMulti_i tmp = *a;
    *a = *b;
    *b = tmp;
    return;
}

void heapify(yMulti_i dist[], uint64_t n, uint64_t i) {
    uint64_t largest = i, left = 2*i + 1, right = 2*i + 2;
    
    if(left < n && 
        (
            dist[left].count > dist[largest].count 
            || 
            (dist[left].count == dist[largest].count && dist[left].token_id < dist[largest].token_id)
        )
    ) {
        largest = left;
    }
    
    if(right < n && 
        (
            dist[right].count > dist[largest].count 
            || 
            (dist[right].count == dist[largest].count && dist[right].token_id < dist[largest].token_id)
        )
    ) {
        largest = right;
    }
    
    if(largest < n && largest != i) {
        swap(&dist[i], &dist[largest]);
        heapify(dist, n, largest);
    }
    
    return;
}

void buildHeap(yMulti_i dist[], uint64_t n) {
    /* Note uint64_t will loop back to int_max, so `i>=0` will not work as an exit condition */
    for(uint64_t i = (n/2) - 1; i < n; i--) {
        heapify(dist, n, i);
    }

    for(uint64_t i = n - 1; i < n; i--) {
        swap(&dist[0], &dist[i]);
        heapify(dist, i, 0);
    }
    
    return;
}

void topRHashMap(HashMap map, uint16_t r, count_t normalizer, yMulti_i *out) {
    yMulti_i dist[VOCAB_SIZE];
    for(uint16_t i = 0; i < VOCAB_SIZE; i++) {
        dist[i].token_id = i;
        dist[i].count = 0;
    }
    populateDistri(dist, map);
    
    /* Note uint64_t will loop back to int_max, so `i>=0` does not work as a stopping condition */
    for(uint64_t i = (VOCAB_SIZE/2) - 1; i < VOCAB_SIZE; i--) {
        heapify(dist, VOCAB_SIZE, i);
    }
    
    for(uint64_t i = 0; i < r; i++) {
        out[i].token_id = dist[0].token_id;
        out[i].count = dist[0].count;
        out[i].prob = (probi_t) (dist[0].count * 1.0f / normalizer);
        swap(&dist[0], &dist[VOCAB_SIZE-i-1]);
        heapify(dist, VOCAB_SIZE-i-1, 0);
    }

    return;
}

void printRecursiveHashMapInOrder(HashMap map, uint16_t depth) {
    if(map == NULL) {
        return;
    }
    printRecursiveHashMapInOrder(map->left, depth);
    for(uint16_t i = 0; i < depth; i++) {
        printf("\t");
    }
    printf("%ld: ", map->key);
    printf("%ld\n", map->value->count);
    printRecursiveHashMapInOrder(map->value->children, depth+1);
    printRecursiveHashMapInOrder(map->right, depth);

    return;
}
