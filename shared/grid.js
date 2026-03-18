const cell={
    empty:0,wall:1 , start:2 ,end: 3, visited:4, path:5
}

let row=15,col=15,cellsize=50;
let grid=[];

function makegrid(){
     grid=[];
    for(let i=0 ; i<row ;i++){
        let temp = [];
        for(let j=0 ; j<col ;j++){  temp.push(cell.empty); }
        grid.push(temp);
    }

}

function cellcolor(cellval){
    switch(cellval){
        case cell.empty: return "#501bc4";
        case cell.wall: return "#2f2714";
        case cell.start: return "#00ff00";
        case cell.end: return "#ff0000";
        case cell.visited: return "#b12675";
        case cell.path: return "#ffff00";
        default: return "white";
    }
}

function drawgrid(canvas,pen){
    pen.clearRec(0,0,canvas.height,canvas.width);

    for(let i=0;i<row;i++){
        for(let j=0;j< col ; j++){
            let a=c*cellsize,b=r*cellsize;
            pen.fillStyle=cellcolor(grid[i][j]);
            pen.fillRect(a,b,cellsize-1,cellsize-1);
            pen.stokeStyle="black";
            pen.strokeRect=(a,b,cellsize-1,cellsize-1);
        }
    }
}

let currState="wall";
function mouseclick(canvas,event,pen){
    let col=Math.floor(event.offsetX/cellsize);
    let row=Math.floor(event.offsetY/cellsize);

    if(currState=="wall"){
        grid[row][col]=cell.wall;
    }   else if(currState=="start"){
        grid[row][col]=cell.start;
    }   
    else if(currState=="end"){
        grid[row][col]=cell.end;
    }
    drawgrid(canvas,pen);
}


