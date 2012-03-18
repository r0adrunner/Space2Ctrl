#! /bin/sh

origmap=$(xmodmap -pke | grep -E "^keycode[[:blank:]]*?65")
#newmap=$(echo ${origmap} | perl -pe "s/65[[:blank:]]*?=[[:blank:]]*?space/65  = Control_L/")
newmap=$(echo ${origmap} | perl -pe "s/ space / Control_L /g")

xmodmap -e "$newmap"
xmodmap -e "keycode 255 = space VoidSymbol VoidSymbol VoidSymbol VoidSymbol"
nohup ./Space2Ctrl >> ~/.Space2Ctrl.log 2>&1 &
sleep 2 # Don't ask me why but we need to wait more than one second else the ctrl modifier is
        # reinitialized to XK_Control_L XK_Control_R as soon as keycode 65 is pressed!
xmodmap -e "add control = Control_L Control_R"
exit 0
