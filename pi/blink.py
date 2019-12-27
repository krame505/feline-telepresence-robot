#!/usr/bin/env python3
from a_star import AStar
import time

a_star = AStar()

while 1:
  a_star.led = False
  time.sleep(0.5)
  a_star.led = True
  time.sleep(0.5)
