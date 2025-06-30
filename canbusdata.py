#!/usr/bin/env python3
"""
Serial Logger for CAN Bus Data
Logs serial data with timestamps to a file
"""
import serial
import datetime
import sys
import os

def find_serial_ports():
    import glob
    ports = glob.glob('/dev/tty.usb*') + glob.glob('/dev/tty.SLAB*')
    return ports

def log_serial_data(port, baudrate=115200, output_file=None):
    if output_file is None:
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        output_file = f"can_log_{timestamp}.txt"
    
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"Connected to {port} at {baudrate} baud")
        print(f"Logging to: {output_file}")
        print("Press Ctrl+C to stop logging\n")
        
        with open(output_file, 'w') as f:
            f.write(f"# CAN Bus Log Started: {datetime.datetime.now()}\n")
            f.write(f"# Port: {port}, Baud: {baudrate}\n")
            f.write("# Format: [TIMESTAMP] DATA\n\n")
            f.flush()
            
            while True:
                try:
                    if ser.in_waiting > 0:
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        if line:
                            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
                            log_entry = f"[{timestamp}] {line}"

                            print(log_entry)
   
                            f.write(log_entry + "\n")
                            f.flush() 
                            
                except UnicodeDecodeError:
                    continue
                    
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        return False
    except KeyboardInterrupt:
        print(f"\nLogging stopped. Data saved to: {output_file}")
        return True
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

def main():
    ports = find_serial_ports()
    
    if not ports:
        print("No ports found")
        print("Make sure your device is connected.")
        return
    
    print("Available serial ports:")
    for i, port in enumerate(ports):
        print(f"{i+1}. {port}")
    
    # Let user select port
    if len(ports) == 1:
        selected_port = ports[0]
        print(f"Using: {selected_port}")
    else:
        try:
            choice = int(input("Select port (number): ")) - 1
            selected_port = ports[choice]
        except (ValueError, IndexError):
            print("Invalid selection")
            return
    
    # Get baud rate
    try:
        baudrate = int(input("Enter baud rate (default 115200): ") or "115200")
    except ValueError:
        baudrate = 115200
    
    # Get output file
    output_file = input("Enter output filename (press Enter for auto): ").strip()
    if not output_file:
        output_file = None
    
    # Start logging
    log_serial_data(selected_port, baudrate, output_file)

if __name__ == "__main__":
    main()