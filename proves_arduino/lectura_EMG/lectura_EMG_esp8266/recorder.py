import serial
import csv
import time

# 1. CONFIGURATION
port_name = 'COM7' 
baud_rate = 115200
file_name = "emg_dataset.csv"

try:
    # Adding a timeout=1 ensures the script stays responsive
    ser = serial.Serial(port_name, baud_rate, timeout=1)
    # Give the ESP8266 a moment to reset after connection
    time.sleep(2) 
    ser.flushInput() # Clear any "garbage" data sitting in the buffer
    print(f"✅ Connected to {port_name}. Listening for data...")
except Exception as e:
    print(f"❌ Error: Could not connect to {port_name}. Details: {e}")
    exit()

# 2. RECORD TO FILE
with open(file_name, "w", newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["Timestamp", "EMG_Value", "Label", "Rep_Number"])
    
    print("Press Ctrl+C to stop recording.")
    
    try:
        print("--- DEBUG MODE: Printing everything from Serial ---")
        while True:
            if ser.in_waiting > 0:
                raw_line = ser.readline()
                # Print the raw bytes so we can see hidden characters
                print(f"RAW BYTES: {raw_line}")
                
                line = raw_line.decode('utf-8', errors='ignore').strip()
                print(f"DECODED TEXT: '{line}'")
            else:
                # No data in buffer
                time.sleep(5)
                print('else')
    except KeyboardInterrupt:
        print("Stopped.")