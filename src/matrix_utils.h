#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include <stdio.h>
#include <stdlib.h>

int   get_matrix_size(const char* filename);
void  free_matrix(int** matrix, int rows);
void cleanup(int** matrix, int matrix_size, 
            int** local_matrix, int local_rows, 
            int** processed_local_matrix, int processed_rows);
int** allocate_matrix(int rows, int cols);
int** read_matrix_from_file(const char* filename, int matrix_size);
int   write_matrix_to_file(const char* filename, int** matrix, int matrix_size);

#endif /* MATRIX_UTILS_H */
