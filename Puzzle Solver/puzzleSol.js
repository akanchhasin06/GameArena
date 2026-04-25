// ═══════════════════════════════════════════════════════════════
// PUZZLE SOLVER — puzzleSol.js
// Animates the full board state at every step using snapshots
// returned by the WASM module (getSnapshotCell).
// ═══════════════════════════════════════════════════════════════

let selectedAlgo = "A*";
let animTimer    = null;
let isAnimating  = false;

// ── Tile colours ────────────────────────────────────────────
const TILE_NORMAL  = "#5c6bc0";
const TILE_EMPTY   = "#1e1e2f";
const TILE_MOVED   = "#f59e0b";   // highlight the tile that just moved
const TILE_GOAL    = "#22c55e";   // flash green when solved

// ═══════════════════════════════════════════════════════════════
// GET / SET PUZZLE STATE
// ═══════════════════════════════════════════════════════════════
function getPuzzleState() {
    return Array.from(document.querySelectorAll(".tile")).map(t => {
        let v = parseInt(t.value);
        return isNaN(v) ? 0 : v;
    });
}

function setPuzzleState(state, movedTile = -1) {
    document.querySelectorAll(".tile").forEach((tile, i) => {
        let v = state[i];
        if (v === 0) {
            tile.value = "";
            tile.style.background = TILE_EMPTY;
            tile.style.color      = "transparent";
            tile.style.transform  = "scale(1)";
        } else {
            tile.value = v;
            tile.style.color = "white";
            if (v === movedTile) {
                // highlight the tile that slid into the blank
                tile.style.background = TILE_MOVED;
                tile.style.transform  = "scale(1.08)";
            } else {
                tile.style.background = TILE_NORMAL;
                tile.style.transform  = "scale(1)";
            }
        }
    });
}

function flashGoal() {
    document.querySelectorAll(".tile").forEach(tile => {
        if (tile.value !== "" && tile.value !== "0") {
            tile.style.background = TILE_GOAL;
            tile.style.transform  = "scale(1.05)";
        }
    });
    setTimeout(() => setPuzzleState([1,2,3,4,5,6,7,8,0]), 800);
}

// ═══════════════════════════════════════════════════════════════
// SHUFFLE / RESET
// ═══════════════════════════════════════════════════════════════
function shuffle() {
    if (isAnimating) return;
    // Keep shuffling until we get a solvable permutation
    let nums;
    do {
        nums = [0,1,2,3,4,5,6,7,8];
        for (let i = nums.length - 1; i > 0; i--) {
            let j = Math.floor(Math.random() * (i + 1));
            [nums[i], nums[j]] = [nums[j], nums[i]];
        }
    } while (!isSolvable(nums));
    setPuzzleState(nums);
    clearStats();
    setStepsMsg("Shuffled! Press ▶ Solve.");
}

function reset() {
    if (isAnimating) stopSolver();
    setPuzzleState([1,2,3,4,5,6,7,8,0]);
    clearStats();
    setStepsMsg("Reset to goal state.");
}

// ═══════════════════════════════════════════════════════════════
// SOLVABILITY CHECK (JS side — for shuffle)
// ═══════════════════════════════════════════════════════════════
function isSolvable(state) {
    let inv = 0;
    for (let i = 0; i < 9; i++)
        for (let j = i + 1; j < 9; j++)
            if (state[i] && state[j] && state[i] > state[j]) inv++;
    return inv % 2 === 0;
}

function isValidState(state) {
    return [...state].sort((a,b)=>a-b).every((v,i) => v === i);
}

// ═══════════════════════════════════════════════════════════════
// ALGORITHM SELECTOR
// ═══════════════════════════════════════════════════════════════
function selectALGO(algo) {
    selectedAlgo = algo;
    ["Astarbtn","BBbtn","cmpbtn"].forEach(id =>
        document.getElementById(id).classList.remove("active"));

    if (algo === "A*") {
        document.getElementById("Astarbtn").classList.add("active");
        document.getElementById("heuristicPanel").style.display = "block";
    } else if (algo === "BranchBound") {
        document.getElementById("BBbtn").classList.add("active");
        document.getElementById("heuristicPanel").style.display = "none";
    } else {
        document.getElementById("cmpbtn").classList.add("active");
        document.getElementById("heuristicPanel").style.display = "block";
    }
}

// ═══════════════════════════════════════════════════════════════
// SPEED SLIDER
// ═══════════════════════════════════════════════════════════════
document.getElementById("speedSlider").addEventListener("input", function() {
    document.getElementById("speedLabel").textContent = this.value + "ms";
});

// ═══════════════════════════════════════════════════════════════
// STATS HELPERS
// ═══════════════════════════════════════════════════════════════
function clearStats() {
    ["astarStates","astarMoves","astarCost","astarTime",
     "bbStates","bbMoves","bbCost","bbTime"].forEach(id => {
        document.getElementById(id).textContent = "—";
    });
    setStepsMsg("Waiting to solve...");
}

function updateAstarStats(states, moves, cost, time) {
    document.getElementById("astarStates").textContent = states;
    document.getElementById("astarMoves").textContent  = moves;
    document.getElementById("astarCost").textContent   = cost;
    document.getElementById("astarTime").textContent   = time + "ms";
}

function updateBBStats(states, moves, cost, time) {
    document.getElementById("bbStates").textContent = states;
    document.getElementById("bbMoves").textContent  = moves;
    document.getElementById("bbCost").textContent   = cost;
    document.getElementById("bbTime").textContent   = time + "ms";
}

