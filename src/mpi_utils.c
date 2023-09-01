#include "mpi.h"
#include "mpi_utils.h"

void mpi_setup(int *argc, char ***argv, int *rank, int *nproc)
{
    int mpi_err;

    mpi_err = MPI_Init(argc, argv);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error initializing MPI.\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    // Set MPI to return errors rather than directly terminating the application
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    mpi_err = MPI_Comm_rank(MPI_COMM_WORLD, rank);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error getting process rank.\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    mpi_err = MPI_Comm_size(MPI_COMM_WORLD, nproc);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error getting number of processes.\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }
}
