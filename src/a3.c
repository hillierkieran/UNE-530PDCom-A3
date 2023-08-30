#include "convolution.h"
#include "headers.h"

int main(int argc, char** argv) {
    int me, nproc;
    int** matrix;
    int matrix_size, depth;
    char* input_file;
    char* output_file;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    // Usage: [executable] [input file] [output file] [depth]
    if (argc != 4 || (depth = atoi(argv[3])) <= 0) {
        fprintf(stderr, "Usage: %s [input_file] [output_file] [depth]\n",
                argv[0]);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    input_file = argv[1];
    output_file = argv[2];
    matrix_size = get_matrix_size(input_file);

    // Initialize matrix for master process
    if (me == MASTER) {
        matrix = read_matrix_from_file(input_file, matrix_size);
    }

    // Split matrix rows among processes
    int rows_per_process = matrix_size / nproc;
    // Add given depth of neighbouring rows
    int rows_required = rows_per_process + 2 * depth;

    int start_row = me * rows_per_process - depth;
    int end_row = start_row + rows_required;

    // Allocate space for each process's sub-matrix
    int** local_matrix = allocate_matrix(rows_per_process, matrix_size);

    // Send sub-matrices to slave processes
    for (int i = start_row; i < end_row; i++) {
        MPI_Bcast(matrix[i], matrix_size, MPI_INT, MASTER, MPI_COMM_WORLD);
    }
/*
    // Send sub-matrices to slave processes
    MPI_Scatter(matrix,                         // send buffer
                rows_per_process * matrix_size, // elements in send buffer
                MPI_INT,                        // send data type
                local_matrix,                   // receive buffer
                rows_per_process * matrix_size, // elements in receive buffer
                MPI_INT,                        // receive data type
                MASTER,                         // rank of root process
                MPI_COMM_WORLD);                // communicator 
*/
    // Each process applies the convolution filter
    int** processed_local_matrix = allocate_matrix(rows_per_process, matrix_size);
    for (int i = 0; i < rows_per_process; i++) {
        for (int j = 0; j < matrix_size; j++) {
            processed_local_matrix[i][j] = 
                apply_convolution(local_matrix, rows_required, i + depth, j, depth);
        }
    }

    // Master process handles leftovers from uneven division of rows
    if (me == MASTER) {
        int leftover_start = rows_per_process * nproc;
        for (int i = leftover_start; i < matrix_size; i++) {
            for (int j = 0; j < matrix_size; j++) {
                matrix[i][j] = apply_convolution(matrix, matrix_size, i, j, depth);
            }
        }
    }

    // Gather the processed sub-matrices at master process
    if (me == MASTER) {
        matrix = allocate_matrix(matrix_size, matrix_size);
    }
    MPI_Gather( processed_local_matrix,         // send buffer
                rows_per_process * matrix_size, // elements in send buffer
                MPI_INT,                        // send data type
                matrix,                         // receive buffer
                rows_per_process * matrix_size, // elements in receive buffer
                MPI_INT,                        // receive data type
                MASTER,                         // rank of root process
                MPI_COMM_WORLD);                // communicator 

    // Master process writes the output matrix to a file
    if (me == MASTER) {
        write_matrix_to_file(output_file, matrix, matrix_size);
        free_matrix(matrix, matrix_size);  // Deallocate memory for the matrix
    }

    // Deallocate memory for the local matrices
    free_matrix(local_matrix, rows_per_process);
    free_matrix(processed_local_matrix, rows_per_process);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
