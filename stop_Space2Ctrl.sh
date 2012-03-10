#! /bin/sh

[ -f ~/.Space2Ctrl.pid ] && kill `cat ~/.Space2Ctrl.pid` && rm -f ~/.Space2Ctrl.pid
[ -f ~/.Space2Ctrl.backup ] && cat ~/.Space2Ctrl.backup | xmodmap - && rm -f ~/.Space2Ctrl.backup
xmodmap -e "keycode 255 ="
xmodmap -e "clear control"
xmodmap -e "add control = Control_L"
xmodmap -e "add control = Control_R"
exit 0
