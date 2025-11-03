#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHAR_SIZE 128 // supports ASCII characters

// Trie node structure
typedef struct TrieNode {
    struct TrieNode *children[CHAR_SIZE];
    int isEndOfFile;
} TrieNode;

// Function to create a new Trie node
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

// Recursively print all files that share a given prefix (bonus feature)
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

// Free the Trie (to avoid memory leaks)
void freeTrie(TrieNode *root) {
    for (int i = 0; i < CHAR_SIZE; i++) {
        if (root->children[i])
            freeTrie(root->children[i]);
    }
    free(root);
}

// Main simulation
int main() {
    TrieNode *root = createNode();

    // Sample file paths
    char *files[] = {
        "/user/docs/report.txt",
        "/user/docs/notes.txt",
        "/user/music/song.mp3",
        "/downloads/setup.exe",
        "/photos/vacation/img1.jpg",
        "/photos/vacation/img2.jpg"
    };
    int n = sizeof(files) / sizeof(files[0]);

    // Measure insertion time
    clock_t start = clock();
    for (int i = 0; i < n; i++)
        insertFile(root, files[i]);
    clock_t end = clock();

    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    // Output
    printf("=========================================\n");
    printf("   FILE ACCESS SYSTEM USING TRIE\n");
    printf("=========================================\n\n");

    printf(">> Files inserted successfully!\n");
    printf(">> Time taken to insert all files: %.6f seconds\n\n", time_taken);

    // Search test
    char query1[] = "/user/music/song.mp3";
    char query2[] = "/user/music/podcast.mp3";

    printf("Searching for files:\n");
    printf("  %-30s : %s\n", query1, searchFile(root, query1) ? "Found ✅" : "Not Found ❌");
    printf("  %-30s : %s\n\n", query2, searchFile(root, query2) ? "Found ✅" : "Not Found ❌");

    // Prefix test
    char prefix[] = "/photos/vacation";
    printf("Checking for prefix: '%s'\n", prefix);
    if (startsWith(root, prefix)) {
        printf("  Prefix exists ✅\n");
        printf("  Files under this prefix:\n");
        char buffer[256];
        strcpy(buffer, prefix);
        printFilesWithPrefix(root, buffer, strlen(prefix));
    } else {
        printf("  Prefix not found ❌\n");
    }

    // Cleanup
    freeTrie(root);

    printf("\n=========================================\n");
    printf("Simulation Complete.\n");
    printf("=========================================\n");

    return 0;
}
