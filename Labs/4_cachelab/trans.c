/* 
 * trans.c - Matrib_size transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
*/ 

#include <stdio.h>
#include <stdlib.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
*/

#define min(a, b) (a < b ? a : b)
#define equal(a, b, c) (a == c && b == c ? 1 : 0)

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int temp;

    int b_size;                             // Blocking size
    if (equal(M, N, 32)) b_size = 8;        // every 8 rows to repeat set index to evict
    else if (equal(M, N, 64)) b_size = 4;   // every 4 rows to repeat set index to evict
    else b_size = 16;                       /* actually works as 8*16 Blockings since the cache's block size is 8,
                                             * and two 8*16 Blockings exactly fit in the cache (32 blocks). */
    for (int R=0; R<N; R+=b_size){
        for (int C=0; C<M; C+=b_size){
            for (int r=R; r<min(R+b_size, N); r++){
                if (M==N){                  // avoid diagonal eviction
                    for (int c=C; c<min(C+b_size, M); c++){
                        if (c != r) B[c][r] = A[r][c];
                        else temp = A[r][c];
                    }
                    if (R==C) B[r][r] = temp;
                }
                else{
                    for (int c=C; c<min(C+b_size, M); c++) B[c][r] = A[r][c];
                }
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

char trans_1_desc[] = "Transpose 8*8";
void trans_1(int M, int N, int A[N][M], int B[M][N])
{
    int col = 8;
    int row = 8;
    int Col = M / col;
    int Row = N / row;

    int temp;

    for (int I=0; I < Col; I++){
        for (int J = 0; J < Row; J++){
            for (int j=0; j<row; j++){
                for (int i=0; i<col; i++){
                    if (i != j) B[I*col+i][J*row+j] = A[J*row+j][I*col+i];
                    else temp = A[J*row+j][I*col+i];
                }
                B[I*col+j][J*row+j] = temp;
            }
        }
    }
}

char trans_2_desc[] = "Transpose 16*8";
void trans_2(int M, int N, int A[N][M], int B[M][N])
{
    int col = 16;
    int row = 8;
    int Col = M / col;
    int Row = N / row;

    int temp;

    for (int I=0; I < Col; I++){
        for (int J = 0; J < Row; J++){
            for (int j=0; j<row; j++){
                for (int i=0; i<col; i++){
                    temp = A[J*row+j][I*col+i];
                    B[I*col+i][J*row+j] = temp;
                }
            }
        }
    }
}

char trans_3_desc[] = "Transpose 4*4";
void trans_3(int M, int N, int A[N][M], int B[M][N])
{
    int b_size = 4;
    int temp;
    for (int R=0; R<N; R+=b_size){
        for (int C=0; C<M; C+=b_size){
            for (int r=R; r<min(R+b_size, N); r++){
                for (int c=C; c<min(C+b_size, M); c++){
                    if (c != r) B[c][r] = A[r][c];
                    else temp = A[r][c];
                }
                if (R==C) B[r][r] = temp;
            }
        }
    }
}

char trans_4_desc[] = "Transpose 8*8 2";
void trans_4(int M, int N, int A[N][M], int B[M][N])
{
    int b = 8;
    int temp;
    // int* a = A[0][0]
    // int* b = B[0][0]
    for (int R=0; R<N; R+=b){
        for (int C=0; C<M; C+=b){
            for (int r=R; r<min(R+b, N); r++){
                for (int c=C; c<min(C+b, M); c++){
                    if (c!=r) *(*(B+c)+r) = *(*(A+r)+c);
                    else temp = *(*(A+r)+c);
                    // if (c != r) B[c][r] = A[r][c];
                    // else temp = A[r][c];
                }
                if (C==R) *(*(B+r)+r) = temp;
                // if (R==C) B[r][r] = temp;
            }
        }
    }
}

char trans_5_desc[] = "Transpose 8*8 3";
void trans_5(int M, int N, int A[N][M], int B[M][N])
{
    int b_size = 8;
    int temp;
    int* a = &A[0][0];
    int* b = &B[0][0];

    for (int R=0; R<N; R+=b_size){
        for (int C=0; C<M; C+=b_size){
            for (int r=R; r<min(R+b_size, N); r++){
                for (int c=C; c<min(C+b_size, M); c++){
                    if (c!=r) *(b+M*c+r) = *(a+N*r+c);
                    else temp = *(a+N*r+c);
                    // if (c != r) B[c][r] = A[r][c];
                    // else temp = A[r][c];
                }
                if (C==R) *(b+M*r+r) = temp;
                // if (R==C) B[r][r] = temp;
            }
        }
    }
}

char trans_6_desc[] = "Transpose 8*8 4";
void trans_6(int M, int N, int A[N][M], int B[M][N])
{
    int b_size = 8;
    int temp;
    for (int R=0; R<N; R+=b_size){
        for (int C=0; C<M; C+=b_size){
            for (int r=R; r<min(R+b_size, N); r++){
                for (int c=C; c<min(C+b_size, M); c++){
                    if (c != r) B[c][r] = A[r][c];
                    else temp = A[r][c];
                }
                if (R==C) B[r][r] = temp;
            }
        }
    }
}

char trans_7_desc[] = "Transpose 8*4";
void trans_7(int M, int N, int A[N][M], int B[M][N])
{
    int b_size = 8; // block size
    int b_half = 4; // half block size
    int i, R, C, r, c;

    for (i=0; i<min(N, M); i++){
        B[i][i] = A[i][i];
    }

    for (R=0; R<N; R+=b_size){
        for (C=0; C<M; C+=b_size){
            // A's first half rows
            for (r=R; r<min(R+b_half, N); r++){
                // copy to B for the first half rows
                for (c=C; c<min(C+b_half, M); c++) if (c != r) B[c][r] = A[r][c];
            }
            for (r=R; r<min(R+b_half, N); r++){
                // copy to B for the second half rows
                for (c=min(C+b_half, M); c<min(C+b_size, M); c++) if (c != r) B[c][r] = A[r][c];
            }
            // A's second half rows
            for (r=min(R+b_half, N); r<min(R+b_size, N); r++){
                // copy to B for the second half rows
                for (c=min(C+b_half, M); c<min(C+b_size, M); c++) if (c != r) B[c][r] = A[r][c];
            }
            for (r=min(R+b_half, N); r<min(R+b_size, N); r++){
                // copy to B for the first half rows
                for (c=C; c<min(C+b_half, M); c++) if (c != r) B[c][r] = A[r][c];
            }
        }
    }
}

char trans_8_desc[] = "Transpose 16*16";
void trans_8(int M, int N, int A[N][M], int B[M][N])
{
    int b_size = 16;
    for (int R=0; R<N; R+=b_size){
        for (int C=0; C<M; C+=b_size){
            for (int r=R; r<min(R+b_size, N); r++){
                for (int c=C; c<min(C+b_size, M); c++){
                    B[c][r] = A[r][c];
                }
            }
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to eb_sizeperiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(trans_3, trans_3_desc);
    registerTransFunction(trans_8, trans_8_desc);
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
