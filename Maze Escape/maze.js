// ═══════════════════════════════════════════════════════════════
// MAZE ESCAPE — maze.js
// Uses a single compiled WASM module (maze_algos.js / .wasm)
// Grid is passed as an Int32Array written directly into WASM heap.
// ═══════════════════════════════════════════════════════════════

// ── Cell state constants ─────────────────────────────────────
const EMPTY   = 0;
const WALL    = 1;
const START   = 2;
const END     = 3;
const VISITED = 4;
const PATH    = 5;

// ── Grid config ──────────────────────────────────────────────
const ROWS = 15, COLS = 15, CELL = 40;
let grid = [];
let startRow = 0,  startCol = 0;
let endRow   = 14, endCol   = 14;
let currMode = "wall";
let isRunning = false;

// ── Canvas ───────────────────────────────────────────────────
const canvas = document.getElementById("mycanvas");
const ctx    = canvas.getContext("2d");

// ── Colors ───────────────────────────────────────────────────
const COLOR = {
    [EMPTY]:   "#1e293b",
    [WALL]:    "#000000",
    [START]:   "#22c55e",
    [END]:     "#ef4444",
    [VISITED]: "#a855f7",
    [PATH]:    "#ffff00"
};

// ═══════════════════════════════════════════════════════════════
// WASM MODULE — wait for it to be ready before allowing runs
// ═══════════════════════════════════════════════════════════════
let wasmReady = false;

// maze_algos.js sets window.Module; we hook onRuntimeInitialized
if (typeof Module !== "undefined") {
    Module["onRuntimeInitialized"] = function() {
        wasmReady = true;
        setMsg("WASM ready. Draw walls, set Start & End, then run.");
    };
} else {
    // Module not yet defined — create a stub that gets populated
    window.Module = {
        onRuntimeInitialized: function() {
            wasmReady = true;
            setMsg("WASM ready. Draw walls, set Start & End, then run.");
        }
    };
}

// ═══════════════════════════════════════════════════════════════
// GRID INIT
// ═══════════════════════════════════════════════════════════════
function makegrid() {
    grid = [];
    for (let r = 0; r < ROWS; r++) {
        let row = [];
        for (let c = 0; c < COLS; c++)
            row.push(Math.random() < 0.28 ? WALL : EMPTY);
        grid.push(row);
    }
    grid[startRow][startCol] = START;
    grid[endRow][endCol]     = END;
}

