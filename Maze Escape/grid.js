// ═══════════════════════════════════════
// MAZE ESCAPE — grid.js
// Layered rendering: walls always on top,
// path/visited never drawn over walls.
// ═══════════════════════════════════════

const cell = {
    empty:   0,
    wall:    1,
    start:   2,
    end:     3,
    visited: 4,
    path:    5
};

let row = 15, col = 15, cellsize = 40;
let grid = [];
let startRow = -1, startCol = -1;
let endRow   = -1, endCol   = -1;

// ── Debug mode: outlines walls in red if path tries to overlap ──
let debugMode = false;

// ═══════════════════════════════════════
// GRID INIT
// ═══════════════════════════════════════
function makegrid() {
    grid = [];
    for (let i = 0; i < row; i++) {
        let temp = [];
        for (let j = 0; j < col; j++) temp.push(cell.empty);
        grid.push(temp);
    }
}

// ═══════════════════════════════════════
// LAYERED DRAW
// Three-pass rendering ensures walls are
// never overwritten by path or visited.
//
//   Pass 1 — base layer  : empty cells
//   Pass 2 — overlay     : visited (semi-transparent purple)
//   Pass 3 — top layer   : path, start, end, walls (walls last)
// ═══════════════════════════════════════
function drawgrid(canvas, pen) {
    // Full clear once per frame
    pen.clearRect(0, 0, canvas.width, canvas.height);

    // ── Pass 1: base layer — empty cells ──
    pen.globalAlpha = 1;
    for (let i = 0; i < row; i++) {
        for (let j = 0; j < col; j++) {
            if (grid[i][j] === cell.empty) {
                let x = j * cellsize, y = i * cellsize;
                pen.fillStyle = "#1e293b"; // dark background
                pen.fillRect(x, y, cellsize - 1, cellsize - 1);
            }
        }
    }

    // ── Pass 2: overlay — visited nodes (semi-transparent) ──
    pen.globalAlpha = 0.5;
    for (let i = 0; i < row; i++) {
        for (let j = 0; j < col; j++) {
            // Guard: never draw visited on a wall cell
            if (grid[i][j] === cell.visited && grid[i][j] !== cell.wall) {
                let x = j * cellsize, y = i * cellsize;
                pen.fillStyle = "#a855f7"; // semi-transparent purple
                pen.fillRect(x, y, cellsize - 1, cellsize - 1);
            }
        }
    }

    // ── Pass 3: top layer — path, start, end, walls ──
    pen.globalAlpha = 1;
    for (let i = 0; i < row; i++) {
        for (let j = 0; j < col; j++) {
            let v = grid[i][j];
            let x = j * cellsize, y = i * cellsize;

            if (v === cell.path) {
                // Strict wall guard — never paint path over a wall
                if (grid[i][j] === cell.wall) continue;
                pen.fillStyle = "#ffff00"; // bright yellow
                pen.fillRect(x, y, cellsize - 1, cellsize - 1);

            } else if (v === cell.start) {
                pen.fillStyle = "#22c55e"; // bright green
                pen.fillRect(x, y, cellsize - 1, cellsize - 1);

            } else if (v === cell.end) {
                pen.fillStyle = "#ef4444"; // bright red
                pen.fillRect(x, y, cellsize - 1, cellsize - 1);

            } else if (v === cell.wall) {
                pen.fillStyle = "#000000"; // solid black
                pen.fillRect(x, y, cellsize - 1, cellsize - 1);

                // Debug mode: red outline on walls
                if (debugMode) {
                    pen.strokeStyle = "#ff0000";
                    pen.lineWidth = 2;
                    pen.strokeRect(x + 1, y + 1, cellsize - 3, cellsize - 3);
                }
            }
        }
    }

    // Reset alpha to safe default
    pen.globalAlpha = 1;
}

// ═══════════════════════════════════════
// MOUSE CLICK HANDLER
// ═══════════════════════════════════════
let currState = "wall";

function mouseclick(canvas, event, pen) {
    let clickedCol = Math.floor(event.offsetX / cellsize);
    let clickedRow = Math.floor(event.offsetY / cellsize);

    if (clickedRow < 0 || clickedRow >= row) return;
    if (clickedCol < 0 || clickedCol >= col) return;

    if (currState === "wall") {
        // Don't place walls on start/end
        if (grid[clickedRow][clickedCol] === cell.start ||
            grid[clickedRow][clickedCol] === cell.end) return;

        grid[clickedRow][clickedCol] =
            grid[clickedRow][clickedCol] === cell.wall ? cell.empty : cell.wall;

    } else if (currState === "start") {
        if (startRow !== -1) grid[startRow][startCol] = cell.empty;
        grid[clickedRow][clickedCol] = cell.start;
        startRow = clickedRow;
        startCol = clickedCol;

    } else if (currState === "end") {
        if (endRow !== -1) grid[endRow][endCol] = cell.empty;
        grid[clickedRow][clickedCol] = cell.end;
        endRow = clickedRow;
        endCol = clickedCol;
    }

    drawgrid(canvas, pen);
}

// ═══════════════════════════════════════
// DEBUG TOGGLE
// Call toggleDebug() from console or wire
// to a button to highlight wall conflicts.
// ═══════════════════════════════════════
function toggleDebug() {
    debugMode = !debugMode;
    console.log("Debug mode:", debugMode ? "ON" : "OFF");
}
