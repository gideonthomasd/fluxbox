#!/bin/bash
set -e


sudo pacman -Syyu --noconfirm
sudo pacman -S menumaker lxsession lxappearance arc-gtk-theme xarchiver xterm base-devel nitrogen firefox git geany --noconfirm
sudo pacman -S lightdm lightdm-gtk-greeter lightdm-gtk-greeter-settings pcmanfm --noconfirm
sudo pacman -S networkmanager network-manager-applet --noconfirm
sudo pacman -S xorg-server xorg xorg-xinit --noconfirm
sudo pacman -S moka-icon-theme --noconfirm
sudo pacman -S w3m fluxbox conky volumeicon mpv youtube-dl --noconfirm
sudo pacman -S pavucontrol pulseaudio lxterminal --noconfirm
sudo pacman -S gmrun termite ntfs-3g slock --noconfirm
sudo pacman -S neofetch rofi gtk2-perl lsb-release --noconfirm
sudo pacman -S adapta-gtk-theme gvfs pdfmixtool xpdf netsurf --noconfirm
sudo pacman -S virtualbox-guest-utils deadbeef unzip wget jgmenu orage --noconfirm
sudo systemctl start NetworkManager
sudo systemctl enable NetworkManager
#sudo systemctl enable lightdm
