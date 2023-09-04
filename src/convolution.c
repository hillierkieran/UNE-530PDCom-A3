/**
 * @file    convolution.c
 * @author  Kieran Hillier
 * @date    4th October 2023
 * @brief   Implementation of convolution-related operations.
 *
 * The functions defined in this file perform convolution operations on matrices
 */

#include "convolution.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * @brief Determines if the coordinates are within matrix bounds.
 * 
 * @param row Row coordinate.
 * @param col Column coordinate.
 * @param matrix_rows Number of rows in the matrix.
 * @param matrix_cols Number of columns in the matrix.
 * @return 1 if within bounds, 0 otherwise.
 */
bool is_valid_cell(int row, int col, int matrix_rows, int matrix_cols)
{
    return  row >= 0 &&
            col >= 0 &&
            row < matrix_rows &&
            col < matrix_cols;
}

/**
 * @brief Validates the input parameters for convolution operations.
 * 
 * @param row Row coordinate.
 * @param col Column coordinate.
 * @param matrix Pointer to the matrix data.
 * @param matrix_rows Number of rows in the matrix.
 * @param matrix_cols Number of columns in the matrix.
 * @param depth Convolution depth.
 * @return true if all parameters are valid, false otherwise.
 */
bool is_valid_input(int row, int col, int *matrix,
                    int matrix_rows, int matrix_cols, int depth)
{
    return  matrix != NULL &&
            matrix_rows > 0 &&
            matrix_cols > 0 &&
            depth >= 0 &&
            is_valid_cell(row, col, matrix_rows, matrix_cols);
}

/**
 * @brief Applies convolution operation on the specified cell of the matrix.
 * 
 * This function takes in the coordinates of a matrix cell, the matrix itself,
 * its dimensions, and a depth value to perform convolution. The function
 * computes the weighted sum of the specified cell's neighbours up to the
 * provided depth.
 *
 * @param row Row coordinate of the cell.
 * @param col Column coordinate of the cell.
 * @param matrix Pointer to the matrix.
 * @param matrix_rows Number of rows in the matrix.
 * @param matrix_cols Number of columns in the matrix.
 * @param depth Depth for convolution operation.
 * @return Sum after applying convolution.
 *         Returns -1 if the parameters are invalid.
 */
int apply_convolution(  int row, int col, int *matrix, 
                        int matrix_rows, int matrix_cols, int depth)
{
    if (!is_valid_input(row, col, matrix, matrix_rows, matrix_cols, depth)) {
        fprintf(stderr, "Invalid input parameters for apply_convolution "
                "(row:%d, col:%d, matrix:%p, "
                "matrix_rows:%d, matrix_cols:%d, depth:%d)\n",
                row, col, (void*)matrix, matrix_rows, matrix_cols, depth);
        return -1;
    }

    if (depth == 0) {
        return matrix[row * matrix_cols + col];
    }

    /*
    fprintf(stderr, "Applying convolution to cell %d,%d from matrix:%p "
            "Starting value = %d\n",
            row, col, (void*)matrix, matrix[row * matrix_cols + col]);
    */

    int sum = 0;
    int row_offset, col_offset;
    for (row_offset = -depth; row_offset <= depth; row_offset++) {
        for (col_offset = -depth; col_offset <= depth; col_offset++) {

            // Calculate neighbouring cell's position by adding the offsets
            int neighbour_row = row + row_offset;
            int neighbour_col = col + col_offset;

            // Ensure that the position is inside the matrix boundaries
            if (!is_valid_cell( neighbour_row, neighbour_col,
                                matrix_rows, matrix_cols)) {
                continue;
            }
            if (neighbour_row == row && neighbour_col == col) {
                continue;
            }
            /*
            fprintf(stderr, "(Convolution loop) "
                    "Adding neighbour %d,%d with value:%d to sum\n",
                    neighbour_row, neighbour_col,
                    matrix[neighbour_row][neighbour_col]);
            */

            // Determine the weight for this neighbour
            double n_depth = fmax(abs(row_offset), abs(col_offset)) + 1;
            double weight = 1 / n_depth;

            // Add the weighted value of the neighbour to the sum
            sum += matrix[neighbour_row *matrix_cols + neighbour_col] * weight;
        }
    }

    /*
    fprintf(stderr, "Convolution of cell %d,%d is complete. "
            "New value = %d\n",
            row, col, sum);
    */

    // Return the final weighted sum of neighbours.
    return sum;
}
