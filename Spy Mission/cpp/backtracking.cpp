#include <iostream>
#include "../Hfiles/gdata.h"
#include "../Hfiles/backtracking.h"

using namespace std;

static bool nodeIsBlocked(int nodeId, bool blockedList[]) {
    return blockedList[nodeId] == true;
}

static bool nodeAlreadyVisited(int nodeId, bool visitedList[]) {
    return visitedList[nodeId] == true;
}

void findAllPaths(int current, int end, bool visited[], bool blocked[], vector<int>& path) {
    if (nodeIsBlocked(current, blocked) == true) return;

    visited[current] = true;
    path.push_back(current);

    if (current == end) {
        cout << "Path: ";
        int printIdx = 0;
        while (printIdx < (int)path.size()) {
            cout << path[printIdx] << " ";
            printIdx = printIdx + 1;
        }
        cout << endl;
    } else {
        int edgeIdx = 0;
        while (edgeIdx < (int)graph[current].size()) {
            int neighborNode = graph[current][edgeIdx].dest_node;
            if (nodeAlreadyVisited(neighborNode, visited) == false) {
                findAllPaths(neighborNode, end, visited, blocked, path);
            }
            edgeIdx = edgeIdx + 1;
        }
    }

    path.pop_back();
    visited[current] = false;
}
