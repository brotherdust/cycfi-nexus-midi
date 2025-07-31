#!/usr/bin/env python3
"""
Nexus MIDI Controller Debug Message Decoder

This script decodes debug messages sent via MIDI SysEx from the Nexus controller.
It can be used standalone or integrated into other MIDI monitoring tools.

Message Format:
F0 00 7D 4E [MSG_TYPE] [TIMESTAMP_MSB] [TIMESTAMP_LSB] [DATA1] [DATA2] [DATA3] [DATA4] [DATA5] F7
"""

import sys
from datetime import datetime
from typing import List, Tuple, Optional

# Message type definitions
MESSAGE_TYPES = {
    0x01: "System Event",
    0x02: "Controller Change", 
    0x03: "Memory Usage",
    0x04: "Error",
    0x05: "Debug String",
    0x06: "Configuration",
    0x07: "Storage Operation",
    0x08: "Performance Metric"
}

# System events
SYSTEM_EVENTS = {
    0x01: "Startup",
    0x02: "Shutdown",
    0x03: "Mode Change",
    0x04: "Calibration Start",
    0x05: "Calibration End",
    0x10: "Test Mode Enter",
    0x11: "Test Mode Exit"
}

# Controller IDs
CONTROLLERS = {
    0x01: "Program Change",
    0x02: "Volume",
    0x03: "Pitch Bend",
    0x04: "Modulation",
    0x05: "FX1",
    0x06: "FX2",
    0x07: "Sustain",
    0x08: "Bank Select"
}

# Error codes
ERROR_CODES = {
    0x01: "Memory Allocation Failed",
    0x02: "Flash Write Failed",
    0x03: "Flash Read Failed",
    0x04: "Invalid Parameter",
    0x05: "Buffer Overflow",
    0x06: "Calibration Failed",
    0x10: "Debug Buffer Overflow"
}

# Configuration events
CONFIG_EVENTS = {
    0x01: "Bank Changed",
    0x02: "Channel Changed",
    0x03: "CC Mapping Changed",
    0x04: "Calibration Updated"
}

# Storage events
STORAGE_EVENTS = {
    0x01: "Flash Read",
    0x02: "Flash Write",
    0x03: "Flash Erase",
    0x04: "Load Settings",
    0x05: "Save Settings"
}

# Performance metrics
PERF_METRICS = {
    0x01: "Loop Time (ms)",
    0x02: "CPU Usage (%)",
    0x03: "Message Rate",
    0x04: "Buffer Usage (%)"
}


