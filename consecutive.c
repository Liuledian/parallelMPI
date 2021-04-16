#include <stdio.h>
#include <mpi.h>
#include <math.h>

int isPrime(int n) {
    if (n < 2) {
        return 0;
    }
    int end = sqrt(n);
    for (int i = 2; i <= end; i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

void consecutive_prime(int upper) {
    int global_num;
    int num;
    int p;
    int id;
    int i;
    double elapsed_time;

    MPI_Init(NULL, NULL);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = - MPI_Wtime();
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    num = 0;
    if (upper % 2 == 0) {
        upper -= 1;
    } else {
        upper -= 2;
    }

    for (i = 3 + id * 2; i < upper; i += p * 2) {
        if (isPrime(i) && isPrime(i + 2)) {
            num += 1;
        }
    }

    MPI_Reduce(&num, &global_num, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    elapsed_time += MPI_Wtime();

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    printf("Process %2d is done, %7d times, %fs from %s!\n", id, num, elapsed_time, processor_name);
    fflush(stdout);
    MPI_Finalize();

    if (id == 0) {
        printf("There are %7d times in total from %s.\n", global_num, processor_name);
    }
}

int main() {
    consecutive_prime(1000000);
    return 0;
}
