#!/usr/bin/env python3

import picamera
import time
import io
import struct
import threading
import subprocess
import os

from wsgiref.simple_server import make_server
from ws4py.websocket import WebSocket
from ws4py.server.wsgirefserver import (
    WSGIServer,
    WebSocketWSGIHandler,
    WebSocketWSGIRequestHandler,
)
from ws4py.server.wsgiutils import WebSocketWSGIApplication

WIDTH = 640
HEIGHT = 480
FRAMERATE = 24
JSMPEG_MAGIC = b'jsmp'
JSMPEG_HEADER = struct.Struct('>4sHH')
VFLIP = False
HFLIP = False

class StreamingWebSocket(WebSocket):
    def opened(self):
        self.send(JSMPEG_HEADER.pack(JSMPEG_MAGIC, WIDTH, HEIGHT), binary=True)

class BroadcastOutput:
    def __init__(self, camera):
        print('Spawning background conversion process')
        self.converter = subprocess.Popen([
            'ffmpeg',
            '-f', 'rawvideo',
            '-pix_fmt', 'yuv420p',
            '-s', '%dx%d' % camera.resolution,
            '-r', str(float(camera.framerate)),
            '-i', '-',
            '-f', 'mpeg1video',
            '-b', '800k',
            '-r', str(float(camera.framerate)),
            '-'],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=io.open(os.devnull, 'wb'),
            shell=False, close_fds=True)

    def write(self, b):
        self.converter.stdin.write(b)

    def flush(self):
        print('Waiting for background conversion process to exit')
        self.converter.stdin.close()
        self.converter.wait()
    
class BroadcastThread(threading.Thread):
    def __init__(self, converter, websocket_server):
        super(BroadcastThread, self).__init__()
        self.converter = converter
        self.websocket_server = websocket_server

    def run(self):
        try:
            while True:
                buf = self.converter.stdout.read1(32768)
                if buf:
                    self.websocket_server.manager.broadcast(buf, binary=True)
                elif self.converter.poll() is not None:
                    break
        finally:
            self.converter.stdout.close()

class CameraServer:
    def __init__(self, port):
        self.port = port
        self.running = False
    
    def run(self):
        assert not self.running
        self.running = True
        print('Initializing camera')
        try:
            with picamera.PiCamera() as camera:
                camera.resolution = (WIDTH, HEIGHT)
                camera.framerate = FRAMERATE
                camera.vflip = VFLIP # flips image rightside up, as needed
                camera.hflip = HFLIP # flips image left-right, as needed
                time.sleep(1) # camera warm-up time
                print('Initializing websockets server on port %d' % self.port)
                WebSocketWSGIHandler.http_version = '1.1'
                websocket_server = make_server(
                    '', self.port,
                    server_class=WSGIServer,
                    handler_class=WebSocketWSGIRequestHandler,
                    app=WebSocketWSGIApplication(handler_cls=StreamingWebSocket))
                websocket_server.initialize_websockets_manager()
                websocket_thread = threading.Thread(target=websocket_server.serve_forever)
                print('Initializing broadcast thread')
                output = BroadcastOutput(camera)
                broadcast_thread = BroadcastThread(output.converter, websocket_server)
                print('Starting recording')
                camera.start_recording(output, 'yuv')
                try:
                    print('Starting websockets thread')
                    websocket_thread.start()
                    print('Starting broadcast thread')
                    broadcast_thread.start()
                    while self.running:
                        camera.wait_recording(1)
                except BrokenPipeError:
                    pass
                finally:
                    try:
                        print('Stopping recording')
                        camera.stop_recording()
                    except BrokenPipeError:
                        pass
                    print('Waiting for broadcast thread to finish')
                    broadcast_thread.join()
                    print('Shutting down websockets server')
                    websocket_server.shutdown()
                    # TODO: This doesn't seem to finish when there is someone still connected
                    print('Waiting for websockets thread to finish')
                    websocket_thread.join()
        except picamera.exc.PiCameraMMALError:
            print('Pi camera is already running!')

    def stop(self):
        assert self.running
        self.running = False
    
if __name__ == '__main__':
    server = CameraServer(8084)
    
    server.run()
