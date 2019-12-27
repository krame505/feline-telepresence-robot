#!/usr/bin/env python3

from flask import Flask
from flask import render_template
from flask import redirect
from subprocess import call
app = Flask(__name__)
app.debug = True

from a_star import AStar
a_star = AStar()

import json

led_state = False

@app.route("/")
def hello():
    return render_template("index.html")

@app.route("/status.json")
def status():
    battery_millivolts = a_star.batteryMillivolts
    encoders = a_star.leftEncoder, a_star.rightEncoder
    data = {
        "battery_millivolts": battery_millivolts,
        "encoders": encoders
    }
    return json.dumps(data)

@app.route("/motors/<left>,<right>")
def motors(left, right):
    a_star.leftMotor = int(left)
    a_star.rightMotor = int(right)
    return ""

@app.route("/led/<int:led>")
def leds(led):
    a_star.led = led
    global led_state
    led_state = led
    return ""

@app.route("/heartbeat/<int:state>")
def heartbeat(state):
    if state == 0:
        a_star.led(led_state)
    else:
        a_star.led(not led_state)
    return ""

@app.route("/play_notes/<notes>")
def play_notes(notes):
    a_star.play_notes(notes)
    return ""

@app.route("/halt")
def halt():
    call(["bash", "-c", "(sleep 2; sudo halt)&"])
    return redirect("/shutting-down")

@app.route("/shutting-down")
def shutting_down():
    return "Shutting down in 2 seconds! You can remove power when the green LED stops flashing."

if __name__ == "__main__":
    app.run(host = "0.0.0.0")
