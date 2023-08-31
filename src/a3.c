#include "convolution.h"
#include "headers.h"

int main(int argc, char** argv)
{
    int me, nproc, depth, matrix_size = -1;
    int* cells_per_process = NULL;
    int* starts_per_process = NULL;
    int** matrix = NULL;
    int** input_submatrix = NULL;
    int** output_submatrix = NULL;
    char* input_file;
    char* output_file;

    int mpi_err;

    mpi_err = MPI_Init(&argc, &argv);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error initializing MPI.\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }
    // Set MPI to return errors rather than directly terminating the application
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
    mpi_err = MPI_Comm_rank(MPI_COMM_WORLD, &me);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error getting process rank.\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }
    mpi_err = MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error getting number of processes.\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    // Parse args
    if (argc != 4 || (depth = atoi(argv[3])) <= 0) {
        fprintf(stderr, "Usage: %s [input_file] [output_file] [depth]\n",
                argv[0]);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Master process initialises matrix from file
    if (me == MASTER) {
        input_file = argv[1];
        output_file = argv[2];

        matrix_size = get_matrix_size(input_file);
        if (matrix_size <= 0) {
            fprintf(stderr, "Failed to get matrix size from file: %s\n", input_file);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        matrix = read_matrix_from_file(input_file, matrix_size);
        if (!matrix) {
            fprintf(stderr, "Failed to read matrix from file: %s\n", input_file);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    // Broadcast the matrix size from master to all processes
    mpi_err = MPI_Bcast(&matrix_size, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error broadcasting matrix size.\n");
        if (me == MASTER) {
            free_matrix(matrix, matrix_size);
        }
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    // Compute portion of the matrix each process will handle
    int rows_per_process = matrix_size / nproc;
    int neighbourhood_rows = rows_per_process + (depth * 2);
    int start_row, end_row;
    compute_local_rows(me, rows_per_process, depth, matrix_size, &start_row, &end_row);

    // All processes allocate space for their local matrices
    input_submatrix  = allocate_matrix(neighbourhood_rows, matrix_size);
    output_submatrix = allocate_matrix(rows_per_process, matrix_size);
    if (!input_submatrix || !output_submatrix) {
        fprintf(stderr, "Failed to allocate memory for local matrices");
        cleanup(matrix, matrix_size,
                input_submatrix, neighbourhood_rows,
                output_submatrix, rows_per_process,
                cells_per_process, starts_per_process);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }


    // Initialise cells_per_process and starts_per_process
    if (me == MASTER) {
        // Allocate space for cells_per_process and starts_per_process
        cells_per_process  = (int *)malloc(nproc * sizeof(int));
        starts_per_process = (int *)malloc(nproc * sizeof(int));

        int row_offset = 0;
        for(int proc = 0; proc < nproc; proc++) {
            int start, end;
            compute_local_rows( proc, rows_per_process, depth,
                                matrix_size, &start, &end);
            cells_per_process[proc] = (end - start) * matrix_size;
            starts_per_process[proc] = row_offset;
            row_offset += rows_per_process * matrix_size;
        }
    }

    // Distribute sub-matrices to processes
    mpi_err = MPI_Scatterv(
        matrix[0],                          // send buffer
        cells_per_process,                  // array of elements to each process
        starts_per_process,                 // array of start row per process
        MPI_INT,                            // send data type
        input_submatrix[0],                 // receive buffer 
        neighbourhood_rows * matrix_size,   // elements in receive buffer
        MPI_INT,                            // receive data type
        MASTER,                             // rank of source process
        MPI_COMM_WORLD                      // communicator
    );
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error during Scatterv operation.\n");
        cleanup(matrix, matrix_size,
                input_submatrix, neighbourhood_rows,
                output_submatrix, rows_per_process,
                cells_per_process, starts_per_process);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    // All processes apply the convolution filter on their portion
    for (int row = depth; row < rows_per_process + depth; row++) {
        for (int col = 0; col < matrix_size; col++) {
            int sum = apply_convolution(input_submatrix, neighbourhood_rows,
                                        row, col, depth);
            if (sum < 0) {
                fprintf(stderr, "Error in apply_convolution at row %d and col %d.\n", row, col);
                cleanup(matrix, matrix_size,
                        input_submatrix, neighbourhood_rows,
                        output_submatrix, rows_per_process,
                        cells_per_process, starts_per_process);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
            output_submatrix[row][col] = sum;
        }
    }

    // Reset cells_per_process array
    if (me == MASTER) {
        int row_offset = 0;
        for (int proc = 0; proc < nproc; proc++) {
            cells_per_process[proc] = rows_per_process * matrix_size;
            row_offset += rows_per_process * matrix_size;
        }
    }

    // Gather the processed sub-matrices at master process
    mpi_err = MPI_Gatherv(
        output_submatrix[0],            // send buffer
        rows_per_process * matrix_size, // number of elements in send buffer
        MPI_INT,                        // send data type
        matrix[0],                      // receive buffer
        cells_per_process,              // array of elements from each process
        starts_per_process,             // array of start rows per process
        MPI_INT,                        // receive data type
        MASTER,                         // rank of destination process
        MPI_COMM_WORLD                  // communicator
    );
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error during Gatherv operation.\n");
        cleanup(matrix, matrix_size,
                input_submatrix, neighbourhood_rows,
                output_submatrix, rows_per_process,
                cells_per_process, starts_per_process);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    // Master process writes the output matrix to a file
    if (me == MASTER) {
        if (write_matrix_to_file(output_file, matrix, matrix_size) != 0) {
            fprintf(stderr, "Failed to write matrix to output file %s.\n", output_file);
        }
    }

    // All processes deallocate their memory
    cleanup(matrix, matrix_size,
            input_submatrix, neighbourhood_rows,
            output_submatrix, rows_per_process,
            cells_per_process, starts_per_process);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
