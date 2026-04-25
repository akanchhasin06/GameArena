#include <emscripten.h>
#include <vector>
#include <queue>
#include <array>
#include <cmath>
#include <map>
using namespace std;

const int goal[9] = {1,2,3,4,5,6,7,8,0};

static int stateSnapshots[1000][9];
static int movesList[1000];
static int movesCount, statesExpanded, pathCost;

int lowerBound(const int s[9]) {
    int dist = 0;
    for(int i = 0; i < 9; i++) {
        if(s[i] == 0) continue;
        int val = s[i] - 1;
        dist += abs(i/3 - val/3) + abs(i%3 - val%3);
    }
    return dist;
}

bool isGoal(const int s[9]) {
    for(int i = 0; i < 9; i++)
        if(s[i] != goal[i]) return false;
    return true;
}

string toKey(const int s[9]) {
    string k;
    for(int i = 0; i < 9; i++) k += char(s[i] + '0');
    return k;
}

int findZero(const int s[9]) {
    for(int i = 0; i < 9; i++)
        if(s[i] == 0) return i;
    return -1;
}

bool solvable(const int s[9]) {
    int inv = 0;
    for(int i = 0; i < 9; i++)
        for(int j = i+1; j < 9; j++)
            if(s[i] && s[j] && s[i] > s[j]) inv++;
    return inv % 2 == 0;
}

struct Node {
    array<int,9> state;
    int g, bound, blank;
    vector<int> moves;
    vector<array<int,9>> path;

    int f() const { return g + bound; }
    bool operator>(const Node& o) const { return f() > o.f(); }
};

extern "C" {

EMSCRIPTEN_KEEPALIVE
int runBranchBound(int s0,int s1,int s2,int s3,int s4,int s5,int s6,int s7,int s8) {
    int startArr[9] = {s0,s1,s2,s3,s4,s5,s6,s7,s8};
    if(!solvable(startArr)) return -1;

    movesCount = statesExpanded = pathCost = 0;

    Node start;
    for(int i = 0; i < 9; i++) start.state[i] = startArr[i];
    start.g = 0;
    start.bound = lowerBound(start.state.data());
    start.blank = findZero(start.state.data());
    start.path.push_back(start.state);

    priority_queue<Node, vector<Node>, greater<Node>> pq;
    pq.push(start);

    map<string,int> best;
    int dir[] = {-3, 3, -1, 1};

    while(!pq.empty()) {
        Node cur = pq.top(); pq.pop();
        statesExpanded++;

        if(isGoal(cur.state.data())) {
            pathCost  = cur.g;
            movesCount = (int)cur.moves.size();

            for(int i = 0; i < movesCount && i < 1000; i++) {
                movesList[i] = cur.moves[i];
                for(int j = 0; j < 9; j++)
                    stateSnapshots[i][j] = cur.path[i+1][j];
            }
            return movesCount;
        }

        string key = toKey(cur.state.data());
        if(best.count(key) && best[key] <= cur.g) continue;
        best[key] = cur.g;

        int b = cur.blank;
        for(int d = 0; d < 4; d++) {
            int nb = b + dir[d];
            if(nb < 0 || nb > 8) continue;
            if(d == 2 && b % 3 == 0) continue;
            if(d == 3 && b % 3 == 2) continue;

            Node nxt = cur;
            swap(nxt.state[b], nxt.state[nb]);
            nxt.blank = nb;
            nxt.g = cur.g + 1;
            nxt.bound = lowerBound(nxt.state.data());
            nxt.moves.push_back(nxt.state[b]);
            nxt.path.push_back(nxt.state);

            string k = toKey(nxt.state.data());
            if(!best.count(k) || best[k] > nxt.g)
                pq.push(nxt);
        }
    }
    return 0;
}

EMSCRIPTEN_KEEPALIVE int getMoveAt(int i)              { return movesList[i]; }
EMSCRIPTEN_KEEPALIVE int getSnapshotCell(int i, int j) { return stateSnapshots[i][j]; }
EMSCRIPTEN_KEEPALIVE int getMovesCount()               { return movesCount; }
EMSCRIPTEN_KEEPALIVE int getStatesExpanded()           { return statesExpanded; }
EMSCRIPTEN_KEEPALIVE int getPathCost()                 { return pathCost; }

}
