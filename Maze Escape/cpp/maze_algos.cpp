#include <emscripten.h>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <climits>
#include <cmath>
using namespace std;

static int visitedOrder[10000][2];
static int visitedCount = 0;
static int pathResult[10000][2];
static int pathLength = 0;

static const int rowStep[4] = {-1, 1, 0, 0};
static const int colStep[4] = {0, 0, -1, 1};

static bool cellIsWall(int cellValue) {
    return cellValue == 1;
}

static bool positionInBounds(int row, int col, int maxRows, int maxCols) {
    if (row < 0) return false;
    if (row >= maxRows) return false;
    if (col < 0) return false;
    if (col >= maxCols) return false;
    return true;
}

static vector<vector<int>> buildGridFromFlat(const int* flatData, int rows, int cols) {
    vector<vector<int>> gridMatrix(rows, vector<int>(cols));
    int ri = 0;
    while (ri < rows) {
        int ci = 0;
        while (ci < cols) {
            gridMatrix[ri][ci] = flatData[ri * cols + ci];
            ci = ci + 1;
        }
        ri = ri + 1;
    }
    return gridMatrix;
}

static int assemblePath(vector<vector<pair<int,int>>>& parentMap,
                         int startRow, int startCol, int endRow, int endCol) {
    vector<pair<int,int>> tracedPath;
    int currentRow = endRow;
    int currentCol = endCol;
    int safetyLimit = 10000;

    while ((currentRow == startRow && currentCol == startCol) == false && safetyLimit > 0) {
        safetyLimit = safetyLimit - 1;
        pair<int,int> stepCell = make_pair(currentRow, currentCol);
        tracedPath.push_back(stepCell);
        pair<int,int> ancestor = parentMap[currentRow][currentCol];
        if (ancestor.first == -1) return 0;
        currentRow = ancestor.first;
        currentCol = ancestor.second;
    }

    pair<int,int> originCell = make_pair(startRow, startCol);
    tracedPath.push_back(originCell);
    reverse(tracedPath.begin(), tracedPath.end());

    pathLength = (int)tracedPath.size();

    int fillIdx = 0;
    while (fillIdx < pathLength && fillIdx < 10000) {
        pathResult[fillIdx][0] = tracedPath[fillIdx].first;
        pathResult[fillIdx][1] = tracedPath[fillIdx].second;
        fillIdx = fillIdx + 1;
    }
    return pathLength;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int runBFS(const int* flatData, int rows, int cols,
           int startR, int startC, int endR, int endC) {
    visitedCount = 0;
    pathLength = 0;

    vector<vector<int>> gridMatrix = buildGridFromFlat(flatData, rows, cols);
    if (cellIsWall(gridMatrix[startR][startC]) == true) return 0;
    if (cellIsWall(gridMatrix[endR][endC]) == true) return 0;

    vector<vector<bool>> cellVisited(rows, vector<bool>(cols, false));
    vector<vector<pair<int,int>>> parentCell(rows, vector<pair<int,int>>(cols));

    int pi = 0;
    while (pi < rows) {
        int pj = 0;
        while (pj < cols) {
            parentCell[pi][pj] = make_pair(-1, -1);
            pj = pj + 1;
        }
        pi = pi + 1;
    }

    queue<pair<int,int>> frontier;
    pair<int,int> startCell = make_pair(startR, startC);
    frontier.push(startCell);
    cellVisited[startR][startC] = true;

    while (frontier.empty() == false) {
        pair<int,int> frontCell = frontier.front();
        frontier.pop();
        int currentRow = frontCell.first;
        int currentCol = frontCell.second;

        visitedOrder[visitedCount][0] = currentRow;
        visitedOrder[visitedCount][1] = currentCol;
        visitedCount = visitedCount + 1;

        if (currentRow == endR && currentCol == endC) {
            return assemblePath(parentCell, startR, startC, endR, endC);
        }

        int dirIdx = 0;
        while (dirIdx < 4) {
            int neighborRow = currentRow + rowStep[dirIdx];
            int neighborCol = currentCol + colStep[dirIdx];

            if (positionInBounds(neighborRow, neighborCol, rows, cols) == false) {
                dirIdx = dirIdx + 1;
                continue;
            }
            if (cellVisited[neighborRow][neighborCol] == true || cellIsWall(gridMatrix[neighborRow][neighborCol]) == true) {
                dirIdx = dirIdx + 1;
                continue;
            }

            cellVisited[neighborRow][neighborCol] = true;
            parentCell[neighborRow][neighborCol] = make_pair(currentRow, currentCol);
            pair<int,int> neighborCell = make_pair(neighborRow, neighborCol);
            frontier.push(neighborCell);

            dirIdx = dirIdx + 1;
        }
    }
    return 0;
}

static vector<vector<bool>> dfsSeenCells;
static vector<vector<pair<int,int>>> dfsParentMap;
static vector<vector<int>> dfsGridData;
static int dfsRowCount;
static int dfsColCount;
static int dfsTargetRow;
static int dfsTargetCol;
static bool dfsGoalReached;

static void depthFirstExplore(int row, int col) {
    if (dfsGoalReached == true) return;

    dfsSeenCells[row][col] = true;
    visitedOrder[visitedCount][0] = row;
    visitedOrder[visitedCount][1] = col;
    visitedCount = visitedCount + 1;

    if (row == dfsTargetRow && col == dfsTargetCol) {
        dfsGoalReached = true;
        return;
    }

    int dirIdx = 0;
    while (dirIdx < 4) {
        if (dfsGoalReached == true) return;

        int neighborRow = row + rowStep[dirIdx];
        int neighborCol = col + colStep[dirIdx];

        if (positionInBounds(neighborRow, neighborCol, dfsRowCount, dfsColCount) == false) {
            dirIdx = dirIdx + 1;
            continue;
        }
        if (dfsSeenCells[neighborRow][neighborCol] == true || cellIsWall(dfsGridData[neighborRow][neighborCol]) == true) {
            dirIdx = dirIdx + 1;
            continue;
        }

        dfsParentMap[neighborRow][neighborCol] = make_pair(row, col);
        depthFirstExplore(neighborRow, neighborCol);

        dirIdx = dirIdx + 1;
    }
}

EMSCRIPTEN_KEEPALIVE
int runDFS(const int* flatData, int rows, int cols,
           int startR, int startC, int endR, int endC) {
    visitedCount = 0;
    pathLength = 0;

    dfsRowCount = rows;
    dfsColCount = cols;
    dfsTargetRow = endR;
    dfsTargetCol = endC;
    dfsGoalReached = false;

    dfsGridData.assign(rows, vector<int>(cols));
    int ri = 0;
    while (ri < rows) {
        int ci = 0;
        while (ci < cols) {
            dfsGridData[ri][ci] = flatData[ri * cols + ci];
            ci = ci + 1;
        }
        ri = ri + 1;
    }

    if (cellIsWall(dfsGridData[startR][startC]) == true) return 0;
    if (cellIsWall(dfsGridData[endR][endC]) == true) return 0;

    dfsSeenCells.assign(rows, vector<bool>(cols, false));
    dfsParentMap.assign(rows, vector<pair<int,int>>(cols));

    int pi = 0;
    while (pi < rows) {
        int pj = 0;
        while (pj < cols) {
            dfsParentMap[pi][pj] = make_pair(-1, -1);
            pj = pj + 1;
        }
        pi = pi + 1;
    }

    depthFirstExplore(startR, startC);
    if (dfsGoalReached == false) return 0;
    return assemblePath(dfsParentMap, startR, startC, endR, endC);
}

EMSCRIPTEN_KEEPALIVE
int runDijkstra(const int* flatData, int rows, int cols,
                int startR, int startC, int endR, int endC) {
    visitedCount = 0;
    pathLength = 0;

    vector<vector<int>> gridMatrix = buildGridFromFlat(flatData, rows, cols);
    if (cellIsWall(gridMatrix[startR][startC]) == true) return 0;
    if (cellIsWall(gridMatrix[endR][endC]) == true) return 0;

    vector<vector<int>> distanceMap(rows, vector<int>(cols, INT_MAX));
    vector<vector<pair<int,int>>> parentCell(rows, vector<pair<int,int>>(cols));
    vector<vector<bool>> settledCell(rows, vector<bool>(cols, false));

    int pi = 0;
    while (pi < rows) {
        int pj = 0;
        while (pj < cols) {
            parentCell[pi][pj] = make_pair(-1, -1);
            pj = pj + 1;
        }
        pi = pi + 1;
    }

    priority_queue<
        pair<int, pair<int,int>>,
        vector<pair<int, pair<int,int>>>,
        greater<pair<int, pair<int,int>>>
    > minHeap;

    distanceMap[startR][startC] = 0;
    pair<int,int> startCell = make_pair(startR, startC);
    pair<int, pair<int,int>> startEntry = make_pair(0, startCell);
    minHeap.push(startEntry);

    while (minHeap.empty() == false) {
        pair<int, pair<int,int>> topEntry = minHeap.top();
        minHeap.pop();
        int currentDist = topEntry.first;
        int currentRow = topEntry.second.first;
        int currentCol = topEntry.second.second;

        if (settledCell[currentRow][currentCol] == true) continue;
        settledCell[currentRow][currentCol] = true;

        visitedOrder[visitedCount][0] = currentRow;
        visitedOrder[visitedCount][1] = currentCol;
        visitedCount = visitedCount + 1;

        if (currentRow == endR && currentCol == endC) {
            return assemblePath(parentCell, startR, startC, endR, endC);
        }

        int dirIdx = 0;
        while (dirIdx < 4) {
            int neighborRow = currentRow + rowStep[dirIdx];
            int neighborCol = currentCol + colStep[dirIdx];

            if (positionInBounds(neighborRow, neighborCol, rows, cols) == false) {
                dirIdx = dirIdx + 1;
                continue;
            }
            if (cellIsWall(gridMatrix[neighborRow][neighborCol]) == true || settledCell[neighborRow][neighborCol] == true) {
                dirIdx = dirIdx + 1;
                continue;
            }

            int tentativeDist = currentDist + 1;

            if (tentativeDist < distanceMap[neighborRow][neighborCol]) {
                distanceMap[neighborRow][neighborCol] = tentativeDist;
                parentCell[neighborRow][neighborCol] = make_pair(currentRow, currentCol);
                pair<int,int> neighborCell = make_pair(neighborRow, neighborCol);
                pair<int, pair<int,int>> neighborEntry = make_pair(tentativeDist, neighborCell);
                minHeap.push(neighborEntry);
            }

            dirIdx = dirIdx + 1;
        }
    }
    return 0;
}

static int manhattanEstimate(int r1, int c1, int r2, int c2) {
    int rowDiff = abs(r1 - r2);
    int colDiff = abs(c1 - c2);
    return rowDiff + colDiff;
}

EMSCRIPTEN_KEEPALIVE
int runAStar(const int* flatData, int rows, int cols,
             int startR, int startC, int endR, int endC) {
    visitedCount = 0;
    pathLength = 0;

    vector<vector<int>> gridMatrix = buildGridFromFlat(flatData, rows, cols);
    if (cellIsWall(gridMatrix[startR][startC]) == true) return 0;
    if (cellIsWall(gridMatrix[endR][endC]) == true) return 0;

    vector<vector<int>> gCostMap(rows, vector<int>(cols, INT_MAX));
    vector<vector<pair<int,int>>> parentCell(rows, vector<pair<int,int>>(cols));
    vector<vector<bool>> closedCell(rows, vector<bool>(cols, false));

    int pi = 0;
    while (pi < rows) {
        int pj = 0;
        while (pj < cols) {
            parentCell[pi][pj] = make_pair(-1, -1);
            pj = pj + 1;
        }
        pi = pi + 1;
    }

    priority_queue<
        pair<int, pair<int,int>>,
        vector<pair<int, pair<int,int>>>,
        greater<pair<int, pair<int,int>>>
    > openSet;

    gCostMap[startR][startC] = 0;
    int initialF = manhattanEstimate(startR, startC, endR, endC);
    pair<int,int> startCell = make_pair(startR, startC);
    pair<int, pair<int,int>> startEntry = make_pair(initialF, startCell);
    openSet.push(startEntry);

    while (openSet.empty() == false) {
        pair<int, pair<int,int>> topEntry = openSet.top();
        openSet.pop();
        int currentRow = topEntry.second.first;
        int currentCol = topEntry.second.second;

        if (closedCell[currentRow][currentCol] == true) continue;
        closedCell[currentRow][currentCol] = true;

        visitedOrder[visitedCount][0] = currentRow;
        visitedOrder[visitedCount][1] = currentCol;
        visitedCount = visitedCount + 1;

        if (currentRow == endR && currentCol == endC) {
            return assemblePath(parentCell, startR, startC, endR, endC);
        }

        int dirIdx = 0;
        while (dirIdx < 4) {
            int neighborRow = currentRow + rowStep[dirIdx];
            int neighborCol = currentCol + colStep[dirIdx];

            if (positionInBounds(neighborRow, neighborCol, rows, cols) == false) {
                dirIdx = dirIdx + 1;
                continue;
            }
            if (cellIsWall(gridMatrix[neighborRow][neighborCol]) == true || closedCell[neighborRow][neighborCol] == true) {
                dirIdx = dirIdx + 1;
                continue;
            }

            int tentativeG = gCostMap[currentRow][currentCol] + 1;

            if (tentativeG < gCostMap[neighborRow][neighborCol]) {
                gCostMap[neighborRow][neighborCol] = tentativeG;
                parentCell[neighborRow][neighborCol] = make_pair(currentRow, currentCol);
                int fScore = tentativeG + manhattanEstimate(neighborRow, neighborCol, endR, endC);
                pair<int,int> neighborCell = make_pair(neighborRow, neighborCol);
                pair<int, pair<int,int>> neighborEntry = make_pair(fScore, neighborCell);
                openSet.push(neighborEntry);
            }

            dirIdx = dirIdx + 1;
        }
    }
    return 0;
}

EMSCRIPTEN_KEEPALIVE int getVisitedCount()    { return visitedCount; }
EMSCRIPTEN_KEEPALIVE int getVisitedRow(int i) { return visitedOrder[i][0]; }
EMSCRIPTEN_KEEPALIVE int getVisitedCol(int i) { return visitedOrder[i][1]; }
EMSCRIPTEN_KEEPALIVE int getPathRow(int i)    { return pathResult[i][0]; }
EMSCRIPTEN_KEEPALIVE int getPathCol(int i)    { return pathResult[i][1]; }

}
