Trie Implementation - Person B
------------------------------

Build:
    gcc main.c trie.c -o trie_demo

Usage:
    ./trie_demo --search word

Description:
    - Builds a trie from sample_files/sample.txt
    - Supports searching (--search word)
    - Measures build and 1000 search timings
    - Outputs results to results/output_trie.csv

Complexities:
    Insert  : O(n·k)
    Search  : O(k)
    Space   : O(n·k)
