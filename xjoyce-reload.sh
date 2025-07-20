#!/usr/bin/bash

xdotool keyup Control_L Control_R R		# release Control keys that might have been used from QtCreator
WID=$(xdotool search --class xjoyce | head -n 1)
#echo $WID
xdotool windowactivate $WID
xdotool windowfocus --sync $WID
xdotool key --window $WID --clearmodifiers --delay 250 "F9" "F5" "y" "Return" "2"
sleep 4
xdotool type --window $WID --clearmodifiers --delay 350 "n:demo"
xdotool key --window	 $WID --clearmodifiers --delay 250 "Return"
