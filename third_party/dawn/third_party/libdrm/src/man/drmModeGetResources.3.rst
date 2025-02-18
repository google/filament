===================
drmModeGetResources
===================

--------------------------------------------------
retrieve current display configuration information
--------------------------------------------------

:Date: September 2012
:Manual section: 3
:Manual group: Direct Rendering Manager

Synopsis
========

``#include <xf86drm.h>``

``#include <xf86drmMode.h>``

``drmModeResPtr drmModeGetResources(int fd);``

Description
===========

``drmModeGetResources`` allocates, populates, and returns a drmModeRes
structure containing information about the current display
configuration. The structure contains the following fields:

::

   typedef struct _drmModeRes {
       int count_fbs;
       uint32_t *fbs;

       int count_crtcs;
       uint32_t *crtcs;

       int count_connectors;
       uint32_t *connectors;

       int count_encoders;
       uint32_t *encoders;

       uint32_t min_width, max_width;
       uint32_t min_height, max_height;
   } drmModeRes, *drmModeResPtr;

The *count_fbs* and *fbs* fields indicate the number of currently allocated
framebuffer objects (i.e., objects that can be attached to a given CRTC
or sprite for display).

The *count_crtcs* and *crtcs* fields list the available CRTCs in the
configuration. A CRTC is simply an object that can scan out a
framebuffer to a display sink, and contains mode timing and relative
position information. CRTCs drive encoders, which are responsible for
converting the pixel stream into a specific display protocol (e.g., MIPI
or HDMI).

The *count_connectors* and *connectors* fields list the available physical
connectors on the system. Note that some of these may not be exposed
from the chassis (e.g., LVDS or eDP). Connectors are attached to
encoders and contain information about the attached display sink (e.g.,
width and height in mm, subpixel ordering, and various other
properties).

The *count_encoders* and *encoders* fields list the available encoders on
the device. Each encoder may be associated with a CRTC, and may be used
to drive a particular encoder.

The *min_\** and *max_\** fields indicate the maximum size of a framebuffer
for this device (i.e., the scanout size limit).

Return Value
============

``drmModeGetResources`` returns a drmModeRes structure pointer on
success, NULL on failure. The returned structure must be freed with
**drmModeFreeResources**\ (3).

Reporting Bugs
==============

Bugs in this function should be reported to
https://gitlab.freedesktop.org/mesa/drm/-/issues

See Also
========

**drm**\ (7), **drm-kms**\ (7), **drmModeGetFB**\ (3), **drmModeAddFB**\ (3),
**drmModeAddFB2**\ (3), **drmModeRmFB**\ (3), **drmModeDirtyFB**\ (3),
**drmModeGetCrtc**\ (3), **drmModeSetCrtc** (3), **drmModeGetEncoder** (3),
**drmModeGetConnector**\ (3)
