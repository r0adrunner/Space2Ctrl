#! /bin/sh

origmap=$(xmodmap -pke | grep -E "^keycode[[:blank:]]*?65")
newmap=$(echo ${origmap} | perl -pe "s/ Control_L/ space/g")

xmodmap -e "$newmap"
xmodmap -e "keycode 255 ="
xmodmap -e "clear control"
xmodmap -e "add control = Control_L"
xmodmap -e "add control = Control_R"
kill -s TERM `pgrep Space2Ctrl` 2>&1
