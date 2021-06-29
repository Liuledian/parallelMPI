#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define M 10

/*
matrix m: M*N
matrix n: N*N
matrix q: result matrix
*/

int main() {
    MPI_Init(NULL, NULL);
    double start, finish;

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int matrix_m[M][N], matrix_n[N][N], matrix_q[M][N];
    int i,j, k;
    int average_row_num;
    int extra_row_num;

    int offset = 0;
    int row_num;

    if (world_rank == 0) {
        srand(time(0));

        for (i = 0; i < M; ++i) {
            for (j = 0; j < N; ++j) {
                matrix_m[i][j] = rand()%10;
            }
        }

        for (i = 0; i < N; ++i) {
            for (j = 0; j < N; ++j) {
                matrix_n[i][j] = rand()%10;
            }
        }

        start = MPI_Wtime();
        average_row_num = M/(world_size - 1);
        extra_row_num = M % (world_size - 1);

        //send matrix n and split matrix m to all process
        for (i = 1; i < world_size; ++i) {
            if (i <= extra_row_num) {
                row_num = average_row_num + 1;
            } else {
                row_num = average_row_num;
            }
            MPI_Send(&offset, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            //printf("process 0 send offset to %d: %d\n", i, offset);
            MPI_Send(&row_num, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            //printf("process 0 send row num to %d: %d", i, row_num);
            MPI_Send(&matrix_m[offset][0], N*row_num, MPI_INT, i, 2, MPI_COMM_WORLD);
            //printf("process 0 send matrix m to %d\n", i);
            MPI_Send(&matrix_n, N*N, MPI_INT, i,3, MPI_COMM_WORLD);
            //printf("process 0 send matrix N*N to %d\n", i);
            offset = offset + row_num;
        }
        //receive result from other process
        for (i = 1; i < world_size; ++i) {
            MPI_Recv(&offset, 1, MPI_INT, i, 101, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&row_num, 1, MPI_INT, i, 102, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&matrix_q[offset][0], row_num*N, MPI_INT, i, 103, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        finish = MPI_Wtime();
        printf("Processes = % d, M: %d x %d, N: %d x % d, Q: %d x % d\n", world_size, M, N, N, N, M, N);
        printf("Matrix multiplication elapsed time %f seconds\n", finish - start);

    } else {

        MPI_Recv(&offset, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //printf("process %d receive offset: %d\n", world_rank, offset);
        MPI_Recv(&row_num, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //printf("process %d receive row num:%d", world_rank, row_num);
        MPI_Recv(&matrix_m, row_num*N, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //printf("process %d receive matrix m\n", world_rank);
        MPI_Recv(&matrix_n, N*N, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //printf("process %d receive N*N\n", world_rank);
        //calculate
        for (i = 0; i < row_num; ++i) {
            for (j = 0; j < N; ++j) {
                matrix_q[i][j] = 0;
                for (k = 0; k < N; ++k) {
                    matrix_q[i][j] = matrix_q[i][j] + matrix_m[i][k]*matrix_n[k][j];
                }
            }
        }
        //send to process 0
        MPI_Send(&offset, 1, MPI_INT, 0, 101, MPI_COMM_WORLD);
        MPI_Send(&row_num, 1, MPI_INT, 0, 102, MPI_COMM_WORLD);
        MPI_Send(&matrix_q, row_num * N, MPI_INT, 0, 103, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
