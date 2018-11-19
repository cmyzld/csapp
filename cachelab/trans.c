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
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */

void transpose_32(int M, int N, int A[N][M], int B[M][N])
{
	int temp = 0, d = 0;
	int b = 8;
	for(int bi = 0; bi < N; bi += b)
		for(int bj = 0; bj < N; bj += b)
			for(int r = bj; r < bj + b; r++)
			{	
				for(int c = bi; c < bi + b; c++)
				{
					if(r != c)
						B[c][r] = A[r][c];
					else
					{
						temp = A[r][c];
						d = r;
					}
				}
				if(bi == bj)
					B[d][d] = temp;
			}
}

void transpose_64(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, k, l;
	int t0, t1, t2, t3, t4, t5, t6, t7;
	for(i = 0; i < N; i += 8)
	{
		for(j = 0; j < N; j += 8)
		{
			for(k = i; k < i + 4; k++)
			{
				t0 = A[k][j];
				t1 = A[k][j + 1];
				t2 = A[k][j + 2];
				t3 = A[k][j+3];
				t4 = A[k][j+4];
				t5 = A[k][j+5];
				t6 = A[k][j + 6];
				t7 = A[k][j + 7];
				B[j][k] = t0;
				B[j+1][k] = t1;
				B[j+2][k] = t2;
				B[j+3][k] = t3;
				B[j][k+4] = t4;
				B[j+1][k+4] = t5;
				B[j+2][k+4] = t6;
				B[j+3][k+4] = t7;
			}
			for(k = j; k < j + 4; k++)
			{
				t4 = A[i+4][k];
				t5 = A[i+5][k];
				t6 = A[i+6][k];
				t7 = A[i+7][k];
				t0 = B[k][i+4];
				t1 = B[k][i+5];
				t2 = B[k][i+6];
				t3 = B[k][i+7];
				B[k][i+4] = t4;
				B[k][i+5] = t5;
				B[k][i+6] = t6;
				B[k][i+7] = t7;
				B[k+4][i] = t0;
				B[k+4][i+1] = t1;
				B[k+4][i+2] = t2;
				B[k+4][i+3] = t3;
				for(l = 0; l < 4; l++)
					B[k+4][i+l+4] = A[i+l+4][k+4];
			}
		}
	}
}

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    REQUIRES(M > 0);
    REQUIRES(N > 0);
	if(N == 32)
		transpose_32(M, N, A, B);
    else
		transpose_64(M, N, A, B);
	ENSURES(is_transpose(M, N, A, B));
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

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

    ENSURES(is_transpose(M, N, A, B));
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

