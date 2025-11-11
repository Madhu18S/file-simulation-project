#include "nary.h"

// Create a new node with the given data
Node* createNode(const char* data) {
    // Allocate memory for a new Node structure
    Node* newNode = (Node*)malloc(sizeof(Node));
    
    // Copy the input data into the node's data field
    // Note: Uses strcpy which assumes data fits in 50 chars (potential buffer overflow risk)
    strcpy(newNode->data, data);
    
    // Initialize child count to 0 (no children yet)
    newNode->childCount = 0;
    
    // Initialize all child pointers to NULL (no children connected)
    for (int i = 0; i < 10; i++)
        newNode->child[i] = NULL;
    
    return newNode;  // Return the newly created node
}

// Insert a new child node with given data under the specified parent node
void insertChild(Node* parent, const char* data) {
    // Check if parent already has maximum allowed children (10)
    if (parent->childCount >= 10) {
        printf("Cannot insert more than 10 children for %s\n", parent->data);
        return;  // Exit early if limit reached
    }
    
    // Create the new child node
    Node* newChild = createNode(data);
    
    // Add the new child to the next available slot in parent's child array
    // Then increment the childCount (post-increment: use current value, then add 1)
    parent->child[parent->childCount++] = newChild;
}

// Search for a node containing the specified key in the tree
// Uses depth-first search (DFS) recursion to traverse the entire tree
Node* search(Node* root, const char* key) {
    // Base case: if current node is NULL, return NULL (not found)
    if (root == NULL) return NULL;
    
    // Check if current node's data matches the search key
    if (strcmp(root->data, key) == 0) 
        return root;  // Found it! Return this node
    
    // Recursively search through all children of current node
    for (int i = 0; i < root->childCount; i++) {
        Node* found = search(root->child[i], key);  // Search in each child's subtree
        if (found != NULL) 
            return found;  // If found in child subtree, return it immediately
    }
    
    return NULL;  // Key not found in this subtree
}

// Perform preorder traversal of the tree (Root -> Children)
// Visits: Current node first, then recursively visits all children
void traverse(Node* root) {
    if (root == NULL) return;
    for (int i = 0; i < root->childCount; i++) {
        traverse(root->child[i]);
    }
}

