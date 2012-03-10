#! /bin/sh

[ -f ~/.Space2Ctrl.pid ] && kill `cat ~/.Space2Ctrl.pid` && rm -f ~/.Space2Ctrl.pid
cat ~/.Space2Ctrl.backup | xmodmap -
xmodmap -e "keycode 255 ="
exit 0
