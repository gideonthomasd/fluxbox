# fluxbox
Change sh settings: chmod +x init.sh && chmod +x getfromyay.sh && chmod +x printer.sh && chmod +x bluetooth.sh

Run first 2 programs.  Copy fbpanel dir into .config dir.  Copy files into ./fluxbox

Copy bashrcAdd and XresourcesAdd to respective files.  Copy termite to .config

Copy mpv.conf to /.config/mpv folder

Change lxappearance.

Add to /etc/sudoers: %sudo ALL = NOPASSWD: /sbin/shutdown, /sbin/poweroff, /sbin/halt, /sbin/reboot

This enables reboot and shutdown in menu (no sudo necessary)

copy wmtext-3 to /home/phil - make sure sh files are executable

(Change wmckgmail to .wmckgmail in home/phil  In terminal do wmckgmail & then disown) - not needed see above

Change createbbdockrc to exec.  Run to create bbdock.
jgmenu.sh change to exec
(In wmtext-3 do 'sudo make install' to put into /usr/bin/local)

In termite do mmaker -c.  Cut and paste into mymenu. In menu do [include] (~/.fluxbox/mymenu)
