// P 6.13
#include "mpi.h"
#include "MyMPI.h"
#include "stdio.h"
#include "stdlib.h"

void update_matrix(int** ori, int sub_m, int n, int* prev_row, int* next_row, char prev_flag, char next_flag) {
    // Copy sub
    int* storage;
    int** sub;
    storage = (int *) malloc(sub_m * n * sizeof(int));
    sub = (int **) malloc (sub_m * sizeof(int*));
    int* ptr = storage;
    for (int i = 0; i < sub_m; i++) {
        sub[i] = ptr;
        ptr += n;
    }
    for (int i = 0; i < sub_m; i++) {
        for (int j = 0; j < n; j++) {
            sub[i][j] = ori[i][j];
        }
    }

    // Update ori
    for (int i = 0; i < sub_m; i++) {
        for (int j = 0; j < n; j++) {
            // Count neighbors
            int l = j > 0 && sub[i][j - 1];
            int r = j < n - 1 && sub[i][j + 1];
            int u = i > 0 && sub[i - 1][j] || i == 0 && prev_flag && prev_row[j];
            int d = i < sub_m - 1 && sub[i + 1][j] || i == sub_m - 1 && next_flag && next_row[j];
            int lu = j > 0 && (i > 0 && sub[i - 1][j - 1] || i == 0 && prev_flag && prev_row[j - 1]);
            int ld = j > 0 && (i < sub_m - 1  && sub[i + 1][j - 1] || i == sub_m - 1 && next_flag && next_row[j - 1]);
            int ru = j < n - 1 && (i > 0 && sub[i - 1][j + 1] || i == 0 && prev_flag && prev_row[j + 1]);
            int rd = j < n - 1 && (i < sub_m - 1 && sub[i + 1][j + 1] || i == sub_m - 1 && next_flag && next_row[j + 1]);
            int neighbors = l + r + u + d + lu + ld + ru + rd;
            int id;
//            MPI_Comm_rank(MPI_COMM_WORLD, &id);
//            printf("%d (%d,%d): %d\n%d,%d,%d,%d,%d,%d,%d,%d\n",id,i,j,neighbors, l, r, u, d, lu, ld, ru, rd);
            if (!ori[i][j] && neighbors == 3) {
                ori[i][j] = 1;
            }
            if (ori[i][j] && (neighbors < 2 || neighbors > 3)) {
                ori[i][j] = 0;
            }
        }
    }
    free(storage);
    free(sub);
}

int main(int argc, char* argv[]) {
    int id;
    int p;
    int n_iter;
    int period;
    double elapsed_time;
    int** sub;
    int* storage;
    int m, n;
    int local_rows;
    int* prev_row;
    int* next_row;
    char prev_flag;
    char next_flag;
    MPI_Status status;

    n_iter = atoi(argv[1]);
    period = atoi(argv[2]);

    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = - MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    read_row_striped_matrix("input_matrix.b", &sub, &storage, MPI_INT, &m, &n, MPI_COMM_WORLD);
    local_rows = BLOCK_SIZE(id, p, m);

    // Initialize prev_row and next_row to accept messages
    prev_row = (int*) malloc(sizeof(int) * n);
    next_row = (int*) malloc(sizeof(int) * n);

    // Show initial matrix
    print_row_striped_matrix(sub, MPI_INT, m, n, MPI_COMM_WORLD);

    for (int i = 0; i < n_iter; i++) {
        // Send and receive required rows
        prev_flag = 0;
        next_flag = 0;
        if (p > 1) {
            if (id == 0) {
                MPI_Send(sub[local_rows - 1], n, MPI_INT, id + 1, DATA_MSG, MPI_COMM_WORLD);
                MPI_Recv(next_row, n, MPI_INT, id + 1, DATA_MSG, MPI_COMM_WORLD, &status);
                next_flag = 1;
            } else if (id == p - 1) {
                MPI_Send(sub[0], n, MPI_INT, id - 1, DATA_MSG, MPI_COMM_WORLD);
                MPI_Recv(prev_row, n, MPI_INT, id - 1, DATA_MSG, MPI_COMM_WORLD, &status);
                prev_flag = 1;
            } else {
                MPI_Send(sub[local_rows - 1], n, MPI_INT, id + 1, DATA_MSG, MPI_COMM_WORLD);
                MPI_Send(sub[0], n, MPI_INT, id - 1, DATA_MSG, MPI_COMM_WORLD);
                MPI_Recv(next_row, n, MPI_INT, id + 1, DATA_MSG, MPI_COMM_WORLD, &status);
                MPI_Recv(prev_row, n, MPI_INT, id - 1, DATA_MSG, MPI_COMM_WORLD, &status);
                prev_flag = 1;
                next_flag = 1;
            }
        }

        // Update the cells in the sub matrix
        update_matrix(sub, local_rows, n, prev_row, next_row, prev_flag, next_flag);
        if ((i + 1) % period == 0) {
            print_row_striped_matrix(sub, MPI_INT, m, n, MPI_COMM_WORLD);
        }
    }

    // Print time
    elapsed_time += MPI_Wtime();
    MPI_Barrier(MPI_COMM_WORLD);
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    printf("Process %2d is done, %fs from %s!\n", id, elapsed_time, processor_name);
    fflush(stdout);
    free(storage);
    free(sub);
    MPI_Finalize();

    return 0;
}



