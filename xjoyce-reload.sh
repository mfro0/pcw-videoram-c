#!/usr/bin/bash

WID=$(xdotool search --class xjoyce | head -n 1)
#echo $WID
xdotool windowactivate $WID
xdotool windowfocus --sync $WID
xdotool key --window $WID --clearmodifiers --delay 200 "F9" "F5" "y" "Return" "2"
sleep 4
xdotool type --window $WID --clearmodifiers --delay 500 "n:demo"
xdotool key --window $WID --delay 200 "Return"

