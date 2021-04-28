#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"

#define ROW_MSG 0
#define RES_MSG 1
#define EMPTY_SMG 2


typedef double dtype;
#define mpitype MPI_DOUBLE

void  manager(char* mfile, char* vfile, int p) {
    dtype **a;             /* The first factor, a matrix */
    dtype  *b;             /* The second factor, a vector */
    dtype  *c;             /* The product, a vector */
    dtype  *storage;
    int     m;             /* Rows in the matrix */
    int     n;             /* Columns in the matrix */
    int     nb;
    FILE    *fileptr;    /* Input file pointer */

    int i;
    int terminated;
    int assign_cnt;
    int *assigned;
    dtype *buffer;
    MPI_Status status;
    int src;
    int tag;

    // Vector
    fileptr = fopen(vfile, "r");
    fread(&nb, sizeof(int), 1, fileptr);

    // Allocate vector memory
    b = (dtype*) malloc(nb * sizeof(dtype));
    fread(b, sizeof(dtype), nb, fileptr);
    fclose(fileptr);

    // Matrix
    fileptr = fopen (mfile, "r");
    fread (&m, sizeof(int), 1,fileptr);
    fread (&n, sizeof(int), 1, fileptr);

    // Allocate matrix storage
    storage = (dtype*) malloc(m * n * sizeof(dtype));
    a = (dtype**) malloc(m * sizeof(dtype*));
    dtype* ptr = storage;
    for (i = 0; i < m; ++i) {
        a[i] = ptr;
        ptr += n;
    }

    fread(storage, sizeof(dtype), m * n, fileptr);
    fclose(fileptr);



    // Broadcast vector
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(b, nb, mpitype, 0, MPI_COMM_WORLD);

    terminated = 0;
    assign_cnt = 0;
    assigned = (int*) malloc(p * sizeof(int));
    buffer = (dtype*) malloc(sizeof(dtype));
    c = (dtype*) malloc(m * sizeof(dtype));

    do {
        MPI_Recv(buffer, 1, mpitype, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        src = status.MPI_SOURCE;
        tag = status.MPI_TAG;

        if (tag == RES_MSG) {
            c[assigned[src]] = *buffer;
        }
        // Assign more tasks or terminate workers
        if (assign_cnt < m) {
            MPI_Send(a[assign_cnt], n, mpitype, src, ROW_MSG, MPI_COMM_WORLD);
            assigned[src] = assign_cnt;
            assign_cnt++;
        } else {
            MPI_Send(NULL, 0, MPI_CHAR, src, EMPTY_SMG, MPI_COMM_WORLD);
            terminated++;
        }
    } while (terminated < p - 1);

    // Print result
    for (i = 0; i < m; i++) {
        printf("%6.4f\t", c[i]);
    }
    printf("\n");
}

void worker() {
    dtype *b;
    dtype res;
    dtype *row;
    int n;
    MPI_Status status;
    MPI_Request request;
    int i;

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    b = (dtype*) malloc(n * sizeof(dtype));
    MPI_Bcast(b, n, mpitype, 0, MPI_COMM_WORLD);

    // Initial request for work
    MPI_Isend(NULL, 0, MPI_CHAR, 0, EMPTY_SMG, MPI_COMM_WORLD, &request);
    // Allocate space
    row = (dtype*) malloc(n * sizeof(dtype));

    while (1) {
        MPI_Recv(row, n, mpitype, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == EMPTY_SMG) break;
        // Compute dot product
        res = 0;
        for (i = 0; i < n; i++) {
            res += row[i] * b[i];
        }
        MPI_Send(&res, 1, mpitype, 0, RES_MSG, MPI_COMM_WORLD);
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

    if (id == 0) manager(argv[1], argv[2], p);
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

