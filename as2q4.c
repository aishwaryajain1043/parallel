#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define MASTER 0        // Rank of the master process
#define MAX_ITERATIONS 1000
#define CONVERGENCE_THRESHOLD 0.001

// Function to initialize the temperature grid
void initialize_grid(double **grid, int rows, int cols, int rank, int size) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            grid[i][j] = 0.0;  // Initialize interior to 0
        }
    }
    
    // Set boundary conditions
    if (rank == 0) {
        // Top boundary (hot)
        for (int j = 0; j < cols; j++) {
            grid[0][j] = 100.0;
        }
    }
    
    if (rank == size - 1) {
        // Bottom boundary (cold)
        for (int j = 0; j < cols; j++) {
            grid[rows-1][j] = 0.0;
        }
    }
    
    // Left boundary (warm)
    for (int i = 0; i < rows; i++) {
        grid[i][0] = 75.0;
    }
    
    // Right boundary (cool)
    for (int i = 0; i < rows; i++) {
        grid[i][cols-1] = 25.0;
    }
}

// Function to compute the new temperature at each point
double compute_iteration(double **current, double **next, int rows, int cols) {
    double max_diff = 0.0;
    
    // Update interior points
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            // Average of 4 neighbors
            next[i][j] = 0.25 * (current[i+1][j] + current[i-1][j] +
                                  current[i][j+1] + current[i][j-1]);
                                  
            double diff = fabs(next[i][j] - current[i][j]);
            if (diff > max_diff) {
                max_diff = diff;
            }
        }
    }
    
    // Preserve boundary conditions
    for (int j = 0; j < cols; j++) {
        next[0][j] = current[0][j];
        next[rows-1][j] = current[rows-1][j];
    }
    for (int i = 0; i < rows; i++) {
        next[i][0] = current[i][0];
        next[i][cols-1] = current[i][cols-1];
    }
    
    return max_diff;
}

// Function to exchange ghost rows between processes
void exchange_ghost_rows(double **grid, int rows, int cols, int rank, int size) {
    MPI_Status status;
    
    // Send bottom row to next process and receive top ghost row from previous process
    if (rank < size - 1) {
        MPI_Send(grid[rows-2], cols, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD);
    }
    if (rank > 0) {
        MPI_Recv(grid[0], cols, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &status);
    }
    
    // Send top row to previous process and receive bottom ghost row from next process
    if (rank > 0) {
        MPI_Send(grid[1], cols, MPI_DOUBLE, rank-1, 1, MPI_COMM_WORLD);
    }
    if (rank < size - 1) {
        MPI_Recv(grid[rows-1], cols, MPI_DOUBLE, rank+1, 1, MPI_COMM_WORLD, &status);
    }
}

// Function to save the final temperature grid to a file
void save_grid(double **grid, int rows, int cols, int rank, int size) {
    char filename[100];
    sprintf(filename, "heat_output_rank%d.csv", rank);
    
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Error opening file for writing\n");
        return;
    }
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(fp, "%.2f", grid[i][j]);
            if (j < cols - 1) fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }
    
    fclose(fp);
    printf("Process %d: Grid saved to %s\n", rank, filename);
}

int main(int argc, char *argv[]) {
    int rank, size, rows, cols;
    double **current_grid, **next_grid;
    double local_diff, global_diff;
    int iteration = 0;
    double start_time, end_time;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Get grid dimensions from command line or use defaults
    if (argc >= 3) {
        rows = atoi(argv[1]) / size + 2;  // +2 for ghost rows
        cols = atoi(argv[2]);
    } else {
        rows = 100 / size + 2;  // Default grid size
        cols = 100;
    }
    
    // Allocate memory for grids
    current_grid = (double**)malloc(rows * sizeof(double*));
    next_grid = (double**)malloc(rows * sizeof(double*));
    for (int i = 0; i < rows; i++) {
        current_grid[i] = (double*)malloc(cols * sizeof(double));
        next_grid[i] = (double*)malloc(cols * sizeof(double));
    }
    
    // Initialize the temperature grid
    initialize_grid(current_grid, rows, cols, rank, size);
    initialize_grid(next_grid, rows, cols, rank, size);
    
    if (rank == MASTER) {
        printf("Starting heat distribution simulation with %d processes\n", size);
        printf("Grid size: %d x %d per process\n", rows-2, cols);
        start_time = MPI_Wtime();
    }
    
    // Main simulation loop
    do {
        // Exchange ghost rows with neighbors
        exchange_ghost_rows(current_grid, rows, cols, rank, size);
        
        // Compute the next iteration
        local_diff = compute_iteration(current_grid, next_grid, rows, cols);
        
        // Find global maximum difference
        MPI_Allreduce(&local_diff, &global_diff, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
        
        // Swap the grids
        double **temp = current_grid;
        current_grid = next_grid;
        next_grid = temp;
        
        iteration++;
        
        if (rank == MASTER && iteration % 100 == 0) {
            printf("Iteration %d: maximum difference = %.6f\n", iteration, global_diff);
        }
        
    } while (iteration < MAX_ITERATIONS && global_diff > CONVERGENCE_THRESHOLD);
    
    // Measure end time
    if (rank == MASTER) {
        end_time = MPI_Wtime();
        printf("Simulation completed after %d iterations\n", iteration);
        printf("Execution time: %.3f seconds\n", end_time - start_time);
    }
    
    // Save results to file
    save_grid(current_grid, rows, cols, rank, size);
    
    // Clean up
    for (int i = 0; i < rows; i++) {
        free(current_grid[i]);
        free(next_grid[i]);
    }
    free(current_grid);
    free(next_grid);
    
    MPI_Finalize();
    return 0;
}

