#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>   // for high-precision timing
#include "trie.h"

int main(int argc, char *argv[]) {
    TrieNode *root = createNode();

    FILE *fp = fopen("sample_files/sample.txt", "r");
    if (!fp) {
        printf("Error: could not open sample_files/sample.txt\n");
        return 1;
    }

    char word[256];

    // ---------- Build timing ----------
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    while (fscanf(fp, "%s", word) != EOF)
        insertFile(root, word);

    QueryPerformanceCounter(&end);
    double build_time = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    fclose(fp);

    // ---------- Search feature ----------
    if (argc == 3 && strcmp(argv[1], "--search") == 0) {
        const char *query = argv[2];
        printf("Searching for '%s'...\n", query);
        if (searchFile(root, query))
            printf("Result: Found\n");
        else
            printf("Result: Not Found\n");
    }

    // ---------- 1000 lookups timing ----------
    QueryPerformanceCounter(&start);
    for (int i = 0; i < 1000; i++)
        searchFile(root, "dummy.txt");
    QueryPerformanceCounter(&end);
    double search_time = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;

    // ---------- Save results ----------
    system("if not exist results mkdir results");
    FILE *csv = fopen("results/output_trie.csv", "w");
    fprintf(csv, "BuildTime(s),SearchTime(s)\n%.6f,%.6f\n", build_time, search_time);
    fclose(csv);

    printf("\nBuild time: %.6f s\n", build_time);
    printf("Search time (1000 lookups): %.6f s\n", search_time);
    printf("Results saved in results/output_trie.csv\n");

    freeTrie(root);
    return 0;
}
