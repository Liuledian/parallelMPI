#include "stdio.h"
#include "stdlib.h"


void write_binary_matrix(char* filename, int* storage, int m, int n) {
    FILE* fp = fopen(filename, "wb");
    int n_bytes;
    size_t int_size = sizeof(int);
    fwrite(&m, int_size, 1, fp);
    fwrite(&n, int_size, 1, fp);
    fwrite(storage, int_size, m * n, fp);
    fclose(fp);
}

int main(int argc, char* argv[]) {
    int* storage;
    int** matrix;
    int m;
    int n;
    char* filename;
    m = atoi(argv[1]);
    n = atoi(argv[2]);
    filename = argv[3];
    storage = (int *) malloc(m * n * sizeof(int));
    matrix = (int **) malloc (m * sizeof(int*));

    int* ptr = storage;
    for (int i = 0; i < m; i++) {
       matrix[i] = ptr;
       ptr += n;
    }
    matrix[0][0] = 1;
    matrix[3][0] = 1;
    matrix[3][1] = 1;
    matrix[3][2] = 1;
    matrix[0][3] = 1;
    matrix[1][3] = 1;
    for (int i = 0; i < m; i++) {
        matrix[i][4] = 1;
    }

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    write_binary_matrix(filename, storage, m, n);

    return 0;
}

