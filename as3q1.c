#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define N (1 << 16) // 2^16 elements
// Serial version of DAXPY
void daxpy_serial(double *X, double *Y, double a, int n) {
    for (int i = 0; i < n; i++) {
        X[i] = a * X[i] + Y[i];
    }
}
// Parallel version of DAXPY using MPI
void daxpy_parallel(double *X, double *Y, double a, int n, int rank, int size) {
    int local_n = n / size; // Local size for each process
    double *local_X = (double *)malloc(local_n * sizeof(double));
    double *local_Y = (double *)malloc(local_n * sizeof(double));
    // Scatter data to all processes
    MPI_Scatter(X, local_n, MPI_DOUBLE, local_X, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(Y, local_n, MPI_DOUBLE, local_Y, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // Perform DAXPY operation locally
    for (int i = 0; i < local_n; i++) {
        local_X[i] = a * local_X[i] + local_Y[i];
    }
    // Gather the results back to the root process
    MPI_Gather(local_X, local_n, MPI_DOUBLE, X, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    free(local_X);
    free(local_Y);
}

int main(int argc, char *argv[]) {
    int rank, size;
    double *X = NULL, *Y = NULL;
    double a = 2.0; // Scalar value for the operation
    double serial_start, serial_end, parallel_start, parallel_end;
    double serial_time, parallel_time;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // Allocate memory for the vectors X and Y (only on root process)
    if (rank == 0) {
        X = (double *)malloc(N * sizeof(double));
        Y = (double *)malloc(N * sizeof(double));
        // Initialize X and Y vectors
        for (int i = 0; i < N; i++) {
            X[i] = i * 1.0; // Some example initialization
            Y[i] = i * 2.0; // Some example initialization
        }
    }
    // Serial version timing
    if (rank == 0) {
        serial_start = MPI_Wtime();
        daxpy_serial(X, Y, a, N);
        serial_end = MPI_Wtime();
        serial_time = serial_end - serial_start;
        printf("Serial Time: %f seconds\n", serial_time);
    }
    // Parallel version timing
    parallel_start = MPI_Wtime();
    daxpy_parallel(X, Y, a, N, rank, size);
    parallel_end = MPI_Wtime();
    parallel_time = parallel_end - parallel_start;

    if (rank == 0) {
        printf("Parallel Time (using %d processes): %f seconds\n", size, parallel_time);
        printf("Speedup: %f\n", serial_time / parallel_time);
    }
    // Clean up
    if (rank == 0) {
        free(X);
        free(Y);
    }
    MPI_Finalize();
    return 0;
}
