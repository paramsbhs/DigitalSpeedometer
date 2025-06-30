#!/usr/bin/env python3

import argparse
import re
import json

def parse_can_line(line):
    pattern = r'\[(\d{4}-\d{2}-\d{2}) (\d{2}:\d{2}:\d{2}\.\d{3})\] RX: ID=0x([0-9A-Fa-f]+) DATA=\[([0-9A-Fa-f\s]+)\]'
    match = re.match(pattern, line.strip())
    
    if not match:
        return None
    
    date_part = match.group(1)
    time_part = match.group(2)
    can_id = match.group(3)
    data_part = match.group(4)
    
    timestamp = f"{date_part} {time_part.split('.')[0]}"
    
    data_bytes = data_part.split()
    
    while len(data_bytes) < 8:
        data_bytes.append("00")
    
    can_id_decimal = int(can_id, 16)
    
    return {
        'timestamp': timestamp,
        'id': can_id_decimal,
        'extended': 'false',
        'dir': 'Rx',
        'bus': '0',
        'length': len([b for b in data_bytes if b != "00" or data_bytes.index(b) < len([b for b in data_bytes if b != "00"])]),
        'data': data_bytes[:8]
    }

def convert_to_gvret_format(parsed_data):
    if not parsed_data:
        return None
    
    actual_length = 8
    for i in range(7, -1, -1):
        if parsed_data['data'][i] != "00":
            break
        actual_length = i
    
    if actual_length == 0:
        actual_length = 1
    
    data_decimal = [str(int(byte, 16)) for byte in parsed_data['data']]
    
    return f"{parsed_data['timestamp']},{parsed_data['id']},{parsed_data['extended']},{parsed_data['dir']},{parsed_data['bus']},{actual_length},{','.join(data_decimal)}"

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert CAN log to GVRET format for SavvyCAN.')
    parser.add_argument('src_filename', type=str, help='Input filename')
    parser.add_argument('dest_filename', type=str, help='Output filename')
    parser.add_argument('-n', '--skip_initial_lines', type=int, help='Number of lines to skip at start of file', default=0)
    parser.add_argument('-t', '--skip_final_lines', type=int, help='Number of lines to skip at end of file', default=0)
    args = parser.parse_args()
    
    header = "Time Stamp,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8\n"
    
    with open(args.src_filename, 'r') as f:
        lines = f.readlines()
    
    numLines = len(lines)
    
    with open(args.dest_filename, 'w') as fOut:
        fOut.write(header)
        
        for i, line in enumerate(lines):
            if i < args.skip_initial_lines or i >= numLines - args.skip_final_lines:
                continue
            
            if line.strip().startswith('#') or not line.strip():
                continue
            
            parsed = parse_can_line(line)
            if parsed:
                gvret_line = convert_to_gvret_format(parsed)
                if gvret_line:
                    fOut.write(gvret_line + '\n')
                    print(f"Converted: {line.strip()}")
            else:
                print(f"Skipped invalid line: {line.strip()}")
    
    print(f"Conversion complete! Output saved to: {args.dest_filename}")