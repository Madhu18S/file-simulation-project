#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nary.h"

#define MODULE_NAME "nary"
#define DEFAULT_NODES 1000
#define DEFAULT_RUNS 5

/* Simple portable timer (seconds) */
static double now_seconds(void) {
    return (double)clock() / CLOCKS_PER_SEC;
}

/* Build a sample N-ary tree fully in memory */
Node* build_sample_tree(size_t n) {
    Node* root = createNode("Root");
    Node** nodes = malloc(sizeof(Node*) * n);
    if (!nodes) return root;

    nodes[0] = root;
    size_t count = 1;

    // Each node gets up to 3 children until we reach 'n' total nodes
    for (size_t i = 0; i < n && count < n; i++) {
        for (int j = 0; j < 3 && count < n; j++) {
            char name[32];
            snprintf(name, sizeof(name), "Node%zu", count);
            Node* newNode = createNode(name);
            nodes[i]->child[nodes[i]->childCount++] = newNode;
            nodes[count++] = newNode;
        }
    }

    free(nodes);
    return root;
}

/* Perform one experiment run */
void run_one(FILE* csv, int run_id, size_t n) {
    double t0, t1;

    // Build tree
    t0 = now_seconds();
    Node* root = build_sample_tree(n);
    t1 = now_seconds();
    double build_ms = (t1 - t0) * 1000.0;

    // Traversal
    t0 = now_seconds();
    traverse(root); // traversal without printing
    t1 = now_seconds();
    double traverse_ms = (t1 - t0) * 1000.0;

    // Search for a specific node
    t0 = now_seconds();
    Node* found = search(root, "Node500");
    t1 = now_seconds();
    double search_ms = (t1 - t0) * 1000.0;

    const char* result = found ? "found" : "not_found";

    // Write results to CSV
    fprintf(csv, "%s,%d,%zu,%.3f,%.3f,%.3f,%s\n",
            MODULE_NAME, run_id, n, build_ms, traverse_ms, search_ms, result);
    fflush(csv);

    printf("Run %d build complete\n", run_id);
}

/* Main program */
int main(int argc, char** argv) {
    size_t n = DEFAULT_NODES;
    int runs = DEFAULT_RUNS;

    if (argc >= 2) n = (size_t)atoi(argv[1]);
    if (argc >= 3) runs = atoi(argv[2]);

    FILE* csv = fopen("results_nary.csv", "w");
    if (!csv) {
        perror("Error opening results_nary.csv");
        return 1;
    }

    fprintf(csv, "module,run_id,n,build_ms,traverse_ms,search_ms,result\n");

    for (int i = 1; i <= runs; i++) {
        run_one(csv, i, n);
    }

    fclose(csv);
    printf("Results stored in CSV\n");
    return 0;
}
