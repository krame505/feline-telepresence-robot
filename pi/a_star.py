import smbus
import time
import cffi

class AStar:
  def __init__(self):
    self._bus = smbus.SMBus(1)
    self._ffi = cffi.FFI()
    self._ffi.cdef("""
struct Data {
  bool led;
  uint16_t batteryMillivolts;
  uint16_t analog[6];
  
  bool playNotes;
  char notes[14];
  
  int16_t leftMotor, rightMotor;
  int32_t leftEncoder, rightEncoder;
};
    """, packed=True)
    self._fields = dict(self._ffi.typeof("struct Data").fields)

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

    try:
      self._bus.write_byte(20, address)
      time.sleep(0.0002)
      byte_list = [self._bus.read_byte(20) for _ in range(size)]
    except OSError as e:
      print(e)
      # Fail on read errors for now
      return None
    
    # Works because Arduino and RPi have the same endianness
    return self._ffi.from_buffer(self._ffi.getctype(field.type, "*"), bytes(byte_list))[0]
    

  def __setattr__(self, fieldName, data):
    if fieldName and fieldName[0] == '_':
      return super().__setattr__(fieldName, data)
    
    field = self._fields[fieldName]
    address = field.offset
    
    # Works because Arduino and RPi have the same endianness
    data_array = list(bytes(self._ffi.buffer(self._ffi.new(self._ffi.getctype(field.type, "*"), data))))
    
    try:
      self._bus.write_i2c_block_data(20, address, data_array)
    except OSError:
      # Ignore write errors for now
      pass
    time.sleep(0.0002)

  def play_notes(self, notes):
    self.notes = bytes(notes, encoding='ASCII')
    self.playNotes = True
