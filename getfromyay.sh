#!/bin/bash
set -e

sudo pacman -Syyu --noconfirm 
git clone https://aur.archlinux.org/yay.git
cd yay
makepkg -si

yay -S fbpanel
yay -S numix-circle-icon-theme-git
yay -S oranchelo-icon-theme
yay -S wmckgmail
yay -S bbdock
yay -S libdockapp
