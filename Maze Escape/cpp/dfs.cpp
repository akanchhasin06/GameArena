#include <emscripten.h>
#include <vector>
#include <algorithm>
using namespace std;

int visitedOrder[10000][2];
int visitedCount = 0;
int pathResult[10000][2];
int pathLength = 0;

// Global state for recursive DFS
static vector<vector<bool>> vis;
static vector<vector<pair<int,int>>> par;
static vector<vector<int>> gGrid;
static int gRows, gCols, gEndR, gEndC;
static bool gFound;

static int dx[] = {-1, 1, 0, 0};
static int dy[] = {0, 0, -1, 1};

void dfsRecurse(int r, int c) {
    if (gFound) return;

    vis[r][c] = true;
    if (visitedCount < 10000) {
        visitedOrder[visitedCount][0] = r;
        visitedOrder[visitedCount][1] = c;
        visitedCount++;
    }

    if (r == gEndR && c == gEndC) {
        gFound = true;
        return;
    }

    for (int d = 0; d < 4; d++) {
        if (gFound) return;
        int nr = r + dx[d];
        int nc = c + dy[d];
        if (nr < 0 || nr >= gRows || nc < 0 || nc >= gCols) continue;
        if (vis[nr][nc]) continue;
        if (gGrid[nr][nc] == 1) continue;
        par[nr][nc] = {r, c};
        dfsRecurse(nr, nc);
    }
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int runDFS(const char* gridStr, int rows, int cols,
           int startR, int startC, int endR, int endC) {

    visitedCount = 0;
    pathLength   = 0;

    gRows = rows; gCols = cols;
    gEndR = endR; gEndC = endC;
    gFound = false;

    gGrid.assign(rows, vector<int>(cols));
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            gGrid[i][j] = gridStr[i * cols + j] - '0';

    if (gGrid[startR][startC] == 1 || gGrid[endR][endC] == 1) return 0;

    vis.assign(rows, vector<bool>(cols, false));
    par.assign(rows, vector<pair<int,int>>(cols, {-1, -1}));

    dfsRecurse(startR, startC);

    if (!gFound) return 0;

    // Reconstruct path by backtracking from end to start
    vector<pair<int,int>> path;
    int r = endR, c = endC;
    while (!(r == startR && c == startC)) {
        path.push_back({r, c});
        auto p = par[r][c];
        if (p.first == -1) return 0; // no valid path
        r = p.first;
        c = p.second;
    }
    path.push_back({startR, startC});
    reverse(path.begin(), path.end());

    pathLength = (int)path.size();
    for (int i = 0; i < pathLength && i < 10000; i++) {
        pathResult[i][0] = path[i].first;
        pathResult[i][1] = path[i].second;
    }

    return pathLength;
}

EMSCRIPTEN_KEEPALIVE int getVisitedCount()    { return visitedCount; }
EMSCRIPTEN_KEEPALIVE int getVisitedRow(int i) { return visitedOrder[i][0]; }
EMSCRIPTEN_KEEPALIVE int getVisitedCol(int i) { return visitedOrder[i][1]; }
EMSCRIPTEN_KEEPALIVE int getPathRow(int i)    { return pathResult[i][0]; }
EMSCRIPTEN_KEEPALIVE int getPathCol(int i)    { return pathResult[i][1]; }

}
