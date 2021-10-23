Linux
================================================================================

By default SDL will only link against glibc, the rest of the features will be
enabled dynamically at runtime depending on the available features on the target
system. So, for example if you built SDL with Xinerama support and the target
system does not have the Xinerama libraries installed, it will be disabled
at runtime, and you won't get a missing library error, at least with the 
default configuration parameters.


Build Dependencies
--------------------------------------------------------------------------------
    
Ubuntu 20.04, all available features enabled:

    sudo apt-get install build-essential git make cmake autoconf automake \
    libtool pkg-config libasound2-dev libpulse-dev libaudio-dev libjack-dev \
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev \
    libxinerama-dev libxxf86vm-dev libxss-dev libgl1-mesa-dev libdbus-1-dev \
    libudev-dev libgles2-mesa-dev libegl1-mesa-dev libibus-1.0-dev \
    fcitx-libs-dev libsamplerate0-dev libsndio-dev libwayland-dev \
    libxkbcommon-dev libdrm-dev libgbm-dev

NOTES:
- This includes all the audio targets except arts and esd, because Ubuntu
  (and/or Debian) pulled their packages, but in theory SDL still supports them.
- libsamplerate0-dev lets SDL optionally link to libresamplerate at runtime
  for higher-quality audio resampling. SDL will work without it if the library
  is missing, so it's safe to build in support even if the end user doesn't
  have this library installed.
- DirectFB isn't included because the configure script (currently) fails to find
  it at all. You can do "sudo apt-get install libdirectfb-dev" and fix the 
  configure script to include DirectFB support. Send patches.  :)


Joystick does not work
--------------------------------------------------------------------------------

If you compiled or are using a version of SDL with udev support (and you should!)
there's a few issues that may cause SDL to fail to detect your joystick. To
debug this, start by installing the evtest utility. On Ubuntu/Debian:

    sudo apt-get install evtest
    
Then run:
    
    sudo evtest
    
You'll hopefully see your joystick listed along with a name like "/dev/input/eventXX"
Now run:
    
    cat /dev/input/event/XX

If you get a permission error, you need to set a udev rule to change the mode of
your device (see below)    
    
Also, try:
    
    sudo udevadm info --query=all --name=input/eventXX
    
If you see a line stating ID_INPUT_JOYSTICK=1, great, if you don't see it,
you need to set up an udev rule to force this variable.

A combined rule for the Saitek Pro Flight Rudder Pedals to fix both issues looks 
like:
    
    SUBSYSTEM=="input", ATTRS{idProduct}=="0763", ATTRS{idVendor}=="06a3", MODE="0666", ENV{ID_INPUT_JOYSTICK}="1"
    SUBSYSTEM=="input", ATTRS{idProduct}=="0764", ATTRS{idVendor}=="06a3", MODE="0666", ENV{ID_INPUT_JOYSTICK}="1"
   
You can set up similar rules for your device by changing the values listed in
idProduct and idVendor. To obtain these values, try:
    
    sudo udevadm info -a --name=input/eventXX | grep idVendor
    sudo udevadm info -a --name=input/eventXX | grep idProduct
    
If multiple values come up for each of these, the one you want is the first one of each.    

On other systems which ship with an older udev (such as CentOS), you may need
to set up a rule such as:
    
    SUBSYSTEM=="input", ENV{ID_CLASS}=="joystick", ENV{ID_INPUT_JOYSTICK}="1"

