#! /bin/sh

# 37 Control_L					# May Differ for your System. Use 'xev' to get the right codes
# 65 space
# 105 Control_R
# 64 Alt_L
# 108 Alt_R (ISO_Level3_Shift)
# 66 Caps_Lock      
# 133 Left Windows Flag
# 135 Right Menu


#xmodmap -e "clear Lock"				
#xmodmap -e "keycode 66 = Meta_L"		# Caps Lock = Meta key
#xmodmap -e "add Mod1 = Meta_L"

xmodmap -e "keysym space = Control_L"   	# Space bar = Left control key
xmodmap -e "add control = Control_L"

./Space2Ctrl

