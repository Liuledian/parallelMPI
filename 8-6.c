#include <stdio.h>
#include <mpi.h>
#include "MyMPI.h"

/* Change these two definitions when the matrix and vector
   element types change */

typedef double dtype;
#define mpitype MPI_DOUBLE

int main (int argc, char *argv[]) {
    dtype **a;             /* The first factor, a matrix */
    dtype  *b;             /* The second factor, a vector */
    dtype  *c;             /* The product, a vector */
    dtype  *c_part_out;    /* Partial sums, sent */
    dtype  *c_part_in;     /* Partial sums, received */
    int    *cnt_out;       /* Elements sent to each proc */
    int    *cnt_in;        /* Elements received per proc */
    int    *disp_out;      /* Indices of sent elements */
    int    *disp_in;       /* Indices of received elements */
    int     i, j;          /* Loop indices */
    int     id;            /* Process ID number */
    int     local_els;     /* Cols of 'a' and elements of 'b'
                             held by this process */
    int     m;             /* Rows in the matrix */
    int     n;             /* Columns in the matrix */
    int     nprime;        /* Size of the vector */
    int     p;             /* Number of processes */
    double    max_seconds;
    double    seconds;     /* Elapsed time */
    dtype  *storage;       /* This process's portion of 'a' */

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);

    read_col_striped_matrix (argv[1], (void ***) &a,
                             (void **) &storage, mpitype, &m, &n, MPI_COMM_WORLD);
    print_col_striped_matrix ((void **) a, mpitype, m, n,
                              MPI_COMM_WORLD);
    read_replicated_vector(argv[2], (void **) &b, mpitype,
                       &nprime, MPI_COMM_WORLD);
    print_replicated_vector((void *) b, mpitype, nprime,
                        MPI_COMM_WORLD);


    MPI_Barrier (MPI_COMM_WORLD);
    seconds = -MPI_Wtime();
    c_part_out = (dtype *) my_malloc (id, n * sizeof(dtype));
    local_els = BLOCK_SIZE(id,p,n);
    int col_start = BLOCK_LOW(id, p, n);

    for (i = 0; i < n; i++) {
        c_part_out[i] = 0.0;
        for (j = 0; j < local_els; j++)
            c_part_out[i] += a[i][j] * b[j + col_start];
    }

    // Replicated result vector c
    c = (dtype *) my_malloc(id, n * sizeof(dtype));
    for (i = 0; i < n; i++) {
        c[i] = 0;
    }
    MPI_Allreduce(c_part_out, c, n, mpitype, MPI_SUM, MPI_COMM_WORLD);

    print_replicated_vector ((void *) c, mpitype, n, MPI_COMM_WORLD);

    MPI_Barrier (MPI_COMM_WORLD);
    seconds += MPI_Wtime();
    MPI_Allreduce (&seconds, &max_seconds, 1, mpitype, MPI_MAX,
                   MPI_COMM_WORLD);
    if (!id) {
        printf ("Matrix size %5d x %5d,Processes = %d, Time = %12.6f sec\n", m, n, p, max_seconds);
    }
    MPI_Finalize();
    return 0;
}


