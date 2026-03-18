# How to Compile — Spy Mission

## Who writes what
- dijkstra.cpp    → Pushpesh
- backtracking.cpp → Pushpesh

## Rules
- Only write your logic inside the marked section
- Do NOT change any function names
- Do NOT change any function parameters
- Do NOT touch spyMission.html, spy.css or spy.js

---

## Step 1 — Open terminal in GameArena/ folder

Make sure you are in the GameArena/ folder, not inside Spy Mission/cpp/

---

## Step 2 — Compile each file one by one

### Dijkstra (Shortest Path):
```
.\emsdk\upstream\emscripten\emcc "Spy Mission/cpp/dijkstra.cpp" -o "Spy Mission/cpp/dijkstra.js" -s EXPORTED_FUNCTIONS="['_runDijkstra','_getVisitedRow','_getVisitedCol','_getPathRow','_getPathCol','_getVisitedCount']" -s MODULARIZE=1 -s EXPORT_NAME="DijkstraModule"
```

### Backtracking (Safest Path):
```
.\emsdk\upstream\emscripten\emcc "Spy Mission/cpp/backtracking.cpp" -o "Spy Mission/cpp/backtracking.js" -s EXPORTED_FUNCTIONS="['_runBacktracking','_getVisitedRow','_getVisitedCol','_getPathRow','_getPathCol','_getVisitedCount']" -s MODULARIZE=1 -s EXPORT_NAME="BacktrackingModule"
```

---

## Step 3 — Check output

After each compile you should see these files appear:
- dijkstra.js + dijkstra.wasm
- backtracking.js + backtracking.wasm

If you see an error, check your C++ code for syntax mistakes.

---

## Step 4 — Test your algorithm
```
.\emsdk\upstream\emscripten\emrun "Spy Mission/cpp/test.html"
```

Click Run Dijkstra and Run Backtracking buttons.
If you see Path Length > 0, your algorithm is working!

---

## Step 5 — Push to GitHub
```
git add .
git commit -m "implemented dijkstra algorithm"
git push origin main
```