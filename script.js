// Animated particle grid background
const canvas = document.getElementById("bgCanvas");
const pen = canvas.getContext("2d");

canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

// Resize canvas when window resizes
window.addEventListener("resize", function() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
});

// Particle settings
let particles = [];
let particleCount = 80;

// Create particles
for(let i = 0; i < particleCount; i++) {
    particles.push({
        x: Math.random() * canvas.width,
        y: Math.random() * canvas.height,
        radius: Math.random() * 3 + 1,
        speedX: (Math.random() - 0.5) * 0.8,
        speedY: (Math.random() - 0.5) * 0.8,
    });
}

// Draw and animate
function animate() {
    pen.clearRect(0, 0, canvas.width, canvas.height);

    // Draw connections between nearby particles
    for(let i = 0; i < particles.length; i++) {
        for(let j = i + 1; j < particles.length; j++) {
            let dx = particles[i].x - particles[j].x;
            let dy = particles[i].y - particles[j].y;
            let dist = Math.sqrt(dx*dx + dy*dy);

            if(dist < 120) {
                pen.beginPath();
                pen.strokeStyle = "rgba(255,100,100," + (1 - dist/120) + ")";
                pen.lineWidth = 0.5;
                pen.moveTo(particles[i].x, particles[i].y);
                pen.lineTo(particles[j].x, particles[j].y);
                pen.stroke();
            }
        }
    }

    // Draw and move particles
    particles.forEach(function(p) {
        pen.beginPath();
        pen.arc(p.x, p.y, p.radius, 0, Math.PI * 2);
        pen.fillStyle = "rgba(255,100,100,0.8)";
        pen.fill();

        p.x += p.speedX;
        p.y += p.speedY;

        // bounce off edges
        if(p.x < 0 || p.x > canvas.width) p.speedX *= -1;
        if(p.y < 0 || p.y > canvas.height) p.speedY *= -1;
    });

    requestAnimationFrame(animate);
}

animate();