/**
 * @file    convolution.h
 * @author  Kieran Hillier
 * @date    4th October 2023
 * @brief   Header file containing the function declarations for
 *          convolution-related operations.
 */

#ifndef CONVOLUTION_H
#define CONVOLUTION_H

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
                        int matrix_rows, int matrix_cols, int depth);

#endif /* CONVOLUTION_H */
