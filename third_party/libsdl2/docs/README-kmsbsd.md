KMSDRM on *BSD
==================================================

KMSDRM is supported on FreeBSD and OpenBSD. DragonFlyBSD works but requires being a root user. NetBSD isn't supported yet because the application will crash when creating the KMSDRM screen.

WSCONS support has been brought back, but only as an input backend. It will not be brought back as a video backend to ease maintenance.

OpenBSD note: Note that the video backend assumes that the user has read/write permissions to the /dev/drm* devices.


SDL2 WSCONS input backend features
===================================================
1. It is keymap-aware; it will work properly with different keymaps.
2. It has mouse support.
3. Accent input is supported.
4. Compose keys are supported.
5. AltGr and Meta Shift keys work as intended.

Partially working or no input on OpenBSD/NetBSD.
==================================================

The WSCONS input backend needs read/write access to the /dev/wskbd* devices, without which it will not work properly. /dev/wsmouse must also be read/write accessible, otherwise mouse input will not work.

Partially working or no input on FreeBSD.
==================================================

The evdev devices are only accessible to the root user by default. Edit devfs rules to allow access to such devices. The /dev/kbd* devices are also only accessible to the root user by default. Edit devfs rules to allow access to such devices.
