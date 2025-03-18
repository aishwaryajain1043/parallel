#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

double monte_carlo_pi(int num_samples) {
    int inside_circle = 0;
    for (int i = 0; i < num_samples; i++) {
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;
        if (x * x + y * y <= 1)
            inside_circle++;
    }
    return inside_circle;
}


int main(int argc, char** argv) {
    int rank, size, num_samples = 1000000, local_count, total_count;
    double pi_estimate;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(rank);  // Seed different for each process
    local_count = monte_carlo_pi(num_samples);

    MPI_Reduce(&local_count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        pi_estimate = 4.0 * total_count / (num_samples * size);
        printf("Estimated Pi: %f using %d samples\n", pi_estimate, num_samples * size);
    }

    MPI_Finalize();
    return 0;
}
