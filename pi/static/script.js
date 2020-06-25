// Copyright Pololu Corporation.  For more information, see https://www.pololu.com/

// Handling for mouse "joystick"s
var joystick_mouse_dragging = false
var laser_mouse_dragging = false

// Current camera position
var camera_pan = 0
var camera_tilt = 0

// Approximate camera field of view, in degrees
var camera_pan_field = 65
var camera_tilt_field = 50

function init() {
    poll()
    setMotors(0,0)
    
    $(document).keydown(keydown);
    $("#joystick").bind("touchstart", joystickTouchMove)
    $("#joystick").bind("touchmove", joystickTouchMove)
    $("#joystick").bind("touchend", joystickTouchEnd)
    $("#joystick").bind("mousedown", joystickMouseDown)
    $(document).bind("mousemove", joystickMouseMove)
    $(document).bind("mouseup", joystickMouseUp)
    $("#laser").bind("touchstart", laserTouchMove)
    $("#laser").bind("touchmove", laserTouchMove)
    $("#laser").bind("touchend", laserTouchEnd)
    $("#laser").bind("mousedown", laserMouseDown)
    $(document).bind("mousemove", laserMouseMove)
    $(document).bind("mouseup", laserMouseUp)
    $("#cameraFeed").bind("mousedown", cameraMouseDown)
    $("#cameraFeed").bind("touchstart", cameraTouchStart)
}

function poll() {
    $.ajax({url: "status.json"}).done(updateStatus).fail(function (jqXHR, textStatus) { setTimeout(poll, 3000) })
}

function updateStatus(json) {
    s = JSON.parse(json)

    $("#battery_millivolts").html(s["battery_millivolts"])
    
    $("#encoders0").html(s["encoders"][0])
    $("#encoders1").html(s["encoders"][1])
    $("#speeds0").html(s["speeds"][0])
    $("#speeds1").html(s["speeds"][1])

    camera_pan = s["camera"][0]
    camera_tilt = s["camera"][1]
    $("#cameraPos").html("Camera: " + camera_pan + " " + camera_tilt)

    laser_power = s["laser"]["power"]
    laser_pattern = s["laser"]["pattern"]
    $('#laserPower')[0].checked = laser_power
    $('#steady')[0].checked = laser_pattern == 'steady'
    $('#blink')[0].checked = laser_pattern == 'blink'
    $('#random_walk')[0].checked = laser_pattern == 'random_walk'

    setTimeout(poll, 100)
}

function keydown(e) {
    switch (e.which) {
    case 37: // left
	cameraLeft()
        break;

    case 38: // up
	cameraUp()
        break;

    case 39: // right
	cameraRight()
        break;

    case 40: // down
	cameraDown()
        break;

    default: return; // exit this handler for other keys
    }
    e.preventDefault(); // prevent the default action (scroll / move caret)
}

function joystickTouchMove(e) {
    e.preventDefault()
    touch = e.originalEvent.touches[0] || e.originalEvent.changedTouches[0];
    joystickDragTo(touch.pageX, touch.pageY)
}

function joystickMouseDown(e) {
    e.preventDefault()
    joystick_mouse_dragging = true
}

function joystickMouseUp(e) {
    if (joystick_mouse_dragging) {
	e.preventDefault()
	joystick_mouse_dragging = false
	setMotors(0,0)
    }
}

function joystickMouseMove(e) {
    if (joystick_mouse_dragging) {
	e.preventDefault()
	joystickDragTo(e.pageX, e.pageY)
    }
}

function joystickDragTo(x, y) {
    elm = $('#joystick').offset()
    x = x - elm.left
    y = y - elm.top
    w = $('#joystick').width()
    h = $('#joystick').height()

    x = (x-w/2.0)/(w/2.0)
    y = (y-h/2.0)/(h/2.0)

    if (x < -1) x = -1
    if (x > 1) x = 1
    if (y < -1) y = -1
    if (y > 1) y = 1

    left_motor = Math.round(400*(-y+x))
    right_motor = Math.round(400*(-y-x))

    if (left_motor > 400) left_motor = 400
    if (left_motor < -400) left_motor = -400

    if (right_motor > 400) right_motor = 400
    if (right_motor < -400) right_motor = -400

    setMotors(left_motor, right_motor)
}

function joystickTouchEnd(e) {
    e.preventDefault()
    setMotors(0,0)
}

function setMotors(left, right) {
    $("#joystick").html("Motors: " + left + " " + right)

    $.ajax({url: "motors/" + left + "," + right})
}

function laserMouseDown(e) {
    e.preventDefault()
    laser_mouse_dragging = true
}

