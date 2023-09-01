#include "convolution.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

bool is_valid_cell(int row, int col, int matrix_rows, int matrix_cols)
{
    return  row >= 0 &&
            col >= 0 &&
            row < matrix_rows &&
            col < matrix_cols;
}

bool is_valid_input(int row, int col, int **matrix,
                    int matrix_rows, int matrix_cols, int depth)
{
    return  matrix != NULL &&
            matrix_rows > 0 &&
            matrix_cols > 0 &&
            depth >= 0 &&
            is_valid_cell(row, col, matrix_rows, matrix_rows);
}

int apply_convolution(  int row, int col, int **matrix, 
                        int matrix_rows, int matrix_cols, int depth)
{
    if (!is_valid_input(row, col, matrix, matrix_rows, matrix_cols, depth)) {
        fprintf(stderr, "Invalid input parameters for apply_convolution.\n");
        return -1;
    }

    if (depth == 0) {
        return matrix[row][col];
    }

    int row_offset, col_offset, sum = 0;
    for (row_offset = -depth; row_offset <= depth; row_offset++) {
        for (col_offset = -depth; col_offset <= depth; col_offset++) {

            // Calculate neighbouring cell's position by adding the offsets
            int neighbour_row = row + row_offset;
            int neighbour_col = col + col_offset;

            // Ensure that the position is inside the matrix boundaries
            if (is_valid_cell(  neighbour_row, neighbour_col,
                                matrix_rows, matrix_cols)) {
                
                // Determine the weight for this neighbour
                double n_depth = fmax(abs(row_offset), abs(col_offset)) + 1;
                if (n_depth == 0.0) {
                    fprintf(stderr, "Divide by zero in apply_convolution.\n");
                    return -1;
                }

                double weight = 1 / n_depth;

                // Add the weighted value of the neighbour to the sum
                sum += matrix[neighbour_row][neighbour_col] * weight;
            }
        }
    }

    // Return the final weighted sum of neighbours.
    return sum;
}
