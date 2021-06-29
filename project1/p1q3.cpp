#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <mpi.h>
#include <assert.h>
#include <math.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

using namespace std;

int read_all_files(vector<string> &lines, string path)
{
    ifstream infile;
    infile.open(path.data());
    string s;
    int n_rows = 0;
    while (getline(infile, s)) {
        lines.push_back(s);
        s.clear();
        n_rows += 1;
    }
    return n_rows;
}

int read_big_file()
{
    int rank, size;
    MPI_Init(NULL, NULL);
    MPI_Barrier(MPI_COMM_WORLD);
    double w_time = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;
    MPI_Request request;
    string path = "./big_file/big_100.txt";
    int total_rows;
    vector<string> tmp;
    total_rows = read_all_files(tmp, path);
    int start_rows, end_rows;
    string stmp;
    map<string, int>::iterator iter;
    map<string, int> tmp_store, final_store;
    start_rows = (int(total_rows/size) + 1) * rank;
    end_rows = min((int(total_rows/size) + 1 )* (rank + 1), total_rows);
    int tmp_num;

    for (int i = start_rows; i<end_rows; i += 1)
    {
        istringstream istr(tmp[i]);
        istr >> stmp;
        while (!istr.eof())
        {
            transform(stmp.begin(), stmp.end(), stmp.begin(), ::tolower);
            while (stmp.size() >0 && !((stmp[stmp.size()-1]>='a') && (stmp[stmp.size()-1]<='z')))
            {
                stmp.pop_back();
            }
            while (stmp.size()>0 && !isalpha(stmp[0]))
            {
                stmp.erase(0);
            }
            if (stmp.size() == 0){
                istr >> stmp;
                continue;
            }
            iter = tmp_store.find(stmp);
            if ( iter!= tmp_store.end())
            {
                tmp_store[stmp] += 1;
            }
            else
            {
                tmp_store.insert(pair<string, int>(stmp, 1));
            }
            istr >> stmp;
        }
    }
    if (rank == 0)
    {
        int word_nums;
        MPI_Status status;

        int tmp_number;
        for (int i = 1;i<size; i+=1)
        {
            MPI_Recv(&word_nums, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
            int *len = (int *) malloc(sizeof(int) * word_nums);
            MPI_Recv(len, word_nums, MPI_INT, i, 2, MPI_COMM_WORLD, &status);

            for (int j=0;j<word_nums;j+=1)
            {
                char *tmp_char = (char*)malloc(sizeof(char) * len[j]);
                MPI_Recv(tmp_char, len[j], MPI_CHAR, i, 3, MPI_COMM_WORLD, &status);
                MPI_Recv(&tmp_number, 1, MPI_INT, i, 4, MPI_COMM_WORLD, &status);
                tmp_char[len[j]] = '\0';

                string tmp_str = tmp_char;
                transform(tmp_str.begin(), tmp_str.end(), tmp_str.begin(), ::tolower);
                while (tmp_str.size() >0 && !((tmp_str[tmp_str.size()-1]>='a') && (tmp_str[tmp_str.size()-1]<='z')))
                {
                    tmp_str.pop_back();
                }
                if (tmp_str.size() == 0){
                    continue;
                }
                iter = tmp_store.find(tmp_str);
                if ( iter!= tmp_store.end())
                {
                    tmp_store[tmp_str] += tmp_number;
                }
                else
                {
                    tmp_store.insert(pair<string, int>(tmp_str, tmp_number));
                }

            }
        }

        iter = tmp_store.begin();
        ofstream out_file;
        out_file.open ("result-big.txt");
        while (iter != tmp_store.end())
        {
            out_file << iter->first << " : " << iter->second << endl;
            iter++;
        }
        out_file.close();
    }
    else
    {
        int word_nums = tmp_store.size();
        int *len;
        len = new int[word_nums];
        MPI_Send(&word_nums, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

        int index = 0;
        iter = tmp_store.begin();
        while (iter != tmp_store.end())
        {
            len[index] = (iter->first).size();
            iter++;
            index += 1;
        }
        MPI_Send(len, word_nums, MPI_INT, 0, 2, MPI_COMM_WORLD);
        int j = 0;
        iter = tmp_store.begin();
        while (iter != tmp_store.end())
        {
            char* tmp_char = (char*)(iter->first).c_str();
            tmp_char[len[j]] = '\0';
            int tmp_number = iter->second;
            MPI_Send(tmp_char, len[j], MPI_CHAR, 0, 3, MPI_COMM_WORLD);
            MPI_Send(&tmp_number, 1, MPI_INT, 0, 4, MPI_COMM_WORLD);
            iter++;
            j += 1;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        w_time = MPI_Wtime() - w_time;
        cout << "Processes: " << size << "; Elapsed time: " << w_time <<endl;
    }
    MPI_Finalize();
    return 1;
}

int read_small_files()
{
    int rank, size;
    MPI_Init(NULL, NULL);
    MPI_Barrier(MPI_COMM_WORLD);
    double w_time = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;
    vector<string> files;
    string filePath = "./small_file";
    for (int i=100;i<200;i+=1)
    {
        string name = filePath + "/small_" + to_string(i) + ".txt";
        files.push_back(name);
    }
    int total_file_num = files.size();
    int start_num = (int(total_file_num/size) +1)*rank;
    int end_num =   (int(total_file_num/size) +1)*(rank+1);
    vector<string> tmp;
    for (int i = start_num;i<end_num;i+=1)
    {
        read_all_files(tmp, files[i]);
    }
    string stmp;
    map<string, int>::iterator iter;
    map<string, int> tmp_store;
    for (int i=0;i<tmp.size();i+=1)
    {
        istringstream istr(tmp[i]);
        istr >> stmp;
        while (!istr.eof())
        {
            transform(stmp.begin(), stmp.end(), stmp.begin(), ::tolower);
            while (stmp.size() >0 && !((stmp[stmp.size()-1]>='a') && (stmp[stmp.size()-1]<='z')))
            {
                stmp.pop_back();
            }
            while (stmp.size()>0 && !isalpha(stmp[0]))
            {
                stmp.erase(0);
            }
            if (stmp.size() == 0){istr >> stmp;
                continue;}
            iter = tmp_store.find(stmp);
            if ( iter!= tmp_store.end())
            {
                tmp_store[stmp] += 1;
            }
            else
            {
                tmp_store.insert(pair<string, int>(stmp, 1));
            }
            istr >> stmp;
        }
    }
    if (rank == 0)
    {
        int word_nums;
        MPI_Status status;


        int tmp_number;
        for (int i = 1;i<size; i+=1)
        {
            MPI_Recv(&word_nums, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);

            int *len = (int *) malloc(sizeof(int) * word_nums);
            MPI_Recv(len, word_nums, MPI_INT, i, 2, MPI_COMM_WORLD, &status);

            for (int j=0;j<word_nums;j+=1)
            {
                char *tmp_char = (char*)malloc(sizeof(char) * (len[j]+1));
                MPI_Recv(tmp_char, len[j], MPI_CHAR, i, 3, MPI_COMM_WORLD, &status);
                MPI_Recv(&tmp_number, 1, MPI_INT, i, 4, MPI_COMM_WORLD, &status);
                tmp_char[len[j]] = '\0';
                string tmp_str = tmp_char;

                transform(tmp_str.begin(), tmp_str.end(), tmp_str.begin(), ::tolower);
                while (tmp_str.size() >0 && !((tmp_str[tmp_str.size()-1]>='a') && (tmp_str[tmp_str.size()-1]<='z')))
                {
                    tmp_str.pop_back();
                }
                if (tmp_str.size() == 0){
                    continue;}
                iter = tmp_store.find(tmp_str);
                if ( iter!= tmp_store.end())
                {
                    tmp_store[tmp_str] += tmp_number;
                }
                else
                {
                    tmp_store.insert(pair<string, int>(tmp_str, tmp_number));
                }

            }
        }

        iter = tmp_store.begin();
        ofstream out_file;
        out_file.open ("result-small.txt");
        while (iter != tmp_store.end())
        {
            out_file << iter->first << " : " << iter->second << endl;
            iter++;
        }
        out_file.close();
    }
    else
    {
        int word_nums = tmp_store.size();
        int *len;
        len = new int[word_nums];
        MPI_Send(&word_nums, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

        int index = 0;
        iter = tmp_store.begin();
        while (iter != tmp_store.end())
        {
            len[index] = (iter->first).size();
            iter++;
            index += 1;
        }
        MPI_Send(len, word_nums, MPI_INT, 0, 2, MPI_COMM_WORLD);
        int j = 0;
        iter = tmp_store.begin();
        while (iter != tmp_store.end())
        {
            char* tmp_char = (char*)(iter->first).c_str();
            tmp_char[len[j]] = '\0';
            int tmp_number = iter->second;
            MPI_Send(tmp_char, len[j], MPI_CHAR, 0, 3, MPI_COMM_WORLD);
            MPI_Send(&tmp_number, 1, MPI_INT, 0, 4, MPI_COMM_WORLD);
            iter++;
            j += 1;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        w_time = MPI_Wtime() - w_time;
        cout << "Processes: " << size << "; Elapsed time: " << w_time <<endl;
    }
    MPI_Finalize();
    return 1;
}

int main(int argc, char** argv)
{
    string t = argv[1];
    if (t == "big") {
        cout << "Start processing the big file\n";
        read_big_file();
    } else if (t == "small") {
        cout << "Start processing small files\n";
        read_small_files();
    }
}