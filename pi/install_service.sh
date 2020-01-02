#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Must be run as root"
  exit
fi

cp a_star_heartbeat.service /lib/systemd/system/
systemctl enable feline_telepresence_robot
systemctl start feline_telepresence_robot
