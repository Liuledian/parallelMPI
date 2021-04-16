// P 6.10
#include "stdio.h"
#include "mpi.h"
#include "stdlib.h"

int My_MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int p;
    int id;

    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (id == root) {
        for (int i = 0; i < p; i++) {
            if (i != root) {
                MPI_Send(buffer, count, datatype, i, 0, comm);
            }
        }
    } else {
        MPI_Status status;
        MPI_Recv(buffer, count, datatype, root, 0, MPI_COMM_WORLD, &status);
    }

    return 0;
}


int main(int argc, char* argv[]) {
    int p;
    int id;
    double time1, time2, time3;
    int* array;
    int n_repeats;
    int n_items;

    n_items = atoi(argv[1]);
    n_repeats = atoi(argv[2]);
    array = (int*) malloc(sizeof(int) * n_items);

    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    time1 =  MPI_Wtime();
    for (int i = 0; i < n_repeats; i++) {
        MPI_Bcast(array, n_items, MPI_INT, 0, MPI_COMM_WORLD);
    }
    time2 = MPI_Wtime();
    for (int i = 0; i < n_repeats; i++) {
        My_MPI_Bcast(array, n_items, MPI_INT, 0, MPI_COMM_WORLD);
    }
    time3 = MPI_Wtime();

    // Print time for each process
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    printf("Process %2d from %s is done, default: %fs / my: %fs!\n",
           id, processor_name, time2 - time1, time3 - time2);
    fflush(stdout);
    MPI_Finalize();

    return 0;
}