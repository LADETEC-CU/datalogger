from websocket import create_connection
import time

ws = create_connection("ws://192.168.1.102/ws")
print("Sending 'Hello, World'...")
while True:
    ws.send("get_time")
    print ("Received '%s'" % ws.recv())
    print ("Received '%s'" % ws.recv())
    time.sleep(2)
