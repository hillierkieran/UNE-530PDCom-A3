#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include "headers.h"

int get_i(int row, int col, int matrix_cols);

void safe_free(int **int_array);

int* allocate_matrix(int rows, int cols);

int get_padding(int proc_rank, int matrix_size, int depth,
                int rows_per_node, int direction);

int get_padded_rows(int proc_rank, int matrix_size, int depth,
                    int rows_per_node);

int* read_matrix_from_file(const char *filename, int *matrix_size);

int write_matrix_to_file(const char *filename, int *matrix, int matrix_size);

char* matrix_to_string(int* matrix, int rows, int cols);

#endif /* MATRIX_UTILS_H */
