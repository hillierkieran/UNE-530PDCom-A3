### UNE Trimester 2, 2023 - COSC530 Parallel and Distributed Computing

# Assignment 3 - Matrix Processing

## Matrix Data Processing

This assignment is to implement a simple convolution filter on a matrix. The process for applying a convolution filter essentially involves replacing the value of each element in the matrix with a new value calculated using some function based upon the neighbouring values. The best real-world example of this process can be seen with the application of blur filters used on images – each pixel in the image is updated using a weighted average calculated from the surrounding neighbours.

The simplest example of this process can be demonstrated by taking a 4x4 matrix and updating each element to contain the sum of its immediate neighbours: 
 
| 1 | 2 | 1 | 4 |  
|---|---|---|---| 
| 2 | 3 | 1 | 2 |  
| 1 | 0 | 1 | 1 |  
| 1 | 5 | 0 | 1 |  

The corresponding processed matrix

|  7 |  8 | 12 |  4 |  
|:--:|:--:|:--:|:--:| 
|  7 |  9 | 14 |  8 |  
| 11 | 14 | 13 |  5 |  
|  6 |  3 |  8 |  2 |  

In this scenario, the neighbourhood is defined as those elements directly adjacent to the element (including diagonals). E.g. There is a neighbourhood depth of 1 in this situation.

## Task

The task is to implement a C program using MPI that can process a square data matrix using a convolution filter that replaces each element with the **weighted sum** of its neighbours. Each element's contribution to the sum should be weighted by _1/n\_depth_, where the _n\_depth_ is the neighbourhood depth of the specific element.

The program should be able to handle any square matrix of arbitrary size and the operation should be carried out by any arbitrary number of MPI processes. The program should also handle a neighbourhood depth of any arbitrary size.

The simplest approach to solve this problem will involve having each process calculate the result for a set of consecutive rows or columns in the output matrix. For example, if the program is executed with two processes, the first process will calculate the results for elements in the top half of the matrix, the second process will work on the bottom half.

**Things to consider in your implementation:**

*   The program should read the matrix in from a file - review the [examples from lecture 17](http://turing.une.edu.au/~cosc330/lectures/lecture_17/examples/) where we used a set of utilities for managing matrices.
*   Each process (i.e. slave node) should only be sent the minimum data needed to calculate its segment of the output matrix (i.e. don’t pass the whole starting matrix to all processes – unless there is only 1 process used!).
*   The elements around the edge of the matrix will have fewer neighbours than those on the interior for matrices of size greater than 2x2. The program will need to handle these cases (the row/column neighbourhood should not wrap around to the opposite side)
*   The partial results from the the slave nodes will need to be communicated to the master node, collated and written to an output file.

The names of the input file, output file and the depth of the neighbourhood should be passed to the program as command line arguments. The program should make use of the matrix utilities that were introduced in lecture 17 for reading and writing matrices to-and-from files:

[lectures 17: matrix.h](http://turing.une.edu.au/~cosc330/lectures/lecture_17/examples/matrix.h)  
[lectures 17: matrix.c](http://turing.une.edu.au/~cosc330/lectures/lecture_17/examples/matrix.c)  
[lectures 17: mkRandomMatrix.c](http://turing.une.edu.au/~cosc330/lectures/lecture_17/examples/mkRandomMatrix.c)  
[lectures 17: getMatrix.c](http://turing.une.edu.au/~cosc330/lectures/lecture_17/examples/getMatrix.c)

The format used in these examples will be used to test the program (e.g. matrix files produced by mkRandomMatrix will be used to test the program).

You are free to use any code that is introduced in the lectures as part of your solution.

## Tentative Marking Scheme

**Solution Correctness - 70%**

*   Does your solution generate the correct output?
*   Have you implemented the weighted sum calculation correctly?
*   Is all I/O done through process 0?
*   Have you implemented efficient communication?
*   Does each process only use the minimal data required?
*   Does the program divide the processing evenly?
*   Does the program scale correctly with different numbers of processes and different sized matrices?

**Quality of Solution - 15%**

*   Is your code broken down into functions (e.g. not more than about 60 lines - excluding braces, comments and whitespace)
*   Have you generated general-purpose/reusable functions?
*   Have you grouped related functions into separate libraries?
*   Have you included a complete makefile with `clean` and `run` targets?
*   Does your makefile use -Wall -pedantic ?
*   Does the code compile without errors/warnings (e.g. with -Wall -pedantic)?
*   Is there error checking on all system calls, user inputs or source file content?
*   Does your solution take appropriate action if an error occurs (e.g. make every effort to save the situation)?
*   Have you avoided the use of hard-coded literals? (e.g. use #define macros)

**Documentation - 10%**

*   Does your header block contain the author's name, the purpose of the program and a description of how to compile and run the solution.
*   Are identifiers named in a meaningful way?
*   Are any obscure constructs fully explained?
*   Does each function have a header block that explains the purpose, its arguments and return value?
*   Have you recorded a submission script (in the `submit` program) showing your assignment compiling and running?

**Source Formatting - 5%**

*   Is your indentation consistent?
*   Have blank lines been used so that the code is easy to read?
*   Are any lines longer than 80 characters?
*   Are capitalisation and naming conventions consistent?
