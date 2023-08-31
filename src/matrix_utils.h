#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include <stdio.h>
#include <stdlib.h>

int get_matrix_size(const char* filename);
void free_matrix(int** matrix, int rows);
void cleanup(int** matrix, int matrix_size, 
            int** input_submatrix, int input_submatrix_rows, 
            int** output_submatrix, int output_submatrix_rows,
            int* cells_per_process, int* starts_per_process);
void compute_local_rows(int proc_rank, int local_rows, int depth, 
                        int matrix_size, int *start_row, int *end_row);
int** allocate_matrix(int rows, int cols);
int** read_matrix_from_file(const char* filename, int matrix_size);
int write_matrix_to_file(const char* filename, int** matrix, int matrix_size);

#endif /* MATRIX_UTILS_H */
