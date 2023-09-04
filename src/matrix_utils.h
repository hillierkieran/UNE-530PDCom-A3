#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include "headers.h"

void free_matrix(int ***matrix, int rows);

void cleanup(int ***matrix, int matrix_size, 
            int ***input_submatrix, int input_submatrix_rows, 
            int ***output_submatrix, int output_submatrix_rows,
            int **cells_per_process, int **starts_per_process);

int** allocate_matrix(int rows, int cols);

int get_padding(int proc_rank, int matrix_size, int depth,
                int rows_per_node, int direction);

int get_padded_rows(int proc_rank, int matrix_size, int depth,
                    int rows_per_node);

int** read_matrix_from_file(const char *filename, int *matrix_size);

int write_matrix_to_file(const char *filename, int **matrix, int matrix_size);

#endif /* MATRIX_UTILS_H */
