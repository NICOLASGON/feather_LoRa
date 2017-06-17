#!/bin/python3

from serial import Serial
from serial.threaded import LineReader, ReaderThread
from binascii import hexlify, unhexlify

class RF95_protocol(LineReader):
    def __init__(self, callback=None):
        super().__init__()
        self.lines = []
        self.callback = callback

    def __call__(self):
        return self

    def handle_line(self, data):
        if self.lines is not None:
            if self.callback is None:
                self.lines.append(unhexlify(data[2:]))
            else:
                self.callback(unhexlify(data[2:]))

class RF95():
    def __init__(self, port, baudrate=115200, recv_callback=None):
        self.port = port
        self.ser = Serial(port, baudrate)
        self.recv_callback = recv_callback
        self.protocol = RF95_protocol(recv_callback)
        self.reader_thread = ReaderThread(self.ser, self.protocol)
        self.reader_thread.start()

    def send(self, message):
        message = b'S ' + hexlify(message)
        self.protocol.write_line(message.decode('utf-8'))

    def recv(self):
        ret_lines = []
        ret_lines += self.protocol.lines
        self.protocol.lines = []
        return ret_lines

    def setFrequency(self, frequency):
        message = b'F ' + repr(frequency).encode('utf-8')
        self.protocol.write_line(message.decode('utf-8'))

    def setPower(self, power):
        message = b'P ' + repr(power).encode('utf-8')
        self.protocol.write_line(message.decode('utf-8'))

    def setSpreadingFactor(self, sf):
        message = b'W ' + repr(sf).encode('utf-8')
        self.protocol.write_line(message.decode('utf-8'))
