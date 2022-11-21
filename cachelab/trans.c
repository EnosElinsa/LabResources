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
 * (s = 5, E = 1, b = 5)
 * 32 × 32: 8 points if m < 300, 0 points if m > 600
 * 64 × 64: 8 points if m < 1, 300, 0 points if m > 2, 000
 * 61 × 67: 10 points if m < 2, 000, 0 points if m > 3, 000
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, bi, t0, t1, t2, t3, t4, t5, t6, t7;
    if (M == 32 && N == 32) {
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (bi = i; bi < i + 8; bi++) {
                    t0 = A[bi][0 + j];
                    t1 = A[bi][1 + j];
                    t2 = A[bi][2 + j];
                    t3 = A[bi][3 + j];
                    t4 = A[bi][4 + j];
                    t5 = A[bi][5 + j];
                    t6 = A[bi][6 + j];
                    t7 = A[bi][7 + j];
                    B[0 + j][bi] = t0;
                    B[1 + j][bi] = t1;
                    B[2 + j][bi] = t2;
                    B[3 + j][bi] = t3;
                    B[4 + j][bi] = t4;
                    B[5 + j][bi] = t5;
                    B[6 + j][bi] = t6;
                    B[7 + j][bi] = t7;
                }
            }
        }
    } else if (M == 64 && N == 64) {
        int i, j, bi, t0, t1, t2, t3;
        for (i = 0; i < N; i += 4) {
            for (j = 0; j < M; j+= 4) {
                for (bi = i; bi < i + 4; bi++) {
                    t0 = A[bi][0 + j];
                    t1 = A[bi][1 + j];
                    t2 = A[bi][2 + j];
                    t3 = A[bi][3 + j];
                    B[0 + j][bi] = t0;
                    B[1 + j][bi] = t1;
                    B[2 + j][bi] = t2;
                    B[3 + j][bi] = t3;
                }
            }
        }
    } else if (M == 61 && N == 67) {
        int n = N / 8 * 8;
        int m = M / 8 * 8;
        int i, j, t0, t1, t2, t3, t4, t5, t6, t7;
        for (j = 0; j < m; j += 8) {
            for (i = 0; i < n; i++) {
                t0 = A[i][0 + j];
                t1 = A[i][1 + j];
                t2 = A[i][2 + j];
                t3 = A[i][3 + j];
                t4 = A[i][4 + j];
                t5 = A[i][5 + j];
                t6 = A[i][6 + j];
                t7 = A[i][7 + j];
                B[0 + j][i] = t0;
                B[1 + j][i] = t1;
                B[2 + j][i] = t2;
                B[3 + j][i] = t3;
                B[4 + j][i] = t4;
                B[5 + j][i] = t5;
                B[6 + j][i] = t6;
                B[7 + j][i] = t7;
            }
        }

        for (i = n; i < N; i++) {
            for (j = m; j < M; j++) {
                t0 = A[i][j];
                B[j][i] = t0;
            }
        }

        for (i = 0; i < n; i++) {
            for (j = m; j < M; j++) {
                t0 = A[i][j];
                B[j][i] = t0;
            }
        }

        for (i = n; i < N; i++) {
            for (j = 0; j < m; j++) {
                t0 = A[i][j];
                B[j][i] = t0;
            }
        }
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

