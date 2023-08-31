#include "matrix.h"
#include "matrix_utils.h"
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Utility functions for handling matrices stored in files.
 * - Assumes matrices are square.
 * - Provides functionality to read, write, allocate, and free matrices.
 */

// Gets the size of the matrix from the file.
int get_matrix_size(const char* filename) {
    struct stat st;

    // Get file size
    if (stat(filename, &st) != 0) {
        perror("Failed to get file stats");
        return -1;
    }

    // Calculate number of integers in the file
    int total_elements = st.st_size / sizeof(int);

    // Find the dimension of the matrix (assuming it's a square matrix)
    int matrix_size = (int) round(sqrt(total_elements));

    return matrix_size;
}

// Deallocates the space used by the matrix.
void free_matrix(int** matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void cleanup(int** matrix, int matrix_size, 
            int** local_matrix, int local_rows, 
            int** processed_local_matrix, int processed_rows) {

    if (matrix) {
        free_matrix(matrix, matrix_size);
    }

    if (local_matrix) {
        free_matrix(local_matrix, local_rows);
    }

    if (processed_local_matrix) {
        free_matrix(processed_local_matrix, processed_rows);
    }
}

// Allocates space for a matrix of size rows x cols.
int** allocate_matrix(int rows, int cols) {
    int** matrix = (int**) malloc(rows * sizeof(int*));
    if (!matrix) {
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*) malloc(cols * sizeof(int));
        if (!matrix[i]) {
            // If allocation fails, free any previously allocated memory
            free_matrix(matrix, i);
            return NULL;
        }
    }
    return matrix;
}

// Reads the matrix from a file.
int** read_matrix_from_file(const char* filename, int matrix_size) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return NULL;
    }

    int** matrix = allocate_matrix(matrix_size, matrix_size);
    if (!matrix) {
        perror("Failed to allocate memory for matrix");
        close(fd);
        return NULL;
    }

    for (int row = 0; row < matrix_size; row++) {
        for (int col = 0; col < matrix_size; col++) {
            int cell_value;
            if (get_slot(fd, matrix_size, row + 1, col + 1, &cell_value) == -1) {
                fprintf(stderr, "Failed to get cell value at [%d][%d]\n", row + 1, col + 1);
                // On error, free any previously allocated memory
                free_matrix(matrix, matrix_size);
                close(fd);
                return NULL;
            }
            matrix[row][col] = cell_value;
        }
    }
    close(fd);
    return matrix;
}

// Writes the matrix to a file.
int write_matrix_to_file(const char* filename, int** matrix, int matrix_size) {
    int fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Failed to open or create file");
        return -1;
    }

    for (int row = 0; row < matrix_size; row++) {
        for (int col = 0; col < matrix_size; col++) {
            if (set_slot(fd, matrix_size, row + 1, col + 1, matrix[row][col]) == -1) {
                fprintf(stderr, "Failed to set cell value at [%d][%d]\n", row + 1, col + 1);
                close(fd);
                unlink(filename);  // Delete the potentially corrupted file
                return -1;
            }
        }
    }
    close(fd);
    return 0;
}
