/**
 * @file    mpi_utils.c
 * @author  Kieran Hillier
 * @date    4 Oct 2023
 * @brief   Implementation of MPI utilities functions.
 *
 * Contains the implementation of utility functions declared in mpi_utils.h for
 * initializing and managing MPI-related tasks.
 */

#include "mpi.h"
#include "mpi_utils.h"
#include <stdio.h>

/**
 * @brief Sets up the MPI environment.
 *
 * This function initializes the MPI environment, sets error handlers,
 * and retrieves the rank and number of processes.
 * 
 * @param [in,out] argc The number of arguments from the main function.
 * @param [in,out] argv The arguments from the main function.
 * @param [out] rank The rank of the current process.
 * @param [out] nproc The total number of processes.
 */
void mpi_setup(int *argc, char ***argv, int *rank, int *nproc)
{
    int mpi_err;

    mpi_err = MPI_Init(argc, argv);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error initializing MPI.\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    // Set MPI to return errors rather than directly terminating the application
    //MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

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
