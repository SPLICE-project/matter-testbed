#!/usr/bin/env python3

import requests
from websocket import create_connection, WebSocketApp
import json

# --- Initiate a session to obtain cookies ---
session = requests.Session()
response = session.get('http://localhost:8123')  # Use http

# Check for successful request
if response.status_code != 200:
    print("Failed to establish session.")
    exit(1)

# Extract cookies from session
cookie_header = '; '.join([f"{name}={value}" for name, value in session.cookies.items()])

headers = {
    "Cookie": cookie_header
}

# --- WebSocket functions ---
def on_message(ws, message):
    print("Message from server:", message)

def on_open(ws):
    print("WebSocket is open")
    message = {
        "message_id": "3",
        "command": "start_listening"
    }
    ws.send(json.dumps(message))


def on_close(ws, close_status_code, close_msg):
    print("WebSocket closed. Code:", close_status_code, "Message:", close_msg)# --- Establish WebSocket connection ---

ws = WebSocketApp(
    "ws://localhost:5580/ws",
    on_message=on_message,
    on_open=on_open,
    on_close=on_close,
    header=headers
)


# Start WebSocket loop
ws.run_forever()

