from flask import Flask, render_template, request, jsonify
from flask_mqtt import Mqtt
import json
import time

app = Flask(name)

# MQTT Configuration (adjust for your broker)
app.config['MQTT_BROKER_URL'] = 'broker.hivemq.com'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = ''
app.config['MQTT_PASSWORD'] = ''
app.config['MQTT_KEEPALIVE'] = 5  # Seconds
app.config['MQTT_TLS_ENABLED'] = False # Set to True if using TLS

mqtt = Mqtt(app)

# Global variables to store latest sensor data
current_sensor_data = {
    "temperature": "N/A",
    "humidity": "N/A",
    "light": "N/A",
    "motion": "N/A"
}

# --- MQTT Callbacks ---
@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker!")
        mqtt.subscribe("home/sensor_data")
    else:
        print(f"Failed to connect, return code {rc}\n")

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    global current_sensor_data
    topic = message.topic
    payload = message.payload.decode()
    print(f"Received message: Topic: {topic}, Payload: {payload}")

    if topic == "home/sensor_data":
        try:
            data = json.loads(payload)
            current_sensor_data.update(data)
            print("Updated sensor data:", current_sensor_data)
        except json.JSONDecodeError:
            print("Error decoding JSON payload.")

# --- Flask Routes ---
@app.route('/')
def index():
    return render_template('index.html')

@app.route('/get_sensor_data', methods=['GET'])
def get_sensor_data():
    return jsonify(current_sensor_data)

@app.route('/control_appliance', methods=['POST'])
def control_appliance():
    data = request.json
    device = data.get('device')
    state = data.get('state') # 0 for off, 1 for on

    if device and state is not None:
        control_payload = {device: state}
        mqtt.publish("home/control", json.dumps(control_payload))
        print(f"Published control command for {device} to {state}")
        return jsonify({"status": "success", "message": f"Command sent for {device}"})
    return jsonify({"status": "error", "message": "Invalid request"}), 400

# --- Voice Assistant Webhook (conceptual) ---
@app.route('/webhook/google_assistant', methods=['POST'])
def google_assistant_webhook():
    req = request.get_json(silent=True, force=True)
    intent = req.get('queryResult').get('intent').get('displayName')

    if intent == "TurnOnLight":
        mqtt.publish("home/control", json.dumps({"light1": 1}))
        response_text = "Turning on the light."
    elif intent == "TurnOffLight":
        mqtt.publish("home/control", json.dumps({"light1": 0}))
        response_text = "Turning off the light."
    elif intent == "GetTemperature":
        temp = current_sensor_data.get("temperature", "N/A")
        response_text = f"The current temperature is {temp} degrees Celsius."
    else:
        response_text = "I'm not sure how to handle that command yet."

    return jsonify({
        "fulfillmentText": response_text,
        "source": "smart_home_webhook"
    })


if name == 'main':
    mqtt.init_app(app) # Initialize MQTT after app config
    app.run(debug=True, host='0.0.0.0') # Set host to 0.0.0.0 to be accessible from other devices
