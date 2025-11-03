#ifndef MERKLE_H
#define MERKLE_H

#include <stddef.h>
#include <stdint.h>

/* Max sizes */
#define HASH_HEX_LEN 65 /* 64 hex chars + null */

/* Node structure for Merkle Tree (binary) */
typedef struct MerkleNode {
    char hash[HASH_HEX_LEN];            /* hex string of the node hash */
    struct MerkleNode *left;
    struct MerkleNode *right;
    struct MerkleNode *parent;
    /* For leaf nodes */
    char *filename;                     /* NULL for internal nodes */
    char *data;                         /* NULL for internal nodes */
} MerkleNode;

/* Handle for the whole Merkle Tree */
typedef struct MerkleTree {
    MerkleNode *root;
    MerkleNode **leaves;   /* dynamic array of leaf pointers */
    size_t leaf_count;
} MerkleTree;

/* Map entry (simple hash table chaining) */
typedef struct NameMapEntry {
    char *name;
    MerkleNode *node;
    struct NameMapEntry *next;
} NameMapEntry;

typedef struct NameMap {
    NameMapEntry **buckets;
    size_t nbuckets;
} NameMap;

/* API */
void sha256_hex(const unsigned char *data, size_t len, char out_hex[HASH_HEX_LEN]);

MerkleNode *create_leaf(const char *filename, const char *data);
MerkleNode *create_parent_node(MerkleNode *left, MerkleNode *right);
void free_node_recursive(MerkleNode *node);
void free_tree(MerkleTree *tree);

/* Build tree from current leaves array (tree->leaves, leaf_count)
   Returns 0 on success, non-zero on error */
void build_merkle_tree(MerkleTree *tree);

/* Build leaves from arrays of filenames & contents */
int build_leaves_from_arrays(MerkleTree *tree, const char **filenames, const char **contents, size_t n);

/* Verify a file by name via map and computed root comparison
   Returns 1 if intact, 0 if tampered, -1 on error */
int verify_file(NameMap *map, MerkleTree *tree, const char *filename);

/* Tamper file content for testing; returns 0 on success */
int tamper_file(NameMap *map, const char *filename, const char *new_content);

/* Create / free NameMap */
NameMap *namemap_create(size_t nbuckets);
void namemap_free(NameMap *map);
int namemap_put(NameMap *map, const char *name, MerkleNode *node);
MerkleNode *namemap_get(NameMap *map, const char *name);

/* Utility */
void print_tree_levelorder(MerkleTree *tree);
char *strdup_safe(const char *s);

#endif /* MERKLE_H */
