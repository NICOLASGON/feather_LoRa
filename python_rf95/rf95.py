#!/bin/python3

import serial

with serial.Serial('/dev/ttyUSB0', 115200) as ser:
    line =  ser.readline()

class RF95():
    def __init__(port):
        self.port = port
        self.ser = serial.Serial(port, 115200)

    def send(message):
        self.ser.write(message)
