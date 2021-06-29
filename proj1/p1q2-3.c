#include "mpi.h"
#include <stdio.h>

#define K 4
#define N 1024
#define RDIM 256


int main(int argc, char *argv[])
{
    int world_size, world_rank, source, dest, rows, averow, extra, offset;
    int i, j, k;
    int kernel[K][K];
    int m[N][N];
    int n[RDIM][RDIM];

    MPI_Status status;

    // MPI start
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_rank == 0)
    {
        // Initialize matrix in master
        for (i = 0; i < N; i++)
            for (j = 0; j < N; j++)
                m[i][j] = i + j;

        for (i = 0; i < K; i++)
            for (j = 0; j < K; j++)
                kernel[i][j] = i + j;

        double start = MPI_Wtime();

        averow = RDIM / (world_size - 1);
        extra = RDIM % (world_size - 1);
        offset = 0;
        for (dest = 1; dest < world_size; dest++)
        {
            rows = (dest <= extra) ? averow + 1 : averow;
            MPI_Send(&offset, 1, MPI_INT, dest, 99, MPI_COMM_WORLD);
            MPI_Send(&rows, 1, MPI_INT, dest, 99, MPI_COMM_WORLD);
            MPI_Send(&m[offset][0], rows * K * N, MPI_INT, dest, 99,
                     MPI_COMM_WORLD);
            MPI_Send(&kernel, K * K, MPI_INT, dest, 99, MPI_COMM_WORLD);
            offset = offset + rows * K;
        }

        // Receive final result
        for (i = 1; i < world_size; i++)
        {
            source = i;
            MPI_Recv(&offset, 1, MPI_INT, source, 100, MPI_COMM_WORLD, &status);
            MPI_Recv(&rows, 1, MPI_INT, source, 100, MPI_COMM_WORLD, &status);
            MPI_Recv(&n[offset / K][0], rows * RDIM, MPI_INT, source, 100,
                     MPI_COMM_WORLD, &status);
        }

        double finish = MPI_Wtime();
        printf("Processes = % d, M: %d x %d, Kernel: %d x %d\n", world_size, N, N, K, K);
        printf("Matrix convolution elapsed time: %f seconds.\n", finish - start);
    }
    else
    {
        MPI_Recv(&offset, 1, MPI_INT, 0, 99, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows, 1, MPI_INT, 0, 99, MPI_COMM_WORLD, &status);
        MPI_Recv(&m, rows * K * N, MPI_INT, 0, 99, MPI_COMM_WORLD, &status);
        MPI_Recv(&kernel, K * K, MPI_INT, 0, 99, MPI_COMM_WORLD, &status);

        for (i = 0; i < rows; i++)
        {
            for (j = 0; j < RDIM; j++)
            {
                int raw_index, col_index;
                int result = 0;
                for (raw_index = i * K; raw_index < (i + 1) * K; raw_index++)
                    for (col_index = j * K; col_index < (j + 1) * K; col_index++)
                        result += m[raw_index][col_index] * kernel[raw_index % K][col_index % K];
                n[i][j] = result;
            }
        }

        MPI_Send(&offset, 1, MPI_INT, 0, 100, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, 0, 100, MPI_COMM_WORLD);
        MPI_Send(&n, rows * RDIM, MPI_INT, 0, 100, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}