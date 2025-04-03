from thermal_comfort import ThermalComfort
import websockets
import asyncio
import sqlite3
import json
from Crypto.Cipher import AES


class SmartThermalComfort:
    def __init__(self, db_path):
        self.conn = sqlite3.connect(db_path)
        self.cursor = self.conn.cursor()
        self.thermal_comfort = ThermalComfort()
        self.student_pmvs = []

        # Clave e IV AES-128 CBC (16 bytes cada uno, igual que en los ESP32)
        self.key = bytes.fromhex("clave de 16 digitos")
        self.iv = bytes.fromhex("clave 2 de 16 digitos")

    def decrypt_message(self, encrypted_data: bytes) -> str:
        """Desencripta el mensaje AES-128 CBC y devuelve el JSON como string."""
        cipher = AES.new(self.key, AES.MODE_CBC, self.iv)
        decrypted = cipher.decrypt(encrypted_data)

        # Eliminar padding (PKCS7)
        padding_len = decrypted[-1]
        if padding_len > 16:
            raise ValueError("Invalid padding length")
        decrypted = decrypted[:-padding_len]

        return decrypted.decode("utf-8")

    async def handle_connection(self, websocket):
        async for message in websocket:
            try:
                # Verificar si es binario → desencriptar
                if isinstance(message, bytes):
                    message = self.decrypt_message(message)

                print(f"Mensaje recibido: {message}")
                data = json.loads(message)
                message_type = data.get("type")

                if message_type == "New scan":
                    uid = data.get("UID")
                    thermal_feeling = data.get("Thermal Feeling")
                    in_class = data.get("In Class")

                    print(f"UID: {uid}, Sensación térmica: {thermal_feeling}, En clase: {in_class}")

                    self.cursor.execute("INSERT INTO scan_logs (uid) VALUES (?)", (uid,))
                    self.cursor.execute(
                        "UPDATE students SET thermal_feeling = ?, in_class = ? WHERE uid = ?",
                        (thermal_feeling, in_class, uid)
                    )
                    self.conn.commit()

                    self.cursor.execute("SELECT thermal_feeling FROM students WHERE in_class = 1")
                    self.student_pmvs = [row[0] for row in self.cursor.fetchall()]

                elif message_type == "New environment data":
                    temperature = data.get("temperature")
                    humidity = data.get("humidity")

                    print(f"Ambiente: Temperatura = {temperature}, Humedad = {humidity}")

                    self.cursor.execute(
                        "INSERT INTO environment_logs (temperature, humidity) VALUES (?, ?)",
                        (temperature, humidity)
                    )
                    self.conn.commit()

                    classroom_pmv = self.thermal_comfort.classroom_pmv(
                        temperature, humidity, self.student_pmvs
                    )

                    if classroom_pmv > 0.5:
                        await websocket.send("turn_on_cooler")
                    elif classroom_pmv < -0.5:
                        await websocket.send("turn_off_cooler")

                else:
                    print(f"Tipo de mensaje desconocido: {message_type}")

            except json.JSONDecodeError:
                print(" Error: No se pudo decodificar el JSON")
            except Exception as e:
                print(f" Error procesando mensaje: {e}")

    async def main(self):
        async with websockets.serve(self.handle_connection, "localhost", 8765):
            print("Servidor WebSocket iniciado en ws://localhost:8765")
            await asyncio.Future()  # Ejecutar indefinidamente

    def close(self):
        self.conn.close()


if __name__ == "__main__":
    smart_thermal_comfort = SmartThermalComfort('..db/database.db')
    try:
        asyncio.run(smart_thermal_comfort.main())
    finally:
        smart_thermal_comfort.close()
