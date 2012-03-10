#! /bin/sh

origmap=$(xmodmap -pke | grep -E "^keycode[[:blank:]]*?65")
echo ${origmap} > ~/.Space2Ctrl.backup

newmap=$(echo ${origmap} | perl -pe "s/65[[:blank:]]*?=[[:blank:]]*?space/255  = space/")
echo $newmap
xmodmap -e "$newmap"
xmodmap -e "keycode 65 = Control_L"
xmodmap -e "add control = Control_L"

nohup ./Space2Ctrl >> ~/.Space2Ctrl.log 2>&1 &
echo $! > ~/.Space2Ctrl.pid
exit 0
