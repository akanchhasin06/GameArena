#include <emscripten.h>
#include <vector>
#include <queue>
using namespace std;

// ═══════════════════════════════════════════
// MAZE ESCAPE — BFS Algorithm
// Written by: Aayushi
// ═══════════════════════════════════════════

// Grid dimensions — dont change these
int ROWS = 15;
int COLS = 15;

// Result arrays — JS reads these
int visitedOrder[225][2];   // stores visited cells in order
int visitedCount = 0;

int pathResult[225][2];     // stores final path
int pathLength = 0;

// ═══════════════════════════════════════════
// WRITE YOUR BFS LOGIC INSIDE THIS FUNCTION
// grid: 0=empty, 1=wall, 2=start, 3=end
// ═══════════════════════════════════════════
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    int runBFS(int* grid, int startRow, int startCol, 
                          int endRow,   int endCol) {
        
        // ↓↓↓ Aayushi writes BFS logic here ↓↓↓
        
        
        // ↑↑↑ end of logic ↑↑↑
        
        return pathLength; // return 0 if no path found
    }

    // JS calls this to get visited cells one by one
    EMSCRIPTEN_KEEPALIVE
    int getVisitedRow(int index) { return visitedOrder[index][0]; }
    
    EMSCRIPTEN_KEEPALIVE
    int getVisitedCol(int index) { return visitedOrder[index][1]; }

    // JS calls this to get final path cells one by one
    EMSCRIPTEN_KEEPALIVE
    int getPathRow(int index) { return pathResult[index][0]; }
    
    EMSCRIPTEN_KEEPALIVE
    int getPathCol(int index) { return pathResult[index][1]; }

    EMSCRIPTEN_KEEPALIVE
    int getVisitedCount() { return visitedCount; }
}