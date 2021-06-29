#include <sys/time.h>
#include <ctime>
#include <iostream>
#include <random>
#include "omp.h"

using namespace std;

void quickSort_parallel(int *array, int lenArray, int numThreads);
void quickSort_parallel_internal(int *array, int left, int right, int cutoff);


void quickSort_parallel(int *array, int lenArray, int numThreads) {
    int cutoff = 1000;
#pragma omp parallel num_threads(numThreads)
    {
#pragma omp single nowait
        { quickSort_parallel_internal(array, 0, lenArray - 1, cutoff); }
    }
}

void quickSort_parallel_internal(int *array, int left, int right, int cutoff) {
    int i = left, j = right;
    int tmp;
    int pivot = array[(left + right) / 2];


    while (i <= j) {
        while (array[i] < pivot) i++;
        while (array[j] > pivot) j--;
        if (i <= j) {
            tmp = array[i];
            array[i] = array[j];
            array[j] = tmp;
            i++;
            j--;
        }
    }

    if (((right - left) < cutoff)) {
        if (left < j) {
            quickSort_parallel_internal(array, left, j, cutoff);
        }
        if (i < right) {
            quickSort_parallel_internal(array, i, right, cutoff);
        }
    } else {
#pragma omp task
        { quickSort_parallel_internal(array, left, j, cutoff); }
#pragma omp task
        { quickSort_parallel_internal(array, i, right, cutoff); }
    }
}

int ARRAY_SIZE = 10000;
int main() {
    int intList[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        intList[i] = ARRAY_SIZE - i;
    }
    int numThreads;
    cin >> numThreads;
    double wtime = omp_get_wtime();
    quickSort_parallel(intList, ARRAY_SIZE, numThreads);
    cout << "Total" << ARRAY_SIZE << ". Print first 10 elements" << endl;
    for (int i = 0; i < 10; ++i) {
        cout << intList[i] << '\t';
    }
    cout<<endl;
    wtime = omp_get_wtime() - wtime;
    cout << "Threads number: " << numThreads << "; Elapsed time: " << wtime << " secs" << endl;
}
