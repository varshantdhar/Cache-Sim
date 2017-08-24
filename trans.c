/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (N == 32 && M == 32) {
        int i, j, k, l, counter, diag_bool = 0, tmp;
        for (i = 0; i < N; i += 8) {
            for(j = 0; j < M; j += 8){
                for (k = 0; k < 8; k++) {
                    for (l = 0; l < 8; l++) {
                        if ((i+k) != (j+l))
                            B[j+l][i+k] = A[i+k][j+l];
                        else{
                            tmp = A[i+k][j+l];
                            counter = i+k;
                            diag_bool = 1;
                        }
                    }
                    if (diag_bool == 1) {
                        B[counter][counter] = tmp;
                        diag_bool = 0;
                    }
                }
            }
        }
        
    } else if (N == 64 && M == 32){
        int i, j, k, l, counter, diag_bool = 0, tmp;
        for (i = 0; i < N; i += 8) {
            for(j = 0; j < M; j += 4){
                for (k = 0; k < 8; k++) {
                    for (l = 0; l < 4; l++) {
                        if ((i+k) != (j+l))
                            B[j+l][i+k] = A[i+k][j+l];
                        else{
                            tmp = A[i+k][j+l];
                            counter = i+k;
                            diag_bool = 1;
                        }
                    }
                    if (diag_bool == 1) {
                        B[counter][counter] = tmp;
                        diag_bool = 0;
                    }
                }
            }
        }

        
    } else if(N == 64 && M == 64){
        int i, j, val0, val1, val2, val3;
        for (i = 0; i < N; i += 4) {
            for(j = 0; j < M; j += 4){
                val0 = A[i][j];
                val1 = A[i+1][j];
                val2 = A[i+2][j];
                val3 = A[i+3][j];
                
                B[j][i] = val0;
                B[j][i+1] = val1;
                B[j][i+2] = val2;
                B[j][i+3] = val3;
                
                val0 = A[i][j+1];
                val1 = A[i+1][j+1];
                val2 = A[i+2][j+1];
                val3 = A[i+3][j+1];
                
                B[j+1][i] = val0;
                B[j+1][i+1] = val1;
                B[j+1][i+2] = val2;
                B[j+1][i+3] = val3;
                
                val0 = A[i][j+2];
                val1 = A[i+1][j+2];
                val2 = A[i+2][j+2];
                val3 = A[i+3][j+2];
                
                B[j+2][i] = val0;
                B[j+2][i+1] = val1;
                B[j+2][i+2] = val2;
                B[j+2][i+3] = val3;
                
                val0 = A[i][j+3];
                val1 = A[i+1][j+3];
                val2 = A[i+2][j+3];
                val3 = A[i+3][j+3];
                
                B[j+3][i] = val0;
                B[j+3][i+1] = val1;
                B[j+3][i+2] = val2;
                B[j+3][i+3] = val3;
                
            }
        }
        
    } else if (N == 67 && M == 61){
        int i, j, k, l, counter, diag_bool = 0, tmp;
        for (i = 0; i < N; i += 16) {
            for(j = 0; j < M; j += 16){
                for (k = 0; k < 16; k++) {
                    for (l = 0; l < 16; l++) {
                    if ((i+k) > 66 || (j+l) > 60)
                        break;
                    if ((i+k) != (j+l))
                        B[j+l][i+k] = A[i+k][j+l];
                    else{
                        tmp = A[i+k][j+l];
                        counter = i+k;
                        diag_bool = 1;
                    }
                }
                if (diag_bool == 1) {
                    B[counter][counter] = tmp;
                    diag_bool = 0;
                }
                }
            }
        }
        
    } else {
        fprintf(stderr, "Invalid Parameters\n");
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

