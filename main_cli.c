/*
 * main_cli.c
 *
 * CLI driver for Merkle module used by Person D.
 *
 * Usage examples:
 *  ./merkle_demo --build 1024 --runs 10 --seed 42 --csv output_merkle.csv
 *  ./merkle_demo --build 1024 --runs 5 --seed 10 --tamper file0.txt --verify file0.txt --csv out.csv
 *
 * This expects your merkle.h/merkle.c/sha256.c implementations to provide:
 *  - int build_leaves_from_arrays(MerkleTree*, const char**, const char**, size_t)
 *  - int build_merkle_tree(MerkleTree*)
 *  - NameMap *namemap_create(size_t)
 *  - int namemap_put(NameMap*, const char*, MerkleNode*)
 *  - int verify_file(NameMap*, MerkleTree*, const char*)
 *  - int tamper_file(NameMap*, const char*, const char*)
 *  - void free_tree(MerkleTree*)
 *  - void namemap_free(NameMap*)
 *
 * CSV format written:
 * module,run_id,n,seed,op,op_time_ms,memory_bytes,result,details
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merkle.h"
#include <string.h>


#define _POSIX_C_SOURCE 200809L
#define MODULE_NAME "merkle"
#define DEFAULT_RUNS 5
#define DEFAULT_N 16
#define DEFAULT_BUCKETS 257
/* portable high-res timer */
#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
  static double now_seconds(void) {
      LARGE_INTEGER freq, pc;
      QueryPerformanceFrequency(&freq);
      QueryPerformanceCounter(&pc);
      return (double)pc.QuadPart / (double)freq.QuadPart;
  }
#else
  #include <time.h>
  static double now_seconds(void) {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
  }
#endif


/* Helper: portable timer (seconds) */

/* Generate deterministic "random" content given seed + index */
static char *gen_content(unsigned int seed, size_t idx, size_t length) {
    srand((unsigned int)(seed + (unsigned int)idx));
    char *s = malloc(length + 1);
    if (!s) return NULL;
    const char *chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
    size_t m = strlen(chars);
    for (size_t i = 0; i < length; ++i) {
        s[i] = chars[rand() % m];
    }
    s[length] = '\0';
    return s;
}

/* Build arrays of filenames and contents for n files (returns 0 on success) */
static int make_datasets(const char *prefix, size_t n, unsigned int seed,
                         char ***out_filenames, char ***out_contents) {
    char **fn = calloc(n, sizeof(char*));
    char **ct = calloc(n, sizeof(char*));
    if (!fn || !ct) { free(fn); free(ct); return -1; }
    for (size_t i = 0; i < n; ++i) {
        char buf[128];
        snprintf(buf, sizeof(buf), "%s%zu.txt", prefix, i);
        fn[i] = strdup(buf);
        /* varied length for realism */
        size_t len = 20 + (rand() % 200);
        ct[i] = gen_content(seed, i, len);
        if (!ct[i]) {
            for (size_t j = 0; j <= i; ++j) { free(fn[j]); free(ct[j]); }
            free(fn); free(ct);
            return -2;
        }
    }
    *out_filenames = fn;
    *out_contents = ct;
    return 0;
}

/* Free datasets */
static void free_datasets(char **fn, char **ct, size_t n) {
    if (!fn || !ct) return;
    for (size_t i = 0; i < n; ++i) {
        free(fn[i]);
        free(ct[i]);
    }
    free(fn);
    free(ct);
}

/* Append one CSV row (opened file pointer must be writable) */
static void csv_write_row(FILE *f, const char *module, int run_id, size_t n, unsigned int seed,
                          const char *op, double time_ms, long memory_bytes,
                          const char *result, const char *details) {
    /* CSV escaping simple: wrap details in quotes, escape internal quotes by doubling */
    fprintf(f, "%s,%d,%zu,%u,%s,%.6f,%ld,%s,\"%s\"\n",
            module, run_id, n, seed, op, time_ms, memory_bytes,
            result ? result : "", details ? details : "");
    fflush(f);
}

