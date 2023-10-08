import websockets
import asyncio
import time
import json

# =========  Get time =========
data = {
    "cmd": "get_time"
}

get_time_json = json.dumps(data)

# =========  Set time =========
data = {
    "cmd": "set_time",
    "date": "Apr 16 2020",
    "time": "18:34:56"
}
set_time_json = json.dumps(data)


async def send_message(websocket, msg):
    while True:
        await websocket.send(msg)
        await asyncio.sleep(2)

async def receive_message(websocket):
    while True:
        message = await websocket.recv()
        print("Received: ", message)

async def connect_websocket():
    uri = "ws://192.168.1.102/ws"
    async with websockets.connect(uri) as websocket:
        print("Connected to WebSocket")
        # Set time
        await websocket.send(set_time_json)
        
        # Get time loop
        send_task = asyncio.create_task(send_message(websocket, get_time_json))
        receive_task = asyncio.create_task(receive_message(websocket))

        await asyncio.gather(send_task, receive_task)

if __name__ == "__main__":
    asyncio.run(connect_websocket())