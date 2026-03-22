#include <iostream>
#include "../Hfiles/gdata.h"
#include "../Hfiles/backtracking.h"

using namespace std;

void findAllPaths(int current, int end, bool visited[], bool blocked[], vector<int>& path) {
    if(blocked[current]) return;

    visited[current] = true;
    path.push_back(current);

    if(current == end) {
        cout << "Path: ";
        for(int x : path)
            cout << x << " ";
        cout << endl;
    }
    else {
        for(auto edge : graph[current]) {
            if(!visited[edge.dest_node])
                findAllPaths(edge.dest_node, end, visited, blocked, path);
        }
    }

    path.pop_back();
    visited[current] = false;
}