/* Very small wrapper to build, map names, measure and output CSV rows */
static int run_one_build(FILE *csv, int run_id, size_t n, unsigned int seed, NameMap *map_prefix) {
    char **filenames = NULL, **contents = NULL;
    int rc = make_datasets("file", n, seed, &filenames, &contents);
    if (rc != 0) return rc;

    MerkleTree tree = {0};

    /* fill leaves and time the build */
    double t0 = now_seconds();
    if (build_leaves_from_arrays(&tree, (const char**)filenames, (const char**)contents, n) != 0) {
        free_datasets(filenames, contents, n);
        return -3;
    }
    if (map_prefix) {
        for (size_t i = 0; i < tree.leaf_count; ++i) {
            namemap_put(map_prefix, tree.leaves[i]->filename, tree.leaves[i]);
        }
    }
    if (build_merkle_tree(&tree)) {
        free_tree(&tree);
        free_datasets(filenames, contents, n);
        return -4;
    }
    double t1 = now_seconds();
    double build_ms = (t1 - t0) * 1000.0;

    /* memory unknown here (set 0) */
    long mem = 0;
    csv_write_row(csv, MODULE_NAME, run_id, n, seed, "build", build_ms, mem, "ok", tree.root ? tree.root->hash : "no_root");

    /* We keep tree and map live for possible verify/tamper during the same run.
       The caller may want to tamper/verify using the same tree; so we return the tree by storing
       it in a file-scoped or caller-managed structure. For our CLI, we'll free at end of function.
    */

    /* For convenience, print root to stdout */
    if (tree.root) printf("Run %d build complete: root=%s (time=%.3f ms)\n", run_id, tree.root->hash, build_ms);

    /* Cleanup */
    free_tree(&tree);
    free_datasets(filenames, contents, n);
    return 0;
}

/* For runs that require tamper + verify in same run, we provide a function that builds and returns tree + map.
   But to keep this file small, Person D will typically call --build then invoke --verify separately.
   Below are simplified verify/tamper helpers that reconstruct the same data (deterministic via seed). */

/* Verify a named file: builds structure, then measures verify */
static int run_verify(FILE *csv, int run_id, size_t n, unsigned int seed, const char *target_filename) {
    char **filenames = NULL, **contents = NULL;
    int rc = make_datasets("file", n, seed, &filenames, &contents);
    if (rc != 0) return rc;

    MerkleTree tree = {0};
    NameMap *map = namemap_create(DEFAULT_BUCKETS);

    if (build_leaves_from_arrays(&tree, (const char**)filenames, (const char**)contents, n) != 0) {
        free_datasets(filenames, contents, n);
        namemap_free(map);
        return -3;
    }
    for (size_t i = 0; i < tree.leaf_count; ++i) namemap_put(map, tree.leaves[i]->filename, tree.leaves[i]);
    if (build_merkle_tree(&tree)) {
        free_tree(&tree); free_datasets(filenames, contents, n); namemap_free(map);
        return -4;
    }

    double t0 = now_seconds();
    int res = verify_file(map, &tree, target_filename);
    double t1 = now_seconds();
    double v_ms = (t1 - t0) * 1000.0;
    csv_write_row(csv, MODULE_NAME, run_id, n, seed, "verify", v_ms, 0, (res==1) ? "ok" : (res==0) ? "tampered" : "error", target_filename);

    free_tree(&tree);
    namemap_free(map);
    free_datasets(filenames, contents, n);
    return 0;
}

