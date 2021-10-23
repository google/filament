Raspberry Pi
================================================================================

Requirements:

Raspbian (other Linux distros may work as well).

================================================================================
 Features
================================================================================

* Works without X11
* Hardware accelerated OpenGL ES 2.x
* Sound via ALSA
* Input (mouse/keyboard/joystick) via EVDEV
* Hotplugging of input devices via UDEV


================================================================================
 Raspbian Build Dependencies
================================================================================

sudo apt-get install libudev-dev libasound2-dev libdbus-1-dev

You also need the VideoCore binary stuff that ships in /opt/vc for EGL and 
OpenGL ES 2.x, it usually comes pre-installed, but in any case:
    
sudo apt-get install libraspberrypi0 libraspberrypi-bin libraspberrypi-dev


================================================================================
 NEON
================================================================================

If your Pi has NEON support, make sure you add -mfpu=neon to your CFLAGS so
that SDL will select some otherwise-disabled highly-optimized code. The
original Pi units don't have NEON, the Pi2 probably does, and the Pi3
definitely does.

================================================================================
 Cross compiling from x86 Linux
================================================================================

To cross compile SDL for Raspbian from your desktop machine, you'll need a
Raspbian system root and the cross compilation tools. We'll assume these tools
will be placed in /opt/rpi-tools

    sudo git clone --depth 1 https://github.com/raspberrypi/tools /opt/rpi-tools

You'll also need a Raspbian binary image.
Get it from: http://downloads.raspberrypi.org/raspbian_latest 
After unzipping, you'll get file with a name like: "<date>-wheezy-raspbian.img"
Let's assume the sysroot will be built in /opt/rpi-sysroot.

    export SYSROOT=/opt/rpi-sysroot
    sudo kpartx -a -v <path_to_raspbian_image>.img
    sudo mount -o loop /dev/mapper/loop0p2 /mnt
    sudo cp -r /mnt $SYSROOT
    sudo apt-get install qemu binfmt-support qemu-user-static
    sudo cp /usr/bin/qemu-arm-static $SYSROOT/usr/bin
    sudo mount --bind /dev $SYSROOT/dev
    sudo mount --bind /proc $SYSROOT/proc
    sudo mount --bind /sys $SYSROOT/sys

Now, before chrooting into the ARM sysroot, you'll need to apply a workaround,
edit $SYSROOT/etc/ld.so.preload and comment out all lines in it.

    sudo chroot $SYSROOT
    apt-get install libudev-dev libasound2-dev libdbus-1-dev libraspberrypi0 libraspberrypi-bin libraspberrypi-dev libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxinerama-dev libxxf86vm-dev libxss-dev
    exit
    sudo umount $SYSROOT/dev
    sudo umount $SYSROOT/proc
    sudo umount $SYSROOT/sys
    sudo umount /mnt
    
There's one more fix required, as the libdl.so symlink uses an absolute path 
which doesn't quite work in our setup.

    sudo rm -rf $SYSROOT/usr/lib/arm-linux-gnueabihf/libdl.so
    sudo ln -s ../../../lib/arm-linux-gnueabihf/libdl.so.2 $SYSROOT/usr/lib/arm-linux-gnueabihf/libdl.so

The final step is compiling SDL itself.

    export CC="/opt/rpi-tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-gcc --sysroot=$SYSROOT -I$SYSROOT/opt/vc/include -I$SYSROOT/usr/include -I$SYSROOT/opt/vc/include/interface/vcos/pthreads -I$SYSROOT/opt/vc/include/interface/vmcs_host/linux"
    cd <SDL SOURCE>
    mkdir -p build;cd build
    LDFLAGS="-L$SYSROOT/opt/vc/lib" ../configure --with-sysroot=$SYSROOT --host=arm-raspberry-linux-gnueabihf --prefix=$PWD/rpi-sdl2-installed --disable-pulseaudio --disable-esd
    make
    make install

To be able to deploy this to /usr/local in the Raspbian system you need to fix up a few paths:
    
    perl -w -pi -e "s#$PWD/rpi-sdl2-installed#/usr/local#g;" ./rpi-sdl2-installed/lib/libSDL2.la ./rpi-sdl2-installed/lib/pkgconfig/sdl2.pc ./rpi-sdl2-installed/bin/sdl2-config
    
================================================================================
 Apps don't work or poor video/audio performance
================================================================================

If you get sound problems, buffer underruns, etc, run "sudo rpi-update" to 
update the RPi's firmware. Note that doing so will fix these problems, but it
will also render the CMA - Dynamic Memory Split functionality useless.

Also, by default the Raspbian distro configures the GPU RAM at 64MB, this is too
low in general, specially if a 1080p TV is hooked up.

See here how to configure this setting: http://elinux.org/RPiconfig

Using a fixed gpu_mem=128 is the best option (specially if you updated the 
firmware, using CMA probably won't work, at least it's the current case).

================================================================================
 No input
================================================================================

Make sure you belong to the "input" group.

    sudo usermod -aG input `whoami`

================================================================================
 No HDMI Audio
================================================================================

If you notice that ALSA works but there's no audio over HDMI, try adding:
    
    hdmi_drive=2
    
to your config.txt file and reboot.

Reference: http://www.raspberrypi.org/phpBB3/viewtopic.php?t=5062

================================================================================
 Text Input API support
================================================================================

The Text Input API is supported, with translation of scan codes done via the
kernel symbol tables. For this to work, SDL needs access to a valid console.
If you notice there's no SDL_TEXTINPUT message being emitted, double check that
your app has read access to one of the following:
    
* /proc/self/fd/0
* /dev/tty
* /dev/tty[0...6]
* /dev/vc/0
* /dev/console

This is usually not a problem if you run from the physical terminal (as opposed
to running from a pseudo terminal, such as via SSH). If running from a PTS, a 
quick workaround is to run your app as root or add yourself to the tty group,
then re-login to the system.

    sudo usermod -aG tty `whoami`
    
The keyboard layout used by SDL is the same as the one the kernel uses.
To configure the layout on Raspbian:
    
    sudo dpkg-reconfigure keyboard-configuration
    
To configure the locale, which controls which keys are interpreted as letters,
this determining the CAPS LOCK behavior:

    sudo dpkg-reconfigure locales

================================================================================
 OpenGL problems
================================================================================

If you have desktop OpenGL headers installed at build time in your RPi or cross 
compilation environment, support for it will be built in. However, the chipset 
does not actually have support for it, which causes issues in certain SDL apps 
since the presence of OpenGL support supersedes the ES/ES2 variants.
The workaround is to disable OpenGL at configuration time:

    ./configure --disable-video-opengl

Or if the application uses the Render functions, you can use the SDL_RENDER_DRIVER
environment variable:

    export SDL_RENDER_DRIVER=opengles2

================================================================================
 Notes
================================================================================

* When launching apps remotely (via SSH), SDL can prevent local keystrokes from
  leaking into the console only if it has root privileges. Launching apps locally
  does not suffer from this issue.
  

