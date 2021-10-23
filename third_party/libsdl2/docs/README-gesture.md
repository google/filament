Dollar Gestures
===========================================================================
SDL provides an implementation of the $1 gesture recognition system. This allows for recording, saving, loading, and performing single stroke gestures.

Gestures can be performed with any number of fingers (the centroid of the fingers must follow the path of the gesture), but the number of fingers must be constant (a finger cannot go down in the middle of a gesture). The path of a gesture is considered the path from the time when the final finger went down, to the first time any finger comes up. 

Dollar gestures are assigned an Id based on a hash function. This is guaranteed to remain constant for a given gesture. There is a (small) chance that two different gestures will be assigned the same ID. In this case, simply re-recording one of the gestures should result in a different ID.

Recording:
----------
To begin recording on a touch device call:
SDL_RecordGesture(SDL_TouchID touchId), where touchId is the id of the touch device you wish to record on, or -1 to record on all connected devices.

Recording terminates as soon as a finger comes up. Recording is acknowledged by an SDL_DOLLARRECORD event.
A SDL_DOLLARRECORD event is a dgesture with the following fields:

* event.dgesture.touchId   - the Id of the touch used to record the gesture.
* event.dgesture.gestureId - the unique id of the recorded gesture.


Performing:
-----------
As long as there is a dollar gesture assigned to a touch, every finger-up event will also cause an SDL_DOLLARGESTURE event with the following fields:

* event.dgesture.touchId    - the Id of the touch which performed the gesture.
* event.dgesture.gestureId  - the unique id of the closest gesture to the performed stroke.
* event.dgesture.error      - the difference between the gesture template and the actual performed gesture. Lower error is a better match.
* event.dgesture.numFingers - the number of fingers used to draw the stroke.

Most programs will want to define an appropriate error threshold and check to be sure that the error of a gesture is not abnormally high (an indicator that no gesture was performed).



Saving:
-------
To save a template, call SDL_SaveDollarTemplate(gestureId, dst) where gestureId is the id of the gesture you want to save, and dst is an SDL_RWops pointer to the file where the gesture will be stored.

To save all currently loaded templates, call SDL_SaveAllDollarTemplates(dst) where dst is an SDL_RWops pointer to the file where the gesture will be stored.

Both functions return the number of gestures successfully saved.


Loading:
--------
To load templates from a file, call SDL_LoadDollarTemplates(touchId,src) where touchId is the id of the touch to load to (or -1 to load to all touch devices), and src is an SDL_RWops pointer to a gesture save file. 

SDL_LoadDollarTemplates returns the number of templates successfully loaded.



===========================================================================
Multi Gestures
===========================================================================
SDL provides simple support for pinch/rotate/swipe gestures. 
Every time a finger is moved an SDL_MULTIGESTURE event is sent with the following fields:

* event.mgesture.touchId - the Id of the touch on which the gesture was performed.
* event.mgesture.x       - the normalized x coordinate of the gesture. (0..1)
* event.mgesture.y       - the normalized y coordinate of the gesture. (0..1)
* event.mgesture.dTheta  - the amount that the fingers rotated during this motion.
* event.mgesture.dDist   - the amount that the fingers pinched during this motion.
* event.mgesture.numFingers - the number of fingers used in the gesture.


===========================================================================
Notes
===========================================================================
For a complete example see test/testgesture.c

Please direct questions/comments to:
   jim.tla+sdl_touch@gmail.com
