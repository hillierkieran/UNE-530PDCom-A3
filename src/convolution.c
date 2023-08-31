#include "convolution.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

bool is_valid_input(int** matrix, int matrix_size, int row, int col, int depth) {
    return  matrix != NULL && 
            matrix_size > 0 && 
            row >= 0 && row < matrix_size && 
            col >= 0 && col < matrix_size && 
            depth >= 0;
}

int apply_convolution(int** matrix, int matrix_size, int row, int col, int depth)
{
    if (!is_valid_input(matrix, matrix_size, row, col, depth)) {
        fprintf(stderr, "Invalid input parameters for apply_convolution.\n");
        return -1;
    }

    // Offsets for scanning the neighbourhood around the given cell
    int xOffset, yOffset;

    // Accumulator for the weighted sum of neighbour values
    int weightedSum = 0.0;

    // Iterate over the neighbourhood defined by depth
    for (xOffset = -depth; xOffset <= depth; xOffset++) {
        for (yOffset = -depth; yOffset <= depth; yOffset++) {

            // Calculate neighbour's position by adding the offsets
            int neighbourRow = row + xOffset;
            int neighbourCol = col + yOffset;

            // Ensure that the position is inside the matrix boundaries
            if (neighbourRow >= 0 &&
                neighbourRow < matrix_size &&
                neighbourCol >= 0 &&
                neighbourCol < matrix_size) {

                // Determine the weight for this neighbour (ie. 1/n_depth)
                // It's based on how far the neighbour is from the current cell.
                double weight = 1.0 / (max(abs(xOffset), abs(yOffset)) + 1);

                // Add the weighted value of the neighbour to the weightedSum
                weightedSum += matrix[neighbourRow][neighbourCol] * weight;
            }
        }
    }

    // Return the final weighted sum of neighbours.
    return weightedSum;
}
