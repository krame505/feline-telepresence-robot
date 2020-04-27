#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Must be run as root"
  exit
fi

cp feline_telepresence_robot.service /lib/systemd/system/
cp camera_server.service /lib/systemd/system/
systemctl enable feline_telepresence_robot
systemctl enable camera_server
systemctl start feline_telepresence_robot
systemctl start camera_server
