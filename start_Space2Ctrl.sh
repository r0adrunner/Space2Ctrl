#! /bin/sh

origmap=$(xmodmap -pke | grep -E "^keycode[[:blank:]]*?65")
echo ${origmap} > ~/.Space2Ctrl.backup

newmap=$(echo ${origmap} | perl -pe "s/[[:blank:]]*?=[[:blank:]]*?space/ = Control_L/")
xmodmap -e "$newmap"
xmodmap -e "add control = Control_L"
xmodmap -e "keycode 255 = space"

nohup ./Space2Ctrl >> ~/.Space2Ctrl.log 2>&1 &
echo $! > ~/.Space2Ctrl.pid
exit 0
