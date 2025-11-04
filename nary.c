#include "nary.h"

// Create a new node
Node* createNode(const char* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    strcpy(newNode->data, data);
    newNode->childCount = 0;
    for (int i = 0; i < 10; i++)
        newNode->child[i] = NULL;
    return newNode;
}

// Insert child under given parent node
void insertChild(Node* parent, const char* data) {
    if (parent->childCount >= 10) {
        printf("Cannot insert more than 10 children for %s\n", parent->data);
        return;
    }
    Node* newChild = createNode(data);
    parent->child[parent->childCount++] = newChild;
}

// Search a node by data
Node* search(Node* root, const char* key) {
    if (root == NULL) return NULL;
    if (strcmp(root->data, key) == 0) return root;

    for (int i = 0; i < root->childCount; i++) {
        Node* found = search(root->child[i], key);
        if (found != NULL) return found;
    }
    return NULL;
}

// Preorder traversal
void traverse(Node* root) {
    if (root == NULL) return;
    printf("%s ", root->data);
    for (int i = 0; i < root->childCount; i++)
        traverse(root->child[i]);
}
