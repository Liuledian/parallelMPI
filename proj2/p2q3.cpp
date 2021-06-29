#include <iostream>
#include <random>
#include "omp.h"

using namespace std;

const int NODES_NUM = 100000;
const int ITER_NUM = 100;
const double D = 0.85;

class Node {
public:
    Node** nodes;
    double weight;
    int size;
    double sum;
    int index;

    Node() {
        this->weight = 1;
        this->sum = 0;
        this->size = 0;
        this->nodes = NULL;
        this->index = -1;
    }

    Node(int index) {
        this->weight = 1;
        this->sum = 0;
        this->size = 0;
        this->nodes = NULL;
        this->index = index;
    }
};

int main() {
    Node* graph = new Node[NODES_NUM];
    for (int i = 0; i < NODES_NUM; ++i) {
        graph[i] = Node(i);
    }

    default_random_engine e;
    uniform_int_distribution<int> link_nodes_num(1, 10);
    uniform_int_distribution<int> link_nodes(0, NODES_NUM - 1);

    int n_threads;
    cin >> n_threads;
    omp_set_num_threads(n_threads);

    double wtime = omp_get_wtime();

    for (int i = 0; i < NODES_NUM; ++i) {
        Node* sourceNode = &graph[i];
        sourceNode->size = link_nodes_num(e);
        sourceNode->nodes = new Node*[sourceNode->size];

        int j = 0;
        int* selected_nodes_index = new int[sourceNode->size];
        while (j < sourceNode->size) {
            int generated_node_index = link_nodes(e);
            bool new_flag = true;
            for (int k = 0; k < j; ++k)
                if (generated_node_index == selected_nodes_index[k] ||
                    generated_node_index == sourceNode->index) {
                    new_flag = false;
                    break;
                }

            if (new_flag) {
                selected_nodes_index[j] = generated_node_index;
                j += 1;
            }
        }

        for (int k = 0; k < sourceNode->size; ++k)
            sourceNode->nodes[k] = &graph[selected_nodes_index[k]];
    }

    for (int iter_num = 0; iter_num < ITER_NUM; ++iter_num) {
#pragma omp parallel for
        for (int i = 0; i < NODES_NUM; ++i) {
            Node* sourceNode = &graph[i];
            for (int j = 0; j < sourceNode->size; ++j) {
                Node* targetNode = sourceNode->nodes[j];
                targetNode->sum +=
                        sourceNode->weight / double(sourceNode->size) * D;
            }
        }

#pragma omp parallel for
        for (int i = 0; i < NODES_NUM; ++i) {
            Node* sourceNode = &graph[i];
            sourceNode->sum += (1 - D) / double(NODES_NUM);
        }

#pragma omp parallel for
        for (int i = 0; i < NODES_NUM; ++i) {
            Node* sourceNode = &graph[i];
            sourceNode->weight = sourceNode->sum;
            sourceNode->sum = 0;
        }
    }


    cout<< "Print the page rank of first ten nodes\n";
    for (int i = 0; i < 10; ++i) {
        cout << graph[i].weight << '\t';
    }
    cout << endl;

    wtime = omp_get_wtime() - wtime;
    cout << "Node number: " << NODES_NUM << "; Iteration number: " << ITER_NUM << endl;
    cout << "Threads number: " << n_threads << "; Elapsed time: " << wtime << " secs\n";
}