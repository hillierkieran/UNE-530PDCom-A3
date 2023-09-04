/**
 * @file    matrix_utils.h
 * @author  Kieran Hillier
 * @date    4th October 2023
 * @brief Utilities for matrix operations.
 *
 * This header file contains utility functions that assist in handling matrices.
 * It includes functions for allocation, reading, writing,
 * and transforming matrices.
 */

#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include "headers.h"

/**
 * @brief Convert 2D matrix indices to 1D index for arrays.
 * 
 * @param row The row number in the matrix.
 * @param col The column number in the matrix.
 * @param matrix_cols Total columns in the matrix.
 * @return int The corresponding index in a 1D array representation.
 */
int get_i(int row, int col, int matrix_cols);

/**
 * @brief Safely free allocated memory and set pointer to NULL.
 * 
 * @param int_array Pointer to the int array to be freed.
 */
void safe_free(int **int_array);

/**
 * @brief Allocate space for a matrix.
 * 
 * @param rows Number of rows.
 * @param cols Number of columns.
 * @return int* Pointer to the allocated matrix. NULL if allocation failed.
 */
int* allocate_matrix(int rows, int cols);

/**
 * @brief Get the amount of padding required for a process.
 * 
 * @param proc_rank Rank of the process.
 * @param matrix_size Size of the matrix (number of rows or columns).
 * @param depth Depth of the convolution.
 * @param rows_per_node Rows assigned to each process.
 * @param direction Direction to check padding (UP = -1, DOWN = 1).
 * @return Number of rows of padding required.
 */
int get_padding(int proc_rank, int matrix_size, int depth,
                int rows_per_node, int direction);

/**
 * @brief Calculate the number of rows a process needs to handle 
 *        after considering padding.
 * 
 * @param proc_rank Rank of the process.
 * @param matrix_size Size of the matrix (number of rows or columns).
 * @param depth Depth of the convolution.
 * @param rows_per_node Rows assigned to each process.
 * @return Total number of rows for the process after adding padding.
 */
int get_padded_rows(int proc_rank, int matrix_size, int depth,
                    int rows_per_node);

/**
 * @brief Read a matrix from a file.
 * 
 * @param filename Name of the file to read from.
 * @param matrix_size Pointer to an int where the matrix's size will be stored.
 * @return Pointer to the read matrix. NULL if reading fails.
 */
int* read_matrix_from_file(const char *filename, int *matrix_size);

/**
 * @brief Write a matrix to a file.
 * 
 * @param filename Name of the file to write to.
 * @param matrix Pointer to the matrix to write.
 * @param matrix_size Size of the matrix to write.
 * @return 0 if the write operation succeeds,
 *        -1 if failed to write,
 *        -2 if failed to close the file.
 */
int write_matrix_to_file(const char *filename, int *matrix, int matrix_size);

/**
 * @brief Convert a matrix to a string for display.
 * 
 * @param matrix Pointer to the matrix.
 * @param rows Number of rows in the matrix.
 * @param cols Number of columns in the matrix.
 * @return char* Pointer to the string representation of the matrix.
 */
char* matrix_to_string(int* matrix, int rows, int cols);

#endif /* MATRIX_UTILS_H */
