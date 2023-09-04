/**
 * @file    mpi_utils.h
 * @author  Kieran Hillier
 * @date    4 Oct 2023
 * @brief   MPI utilities for initializing and setting up MPI-related tasks.
 */

#ifndef MPI_UTILS_H
#define MPI_UTILS_H

#include <mpi.h>

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
void mpi_setup(int *argc, char ***argv, int *rank, int *nproc);

#endif // MPI_UTILS_H
