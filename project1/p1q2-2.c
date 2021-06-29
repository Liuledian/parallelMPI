#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define P 4


int main() {
    MPI_Init(NULL, NULL);
    double start, finish;

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int result_rows = N/P;

    int m[N][N];
    int result[N/P][N/P];
    int i,j,k1,k2;
    int average_row_num;
    int extra_row_num;

    int offset = 0;
    int row_num;

    if (world_rank == 0) {
        srand(time(0));

        //generate random matrix
        for (i = 0; i < N; ++i) {
            for (j = 0; j < N; ++j) {
                m[i][j] = rand()%100;
            }
        }

        start = MPI_Wtime();

        average_row_num = result_rows/(world_size - 1);
        extra_row_num = result_rows % (world_size - 1);

        //send matrix n and split matrix m to all process
        for (i = 1; i < world_size; ++i) {
            if (i <= extra_row_num) {
                row_num = (average_row_num + 1)*P;
            } else {
                row_num = average_row_num*P;
            }
            MPI_Send(&offset, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&row_num, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            MPI_Send(&m[offset][0], N*row_num, MPI_INT, i, 2, MPI_COMM_WORLD);
            offset = offset + row_num;
        }

        //receive result from other process
        for (i = 1; i < world_size; ++i) {
            MPI_Recv(&offset, 1, MPI_INT, i, 101, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&row_num, 1, MPI_INT, i, 102, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&result[offset][0], row_num*result_rows, MPI_INT, i, 103, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        finish = MPI_Wtime();
        printf("Processes = % d, M: %d x %d, Kernel: %d x %d\n", world_size, N, N, P, P);
        printf("Matrix pooling elapsed time: %f seconds\n", finish - start);

    } else {

        MPI_Recv(&offset, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&row_num, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&m, row_num*N, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (i = 0; i < row_num/P; i = i + 1) {
            for (j = 0; j < N/P; j = j + 1) {
                result[i][j] = m[i*P][j*P];

                for (k1 = 0; k1 < P; ++k1) {
                    for (k2 = 0; k2 < P; ++k2) {
                        if (result[i][j] < m[i*P+k1][j*P+k2]) {
                            result[i][j] = m[i*P+k1][j*P+k2];
                        }
                    }
                }

            }
        }

        //send to process 0
        offset = offset/P;
        row_num = row_num/P;
        MPI_Send(&offset, 1, MPI_INT, 0, 101, MPI_COMM_WORLD);
        MPI_Send(&row_num, 1, MPI_INT, 0, 102, MPI_COMM_WORLD);
        MPI_Send(&result, row_num * result_rows, MPI_INT, 0, 103, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}