// ═══════════════════════════════════════════════════════════════
// DRAW — single pass, walls always on top
// ═══════════════════════════════════════════════════════════════
function draw() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    for (let r = 0; r < ROWS; r++) {
        for (let c = 0; c < COLS; c++) {
            let v = grid[r][c];
            let x = c * CELL, y = r * CELL;

            if (v === VISITED) {
                // semi-transparent purple over dark background
                ctx.fillStyle = "#1e293b";
                ctx.fillRect(x, y, CELL - 1, CELL - 1);
                ctx.globalAlpha = 0.6;
                ctx.fillStyle = COLOR[VISITED];
                ctx.fillRect(x, y, CELL - 1, CELL - 1);
                ctx.globalAlpha = 1;
            } else {
                ctx.fillStyle = COLOR[v] || COLOR[EMPTY];
                ctx.fillRect(x, y, CELL - 1, CELL - 1);
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════
// MODE BUTTONS
// ═══════════════════════════════════════════════════════════════
function setMode(mode) {
    currMode = mode;
    ["wall","start","end"].forEach(m => {
        let btn = document.getElementById("btn" + m.charAt(0).toUpperCase() + m.slice(1));
        if (btn) btn.classList.toggle("active", m === mode);
    });
}

// ═══════════════════════════════════════════════════════════════
// CANVAS CLICK
// ═══════════════════════════════════════════════════════════════
canvas.addEventListener("click", e => {
    if (isRunning) return;
    let c = Math.floor(e.offsetX / CELL);
    let r = Math.floor(e.offsetY / CELL);
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return;

    if (currMode === "wall") {
        if (grid[r][c] === START || grid[r][c] === END) return;
        grid[r][c] = grid[r][c] === WALL ? EMPTY : WALL;

    } else if (currMode === "start") {
        grid[startRow][startCol] = EMPTY;
        startRow = r; startCol = c;
        grid[r][c] = START;

    } else if (currMode === "end") {
        grid[endRow][endCol] = EMPTY;
        endRow = r; endCol = c;
        grid[r][c] = END;
    }
    draw();
});

// ═══════════════════════════════════════════════════════════════
// PASS GRID TO WASM
// Writes a flat Int32Array into WASM heap memory.
// Returns the heap pointer (caller must _free it).
// ═══════════════════════════════════════════════════════════════
function allocGrid() {
    const bytes = ROWS * COLS * 4; // Int32 = 4 bytes each
    const ptr   = Module._malloc(bytes);
    for (let r = 0; r < ROWS; r++) {
        for (let c = 0; c < COLS; c++) {
            let v = grid[r][c];
            // Treat start/end as passable (0) for the algorithm
            let val = (v === WALL) ? 1 : 0;
            Module.HEAP32[(ptr >> 2) + r * COLS + c] = val;
        }
    }
    return ptr;
}

// ═══════════════════════════════════════════════════════════════
// ANIMATION — visited phase then path phase
// ═══════════════════════════════════════════════════════════════
function delay(ms) { return new Promise(r => setTimeout(r, ms)); }

async function animate(visitedCount, pathLen) {
    // Phase 1: visited nodes (purple)
    for (let i = 0; i < visitedCount; i++) {
        let r = Module._getVisitedRow(i);
        let c = Module._getVisitedCol(i);
        if (grid[r][c] === EMPTY) grid[r][c] = VISITED;
        draw();
        await delay(18);
    }
    // Phase 2: final path (yellow) — slower so it's clearly visible
    for (let i = 0; i < pathLen; i++) {
        let r = Module._getPathRow(i);
        let c = Module._getPathCol(i);
        if (grid[r][c] !== START && grid[r][c] !== END)
            grid[r][c] = PATH;
        draw();
        await delay(45);
    }
}

// ═══════════════════════════════════════════════════════════════
// RUN SINGLE ALGORITHM
// ═══════════════════════════════════════════════════════════════
async function runAlgo(type) {
    if (!wasmReady) { setMsg("⏳ WASM still loading, please wait…"); return; }
    if (isRunning)  { setMsg("⏳ Already running…"); return; }

    clearVisited();
    isRunning = true;
    setMsg("⏳ Running " + type.toUpperCase() + "…");

    const ptr = allocGrid();
    let pathLen = 0;

    if      (type === "bfs")      pathLen = Module._runBFS(ptr, ROWS, COLS, startRow, startCol, endRow, endCol);
    else if (type === "dfs")      pathLen = Module._runDFS(ptr, ROWS, COLS, startRow, startCol, endRow, endCol);
    else if (type === "dijkstra") pathLen = Module._runDijkstra(ptr, ROWS, COLS, startRow, startCol, endRow, endCol);
    else                          pathLen = Module._runAStar(ptr, ROWS, COLS, startRow, startCol, endRow, endCol);

    Module._free(ptr);

    const nodes = Module._getVisitedCount();
    document.getElementById("nodesVisited").innerText = nodes;
    document.getElementById("pathLength").innerText   = pathLen;

    await animate(nodes, pathLen);

    update(type, nodes, pathLen);
    isRunning = false;
    setMsg(pathLen > 0
        ? "✅ " + type.toUpperCase() + " found path in " + pathLen + " steps, visited " + nodes + " nodes."
        : "❌ " + type.toUpperCase() + " — no path found!");
}

// ═══════════════════════════════════════════════════════════════
// RUN ALL — instant stats, no animation
// ═══════════════════════════════════════════════════════════════
function runAll() {
    if (!wasmReady) { setMsg("⏳ WASM still loading…"); return; }
    if (isRunning)  return;

    clearVisited();

    const algos = [
        { key:"bfs",      fn: ptr => Module._runBFS(ptr,      ROWS,COLS,startRow,startCol,endRow,endCol), nId:"bfsNodes",   pId:"bfsPath"   },
        { key:"dfs",      fn: ptr => Module._runDFS(ptr,      ROWS,COLS,startRow,startCol,endRow,endCol), nId:"dfsNodes",   pId:"dfsPath"   },
        { key:"dijkstra", fn: ptr => Module._runDijkstra(ptr, ROWS,COLS,startRow,startCol,endRow,endCol), nId:"dijNodes",   pId:"dijPath"   },
        { key:"astar",    fn: ptr => Module._runAStar(ptr,    ROWS,COLS,startRow,startCol,endRow,endCol), nId:"astarNodes", pId:"astarPath" },
    ];

    algos.forEach(a => {
        const ptr = allocGrid();
        const p   = a.fn(ptr);
        const n   = Module._getVisitedCount();
        Module._free(ptr);
        document.getElementById(a.nId).innerText = n;
        document.getElementById(a.pId).innerText = p;
    });

    setMsg("✅ All algorithms complete. Check comparison table.");
}

// ═══════════════════════════════════════════════════════════════
// HELPERS
// ═══════════════════════════════════════════════════════════════
function update(type, n, p) {
    const map = { bfs:["bfsNodes","bfsPath"], dfs:["dfsNodes","dfsPath"],
                  dijkstra:["dijNodes","dijPath"], astar:["astarNodes","astarPath"] };
    if (map[type]) {
        document.getElementById(map[type][0]).innerText = n;
        document.getElementById(map[type][1]).innerText = p;
    }
}

function clearVisited() {
    for (let r = 0; r < ROWS; r++)
        for (let c = 0; c < COLS; c++)
            if (grid[r][c] === VISITED || grid[r][c] === PATH)
                grid[r][c] = EMPTY;
    grid[startRow][startCol] = START;
    grid[endRow][endCol]     = END;
    draw();
}

function resetMaze() {
    isRunning = false;
    makegrid();
    draw();
    setMsg("Maze reset. Draw walls, set Start & End, then run.");
}

function setMsg(txt) {
    const el = document.getElementById("msg");
    if (el) el.textContent = txt;
}

// ═══════════════════════════════════════════════════════════════
// STARTUP
// ═══════════════════════════════════════════════════════════════
makegrid();
draw();
setMode("wall");
