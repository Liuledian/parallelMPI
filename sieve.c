#include "mpi.h"
#include "stdio.h"
#include <math.h>
#include <stdlib.h>

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

int main(int argc, char* argv[]) {
    int p;
    int id;
    int n;
    int i, j;
    double elapsed_time;

    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time  = - MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    // Find all prime between 2 and n^0.5
    n = atoi(argv[1]);
    int sqrt_n = sqrt(n);
    int len = 0;
    int* primeArr = (int*) malloc(sizeof(int) * sqrt_n);
    for (i = 2; i <= sqrt_n; i++) {
        if (isPrime(i)) {
            primeArr[len++] = i;
        }
    }

    // Mark numbers
    char* marked = (char*) malloc(n - 1);
    for (i = 0; i < n - 1; i++) marked[i] = 0;
    char* globalMarked = (char*) malloc(n - 1);
    for (i = 0; i < n - 1; i++) globalMarked[i] = 0;

    for (i = id; i < len; i += p) {
        int k = primeArr[i];
        for (j = k * k; j <= n; j += k) {
            marked[j - 2] = 1;
        }
    }

    // Reduce
    MPI_Reduce(marked, globalMarked, n - 1, MPI_CHAR, MPI_LOR, 0, MPI_COMM_WORLD);

    // Count the unmarked number in process 0
    int res = 0;
    if (id == 0) {
        for (i = 0; i < n - 1; i++) {
            if (!globalMarked[i]) res++;
        }
    }

    // Print time for each process
    elapsed_time += MPI_Wtime();
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    printf("Process %2d is done, %fs from %s!\n", id,  elapsed_time, processor_name);
    fflush(stdout);
    MPI_Finalize();

    if (id == 0) {
        printf("There are %7d primes from 2 to %d.\n", res, n);
    }

    return 0;
}
