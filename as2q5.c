#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// Custom reduction function for MPI_Op_create
void sum_function(void* inputBuffer, void* outputBuffer, int* len, MPI_Datatype* datatype) {
    int i;
    int* input = (int*)inputBuffer;
    int* output = (int*)outputBuffer;
    
    if (*datatype == MPI_INT) {
        for (i = 0; i < *len; i++) {
            output[i] += input[i];
        }
    }
}

// Manual reduction implementation (tree-based)
int manual_reduction(int local_value, int rank, int size, MPI_Comm comm) {
    int global_sum = local_value;
    int step = 1;
    
    while (step < size) {
        if (rank % (2 * step) == 0) {
            // This process receives data
            if (rank + step < size) {
                int received_value;
                MPI_Status status;
                
                MPI_Recv(&received_value, 1, MPI_INT, rank + step, 0, comm, &status);
                global_sum += received_value;
            }
        } else if (rank % (2 * step) == step) {
            // This process sends data
            MPI_Send(&global_sum, 1, MPI_INT, rank - step, 0, comm);
            break;
        }
        
        step *= 2;
    }
    
    return global_sum;
}

int main(int argc, char** argv) {
    int rank, size, i;
    int array_size = 1000000;  // Default size
    int* data = NULL;
    int local_sum = 0;
    int global_sum_built_in = 0;
    int global_sum_custom_op = 0;
    int global_sum_manual = 0;
    double start_time, end_time;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Process command line arguments
    if (argc > 1) {
        array_size = atoi(argv[1]);
    }
    
    // Calculate local array size (distribute evenly)
    int local_size = array_size / size;
    if (rank < array_size % size) {
        local_size++;
    }
    
    // Seed random number generator differently for each process
    srand(time(NULL) + rank);
    
    // Allocate and initialize local array with random numbers (1-100)
    data = (int*)malloc(local_size * sizeof(int));
    for (i = 0; i < local_size; i++) {
        data[i] = rand() % 100 + 1;
        local_sum += data[i];
    }
    
    // Create a custom reduction operation
    MPI_Op custom_sum_op;
    MPI_Op_create(sum_function, 1, &custom_sum_op);
    
    if (rank == 0) {
        printf("Running reduction with %d processes on array of size %d\n", size, array_size);
        printf("Each process has approximately %d elements\n", local_size);
    }
    
    // Barrier to synchronize all processes before timing
    MPI_Barrier(MPI_COMM_WORLD);
    
    // METHOD 1: Using built-in MPI_Reduce
    start_time = MPI_Wtime();
    MPI_Reduce(&local_sum, &global_sum_built_in, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("\n1. Built-in MPI_Reduce:\n");
        printf("   Sum: %d\n", global_sum_built_in);
        printf("   Time: %f seconds\n", end_time - start_time);
    }
    
    // METHOD 2: Using custom reduction operation
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    MPI_Reduce(&local_sum, &global_sum_custom_op, 1, MPI_INT, custom_sum_op, 0, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("\n2. Custom reduction operation:\n");
        printf("   Sum: %d\n", global_sum_custom_op);
        printf("   Time: %f seconds\n", end_time - start_time);
    }
    
    // METHOD 3: Manual tree-based reduction implementation
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    global_sum_manual = manual_reduction(local_sum, rank, size, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("\n3. Manual tree-based reduction:\n");
        printf("   Sum: %d\n", global_sum_manual);
        printf("   Time: %f seconds\n", end_time - start_time);
    }
    
    // Demonstrate using MPI_Allreduce (everyone gets the result)
    int all_reduce_result = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    MPI_Allreduce(&local_sum, &all_reduce_result, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("\n4. MPI_Allreduce (everyone gets result):\n");
        printf("   Sum: %d\n", all_reduce_result);
        printf("   Time: %f seconds\n", end_time - start_time);
    }
    
    // Print each process's result from Allreduce (confirming all processes got the same sum)
    for (i = 0; i < size; i++) {
        if (rank == i) {
            printf("   Process %d received sum: %d\n", rank, all_reduce_result);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    // Clean up
    MPI_Op_free(&custom_sum_op);
    free(data);
    MPI_Finalize();  
    return 0;
}