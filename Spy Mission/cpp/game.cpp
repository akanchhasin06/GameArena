#include <iostream>
#include "../Hfiles/gdata.h"
#include "../Hfiles/dijkstra.h"
#include "../Hfiles/backtracking.h"

using namespace std;

vector<Edge> graph[n] = {
    {{1, 2}, {3, 4}},
    {{0, 2}, {2, 3}, {4, 5}},
    {{1, 3}, {5, 4}},
    {{0, 4}, {4, 2}},
    {{1, 5}, {3, 2}, {5, 1}},
    {{2, 4}, {4, 1}}
};

string roomNames[n] = {
    "Entry", "Corridor", "Camera Room", "Guard Hall", "Vault", "Exit"
};

bool blocked[n] = {false, false, false, false, false, false};

static bool roomIsBlocked(int roomId) {
    return blocked[roomId] == true;
}

static bool moveIsValid(int fromRoom, int toRoom) {
    int edgeIdx = 0;
    while (edgeIdx < (int)graph[fromRoom].size()) {
        bool destinationMatches = graph[fromRoom][edgeIdx].dest_node == toRoom;
        bool destinationClear = roomIsBlocked(toRoom) == false;
        if (destinationMatches == true && destinationClear == true) {
            return true;
        }
        edgeIdx = edgeIdx + 1;
    }
    return false;
}

int main() {
    int currentRoom = 0;
    int turnNumber = 0;
    int guardPosition = 3;

    blocked[guardPosition] = true;

    while (currentRoom != 5) {
        turnNumber = turnNumber + 1;
        cout << "You are at: " << roomNames[currentRoom] << endl;

        if (turnNumber % 2 == 0) {
            blocked[2] = true;
        } else {
            blocked[2] = false;
        }

        blocked[guardPosition] = false;
        guardPosition = (guardPosition + 1) % n;

        if (guardPosition != 0 && guardPosition != 5) {
            blocked[guardPosition] = true;
        }

        cout << "\nAvailable moves:\n";

        int edgeIdx = 0;
        while (edgeIdx < (int)graph[currentRoom].size()) {
            int neighborRoom = graph[currentRoom][edgeIdx].dest_node;
            int edgeWeight = graph[currentRoom][edgeIdx].weight;
            if (roomIsBlocked(neighborRoom) == false) {
                cout << neighborRoom << " -> " << roomNames[neighborRoom]
                     << " (Danger " << edgeWeight << ")\n";
            }
            edgeIdx = edgeIdx + 1;
        }

        bool visitedNodes[n] = {false};
        vector<int> currentPath;

        cout << "\nPossible safe paths:\n";
        findAllPaths(currentRoom, 5, visitedNodes, blocked, currentPath);

        vector<int> safestPath = dijkstra(currentRoom, 5, blocked);

        cout << "\nSafest remaining path: ";
        int pathIdx = 0;
        while (pathIdx < (int)safestPath.size()) {
            cout << safestPath[pathIdx] << " ";
            pathIdx = pathIdx + 1;
        }
        cout << endl;

        int chosenRoom;
        cout << "\nChoose next room: ";
        cin >> chosenRoom;

        bool validChoice = moveIsValid(currentRoom, chosenRoom);

        if (validChoice == false) {
            cout << "Invalid move or blocked room!\n";
            continue;
        }

        currentRoom = chosenRoom;

        if (currentRoom == 4) {
            cout << "\nVault reached! Alarm activated!\n";

            int ei = 0;
            while (ei < (int)graph[1].size()) {
                if (graph[1][ei].dest_node == 4) {
                    graph[1][ei].weight = graph[1][ei].weight + 5;
                }
                ei = ei + 1;
            }
        }

        cout << "\nBlocked rooms: ";
        int roomIdx = 0;
        while (roomIdx < n) {
            if (roomIsBlocked(roomIdx) == true) {
                cout << roomNames[roomIdx] << " ";
            }
            roomIdx = roomIdx + 1;
        }

        cout << endl;
    }

    cout << "\nMission Successfull.\n";

    return 0;
}
