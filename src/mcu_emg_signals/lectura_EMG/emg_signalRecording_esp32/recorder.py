import serial
import csv
import time

# 1. CONFIGURATION
port_name = 'COM7' 
baud_rate = 115200
file_name = "emg_dataset.csv"

try:
    ser = serial.Serial(port_name, baud_rate, timeout=1)
    # The ESP32 reboots when the serial port opens; 2 seconds is perfect.
    time.sleep(2) 
    ser.flushInput() 
    print(f"✅ Connected to {port_name}. Listening for data...")
except Exception as e:
    print(f"❌ Error: Could not connect to {port_name}. Details: {e}")
    exit()

# 2. RECORD TO FILE
with open(file_name, "w", newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["Timestamp", "EMG_Value", "Label", "Rep_Number"])
    
    print("Reading data... Press Ctrl+C to stop.")
    
    try:
        while True:
            if ser.in_waiting > 0:
                # Read line and decode
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                # Check if the line is data (starts with a digit) and not a message like "# STARTING"
                if line and not line.startswith('#'):
                    parts = line.split(',')
                    
                    if len(parts) == 4:
                        # Write to CSV: [Timestamp, Value, Label, Rep]
                        writer.writerow(parts)
                        # Optional: Print to console so you know it's working
                        print(f"Recorded: {parts}")
                
                elif line.startswith('#'):
                    # Just print status messages to the console
                    print(f"STATUS: {line}")
            
            # Tiny sleep to prevent 100% CPU usage
            time.sleep(0.001)

    except KeyboardInterrupt:
        print("\n✅ Recording stopped. Data saved to", file_name)
    finally:
        ser.close()