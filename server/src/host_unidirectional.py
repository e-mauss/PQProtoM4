#!/usr/bin/env python3
import serial
import sys

dev = serial.Serial("/dev/ttyACM0", 38400)

print("> Returned data:", file=sys.stderr)
x = list()

while len(x) < 3000:
    x.append(dev.read(4))
    # print(int.from_bytes(x, "little"))

for i in range(3000):
    if int.from_bytes(x[i], "little") != i:
        print(f"fail {i}")