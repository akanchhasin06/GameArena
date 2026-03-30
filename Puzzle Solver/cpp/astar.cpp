#include <emscripten.h>
#include <vector>
#include <queue>
#include <array>
#include <cmath>
#include <map>
#include <string>
using namespace std;


//A* Algorithm


int goalState[9] = {1,2,3,4,5,6,7,8,0};

int movesList[1000];
int movesCount = 0;
int statesExpanded = 0;
int pathCost = 0;

int manhattanDistance(int state[9]) {
    int dist = 0;
    for(int i = 0; i < 9; i++) {
        if(state[i] != 0) {
            int goalPos = 0;
            for(int j = 0; j < 9; j++) {
                if(goalState[j] == state[i]) { goalPos = j; break; }
            }
            int curRow = i / 3, curCol = i % 3;
            int goalRow = goalPos / 3, goalCol = goalPos % 3;
            dist += abs(curRow - goalRow) + abs(curCol - goalCol);
        }
    }
    return dist;
}

int misplacedTiles(int state[9]) {
    int count = 0;
    for(int i = 0; i < 9; i++) {
        if(state[i] != 0 && state[i] != goalState[i]) count++;
    }
    return count;
}

bool isGoal(int state[9]) {
    for(int i = 0; i < 9; i++) {
        if(state[i] != goalState[i]) return false;
    }
    return true;
}

string stateToKey(int state[9]) {
    string key = "";
    for(int i = 0; i < 9; i++) key += to_string(state[i]) + ",";
    return key;
}

int findBlank(int state[9]) {
    for(int i = 0; i < 9; i++) {
        if(state[i] == 0) return i;
    }
    return -1;
}

// Check if puzzle is solvable -( no of inversions is even)
bool isSolvable(int state[9]) {
    int inversions = 0;
    for(int i = 0; i < 9; i++) {
        for(int j = i+1; j < 9; j++) {
            if(state[i] && state[j] && state[i] > state[j]) {
                inversions++;
            }
        }
    }
    return inversions % 2 == 0;
}


struct Node {
    array<int,9> state;
    int g;
    int h;
    int f;
    int blankPos;
    vector<int> moves;

    bool operator>(const Node& other) const {
        return f > other.f;
    }
};


extern "C" {

    EMSCRIPTEN_KEEPALIVE
    int runAstar(int s0, int s1, int s2,
                 int s3, int s4, int s5,
                 int s6, int s7, int s8,
                 int heuristicType) {

        int inputState[9] = {s0,s1,s2,s3,s4,s5,s6,s7,s8};

        // check solvability
        if(!isSolvable(inputState)) {
            return -1;
        }

        // reset results
        movesCount = 0;
        statesExpanded = 0;
        pathCost = 0;

        // build start node
        Node start;
        for(int i = 0; i < 9; i++) start.state[i] = inputState[i];
        start.blankPos = findBlank(start.state.data());
        start.g = 0;

        if(heuristicType == 0) {
            start.h = manhattanDistance(start.state.data());
        } else {
            start.h = misplacedTiles(start.state.data());
        }
        start.f = start.g + start.h;

        priority_queue<Node, vector<Node>, greater<Node>> pq;
        pq.push(start);

        map<string, bool> visited;

        int dx[] = {-3, 3, -1, 1};

        while(!pq.empty()) {
            Node curr = pq.top();
            pq.pop();

            statesExpanded++;

            // check if goal
            if(isGoal(curr.state.data())) {
                pathCost = curr.g;
                movesCount = curr.moves.size();
                for(int i = 0; i < movesCount; i++) {
                    movesList[i] = curr.moves[i];
                }
                return movesCount;
            }

            string key = stateToKey(curr.state.data());
            if(visited[key]) continue;
            visited[key] = true;

            // expand neighbours
            int blank = curr.blankPos;
            for(int d = 0; d < 4; d++) {
                int newBlank = blank + dx[d];

                if(newBlank < 0 || newBlank > 8) continue;
                if(d == 2 && blank % 3 == 0) continue;
                if(d == 3 && blank % 3 == 2) continue;

                Node next;
                next.state = curr.state;
                swap(next.state[blank], next.state[newBlank]);
                next.blankPos = newBlank;
                next.g = curr.g + 1;
                next.moves = curr.moves;
                next.moves.push_back(next.state[blank]);

                string nextKey = stateToKey(next.state.data());
                if(!visited[nextKey]) {
                    if(heuristicType == 0) {
                        next.h = manhattanDistance(next.state.data());
                    } else {
                        next.h = misplacedTiles(next.state.data());
                    }
                    next.f = next.g + next.h;
                    pq.push(next);
                }
            }
        }

        return 0;
    }

    EMSCRIPTEN_KEEPALIVE
    int getMoveAt(int index) { return movesList[index]; }

    EMSCRIPTEN_KEEPALIVE
    int getMovesCount() { return movesCount; }

    EMSCRIPTEN_KEEPALIVE
    int getStatesExpanded() { return statesExpanded; }

    EMSCRIPTEN_KEEPALIVE
    int getPathCost() { return pathCost; }
}