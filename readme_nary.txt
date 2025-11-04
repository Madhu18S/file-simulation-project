===========================================
N-ARY TREE IMPLEMENTATION – PERSON A
====================================

Project Title  : File Simulation using N-ary Tree
Author         : Person A
Language       : C Programming
Date           : November 2025

---

1. FILE STRUCTURE

---

The project consists of the following source files:

1. nary.h

   * Header file containing:
     • Structure definition for the N-ary tree node
     • Function declarations for creation, insertion, search, and traversal

2. nary.c

   * Source file implementing the functions declared in nary.h:
     • createNode()   – Creates and initializes a new node
     • insertChild()  – Inserts a child node under a given parent
     • search()       – Recursively searches for a node by its data
     • traverse()     – Displays tree data using preorder traversal

3. nary_main.c

   * Main driver file used to test the program.
   * Builds an example tree and demonstrates:
     • Insertion of nodes
     • Searching for a node
     • Traversal of the entire tree

---

2. HOW TO COMPILE AND RUN

---

Step 1: Open the terminal (in VS Code or Command Prompt)
Step 2: Navigate to the project folder:
cd "D:\Cprog\Sem 3\file_simulation"

Step 3: Compile both source files:
gcc nary.c nary_main.c -o nary

Step 4: Run the program:
.\nary

## Expected Output:

N-ary Tree Traversal: Root A A1 A2 B B1
Node 'B1' found!

---

3. PROGRAM DETAILS

---

• Each node can have up to 10 children.
• The program builds a simple N-ary tree:
Root
├── A
│   ├── A1
│   └── A2
└── B
└── B1

• Traversal is done in preorder (Root → Children recursively).

---

4. NOTES

---

* The project is written in standard C (C99) and can be compiled using GCC.
* Ensure all files are in the same directory before compilation.
* To modify tree structure, edit the insertChild() calls in nary_main.c.

---

## END OF FILE
