#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// -----------------------------
// Structure Definition
// -----------------------------

// Each node represents either a file or folder
typedef struct Node {
    char name[50];
    struct Node *child;   // pointer to first subdirectory or file
    struct Node *sibling; // pointer to next item in the same directory
} Node;

// -----------------------------
// Function Prototypes
// -----------------------------
Node* createNode(const char *name);
void addChild(Node *parent, Node *child);
void displayFileSystem(Node *root);
int searchFileSystem(Node *root, const char *target);
void freeFileSystem(Node *root);

// -----------------------------
// Create a new node (file/folder)
// -----------------------------
Node* createNode(const char *name) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    strcpy(newNode->name, name);
    newNode->child = NULL;
    newNode->sibling = NULL;
    return newNode;
}

// -----------------------------
// Add child (used to build hierarchy)
// -----------------------------
void addChild(Node *parent, Node *child) {
    if (parent->child == NULL)
        parent->child = child;
    else {
        Node *temp = parent->child;
        while (temp->sibling != NULL)
            temp = temp->sibling;
        temp->sibling = child;
    }
}

// -----------------------------
// Display the file system using DFS traversal
// -----------------------------
void displayFileSystem(Node *root) {
    if (root == NULL) return;

    printf("%s\n", root->name);
    Node *temp = root->child;
    while (temp != NULL) {
        displayFileSystem(temp);
        temp = temp->sibling;
    }
}

// -----------------------------
// Search the file system recursively
// Returns 1 if found, 0 otherwise
// -----------------------------
int searchFileSystem(Node *root, const char *target) {
    if (root == NULL)
        return 0;

    if (strcmp(root->name, target) == 0)
        return 1;

    // Search in child and sibling recursively
    if (searchFileSystem(root->child, target))
        return 1;
    return searchFileSystem(root->sibling, target);
}

// -----------------------------
// Free memory used by the file system
// -----------------------------
void freeFileSystem(Node *root) {
    if (root == NULL) return;
    freeFileSystem(root->child);
    freeFileSystem(root->sibling);
    free(root);
}

// -----------------------------
// Main Function
// -----------------------------
int main() {
    clock_t start, end;
    double cpu_time_used;
    int choice;
    char searchName[50];

    // -------------------------
    // Build a predefined file system
    // -------------------------
    Node *root = createNode("root");

    Node *docs = createNode("Documents");
    Node *pics = createNode("Pictures");
    Node *vids = createNode("Videos");

    addChild(root, docs);
    addChild(root, pics);
    addChild(root, vids);

    addChild(docs, createNode("resume.pdf"));
    addChild(docs, createNode("report.docx"));

    addChild(pics, createNode("photo1.jpg"));
    addChild(pics, createNode("photo2.png"));

    Node *movies = createNode("Movies");
    addChild(vids, movies);
    addChild(movies, createNode("inception.mp4"));
    addChild(movies, createNode("matrix.mp4"));

    // -------------------------
    // Menu-driven simulation
    // -------------------------
    do {
        printf("\n===== FILE SYSTEM MENU =====\n");
        printf("1. Display File System (Traversal)\n");
        printf("2. Search File/Folder\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                start = clock();
                printf("\n--- Directory Structure ---\n");
                displayFileSystem(root);
                end = clock();
                cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
                printf("\nTraversal Time: %.6f seconds\n", cpu_time_used);
                break;

            case 2:
                printf("\nEnter file/folder name to search: ");
                scanf("%s", searchName);
                start = clock();
                if (searchFileSystem(root, searchName))
                    printf("'%s' found in the file system.\n", searchName);
                else
                    printf("'%s' not found.\n", searchName);
                end = clock();
                cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
                printf("Search Time: %.6f seconds\n", cpu_time_used);
                break;

            case 3:
                printf("\nExiting program...\n");
                break;

            default:
                printf("Invalid choice. Try again.\n");
        }
    } while (choice != 3);

    // Free memory
    freeFileSystem(root);
    return 0;
}
