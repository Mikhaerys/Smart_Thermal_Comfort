import thermal_comfort as tc
import websockets
import asyncio
import json


async def handle_connection(websocket):
    async for message in websocket:
        print(f"Received message: {message}")

        try:
            # Parse the JSON message
            data = json.loads(message)
            message_type = data.get("type")

            # Handle the message based on its type
            if message_type == "New scan":
                uid = data.get("UID")
            elif message_type == "New environment data":
                temperature = data.get("temperature")
                humidity = data.get("humidity")
            else:
                print(f"Unknown message type: {message_type}")
        except json.JSONDecodeError:
            print("Failed to decode JSON message")


async def main():
    async with websockets.serve(handle_connection, "localhost", 8765):
        await asyncio.Future()  # Run forever

if __name__ == "__main__":
    asyncio.run(main())
