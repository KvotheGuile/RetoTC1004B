const socket = new WebSocket('ws://localhost:1880/data');

socket.onopen = () => console.log("Conexión WebSocket abierta");
socket.onclose = () => console.log("Conexión WebSocket cerrada");
socket.onerror = (error) => console.log("Error en WebSocket: " + error);


socket.onmessage = function (event) {
    const data = JSON.parse(event.data);

        if (data.temperature !== undefined) document.getElementById("tempDisplay").innerText = data.temperature.toFixed(2) + ' °C';
        if (data.pressure !== undefined) document.getElementById("pressDisplay").innerText = data.pressure.toFixed(2) + ' Pa';
        if (data.height !== undefined) document.getElementById("heightDisplay").innerText = data.height.toFixed(2) + ' m';
        if (data.co2 !== undefined) document.getElementById("co2Display").innerText = data.co2.toFixed(2) + ' ppm';
        if (data.tvoc !== undefined) document.getElementById("tvocDisplay").innerText = data.tvoc.toFixed(2) + ' ppb';
        if (data.humidity !== undefined) {
            document.getElementById("humDisplay").innerText = data.humidity.toFixed(2) + ' %';
            document.getElementById("humidity").style.transform = `rotate(${180+(data.humidity.toFixed(2)/ 100) * 180}deg)`;
        }
        if (data.par03 !== undefined) document.getElementById("par03Display").innerText = data.par03 + " um";
        if (data.par05 !== undefined) document.getElementById("par05Display").innerText = data.par05 + " um";
        if (data.par10 !== undefined) document.getElementById("par10Display").innerText = data.par10 + " um";
        if (data.par25 !== undefined) document.getElementById("par25Display").innerText = data.par25 + " um";
        if (data.par50 !== undefined) document.getElementById("par50Display").innerText = data.par50 + " um";
        if (data.par100 !== undefined) document.getElementById("par100Display").innerText = data.par100 + " um";

        function logScale(value) {
            const maxWidth = document.getElementById("particleChart").clientWidth;
            if (value <= 0) return 0;
            return (Math.log10(value + 1) / Math.log10(100000)) * maxWidth;
        }

        // Grafica de partículas
        if (data.par03 !== undefined) document.getElementById("par03Bar").style.width = logScale(data.par03) + "px";
        if (data.par05 !== undefined) document.getElementById("par05Bar").style.width = logScale(data.par05) + "px";
        if (data.par10 !== undefined) document.getElementById("par10Bar").style.width = logScale(data.par10) + "px";
        if (data.par25 !== undefined) document.getElementById("par25Bar").style.width = logScale(data.par25) + "px";
        if (data.par50 !== undefined) document.getElementById("par50Bar").style.width = logScale(data.par50) + "px";
        if (data.par100 !== undefined) document.getElementById("par100Bar").style.width = logScale(data.par100) + "px";
    
    };

function sendSetpoint(key, value) {
    if (value === "" || isNaN(value)) {
        alert("Introduce un valor válido");
        return;
    }

    var msg = {};
    msg[key] = parseFloat(value);

    socket.send(JSON.stringify(msg));
    console.log("Enviado:", msg);
};