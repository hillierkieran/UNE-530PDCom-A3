#include "headers.h"

int main(int argc, char **argv)
{
    int     my_rank,    // Rank of this process (node)
            nproc,      // Number of processes (nodes)
            depth,      // Number of neighbours to include in the convolution
            mpi_err,    // Error codes returned from MPI functions 
            matrix_size    = -1,    // Size of the master matrix
            my_padded_rows = -1,    // Size of submatrix plus depth
            rows_per_node  = -1,    // Size of processed submatrix
            my_top_padding,
            my_bottom_padding,
            *cells_per_process       = NULL,    // Number of rows per node
            *starts_per_process      = NULL,    // Starting element per node
            **matrix                 = NULL,    // Main matrix
            **my_padded_submatrix    = NULL,    // Padded working sub-matrix
            **my_processed_submatrix = NULL;    // Processed output sub-matrix

    char    *input_filename,    // Filename of input matrix
            *output_filename;   // Filename of output matrix


    // Setup MPI (initialise, get rank and number of processes)
    mpi_setup(&argc, &argv, &my_rank, &nproc);


    // Parse args
    if (argc != 4 || (depth = atoi(argv[3])) < 0) {
        fprintf(stderr, "Usage: %s [input] [output] [depth]\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }


    // Master process retrieves matrix from file
    if (my_rank == MASTER) {
        input_filename = argv[1];
        output_filename = argv[2];
        matrix = read_matrix_from_file(input_filename, &matrix_size);
        if (matrix_size <= 0 || !matrix) {
            fprintf(stderr, "Failed to read matrix from file: %s\n", input_filename);
            free_matrix(matrix, matrix_size);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // If zero depth, no work to do. Write input matrix to output file 
        if (depth == 0) {
        int result = write_matrix_to_file(output_filename, matrix, matrix_size);
        if (result == -1) {
            fprintf(stderr, "Failed to write matrix to output file %s.\n", output_filename);
        } else if (result == -2) {
            fprintf(stderr, "Failed to close the file after writing matrix to output file %s.\n", output_filename);
        }
        free_matrix(matrix, matrix_size);
        MPI_Finalize();
        return EXIT_SUCCESS;
        }
    }


    // Broadcast the master matrix's size from master to all processes
    mpi_err = MPI_Bcast(&matrix_size, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error broadcasting matrix size.\n");
        free_matrix(matrix, matrix_size);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }


    // All processes compute the size of their submatrix
    rows_per_node = matrix_size / nproc;
    my_top_padding = get_padding(my_rank, matrix_size, depth, rows_per_node, UP);
    my_bottom_padding = get_padding(my_rank, matrix_size, depth, rows_per_node, DOWN);
    my_padded_rows = my_top_padding + rows_per_node + my_bottom_padding;
    if (my_top_padding < 0 || my_bottom_padding < 0 || my_padded_rows <= 0) {
        fprintf(stderr, "Error calculating padding.\n");
        if (my_rank == MASTER)
            free_matrix(matrix, matrix_size);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }


    // All processes allocate space for their padded submatrix
    my_padded_submatrix = allocate_matrix(my_padded_rows, matrix_size);
    if (!my_padded_submatrix) {
        fprintf(stderr, "Failed to allocate memory for padded submatrix");
        if (my_rank == MASTER) {
            free_matrix(matrix, matrix_size);
        }
        free_matrix(my_padded_submatrix, my_padded_rows);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }


    // Master process determines the number of elements and starting element 
    // to send to each process
    if (my_rank == MASTER) {
        // Allocate space for cells_per_process and starts_per_process
        cells_per_process  = (int *)malloc(nproc * sizeof(int));
        starts_per_process = (int *)malloc(nproc * sizeof(int));
        if (!cells_per_process || !starts_per_process) {
            fprintf(stderr, "Failed to allocate memory for cells_per_process or starts_per_process");
            cleanup(&matrix, matrix_size, 
                    &my_padded_submatrix, my_padded_rows, NULL, EMPTY,
                    &cells_per_process, &starts_per_process);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        for(int proc = 0; proc < nproc; proc++) {
            int top_padding = get_padding(proc, matrix_size, depth, rows_per_node, UP);
            int bottom_padding = get_padding(proc, matrix_size, depth, rows_per_node, DOWN);
            int padded_portion = top_padding + rows_per_node + bottom_padding;

            cells_per_process[proc] = padded_portion * matrix_size;
            starts_per_process[proc] = proc * rows_per_node - top_padding;
        }
    }


    // TODO: Check this is correct
    // Distribute sub-matrices to processes
    mpi_err = MPI_Scatterv(
        matrix[0],                      // send buffer
        cells_per_process,              // array of elements to each process
        starts_per_process,             // array of start element per process
        MPI_INT,                        // send data type
        my_padded_submatrix[0],         // receive buffer 
        my_padded_rows * matrix_size,   // elements in receive buffer
        MPI_INT,                        // receive data type
        MASTER,                         // rank of source process
        MPI_COMM_WORLD                  // communicator
    );
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error during Scatterv operation.\n");
        cleanup(&matrix, matrix_size,
                &my_padded_submatrix, my_padded_rows, NULL, EMPTY,
                &cells_per_process, &starts_per_process);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    if (my_rank == MASTER) {
        free(cells_per_process);
        cells_per_process = NULL;
        free(starts_per_process);
        starts_per_process = NULL;
    }


    // All processes allocate space for their processed submatrix
    my_processed_submatrix = allocate_matrix(rows_per_node, matrix_size);
    if (!my_processed_submatrix) {
        fprintf(stderr, "Failed to allocate memory for processed matrix");
        cleanup(&matrix, matrix_size,
                &my_padded_submatrix, my_padded_rows,
                &my_processed_submatrix, rows_per_node, NULL, NULL);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }


    // All processes apply the convolution filter on their portion
    for (int row = my_top_padding; row < rows_per_node - my_bottom_padding; row++) {
        for (int col = 0; col < matrix_size; col++) {
            int sum = apply_convolution(row, col, my_padded_submatrix,
                                        my_padded_rows, matrix_size, depth);
            if (sum < 0) {
                fprintf(stderr, "Error in apply_convolution at row %d and col %d.\n", row, col);
                cleanup(&matrix, matrix_size,
                        &my_padded_submatrix, my_padded_rows,
                        &my_processed_submatrix, rows_per_node, NULL, NULL);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
            my_processed_submatrix[row][col] = sum;
        }
    }

    free_matrix(my_padded_submatrix, my_padded_rows);


    // Gather the processed sub-matrices at master process
    mpi_err = MPI_Gather(
        my_processed_submatrix[0],      // send buffer
        rows_per_node * matrix_size,    // number of elements in send buffer
        MPI_INT,                        // send data type
        matrix,                         // receive buffer
        rows_per_node * matrix_size,    // number of elements in receive buffer
        MPI_INT,                        // receive data type
        MASTER,                         // rank of destination process
        MPI_COMM_WORLD                  // communicator
    );
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error during Gather operation.\n");
        cleanup(&matrix, matrix_size, NULL, EMPTY,
                &my_processed_submatrix, rows_per_node, NULL, NULL);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }


    free_matrix(my_processed_submatrix, rows_per_node);


    // Master process writes the output matrix to a file
    if (my_rank == MASTER) {
        int result = write_matrix_to_file(output_filename, matrix, matrix_size);
        if (result == -1) {
            fprintf(stderr, "Failed to write matrix to output file %s.\n", output_filename);
        } else if (result == -2) {
            fprintf(stderr, "Failed to close the file after writing matrix to output file %s.\n", output_filename);
        }
        free_matrix(matrix, matrix_size);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
