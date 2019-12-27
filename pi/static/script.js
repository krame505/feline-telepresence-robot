// Copyright Pololu Corporation.  For more information, see https://www.pololu.com/
stop_motors = true
block_set_motors = false
mouse_dragging = false

camera_pan = 0
camera_tilt = 0

function init() {
    poll()
    $("#joystick").bind("touchstart",touchmove)
    $("#joystick").bind("touchmove",touchmove)
    $("#joystick").bind("touchend",touchend)
    $("#joystick").bind("mousedown",mousedown)
    $(document).bind("mousemove",mousemove)
    $(document).bind("mouseup",mouseup)
    $(document).keydown(function(e) {
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
    $.ajax({url: "status.json"}).done(update_status)
    if (stop_motors && !block_set_motors) {
	setMotors(0,0);
	stop_motors = false
    }
}

function update_status(json) {
    s = JSON.parse(json)

    $("#battery_millivolts").html(s["battery_millivolts"])
    
    $("#encoders0").html(s["encoders"][0])
    $("#encoders1").html(s["encoders"][1])

    camera_pan = s["camera"][0]
    camera_tilt = s["camera"][1]
    $("#camera").html("Camera: " + camera_pan + " " + camera_tilt)

    setTimeout(poll, 100)
}

function touchmove(e) {
    e.preventDefault()
    touch = e.originalEvent.touches[0] || e.originalEvent.changedTouches[0];
    dragTo(touch.pageX, touch.pageY)
}

function mousedown(e) {
    e.preventDefault()
    mouse_dragging = true
}

function mouseup(e) {
    if (mouse_dragging) {
	e.preventDefault()
	mouse_dragging = false
	stop_motors = true
    }
}

function mousemove(e) {
    if (mouse_dragging) {
	e.preventDefault()
	dragTo(e.pageX, e.pageY)
    }
}

function dragTo(x, y) {
    elm = $('#joystick').offset();
    x = x - elm.left;
    y = y - elm.top;
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

    stop_motors = false
    setMotors(left_motor, right_motor)
}

function touchend(e) {
    e.preventDefault()
    stop_motors = true
}

function setMotors(left, right) {
    $("#joystick").html("Motors: " + left + " " + right)

    if (block_set_motors) return
    block_set_motors = true

    $.ajax({url: "motors/" + left + "," + right}).done(setMotorsDone)
}

function setMotorsDone() {
    block_set_motors = false
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
    $.ajax({url: "camera/" + pan + "," + tilt})
}

function setLed() {
    led = $('#led')[0].checked ? 1 : 0
    $.ajax({url: "led/"+led})
}

function playNotes() {
    notes = $('#notes').val()
    $.ajax({url: "play_notes/"+notes})
}

function shutdown() {
    if (confirm("Really shut down the Raspberry Pi?"))
	return true
    return false
}
