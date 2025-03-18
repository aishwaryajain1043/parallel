#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 70  // Matrix size

void multiply_matrix(int rows, double A[rows][N], double B[N][N], double C[rows][N]) {
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++)
                C[i][j] += A[i][k] * B[k][j];
        }
}

int main(int argc, char** argv) {
    int rank, size;
    double A[N][N], B[N][N], C[N][N];
    double local_A[N / 2][N], local_C[N / 2][N];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        srand(time(NULL));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++) {
                A[i][j] = rand() % 10;
                B[i][j] = rand() % 10;
            }
    }

    MPI_Bcast(B, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(A, (N * N) / size, MPI_DOUBLE, local_A, (N * N) / size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double start_time = MPI_Wtime();
    multiply_matrix(N / size, local_A, B, local_C);
    MPI_Gather(local_C, (N * N) / size, MPI_DOUBLE, C, (N * N) / size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    double run_time = MPI_Wtime() - start_time;

    if (rank == 0) {
        printf("Parallel MPI Matrix Multiplication Time: %f seconds\n", run_time);
    }

    MPI_Finalize();
    return 0;
}


//for serial execution
// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>

// #define N 70

// void multiply_matrix(double A[N][N], double B[N][N], double C[N][N]) {
//     for (int i = 0; i < N; i++)
//         for (int j = 0; j < N; j++) {
//             C[i][j] = 0;
//             for (int k = 0; k < N; k++)
//                 C[i][j] += A[i][k] * B[k][j];
//         }
// }

// int main() {
//     double A[N][N], B[N][N], C[N][N];
//     srand(time(NULL));

//     for (int i = 0; i < N; i++)
//         for (int j = 0; j < N; j++) {
//             A[i][j] = rand() % 10;
//             B[i][j] = rand() % 10;
//         }

//     clock_t start_time = clock();
//     multiply_matrix(A, B, C);
//     double run_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;

//     printf("Sequential Matrix Multiplication Time: %f seconds\n", run_time);
//     return 0;
// }