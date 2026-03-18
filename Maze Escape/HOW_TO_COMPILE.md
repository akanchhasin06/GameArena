# How to Compile — Maze Escape

## Who writes what
- bfs.cpp       
- dfs.cpp      
- dijkstra.cpp 
- astar.cpp    

## Rules
- Only write your logic inside the marked section
- Do NOT change any function names
- Do NOT change any function parameters
- Do NOT touch maze.html, maze.css or maze.js

---

## Step 1 — Open terminal in GameArena/ folder

Make sure you are in the GameArena/ folder, not inside Maze Escape/cpp/

---

## Step 2 — Compile each file one by one

### BFS:
```
.\emsdk\upstream\emscripten\emcc "Maze Escape/cpp/bfs.cpp" -o "Maze Escape/cpp/bfs.js" -s EXPORTED_FUNCTIONS="['_runBFS','_getVisitedRow','_getVisitedCol','_getPathRow','_getPathCol','_getVisitedCount']" -s MODULARIZE=1 -s EXPORT_NAME="BFSModule"
```

### DFS:
```
.\emsdk\upstream\emscripten\emcc "Maze Escape/cpp/dfs.cpp" -o "Maze Escape/cpp/dfs.js" -s EXPORTED_FUNCTIONS="['_runDFS','_getVisitedRow','_getVisitedCol','_getPathRow','_getPathCol','_getVisitedCount']" -s MODULARIZE=1 -s EXPORT_NAME="DFSModule"
```

### Dijkstra:
```
.\emsdk\upstream\emscripten\emcc "Maze Escape/cpp/dijkstra.cpp" -o "Maze Escape/cpp/dijkstra.js" -s EXPORTED_FUNCTIONS="['_runDijkstra','_getVisitedRow','_getVisitedCol','_getPathRow','_getPathCol','_getVisitedCount']" -s MODULARIZE=1 -s EXPORT_NAME="DijkstraModule"
```

### A*:
```
.\emsdk\upstream\emscripten\emcc "Maze Escape/cpp/astar.cpp" -o "Maze Escape/cpp/astar.js" -s EXPORTED_FUNCTIONS="['_runAstar','_getVisitedRow','_getVisitedCol','_getPathRow','_getPathCol','_getVisitedCount']" -s MODULARIZE=1 -s EXPORT_NAME="AstarModule"
```

---

## Step 3 — Check output

After each compile you should see these files appear:
- bfs.js + bfs.wasm
- dfs.js + dfs.wasm
- dijkstra.js + dijkstra.wasm
- astar.js + astar.wasm

If you see an error, check your C++ code for syntax mistakes.

---

## Step 4 — Test your algorithm
```
.\emsdk\upstream\emscripten\emrun "Maze Escape/cpp/test.html"
```

CREATE A test.html file to test your code 

Click the Run button for each algorithm.
If you see Path Length > 0, your algorithm is working!

---

## Step 5 — Push to GitHub
```
git add .
git commit -m "implemented bfs algorithm"
git push origin main
```