/* Tamper a named file and measure verify post-tamper: */
static int run_tamper_and_verify(FILE *csv, int run_id, size_t n, unsigned int seed,
                                const char *target_filename, const char *new_content) {
    /* Build deterministic dataset and map */
    char **filenames = NULL, **contents = NULL;
    int rc = make_datasets("file", n, seed, &filenames, &contents);
    if (rc != 0) return rc;

    MerkleTree tree = {0};
    NameMap *map = namemap_create(DEFAULT_BUCKETS);

    if (build_leaves_from_arrays(&tree, (const char**)filenames, (const char**)contents, n) != 0) {
        free_datasets(filenames, contents, n); namemap_free(map); return -3;
    }
    for (size_t i = 0; i < tree.leaf_count; ++i) namemap_put(map, tree.leaves[i]->filename, tree.leaves[i]);
    if (build_merkle_tree(&tree)) {
        free_tree(&tree); free_datasets(filenames, contents, n); namemap_free(map); return -4;
    }

    /* Verify before tamper */
    double t0 = now_seconds();
    int res_before = verify_file(map, &tree, target_filename);
    double t1 = now_seconds();
    csv_write_row(csv, MODULE_NAME, run_id, n, seed, "verify_before_tamper", (t1 - t0)*1000.0, 0,
                  (res_before==1) ? "ok" : (res_before==0) ? "tampered" : "error", target_filename);

    /* Tamper (should change only data) */
    tamper_file(map, &tree, target_filename, new_content);


    /* Verify after tamper */
    double t2 = now_seconds();
    int res_after = verify_file(map, &tree, target_filename);
    double t3 = now_seconds();
    csv_write_row(csv, MODULE_NAME, run_id, n, seed, "verify_after_tamper", (t3 - t2)*1000.0, 0,
                  (res_after==1) ? "ok" : (res_after==0) ? "tampered" : "error", target_filename);

    /* Optionally rebuild and record rebuild time */
    double t4 = now_seconds();
    build_merkle_tree(&tree);
    double t5 = now_seconds();
    csv_write_row(csv, MODULE_NAME, run_id, n, seed, "rebuild_after_tamper", (t5 - t4)*1000.0, 0, "ok", "rebuild");

    /* Cleanup */
    free_tree(&tree);
    namemap_free(map);
    free_datasets(filenames, contents, n);
    return 0;
}

/* Simple CLI arg parsing (naive) */
int main(int argc, char **argv) {
    size_t n = DEFAULT_N;
    int runs = DEFAULT_RUNS;
    unsigned int seed = 42;
    char csv_path[512] = "output_merkle.csv";
    int do_build = 0, do_verify = 0, do_tamper = 0;
    char verify_target[256] = {0};
    char tamper_target[256] = {0}, tamper_content[512] = {0};

    /* parse arguments */
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--build") == 0 && i+1 < argc) { n = (size_t)atoi(argv[++i]); do_build = 1; }
        else if (strcmp(argv[i], "--runs") == 0 && i+1 < argc) { runs = atoi(argv[++i]); }
        else if (strcmp(argv[i], "--seed") == 0 && i+1 < argc) { seed = (unsigned int)atoi(argv[++i]); }
        else if (strcmp(argv[i], "--csv") == 0 && i+1 < argc) { strncpy(csv_path, argv[++i], sizeof(csv_path)-1); }
        else if (strcmp(argv[i], "--verify") == 0 && i+1 < argc) { strncpy(verify_target, argv[++i], sizeof(verify_target)-1); do_verify = 1; }
        else if (strcmp(argv[i], "--tamper") == 0 && i+2 < argc) { strncpy(tamper_target, argv[++i], sizeof(tamper_target)-1); strncpy(tamper_content, argv[++i], sizeof(tamper_content)-1); do_tamper = 1; }
        else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--build N] [--runs R] [--seed S] [--verify filename] [--tamper filename newcontent] [--csv out.csv]\n", argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown arg: %s\n", argv[i]);
            return 1;
        }
    }

    FILE *csv = fopen(csv_path, "w");
    if (!csv) { perror("fopen"); return 1; }
    /* CSV header */
    fprintf(csv, "module,run_id,n,seed,op,op_time_ms,memory_bytes,result,details\n");

    /* For each run: call requested ops */
    for (int run = 1; run <= runs; ++run) {
        unsigned int this_seed = seed + (unsigned int)run; /* vary seed per run for variability but deterministic */
        if (do_build) {
            int rc = run_one_build(csv, run, n, this_seed, NULL);
            if (rc != 0) fprintf(stderr, "run_one_build failed (rc=%d)\n", rc);
        }
        if (do_verify) {
            int rc = run_verify(csv, run, n, this_seed, verify_target);
            if (rc != 0) fprintf(stderr, "run_verify failed (rc=%d)\n", rc);
        }
        if (do_tamper) {
            int rc = run_tamper_and_verify(csv, run, n, this_seed, tamper_target, tamper_content);
            if (rc != 0) fprintf(stderr, "run_tamper_and_verify failed (rc=%d)\n", rc);
        }
    }

    fclose(csv);
    printf("Results written to %s\n", csv_path);
    return 0;
}
