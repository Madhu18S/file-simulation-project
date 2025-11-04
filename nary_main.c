#include "nary.h"

int main() {
    Node* root = createNode("Root");

    insertChild(root, "A");
    insertChild(root, "B");

    Node* nodeA = search(root, "A");
    insertChild(nodeA, "A1");
    insertChild(nodeA, "A2");

    Node* nodeB = search(root, "B");
    insertChild(nodeB, "B1");

    printf("N-ary Tree Traversal: ");
    traverse(root);
    printf("\n");

    Node* found = search(root, "B1");
    if (found)
        printf("Node '%s' found!\n", found->data);
    else
        printf("Node not found!\n");

    return 0;
}
