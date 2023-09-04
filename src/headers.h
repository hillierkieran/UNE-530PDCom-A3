#include <stdio.h>
#include <stdlib.h>
#include "convolution.h"
#include "mpi.h"
#include "mpi_utils.h"
#include "matrix.h"
#include "matrix_utils.h"
#define MASTER 0
#define EMPTY  0
#define TRUE   1
#define FALSE  0
#define UP -1
#define DOWN 1
#define VERBOSE 1  // Set to 1 for verbose output, 0 to disable.
#define LOG(fmt, ...) do { if (VERBOSE) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
