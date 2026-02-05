# Character-Level Text Embedding Generator in C

## Description
A C program that generates character-level embeddings for question–answer text pairs.
Each character is encoded using one-hot encoding based on dynamically extracted unique
characters from the dataset.

The program reads QA pairs from a file, encodes each character, and outputs fixed-size
binary embeddings suitable for further text processing or analysis.

## Features
- Parses question–answer pairs from a text file
- Extracts unique characters dynamically
- Generates one-hot character embeddings
- Supports variable-length sentences with fixed embedding dimensions
- Uses extensive dynamic memory allocation
- Outputs embeddings to a structured text file

## Concepts & Techniques
- Character-level text embeddings
- One-hot encoding
- Dynamic memory allocation (`malloc`, `free`)
- Struct-based data modeling
- File I/O and string processing
- Sorting and indexing of unique characters

## Technologies
- C
- GCC
- Standard C libraries (`stdio.h`, `stdlib.h`, `string.h`)

## Input Format
The input file (`database.txt`) contains question–answer pairs:
