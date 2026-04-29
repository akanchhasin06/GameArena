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

static bool isCellWall(int cellValue) {
    return cellValue == 1;
}

static bool isWithinBounds(int row, int col, int totalRows, int totalCols) {
    if (row < 0) return false;
    if (row >= totalRows) return false;
    if (col < 0) return false;
    if (col >= totalCols) return false;
    return true;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int runDijkstra(const char* gridStr, int rows, int cols,
                int startR, int startC, int endR, int endC) {

    visitedCount = 0;
    pathLength = 0;

    vector<vector<int>> gridData(rows, vector<int>(cols));

    int rowIdx = 0;
    while (rowIdx < rows) {
        int colIdx = 0;
        while (colIdx < cols) {
            int flatIndex = rowIdx * cols + colIdx;
            gridData[rowIdx][colIdx] = gridStr[flatIndex] - '0';
            colIdx = colIdx + 1;
        }
        rowIdx = rowIdx + 1;
    }

    if (isCellWall(gridData[startR][startC]) == true) return 0;
    if (isCellWall(gridData[endR][endC]) == true) return 0;

    vector<vector<int>> distanceMap(rows, vector<int>(cols, INT_MAX));
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

    priority_queue<
        pair<int, pair<int,int>>,
        vector<pair<int, pair<int,int>>>,
        greater<pair<int, pair<int,int>>>
    > minHeap;

    distanceMap[startR][startC] = 0;
    pair<int,int> startCell = make_pair(startR, startC);
    pair<int, pair<int,int>> startEntry = make_pair(0, startCell);
    minHeap.push(startEntry);

    int rowDelta[4] = {-1, 1, 0, 0};
    int colDelta[4] = {0, 0, -1, 1};

    bool targetFound = false;

    while (minHeap.empty() == false) {
        pair<int, pair<int,int>> topEntry = minHeap.top();
        minHeap.pop();

        int currentDist = topEntry.first;
        int currentRow = topEntry.second.first;
        int currentCol = topEntry.second.second;

        if (currentDist > distanceMap[currentRow][currentCol]) continue;

        if (visitedCount >= 10000) break;

        visitedOrder[visitedCount][0] = currentRow;
        visitedOrder[visitedCount][1] = currentCol;
        visitedCount = visitedCount + 1;

        if (currentRow == endR && currentCol == endC) {
            targetFound = true;
            break;
        }

        int dirIdx = 0;
        while (dirIdx < 4) {
            int neighborRow = currentRow + rowDelta[dirIdx];
            int neighborCol = currentCol + colDelta[dirIdx];

            if (isWithinBounds(neighborRow, neighborCol, rows, cols) == false) {
                dirIdx = dirIdx + 1;
                continue;
            }
            if (isCellWall(gridData[neighborRow][neighborCol]) == true) {
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

    if (targetFound == false) return 0;

    vector<pair<int,int>> reconstructedPath;
    int traceRow = endR;
    int traceCol = endC;

    while (traceRow != -1 && traceCol != -1) {
        pair<int,int> stepCell = make_pair(traceRow, traceCol);
        reconstructedPath.push_back(stepCell);
        if (traceRow == startR && traceCol == startC) break;
        pair<int,int> parentEntry = parentCell[traceRow][traceCol];
        traceRow = parentEntry.first;
        traceCol = parentEntry.second;
    }

    reverse(reconstructedPath.begin(), reconstructedPath.end());

    pathLength = reconstructedPath.size();

    int fillIdx = 0;
    while (fillIdx < pathLength && fillIdx < 10000) {
        pathResult[fillIdx][0] = reconstructedPath[fillIdx].first;
        pathResult[fillIdx][1] = reconstructedPath[fillIdx].second;
        fillIdx = fillIdx + 1;
    }

    return pathLength;
}

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
