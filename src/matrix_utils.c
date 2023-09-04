#include "matrix_utils.h"

/*
 * Utility functions for handling matrices stored in files.
 * - Assumes matrices are square.
 * - Provides functionality to read, write, allocate, and free matrices.
 */

int get_i(int row, int col, int matrix_cols)
{
    return row * matrix_cols + col;
}

// Deallocates the space used by the matrix.
void safe_free(int **int_array)
{
    if (*int_array) {
        free(*int_array);
        *int_array = NULL;
    }
}

// Allocates space for a matrix of size rows x cols.
int* allocate_matrix(int rows, int cols)
{
    int *int_array = (int*) calloc(rows * cols, sizeof(int));
    if (!int_array) {
        LOG("Failed to allocate space");
        return NULL;
    }
    return int_array;
}

int get_padding(int proc_rank, int matrix_rows, int depth,
                int rows_per_node, int direction)
{
    if (depth == 0)
        return 0;

    int start_row = proc_rank * rows_per_node;
    int end_row = start_row + rows_per_node - 1;

    int rows_from_edge;  // Distance from the respective edge (top or bottom)

    if (direction == UP) {
        rows_from_edge = start_row;
    } else if (direction == DOWN) {
        rows_from_edge = (matrix_rows - 1) - end_row;
    } else {
        return -1;  // Invalid direction
    }

    if (depth > rows_from_edge) {
        int overflow = (rows_from_edge - depth) * -1;
        return depth - overflow;
    }

    return depth;
}

// Computes sub-matrix portion for a process
int get_padded_rows(int proc_rank, int matrix_size, int depth,
                    int rows_per_node)
{
    int top_padding = get_padding(proc_rank, matrix_size, depth, rows_per_node, -1);
    int bottom_padding = get_padding(proc_rank, matrix_size, depth, rows_per_node, 1);
    return top_padding + rows_per_node + bottom_padding;
}

// Gets the size of the matrix from the file.
int get_matrix_size_from_file(const char *filename)
{
    struct stat st;

    // Get file size
    if (stat(filename, &st) != 0) {
        perror("Failed to get file stats");
        return -1;
    }

    // Calculate number of integers in the file
    int total_elements = st.st_size / sizeof(int);

    // Find the dimension of the matrix (assuming it's a square matrix)
    int matrix_size = (int) round(sqrt(total_elements));

    return matrix_size;
}

// Reads the matrix from a file.
int* read_matrix_from_file(const char *filename, int *size) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        LOG("Failed to open file.\n");
        return NULL;
    }

    *size = get_matrix_size_from_file(filename);
    if (*size <= 0) {
        LOG("Failed get matrix size.\n");
        close(fd);
        return NULL;
    }

    int* matrix = allocate_matrix(*size, *size);
    if (!matrix) {
        LOG("Failed to allocate space for main matrix.\n");
        close(fd);
        return NULL;
    }

    for (int row = 0; row < *size; row++) {
        for (int col = 0; col < *size; col++) {
            int cell_value;
            if (get_slot(fd, *size, row+1, col+1, &cell_value) == -1) {
                LOG("Failed to get slot %d,%d.\n", row, col);
                free(matrix);
                close(fd);
                return NULL;
            }
            matrix[row * *size + col] = cell_value;
        }
    }

    close(fd);
    return matrix;
}

// Writes the matrix to a file.
int write_matrix_to_file(const char *filename, int *matrix, int size) {
    int fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        LOG("Failed to open/create file.\n");
        return -1;
    }

    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            int cell_value = matrix[row * size + col];
            if (set_slot(fd, size, row+1, col+1, cell_value) == -1) {
                LOG("Failed to set slot %d,%d.\n", row, col);
                free(matrix);
                close(fd);
                return -1;
            }
        }
    }

    if (close(fd) == -1) {
        LOG("Failed to close file");
        return -2;
    }
    return 0;
}

char* matrix_to_string(int* matrix, int rows, int cols) {
    // Calculate the needed buffer size. 
    // Assuming each number can be 10 chars long + 1 space + 1 for '\n'
    int buffer_size = rows * (cols * 4 + 1) + 1;
    char* buffer = (char*)malloc(buffer_size);

    if (buffer == NULL) {
        return NULL;  // Memory allocation failed
    }

    int offset = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            offset += sprintf(buffer + offset, "%3d ", matrix[i * cols + j]);
        }
        buffer[offset - 1] = '\n';  // Replace the last space with a newline
    }
    buffer[offset] = '\0';  // Null-terminate the string

    return buffer;
}
