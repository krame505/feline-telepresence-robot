#!/usr/bin/env python3

import json
import subprocess

from flask import Flask, render_template, redirect, Response
app = Flask(__name__)
#app.debug = True

from a_star import AStar
a_star = AStar()

from flask_httpauth import HTTPBasicAuth
auth = HTTPBasicAuth()

try:
    with open("/home/pi/password.txt") as f:
        password = f.readline().rstrip()
except IOError as e:
    print("Password not set!")
    password = None

@auth.verify_password
def verity_password(given_username, given_password):
    # Ignore user name for now
    #print(password, given_password, password == given_password)
    return password is None or given_password == password

@app.route("/")
@auth.login_required
def index():
    return render_template("index.html")

@app.route("/status.json")
@auth.login_required
def status():
    battery_millivolts = a_star.batteryMillivolts
    encoders = a_star.leftEncoder, a_star.rightEncoder
    speeds = a_star.leftSpeed, a_star.rightSpeed
    camera = a_star.cameraPan, a_star.cameraTilt
    laser = {"power": a_star.laserPower, "pattern": a_star.getLaserPattern()}
    data = {
        "battery_millivolts": battery_millivolts,
        "encoders": encoders,
        "speeds": speeds,
        "camera": camera,
        "laser": laser
    }
    return json.dumps(data)

@app.route("/motors/<left>,<right>")
@auth.login_required
def motors(left, right):
    try:
        a_star.leftMotor = int(left)
        a_star.rightMotor = int(right)
    except ValueError as e:
        print(e)
    return ""

@app.route("/camera/<pan>,<tilt>")
@auth.login_required
def camera(pan, tilt):
    try:
        a_star.camera(int(pan), int(tilt))
    except ValueError as e:
        print(e)
    return json.dumps((a_star.cameraPan, a_star.cameraTilt))

@app.route("/laser/<pan>,<tilt>")
@auth.login_required
def laser(pan, tilt):
    try:
        a_star.laser(int(pan), int(tilt))
    except ValueError as e:
        print(e)
    return ""

@app.route("/laserPower/<on>")
@auth.login_required
def laserPower(on):
    try:
        a_star.laserPower = bool(int(on))
    except ValueError as e:
        print(e)
    return json.dumps(on)

@app.route("/laserPattern/<pattern>")
@auth.login_required
def laserPattern(pattern):
    try:
        a_star.setLaserPattern(pattern)
    except ValueError as e:
        print(e)
    return json.dumps(pattern)

@app.route("/dispenseTreats")
@auth.login_required
def dispenseTreats():
    a_star.dispenseTreats()
    return ""

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
@auth.login_required
def play_notes(notes):
    a_star.play_notes(notes)
    return ""

@app.route("/halt")
@auth.login_required
def halt():
    subprocess.call(["bash", "-c", "(sleep 1; sudo halt)&"])
    return redirect("/shutting-down")

@app.route("/restart")
@auth.login_required
def restart():
    subprocess.call(["bash", "-c", "(sleep 1; sudo shutdown -r now)&"])
    return redirect("/restarting")

@app.route("/reset")
@auth.login_required
def reset():
    a_star.reset()
    return ""

@app.route("/shutting-down")
@auth.login_required
def shutting_down():
    return "Shutting down in 2 seconds! You can remove power when the green LED stops flashing."

@app.route("/restarting")
@auth.login_required
def restarting():
    return "<html><body><p>Restarting, you will be redirected in 15 seconds...</p><script>var timer = setTimeout(function() {{window.location='/'}}, 15000);</script></body></html>"

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=4242)
