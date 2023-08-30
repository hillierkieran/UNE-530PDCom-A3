#include "convolution.h"
#include <stdlib.h>

double apply_convolution(int** matrix, int size, int x, int y, int depth) {
    int i, j;
    double sum = 0.0;
    int count = 0;
    for (i = -depth; i <= depth; i++) {
        for (j = -depth; j <= depth; j++) {
            int nx = x + i;
            int ny = y + j;
            if (nx >= 0 && nx < size && ny >= 0 && ny < size) {
                int n_depth = abs(i) + abs(j) + 1;
                sum += matrix[nx][ny] * (1.0 / n_depth);
                count++;
            }
        }
    }
    return sum / count;
}
