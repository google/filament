Touch
===========================================================================
System Specific Notes
===========================================================================
Linux:
The linux touch system is currently based off event streams, and proc/bus/devices. The active user must be given permissions to read /dev/input/TOUCHDEVICE, where TOUCHDEVICE is the event stream for your device. Currently only Wacom tablets are supported. If you have an unsupported tablet contact me at jim.tla+sdl_touch@gmail.com and I will help you get support for it.

Mac:
The Mac and iPhone APIs are pretty. If your touch device supports them then you'll be fine. If it doesn't, then there isn't much we can do.

iPhone: 
Works out of box.

Windows:
Unfortunately there is no windows support as of yet. Support for Windows 7 is planned, but we currently have no way to test. If you have a Windows 7 WM_TOUCH supported device, and are willing to help test please contact me at jim.tla+sdl_touch@gmail.com

===========================================================================
Events
===========================================================================
SDL_FINGERDOWN:
Sent when a finger (or stylus) is placed on a touch device.
Fields:
* event.tfinger.touchId  - the Id of the touch device.
* event.tfinger.fingerId - the Id of the finger which just went down.
* event.tfinger.x        - the x coordinate of the touch (0..1)
* event.tfinger.y        - the y coordinate of the touch (0..1)
* event.tfinger.pressure - the pressure of the touch (0..1)

SDL_FINGERMOTION:
Sent when a finger (or stylus) is moved on the touch device.
Fields:
Same as SDL_FINGERDOWN but with additional:
* event.tfinger.dx       - change in x coordinate during this motion event.
* event.tfinger.dy       - change in y coordinate during this motion event.

SDL_FINGERUP:
Sent when a finger (or stylus) is lifted from the touch device.
Fields:
Same as SDL_FINGERDOWN.


===========================================================================
Functions
===========================================================================
SDL provides the ability to access the underlying SDL_Finger structures.
These structures should _never_ be modified.

The following functions are included from SDL_touch.h

To get a SDL_TouchID call SDL_GetTouchDevice(int index).
This returns a SDL_TouchID.
IMPORTANT: If the touch has been removed, or there is no touch with the given index, SDL_GetTouchDevice() will return 0. Be sure to check for this!

The number of touch devices can be queried with SDL_GetNumTouchDevices().

A SDL_TouchID may be used to get pointers to SDL_Finger.

SDL_GetNumTouchFingers(touchID) may be used to get the number of fingers currently down on the device.

The most common reason to access SDL_Finger is to query the fingers outside the event. In most cases accessing the fingers is using the event. This would be accomplished by code like the following:

      float x = event.tfinger.x;
      float y = event.tfinger.y;



To get a SDL_Finger, call SDL_GetTouchFinger(SDL_TouchID touchID, int index), where touchID is a SDL_TouchID, and index is the requested finger.
This returns a SDL_Finger *, or NULL if the finger does not exist, or has been removed.
A SDL_Finger is guaranteed to be persistent for the duration of a touch, but it will be de-allocated as soon as the finger is removed. This occurs when the SDL_FINGERUP event is _added_ to the event queue, and thus _before_ the SDL_FINGERUP event is polled.
As a result, be very careful to check for NULL return values.

A SDL_Finger has the following fields:
* x, y:
	The current coordinates of the touch.
* pressure:
	The pressure of the touch.


===========================================================================
Notes
===========================================================================
For a complete example see test/testgesture.c

Please direct questions/comments to:
   jim.tla+sdl_touch@gmail.com
   (original author, API was changed since)
