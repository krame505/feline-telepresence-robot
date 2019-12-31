// Copyright Pololu Corporation.  For more information, see https://www.pololu.com/
var stop_motors = true
var block_set_motors = false
var block_set_camera = false
var mouse_dragging = false

var camera_pan = 0
var camera_tilt = 0
var camera_pan_field = 65
var camera_tilt_field = 50

function init() {
    poll()
    $("#joystick").bind("touchstart", joystickTouchMove)
    $("#joystick").bind("touchmove", joystickTouchMove)
    $("#joystick").bind("touchend", joystickTouchEnd)
    $("#joystick").bind("mousedown", joystickMouseDown)
    $("#cameraFeed").bind("mousedown", cameraMouseDown)
    $("#cameraFeed").bind("touchstart", cameraTouchStart)
    $(document).bind("mousemove", joystickMouseMove)
    $(document).bind("mouseup", joystickMouseUp)
    $(document).keydown(function (e) {
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
    });
}

function poll() {
    $.ajax({url: "status.json"}).done(updateStatus)
    if (stop_motors && !block_set_motors) {
	setMotors(0,0);
	stop_motors = false
    }
}

function updateStatus(json) {
    s = JSON.parse(json)

    $("#battery_millivolts").html(s["battery_millivolts"])
    
    $("#encoders0").html(s["encoders"][0])
    $("#encoders1").html(s["encoders"][1])

    camera_pan = s["camera"][0]
    camera_tilt = s["camera"][1]
    $("#cameraPos").html("Camera: " + camera_pan + " " + camera_tilt)

    setTimeout(poll, 100)
}

function joystickTouchMove(e) {
    e.preventDefault()
    touch = e.originalEvent.touches[0] || e.originalEvent.changedTouches[0];
    joystickDragTo(touch.pageX, touch.pageY)
}

function joystickMouseDown(e) {
    e.preventDefault()
    mouse_dragging = true
}

function joystickMouseUp(e) {
    if (mouse_dragging) {
	e.preventDefault()
	mouse_dragging = false
	stop_motors = true
    }
}

function joystickMouseMove(e) {
    if (mouse_dragging) {
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

    left_motor = Math.round(400*(-y-x))
    right_motor = Math.round(400*(-y+x))

    if (left_motor > 400) left_motor = 400
    if (left_motor < -400) left_motor = -400

    if (right_motor > 400) right_motor = 400
    if (right_motor < -400) right_motor = -400

    stop_motors = false
    setMotors(left_motor, right_motor)
}

function joystickTouchEnd(e) {
    e.preventDefault()
    stop_motors = true
}

function setMotors(left, right) {
    $("#joystick").html("Motors: " + left + " " + right)

    if (block_set_motors) return
    block_set_motors = true

    $.ajax({url: "motors/" + left + "," + right}).done(function () {
	block_set_motors = false
    })
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

function cameraUp() {
    setCamera(camera_pan, camera_tilt + 10)
}

function cameraDown() {
    setCamera(camera_pan, camera_tilt - 10)
}

function cameraLeft() {
    setCamera(camera_pan + 10, camera_tilt)
}

function cameraRight() {
    setCamera(camera_pan - 10, camera_tilt)
}

function setCamera(pan, tilt) {
    if (block_set_camera) return
    block_set_camera = true

    if (!pan)  pan = 0;
    if (!tilt) tilt = 0;
    
    $.ajax({url: "camera/" + pan + "," + tilt}).done(function (json) {
	c = JSON.parse(json)
	camera_pan = c[0]
	camera_tilt = c[1]
	$("#cameraPos").html("Camera: " + camera_pan + " " + camera_tilt)
	block_set_camera = false
    })
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
