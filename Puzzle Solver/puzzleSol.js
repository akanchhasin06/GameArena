// ═══════════════════════════════════════
// PUZZLE SOLVER — puzzle.js
// ═══════════════════════════════════════

let selectedAlgo = "A*";
let animInterval = null;

// ═══════════════════════════════════════
// GET PUZZLE STATE FROM INPUTS
// ═══════════════════════════════════════
function getPuzzleState() {
    let tiles = document.querySelectorAll(".tile");
    let state = [];
    tiles.forEach(function(tile) {
        let val = parseInt(tile.value);
        state.push(isNaN(val) ? 0 : val);
    });
    return state;
}

// ═══════════════════════════════════════
// SET PUZZLE STATE TO INPUTS
// ═══════════════════════════════════════
function setPuzzleState(state) {
    let tiles = document.querySelectorAll(".tile");
    tiles.forEach(function(tile, index) {
        if(state[index] === 0) {
            tile.value = "";
            tile.style.backgroundColor = "#0f0f1a";
            tile.style.color = "transparent";
        } else {
            tile.value = state[index];
            tile.style.backgroundColor = "#5c6bc0";
            tile.style.color = "white";
        }
    });
}

// ═══════════════════════════════════════
// SHUFFLE
// ═══════════════════════════════════════
function shuffle() {
    let nums = [0,1,2,3,4,5,6,7,8];
    for(let i = nums.length - 1; i > 0; i--) {
        let j = Math.floor(Math.random() * (i + 1));
        [nums[i], nums[j]] = [nums[j], nums[i]];
    }
    setPuzzleState(nums);
    clearStats();
    document.getElementById("solutionSteps").textContent = "Waiting to solve...";
}

// ═══════════════════════════════════════
// RESET
// ═══════════════════════════════════════
function reset() {
    setPuzzleState([1,2,3,4,5,6,7,8,0]);
    clearStats();
    document.getElementById("solutionSteps").textContent = "Waiting to solve...";
}

// ═══════════════════════════════════════
// ALGORITHM SELECTOR
// ═══════════════════════════════════════
function selectALGO(algo) {
    selectedAlgo = algo;

    document.getElementById("Astarbtn").classList.remove("active");
    document.getElementById("BBbtn").classList.remove("active");
    document.getElementById("cmpbtn").classList.remove("active");

    if(algo === "A*") {
        document.getElementById("Astarbtn").classList.add("active");
        document.getElementById("heuristicPanel").style.display = "block";
    } else if(algo === "BranchBound") {
        document.getElementById("BBbtn").classList.add("active");
        document.getElementById("heuristicPanel").style.display = "none";
    } else if(algo === "compare") {
        document.getElementById("cmpbtn").classList.add("active");
        document.getElementById("heuristicPanel").style.display = "block";
    }
}

// ═══════════════════════════════════════
// SPEED SLIDER
// ═══════════════════════════════════════
document.getElementById("speedSlider").addEventListener("input", function() {
    document.getElementById("speedLabel").textContent = this.value + "ms";
});

// ═══════════════════════════════════════
// CLEAR STATS
// ═══════════════════════════════════════
function clearStats() {
    document.getElementById("astarStates").textContent = "0";
    document.getElementById("astarMoves").textContent = "0";
    document.getElementById("astarCost").textContent = "0";
    document.getElementById("astarTime").textContent = "0ms";
    document.getElementById("bbStates").textContent = "0";
    document.getElementById("bbMoves").textContent = "0";
    document.getElementById("bbCost").textContent = "0";
    document.getElementById("bbTime").textContent = "0ms";
    document.getElementById("solutionSteps").textContent = "";
}

// UPDATE STATS

function updateAstarStats(states, moves, cost, time) {
    document.getElementById("astarStates").textContent = states;
    document.getElementById("astarMoves").textContent = moves;
    document.getElementById("astarCost").textContent = cost;
    document.getElementById("astarTime").textContent = time + "ms";
}

function updateBBStats(states, moves, cost, time) {
    document.getElementById("bbStates").textContent = states;
    document.getElementById("bbMoves").textContent = moves;
    document.getElementById("bbCost").textContent = cost;
    document.getElementById("bbTime").textContent = time + "ms";
}

// ═══════════════════════════════════════
// ANIMATE SOLUTION STEPS

function animateSolution(moves, algoName) {
    let speed = parseInt(document.getElementById("speedSlider").value);
    let step = 0;
    let stepsDiv = document.getElementById("solutionSteps");
    
    // append instead of overwrite
    stepsDiv.textContent += "\n--- " + algoName + " ---\n";

    animInterval = setInterval(function() {
        if(step >= moves.length) {
            clearInterval(animInterval);
            stepsDiv.textContent += "✅ Solved in " + moves.length + " moves!\n";
            stepsDiv.scrollTop = stepsDiv.scrollHeight;
            return;
        }
        stepsDiv.textContent += "Step " + (step+1) + ": Move tile " + moves[step] + "\n";
        stepsDiv.scrollTop = stepsDiv.scrollHeight;
        step++;
    }, speed);
}


// STOP ANIMATION

