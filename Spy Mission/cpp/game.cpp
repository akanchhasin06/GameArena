#include <iostream>
#include "../Hfiles/gdata.h"
#include "../Hfiles/dijkstra.h"
#include "../Hfiles/backtracking.h"

using namespace std;

vector<Edge> graph[n] = {
    {{1,2}, {3,4}},       
    {{0,2}, {2,3}, {4,5}}, 
    {{1,3}, {5,4}},        
    {{0,4}, {4,2}},       
    {{1,5}, {3,2}, {5,1}}, 
    {{2,4}, {4,1}}        
};

string roomNames[n] = {
    "Entry","Corridor","Camera Room","Guard Hall","Vault","Exit"
};

bool blocked[n] = {false,false,false,false,false,false};

int main() {
    int current = 0;
    int turn = 0;
    int guardPos = 3;

    blocked[guardPos] = true;

    while(current != 5) {
        turn++;
        cout << "You are at: " << roomNames[current] << endl;
        if(turn % 2 == 0)
            blocked[2] = true;
        else
            blocked[2] = false;

       
        blocked[guardPos] = false;
        guardPos = (guardPos + 1) % n;

        if(guardPos != 0 && guardPos != 5)
            blocked[guardPos] = true;

        cout << "\nAvailable moves:\n";

        for(auto edge : graph[current]) {
            if(!blocked[edge.dest_node]) {
                cout << edge.dest_node << " -> " << roomNames[edge.dest_node]
                     << " (Danger " << edge.weight << ")\n";
            }
        }
        bool visited[n] = {false};
        vector<int> path;

        cout << "\nPossible safe paths:\n";
        findAllPaths(current,5,visited,blocked,path);

        
        vector<int> safePath = dijkstra(current,5,blocked);

        cout << "\nSafest remaining path: ";
        for(int x : safePath)
            cout << x << " ";
        cout << endl;

        int choice;
        cout << "\nChoose next room: ";
        cin >> choice;

        bool valid = false;

        for(auto edge : graph[current]) {
            if(edge.dest_node == choice && !blocked[choice]) {
                current = choice;
                valid = true;
                break;
            }
        }

        if(!valid) {
            cout << "Invalid move or blocked room!\n";
            continue;
        }

        if(current == 4) {
            cout << "\nVault reached! Alarm activated!\n";

            for(auto &edge : graph[1]) {
                if(edge.dest_node == 4)
                    edge.weight += 5;
            }
        }

        cout << "\nBlocked rooms: ";
        for(int i=0;i<n;i++) {
            if(blocked[i])
                cout << roomNames[i] << " ";
        }

        cout << endl;
    }

    cout << "\nMission Successfull.\n";

    return 0;
}