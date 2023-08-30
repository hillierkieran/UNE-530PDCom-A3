#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include <stdio.h>
#include <stdlib.h>

int get_matrix_size(const char* filename);
int** read_matrix_from_file(const char* filename, int matrix_size);
int** allocate_matrix(int rows, int cols);
void write_matrix_to_file(const char* filename, int** matrix, int matrix_size);
void free_matrix(int** matrix, int rows);

#endif /* MATRIX_UTILS_H */
