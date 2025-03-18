#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_rank == 0) {
        // Process 0 sends a random number of integers to process 1
        srand(time(NULL));
        int num_elements = rand() % 100 + 1; // Random number between 1 and 100
        int* data = (int*)malloc(num_elements * sizeof(int));

        // Initialize the data with some values
        for (int i = 0; i < num_elements; i++) {
            data[i] = i;
        }

        MPI_Send(data, num_elements, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("Process 0 sent %d numbers to Process 1\n", num_elements);
        free(data);
    } else if (world_rank == 1) {
        MPI_Status status;

        // Probe for an incoming message from process 0
        MPI_Probe(0, 0, MPI_COMM_WORLD, &status);

        // Get the number of elements in the incoming message
        int num_elements;
        MPI_Get_count(&status, MPI_INT, &num_elements);

        // Allocate a buffer to hold the incoming data
        int* data = (int*)malloc(num_elements * sizeof(int));

        // Now receive the message
        MPI_Recv(data, num_elements, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process 1 received %d numbers from Process 0\n", num_elements);

        // Free the allocated memory
        free(data);
    }

    MPI_Finalize();
    return 0;
}