class NexusDebugDecoder:
    """Decoder for Nexus MIDI debug messages"""
    
    def __init__(self):
        self.start_time = datetime.now()
    
    def decode_sysex(self, data: List[int]) -> Optional[str]:
        """
        Decode a SysEx message
        
        Args:
            data: List of bytes including F0 and F7
            
        Returns:
            Decoded message string or None if invalid
        """
        # Validate message structure
        if len(data) < 13:  # Minimum size for our format
            return None
            
        if data[0] != 0xF0 or data[-1] != 0xF7:
            return None
            
        # Check manufacturer ID
        if data[1] != 0x00 or data[2] != 0x7D or data[3] != 0x4E:
            return None
            
        # Extract message components
        msg_type = data[4]
        timestamp = (data[5] << 7) | data[6]  # 14-bit timestamp
        payload = data[7:12]  # 5 data bytes
        
        # Decode based on message type
        if msg_type not in MESSAGE_TYPES:
            return f"[{timestamp}ms] Unknown message type: 0x{msg_type:02X}"
            
        type_name = MESSAGE_TYPES[msg_type]
        
        # Decode specific message types
        if msg_type == 0x01:  # System Event
            return self._decode_system_event(timestamp, payload)
        elif msg_type == 0x02:  # Controller Change
            return self._decode_controller_change(timestamp, payload)
        elif msg_type == 0x03:  # Memory Usage
            return self._decode_memory_usage(timestamp, payload)
        elif msg_type == 0x04:  # Error
            return self._decode_error(timestamp, payload)
        elif msg_type == 0x05:  # Debug String
            return self._decode_debug_string(timestamp, payload)
        elif msg_type == 0x06:  # Configuration
            return self._decode_configuration(timestamp, payload)
        elif msg_type == 0x07:  # Storage Operation
            return self._decode_storage_operation(timestamp, payload)
        elif msg_type == 0x08:  # Performance Metric
            return self._decode_performance_metric(timestamp, payload)
            
        return f"[{timestamp}ms] {type_name}: {' '.join(f'{b:02X}' for b in payload)}"
    
    def _decode_system_event(self, timestamp: int, payload: List[int]) -> str:
        event = SYSTEM_EVENTS.get(payload[0], f"Unknown(0x{payload[0]:02X})")
        param1 = payload[1]
        param2 = payload[2]
        
        if payload[0] == 0x01:  # Startup
            version = f"{param1}.{param2}"
            return f"[{timestamp}ms] System Event: {event} v{version}"
        else:
            return f"[{timestamp}ms] System Event: {event} (p1={param1}, p2={param2})"
    
    def _decode_controller_change(self, timestamp: int, payload: List[int]) -> str:
        event_type = "Change" if payload[0] == 0x01 else "Calibrated"
        controller = CONTROLLERS.get(payload[1], f"Unknown(0x{payload[1]:02X})")
        value = payload[2]
        return f"[{timestamp}ms] Controller {event_type}: {controller} = {value}"
    
    def _decode_memory_usage(self, timestamp: int, payload: List[int]) -> str:
        usage_percent = payload[0]
        free_bytes = (payload[1] << 7) | payload[2]
        return f"[{timestamp}ms] Memory: {usage_percent}% used, {free_bytes} bytes free"
    
    def _decode_error(self, timestamp: int, payload: List[int]) -> str:
        error = ERROR_CODES.get(payload[0], f"Unknown(0x{payload[0]:02X})")
        context = payload[1]
        return f"[{timestamp}ms] ERROR: {error} (context=0x{context:02X})"
    
    def _decode_debug_string(self, timestamp: int, payload: List[int]) -> str:
        # Convert 4 bytes to ASCII string
        chars = []
        for i in range(4):
            if payload[i] >= 0x20 and payload[i] <= 0x7E:
                chars.append(chr(payload[i]))
            else:
                chars.append('.')
        string = ''.join(chars).rstrip()
        value = payload[4]
        return f"[{timestamp}ms] Debug: '{string}' = {value}"
    
    def _decode_configuration(self, timestamp: int, payload: List[int]) -> str:
        event = CONFIG_EVENTS.get(payload[0], f"Unknown(0x{payload[0]:02X})")
        param1 = payload[1]
        param2 = payload[2]
        return f"[{timestamp}ms] Config: {event} (p1={param1}, p2={param2})"
    
    def _decode_storage_operation(self, timestamp: int, payload: List[int]) -> str:
        event = STORAGE_EVENTS.get(payload[0], f"Unknown(0x{payload[0]:02X})")
        address = (payload[1] << 7) | payload[2]
        value = payload[3]
        return f"[{timestamp}ms] Storage: {event} @ 0x{address:04X} = 0x{value:02X}"
    
    def _decode_performance_metric(self, timestamp: int, payload: List[int]) -> str:
        metric = PERF_METRICS.get(payload[0], f"Unknown(0x{payload[0]:02X})")
        value = payload[1]
        return f"[{timestamp}ms] Performance: {metric} = {value}"


def hex_string_to_bytes(hex_str: str) -> List[int]:
    """Convert hex string to list of bytes"""
    hex_str = hex_str.replace(' ', '').replace('\n', '')
    return [int(hex_str[i:i+2], 16) for i in range(0, len(hex_str), 2)]


def main():
    """Main entry point for command-line usage"""
    decoder = NexusDebugDecoder()
    
    print("Nexus MIDI Debug Message Decoder")
    print("Enter SysEx messages as hex (e.g., F0 00 7D 4E 01 00 0A 01 02 00 00 00 00 F7)")
    print("Type 'quit' to exit\n")
    
    while True:
        try:
            line = input("> ").strip()
            if line.lower() == 'quit':
                break
                
            if not line:
                continue
                
            # Parse hex input
            try:
                bytes_data = hex_string_to_bytes(line)
            except ValueError:
                print("Invalid hex format")
                continue
                
            # Decode message
            result = decoder.decode_sysex(bytes_data)
            if result:
                print(result)
            else:
                print("Invalid or unrecognized SysEx message")
                
        except KeyboardInterrupt:
            print("\nExiting...")
            break
        except Exception as e:
            print(f"Error: {e}")


if __name__ == "__main__":
    main()