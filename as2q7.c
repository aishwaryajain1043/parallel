#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void print_array(int *arr, int size, int rank) {
    printf("Process %d: ", rank);
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main(int argc, char** argv) {
    int world_rank, world_size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int N = 8;  // Total number of elements in the array
    int local_size = N / world_size;  // Each process gets equal elements

    int *arr = NULL;
    int local_array[local_size];
    int local_prefix[local_size];

    if (world_rank == 0) {
        // Master initializes the array
        arr = (int*)malloc(N * sizeof(int));
        for (int i = 0; i < N; i++) {
            arr[i] = i + 1;  // Example array: {1, 2, 3, 4, 5, 6, 7, 8}
        }
        printf("Initial Array:\n");
        print_array(arr, N, world_rank);
    }

    // Scatter the data to all processes
    MPI_Scatter(arr, local_size, MPI_INT, local_array, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Compute local prefix sum
    local_prefix[0] = local_array[0];
    for (int i = 1; i < local_size; i++) {
        local_prefix[i] = local_prefix[i - 1] + local_array[i];
    }

    // Get the previous sum from other processes
    int prev_sum;
    MPI_Scan(&local_prefix[local_size - 1], &prev_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // Adjust each process's local prefix sum (except the first process)
    if (world_rank != 0) {
        for (int i = 0; i < local_size; i++) {
            local_prefix[i] += prev_sum - local_prefix[local_size - 1];
        }
    }

    // Gather results back to the root process
    MPI_Gather(local_prefix, local_size, MPI_INT, arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        printf("Final Prefix Sum:\n");
        print_array(arr, N, world_rank);
        free(arr);
    }

    MPI_Finalize();
    return 0;
}
