#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merkle.h"
#include "sha256.h"

/* Simple Windows-compatible timer using clock() */
double get_time_seconds() {
    return (double)clock() / CLOCKS_PER_SEC;
}

/* Global Merkle Tree (for easy tamper testing) */
MerkleTree tree;

/* Sample file data */
const char *filenames[] = {"math.txt", "ai.txt", "ethics.txt"};
const char *contents[]  = {
    "Mathematics is the language of the universe.",
    "Artificial Intelligence is shaping the future.",
    "Ethics keeps technology human-centered."
};

int main() {
    printf("\n=== Merkle Tree Integrity Verification ===\n");

    /* Create a NameMap (hash table to store filename-node mapping) */
    NameMap *map = namemap_create(32);

    /* Build the leaves of the Merkle Tree */
    build_leaves_from_arrays(&tree, filenames, contents, 3);

    /* Link each filename to its leaf node in the map */
    for (size_t i = 0; i < tree.leaf_count; i++) {
        namemap_put(map, tree.leaves[i]->filename, tree.leaves[i]);
    }

    /* Build the complete Merkle Tree */
    build_merkle_tree(&tree);
    print_tree_levelorder(&tree);

    /* Verification Phase */
    printf("\nVerifying files...\n");
    for (size_t i = 0; i < tree.leaf_count; i++) {
        verify_file(map, &tree, filenames[i]);
    }

    /* Tampering test */
    printf("\nTampering 'math.txt'...\n");
    tamper_file(map, &tree, "math.txt", "hacked content!");


    printf("\nVerifying again after tampering:\n");
    for (size_t i = 0; i < tree.leaf_count; i++) {
        verify_file(map, &tree, filenames[i]);
    }

    /* Timing (Windows version) */
    double start = get_time_seconds();
    // simulate workload
    double end = get_time_seconds();
    printf("\nTime taken: %.6f seconds\n", end - start);

    /* Cleanup */
    free_tree(&tree);
    namemap_free(map);

    return 0;
}
