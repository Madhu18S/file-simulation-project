// Header guard - prevents this file from being included multiple times
// This avoids duplicate definition errors during compilation
#ifndef NARY_H
#define NARY_H

// Standard C library includes:
#include <stdio.h>   // For input/output functions (printf, etc.)
#include <stdlib.h>  // For memory allocation (malloc, free) and other utilities
#include <string.h>  // For string manipulation functions (strcpy, strcmp, etc.)
#include <time.h>   // for clock()

// N-ary Tree Node structure definition
// Each node in our tree can store data and have multiple children
typedef struct Node {
    char data[50];           // Stores the node's content (filename or other data)
                             // Fixed size of 50 characters for simplicity
    
    struct Node *child[10];  // Array of pointers to child nodes
                             // Maximum of 10 children per node for simplicity
                             // This limits the tree's branching factor
    
    int childCount;          // Tracks how many children this node currently has
                             // Helps know which positions in 'child' array are used
} Node;

// Function declarations (prototypes) - these tell the compiler what functions exist
// Actual implementation will be in nary.c

// Creates and returns a new node with the given data
Node* createNode(const char* data);

// Adds a new child node with the given data to the specified parent node
void insertChild(Node* parent, const char* data);

// Searches the entire tree starting from 'root' for a node containing 'key'
// Returns pointer to the node if found, NULL otherwise
Node* search(Node* root, const char* key);

// Traverses and prints all nodes in the tree starting from 'root'
// uses depth-first traversal to visit all nodes
void traverse(Node* root);

#endif // End of header guard
