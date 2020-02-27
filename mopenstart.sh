#!/bin/bash
set -e

chmod +x mopeninit.sh
chmod +x getfromyay.sh
chmod +x printer.sh 
chmod +x bluetooth.sh
chmod +x ~/fluxbox/wmtext-3/mail
chmod +x ~/fluxbox/wmtext-3/wmtext
chmod +x ~/fluxbox/wmtext-3/scripts/memory.sh
chmod +x ~/fluxbox/wmtext-3/scripts/date.sh
chmod +x createbbdockrc.sh
chmod +x jgmenu.sh

./init.sh
./getfromyay.sh

mv ~/.fluxbox ~/fluxboxOLD
cp -r fluxbox ~/.fluxbox
cp -r fbpanel ~/.config/fbpanel
mkdir ~/.config/mpv
cp mpv.conf ~/.config/mpv/mpv.conf
cp -r wmtext-3 ~/wmtext-3
cp createbbdockrc.sh ~/createbbdockrc.sh
cp jgmenu.sh ~/jgmenu.sh
cp -r wmckgmail ~/.wmckgmail 
cp -r backgrounds ~/backgrounds

cp -r myicons1 ~/myicons1
cp -r myicons2 ~/myicons2
cp -r myicons3 ~/myicons3
cp -r myicons4 ~/myicons4
cp -r myicons5 ~/myicons5
cp -r myicons6 ~/myicons6

cp XresourcesAdd ~/.Xresources
cp -r termite ~/.config/termite
cp -r jgmenu ~/.config/jgmenu

cp -r wmmenu ~/.wmmenu
cp -r xpm ~/xpm

./createbbdockrc.sh
sudo cp oblogout.conf /etc/oblogout.conf
mkdir ~/.themes
cp -r themes ~/.themes
mv ~/.xprofile ~/.xprofileOLD
