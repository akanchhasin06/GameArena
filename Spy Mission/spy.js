const GRID_SIZE = 10;
let gameState = {
    spyPos: { x: 0, y: 0 },
    exitPos: { x: 9, y: 9 },
    grid: [],
    optimalPath: [], 
    isGameOver: false,
    wasmModule: null,
    health: 100
};
async function bootstrapWasm() {
    console.log("System: Attempting to link C++ Engine...");
    try {
        if (typeof createSpyModule !== 'undefined') {
            gameState.wasmModule = await createSpyModule();
        } 
        else if (typeof Module !== 'undefined' && typeof Module === 'function') {
            gameState.wasmModule = await Module();
        }

        if (gameState.wasmModule) {
            console.log("SUCCESS: C++ Brain Online.");
            const btns = document.querySelectorAll('button');
            btns.forEach(b => b.style.boxShadow = "0 0 15px #00ff41");
        }
    } catch (e) {
        console.warn("WASM Engine loading... (Wait for initialization)", e);
    }
}

window.onload = () => {
    initGame();
    bootstrapWasm();
    setInterval(patrolEntities, 2500); 
};

document.getElementById('shortest-path-btn').onclick = function() {
    if (!gameState.wasmModule) {
        console.log("WASM Brain not synced yet.");
        return; 
    }
    
    try {
        const flatGrid = gameState.grid.flat().map(v => v === 'G' ? 71 : (v === 'C' ? 67 : 0));
        const gridPtr = gameState.wasmModule._malloc(flatGrid.length * 4);
        gameState.wasmModule.HEAP32.set(new Int32Array(flatGrid), gridPtr / 4);

        const pathPtr = gameState.wasmModule._calculateCleanShortestPath(
            gameState.spyPos.x, gameState.spyPos.y,
            gameState.exitPos.x, gameState.exitPos.y,
            gridPtr, GRID_SIZE
        );

        const pathData = new Int32Array(gameState.wasmModule.HEAP32.buffer, pathPtr, 50);
        gameState.optimalPath = Array.from(pathData).filter(idx => idx >= 0 && idx < 100);

        renderGrid(); 
        gameState.wasmModule._free(gridPtr);
    } catch (err) {
        console.error("Dijkstra Logic Error:", err);
    }
};

document.getElementById('hint-btn').onclick = function() {
    if (!gameState.wasmModule) return;
    try {
        const junctionIdx = gameState.wasmModule._findLastJunction();
        const jy = Math.floor(junctionIdx / 10);
        const jx = junctionIdx % 10;
        alert(`BACKTRACK HINT: Last safe junction was at (${jx}, ${jy})`);
    } catch (err) {
        console.error("Hint Logic Error:", err);
    }
};

function initGame() {
    gameState.grid = Array.from({ length: GRID_SIZE }, () => Array(GRID_SIZE).fill(0));
    gameState.grid[4][4] = 'G'; 
    gameState.grid[7][7] = 'C'; 
    gameState.grid[9][9] = 'E'; 
    renderGrid();
}

function renderGrid() {
    const container = document.getElementById('grid-container');
    if (!container) return;
    container.innerHTML = '';
    
    for (let y = 0; y < GRID_SIZE; y++) {
        for (let x = 0; x < GRID_SIZE; x++) {
            const cell = document.createElement('div');
            cell.className = 'cell';
            const val = gameState.grid[y][x];
            const currentIndex = y * GRID_SIZE + x;

            if (gameState.optimalPath.includes(currentIndex)) {
                cell.classList.add('path-highlight');
            }

            if (val === 'E') {
                cell.classList.add('exit');
                cell.textContent = "OBJ"; 
            }

            if (x === gameState.spyPos.x && y === gameState.spyPos.y) {
                cell.classList.add('spy');
                cell.textContent = '🕵️';
                gameState.optimalPath = gameState.optimalPath.filter(p => p !== currentIndex);
            } else if (val === 0) {
                cell.textContent = `${x.toString().padStart(2, '0')}.${y.toString().padStart(2, '0')}`;
            }
            container.appendChild(cell);
        }
    }
    updateUI();
}

function updateUI() {
    const {x, y} = gameState.spyPos;
    const msg = document.getElementById('game-message');
    const healthFill = document.getElementById('health-fill');
    
    if(healthFill) healthFill.style.width = gameState.health + "%";

    let nearThreat = false;
    for(let i = -1; i <= 1; i++) {
        for(let j = -1; j <= 1; j++) {
            let ny = y + i, nx = x + j;
            if(ny >= 0 && ny < 10 && nx >= 0 && nx < 10) {
                const cellVal = gameState.grid[ny][nx];
                if(cellVal === 'G' || cellVal === 'C') nearThreat = true;
            }
        }
    }

    if (msg) {
        msg.innerHTML = nearThreat ? 
            "<span style='color:#ff003c; font-weight:bold;'>⚠️ WARNING: PROXIMITY SENSOR TRIGGERED</span>" : 
            "Status: Stealth Active";
    }
    if (gameState.grid[y][x] === 'G') {
        endGame("TERMINATED: Caught by Guard.");
    } else if (gameState.grid[y][x] === 'C') {
        endGame("TERMINATED: Captured on Camera.");
    }
}

function moveSpy(dx, dy) {
    if (gameState.isGameOver) return;
    const nx = gameState.spyPos.x + dx, ny = gameState.spyPos.y + dy;
    
    if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10) {
        gameState.spyPos = {x: nx, y: ny};
        renderGrid();
        
        if (nx === 9 && ny === 9) { 
            alert("MISSION SUCCESS: Data Extracted!"); 
            location.reload(); 
        }
    }
}

window.onkeydown = (e) => {
    if (["ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight"].includes(e.key)) {
        e.preventDefault(); 
    }
    if (e.key === "ArrowUp") moveSpy(0, -1);
    if (e.key === "ArrowDown") moveSpy(0, 1);
    if (e.key === "ArrowLeft") moveSpy(-1, 0);
    if (e.key === "ArrowRight") moveSpy(1, 0);
};

function patrolEntities() {
    if (gameState.isGameOver) return;
    let newGrid = JSON.parse(JSON.stringify(gameState.grid));
    
    for (let y = 0; y < GRID_SIZE; y++) {
        for (let x = 0; x < GRID_SIZE; x++) {
            if (gameState.grid[y][x] === 'G' || gameState.grid[y][x] === 'C') {
                const dirs = [{dx:1,dy:0},{dx:-1,dy:0},{dx:0,dy:1},{dx:0,dy:-1}];
                const d = dirs[Math.floor(Math.random()*4)];
                const nx = x+d.dx, ny = y+d.dy;
                
                if(nx>=0 && nx<10 && ny>=0 && ny<10 && gameState.grid[ny][nx] === 0) {
                    newGrid[ny][nx] = gameState.grid[y][x]; 
                    newGrid[y][x] = 0;
                }
            }
        }
    }
    gameState.grid = newGrid;
    renderGrid();
}

function endGame(m) { 
    gameState.isGameOver = true; 
    alert(m); 
    location.reload(); 
}
