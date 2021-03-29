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

with open(input_path) as i:
    with open(output_path, "bw") as o:
        line = i.readline()
        while line:
            [vertex, value] = line.split(seperator)

            if value_type == -1:
                value_type = detect_value_type(value)

            bin = None
            if value_type == VALUE_TYPE_INT:
                bin = struct.pack("qq", int(vertex), int(value))
            elif value_type == VALUE_TYPE_DOUBLE:
                bin = struct.pack("qd", int(vertex), float(value))
            o.write(bin)


