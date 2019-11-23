#!/bin/bash
set -e

sudo pacman -S bluez bluez-utils blueberry

sudo systemctl enable bluetooth
sudo systemctl start bluetooth

echo "Connect to blueberry.  Connect Marathon or device.  Close dialog"
