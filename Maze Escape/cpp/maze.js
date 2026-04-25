// ═══════════════════════════════════════
// MAZE ESCAPE — maze.js
// ═══════════════════════════════════════

let selectedAlgo = "bfs";
let animInterval = null;
let isRunning = false;
let canvas, pen;

// ═══════════════════════════════════════
// INIT ON PAGE LOAD
// ═══════════════════════════════════════
document.addEventListener("DOMContentLoaded", function() {
    canvas = document.getElementById("mycanvas");
    pen = canvas.getContext("2d");

    makegrid();
    drawgrid(canvas, pen);

    canvas.addEventListener("click", function(e) {
        mouseclick(canvas, e, pen);
    });

    document.getElementById("speedSlider").addEventListener("input", function() {
        document.getElementById("speedLabel").textContent = this.value + "ms";
    });
});

// ═══════════════════════════════════════
// ALGORITHM SELECTOR
// ═══════════════════════════════════════
function selectAlgo(algo) {
    selectedAlgo = algo;

    document.getElementById("bfsBtn").classList.remove("active");
    document.getElementById("dfsBtn").classList.remove("active");
    document.getElementById("dijkstraBtn").classList.remove("active");
    document.getElementById("astarBtn").classList.remove("active");

    document.getElementById(algo + "Btn").classList.add("active");
    document.getElementById("algoName").textContent = algo.toUpperCase();
}

// ═══════════════════════════════════════
// RESET STATS
// ═══════════════════════════════════════
function resetStats() {
    document.getElementById("nodesVisited").textContent = "0";
    document.getElementById("pathLength").textContent = "0";
    document.getElementById("timeTaken").textContent = "0ms";
}

// ═══════════════════════════════════════
// UPDATE STATS
// ═══════════════════════════════════════
function updateStats(nodes, pathLen, time) {
    document.getElementById("nodesVisited").textContent = nodes;
    document.getElementById("pathLength").textContent = pathLen;
    document.getElementById("timeTaken").textContent = time + "ms";
}

// ═══════════════════════════════════════
// CLEAR VISITED CELLS
// Only clears visited/path — walls are
// never touched. Start/end are restored.
// ═══════════════════════════════════════
function clearVisited() {
    for (let i = 0; i < row; i++) {
        for (let j = 0; j < col; j++) {
            // Only reset visited/path — never touch walls
            if (grid[i][j] === cell.visited || grid[i][j] === cell.path) {
                grid[i][j] = cell.empty;
            }
        }
    }
    // Restore start/end markers in case they were overwritten
    if (startRow !== -1) grid[startRow][startCol] = cell.start;
    if (endRow   !== -1) grid[endRow][endCol]     = cell.end;
    drawgrid(canvas, pen);
    resetStats();
}

// ═══════════════════════════════════════
// RESET ENTIRE MAZE
// ═══════════════════════════════════════
function resetMaze() {
    if(animInterval) clearInterval(animInterval);
    isRunning = false;
    startRow=-1; startCol=-1;
    endRow=-1; endCol=-1;
    makegrid();
    drawgrid(canvas, pen);
    resetStats();
}

// ═══════════════════════════════════════
// CONVERT GRID TO STRING FOR C++
// ═══════════════════════════════════════
function gridToString() {
    let gridStr = "";
    for(let i=0; i<row; i++) {
        for(let j=0; j<col; j++) {
            gridStr += grid[i][j].toString();
        }
    }
    return gridStr;
}

