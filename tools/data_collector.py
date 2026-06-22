import serial
import time

SERIAL_PORT = 'COM3'
BAUD_RATE = 115200
RECORD_SECONDS = 30

gesture_name = input("Enter the gesture you are recording: ")
filename = f"{gesture_name}.csv"

try:
    arduino = serial.Serial(port = SERIAL_PORT, baudrate = BAUD_RATE, timeout = 1)

    time.sleep(2)
    print(f"Connected to ESP32 ON {SERIAL_PORT}")
    print(f"Recording data to {filename}....")

    end_time = time.time() + RECORD_SECONDS

    with open(filename, "a") as file:
        while time.time() < end_time:

            raw_data = arduino.readline()

            cleaned_data = raw_data.decode('utf-8').strip()

            if cleaned_data:
                print(f"Captured: {cleaned_data}")
                file.write(cleaned_data + "\n")
                        


except serial.SerialException as e:
    print(f"Error connecting to serial port: {e}")
except KeyboardInterrupt:
    print(f"\nProgram stopped by user...")
finally:

    if 'arduino' in locals() and arduino.is_open:
        arduino.close()
        print("Serial Port Closed...")