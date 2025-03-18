#include <stdio.h>
#include <mpi.h>

#define ROW 4
#define COL 4

void printMatrix(int matrix[ROW][COL]) {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int matrix[ROW][COL] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    int transposed[COL][ROW];
    int local_row[COL];

    MPI_Init(&argc, &argv);  
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != ROW) {
        if (rank == 0)
            printf("This program requires %d MPI processes!\n", ROW);
        MPI_Finalize();
        return 0;
    }

    // Scatter rows of the matrix to each process
    MPI_Scatter(matrix, COL, MPI_INT, local_row, COL, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process sends its row to the correct column of the transposed matrix
    for (int i = 0; i < COL; i++) {
        MPI_Gather(&local_row[i], 1, MPI_INT, &transposed[i], 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // Process 0 prints the transposed matrix
    if (rank == 0) {
        printf("\nOriginal Matrix:\n");
        printMatrix(matrix);

        printf("\nTransposed Matrix:\n");
        printMatrix(transposed);
    }

    MPI_Finalize();
    return 0;
}
