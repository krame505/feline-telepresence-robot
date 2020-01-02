#!/usr/bin/env python3

import json
import subprocess

from flask import Flask, render_template, redirect, Response
app = Flask(__name__)
#app.debug = True

from a_star import AStar
a_star = AStar()

import camera
camera_server = camera.CameraServer()

@app.route("/")
def hello():
    return render_template("index.html")

@app.route("/status.json")
def status():
    battery_millivolts = a_star.batteryMillivolts
    encoders = a_star.leftEncoder, a_star.rightEncoder
    camera = a_star.cameraPan, a_star.cameraTilt
    laser = {"power": a_star.laserPower, "pattern": a_star.getLaserPattern()}
    data = {
        "battery_millivolts": battery_millivolts,
        "encoders": encoders,
        "camera": camera,
        "laser": laser
    }
    return json.dumps(data)

@app.route("/motors/<left>,<right>")
def motors(left, right):
    try:
        a_star.leftMotor = int(left)
        a_star.rightMotor = int(right)
    except ValueError as e:
        print(e)
    return ""

@app.route("/camera/<pan>,<tilt>")
def camera(pan, tilt):
    try:
        a_star.camera(int(pan), int(tilt))
    except ValueError as e:
        print(e)
    return json.dumps((a_star.cameraPan, a_star.cameraTilt))

@app.route("/laser/<pan>,<tilt>")
def laser(pan, tilt):
    try:
        a_star.laser(int(pan), int(tilt))
    except ValueError as e:
        print(e)
    return ""

@app.route("/laserPower/<on>")
def laserPower(on):
    try:
        a_star.laserPower = bool(int(on))
    except ValueError as e:
        print(e)
    return json.dumps(on)

@app.route("/laserPattern/<pattern>")
def laserPattern(pattern):
    try:
        a_star.setLaserPattern(pattern)
    except ValueError as e:
        print(e)
    return json.dumps(pattern)

@app.route("/dispenseTreats")
def dispenseTreats():
    a_star.dispenseTreats = True

led_state = False
@app.route("/led/<int:led>")
def led(led):
    a_star.led = led
    global led_state
    led_state = led
    return ""

@app.route("/heartbeat/<int:state>")
def heartbeat(state):
    if state == 0:
        a_star.led = led_state
    else:
        a_star.led = not led_state
    return ""

@app.route("/play_notes/<notes>")
def play_notes(notes):
    a_star.play_notes(notes)
    return ""

@app.route("/halt")
def halt():
    subprocess.call(["bash", "-c", "(sleep 2; sudo halt)&"])
    return redirect("/shutting-down")

@app.route("/shutting-down")
def shutting_down():
    return "Shutting down in 2 seconds! You can remove power when the green LED stops flashing."

@app.before_first_request
def start_camera():
    pass

if __name__ == "__main__":
    camera_server.start()
    app.run(host = "0.0.0.0")
    camera_server.stop()
