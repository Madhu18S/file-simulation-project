#ifndef NARY_H
#define NARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// N-ary Tree Node structure
typedef struct Node {
    char data[50];
    struct Node *child[10]; // Maximum 10 children for simplicity
    int childCount;
} Node;

// Function declarations
Node* createNode(const char* data);
void insertChild(Node* parent, const char* data);
Node* search(Node* root, const char* key);
void traverse(Node* root);

#endif
