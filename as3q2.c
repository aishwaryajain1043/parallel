#include <mpi.h>
#include <stdio.h>
#include <math.h>

#define NUM_STEPS 100000

double compute_pi(int start, int end, double step) {
    double sum = 0.0;
    for (int i = start; i < end; i++) {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int rank, size;
    double step = 1.0 / (double)NUM_STEPS;
    double local_sum = 0.0, global_sum = 0.0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int chunk_size = NUM_STEPS / size;
    int start = rank * chunk_size;
    int end = (rank == size - 1) ? NUM_STEPS : start + chunk_size;
    
    local_sum = compute_pi(start, end, step);
    
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double pi = step * global_sum;
        printf("Approximate value of Pi: %lf\n", pi);
    }

    MPI_Finalize();
    return 0;
}