function setStepsMsg(txt) {
    let el = document.getElementById("solutionSteps");
    el.textContent = txt;
    el.scrollTop = el.scrollHeight;
}

function appendStep(txt) {
    let el = document.getElementById("solutionSteps");
    el.textContent += txt + "\n";
    el.scrollTop = el.scrollHeight;
}

// ═══════════════════════════════════════════════════════════════
// CORE ANIMATION
// Plays back the full board snapshot at each step.
// instance  — the WASM module instance
// moveCount — number of steps
// algoName  — label for the steps panel
// onDone    — callback when animation finishes
// ═══════════════════════════════════════════════════════════════
function animateSolution(instance, moveCount, algoName, onDone) {
    let speed = parseInt(document.getElementById("speedSlider").value);
    let step  = 0;
    isAnimating = true;

    setStepsMsg("▶ Animating " + algoName + " — " + moveCount + " moves\n");

    function tick() {
        if (step >= moveCount) {
            // All steps done — flash green
            flashGoal();
            appendStep("✅ " + algoName + " solved in " + moveCount + " moves!");
            isAnimating = false;
            if (onDone) onDone();
            return;
        }

        // Read full board snapshot for this step from WASM
        let snapshot = [];
        for (let j = 0; j < 9; j++)
            snapshot.push(instance._getSnapshotCell(step, j));

        let movedTile = instance._getMoveAt(step);

        // Update the visual puzzle board
        setPuzzleState(snapshot, movedTile);

        // Log the step
        appendStep("Step " + (step + 1) + ": tile " + movedTile + " slides in");

        step++;
        animTimer = setTimeout(tick, speed);
    }

    tick();
}

// ═══════════════════════════════════════════════════════════════
// STOP
// ═══════════════════════════════════════════════════════════════
function stopSolver() {
    if (animTimer) { clearTimeout(animTimer); animTimer = null; }
    isAnimating = false;
    setStepsMsg("⏹ Stopped.");
}

// ═══════════════════════════════════════════════════════════════
// RUN A*
// ═══════════════════════════════════════════════════════════════
function runAstar(state, heuristic, onDone) {
    let hType = heuristic === "manhattan" ? 0 : 1;
    let t0 = performance.now();

    AstarModule().then(function(instance) {
        let moveCount = instance._runAstar(
            state[0],state[1],state[2],
            state[3],state[4],state[5],
            state[6],state[7],state[8],
            hType
        );

        let time = (performance.now() - t0).toFixed(2);

        if (moveCount === -1) {
            setStepsMsg("❌ This puzzle is unsolvable!");
            if (onDone) onDone();
            return;
        }
        if (moveCount === 0) {
            setStepsMsg("✅ Already solved!");
            if (onDone) onDone();
            return;
        }

        updateAstarStats(
            instance._getStatesExpanded(),
            moveCount,
            instance._getPathCost(),
            time
        );

        animateSolution(instance, moveCount, "A*", onDone);
    });
}

// ═══════════════════════════════════════════════════════════════
// RUN BRANCH & BOUND
// ═══════════════════════════════════════════════════════════════
function runBranchBound(state, onDone) {
    let t0 = performance.now();

    BranchBoundModule().then(function(instance) {
        let moveCount = instance._runBranchBound(
            state[0],state[1],state[2],
            state[3],state[4],state[5],
            state[6],state[7],state[8]
        );

        let time = (performance.now() - t0).toFixed(2);

        if (moveCount === -1) {
            setStepsMsg("❌ This puzzle is unsolvable!");
            if (onDone) onDone();
            return;
        }
        if (moveCount === 0) {
            setStepsMsg("✅ Already solved!");
            if (onDone) onDone();
            return;
        }

        updateBBStats(
            instance._getStatesExpanded(),
            moveCount,
            instance._getPathCost(),
            time
        );

        animateSolution(instance, moveCount, "Branch & Bound", onDone);
    });
}

// ═══════════════════════════════════════════════════════════════
// MAIN ENTRY — runSolver()
// ═══════════════════════════════════════════════════════════════
function runSolver() {
    if (isAnimating) { stopSolver(); return; }

    let state = getPuzzleState();
    if (!isValidState(state)) {
        alert("Invalid puzzle! Tiles 0–8 must each appear exactly once.");
        return;
    }

    let heuristic = document.querySelector('input[name="heuristic"]:checked').value;

    stopSolver();
    clearStats();
    setStepsMsg("⏳ Solving with " + selectedAlgo + "…");

    // Save initial board so we can restore it for compare mode
    let savedState = [...state];

    if (selectedAlgo === "A*") {
        runAstar(state, heuristic, null);

    } else if (selectedAlgo === "BranchBound") {
        runBranchBound(state, null);

    } else if (selectedAlgo === "compare") {
        // Run A* first, then B&B after A* animation finishes
        appendStep("=== A* ===");
        runAstar(state, heuristic, function() {
            // Restore board to original state for B&B
            setPuzzleState(savedState);
            setTimeout(function() {
                appendStep("\n=== Branch & Bound ===");
                runBranchBound(savedState, null);
            }, 600);
        });
    }
}

// ═══════════════════════════════════════════════════════════════
// INIT
// ═══════════════════════════════════════════════════════════════
document.addEventListener("DOMContentLoaded", function() {
    setPuzzleState([1,2,3,4,5,6,7,8,0]);
    setStepsMsg("Ready. Shuffle or edit tiles, then press ▶ Solve.");
});
