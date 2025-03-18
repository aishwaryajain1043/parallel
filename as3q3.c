#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>

bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    int rank, size, num;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int max_value = 100;
    if (rank == 0) {
        for (int i = 2; i <= max_value; i++) {
            int worker;
            MPI_Recv(&worker, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&i, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
        }
        for (int i = 1; i < size; i++) {
            num = -1;
            MPI_Send(&num, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        while (1) {
            MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Recv(&num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (num < 0) break;
            if (is_prime(num)) {
                printf("Prime: %d\n", num);
            }
        }
    }

    MPI_Finalize();
    return 0;
}