// ═══════════════════════════════════════
// ANIMATE VISITED + PATH
// Two-phase animation:
//   Phase 1 — visited nodes (base speed)
//   Phase 2 — final path   (slightly slower for clarity)
// Both phases skip wall cells entirely.
// ═══════════════════════════════════════
function animateResult(visited, path, timeTaken) {
    let visitedSpeed = parseInt(document.getElementById("speedSlider").value);
    let pathSpeed    = visitedSpeed + 20; // path phase is slightly slower

    let step = 0;
    let totalSteps = visited.length + path.length;

    function tick() {
        if (step >= totalSteps) {
            // Animation complete
            isRunning = false;
            updateStats(visited.length, path.length, timeTaken);
            document.getElementById("mazeMsg").textContent =
                path.length > 0
                    ? "✅ Path found in " + path.length + " steps!"
                    : "❌ No path found!";
            return;
        }

        if (step < visited.length) {
            // ── Phase 1: visited nodes ──
            let r = visited[step][0];
            let c = visited[step][1];

            // Never mark a wall cell as visited
            if (grid[r][c] !== cell.wall &&
                grid[r][c] !== cell.start &&
                grid[r][c] !== cell.end) {
                grid[r][c] = cell.visited;
            }

            drawgrid(canvas, pen);
            document.getElementById("nodesVisited").textContent = step + 1;
            step++;
            animInterval = setTimeout(tick, visitedSpeed);

        } else {
            // ── Phase 2: final path ──
            let pathStep = step - visited.length;
            let r = path[pathStep][0];
            let c = path[pathStep][1];

            // Strict wall guard — never draw path on a wall cell
            if (grid[r][c] !== cell.wall &&
                grid[r][c] !== cell.start &&
                grid[r][c] !== cell.end) {
                grid[r][c] = cell.path;
            }

            drawgrid(canvas, pen);
            step++;
            animInterval = setTimeout(tick, pathSpeed);
        }
    }

    tick();
}

// ═══════════════════════════════════════
// RUN ALGORITHM — connects to C++ WASM
// ═══════════════════════════════════════
function runAlgorithm(instance, funcName) {
    let gridStr = gridToString();
    let startTime = performance.now();

    // allocate string in WASM memory
    let strPtr = instance._malloc(gridStr.length + 1);
    for(let i=0; i<gridStr.length; i++) {
        instance.HEAPU8[strPtr + i] = gridStr.charCodeAt(i);
    }
    instance.HEAPU8[strPtr + gridStr.length] = 0; // null terminator

    let pathLen = instance[funcName](
        strPtr, row, col, startRow, startCol, endRow, endCol
    );

    instance._free(strPtr);

    let endTime = performance.now();
    let timeTaken = (endTime - startTime).toFixed(2);

    // get visited nodes
    let visitedCount = instance._getVisitedCount();
    let visited = [];
    for(let i=0; i<visitedCount; i++) {
        visited.push([instance._getVisitedRow(i), instance._getVisitedCol(i)]);
    }

    // get path
    let path = [];
    for(let i=0; i<pathLen; i++) {
        path.push([instance._getPathRow(i), instance._getPathCol(i)]);
    }

    return { visited, path, timeTaken };
}

// ═══════════════════════════════════════
// RUN MAZE
// ═══════════════════════════════════════
function runMaze() {
    if(isRunning) return;

    if(startRow === -1 || startCol === -1) {
        alert("Place a Start point first!");
        return;
    }
    if(endRow === -1 || endCol === -1) {
        alert("Place an End point first!");
        return;
    }

    clearVisited();
    isRunning = true;
    document.getElementById("mazeMsg").textContent = "⏳ Running " + selectedAlgo.toUpperCase() + "...";

    if(selectedAlgo === "bfs") {
        BFSModule().then(function(instance) {
            let result = runAlgorithm(instance, "_runBFS");
            animateResult(result.visited, result.path, result.timeTaken);
        });
    } else if(selectedAlgo === "dfs") {
        DFSModule().then(function(instance) {
            let result = runAlgorithm(instance, "_runDFS");
            animateResult(result.visited, result.path, result.timeTaken);
        });
    } else if(selectedAlgo === "dijkstra") {
        DijkstraModule().then(function(instance) {
            let result = runAlgorithm(instance, "_runDijkstra");
            animateResult(result.visited, result.path, result.timeTaken);
        });
    } else if(selectedAlgo === "astar") {
        AstarModule().then(function(instance) {
            let result = runAlgorithm(instance, "_runAstar");
            animateResult(result.visited, result.path, result.timeTaken);
        });
    }
}