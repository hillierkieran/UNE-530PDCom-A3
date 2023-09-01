#ifndef MPI_UTILS_H
#define MPI_UTILS_H

#include <mpi.h>

void mpi_setup(int *argc, char ***argv, int *rank, int *nproc);

#endif // MPI_UTILS_H
