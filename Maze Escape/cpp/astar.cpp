#include <emscripten.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
using namespace std;

int visitedOrder[10000][2];
int visitedCount = 0;
int pathResult[10000][2];
int pathLength = 0;

int heuristic(int r1, int c1, int r2, int c2) {
    return abs(r1 - r2) + abs(c1 - c2);
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int runAStar(const char* gridStr, int rows, int cols,
             int startR, int startC, int endR, int endC) {

    visitedCount = 0;
    pathLength = 0;

    vector<vector<int>> grid(rows, vector<int>(cols));
    for(int i=0;i<rows;i++)
        for(int j=0;j<cols;j++)
            grid[i][j] = gridStr[i*cols + j] - '0';

    if(grid[startR][startC] == 1 || grid[endR][endC] == 1) return 0;

    vector<vector<int>> g(rows, vector<int>(cols, 1e9));
    vector<vector<pair<int,int>>> parent(rows, vector<pair<int,int>>(cols, {-1,-1}));

    // f(n), r, c
    priority_queue<
        pair<int, pair<int,int>>,
        vector<pair<int, pair<int,int>>>,
        greater<>
    > pq;

    g[startR][startC] = 0;
    pq.push({heuristic(startR,startC,endR,endC), {startR,startC}});

    int dx[] = {-1,1,0,0};
    int dy[] = {0,0,-1,1};

    bool found = false;

    while(!pq.empty()) {
        auto top = pq.top(); pq.pop();

        int r = top.second.first;
        int c = top.second.second;

        if(visitedCount < 10000) {
            visitedOrder[visitedCount][0] = r;
            visitedOrder[visitedCount][1] = c;
            visitedCount++;
        }

        if(r == endR && c == endC) {
            found = true;
            break;
        }

        for(int i=0;i<4;i++) {
            int nr = r + dx[i];
            int nc = c + dy[i];

            if(nr<0||nr>=rows||nc<0||nc>=cols) continue;
            if(grid[nr][nc] == 1) continue;

            int newG = g[r][c] + 1;

            if(newG < g[nr][nc]) {
                g[nr][nc] = newG;
                parent[nr][nc] = {r,c};

                int f = newG + heuristic(nr,nc,endR,endC);
                pq.push({f,{nr,nc}});
            }
        }
    }

    if(!found) return 0;

    vector<pair<int,int>> path;
    int r = endR, c = endC;

    while(r != -1 && c != -1) {
        path.push_back({r,c});
        if(r == startR && c == startC) break;
        auto p = parent[r][c];
        r = p.first;
        c = p.second;
    }

    reverse(path.begin(), path.end());

    pathLength = path.size();
    for(int i=0;i<pathLength;i++) {
        pathResult[i][0] = path[i].first;
        pathResult[i][1] = path[i].second;
    }

    return pathLength;
}

// expose functions

EMSCRIPTEN_KEEPALIVE
int getVisitedCount() { return visitedCount; }

EMSCRIPTEN_KEEPALIVE
int getVisitedRow(int i) { return visitedOrder[i][0]; }

EMSCRIPTEN_KEEPALIVE
int getVisitedCol(int i) { return visitedOrder[i][1]; }

EMSCRIPTEN_KEEPALIVE
int getPathRow(int i) { return pathResult[i][0]; }

EMSCRIPTEN_KEEPALIVE
int getPathCol(int i) { return pathResult[i][1]; }

}