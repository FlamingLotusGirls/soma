#!/usr/bin/python3
# vi:set ai sw=4 ts=4 et smarttab:
#
# Copyright 2024 Brian Bulkowski <brian@bulkowski.org>
# with code Copywright 2014 Michael Toren <mct@toren.net> in Perl as a starting point
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# send.py
# The original bulb test code implements sending the 'FLG protocol' over RS485 in Perl.
# Perl is a little annoying to use and install now. Can't get it working on a Windows machine.
# so a quick re-write in Python3 will really help us debug stuff faster

# pyserial
import serial
import argparse


crc8_table = [
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3 ]

FRAME_END    = 0xDA
FRAME_ESC    = 0xDB
FRAME_TEND   = 0xDC
FRAME_TESC   = 0xDD

PROTO_NOOP         = 0x11
PROTO_BAUD         = 0x12
PROTO_LATCH        = 0x13
PROTO_RGBDATA      = 0x14
PROTO_RGBLATCH     = 0x15

# while it may seem more logical to do a Dict here, I want to do a case insensitive match, which,
# without getting super fancy, it best done by iterating a list
commands = [['NOOP', PROTO_NOOP],
             ['BAUD', PROTO_BAUD],
             ['BITRATE', PROTO_BAUD],
             ['LATCH', PROTO_LATCH],
             ['RGBDATA', PROTO_RGBDATA],
             ['RGB', PROTO_RGBLATCH],
             ['RGBLATCH', PROTO_RGBLATCH]]


# my $device  = (glob "/dev/ttyUSB*")[0] || (glob "/dev/tty.usbserial*")[0] || "";
# my $verbose = 0;
# my $dry_run = 0;
# my $raw     = 0;
# my $baud    = 230400; # 115200*2
# my $allbaud;
# my $loop;
# my $port;
# my $help;
# my $sendbytes;

# my @allbauds = (9600, 19200, 38400, 115200, 230400);

def openSerial(device: str, speed: int):
#    print (f'Opening {device} at speed {speed}')

#    $port = new Device::SerialPort($device) or die "$device: $!";
# 8,N,1 no handshake is the default for this library
# https://pyserial.readthedocs.io/en/latest/pyserial_api.html
# alternately, if you want to declare everything seprately, do it, then call open

    port = None
    try:
        port = serial.Serial(device, speed, timeout=1.0,
            parity=serial.PARITY_NONE,bytesize=serial.EIGHTBITS,stopbits=serial.STOPBITS_ONE);
    except ValueError:
        print ("can't open serial port, something is out of bounds")
        exit()
    except serial.SerialException:
        print (f'device {device} not found')
        exit()

    return port


def escape(srcBuf: bytes) -> bytes :

    # it is better to two-pass, because often we dont have to escape, saves an object
    # and if we do have to escape, we can create the object once
    escapes = 0
    for b in srcBuf:
        if (b == FRAME_END) or (b == FRAME_ESC) :
            escapes += 1

    # fast path, common to not need any escapes
    if escapes == 0:
        return srcBuf

    destBuf = bytearray(len(srcBuf)+escapes)
    e = 0
    for idx, b in srcBuf:
        if (srcBuf[idx] == FRAME_END):
            destBuf[idx+e] = FRAME_ESC
            destBuf[idx+e+1] = FRAME_TEND
            e += 1
        elif (srcBuf[idx] == FRAME_ESC):
            destBuf[idx+e] = FRAME_ESC
            destBuf[idx+e+1] = FRAME_TESC;
            e += 1
        else:
            destBuf[idx+e] = srcBuf[idx]

    return( bytes(destBuf) )

# creates an FLG package

def packet(address: int, operation: int, buf: bytes ) -> bytes :

    p = bytearray(3)
    # this frame end at the front seems either wrong or superfulous?
    p[0] = FRAME_END
    p[1] = address
    p[2] = operation

    if buf is not None:
        p +=  escape(buf) 


    # calculate CRC. hard to tell here what the CRC covers, do we also add address and opertion?
    # good news, I think I have a working example....
    crc = 0
    for b in p[1:] :
        crc = crc8_table[ b ^ crc ]
    p.append( crc )
    p.append( FRAME_END )

    return bytes(p)


# the old code had a 'dryrun' and 'verbose' and 'raw' option.
# skipping those for now. 

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Send FLG protocol over 485 for testing SOMA bulbs')
    parser.add_argument('-p', '--port', required=True, type=str, help='Port is the serial device, required, full string' )
    parser.add_argument('-b', '--baud', type=int, default=230400, help='baud to set serial port to, defaults to 230400')
    parser.add_argument('-c', '--command', type=str, default='RGBLATCH', help='Command that will be sent')
    parser.add_argument('-a', '--address', type=str, default='0xff', help='Address of destination fixture IN HEX')
    parser.add_argument('--color', type=str, help='string of form ffffff of rgb IN HEX')
    args = parser.parse_args()
    if args == None : 
        print( 'Arguments incorrect' )
        parser.print_help()
        exit()


    # get command
    cmd = 0
    for c in commands:
        if c[0].casefold() == args.command.casefold():
            cmd = c[1]

    # convert color to num_bytes + bytes (4 bytes total), only used if rgb
    color = None
    if hasattr(args,'color') and isinstance(args.color, str):
        colorArg = int(args.color, 16)
        color = bytes([ 1, colorArg >> 16, (colorArg >> 8) & 0xff, colorArg & 0xff ])

    if cmd == PROTO_RGBLATCH or cmd == PROTO_RGBDATA :
        if color == None:
            print(' Must provide color if sending rgblatch or rgbdata')
            exit()

    # create serial
    ser = openSerial(args.port, args.baud)

    # create buffer to write
    buf = packet(int(args.address, 16), cmd, color)

    # USEFUL PRINT - althought it would be nicer to have an easier to read hex string (python 3.6)
    # print( 'sending packet: ' + buf.hex() )

    ser.write(buf)

    ser.close()

