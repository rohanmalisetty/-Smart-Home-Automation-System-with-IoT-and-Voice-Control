const backend_url = "http://YOUR_RASPBERRY_PI_IP:5000"; // Replace with your Flask server IP/port

async function fetchSensorData() {
    try {
        const response = await fetch(`${backend_url}/get_sensor_data`);
        const data = await response.json();
        document.getElementById('temperature').textContent = data.temperature.toFixed(2);
        document.getElementById('humidity').textContent = data.humidity.toFixed(2);
        document.getElementById('light').textContent = data.light;
        document.getElementById('motion').textContent = data.motion === 1 ? 'Detected' : 'No Motion';
    } catch (error) {
        console.error("Error fetching sensor data:", error);
        document.getElementById('temperature').textContent = 'Error';
        document.getElementById('humidity').textContent = 'Error';
        document.getElementById('light').textContent = 'Error';
        document.getElementById('motion').textContent = 'Error';
    }
}

async function controlAppliance(device, state) {
    try {
        const response = await fetch(`${backend_url}/control_appliance`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ device: device, state: state })
        });
        const data = await response.json();
        console.log(data.message);
        // Optionally update UI based on success/failure
    } catch (error) {
        console.error(`Error controlling ${device}:`, error);
    }
}

// Fetch data initially and then every 5 seconds
fetchSensorData();
setInterval(fetchSensorData, 5000);
