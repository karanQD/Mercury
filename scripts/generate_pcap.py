#!/usr/bin/env python3
import struct
from pathlib import Path

def build_add(symbol: bytes = b"AAPL    ") -> bytes:
    return (
        b"A"
        + struct.pack(">H", 0)
        + struct.pack(">H", 1)
        + b"\x00\x00\x00\x00\x00\x01"
        + struct.pack(">Q", 42)
        + b"B"
        + struct.pack(">I", 100)
        + symbol
        + struct.pack(">I", 100000)
    )

def main() -> None:
    out = Path("sample.pcap")
    global_header = struct.pack("<IHHIIII", 0xA1B2C3D4, 2, 4, 0, 0, 65535, 1)
    payload = build_add()
    ethernet = b"\x00" * 14
    ipv4 = b"\x45" + b"\x00" * 19
    udp = b"\x00" * 8
    packet = ethernet + ipv4 + udp + payload
    pkt_header = struct.pack("<IIII", 0, 0, len(packet), len(packet))
    out.write_bytes(global_header + pkt_header + packet)
    print(f"wrote {out}")

if __name__ == "__main__":
    main()
