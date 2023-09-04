/**
 * @file    headers.h
 * @author  Kieran Hillier
 * @date    4th October 2023
 * @brief   Primary header file for the MPI-based matrix convolution project.
 *
 * This header file contains the necessary includes and definitions
 * required across multiple source files in the project.
 */


#ifndef HEADERS_H
#define HEADERS_H

// Standard I/O and system libraries
#include <stdio.h>
#include <stdlib.h>

// Specific library and module headers
#include "convolution.h"
#include "mpi.h"
#include "mpi_utils.h"
#include "matrix.h"
#include "matrix_utils.h"

// Preprocessor definitions
#define MASTER 0   /* Master rank identifier in MPI context. */
#define EMPTY  0   /* Represents an empty value in the matrix. */
#define TRUE   1   /* Represents a boolean TRUE value. */
#define FALSE  0   /* Represents a boolean FALSE value. */
#define UP -1      /* Represents upward direction for padding calculations. */
#define DOWN 1     /* Represents downward direction for padding calculations. */
#define VERBOSE 1  /* If set, enables verbose logging. */

/**
 * @brief Macro to log formatted messages.
 * 
 * This macro allows for conditional logging based on the VERBOSE definition.
 * If VERBOSE is set, it will print the formatted message to stderr.
 * 
 * @param fmt Formatting string (similar to printf).
 * @param ... Variadic arguments to fit the formatting string.
 */
#define LOG(fmt, ...) do {
            if (VERBOSE) fprintf(stderr, fmt, ##__VA_ARGS__); 
        } while (0)

#endif /* HEADERS_H */
