#include "mpi.h"
#include "stdio.h"

#define INTERVALS 1000000

int main(int argc, char* argv[]) {
    double global_area;
    double area;
    double ysum;
    double xi;
    int i;
    int p;
    int id;
    double elapsed_time;

    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = - MPI_Wtime();
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    ysum = 0;
    for (i = id; i < INTERVALS; i+= p) {
        xi = (1.0 / INTERVALS) * (i + 0.5);
        ysum += 4.0 / (1 + xi * xi);
    }
    area = ysum * (1.0 / INTERVALS);

    MPI_Reduce(&area, &global_area, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    elapsed_time += MPI_Wtime();

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    printf("Process %2d is done, %13.11f area, %fs from %s!\n", id, area, elapsed_time, processor_name);
    fflush(stdout);
    MPI_Finalize();

    if (id == 0) {
        printf("Pi is %13.11f from %s.\n", global_area, processor_name);
    }
}