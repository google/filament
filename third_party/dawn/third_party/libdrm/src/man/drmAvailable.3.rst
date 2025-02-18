============
drmAvailable
============

-----------------------------------------------------
determine whether a DRM kernel driver has been loaded
-----------------------------------------------------

:Date: September 2012
:Manual section: 3
:Manual group: Direct Rendering Manager

Synopsis
========

``#include <xf86drm.h>``

``int drmAvailable(void);``

Description
===========

``drmAvailable`` allows the caller to determine whether a kernel DRM
driver is loaded.

Return Value
============

``drmAvailable`` returns 1 if a DRM driver is currently loaded.
Otherwise 0 is returned.

Reporting Bugs
==============

Bugs in this function should be reported to
https://gitlab.freedesktop.org/mesa/drm/-/issues

See Also
========

**drm**\ (7), **drmOpen**\ (3)
