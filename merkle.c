#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merkle.h"
#include "sha256.h"

/* ---------- Utility ---------- */

char *strdup_safe(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *dup = malloc(len + 1);
    if (dup) strcpy(dup, s);
    return dup;
}

/* Compute SHA-256 and store as hex */
void sha256_hex(const unsigned char *data, size_t len, char out_hex[HASH_HEX_LEN]) {
    unsigned char hash[32];
    sha256(data, len, hash);
    for (int i = 0; i < 32; i++)
        sprintf(out_hex + (i * 2), "%02x", hash[i]);
    out_hex[64] = '\0';
    return ;
}

/* ---------- Merkle Tree ---------- */

MerkleNode *create_leaf(const char *filename, const char *data) {
    MerkleNode *node = calloc(1, sizeof(MerkleNode));
    node->filename = strdup_safe(filename);
    node->data = strdup_safe(data);

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s%s", filename, data);
    sha256_hex((unsigned char *)buffer, strlen(buffer), node->hash);
    return node;
}

MerkleNode *create_parent_node(MerkleNode *left, MerkleNode *right) {
    MerkleNode *node = calloc(1, sizeof(MerkleNode));
    node->left = left;
    node->right = right;
    if (left) left->parent = node;
    if (right) right->parent = node;

    char concat[HASH_HEX_LEN * 2];
    snprintf(concat, sizeof(concat), "%s%s",
             left ? left->hash : "",
             right ? right->hash : "");
    sha256_hex((unsigned char *)concat, strlen(concat), node->hash);
    return node;
}

void free_node_recursive(MerkleNode *node) {
    if (!node) return;
    free_node_recursive(node->left);
    free_node_recursive(node->right);
    free(node->filename);
    free(node->data);
    free(node);
}

void free_tree(MerkleTree *tree) {
    if (!tree) return;
    free_node_recursive(tree->root);
    free(tree->leaves);
}

/* ---------- Hash Map (NameMap) ---------- */

static unsigned long djb2_hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

NameMap *namemap_create(size_t nbuckets) {
    NameMap *map = calloc(1, sizeof(NameMap));
    map->nbuckets = nbuckets;
    map->buckets = calloc(nbuckets, sizeof(NameMapEntry *));
    return map;
}

void namemap_free(NameMap *map) {
    if (!map) return;
    for (size_t i = 0; i < map->nbuckets; i++) {
        NameMapEntry *entry = map->buckets[i];
        while (entry) {
            NameMapEntry *tmp = entry->next;
            free(entry->name);
            free(entry);
            entry = tmp;
        }
    }
    free(map->buckets);
    free(map);
}

int namemap_put(NameMap *map, const char *name, MerkleNode *node) {
    unsigned long h = djb2_hash(name) % map->nbuckets;
    NameMapEntry *e = malloc(sizeof(NameMapEntry));
    e->name = strdup_safe(name);
    e->node = node;
    e->next = map->buckets[h];
    map->buckets[h] = e;
    return 0;
}

MerkleNode *namemap_get(NameMap *map, const char *name) {
    unsigned long h = djb2_hash(name) % map->nbuckets;
    NameMapEntry *e = map->buckets[h];
    while (e) {
        if (strcmp(e->name, name) == 0)
            return e->node;
        e = e->next;
    }
    return NULL;
}

/* ---------- Build Tree ---------- */

int build_leaves_from_arrays(MerkleTree *tree, const char **filenames, const char **contents, size_t n) {
    tree->leaf_count = n;
    tree->leaves = calloc(n, sizeof(MerkleNode *));
    for (size_t i = 0; i < n; i++)
        tree->leaves[i] = create_leaf(filenames[i], contents[i]);
    return 0;
}

int build_merkle_tree(MerkleTree *tree) {
    if (!tree || tree->leaf_count == 0) return -1;

    size_t n = tree->leaf_count;
    MerkleNode **level = tree->leaves;

    while (n > 1) {
        size_t parent_count = (n + 1) / 2;
        MerkleNode **new_level = calloc(parent_count, sizeof(MerkleNode *));

        for (size_t i = 0; i < n; i += 2)
            new_level[i / 2] = create_parent_node(level[i], (i + 1 < n) ? level[i + 1] : NULL);

        if (level != tree->leaves)
            free(level);
        level = new_level;
        n = parent_count;
    }

    tree->root = level[0];
    return 0;
}

/* ---------- Verification ---------- */

int verify_file(NameMap *map, MerkleTree *tree, const char *filename) {
    (void)map;
    MerkleNode *leaf = NULL;

    for (size_t i = 0; i < tree->leaf_count; i++) {
        if (strcmp(tree->leaves[i]->filename, filename) == 0) {
            leaf = tree->leaves[i];
            break;
        }
    }
    if (!leaf) return -1;

    char old_root[HASH_HEX_LEN];
    strcpy(old_root, tree->root->hash);

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s%s", leaf->filename, leaf->data);
    sha256_hex((unsigned char *)buffer, strlen(buffer), leaf->hash);

    build_merkle_tree(tree);

    if (strcmp(old_root, tree->root->hash) == 0) {
        printf(" - %s: ✅ Intact\n", filename);
        return 1;
    } else {
        printf(" - %s: ⚠️ Tampered\n", filename);
        return 0;
    }
}

/* ---------- Tampering Simulation ---------- */

int tamper_file(NameMap *map, MerkleTree *tree, const char *filename, const char *new_content) {
    (void)map;
    if (!tree || !filename || !new_content) return -1;

    for (size_t i = 0; i < tree->leaf_count; i++) {
        if (strcmp(tree->leaves[i]->filename, filename) == 0) {
            free(tree->leaves[i]->data);
            tree->leaves[i]->data = strdup_safe(new_content);
            printf("Tampering %s with '%s'\n", filename, new_content);
            return 0;
        }
    }
    return -1;
}


/* ---------- Print Tree ---------- */

void print_tree_levelorder(MerkleTree *tree) {
    if (!tree->root) return;
    printf("\nMerkle Root: %s\n", tree->root->hash);
}
