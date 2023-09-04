#include "matrix_utils.h"

/**
 * @brief Convert 2D matrix indices to 1D index for arrays.
 * 
 * @param row The row number in the matrix.
 * @param col The column number in the matrix.
 * @param matrix_cols Total columns in the matrix.
 * @return int The corresponding index in a 1D array representation.
 */
int get_i(int row, int col, int matrix_cols)
{
    return row * matrix_cols + col;
}

/**
 * @brief Safely free allocated memory and set pointer to NULL.
 * 
 * @param int_array Pointer to the int array to be freed.
 */
void safe_free(int **int_array)
{
    if (*int_array) {
        free(*int_array);
        *int_array = NULL;
    }
}

/**
 * @brief Allocate space for a matrix.
 * 
 * @param rows Number of rows.
 * @param cols Number of columns.
 * @return int* Pointer to the allocated matrix. NULL if allocation failed.
 */
int* allocate_matrix(int rows, int cols)
{
    int *int_array = (int*) calloc(rows * cols, sizeof(int));
    if (!int_array) {
        LOG("Failed to allocate space");
        return NULL;
    }
    return int_array;
}

/**
 * @brief Get the amount of padding required for a process.
 * 
 * @param proc_rank Rank of the process.
 * @param matrix_size Size of the matrix (number of rows or columns).
 * @param depth Depth of the convolution.
 * @param rows_per_node Rows assigned to each process.
 * @param direction Direction to check padding (UP = -1, DOWN = 1).
 * @return Number of rows of padding required.
 */
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

/**
 * @brief Calculate the number of rows a process needs to handle 
 *        after considering padding.
 * 
 * @param proc_rank Rank of the process.
 * @param matrix_size Size of the matrix (number of rows or columns).
 * @param depth Depth of the convolution.
 * @param rows_per_node Rows assigned to each process.
 * @return Total number of rows for the process after adding padding.
 */
int get_padded_rows(int proc_rank, int matrix_size, int depth,
                    int rows_per_node)
{
    int top_padding = get_padding(proc_rank, matrix_size, depth, rows_per_node, -1);
    int bottom_padding = get_padding(proc_rank, matrix_size, depth, rows_per_node, 1);
    return top_padding + rows_per_node + bottom_padding;
}

/**
 * @brief Get the size of a matrix from a file.
 *
 * @param filename Name of the file to read from.
 * @return Size of the matrix if successful,
 *         -1 if there's an error reading the file.
 */
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

/**
 * @brief Read a matrix from a file.
 * 
 * @param filename Name of the file to read from.
 * @param matrix_size Pointer to an int where the matrix's size will be stored.
 * @return Pointer to the read matrix. NULL if reading fails.
 */
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

/**
 * @brief Write a matrix to a file.
 * 
 * @param filename Name of the file to write to.
 * @param matrix Pointer to the matrix to write.
 * @param matrix_size Size of the matrix to write.
 * @return 0 if the write operation succeeds,
 *        -1 if failed to write,
 *        -2 if failed to close the file.
 */
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

/**
 * @brief Convert a matrix to a string for display.
 * 
 * @param matrix Pointer to the matrix.
 * @param rows Number of rows in the matrix.
 * @param cols Number of columns in the matrix.
 * @return char* Pointer to the string representation of the matrix.
 */
char* matrix_to_string(int* matrix, int rows, int cols) {
    // Calculate the needed buffer size. 
    // Assuming each number can be 10 chars long + 1 space + 1 for '\n'
    int buffer_size = rows * (cols * 5 + 1) + 1;
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
