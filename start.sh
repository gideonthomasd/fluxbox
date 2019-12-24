chmod +x init.sh
chmod +x getfromyay.sh
chmod +x printer.sh 
chmod +x bluetooth.sh
chmod +x /wmtext-3/mail
chmod +x /wmtext-3/wmtext
chmod +x /wmtext-3/scripts/memory.sh
chmod +x /wmtext-3/scripts/date.sh
chmod +x createbbdockrc.sh
chmod +x jgmenu.sh

./init.sh
./getfromyay.sh

mv ~/.fluxbox ~/fluxboxOLD
cp fluxbox ~/.fluxbox
cp fbpanel ~/.config/fbpanel
cp mpv.conf ~/.config/mpv/mpv.conf
cp wmtext-3 ~/wmtext-3
cp createbbdockrc.sh ~/createbbdockrc.sh
cp jgmenu.sh ~/jgmenu.sh
cp wmckgmail ~/.wmckgmail 

cp myicons1 ~/myicons1
cp myicons2 ~/myicons2
cp myicons3 ~/myicons3
cp myicons4 ~/myicons4
cp myicons5 ~/myicons5
cp myicons6 ~/myicons6

createbbdockrc.sh
