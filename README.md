# parallel
These assignments cover MPI basics like message passing, dynamic communication, and parallel computing tasks such as matrix multiplication, sorting, and numerical approximations. Advanced topics include reduction, prefix sum, DAXPY, and prime detection, showcasing efficient inter-process communication and performance gains.


Assignment 1
Q1.1: Hello World with MPI
This program introduces basic MPI functionality by having multiple processes print "Hello World" along with their rank and total number of processes. The key MPI functions used are MPI_Init, MPI_Comm_rank, MPI_Comm_size, and MPI_Finalize.

Q1.2: Sending and Receiving with MPI_Send and MPI_Recv
This program demonstrates point-to-point communication between MPI processes. A sender process (rank 0) transmits a message using MPI_Send, while the receiver process (rank 1) collects it using MPI_Recv. The program ensures proper synchronization between sender and receiver.

Q1.3: Dynamic Receiving with MPI_Probe and MPI_Status
Unlike MPI_Recv, MPI_Probe allows processes to check incoming messages without consuming them immediately. This program uses MPI_Probe to determine the message size before dynamically allocating memory for reception.

Q1.4: Random Walk Simulation using MPI
A simple simulation where a walker moves randomly based on communication between MPI processes. Each process computes the next step and updates the position. MPI is used for passing movement data among processes, showcasing inter-process communication in simulations.

Assignment 2
Q2.1: Estimating Pi using Monte Carlo Method
The Monte Carlo method estimates π by randomly generating points inside a unit square and checking if they fall inside a unit circle. Each process performs a portion of the calculations independently, and MPI_Reduce is used to aggregate results.

Q2.2: Parallel Matrix Multiplication (70×70)
Matrix multiplication is parallelized using MPI, where each process computes a portion of the result. The execution time is measured using omp_get_wtime() to compare serial vs. parallel execution speeds.

Q2.3: Parallel Sorting using Odd-Even Sort
A parallel sorting technique where neighboring processes exchange elements iteratively to ensure ordering. Communication between processes is done using MPI_Send and MPI_Recv.

Q2.4: Heat Distribution Simulation using MPI
A 2D grid-based simulation where each process handles a portion of the grid, updating temperatures based on neighbor values. MPI is used to exchange border values between adjacent processes.

Q2.5: Parallel Reduction using MPI
A reduction operation combines values from multiple processes into a single result. This is useful for operations like summation, minimum, or maximum calculations. MPI_Reduce performs the operation efficiently.

Q2.6: Parallel Dot Product using MPI
Each process computes a portion of the dot product independently, and results are aggregated using MPI_Reduce. This reduces computation time significantly for large vectors.

Q2.7: Parallel Prefix Sum (Scan) using MPI
The prefix sum operation computes cumulative sums across an array. MPI_Scan is used to ensure efficient computation across multiple processes.

Q2.8: Parallel Matrix Transposition using MPI
The matrix is split among processes, and each process exchanges its rows and columns with others. MPI communication ensures proper data transfer between processes for efficient transposition.

Assignment 3
Q3.1: DAXPY Loop Using MPI
DAXPY (X[i] = a * X[i] + Y[i]) is a basic vector operation parallelized across MPI processes. Each process handles a subset of the vectors and performs computations in parallel. The speedup is measured using MPI_Wtime(), comparing parallel execution to a single-threaded implementation.

Q3.2: Calculation of π Using MPI_Bcast and MPI_Reduce
This program calculates π in parallel, with the total number of iterations (num_steps) broadcasted using MPI_Bcast. Each process computes a partial sum, and MPI_Reduce gathers results to obtain the final value.

Q3.3: Prime Number Calculation Using MPI_Recv
A master-slave model where the master distributes numbers to be tested for primality. Slaves receive numbers using MPI_Recv, test for primality, and return results (positive for primes, negative for non-primes) using MPI_Send. The master processes results accordingly.

In conclusion, these assignments provide a comprehensive understanding of MPI, from basic communication to complex parallel computing tasks. They highlight the power of parallelism in optimizing performance and demonstrate the importance of efficient inter-process communication for large-scale computations.

Problem 1: Sum of First N Integers Using CUDA
Objective:
Implement CUDA programs to calculate the sum of the first N integers using two different methods.

Tasks:

Iterative Method: Compute the sum using a loop without using the mathematical formula.

Formula Method: Compute the sum using the mathematical formula within a CUDA kernel.

Implementation Steps:

Define the number N

Create input and output arrays

Allocate memory on the GPU

Populate the array with the first N integers

Copy data from host to device

Define CUDA grid and block dimensions

Launch the kernel to compute the sum

Copy the result back to host and display it

Problem 2: Merge Sort Using CPU and CUDA
Objective:
Implement and compare merge sort algorithms using CPU-based pipelining and GPU parallelism.

Tasks:

Implement merge sort using pipelining on CPU.

Implement merge sort in parallel using CUDA.

Measure and compare the performance of both approaches.

Implementation Steps:

Generate an array of size 1000

Implement both CPU and CUDA versions of merge sort

Use timing functions to benchmark both versions

Display execution time and calculate the speedup

Problem 3: Vector Addition and Bandwidth Measurement in CUDA
Objective:
Write a CUDA program for basic vector addition and compute memory bandwidth using profiling tools.

Tasks:

Modify the example to use statically defined global variables instead of dynamic memory allocation.

Record the kernel execution timing and print the results to the console.

Query the GPU device properties such as memory clock rate and memory bus width. Use these to calculate and print the theoretical bandwidth of the device.

Measure the actual bandwidth of the vector addition kernel by calculating the number of bytes read/written and dividing by the execution time. Print this to the console and compare it with the theoretical bandwidth.
