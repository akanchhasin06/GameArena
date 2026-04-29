#include <emscripten.h>
#include <vector>
#include <algorithm>
using namespace std;

int visitedOrder[10000][2];
int visitedCount = 0;
int pathResult[10000][2];
int pathLength = 0;

static vector<vector<bool>> cellSeen;
static vector<vector<pair<int,int>>> cellParent;
static vector<vector<int>> globalGrid;
static int globalRowCount;
static int globalColCount;
static int targetRow;
static int targetCol;
static bool solutionFound;

static int rowOffset[4] = {-1, 1, 0, 0};
static int colOffset[4] = {0, 0, -1, 1};

static bool isWallCell(int value) {
    return value == 1;
}

static bool positionValid(int row, int col) {
    if (row < 0) return false;
    if (row >= globalRowCount) return false;
    if (col < 0) return false;
    if (col >= globalColCount) return false;
    return true;
}

void exploreDepthFirst(int row, int col) {
    if (solutionFound == true) return;

    cellSeen[row][col] = true;

    if (visitedCount < 10000) {
        visitedOrder[visitedCount][0] = row;
        visitedOrder[visitedCount][1] = col;
        visitedCount = visitedCount + 1;
    }

    if (row == targetRow && col == targetCol) {
        solutionFound = true;
        return;
    }

    int dirIdx = 0;
    while (dirIdx < 4) {
        if (solutionFound == true) return;

        int nextRow = row + rowOffset[dirIdx];
        int nextCol = col + colOffset[dirIdx];

        if (positionValid(nextRow, nextCol) == false) {
            dirIdx = dirIdx + 1;
            continue;
        }
        if (cellSeen[nextRow][nextCol] == true) {
            dirIdx = dirIdx + 1;
            continue;
        }
        if (isWallCell(globalGrid[nextRow][nextCol]) == true) {
            dirIdx = dirIdx + 1;
            continue;
        }

        cellParent[nextRow][nextCol] = make_pair(row, col);
        exploreDepthFirst(nextRow, nextCol);

        dirIdx = dirIdx + 1;
    }
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int runDFS(const char* gridStr, int rows, int cols,
           int startR, int startC, int endR, int endC) {

    visitedCount = 0;
    pathLength = 0;

    globalRowCount = rows;
    globalColCount = cols;
    targetRow = endR;
    targetCol = endC;
    solutionFound = false;

    globalGrid.assign(rows, vector<int>(cols));

    int ri = 0;
    while (ri < rows) {
        int ci = 0;
        while (ci < cols) {
            globalGrid[ri][ci] = gridStr[ri * cols + ci] - '0';
            ci = ci + 1;
        }
        ri = ri + 1;
    }

    if (isWallCell(globalGrid[startR][startC]) == true) return 0;
    if (isWallCell(globalGrid[endR][endC]) == true) return 0;

    cellSeen.assign(rows, vector<bool>(cols, false));
    cellParent.assign(rows, vector<pair<int,int>>(cols));

    int pi = 0;
    while (pi < rows) {
        int pj = 0;
        while (pj < cols) {
            cellParent[pi][pj] = make_pair(-1, -1);
            pj = pj + 1;
        }
        pi = pi + 1;
    }

    exploreDepthFirst(startR, startC);

    if (solutionFound == false) return 0;

    vector<pair<int,int>> recoveredPath;
    int traceRow = endR;
    int traceCol = endC;

    while ((traceRow == startR && traceCol == startC) == false) {
        pair<int,int> stepCell = make_pair(traceRow, traceCol);
        recoveredPath.push_back(stepCell);
        pair<int,int> ancestor = cellParent[traceRow][traceCol];
        if (ancestor.first == -1) return 0;
        traceRow = ancestor.first;
        traceCol = ancestor.second;
    }

    pair<int,int> originCell = make_pair(startR, startC);
    recoveredPath.push_back(originCell);
    reverse(recoveredPath.begin(), recoveredPath.end());

    pathLength = (int)recoveredPath.size();

    int fillIdx = 0;
    while (fillIdx < pathLength && fillIdx < 10000) {
        pathResult[fillIdx][0] = recoveredPath[fillIdx].first;
        pathResult[fillIdx][1] = recoveredPath[fillIdx].second;
        fillIdx = fillIdx + 1;
    }

    return pathLength;
}

EMSCRIPTEN_KEEPALIVE int getVisitedCount()    { return visitedCount; }
EMSCRIPTEN_KEEPALIVE int getVisitedRow(int i) { return visitedOrder[i][0]; }
EMSCRIPTEN_KEEPALIVE int getVisitedCol(int i) { return visitedOrder[i][1]; }
EMSCRIPTEN_KEEPALIVE int getPathRow(int i)    { return pathResult[i][0]; }
EMSCRIPTEN_KEEPALIVE int getPathCol(int i)    { return pathResult[i][1]; }

}
