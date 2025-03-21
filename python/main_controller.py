import thermal_comfort as tc
import websockets
import asyncio
import sqlite3
import json

conn = sqlite3.connect('../db/database.db')
cursor = conn.cursor()


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
                thermal_feeling = data.get("Thermal Feeling")
                in_class = data.get("In Class")
                print(
                    f"New scan detected. UID: {uid}, Thermal Feeling: {thermal_feeling}")

                # Insert a new log into scan_logs
                cursor.execute(
                    "INSERT INTO scan_logs (uid) VALUES (?)", (uid,)
                )

                # Update the thermal_feeling and in_class status of the student
                cursor.execute(
                    "UPDATE students SET thermal_feeling = ?, in_class = ? WHERE uid = ?", (
                        thermal_feeling, in_class, uid)
                )

                # Commit the changes
                conn.commit()

            elif message_type == "New environment data":
                temperature = data.get("temperature")
                humidity = data.get("humidity")
                print(
                    f"New environment data. Temperature: {temperature}, Humidity: {humidity}")

                # Insert a new log into environment_logs
                cursor.execute(
                    "INSERT INTO environment_logs (temperature, humidity) VALUES (?, ?)", (
                        temperature, humidity)
                )

                # Commit the changes
                conn.commit()

            else:
                print(f"Unknown message type: {message_type}")
        except json.JSONDecodeError:
            print("Failed to decode JSON message")


async def main():
    async with websockets.serve(handle_connection, "localhost", 8765):
        await asyncio.Future()  # Run forever

if __name__ == "__main__":
    asyncio.run(main())

    # Close the database connection when the program ends
    conn.close()
