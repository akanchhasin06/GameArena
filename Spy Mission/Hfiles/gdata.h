#ifndef graph_data_H
#define graph_data_H

#include <vector>
#include <string>
using namespace std;

struct Edge {
    int dest_node;
    int weight;
};

const int n = 6;

extern vector<Edge> graph[n];
extern string roomNames[n];

#endif