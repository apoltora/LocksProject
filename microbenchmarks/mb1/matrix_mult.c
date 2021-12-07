/**
 * This completes matrix multiplication
 * Using example from google image:
 * https://towardsdatascience.com/a-complete-beginners-guide-to-matrix-multiplication-for-data-science-with-python-numpy-9274ecfc1dc6
 * 
 * Authors: Alexandra Poltorak, Kiran Kumar Rajan Babu
 * Contact: apoltora@andrew.cmu.edu, krajanba@andrew.cmu.edu
 *
 */

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#define A_ROWS 2
#define A_COLS 3
#define B_ROWS 3
#define B_COLS 2

/*
 * A            - matrix 1
 * B            - matrix 2
 * rows_A       - # of rows in A
 * rows_B       - # of rows in B
 * cols_A       - # of cols in A
 * cols_B       - # of cols in B
 *
 * This function does matrix multiplication between matrix A and matrix B
 *
 * A = [ a b c d                B = [ m n o                     C = [ a*m + b*p + c*s + d*v     etc...]
 *       e f g h        x             p q r         =
 *       i j k l ]                    s t u
 *                                    v w x ]
 *
 *
 *
 * return NULL if cannot do matrix multiplication with the input matrices
 * return resulting matrix of doing AxB otherwise
 */
int *matrix_multiplication(int *A, int *B, int rows_A, int cols_A, int rows_B, int cols_B) {
    
    int length_A = rows_A * cols_A;
    int length_B = rows_B * cols_B;

    int *C = malloc(rows_A * cols_B * sizeof(int));
    
    // you cannot do matrix multiplication for matrices that are not the same size !!
    if (length_A != length_B || cols_A != rows_B) {
        return NULL;
    }

    int i,j,k;
    int A_index, B_index, C_index;
  
    for (i = 0; i < rows_A; i++) {
        for (j = 0; j < cols_B; j++) {
            // i * cols_A + j
            // we want row i col j for C
            C_index = i * cols_B + j;

            // C[i][j] = 0;
            C[C_index] = 0;
 
            for (k = 0; k < rows_B; k++) {
                A_index = i * cols_A + k;
                B_index = k * cols_B + j;
                // C[i][j] += A[i][k] * B[k][j];
                C[C_index] += A[A_index] * B[B_index];
            }
         }
     }

    return C;
}

void print_array(int *array, int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("array[%d] = %d\n", i, array[i]);
    }
}

int main() {

    // initialize the arrays here
    int A[A_ROWS][A_COLS];
    int B[B_ROWS][B_COLS];

    A[0][0] = 1; A[0][1] = 2; A[0][2] = 3;
    A[1][0] = 4; A[1][1] = 5; A[1][2] = 6;

    B[0][0] = 10; B[0][1] = 11;
    B[1][0] = 20; B[1][1] = 21;
    B[2][0] = 30; B[2][1] = 31;

    // calling for matrix multiplication to happen
    int *C = matrix_multiplication(A,B,A_ROWS,A_COLS,B_ROWS,B_COLS);

    // check to see if result is what we expect it to be
    print_array(C,A_ROWS*B_COLS);

    // successfully ran main
    return 0;
}
