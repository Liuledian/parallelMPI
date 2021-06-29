cmake_minimum_required(VERSION 3.17)
project(parallel C)

set(CMAKE_C_STANDARD 99)
include_directories(/usr/local/include)
add_executable(parallel consecutive.c sieve.c pi.c mv2.c gen-double-matrix.c gen-vector.c mybcast.c life.c MyMPI.h
        MyMPI.c writeBinaryFile.c 8-6.c 8-12.c 9-7.c 9-10.c project1/p1q1.c project1/p1q2-1.c project1/p1q2-2.c project1/p1q2-3.c project2/p2q1.cpp project2/p2q2.cpp project2/p2q3.cpp project1/p1q3.cpp)