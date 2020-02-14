setxkbmap gb
[[ -f ~/.Xresources ]] && xrdb -merge -I$HOME ~/.Xresources
exec startfluxbox

