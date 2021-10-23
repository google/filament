#!/bin/sh
#
# Check to make sure 8BitDo controller configurations are correct

echo "Expected output:"
cat <<__EOF__
    "050000003512000020ab000000780f00,8BitDo SNES30 Gamepad,a:b20,b:b21,back:b30,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b26,rightshoulder:b27,start:b31,x:b23,y:b24,hint:SDL_GAMECONTROLLER_USE_BUTTON_LABELS:=1,",
    "050000003512000020ab000000780f00,8BitDo SNES30 Gamepad,a:b21,b:b20,back:b30,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b26,rightshoulder:b27,start:b31,x:b24,y:b23,hint:!SDL_GAMECONTROLLER_USE_BUTTON_LABELS:=1,",

__EOF__

echo "Actual output:"
fgrep 8BitDo SDL_gamecontrollerdb.h | fgrep -v hint
egrep "hint:SDL_GAMECONTROLLER_USE_BUTTON_LABELS:=1" SDL_gamecontrollerdb.h  | fgrep -i 8bit | fgrep -v x:b2,y:b3 | fgrep -v x:b3,y:b4
egrep "hint:.SDL_GAMECONTROLLER_USE_BUTTON_LABELS:=1" SDL_gamecontrollerdb.h  | fgrep -i 8bit | fgrep -v x:b3,y:b2 | fgrep -v x:b4,y:b3
