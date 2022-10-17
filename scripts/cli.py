#!/usr/bin/env python
import argparse
import select
import serial
import sys
import time

BAUDRATE = 115200


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', default='/dev/ttyACM0')
    return parser.parse_args()


def print_from_serial(com):
    rx_data = com.read_all()
    if len(rx_data) == 0:
        return

    try:
        print(rx_data.decode(), end='')
    except UnicodeDecodeError:
        print(repr(rx_data))


def has_user_input():
    return sys.stdin in select.select([sys.stdin], [], [], 0)[0]


def main():
    args = parse_args()
    com = serial.Serial(args.port, baudrate=BAUDRATE)

    while True:
        if has_user_input():
            try:
                command = input().encode() + b'\n'
            except EOFError:
                break

            com.write(command)

        print_from_serial(com)


if __name__ == "__main__":
    main()
