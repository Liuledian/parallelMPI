#include <cmath>
#include <iostream>
#include <random>
#include <omp.h>

using namespace std;
int TEST_NUM = 100000;

int main() {
    int n_threads;
    cin >> n_threads;
    omp_set_num_threads(n_threads);
    int in_circle = 0;
    double wtime = omp_get_wtime();
#pragma omp parallel for reduction(+ : in_circle)
    for (int i = 0; i < TEST_NUM; ++i) {
        double x = double(rand()) / RAND_MAX;
        double y = double(rand()) / RAND_MAX;
        double distance = x * x + y * y;
        if (distance < 1) {
            in_circle += 1;
        }
    }
    wtime = omp_get_wtime() - wtime;
    cout << "Threads: " << n_threads << "; Elapsed time: " << wtime << " secs"<<endl;
    cout << "Pi value: " << in_circle / double(TEST_NUM) * 4 << endl;
}
