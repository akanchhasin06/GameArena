# How to Compile — Puzzle Solver

## Who writes what
- astar.cpp       → Muhammad
- branchBound.cpp → Muhammad

## Rules
- Only write your logic inside the marked section
- Do NOT change any function names
- Do NOT change any function parameters
- Do NOT touch puzzleSol.html, puzzle.css or puzzle.js

---

## Step 1 — Open terminal in GameArena/ folder

Make sure you are in the GameArena/ folder, not inside Puzzle Solver/cpp/

---

## Step 2 — Compile each file one by one

### A*:
```
.\emsdk\upstream\emscripten\emcc "Puzzle Solver/cpp/astar.cpp" -o "Puzzle Solver/cpp/astar.js" -s EXPORTED_FUNCTIONS="['_runAstar','_getStatesExpanded','_getMoveRow','_getMoveCol','_getMoveCount']" -s MODULARIZE=1 -s EXPORT_NAME="AstarModule"
```

### Branch & Bound:
```
.\emsdk\upstream\emscripten\emcc "Puzzle Solver/cpp/branchBound.cpp" -o "Puzzle Solver/cpp/branchBound.js" -s EXPORTED_FUNCTIONS="['_runBranchBound','_getStatesExpanded','_getMoveRow','_getMoveCol','_getMoveCount']" -s MODULARIZE=1 -s EXPORT_NAME="BranchBoundModule"
```

---

## Step 3 — Check output

After each compile you should see these files appear:
- astar.js + astar.wasm
- branchBound.js + branchBound.wasm

If you see an error, check your C++ code for syntax mistakes.

---

## Step 4 — Test your algorithm
```
.\emsdk\upstream\emscripten\emrun "Puzzle Solver/cpp/test.html"
```

Click the Run button for each algorithm.
If you see Moves Count > 0, your algorithm is working!

---

## Step 5 — Push to GitHub
```
git add .
git commit -m "implemented astar algorithm"
git push origin main
```