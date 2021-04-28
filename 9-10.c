#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#define TASK_MSG 0
#define RES_MSG 1
#define EMPTY_SMG 2
#define MAX_N 64

int isPrime(long long n) {
    if (n < 2) {
        return 0;
    }
    long long end = sqrt(n);
    for (long long i = 2; i <= end; i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

void  manager(int p) {

    long long res[MAX_N + 1];
    int found;
    int i;
    int terminated;
    int assign_n;
    int *assigned;
    long long buffer;
    MPI_Status status;
    int src;
    int tag;


    terminated = 0;
    assign_n = 2;
    found = 0;
    assigned = (int*) malloc(p * sizeof(int));
    for (i = 0; i <= MAX_N; i++) {
        res[i] = 0;
    }

    do {
        MPI_Recv(&buffer, 1, MPI_LONG_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        src = status.MPI_SOURCE;
        tag = status.MPI_TAG;

        if (tag == RES_MSG && buffer) {
            res[assigned[src]] = buffer;
            found++;
        }
        // Assign more tasks or terminate workers
        if (found < 8) {
            MPI_Send(&assign_n, 1, MPI_INT, src, TASK_MSG, MPI_COMM_WORLD);
            assigned[src] = assign_n;
            assign_n++;
        } else {
            MPI_Send(NULL, 0, MPI_CHAR, src, EMPTY_SMG, MPI_COMM_WORLD);
            terminated++;
        }
    } while (terminated < p - 1);

    // Print result
    int j = 0;
    for (i = 0; i <= MAX_N && j < 8; i++) {
        if (res[i]) {
            printf("%lld\n", res[i]);
            j++;
        }
    }
}

void worker() {

    int n;
    long long res;
    MPI_Status status;
    MPI_Request request;
    int i;

    // Initial request for work
    MPI_Isend(NULL, 0, MPI_CHAR, 0, EMPTY_SMG, MPI_COMM_WORLD, &request);

    while (1) {
        MPI_Recv(&n, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == EMPTY_SMG) break;
        // Check 2^n -1 is prime
        long long tmp = (1 << n) - 1;
        if (isPrime(tmp)) {
            res = tmp * (1 << (n -1));
        } else {
            res = 0;
        }
        MPI_Send(&res, 1, MPI_LONG_LONG, 0, RES_MSG, MPI_COMM_WORLD);
    }
}

int main(int argc, char* argv[]) {
    int id;
    int p;
    double elapsed_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = - MPI_Wtime();

    if (id == 0) manager(p);
    else worker();

    // Compute running time
    elapsed_time += MPI_Wtime();
    double max_seconds;
    MPI_Allreduce (&elapsed_time, &max_seconds, 1, MPI_DOUBLE, MPI_MAX,
                   MPI_COMM_WORLD);
    if (!id) {
        printf ("Processes = %d, Time = %12.6f sec\n", p, max_seconds);
    }
    MPI_Finalize();

    return 0;
}


