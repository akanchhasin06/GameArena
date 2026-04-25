#include <emscripten.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
using namespace std;

int visitedOrder[10000][2];
int visitedCount = 0;
int pathResult[10000][2];
int pathLength = 0;

extern "C" {

EMSCRIPTEN_KEEPALIVE
int runDijkstra(const char* gridStr, int rows, int cols,
                int startR, int startC, int endR, int endC) {

    visitedCount = 0;
    pathLength = 0;

    vector<vector<int>> grid(rows, vector<int>(cols));
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            grid[i][j] = gridStr[i*cols + j] - '0';
        }
    }

    if(grid[startR][startC] == 1 || grid[endR][endC] == 1) return 0;

    vector<vector<int>> dist(rows, vector<int>(cols, INT_MAX));
    vector<vector<pair<int,int>>> parent(rows, vector<pair<int,int>>(cols, {-1,-1}));

    // Min heap
    priority_queue<
        pair<int, pair<int,int>>,
        vector<pair<int, pair<int,int>>>,
        greater<pair<int, pair<int,int>>>
    > pq;

    pq.push({0, {startR, startC}});
    dist[startR][startC] = 0;

    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    bool found = false;

    while(!pq.empty()) {
        auto top = pq.top();
        pq.pop();

        int d = top.first;
        int r = top.second.first;
        int c = top.second.second;

        if(d > dist[r][c]) continue;

        if(visitedCount >= 10000) break;

        visitedOrder[visitedCount][0] = r;
        visitedOrder[visitedCount][1] = c;
        visitedCount++;

        if(r == endR && c == endC) {
            found = true;
            break;
        }

        for(int i=0; i<4; i++) {
            int nr = r + dx[i];
            int nc = c + dy[i];

            if(nr<0 || nr>=rows || nc<0 || nc>=cols) continue;
            if(grid[nr][nc] == 1) continue;

            int newDist = d + 1;

            if(newDist < dist[nr][nc]) {
                dist[nr][nc] = newDist;
                parent[nr][nc] = {r, c};
                pq.push({newDist, {nr, nc}});
            }
        }
    }

    if(!found) return 0;

    vector<pair<int,int>> path;
    int r = endR, c = endC;

    while(r != -1 && c != -1) {
        path.push_back({r, c});
        if(r == startR && c == startC) break;
        auto p = parent[r][c];
        r = p.first;
        c = p.second;
    }

    reverse(path.begin(), path.end());

    pathLength = path.size();
    for(int i=0; i<pathLength && i<10000; i++) {
        pathResult[i][0] = path[i].first;
        pathResult[i][1] = path[i].second;
    }

    return pathLength;
}

// ✅ KEEP EVERYTHING INSIDE extern "C"

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