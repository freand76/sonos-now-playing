#!/usr/bin/python3

from io import BytesIO
from scipy import misc
from soco.discovery import by_name
import argparse
import requests
import socket
import struct
import sys
import time


def get_album_art(room):
    device = by_name(room)
    if device == None:
        return (None, None)
    track_info = device.get_current_track_info()
    link = track_info['album_art']
    if not link:
        return (link, None)
    
    res = requests.get(link)
    img_arr = misc.imread(BytesIO(res.content))

    dest_width = 320
    src_width = len(img_arr[0])
    ratio = dest_width / src_width
    arr = misc.imresize(img_arr, ratio, interp='bilinear')
    return (link, arr)


def display_album_art(image_array, device_name):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((device_name, 80))
    
        # Clear previous image
        s.send(struct.pack('<HHHHH', 5, 0, 0, 0, 0))
        s.recv(4)

        # send 10 row in each packet
        send_rows = 10

        h_size = len(arr[0])
        out_array = [None] * (send_rows * h_size)
        pixel_pos = 0
        out_row = 0
        for row_idx, row in enumerate(arr):
            for pixel_idx, pixel in enumerate(row):
                red = (pixel[0] & 0xf8) >> 3;
                green = (pixel[1] & 0xfc) >> 2;
                blue = (pixel[2] & 0xf8) >> 3;

                new_pix = red << 11 | green << 5 | blue
                out_array[pixel_pos] = new_pix
                pixel_pos += 1
                if pixel_pos == len(out_array):
                    head = struct.pack('<HHHHH', send_rows*h_size + 5, 0, out_row, h_size, send_rows)
                    s.send(head)
                    dataArray = bytearray()

                    for data in out_array:
                        dataArray += bytearray(struct.pack('<H', data))

                    s.send(dataArray)
                    out_row += send_rows
                    s.recv(4)
                    pixel_pos = 0

                if out_row >= 240:
                    return True
        return True
    except:
        return False
    

def get_play_state(room):
    device = by_name(room)
    if device == None:
        return False
    playState=device.group.coordinator.get_current_transport_info()['current_transport_state']
    switcher = {
        'PLAYING': True,
        'TRANSITIONING': True,
        'PAUSED_PLAYBACK': False,
        'STOPPED': False,        
        }
    return switcher.get(playState)

# Here we go
parser = argparse.ArgumentParser()
parser.add_argument("device", help='The sonos device to get album art from', nargs='+')
parser.add_argument("display", help='The now playing display name (or IP address) to use', nargs='+')
result = parser.parse_args()

old_link = None

while(True):
    time.sleep(1)
    if not get_play_state(result.device[0]):
        continue

    link, arr = get_album_art(result.device[0])
    if not link:
        old_link = link
        continue

    if link != old_link:
        if display_album_art(arr, result.display[0]):
            old_link = link
        else:
            old_link = None
