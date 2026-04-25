#include <emscripten.h>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <climits>
#include <cmath>
using namespace std;

// ── Shared output buffers ─────────────────────────────────────────────────────
static int visitedOrder[10000][2];
static int visitedCount = 0;
static int pathResult[10000][2];
static int pathLength = 0;

// ── Direction vectors ─────────────────────────────────────────────────────────
static const int dx[] = {-1, 1, 0, 0};
static const int dy[] = {0, 0, -1, 1};

// ── Parse grid from flat char array ──────────────────────────────────────────
static vector<vector<int>> parseGrid(const int* flat, int rows, int cols) {
    vector<vector<int>> g(rows, vector<int>(cols));
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            g[i][j] = flat[i * cols + j];
    return g;
}

// ── Reconstruct path from parent map ─────────────────────────────────────────
static int buildPath(vector<vector<pair<int,int>>>& parent,
                     int startR, int startC, int endR, int endC) {
    vector<pair<int,int>> path;
    int r = endR, c = endC;
    int safety = 10000;
    while (!(r == startR && c == startC) && safety-- > 0) {
        path.push_back({r, c});
        auto p = parent[r][c];
        if (p.first == -1) return 0; // no path
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

extern "C" {

// ═════════════════════════════════════════════════════════════════════════════
// BFS — shortest path, explores level by level
// ═════════════════════════════════════════════════════════════════════════════
EMSCRIPTEN_KEEPALIVE
int runBFS(const int* flat, int rows, int cols,
           int startR, int startC, int endR, int endC) {
    visitedCount = 0; pathLength = 0;

    auto grid = parseGrid(flat, rows, cols);
    if (grid[startR][startC] == 1 || grid[endR][endC] == 1) return 0;

    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<vector<pair<int,int>>> parent(rows, vector<pair<int,int>>(cols, {-1,-1}));

    queue<pair<int,int>> q;
    q.push({startR, startC});
    visited[startR][startC] = true;

    while (!q.empty()) {
        auto [r, c] = q.front(); q.pop();

        visitedOrder[visitedCount][0] = r;
        visitedOrder[visitedCount][1] = c;
        visitedCount++;

        if (r == endR && c == endC)
            return buildPath(parent, startR, startC, endR, endC);

        for (int d = 0; d < 4; d++) {
            int nr = r + dx[d], nc = c + dy[d];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
            if (visited[nr][nc] || grid[nr][nc] == 1) continue;
            visited[nr][nc] = true;
            parent[nr][nc] = {r, c};
            q.push({nr, nc});
        }
    }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════════
// DFS — recursive, explores deep before backtracking
// ═════════════════════════════════════════════════════════════════════════════
static vector<vector<bool>>        dfsVis;
static vector<vector<pair<int,int>>> dfsPar;
static vector<vector<int>>         dfsGrid;
static int dfsRows, dfsCols, dfsEndR, dfsEndC;
static bool dfsFound;

static void dfsRecurse(int r, int c) {
    if (dfsFound) return;
    dfsVis[r][c] = true;
    visitedOrder[visitedCount][0] = r;
    visitedOrder[visitedCount][1] = c;
    visitedCount++;

    if (r == dfsEndR && c == dfsEndC) { dfsFound = true; return; }

    for (int d = 0; d < 4; d++) {
        if (dfsFound) return;
        int nr = r + dx[d], nc = c + dy[d];
        if (nr < 0 || nr >= dfsRows || nc < 0 || nc >= dfsCols) continue;
        if (dfsVis[nr][nc] || dfsGrid[nr][nc] == 1) continue;
        dfsPar[nr][nc] = {r, c};
        dfsRecurse(nr, nc);
    }
}

EMSCRIPTEN_KEEPALIVE
int runDFS(const int* flat, int rows, int cols,
           int startR, int startC, int endR, int endC) {
    visitedCount = 0; pathLength = 0;

    dfsRows = rows; dfsCols = cols;
    dfsEndR = endR; dfsEndC = endC;
    dfsFound = false;

    dfsGrid.assign(rows, vector<int>(cols));
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            dfsGrid[i][j] = flat[i * cols + j];

    if (dfsGrid[startR][startC] == 1 || dfsGrid[endR][endC] == 1) return 0;

    dfsVis.assign(rows, vector<bool>(cols, false));
    dfsPar.assign(rows, vector<pair<int,int>>(cols, {-1,-1}));

    dfsRecurse(startR, startC);
    if (!dfsFound) return 0;
    return buildPath(dfsPar, startR, startC, endR, endC);
}

// ═════════════════════════════════════════════════════════════════════════════
// Dijkstra — optimal cost, uniform edge weights
// ═════════════════════════════════════════════════════════════════════════════
EMSCRIPTEN_KEEPALIVE
int runDijkstra(const int* flat, int rows, int cols,
                int startR, int startC, int endR, int endC) {
    visitedCount = 0; pathLength = 0;

    auto grid = parseGrid(flat, rows, cols);
    if (grid[startR][startC] == 1 || grid[endR][endC] == 1) return 0;

    vector<vector<int>> dist(rows, vector<int>(cols, INT_MAX));
    vector<vector<pair<int,int>>> parent(rows, vector<pair<int,int>>(cols, {-1,-1}));
    vector<vector<bool>> settled(rows, vector<bool>(cols, false));

    // {dist, {r, c}}
    priority_queue<pair<int,pair<int,int>>,
                   vector<pair<int,pair<int,int>>>,
                   greater<>> pq;
    dist[startR][startC] = 0;
    pq.push({0, {startR, startC}});

    while (!pq.empty()) {
        auto [d, rc] = pq.top(); pq.pop();
        auto [r, c] = rc;
        if (settled[r][c]) continue;
        settled[r][c] = true;

        visitedOrder[visitedCount][0] = r;
        visitedOrder[visitedCount][1] = c;
        visitedCount++;

        if (r == endR && c == endC)
            return buildPath(parent, startR, startC, endR, endC);

        for (int i = 0; i < 4; i++) {
            int nr = r + dx[i], nc = c + dy[i];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
            if (grid[nr][nc] == 1 || settled[nr][nc]) continue;
            int nd = d + 1;
            if (nd < dist[nr][nc]) {
                dist[nr][nc] = nd;
                parent[nr][nc] = {r, c};
                pq.push({nd, {nr, nc}});
            }
        }
    }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════════
// A* — heuristic-guided, fewest nodes explored
// ═════════════════════════════════════════════════════════════════════════════
static inline int heuristic(int r1, int c1, int r2, int c2) {
    return abs(r1 - r2) + abs(c1 - c2); // Manhattan distance
}

EMSCRIPTEN_KEEPALIVE
int runAStar(const int* flat, int rows, int cols,
             int startR, int startC, int endR, int endC) {
    visitedCount = 0; pathLength = 0;

    auto grid = parseGrid(flat, rows, cols);
    if (grid[startR][startC] == 1 || grid[endR][endC] == 1) return 0;

    vector<vector<int>> gCost(rows, vector<int>(cols, INT_MAX));
    vector<vector<pair<int,int>>> parent(rows, vector<pair<int,int>>(cols, {-1,-1}));
    vector<vector<bool>> closed(rows, vector<bool>(cols, false));

    // {f, {r, c}}
    priority_queue<pair<int,pair<int,int>>,
                   vector<pair<int,pair<int,int>>>,
                   greater<>> pq;
    gCost[startR][startC] = 0;
    pq.push({heuristic(startR, startC, endR, endC), {startR, startC}});

    while (!pq.empty()) {
        auto [f, rc] = pq.top(); pq.pop();
        auto [r, c] = rc;
        if (closed[r][c]) continue;
        closed[r][c] = true;

        visitedOrder[visitedCount][0] = r;
        visitedOrder[visitedCount][1] = c;
        visitedCount++;

        if (r == endR && c == endC)
            return buildPath(parent, startR, startC, endR, endC);

        for (int i = 0; i < 4; i++) {
            int nr = r + dx[i], nc = c + dy[i];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
            if (grid[nr][nc] == 1 || closed[nr][nc]) continue;
            int ng = gCost[r][c] + 1;
            if (ng < gCost[nr][nc]) {
                gCost[nr][nc] = ng;
                parent[nr][nc] = {r, c};
                pq.push({ng + heuristic(nr, nc, endR, endC), {nr, nc}});
            }
        }
    }
    return 0;
}

// ── Accessors ─────────────────────────────────────────────────────────────────
EMSCRIPTEN_KEEPALIVE int getVisitedCount()    { return visitedCount; }
EMSCRIPTEN_KEEPALIVE int getVisitedRow(int i) { return visitedOrder[i][0]; }
EMSCRIPTEN_KEEPALIVE int getVisitedCol(int i) { return visitedOrder[i][1]; }
EMSCRIPTEN_KEEPALIVE int getPathRow(int i)    { return pathResult[i][0]; }
EMSCRIPTEN_KEEPALIVE int getPathCol(int i)    { return pathResult[i][1]; }

} // extern "C"
