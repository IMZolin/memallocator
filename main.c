#pragma once
#pragma warning(disable : 4996)
#include "memallocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#define GetHeadSize(start) *((int*)(start))
#define GetNextBlock(start) *(void**)((int*)start + 1)
#define GetFooterSize(start) *(int*)((char*)start + GetHeadSize(start) - 4)
#define MAX_ALLOCATING_SIZE 1000
#define MAX_ITERATIONS 100
#define TRUE 1
#define FALSE 0

#define TIME_INIT \
LARGE_INTEGER frequency; \
LARGE_INTEGER start; \
LARGE_INTEGER end; \
double elapsedTime; \
QueryPerformanceFrequency(&frequency);

#define TIME_START QueryPerformanceCounter(&start);

#define TIME_FINISH \
QueryPerformanceCounter(&end); \
elapsedTime=(double)(end.QuadPart-start.QuadPart)/frequency.QuadPart;

double mallocTest(int size) {
    TIME_INIT
        void* arr[MAX_ITERATIONS] = { NULL };
    int last_i = 0;
    TIME_START
        for (int i = 0; i < MAX_ITERATIONS; i++) {
            if (rand() % 3 != 0) {
                arr[i] = malloc(size);
                last_i = i;
            }
            else if (arr[last_i] != NULL) {
                free(arr[last_i]);
                arr[last_i] = NULL;
            }
        }
    TIME_FINISH
        for (int i = 0; i < MAX_ITERATIONS; i++) {
            free(arr[i]);
        }
    return elapsedTime;

}

double memallocTest(int size) {
    TIME_INIT
        void* arr[MAX_ITERATIONS] = { NULL };
    int last_i = 0;
    TIME_START
        for (int i = 0; i < MAX_ITERATIONS; i++) {
            if (rand() % 3 != 0) {
                arr[i] = memalloc(size);
                last_i = i;
            }
            else if (arr[last_i] != NULL) {
                memfree(arr[last_i]);
                arr[last_i] = NULL;
            }
        }
    TIME_FINISH
        for (int i = 0; i < MAX_ITERATIONS; i++) {
            memfree(arr[i]);
        }
    return elapsedTime;
}

int main(void) {
    double time[2];
    FILE* f;
    if ((f = fopen("compare.csv", "wb")) == NULL)
        printf("The file 'cmp' was not opened\n");
    int memorySize = MAX_ITERATIONS * (MAX_ALLOCATING_SIZE + memgetblocksize());
    void* ptr = malloc(memorySize);
    if (ptr == NULL) {
        printf("Error allocating memory\n");
        exit(1);
    }
    int init = meminit(ptr, memorySize);
    if (init == 0) {
        printf("Error initialize allocating system\n");
        exit(1);
    }
    fprintf(f, "malloc;   memalloc\n");
    for (int i = 10; i < MAX_ALLOCATING_SIZE; i += 10) {
        time[0] = mallocTest(i);
        time[1] = memallocTest(i);
        fprintf(f, "%lf; %lf\n", time[0], time[1]);
    }
    fclose(f);
    memdone();
    free(ptr);
    return 0;
}