#!/bin/bash
set -e

sudo pacman -Syyu --noconfirm 
git clone https://aur.archlinux.org/yay.git
cd yay
makepkg -si --noconfirm

yay -S --noconfirm oblogout
yay -S --noconfirm fbpanel
yay -S --noconfirm oranchelo-icon-theme
yay -S --noconfirm wmckgmail
yay -S --noconfirm bbdock
yay -S --noconfirm libdockapp
