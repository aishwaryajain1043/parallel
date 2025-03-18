#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 20  // Array size

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void odd_even_sort(int local_array[], int n, int rank, int size, MPI_Comm comm) {
    for (int phase = 0; phase < n; phase++) {
        int partner = (phase % 2 == 0) ? rank ^ 1 : rank ^ 1;
        if (partner >= 0 && partner < size) {
            int temp_array[n];
            MPI_Sendrecv(local_array, n, MPI_INT, partner, 0,
                         temp_array, n, MPI_INT, partner, 0, comm, MPI_STATUS_IGNORE);
            for (int i = 0; i < n; i++) {
                if ((rank < partner && local_array[i] > temp_array[i]) ||
                    (rank > partner && local_array[i] < temp_array[i])) {
                    swap(&local_array[i], &temp_array[i]);
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    int rank, size;
    int local_array[N / 4];  // Divide among processes
    int global_array[N];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        srand(time(NULL));
        for (int i = 0; i < N; i++) global_array[i] = rand() % 100;
        printf("Unsorted array: ");
        for (int i = 0; i < N; i++) printf("%d ", global_array[i]);
        printf("\n");
    }

    MPI_Scatter(global_array, N / size, MPI_INT, local_array, N / size, MPI_INT, 0, MPI_COMM_WORLD);
    odd_even_sort(local_array, N / size, rank, size, MPI_COMM_WORLD);
    MPI_Gather(local_array, N / size, MPI_INT, global_array, N / size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Sorted array: ");
        for (int i = 0; i < N; i++) printf("%d ", global_array[i]);
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}

