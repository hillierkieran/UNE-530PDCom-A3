#include "convolution.h"
#include "headers.h"

int main(int argc, char** argv)
{
    int me, nproc, depth, matrix_size = -1;
    int** matrix = NULL;
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
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    // Compute portion of the matrix each process will handle
    int local_rows = matrix_size / nproc;
    int local_neighbourhood = local_rows + (depth * 2);
    int start_row = (me * local_rows) - depth;
    int end_row = start_row + local_neighbourhood;

    // All processes allocate space for their local matrices
    int** local_matrix = allocate_matrix(local_neighbourhood, matrix_size);
    int** processed_local_matrix = allocate_matrix(local_rows, matrix_size);
    if (!local_matrix || !processed_local_matrix) {
        fprintf(stderr, "Failed to allocate memory for local matrices");
        cleanup(matrix, matrix_size,
                local_matrix, local_neighbourhood,
                processed_local_matrix, local_rows)
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Master process distributes portions among processes
    if (me == MASTER) {
        for (int proc = 0; proc < nproc; proc++) {
            int start = proc * local_rows - depth;
            int end = start + local_neighbourhood;
            for (int i = start; i < end; i++) {
                mpi_err = MPI_Send(
                            matrix[i],      // send buffer
                            matrix_size,    // elements in send buffer
                            MPI_BYTE,       // send data type
                            proc,           // rank of destination process
                            0,              // message tag
                            MPI_COMM_WORLD  // communicator
                        );
                if (mpi_err != MPI_SUCCESS) {
                    fprintf(stderr, "Error sending matrix row %d to process %d.\n", i, proc);
                    MPI_Abort(MPI_COMM_WORLD, mpi_err);
                }
            }
        }
    }

    // All processes receive their portion of the matrix
    for (int i = 0; i < local_neighbourhood; i++) {
        mpi_err = MPI_Recv(
                    local_matrix[i],    // receive buffer
                    matrix_size,        // elements in receive buffer
                    MPI_BYTE,           // receive data type
                    MASTER,             // rank of source process
                    0,                  // message tag
                    MPI_COMM_WORLD,     // communicator
                    MPI_STATUS_IGNORE   // status object
                );
        if (mpi_err != MPI_SUCCESS) {
            fprintf(stderr, "Error receiving matrix row %d from master process.\n", i);
            MPI_Abort(MPI_COMM_WORLD, mpi_err);
        }
    }

    // All processes apply the convolution filter on their portion
    int weightedSum; 
    for (int row = depth; row < local_rows + depth; row++) {
        for (int col = 0; col < matrix_size; col++) {
            weightedSum = apply_convolution(local_matrix, local_neighbourhood,
                                            row, col, depth);
            if (weightedSum < 0) {
                fprintf(stderr, "Error in apply_convolution at row %d and col %d.\n", row, col);
                cleanup(matrix, matrix_size,
                        local_matrix, local_neighbourhood,
                        processed_local_matrix, local_rows);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
            processed_local_matrix[row][col] = weightedSum;
        }
    }

    // Gather the processed sub-matrix portions at master process
    mpi_err = MPI_Gather(
                processed_local_matrix,     // send buffer
                local_rows * matrix_size,   // elements in send buffer
                MPI_BYTE,                   // send data type
                matrix,                     // receive buffer
                local_rows * matrix_size,   // elements in receive buffer
                MPI_BYTE,                   // receive data type
                MASTER,                     // rank of destination process
                MPI_COMM_WORLD              // communicator
            );
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "Error gathering data from processes.\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    // Master process writes the output matrix to a file
    if (me == MASTER) {
        if (write_matrix_to_file(output_file, matrix, matrix_size) != 0) {
            fprintf(stderr, "Failed to write matrix to output file %s.\n", output_file);
        }
    }

    // All processes deallocate memory for all their matrices
    cleanup(matrix, matrix_size,
            local_matrix, local_neighbourhood,
            processed_local_matrix, local_rows);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
