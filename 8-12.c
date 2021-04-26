#include "stdlib.h"
#include "stdio.h"
#include "MyMPI.h"
#include "mpi.h"

#define MAX_FLOAT 10000

void alloc_matrix(void ***a, int m, int n, int size) {
    int i;
    void* storage;
    storage = malloc(m * n * sizeof(size));
    *a = (void**) malloc(m * sizeof(void*));
    for (i = 0; i < m; i++) {
        (*a)[i] = storage + i * n * size;
    }
}

void print_root(int** root, int low, int high) {
    printf("Root of tree spanning %d-%d is %d\n", low, high, root[low][high+1]);
    if (low < root[low][high+1] - 1) {
        print_root(root, low, root[low][high+1] - 1);
    }
    if (root[low][high+1] < high - 1) {
        print_root(root, root[low][high+1] + 1, high);
    }

}

int main(int argc, char* argv[]) {
    float* p; //probability
    int n; // number of keys
    int** root;
    float** cost;
    int i;
    int id;
    int proc;
    int* disp_in;
    int* cnt_in;

    // Initialize MPI
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &proc);

    // Get input p
    if (id == 0) {
        printf("start\n");
        scanf("%d", &n);
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    p = (float*) malloc(n * sizeof(float));
    if (id == 0) {
        for (i = 0; i < n; i++) {
            scanf("%f", &p[i]);
        }
    }
    MPI_Bcast(p, n, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Start counting time
    MPI_Barrier(MPI_COMM_WORLD);
    double elapsed_time = -MPI_Wtime();

    // Create solution table
    alloc_matrix((void***) &cost, n+1, n+1, sizeof(float));
    alloc_matrix((void***) &root, n+1, n+1, sizeof(int));


    for (i = 0; i <= n; i++) {
        cost[i][i] = 0;
        root[i][i] = i;
    }

    float* cost_send;
    int* root_send;
    float* cost_recv;
    int* root_recv;
    cost_send = (float*) malloc(n * sizeof(float));
    root_send = (int*) malloc(n * sizeof(int));
    cost_recv = (float*) malloc(n * sizeof(float));
    root_recv = (int*) malloc(n * sizeof(int));

    for (int axis = 1; axis <=n; axis++) {
        int size = n + 1 - axis;
        int block_size = BLOCK_SIZE(id, proc, size);
        int row_start = BLOCK_LOW(id, proc, size);
        for (i = 0; i < block_size; i++) {
            int row = row_start + i;
            int col = row + axis;
            float best_cost = MAX_FLOAT;
            int best_root;
            for (int r = row; r < col; r++) {
                float rcost = cost[row][r] + cost[r+1][col];
                for (int j = row; j < col; j++) rcost += p[j];
                if (rcost < best_cost) {
                    best_cost = rcost;
                    best_root = r;
                }
            }
            cost_send[i] = best_cost;
            root_send[i] = best_root;
        }

        create_mixed_xfer_arrays(id, proc, size, &cnt_in, &disp_in);
        MPI_Allgatherv(cost_send, block_size, MPI_FLOAT, cost_recv, cnt_in, disp_in, MPI_FLOAT, MPI_COMM_WORLD);
        MPI_Allgatherv(root_send, block_size, MPI_INT, root_recv, cnt_in, disp_in, MPI_INT, MPI_COMM_WORLD);
        for (i = 0; i < size; i++) {
            cost[i][i + axis] = cost_recv[i];
            root[i][i + axis] = root_recv[i];
        }
    }

    if (id == 0) {
        print_root(root, 0, n-1);
    }

    free(cost_send);
    free(cost_recv);
    free(root_recv);
    free(root_send);
    free(cost[0]);
    free(cost);
    free(root[0]);
    free(root);

    elapsed_time += MPI_Wtime();
    double max_seconds;
    MPI_Allreduce (&elapsed_time, &max_seconds, 1, MPI_DOUBLE, MPI_MAX,
                   MPI_COMM_WORLD);
    if (!id) {
        printf ("Processes = %d, Time = %12.6f sec\n", proc, max_seconds);
    }
    MPI_Finalize();
    return 0;
}