function laserMouseMove(e) {
    e.preventDefault()
    if (laser_mouse_dragging) {
	elem = $('#laser').offset()
	x = e.pageX - elem.left
	y = e.pageY - elem.top
	w = $('#laser').width()
	h = $('#laser').height()
	if (x > 0 && x < w && y > 0 && y < h) {
	    pan = Math.round((1 - x / w) * 180)
	    tilt = Math.round((1 - y / h) * 180)
	    setLaser(pan, tilt)
	}
    }
}

function laserTouchMove(e) {
    e.preventDefault()
    touch = e.originalEvent.touches[0] || e.originalEvent.changedTouches[0];
    elem = $('#laser').offset()
    x = touch.pageX - elem.left
    y = touch.pageY - elem.top
    w = $('#laser').width()
    h = $('#laser').height()
    if (x > 0 && x < w && y > 0 && y < h) {
	pan = Math.round((1 - x / w) * 180)
	tilt = Math.round((1 - y / h) * 180)
	setLaser(pan, tilt)
    }
}

function laserMouseUp(e) {
    e.preventDefault()
    if (laser_mouse_dragging) {
	laser_mouse_dragging = false
    }
}

function laserTouchEnd(e) {
    e.preventDefault()
}

function setLaser(pan, tilt) {
    if (!pan)  pan = 0;
    if (!tilt) tilt = 0;
    
    $("#laser").html("Laser: " + pan + " " + tilt)
    
    $.ajax({url: "laser/" + pan + "," + tilt})
}

function setLaserPower() {
    power = $('#laserPower')[0].checked
    $.ajax({url: "laserPower/" + (power? 1 : 0)})
}

function setLaserPattern(pattern) {
    $.ajax({url: "laserPattern/" + pattern})
}

function cameraMouseDown(e) {
    e.preventDefault()
    elem = $('#camera').offset()
    x = e.pageX - elem.left
    y = e.pageY - elem.top
    w = $('#camera').width()
    h = $('#camera').height()
    if (x > 0 && x < w && y > 0 && y < h) {
	x = (x-w/2.0)/w
	y = (y-h/2.0)/h
	pan = camera_pan - Math.round(x * camera_pan_field)
	tilt = camera_tilt - Math.round(y * camera_tilt_field)
	setCamera(pan, tilt)
    }
}

function cameraTouchStart(e) {
    e.preventDefault()
    touch = e.originalEvent.touches[0]
    elem = $('#camera').offset()
    x = touch.pageX - elem.left
    y = touch.pageY - elem.top
    w = $('#camera').width()
    h = $('#camera').height()
    if (x > 0 && x < w && y > 0 && y < h) {
	x = (x-w/2.0)/w
	y = (y-h/2.0)/h
	pan = camera_pan - Math.round(x * camera_pan_field)
	tilt = camera_tilt - Math.round(y * camera_tilt_field)
	setCamera(pan, tilt)
    }
}

function cameraLeft() {
    cameraPanBy(10)
}

function cameraRight() {
    cameraPanBy(-10)
}

function cameraUp() {
    cameraTiltBy(10)
}

function cameraDown() {
    cameraTiltBy(-10)
}

function setCamera(pan, tilt) {
    if (!pan)  pan = 0;
    if (!tilt) tilt = 0;
    
    $.ajax({url: "camera/" + pan + "," + tilt}).done(function (json) {
	c = JSON.parse(json)
	camera_pan = c[0]
	camera_tilt = c[1]
	$("#cameraPos").html("Camera: " + camera_pan + " " + camera_tilt)
    })
}

function cameraPanBy(delta) {
    if (!delta)  delta = 0;
    
    $.ajax({url: "cameraPanBy/" + delta}).done(function (json) {
	camera_pan = JSON.parse(json)
	$("#cameraPos").html("Camera: " + camera_pan + " " + camera_tilt)
    })
}

function cameraTiltBy(delta) {
    if (!delta)  delta = 0;
    
    $.ajax({url: "cameraTiltBy/" + delta}).done(function (json) {
	camera_tilt = JSON.parse(json)
	$("#cameraPos").html("Camera: " + camera_pan + " " + camera_tilt)
    })
}

function dispenseTreats() {
    $.ajax({url: "dispenseTreats"})
}

function setLed() {
    led = $('#led')[0].checked ? 1 : 0
    $.ajax({url: "led/" + led})
}

function playNotes() {
    notes = $('#notes').val()
    $.ajax({url: "play_notes/" + notes})
}

function shutdown() {
    if (confirm("Really shut down the Raspberry Pi?"))
	return true
    return false
}

function restart() {
    if (confirm("Really reboot the Raspberry Pi?"))
	return true
    return false
}

function reset() {
    $.ajax({url: "reset"})
}
