#ifndef TRIE_H
#define TRIE_H

#define CHAR_SIZE 128

typedef struct TrieNode {
    struct TrieNode *children[CHAR_SIZE];
    int isEndOfFile;
} TrieNode;

TrieNode* createNode();
void insertFile(TrieNode *root, const char *path);
int searchFile(TrieNode *root, const char *path);
int startsWith(TrieNode *root, const char *prefix);
void printFilesWithPrefix(TrieNode *root, char *prefix, int level);
void freeTrie(TrieNode *root);

#endif
