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
            my_start_row,
            my_end_row,
            *cells_per_process       = NULL,    // Number of rows per node
            *starts_per_process      = NULL,    // Starting element per node
            **matrix                 = NULL,    // Main matrix
            **my_padded_submatrix    = NULL,    // Padded working sub-matrix
            **my_processed_submatrix = NULL;    // Processed output sub-matrix

    char    *input_filename,    // Filename of input matrix
            *output_filename;   // Filename of output matrix


    // Setup MPI (initialise, get rank and number of processes)
    mpi_setup(&argc, &argv, &my_rank, &nproc);
    LOG("Initialised P%d of %d\n", my_rank, nproc);


    // Parse args
    if (argc != 4 || (depth = atoi(argv[3])) < 0) {
        fprintf(stderr, "Usage: %s [input] [output] [depth]\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }


    // Master process retrieves matrix from file
    if (my_rank == MASTER) {
        input_filename = argv[1];
        output_filename = argv[2];
        LOG("ARGS: %s, %s, %d\n",
            input_filename, output_filename, depth);
        matrix = read_matrix_from_file(input_filename, &matrix_size);
        if (matrix_size <= 0 || !matrix) {
            fprintf(stderr, "Failed to read matrix from file: %s\n", input_filename);
            free_matrix(&matrix, matrix_size);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        LOG("Master process read a %d by %d matrix from file\n", matrix_size, matrix_size);

        // If zero depth, no work to do. Write input matrix to output file 
        if (depth == 0) {
            LOG("Zero depth set. No work to do\n");
            int result = write_matrix_to_file(output_filename, matrix, matrix_size);
            if (result == -1) {
                fprintf(stderr, "Failed to write matrix to output file %s.\n", output_filename);
            } else if (result == -2) {
                fprintf(stderr, "Failed to close the file after writing matrix to output file %s.\n", output_filename);
            }
            LOG("Master process wrote matrix to file\n");
            free_matrix(&matrix, matrix_size);
            MPI_Finalize();
            return EXIT_SUCCESS;
        }
    }


    // Broadcast the master matrix's size from master to all processes
    mpi_err = MPI_Bcast(&matrix_size, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "P%d experienced an error during broadcast of matrix size.\n",
                my_rank);
        free_matrix(&matrix, matrix_size);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    // All processes compute the size of their submatrix
    rows_per_node = matrix_size / nproc;
    my_top_padding = get_padding(my_rank, matrix_size, depth, rows_per_node, UP);
    my_bottom_padding = get_padding(my_rank, matrix_size, depth, rows_per_node, DOWN);
    my_padded_rows = my_top_padding + rows_per_node + my_bottom_padding;
    my_start_row = (my_rank * rows_per_node) - my_top_padding;
    my_end_row = my_start_row + my_padded_rows - 1;

    if (my_top_padding < 0 ||
        my_bottom_padding < 0 ||
        my_padded_rows <= 0 ||
        my_start_row < 0 ||
        my_end_row >= matrix_size) {
        fprintf(stderr, "P%d experienced an error calculating padding\n",
                my_rank);
        if (my_rank == MASTER)
            free_matrix(&matrix, matrix_size);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }
    LOG("P%d will handle %d rows starting at row %d and ending at row %d. "
        "(%d upper padding, %d working rows, %d lower padding)\n",
        my_rank, my_padded_rows, my_start_row, my_end_row,
        my_top_padding, rows_per_node, my_bottom_padding);


    // All processes allocate space for their padded submatrix
    my_padded_submatrix = allocate_matrix(my_padded_rows, matrix_size);
    if (!my_padded_submatrix) {
        fprintf(stderr, "P%d experienced an error while allocating memory for their padded submatrix\n",
                my_rank);
        if (my_rank == MASTER) {
            free_matrix(&matrix, matrix_size);
        }
        free_matrix(&my_padded_submatrix, my_padded_rows);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    LOG("P%d has allocated their %dx%d padded submatix=%p "
        "(0,0=%d. %d,%d=%d)\n",
        my_rank, my_padded_rows, matrix_size, (void*)my_padded_submatrix,
        my_padded_submatrix[0][0],
        my_padded_rows-1, matrix_size-1, 
        my_padded_submatrix[my_padded_rows-1][matrix_size-1]);


    // Allocate space for cells_per_process and starts_per_process
    cells_per_process  = (int *)malloc(nproc * sizeof(int));
    starts_per_process = (int *)malloc(nproc * sizeof(int));
    if (!cells_per_process || !starts_per_process) {
        fprintf(stderr, "P%d experienced an error while allocating memory for cells_per_process or starts_per_process\n",
                my_rank);
        cleanup(&matrix, matrix_size, 
                &my_padded_submatrix, my_padded_rows, NULL, EMPTY,
                &cells_per_process, &starts_per_process);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    LOG("P%d has allocated cells_per_process=%p and starts_per_process=%p\n",
        my_rank, (void*)cells_per_process, (void*)starts_per_process);


    // Master process determines the number of elements and starting element 
    // to send to each process
    if (my_rank == MASTER) {
        for(int proc = 0; proc < nproc; proc++) {
            int top_padding = get_padding(proc, matrix_size, depth, rows_per_node, UP);
            int bottom_padding = get_padding(proc, matrix_size, depth, rows_per_node, DOWN);
            int padded_portion = top_padding + rows_per_node + bottom_padding;

            cells_per_process[proc] = padded_portion * matrix_size;
            starts_per_process[proc] = ((proc * rows_per_node) - top_padding) * matrix_size;
            LOG("P%d will be given %d cells (%d rows) at starting position %d (in row %d)\n",
                proc, cells_per_process[proc], padded_portion,
                starts_per_process[proc], starts_per_process[proc] / matrix_size);
        }
        LOG("Master process has computed cells_per_process and starts_per_process\n");
    }

    // Broadcasting cells_per_process and starts_per_process to all processes
    mpi_err = MPI_Bcast(cells_per_process, nproc, MPI_INT, MASTER, MPI_COMM_WORLD);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "P%d experienced an error during broadcast of cells_per_process\n",
                my_rank);
        free_matrix(&matrix, matrix_size);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }
    mpi_err = MPI_Bcast(starts_per_process, nproc, MPI_INT, MASTER, MPI_COMM_WORLD);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "P%d experienced an error during broadcast of starts_per_process\n",
                my_rank);
        free_matrix(&matrix, matrix_size);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }
    LOG("P%d has received cells_per_process[me]=%d and starts_per_process[me]=%d\n",
        my_rank, cells_per_process[my_rank], starts_per_process[my_rank]);


    LOG("Before Scatterv, P%d's data is:\n"
        " - sndbuf  = %p\n"
        " - sndcnts = %p (me=%d)\n"
        " - displs  = %p (me=%d)\n"
        " - recvbuf = %p\n"
        " - recvcnt = %d\n"
        " - root    = %d\n",
        my_rank,                       // Process rank
        (void*)matrix,                 // send buffer address
        (void*)cells_per_process,      // address of the array of elements to each process
        cells_per_process[my_rank],
        (void*)starts_per_process,     // address of the array of start element per process
        starts_per_process[my_rank],
        (void*)my_padded_submatrix,    // receive buffer address
        my_padded_rows * matrix_size,  // elements in receive buffer
        MASTER                         // rank of source process
    );

    mpi_err = MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "P%d experienced an error during mpi barrier.\n",
                my_rank);
        free_matrix(&matrix, matrix_size);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }


    // TODO: Check this is correct
    // Distribute sub-matrices to processes
    mpi_err = MPI_Scatterv(
        matrix,                      // send buffer
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
        fprintf(stderr, "P%d experienced an error during Scatterv operation.\n",
                my_rank);
        cleanup(&matrix, matrix_size,
                &my_padded_submatrix, my_padded_rows, NULL, EMPTY,
                &cells_per_process, &starts_per_process);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    LOG("P%d expected data for rows %d to %d.\n",
        my_rank, my_start_row, my_end_row);

    LOG("P%d received data: Start(0,0) = %d, End(%d,%d) = %d\n", 
        my_rank, 
        my_padded_submatrix[0][0],
        my_padded_rows-1, matrix_size-1,
        my_padded_submatrix[my_padded_rows-1][matrix_size-1]);


    free(cells_per_process);
    cells_per_process = NULL;
    free(starts_per_process);
    starts_per_process = NULL;
    LOG("P%d has freed their cells_per_process and starts_per_process\n", my_rank);


    // All processes allocate space for their processed submatrix
    my_processed_submatrix = allocate_matrix(rows_per_node, matrix_size);
    if (!my_processed_submatrix) {
        fprintf(stderr, "P%d experienced an error allocating memory for processed matrix",
                my_rank);
        cleanup(&matrix, matrix_size,
                &my_padded_submatrix, my_padded_rows,
                &my_processed_submatrix, rows_per_node, NULL, NULL);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    LOG("P%d has allocated their %d-row by %d-col processed submatix=%p\n",
        my_rank, rows_per_node, matrix_size, (void*)my_processed_submatrix);


    mpi_err = MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "P%d experienced an error during mpi barrier.\n",
                my_rank);
        free_matrix(&matrix, matrix_size);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }

    // All processes apply the convolution filter on their portion
    LOG("P%d will apply convolution on %d rows (%d upper padding, %d working rows, %d lower padding)\n",
        my_rank, my_padded_rows, my_top_padding, rows_per_node, my_bottom_padding);
    for (int row = my_top_padding; row < (rows_per_node + my_top_padding); row++) {
        for (int col = 0; col < matrix_size; col++) {
            int sum = apply_convolution(row, col, my_padded_submatrix,
                                        my_padded_rows, matrix_size, depth);
            if (sum < 0) {
                fprintf(stderr, "P%d experienced an error in apply_convolution at local row %d and col %d of their padded submatrix\n",
                        my_rank, row, col);
                cleanup(&matrix, matrix_size,
                        &my_padded_submatrix, my_padded_rows,
                        &my_processed_submatrix, rows_per_node, NULL, NULL);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
            my_processed_submatrix[row][col] = sum;
        }
    }
    LOG("P%d has finished processing their submatix\n", my_rank);

    free_matrix(&my_padded_submatrix, my_padded_rows);
    LOG("P%d has freed their padded submatix. Waiting for gather call\n", my_rank);

    
    LOG("Before Gather, P%d's data is:\n"
        " - sndbuf  = %p\n"
        " - sndcnt  = %d\n"
        " - recvbuf = %p\n"
        " - recvcnt = %d\n"
        " - root    = %d\n",
        my_rank,                        // Process rank
        (void*)my_processed_submatrix,  // send buffer address
        rows_per_node * matrix_size,    // elements in send buffer
        (void*)matrix,                  // receive buffer address
        matrix_size * matrix_size,      // elements in receive buffer
        MASTER                          // rank of source process
    );

    mpi_err = MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "P%d experienced an error during mpi barrier.\n",
                my_rank);
        free_matrix(&matrix, matrix_size);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }


    // Gather the processed sub-matrices at master process
    mpi_err = MPI_Gather(
        my_processed_submatrix[0],         // send buffer
        rows_per_node * matrix_size,    // number of elements in send buffer
        MPI_INT,                        // send data type
        matrix[0],                         // receive buffer
        matrix_size * matrix_size,    // number of elements in receive buffer
        MPI_INT,                        // receive data type
        MASTER,                         // rank of destination process
        MPI_COMM_WORLD                  // communicator
    );
    if (mpi_err != MPI_SUCCESS) {
        fprintf(stderr, "P%d experienced an error during Gather operation.\n", my_rank);
        cleanup(&matrix, matrix_size, NULL, EMPTY,
                &my_processed_submatrix, rows_per_node, NULL, NULL);
        MPI_Abort(MPI_COMM_WORLD, mpi_err);
    }
    LOG("P%d still here after data has been gathered\n", my_rank);


    free_matrix(&my_processed_submatrix, rows_per_node);
    LOG("P%d has freed their processed submatix\n", my_rank);


    // Master process writes the output matrix to a file
    if (my_rank == MASTER) {
        int result = write_matrix_to_file(output_filename, matrix, matrix_size);
        if (result == -1) {
            fprintf(stderr, "Failed to write matrix to output file %s.\n", output_filename);
        } else if (result == -2) {
            fprintf(stderr, "Failed to close the file after writing matrix to output file %s.\n", output_filename);
        }
        LOG("Master process wrote matrix to file\n");
        free_matrix(&matrix, matrix_size);
    }

    
    LOG("P%d has finished\n", my_rank);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
