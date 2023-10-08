import websockets
import asyncio
import time
import json

# =========  Get status =========
data = {
    "cmd": "get_status"
}

get_time_json = json.dumps(data)

# =========  Set time =========
data = {
    "cmd": "set_time",
    "date": "Apr 16 2020",
    "time": "18:34:56"
}
set_time_json = json.dumps(data)

# =========  Get files =========
data = {
    "cmd": "get_files",
}
get_files_json = json.dumps(data)

# =========  Delete files =========
data = {
    "cmd": "rm_file",
    'filename': '/logs/07102023.csv'
}
rm_file_json = json.dumps(data)

# =========  Set interval =========
data = {
    "cmd": "set_interval",
    'interval': 30
}
set_interval_json = json.dumps(data)

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
        # Receive task
        receive_task = asyncio.create_task(receive_message(websocket))
        # Set time
        await websocket.send(set_time_json)
        await asyncio.sleep(1)
        # Get files
        await websocket.send(get_files_json)
        await asyncio.sleep(1)
        # Delete file
        await websocket.send(rm_file_json)
        await asyncio.sleep(1)
        # Delete file
        await websocket.send(set_interval_json)
        await asyncio.sleep(1)
        # Get time
        await websocket.send(get_time_json)
        await asyncio.sleep(1)
        
        # # Get time loop
        # send_task = asyncio.create_task(send_message(websocket, get_time_json))

        # await asyncio.gather(send_task, receive_task)
        await asyncio.gather(receive_task)

if __name__ == "__main__":
    asyncio.run(connect_websocket())