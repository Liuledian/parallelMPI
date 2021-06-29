#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


int my_mpi_allgather() {
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int N = 2;
    int *send_buf;
    int *recv_buf;
    int i,j;


    send_buf = malloc(sizeof(int)*N);
    recv_buf = malloc(sizeof(int)*N*world_size);
    for (i = 0; i < N; ++i) {send_buf[i] = world_rank;}

    //send to each process except itself
    for (i = 0; i < world_size; ++i) {
        if (i != world_rank) {
//            printf("P %d, Send %d\n",world_rank, i);
            MPI_Send(send_buf, N, MPI_INT, i, 0, MPI_COMM_WORLD);
//            printf("P %d, Sent %d\n",world_rank, i);
        }
    }

    //receive from other process
    for (i = 0; i < world_size; ++i) {
        if (i != world_rank) {
//            printf("P %d, Recv %d\n",world_rank, i);
            MPI_Recv(&recv_buf[i*N], N, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//            printf("P %d, Recved %d\n",world_rank, i);
        } else {
            for (j = 0; j < N; ++j) {
                recv_buf[i*N+j] = send_buf[j];
            }
        }

    }

    free(send_buf);
    free(recv_buf);
    return 0;
}

int real_mpi_allgather() {
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int N = 2;
    int *send_buf;
    int *recv_buf;
    int i,j;

    send_buf = malloc(sizeof(int)*N);
    recv_buf = malloc(sizeof(int)*N*world_size);
    for (i = 0; i < N; ++i) {send_buf[i] = world_rank;}

    MPI_Allgather(send_buf, N, MPI_INT, recv_buf, N, MPI_INT, MPI_COMM_WORLD);

    free(send_buf);
    free(recv_buf);
    return 0;


}

int main(){
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    double start, finish;
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    my_mpi_allgather();
    MPI_Barrier(MPI_COMM_WORLD);
    finish = MPI_Wtime();
    if (!world_rank){
        printf("My: Processes = %d, Elaspsed time= %e secs\n",world_size, finish - start);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    real_mpi_allgather();
    MPI_Barrier(MPI_COMM_WORLD);
    finish = MPI_Wtime();

    if (!world_rank){
        printf("MPI_Allgather: Processes = %d, Elaspsed time= %e secs\n",world_size, finish - start);
    }

    MPI_Finalize();
    return 0;
}
