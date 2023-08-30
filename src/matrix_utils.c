#include "matrix.h"
#include "matrix_utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

// Utility Functions

// Gets the size of the matrix from the file.
int get_matrix_size(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char ch;
    int size = 0;
    int line_count = 0;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == ' ') {  // Count spaces to deduce number of columns
            size++;
        }
        if (ch == '\n') {
            line_count++;
            // We only need to read the first line to get the number of columns
            break;
        }
    }
    fclose(file);

    // Since it's a square matrix, the number of rows equals the number of columns
    return size + 1;  // +1 because there is one less space than numbers in a line
}


// Reads the matrix from a file.
// Assumes the matrix is square and of size matrix_size.
int** read_matrix_from_file(const char* filename, int matrix_size) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int** matrix = allocate_matrix(matrix_size, matrix_size);
    int slot;
    for (int i = 1; i <= matrix_size; i++) {
        for (int j = 1; j <= matrix_size; j++) {
            if(get_slot(fd, matrix_size, i, j, &slot) == -1) {
                fprintf(stderr, "Failed to get slot at [%d][%d]\n", i, j);
                exit(EXIT_FAILURE);
            }
            matrix[i-1][j-1] = slot;
        }
    }
    close(fd);
    return matrix;
}

// Allocates space for a matrix of size rows x cols
int** allocate_matrix(int rows, int cols) {
    int** matrix = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*)malloc(cols * sizeof(int));
    }
    return matrix;
}

// Writes the matrix to a file.
void write_matrix_to_file(const char* filename, int** matrix, int matrix_size) {
    int fd = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i <= matrix_size; i++) {
        for (int j = 1; j <= matrix_size; j++) {
            if(set_slot(fd, matrix_size, i, j, matrix[i-1][j-1]) == -1) {
                fprintf(stderr, "Failed to set slot at [%d][%d]\n", i, j);
                exit(EXIT_FAILURE);
            }
        }
    }
    close(fd);
}

// Deallocates the space used by the matrix
void free_matrix(int** matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}
