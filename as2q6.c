#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <math.h>

// Function to initialize a vector with random values
void init_vector(double *vec, int size) {
    for (int i = 0; i < size; i++) {
        vec[i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;  // Random value between -1 and 1
    }
}

// Function to calculate the dot product of two vectors
double dot_product(double *vec1, double *vec2, int size) {
    double result = 0.0;
    for (int i = 0; i < size; i++) {
        result += vec1[i] * vec2[i];
    }
    return result;
}

// Function to verify the dot product calculation is correct
double sequential_dot_product(double *vec1, double *vec2, int size) {
    double result = 0.0;
    for (int i = 0; i < size; i++) {
        result += vec1[i] * vec2[i];
    }
    return result;
}

int main(int argc, char *argv[]) {
    int rank, size, n;
    double *vec_a = NULL, *vec_b = NULL;      // Full vectors (only on rank 0)
    double *local_a = NULL, *local_b = NULL;  // Local portions of vectors
    double local_dot = 0.0, global_dot = 0.0;
    double start_time, end_time, total_time;
    int local_size;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Get vector size from command line or use default
    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        n = 100000000;  // Default size: 100 million elements
    }
    
    // Calculate how many elements each process will handle
    local_size = n / size;
    int remainder = n % size;
    
    // Adjust local_size if n is not perfectly divisible by size
    if (rank < remainder) {
        local_size++;
    }
    
    // Allocate memory for local vectors
    local_a = (double*)malloc(local_size * sizeof(double));
    local_b = (double*)malloc(local_size * sizeof(double));
    
    if (local_a == NULL || local_b == NULL) {
        fprintf(stderr, "Process %d: Memory allocation failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Only rank 0 initializes the full vectors
    if (rank == 0) {
        // Allocate and initialize full vectors
        vec_a = (double*)malloc(n * sizeof(double));
        vec_b = (double*)malloc(n * sizeof(double));
        
        if (vec_a == NULL || vec_b == NULL) {
            fprintf(stderr, "Rank 0: Memory allocation failed for full vectors\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        // Seed random number generator
        srand(time(NULL));
        
        // Initialize vectors with random values
        init_vector(vec_a, n);
        init_vector(vec_b, n);
        
        printf("Starting parallel dot product calculation of two vectors of size %d using %d processes\n", n, size);
    }
    
    // Start timing
    MPI_Barrier(MPI_COMM_WORLD);  // Synchronize before starting timer
    start_time = MPI_Wtime();
    
    // Calculate send counts and displacements for scatterv
    int *sendcounts = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));
    
    if (sendcounts == NULL || displs == NULL) {
        fprintf(stderr, "Process %d: Memory allocation failed for scatterv arrays\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    int disp = 0;
    for (int i = 0; i < size; i++) {
        sendcounts[i] = n / size;
        if (i < remainder) {
            sendcounts[i]++;
        }
        displs[i] = disp;
        disp += sendcounts[i];
    }
    
    // Distribute data using Scatterv (handles uneven distribution)
    MPI_Scatterv(vec_a, sendcounts, displs, MPI_DOUBLE, 
                 local_a, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    MPI_Scatterv(vec_b, sendcounts, displs, MPI_DOUBLE, 
                 local_b, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    // Calculate local dot product
    local_dot = dot_product(local_a, local_b, local_size);
    
    // Reduce all local dot products to get the global dot product
    MPI_Reduce(&local_dot, &global_dot, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // End timing
    end_time = MPI_Wtime();
    total_time = end_time - start_time;
    
    // Calculate the maximum time across all processes
    double max_time;
    MPI_Reduce(&total_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    
    // Verify result on rank 0 (only for moderately-sized vectors)
    if (rank == 0) {
        printf("Parallel dot product result: %.8f\n", global_dot);
        printf("Execution time: %.6f seconds\n", max_time);
        
        // Verify with sequential calculation if vector size is manageable
        if (n <= 10000000) {  // Only verify for vectors up to 10M elements
            double seq_result = sequential_dot_product(vec_a, vec_b, n);
            printf("Sequential verification result: %.8f\n", seq_result);
            printf("Difference: %.10f\n", fabs(global_dot - seq_result));
            
            // Calculate relative error
            double rel_error = fabs(global_dot - seq_result) / (fabs(seq_result) > 1e-10 ? fabs(seq_result) : 1.0);
            printf("Relative error: %.10e\n", rel_error);
            
            if (rel_error < 1e-10) {
                printf("Verification: PASSED\n");
            } else {
                printf("Verification: FAILED (error too large)\n");
            }
        } else {
            printf("Vector too large for sequential verification\n");
        }
        
        // Calculate performance metrics
        double gflops = (2.0 * n) / (max_time * 1e9);  // 2 FLOPS per element (multiply + add)
        printf("Performance: %.4f GFLOPS\n", gflops);
        
        // Free full vectors
        free(vec_a);
        free(vec_b);
    }
    
    // Free local vectors and arrays
    free(local_a);
    free(local_b);
    free(sendcounts);
    free(displs);
    
    MPI_Finalize();
    return 0;
}
