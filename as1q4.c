#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WALKER_STEPS 10  // Maximum number of steps a walker takes
#define NUM_WALKERS 5    // Number of walkers per process

int main(int argc, char** argv) {
    int world_rank, world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    srand(time(NULL) + world_rank);  // Seed random differently per process

    int walkers[NUM_WALKERS];  // Each process starts with NUM_WALKERS

    // Initialize walkers with random steps
    for (int i = 0; i < NUM_WALKERS; i++) {
        walkers[i] = rand() % WALKER_STEPS + 1;  // 1 to WALKER_STEPS steps
    }

    printf("Process %d: Initialized %d walkers\n", world_rank, NUM_WALKERS);
    fflush(stdout);

    // Simulate walker movement
    for (int step = 0; step < WALKER_STEPS; step++) {
        int send_to = (world_rank + 1) % world_size; // Next process
        int recv_from = (world_rank - 1 + world_size) % world_size; // Previous process

        int outgoing_walker = -1;
        for (int i = 0; i < NUM_WALKERS; i++) {
            if (walkers[i] > 0) {
                walkers[i]--; // Take one step
                if (walkers[i] == 0) {
                    outgoing_walker = i; // Walker finished steps, send it
                }
            }
        }

        // Send walker if it finished steps
        if (outgoing_walker != -1) {
            MPI_Send(&walkers[outgoing_walker], 1, MPI_INT, send_to, 0, MPI_COMM_WORLD);
            printf("Process %d: Sent walker %d to Process %d\n", world_rank, outgoing_walker, send_to);
            fflush(stdout);
        }

        // Receive new walker from previous process
        MPI_Status status;
        int incoming_walker;
        int flag;
        MPI_Iprobe(recv_from, 0, MPI_COMM_WORLD, &flag, &status);
        if (flag) {
            MPI_Recv(&incoming_walker, 1, MPI_INT, recv_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Process %d: Received walker from Process %d\n", world_rank, recv_from);
            fflush(stdout);
        }

        MPI_Barrier(MPI_COMM_WORLD); // Synchronization after each step
    }

    printf("Process %d: Finished execution!\n", world_rank);
    fflush(stdout);

    MPI_Finalize();
    return 0;
}
