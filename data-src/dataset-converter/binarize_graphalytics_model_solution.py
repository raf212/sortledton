#!python3

import sys
import struct

seperator = " "

input_path = sys.argv[1]
output_path = sys.argv[2]

VALUE_TYPE_DOUBLE = 0
VALUE_TYPE_INT = 1

value_type = -1


def detect_value_type(value):
    try:
        int(value)
        return VALUE_TYPE_INT
    except ValueError:
        return VALUE_TYPE_DOUBLE


lines = 0
with open(input_path) as i:
    line = i.readline()
    while line:
        lines += 1
        line = i.readline()

with open(input_path) as i:
    with open(output_path, "bw") as o:
        o.write(struct.pack("q", lines))
        line = i.readline()
        while line:
            [vertex, value] = line.split(seperator)

            if value_type == -1:
                value_type = detect_value_type(value)

            bin = None
            if value_type == VALUE_TYPE_INT:
                # TODO need to support 64 bit and 32 bit output
                bin = struct.pack("qI", int(vertex), int(value))
            elif value_type == VALUE_TYPE_DOUBLE:
                bin = struct.pack("qd", int(vertex), float(value))
            o.write(bin)

            line = i.readline()
