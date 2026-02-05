#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAR 128  
#define MAX_PAIRS 2048
#define EMBEDDING_DIM 65
#define MAX_SENTENCE_LENGTH 350  

typedef struct {
    char *sentence;  
    char **embedding;  
    int char_count;  
} SentenceEmbedding;

typedef struct {
    SentenceEmbedding question;
    SentenceEmbedding answer;
} QA;

char unique_chars[MAX_CHAR];
int char_count = 0;
int max_sentence_embedding_len = 0;  

void sort_unique_chars() {
    int i, j;
    for (i = 0; i < char_count - 1; ++i) {
        for (j = i + 1; j < char_count; ++j) {
            if (unique_chars[i] > unique_chars[j]) {
                char tmp = unique_chars[i];
                unique_chars[i] = unique_chars[j];
                unique_chars[j] = tmp;
            }
        }
    }
}

int add_unique_char(char c) {
    int i;
    for (i = 0; i < char_count; ++i)
        if (unique_chars[i] == c)
            return i;

    if (char_count < MAX_CHAR) {
        unique_chars[char_count++] = c;
        return char_count - 1;
    }

    return -1;  
}

void encode_character(char c, char *embedding) {
    int j;
    for (j = 0; j < EMBEDDING_DIM; ++j) {
        embedding[j] = (c == unique_chars[j]) ? '1' : '0';
    }
    embedding[EMBEDDING_DIM] = '\0';
}

int extract_unique_chars(QA *pairs, int count) {
    int i, j;
    char_count = 0;
    for (i = 0; i < count; ++i) {
        for (j = 0; j < pairs[i].question.char_count; ++j)
            add_unique_char(pairs[i].question.sentence[j]);

        for (j = 0; j < pairs[i].answer.char_count; ++j)
            add_unique_char(pairs[i].answer.sentence[j]);
    }

    sort_unique_chars();
    return char_count;
}

int parse_file(const char *filename, QA *pairs) {
    FILE *file = fopen(filename, "r");
    char line[2048];
    int idx = 0;

    if (!file) {
        perror("Error opening input file");
        return 0;
    }

    while (fgets(line, sizeof(line), file) && idx < MAX_PAIRS) {
        line[strcspn(line, "\r\n")] = '\0';

        if (strncmp(line, "Q:", 2) == 0) {
            pairs[idx].question.sentence = strdup(line + 2);
            pairs[idx].question.char_count = strlen(pairs[idx].question.sentence);
        } else if (strncmp(line, "A:", 2) == 0) {
            pairs[idx].answer.sentence = strdup(line + 2);
            pairs[idx].answer.char_count = strlen(pairs[idx].answer.sentence);
        } else if (strncmp(line, "---", 3) == 0) {
            ++idx;
        }
    }

    fclose(file);
    return idx;
}

void allocate_embeddings(QA *pairs, int num_pairs) {
    int i, j;
    for (i = 0; i < num_pairs; ++i) {
        pairs[i].question.embedding = (char **)malloc(MAX_SENTENCE_LENGTH * sizeof(char *));
        pairs[i].answer.embedding = (char **)malloc(MAX_SENTENCE_LENGTH * sizeof(char *));
        
        for (j = 0; j < MAX_SENTENCE_LENGTH; ++j) {
            pairs[i].question.embedding[j] = (char *)malloc((EMBEDDING_DIM + 1) * sizeof(char));
            memset(pairs[i].question.embedding[j], '0', EMBEDDING_DIM);  
            pairs[i].question.embedding[j][EMBEDDING_DIM] = '\0';

            pairs[i].answer.embedding[j] = (char *)malloc((EMBEDDING_DIM + 1) * sizeof(char));
            memset(pairs[i].answer.embedding[j], '0', EMBEDDING_DIM);  
            pairs[i].answer.embedding[j][EMBEDDING_DIM] = '\0';
        }
    }
}

void free_embeddings(QA *pairs, int num_pairs) {
    int i, j;
    for (i = 0; i < num_pairs; ++i) {
        free(pairs[i].question.sentence);
        free(pairs[i].answer.sentence);

        for (j = 0; j < MAX_SENTENCE_LENGTH; ++j) {
            free(pairs[i].question.embedding[j]);
            free(pairs[i].answer.embedding[j]);
        }
        free(pairs[i].question.embedding);
        free(pairs[i].answer.embedding);
    }
}

int main() {
    QA *pairs;
    int num_pairs, char_dim, i, j, sentence_length;
    FILE *out;

    pairs = (QA *)malloc(MAX_PAIRS * sizeof(QA));
    if (!pairs) {
        perror("Memory allocation failed");
        return 1;
    }

    num_pairs = parse_file("database.txt", pairs);
    if (num_pairs == 0) {
        fprintf(stderr, "No QA pairs found.\n");
        free(pairs);
        return 1;
    }

    char_dim = extract_unique_chars(pairs, num_pairs);
    allocate_embeddings(pairs, num_pairs);

    for (i = 0; i < num_pairs; ++i) {
        sentence_length = 0;
        for (j = 0; j < pairs[i].question.char_count; ++j) {
            encode_character(pairs[i].question.sentence[j], pairs[i].question.embedding[j]);
            sentence_length += strlen(pairs[i].question.embedding[j]);
        }
        if (sentence_length > max_sentence_embedding_len) max_sentence_embedding_len = sentence_length;

        sentence_length = 0;
        for (j = 0; j < pairs[i].answer.char_count; ++j) {
            encode_character(pairs[i].answer.sentence[j], pairs[i].answer.embedding[j]);
            sentence_length += strlen(pairs[i].answer.embedding[j]);
        }
        if (sentence_length > max_sentence_embedding_len) max_sentence_embedding_len = sentence_length;
    }

    out = fopen("embeddings.txt", "w");
    if (!out) {
        perror("Failed to open output file");
        free_embeddings(pairs, num_pairs);
        free(pairs);
        return 1;
    }

    fprintf(out, "CSE102 Assignment 9\n");
    fprintf(out, "Dimension of letter embeddings: %d\n", char_dim);
    fprintf(out, "Number of QA pairs: %d\n", num_pairs);
    fprintf(out, "Maximum embedding length: %d\n", max_sentence_embedding_len);
    fprintf(out, "#####\n");

    for (i = 0; i < num_pairs; ++i) {
        fprintf(out, "Question:\n");
        for (j = 0; j < MAX_SENTENCE_LENGTH; ++j)
            fprintf(out, "%s\n", pairs[i].question.embedding[j]);

        fprintf(out, "Answer:\n");
        for (j = 0; j < MAX_SENTENCE_LENGTH; ++j)
            fprintf(out, "%s\n", pairs[i].answer.embedding[j]);

        fprintf(out, "---\n");
    }

    fclose(out);
    free_embeddings(pairs, num_pairs);
    free(pairs);
    printf("embeddings.txt generated successfully.\n");

    return 0;
}
