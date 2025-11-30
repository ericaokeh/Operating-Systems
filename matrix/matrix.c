/*
 * Lab 7: Programming using Threads
 * Parallel Matrix Arithmetic with pThreads
 *
 * - Matrices are MAX x MAX (20 x 20)
 * - Use 10 threads to compute:
 *     A + B, A - B, and A x B
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX          20     /* 20 x 20 matrix */
#define THREAD_COUNT 10

/* Actual matrix dimension (fixed to MAX for this lab) */
int N = MAX;

/* Global matrices */
int matA[MAX][MAX];
int matB[MAX][MAX];
int matSum[MAX][MAX];
int matDiff[MAX][MAX];
int matProd[MAX][MAX];

/* Data passed to each thread: which rows to work on */
typedef struct {
    int startRow;  /* inclusive */
    int endRow;    /* inclusive */
} ThreadData;

/* --------- Utility functions --------- */

/* Fill matrices A and B with random values */
void fillMatrices(void)
{
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            matA[i][j] = rand() % 10;  /* 0–9 */
            matB[i][j] = rand() % 10;  /* 0–9 */
        }
    }
}

/* Print an N x N matrix with a label */
void printMatrix(const char *label, int m[MAX][MAX])
{
    int i, j;
    printf("%s:\n", label);
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("%4d ", m[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* --------- Thread functions --------- */

/* Thread function to compute A + B into matSum for a range of rows */
void* computeSum(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int i, j;

    if (data->startRow > data->endRow) return NULL;  /* no work */

    for (i = data->startRow; i <= data->endRow; i++) {
        for (j = 0; j < N; j++) {
            matSum[i][j] = matA[i][j] + matB[i][j];
        }
    }
    return NULL;
}

/* Thread function to compute A - B into matDiff for a range of rows */
void* computeDiff(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int i, j;

    if (data->startRow > data->endRow) return NULL;  /* no work */

    for (i = data->startRow; i <= data->endRow; i++) {
        for (j = 0; j < N; j++) {
            matDiff[i][j] = matA[i][j] - matB[i][j];
        }
    }
    return NULL;
}

/* Thread function to compute A x B into matProd for a range of rows */
void* computeProduct(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int i, j, k;

    if (data->startRow > data->endRow) return NULL;  /* no work */

    for (i = data->startRow; i <= data->endRow; i++) {
        for (j = 0; j < N; j++) {
            int sum = 0;
            for (k = 0; k < N; k++) {
                sum += matA[i][k] * matB[k][j];
            }
            matProd[i][j] = sum;
        }
    }
    return NULL;
}

/* Helper to run one of the compute* functions using THREAD_COUNT threads */
void run_threads(void *(*threadFunc)(void *))
{
    pthread_t  threads[THREAD_COUNT];
    ThreadData tdata[THREAD_COUNT];

    int baseRows = N / THREAD_COUNT;  /* rows per thread */
    int extra    = N % THREAD_COUNT;  /* leftover rows     */
    int current  = 0;

    /* Create threads with their row ranges */
    for (int t = 0; t < THREAD_COUNT; t++) {
        int rows = baseRows + (t < extra ? 1 : 0);
        tdata[t].startRow = current;
        tdata[t].endRow   = current + rows - 1;
        current += rows;

        /* If no rows, mark as an empty range: start > end */
        if (rows <= 0) {
            tdata[t].startRow = 0;
            tdata[t].endRow   = -1;   /* clearly "no work" */
        }

        pthread_create(&threads[t], NULL, threadFunc, &tdata[t]);
    }

    /* Wait for all threads to finish */
    for (int t = 0; t < THREAD_COUNT; t++) {
        pthread_join(threads[t], NULL);
    }
}

/* --------- main --------- */

int main(int argc, char *argv[])
{
    /* Seed RNG once */
    srand((unsigned int)time(NULL));

    /* For this lab N is fixed at MAX=20, already set globally */

    /* Fill input matrices with random values */
    fillMatrices();

    /* Show input matrices */
    printMatrix("Matrix A", matA);
    printMatrix("Matrix B", matB);

    /* Compute A + B using threads */
    run_threads(computeSum);
    printMatrix("A + B", matSum);

    /* Compute A - B using threads */
    run_threads(computeDiff);
    printMatrix("A - B", matDiff);

    /* Compute A x B using threads */
    run_threads(computeProduct);
    printMatrix("A x B", matProd);

    return 0;
}
