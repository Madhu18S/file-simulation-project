#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

// Create a new Trie node
TrieNode* createNode() {
    TrieNode *node = (TrieNode*)malloc(sizeof(TrieNode));
    node->isEndOfFile = 0;
    for (int i = 0; i < CHAR_SIZE; i++)
        node->children[i] = NULL;
    return node;
}

// Insert a file path into the Trie
void insertFile(TrieNode *root, const char *path) {
    TrieNode *curr = root;
    for (int i = 0; path[i] != '\0'; i++) {
        int index = (int)path[i];
        if (!curr->children[index])
            curr->children[index] = createNode();
        curr = curr->children[index];
    }
    curr->isEndOfFile = 1;
}

// Search for a full file path
int searchFile(TrieNode *root, const char *path) {
    TrieNode *curr = root;
    for (int i = 0; path[i] != '\0'; i++) {
        int index = (int)path[i];
        if (!curr->children[index])
            return 0;
        curr = curr->children[index];
    }
    return curr->isEndOfFile;
}

// Check if any file starts with the given prefix
int startsWith(TrieNode *root, const char *prefix) {
    TrieNode *curr = root;
    for (int i = 0; prefix[i] != '\0'; i++) {
        int index = (int)prefix[i];
        if (!curr->children[index])
            return 0;
        curr = curr->children[index];
    }
    return 1;
}

// Recursively print all files that share a given prefix
void printFilesWithPrefix(TrieNode *root, char *prefix, int level) {
    if (root->isEndOfFile) {
        prefix[level] = '\0';
        printf("  %s\n", prefix);
    }
    for (int i = 0; i < CHAR_SIZE; i++) {
        if (root->children[i]) {
            prefix[level] = (char)i;
            printFilesWithPrefix(root->children[i], prefix, level + 1);
        }
    }
}

// Free the Trie memory
void freeTrie(TrieNode *root) {
    for (int i = 0; i < CHAR_SIZE; i++) {
        if (root->children[i])
            freeTrie(root->children[i]);
    }
    free(root);
}