function stopSolver() {
    if(animInterval) {
        clearInterval(animInterval);
        animInterval = null;
    }
}


// VALIDATE PUZZLE STATE

function isValidState(state) {
    let sorted = [...state].sort((a,b) => a-b);
    for(let i = 0; i < 9; i++) {
        if(sorted[i] !== i) return false;
    }
    return true;
}

// PASS STATE TO C++ VIA WASM
/*
function passStateToWasm(instance, state) {
    console.log("instance keys:", Object.keys(instance));
    let ptr = instance._malloc(9 * 4);
    let heap = new Int32Array(instance.HEAP32.buffer, ptr, 9);
    for(let i = 0; i < 9; i++) {
        heap[i] = state[i];
    }
    return ptr;
}

*/


// RUN A* 

function runAstar(state, heuristic) {
    console.log("exact state being passed:", state);
    console.log("state as array:", JSON.stringify(state));
    console.log("runAstar called with:", state, heuristic);
    let startTime = performance.now();

    AstarModule().then(function(instance) {
        console.log("inside AstarModule");

        let hType = heuristic === "manhattan" ? 0 : 1;
        
        // pass all 9 values individually
        let moveCount = instance._runAstar(
            state[0], state[1], state[2],
            state[3], state[4], state[5],
            state[6], state[7], state[8],
            hType
        );
        console.log("moveCount:", moveCount);

        let endTime = performance.now();
        let timeTaken = (endTime - startTime).toFixed(2);

        let states = instance._getStatesExpanded();
        let cost = instance._getPathCost();
        console.log("states:", states, "cost:", cost);

        let moves = [];
        for(let i = 0; i < moveCount; i++) {
            moves.push(instance._getMoveAt(i));
        }
        console.log("moves:", moves);

        updateAstarStats(states, moveCount, cost, timeTaken);

        if(moveCount > 0) {
            animateSolution(moves, "A*");
        } else {
            document.getElementById("solutionSteps").textContent = "❌ No solution found!";
        }
    });
}

// RUN BRANCH & BOUND
function runBranchBound(state) {
    let startTime = performance.now();

    BranchBoundModule().then(function(instance) {
        let moveCount = instance._runBranchBound(
            state[0], state[1], state[2],
            state[3], state[4], state[5],
            state[6], state[7], state[8]
        );

        if(moveCount === -1) {
            document.getElementById("solutionSteps").textContent = "❌ This puzzle is unsolvable!";
            return;
        }

        let endTime = performance.now();
        let timeTaken = (endTime - startTime).toFixed(2);

        let states = instance._getStatesExpanded();
        let cost = instance._getPathCost();

        let moves = [];
        for(let i = 0; i < moveCount; i++) {
            moves.push(instance._getMoveAt(i));
        }

        updateBBStats(states, moveCount, cost, timeTaken);

        if(moveCount > 0) {
            animateSolution(moves, "Branch & Bound");
        } else {
            document.getElementById("solutionSteps").textContent = "❌ No solution found!";
        }
    });
}

// MAIN FUNCTION

function runSolver() {

     let state = getPuzzleState();
    console.log("State:", state);  
    
    let heuristic = document.querySelector('input[name="heuristic"]:checked').value;
    console.log("Heuristic:", heuristic);
    console.log("Algorithm:", selectedAlgo); 

    if(!isValidState(state)) {
        alert("Invalid puzzle state! Make sure tiles 0-8 are all present.");
        return;
    }
    /*
    let state = getPuzzleState();
    let heuristic = document.querySelector('input[name="heuristic"]:checked').value;
    */
    // validate
    if(!isValidState(state)) {
        alert("Invalid puzzle state! Make sure tiles 0-8 are all present.");
        return;
    }

    // stop any existing animation
    stopSolver();
    clearStats();
    document.getElementById("solutionSteps").textContent = "⏳ Solving...";

    if(selectedAlgo === "A*") {
        runAstar(state, heuristic);
    } else if(selectedAlgo === "BranchBound") {
        runBranchBound(state);
    } else if(selectedAlgo === "compare") {
        runAstar(state, heuristic);
        setTimeout(function() {
            runBranchBound(state);
        }, 500);
    }
}

// ═══════════════════════════════════════
// LOAD WASM MODULES INTO HTML
// ═══════════════════════════════════════
// Add these script tags to puzzleSol.html:
// <script src="cpp/astar.js"></script>
// <script src="cpp/branchBound.js"></script>

// ═══════════════════════════════════════
// INIT ON PAGE LOAD
// ═══════════════════════════════════════
document.addEventListener("DOMContentLoaded", function() {
    setPuzzleState([1,2,3,4,5,6,7,8,0]);
});

// TEST — add this at very bottom of puzzle.js
window.addEventListener("load", function() {
    console.log("Testing AstarModule...");
    AstarModule().then(function(instance) {
        console.log("✅ AstarModule loaded successfully");
    }).catch(function(err) {
        console.log("❌ AstarModule failed:", err);
    });

    BranchBoundModule().then(function(instance) {
        console.log("✅ BranchBoundModule loaded successfully");
    }).catch(function(err) {
        console.log("❌ BranchBoundModule failed:", err);
    });
});