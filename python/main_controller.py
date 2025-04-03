from thermal_comfort import ThermalComfort
import websockets
import asyncio
import sqlite3
import json


class SmartThermalComfort:
    def __init__(self, db_path):
        self.conn = sqlite3.connect(db_path)
        self.cursor = self.conn.cursor()
        self.thermal_comfort = ThermalComfort()
        self.student_pmvs = []
        print("Connected to the database.")

    async def handle_connection(self, websocket):
        async for message in websocket:
            try:
                print(f"Received message: {message}")
                # Parse the JSON message
                data = json.loads(message)
                message_type = data.get("type")

                # Handle the message based on its type
                if message_type == "New scan":
                    uid = data.get("UID")
                    thermal_feeling = data.get("Thermal Feeling")
                    in_class = data.get("In Class")

                    print(
                        f"New scan detected. UID: {uid}, Thermal Feeling: {thermal_feeling}"
                    )

                    # Insert a new log into scan_logs
                    self.cursor.execute(
                        "INSERT INTO scan_logs (uid) VALUES (?)", (uid,)
                    )

                    # Update the thermal_feeling and in_class status of the student
                    self.cursor.execute(
                        "UPDATE students SET thermal_feeling = ?, in_class = ? WHERE uid = ?", (
                            thermal_feeling, in_class, uid)
                    )

                    # Commit the changes
                    self.conn.commit()

                    # Query the database for thermal feelings of students in class
                    self.cursor.execute(
                        "SELECT thermal_feeling FROM students WHERE in_class = 1"
                    )
                    self.student_pmvs = [
                        row[0] for row in self.cursor.fetchall()
                    ]
                    print(f"Student PMVs: {self.student_pmvs}")

                elif message_type == "New environment data":
                    temperature = data.get("temperature")
                    humidity = data.get("humidity")

                    print(
                        f"New environment data. Temperature: {temperature}, Humidity: {humidity}"
                    )

                    # Insert a new log into environment_logs
                    self.cursor.execute(
                        "INSERT INTO environment_logs (temperature, humidity) VALUES (?, ?)", (
                            temperature, humidity)
                    )

                    # Commit the changes
                    self.conn.commit()

                    # Calculate the classroom PMV
                    classroom_pmv = self.thermal_comfort.classroom_pmv(
                        temperature, humidity, self.student_pmvs
                    )

                    if classroom_pmv > 0.5:
                        await websocket.send("turn_on_cooler")
                    elif classroom_pmv < -0.5:
                        await websocket.send("turn_off_cooler")

                else:
                    print(f"Unknown message type: {message_type}")

            except json.JSONDecodeError:
                print("Failed to decode JSON message")
            except sqlite3.Error as e:
                print(f"Database error: {e}")
            except websockets.exceptions.ConnectionClosedError as e:
                print(f"WebSocket connection closed: {e}")
            except Exception as e:
                print(f"Unexpected error: {e}")

    async def main(self):
        async with websockets.serve(self.handle_connection, "192.168.0.x", 8765):
            await asyncio.Future()  # Run forever

    def close(self):
        self.conn.close()


if __name__ == "__main__":
    smart_thermal_comfort = SmartThermalComfort('../db/database.db')
    try:
        asyncio.run(smart_thermal_comfort.main())
    finally:
        smart_thermal_comfort.close()
