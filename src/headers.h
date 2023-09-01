#include <stdio.h>
#include <stdlib.h>
#include "convolution.h"
#include "mpi.h"
#include "mpi_utils.h"
#include "matrix.h"
#include "matrix_utils.h"
#define MASTER 0
#define EMPTY -1
#define TRUE   1
#define FALSE  0
#define UP -1
#define DOWN 1
