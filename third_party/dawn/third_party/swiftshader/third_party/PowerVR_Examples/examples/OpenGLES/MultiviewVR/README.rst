===========
MultiviewVR
===========

.. figure:: ./MultiviewVR.png

Render a scene stereoscopically intended for VR hardware using the ``GL_OVR_multiview`` extension.

API
---
* OpenGL ES 3.0+

Description
-----------
This example introduces the ``GL_OVR_multiview extension`` and shows how to use it to render the scene from two different eye locations.

Two sets of FBOs are being used, one low resolution and one high resolution. The intention is to render the centre of the screen in high resolution, and the edges of the screen (which will be distorted by the VR lenses) at a lower resolution. The end result is a split screen suitable for VR.

Controls
--------
- Quit- Close the application
