#include <emscripten.h>
#include <vector>
#include <queue>
#include <array>
#include <cmath>
#include <map>
using namespace std;

const int goalState[9] = {1, 2, 3, 4, 5, 6, 7, 8, 0};

static int stateSnapshots[1000][9];
static int movesList[1000];
static int movesCount;
static int statesExpanded;
static int pathCost;

static int computeManhattanDistance(const int boardState[9]) {
    int totalDist = 0;
    int tileIdx = 0;
    while (tileIdx < 9) {
        if (boardState[tileIdx] != 0) {
            int tileValue = boardState[tileIdx] - 1;
            int targetRow = tileValue / 3;
            int currentRow = tileIdx / 3;
            int targetCol = tileValue % 3;
            int currentCol = tileIdx % 3;
            int rowDist = abs(currentRow - targetRow);
            int colDist = abs(currentCol - targetCol);
            totalDist = totalDist + rowDist + colDist;
        }
        tileIdx = tileIdx + 1;
    }
    return totalDist;
}

static int computeMisplacedTiles(const int boardState[9]) {
    int misplacedCount = 0;
    int tileIdx = 0;
    while (tileIdx < 9) {
        if (boardState[tileIdx] != 0 && boardState[tileIdx] != goalState[tileIdx]) {
            misplacedCount = misplacedCount + 1;
        }
        tileIdx = tileIdx + 1;
    }
    return misplacedCount;
}

static int selectHeuristic(const int boardState[9], int heuristicType) {
    if (heuristicType == 0) {
        return computeManhattanDistance(boardState);
    }
    return computeMisplacedTiles(boardState);
}

static bool boardIsGoal(const int boardState[9]) {
    int idx = 0;
    while (idx < 9) {
        if (boardState[idx] != goalState[idx]) return false;
        idx = idx + 1;
    }
    return true;
}

static string boardToKey(const int boardState[9]) {
    string keyStr;
    int idx = 0;
    while (idx < 9) {
        keyStr = keyStr + char(boardState[idx] + '0');
        idx = idx + 1;
    }
    return keyStr;
}

static int locateBlankTile(const int boardState[9]) {
    int idx = 0;
    while (idx < 9) {
        if (boardState[idx] == 0) return idx;
        idx = idx + 1;
    }
    return -1;
}

static bool puzzleIsSolvable(const int boardState[9]) {
    int inversionCount = 0;
    int outer = 0;
    while (outer < 9) {
        int inner = outer + 1;
        while (inner < 9) {
            if (boardState[outer] != 0 && boardState[inner] != 0) {
                if (boardState[outer] > boardState[inner]) {
                    inversionCount = inversionCount + 1;
                }
            }
            inner = inner + 1;
        }
        outer = outer + 1;
    }
    return inversionCount % 2 == 0;
}

struct SearchNode {
    array<int, 9> boardTiles;
    int gCost;
    int hCost;
    int blankPosition;
    vector<int> moveHistory;
    vector<array<int, 9>> boardHistory;

    int totalCost() const { return gCost + hCost; }
    bool operator>(const SearchNode& other) const { return totalCost() > other.totalCost(); }
};

extern "C" {

EMSCRIPTEN_KEEPALIVE
int runAstar(int s0, int s1, int s2, int s3, int s4, int s5, int s6, int s7, int s8, int hType) {
    int initialBoard[9] = {s0, s1, s2, s3, s4, s5, s6, s7, s8};

    if (puzzleIsSolvable(initialBoard) == false) return -1;

    movesCount = 0;
    statesExpanded = 0;
    pathCost = 0;

    SearchNode startNode;
    int fillIdx = 0;
    while (fillIdx < 9) {
        startNode.boardTiles[fillIdx] = initialBoard[fillIdx];
        fillIdx = fillIdx + 1;
    }
    startNode.gCost = 0;
    startNode.hCost = selectHeuristic(startNode.boardTiles.data(), hType);
    startNode.blankPosition = locateBlankTile(startNode.boardTiles.data());
    startNode.boardHistory.push_back(startNode.boardTiles);

    priority_queue<SearchNode, vector<SearchNode>, greater<SearchNode>> openQueue;
    openQueue.push(startNode);

    map<string, bool> visitedStates;
    int slideDirs[4] = {-3, 3, -1, 1};

    while (openQueue.empty() == false) {
        SearchNode currentNode = openQueue.top();
        openQueue.pop();
        statesExpanded = statesExpanded + 1;

        if (boardIsGoal(currentNode.boardTiles.data()) == true) {
            pathCost = currentNode.gCost;
            movesCount = (int)currentNode.moveHistory.size();

            int stepIdx = 0;
            while (stepIdx < movesCount && stepIdx < 1000) {
                movesList[stepIdx] = currentNode.moveHistory[stepIdx];
                int cellIdx = 0;
                while (cellIdx < 9) {
                    stateSnapshots[stepIdx][cellIdx] = currentNode.boardHistory[stepIdx + 1][cellIdx];
                    cellIdx = cellIdx + 1;
                }
                stepIdx = stepIdx + 1;
            }
            return movesCount;
        }

        string currentKey = boardToKey(currentNode.boardTiles.data());
        if (visitedStates[currentKey] == true) continue;
        visitedStates[currentKey] = true;

        int blankPos = currentNode.blankPosition;

        int dirIdx = 0;
        while (dirIdx < 4) {
            int newBlankPos = blankPos + slideDirs[dirIdx];

            if (newBlankPos < 0 || newBlankPos > 8) {
                dirIdx = dirIdx + 1;
                continue;
            }
            if (dirIdx == 2 && blankPos % 3 == 0) {
                dirIdx = dirIdx + 1;
                continue;
            }
            if (dirIdx == 3 && blankPos % 3 == 2) {
                dirIdx = dirIdx + 1;
                continue;
            }

            SearchNode nextNode = currentNode;
            int tempTile = nextNode.boardTiles[blankPos];
            nextNode.boardTiles[blankPos] = nextNode.boardTiles[newBlankPos];
            nextNode.boardTiles[newBlankPos] = tempTile;
            nextNode.blankPosition = newBlankPos;
            nextNode.gCost = currentNode.gCost + 1;
            nextNode.hCost = selectHeuristic(nextNode.boardTiles.data(), hType);
            nextNode.moveHistory.push_back(nextNode.boardTiles[blankPos]);
            nextNode.boardHistory.push_back(nextNode.boardTiles);

            string neighborKey = boardToKey(nextNode.boardTiles.data());
            if (visitedStates[neighborKey] == false) {
                openQueue.push(nextNode);
            }

            dirIdx = dirIdx + 1;
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
