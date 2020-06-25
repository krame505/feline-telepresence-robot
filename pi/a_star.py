import smbus
import time
import cffi
import threading

class AStar:
  def __init__(self):
    self._bus = smbus.SMBus(1)
    self._ffi = cffi.FFI()
    self._ffi.cdef("""
struct Data {
  bool led;
  uint16_t batteryMillivolts;
  
  bool playNotes;
  char notes[14];
  
  int16_t leftMotor, rightMotor;
  int32_t leftEncoder, rightEncoder;
  int16_t leftSpeed, rightSpeed;

  bool cameraServoCommand;
  uint8_t cameraPan, cameraTilt;
  bool laserServoCommand;
  uint8_t laserPan, laserTilt;
  bool laserPower;
  uint8_t laserPattern;

  uint8_t dispenseTreatsCode;
  uint8_t resetCode;
};

enum LaserPattern {
  STEADY,
  BLINK,
  RANDOM_WALK
};
    """, packed=True)
    self._lib = self._ffi.dlopen(None)
    self._fields = dict(self._ffi.typeof("struct Data").fields)
    self._lock = threading.Semaphore()

  def __getattr__(self, fieldName):
    field = self._fields[fieldName]
    address = field.offset
    size = self._ffi.sizeof(field.type)
    
    # Ideally we could do this:
    #    byte_list = self._bus.read_i2c_block_data(20, address, size)
    # But the AVR's TWI module can't handle a quick write->read transition,
    # since the STOP interrupt will occasionally happen after the START
    # condition, and the TWI module is disabled until the interrupt can
    # be processed.
    #
    # A delay of 0.0001 (100 us) after each write is enough to account
    # for the worst-case situation in our example code.

    self._lock.acquire()
    try:
      self._bus.write_byte(20, address)
      time.sleep(0.0002)
      byte_list = [self._bus.read_byte(20) for _ in range(size)]
    except OSError as e:
      print(e)
      # Fail on read errors for now
      return None
    finally:
      self._lock.release()
      
    try:
      # Works because Arduino and RPi have the same endianness
      return self._ffi.from_buffer(self._ffi.getctype(field.type, "*"), bytes(byte_list))[0]
    except ValueError as e:
      # Fail on decoding errors for now
      return None

  def __setattr__(self, fieldName, data):
    if fieldName and fieldName[0] == '_':
      return super().__setattr__(fieldName, data)
    
    field = self._fields[fieldName]
    address = field.offset
    
    try:
      # Works because Arduino and RPi have the same endianness
      data_array = list(bytes(self._ffi.buffer(self._ffi.new(self._ffi.getctype(field.type, "*"), data))))
    except ValueError as e:
      print(e)
      # Fail on encoding errors for now
      return None
      
    self._lock.acquire()
    try:
      self._bus.write_i2c_block_data(20, address, data_array)
      time.sleep(0.0002)
    except OSError:
      # Ignore write errors for now
      pass
    finally:
      self._lock.release()

  def play_notes(self, notes):
    self.notes = bytes(notes, encoding='ASCII')
    self.playNotes = True
  
  def camera(self, pan, tilt):
    self.cameraServoCommand = True
    self.cameraPan = newPan = min(max(pan, 0), 255)
    self.cameraTilt = newTilt = min(max(tilt, 0), 255)
    return newPan, newTilt
  
  def cameraPanBy(self, delta):
    self.cameraServoCommand = True
    self.cameraPan = res = min(max(self.cameraPan + delta, 0), 255)
    return res
  
  def cameraTiltBy(self, delta):
    self.cameraServoCommand = True
    self.cameraTilt = res = min(max(self.cameraTilt + delta, 0), 255)
    return res
  
  def laser(self, pan, tilt):
    self.laserServoCommand = True
    self.laserPan = min(max(pan, 0), 255)
    self.laserTilt = min(max(tilt, 0), 255)

  def getLaserPattern(self):
    try:
      return self._ffi.typeof('enum LaserPattern').elements[self.laserPattern].lower()
    except KeyError:
      return None

  def setLaserPattern(self, pattern):
    try:
      self.laserPattern = getattr(self._lib, pattern.upper())
    except KeyError:
      return None

  def dispenseTreats(self):
    self.dispenseTreatsCode = 0xAA # Non-trivial bit pattern to avoid accidental release due to bit errors

  def reset(self):
    self.resetCode = 0xBB
