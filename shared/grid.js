const cell={
    empty:0,wall:1 , start:2 ,end: 3, visited:4, path:5
}

let row=15,col=15,cellsize=40;
let grid=[];
let startRow=-1, startCol=-1;
let endRow=-1, endCol=-1;

function makegrid(){
    grid=[];
    for(let i=0 ; i<row ;i++){
        let temp = [];
        for(let j=0 ; j<col ;j++){ temp.push(cell.empty); }
        grid.push(temp);
    }
}

function cellcolor(cellval){
    switch(cellval){
        case cell.empty:   return "#501bc4";
        case cell.wall:    return "#2f2714";
        case cell.start:   return "#00ff00";
        case cell.end:     return "#ff0000";
        case cell.visited: return "#b12675";
        case cell.path:    return "#ffff00";
        default:           return "white";
    }
}

function drawgrid(canvas,pen){
    pen.clearRect(0,0,canvas.width,canvas.height);

    for(let i=0;i<row;i++){
        for(let j=0;j<col;j++){
            let a=j*cellsize, b=i*cellsize;
            pen.fillStyle=cellcolor(grid[i][j]);
            pen.fillRect(a,b,cellsize-1,cellsize-1);
            pen.strokeStyle="black";
            pen.strokeRect(a,b,cellsize-1,cellsize-1);
        }
    }
}

let currState="wall";

function mouseclick(canvas,event,pen){
    let clickedCol=Math.floor(event.offsetX/cellsize);
    let clickedRow=Math.floor(event.offsetY/cellsize);

    if(clickedRow<0 || clickedRow>=row) return;
    if(clickedCol<0 || clickedCol>=col) return;

    if(currState=="wall"){
        if(grid[clickedRow][clickedCol]==cell.wall){
            grid[clickedRow][clickedCol]=cell.empty;
        } else {
            grid[clickedRow][clickedCol]=cell.wall;
        }
    } else if(currState=="start"){
        if(startRow!=-1) grid[startRow][startCol]=cell.empty;
        grid[clickedRow][clickedCol]=cell.start;
        startRow=clickedRow;
        startCol=clickedCol;
    } else if(currState=="end"){
        if(endRow!=-1) grid[endRow][endCol]=cell.empty;
        grid[clickedRow][clickedCol]=cell.end;
        endRow=clickedRow;
        endCol=clickedCol;
    }
    drawgrid(canvas,pen);
}