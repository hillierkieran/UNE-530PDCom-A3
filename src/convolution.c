#include "convolution.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

bool is_valid_input(int **matrix, int matrix_size, int row, int col, int depth)
{
    return  matrix != NULL && 
            matrix_size > 0 && 
            row >= 0 && row < matrix_size && 
            col >= 0 && col < matrix_size && 
            depth >= 0;
}

bool is_valid_cell(int row, int col, int matrix_size)
{
    return  row >= 0 &&
            row < matrix_size &&
            col >= 0 &&
            col < matrix_size;
}

int apply_convolution(int **matrix, int matrix_size, int row, int col, int depth)
{
    if (!is_valid_input(matrix, matrix_size, row, col, depth)) {
        fprintf(stderr, "Invalid input parameters for apply_convolution.\n");
        return -1;
    }

    if (depth == 0) {
        return matrix[row][col];
    }

    // Offsets for scanning the neighbourhood around the given cell
    int row_offset, col_offset;

    // Accumulator for the weighted sum of neighbour values
    int sum = 0;

    // Iterate over the neighbourhood defined by depth
    for (row_offset = -depth; row_offset <= depth; row_offset++) {
        for (col_offset = -depth; col_offset <= depth; col_offset++) {

            // Calculate neighbour's position by adding the offsets
            int neighbour_row = row + row_offset;
            int neighbour_col = col + col_offset;

            // Ensure that the position is inside the matrix boundaries
            if (is_valid_cell(neighbour_row, neighbour_col, matrix_size)) {

                // Determine the weight for this neighbour (ie. 1/n_depth)
                // It's based on how far the neighbour is from the current cell.
                double weight = 1.0 / (max(abs(row_offset), abs(col_offset)) + 1);

                // Add the weighted value of the neighbour to the sum
                sum += matrix[neighbour_row][neighbour_col] * weight;
            }
        }
    }

    // Return the final weighted sum of neighbours.
    return sum;
}
