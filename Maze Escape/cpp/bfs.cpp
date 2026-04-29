#include <emscripten.h>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
using namespace std;

int visitedOrder[10000][2];
int visitedCount = 0;
int pathResult[10000][2];
int pathLength = 0;

static bool cellIsBlocked(int cellValue) {
    return cellValue == 1;
}

static bool positionInBounds(int row, int col, int totalRows, int totalCols) {
    if (row < 0) return false;
    if (row >= totalRows) return false;
    if (col < 0) return false;
    if (col >= totalCols) return false;
    return true;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int runBFS(const char* gridStr, int rows, int cols,
           int startR, int startC, int endR, int endC) {

    visitedCount = 0;
    pathLength = 0;

    vector<vector<int>> gridData(rows, vector<int>(cols));

    int ri = 0;
    while (ri < rows) {
        int ci = 0;
        while (ci < cols) {
            int flatPos = ri * cols + ci;
            gridData[ri][ci] = gridStr[flatPos] - '0';
            ci = ci + 1;
        }
        ri = ri + 1;
    }

    if (cellIsBlocked(gridData[startR][startC]) == true) return 0;
    if (cellIsBlocked(gridData[endR][endC]) == true) return 0;

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

    vector<vector<bool>> seenCell(rows, vector<bool>(cols, false));

    queue<pair<int,int>> frontier;
    pair<int,int> startCell = make_pair(startR, startC);
    frontier.push(startCell);
    seenCell[startR][startC] = true;

    int rowStep[4] = {-1, 1, 0, 0};
    int colStep[4] = {0, 0, -1, 1};

    bool destinationReached = false;

    while (frontier.empty() == false) {
        pair<int,int> frontCell = frontier.front();
        int currentRow = frontCell.first;
        int currentCol = frontCell.second;
        frontier.pop();

        if (visitedCount >= 10000) break;

        visitedOrder[visitedCount][0] = currentRow;
        visitedOrder[visitedCount][1] = currentCol;
        visitedCount = visitedCount + 1;

        if (currentRow == endR && currentCol == endC) {
            destinationReached = true;
            break;
        }

        int dirIdx = 0;
        while (dirIdx < 4) {
            int neighborRow = currentRow + rowStep[dirIdx];
            int neighborCol = currentCol + colStep[dirIdx];

            if (positionInBounds(neighborRow, neighborCol, rows, cols) == false) {
                dirIdx = dirIdx + 1;
                continue;
            }
            if (seenCell[neighborRow][neighborCol] == true) {
                dirIdx = dirIdx + 1;
                continue;
            }
            if (cellIsBlocked(gridData[neighborRow][neighborCol]) == true) {
                dirIdx = dirIdx + 1;
                continue;
            }

            seenCell[neighborRow][neighborCol] = true;
            parentCell[neighborRow][neighborCol] = make_pair(currentRow, currentCol);
            pair<int,int> neighborCell = make_pair(neighborRow, neighborCol);
            frontier.push(neighborCell);

            dirIdx = dirIdx + 1;
        }
    }

    if (destinationReached == false) return 0;

    vector<pair<int,int>> tracedPath;
    int traceRow = endR;
    int traceCol = endC;

    while (traceRow != -1 && traceCol != -1) {
        pair<int,int> step = make_pair(traceRow, traceCol);
        tracedPath.push_back(step);
        if (traceRow == startR && traceCol == startC) break;
        pair<int,int> parentEntry = parentCell[traceRow][traceCol];
        int parentRow = parentEntry.first;
        int parentCol = parentEntry.second;
        traceRow = parentRow;
        traceCol = parentCol;
    }

    reverse(tracedPath.begin(), tracedPath.end());

    pathLength = tracedPath.size();

    int fillIdx = 0;
    while (fillIdx < pathLength && fillIdx < 10000) {
        pathResult[fillIdx][0] = tracedPath[fillIdx].first;
        pathResult[fillIdx][1] = tracedPath[fillIdx].second;
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
