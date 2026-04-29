#include <emscripten.h>
#include <vector>
#include <queue>
#include <array>
#include <cmath>
#include <map>
using namespace std;

const int goalPattern[9] = {1, 2, 3, 4, 5, 6, 7, 8, 0};

static int stateSnapshots[1000][9];
static int movesList[1000];
static int movesCount;
static int statesExpanded;
static int pathCost;

static int computeLowerBound(const int boardState[9]) {
    int totalDistance = 0;
    int tileIdx = 0;
    while (tileIdx < 9) {
        if (boardState[tileIdx] != 0) {
            int targetPosition = boardState[tileIdx] - 1;
            int targetRow = targetPosition / 3;
            int currentRow = tileIdx / 3;
            int targetCol = targetPosition % 3;
            int currentCol = tileIdx % 3;
            int rowDistance = abs(currentRow - targetRow);
            int colDistance = abs(currentCol - targetCol);
            totalDistance = totalDistance + rowDistance + colDistance;
        }
        tileIdx = tileIdx + 1;
    }
    return totalDistance;
}

static bool boardMatchesGoal(const int boardState[9]) {
    int idx = 0;
    while (idx < 9) {
        if (boardState[idx] != goalPattern[idx]) return false;
        idx = idx + 1;
    }
    return true;
}

static string encodeBoard(const int boardState[9]) {
    string encoded;
    int idx = 0;
    while (idx < 9) {
        encoded = encoded + char(boardState[idx] + '0');
        idx = idx + 1;
    }
    return encoded;
}

static int findBlankPosition(const int boardState[9]) {
    int idx = 0;
    while (idx < 9) {
        if (boardState[idx] == 0) return idx;
        idx = idx + 1;
    }
    return -1;
}

static bool inversionsEven(const int boardState[9]) {
    int invCount = 0;
    int outer = 0;
    while (outer < 9) {
        int inner = outer + 1;
        while (inner < 9) {
            bool bothNonZero = boardState[outer] != 0 && boardState[inner] != 0;
            if (bothNonZero == true && boardState[outer] > boardState[inner]) {
                invCount = invCount + 1;
            }
            inner = inner + 1;
        }
        outer = outer + 1;
    }
    return invCount % 2 == 0;
}

struct BoundNode {
    array<int, 9> boardTiles;
    int gCost;
    int boundEstimate;
    int blankPos;
    vector<int> moveHistory;
    vector<array<int, 9>> boardHistory;

    int totalCost() const { return gCost + boundEstimate; }
    bool operator>(const BoundNode& other) const { return totalCost() > other.totalCost(); }
};

extern "C" {

EMSCRIPTEN_KEEPALIVE
int runBranchBound(int s0, int s1, int s2, int s3, int s4, int s5, int s6, int s7, int s8) {
    int initialTiles[9] = {s0, s1, s2, s3, s4, s5, s6, s7, s8};

    if (inversionsEven(initialTiles) == false) return -1;

    movesCount = 0;
    statesExpanded = 0;
    pathCost = 0;

    BoundNode rootNode;
    int fi = 0;
    while (fi < 9) {
        rootNode.boardTiles[fi] = initialTiles[fi];
        fi = fi + 1;
    }
    rootNode.gCost = 0;
    rootNode.boundEstimate = computeLowerBound(rootNode.boardTiles.data());
    rootNode.blankPos = findBlankPosition(rootNode.boardTiles.data());
    rootNode.boardHistory.push_back(rootNode.boardTiles);

    priority_queue<BoundNode, vector<BoundNode>, greater<BoundNode>> expansionQueue;
    expansionQueue.push(rootNode);

    map<string, int> bestKnownCost;
    int movementOffsets[4] = {-3, 3, -1, 1};

    while (expansionQueue.empty() == false) {
        BoundNode currentNode = expansionQueue.top();
        expansionQueue.pop();
        statesExpanded = statesExpanded + 1;

        if (boardMatchesGoal(currentNode.boardTiles.data()) == true) {
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

        string currentKey = encodeBoard(currentNode.boardTiles.data());
        bool alreadyBetter = bestKnownCost.count(currentKey) > 0 && bestKnownCost[currentKey] <= currentNode.gCost;
        if (alreadyBetter == true) continue;
        bestKnownCost[currentKey] = currentNode.gCost;

        int blankPosition = currentNode.blankPos;

        int dirIdx = 0;
        while (dirIdx < 4) {
            int newBlankPos = blankPosition + movementOffsets[dirIdx];

            if (newBlankPos < 0 || newBlankPos > 8) {
                dirIdx = dirIdx + 1;
                continue;
            }
            if (dirIdx == 2 && blankPosition % 3 == 0) {
                dirIdx = dirIdx + 1;
                continue;
            }
            if (dirIdx == 3 && blankPosition % 3 == 2) {
                dirIdx = dirIdx + 1;
                continue;
            }

            BoundNode childNode = currentNode;
            int tempValue = childNode.boardTiles[blankPosition];
            childNode.boardTiles[blankPosition] = childNode.boardTiles[newBlankPos];
            childNode.boardTiles[newBlankPos] = tempValue;
            childNode.blankPos = newBlankPos;
            childNode.gCost = currentNode.gCost + 1;
            childNode.boundEstimate = computeLowerBound(childNode.boardTiles.data());
            childNode.moveHistory.push_back(childNode.boardTiles[blankPosition]);
            childNode.boardHistory.push_back(childNode.boardTiles);

            string childKey = encodeBoard(childNode.boardTiles.data());
            bool childHasBetter = bestKnownCost.count(childKey) > 0 && bestKnownCost[childKey] <= childNode.gCost;
            if (childHasBetter == false) {
                expansionQueue.push(childNode);
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
