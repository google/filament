=======
drm-kms
=======

-------------------
Kernel Mode-Setting
-------------------

:Date: September 2012
:Manual section: 7
:Manual group: Direct Rendering Manager

Synopsis
========

``#include <xf86drm.h>``

``#include <xf86drmMode.h>``

Description
===========

Each DRM device provides access to manage which monitors and displays are
currently used and what frames to be displayed. This task is called *Kernel
Mode-Setting* (KMS). Historically, this was done in user-space and called
*User-space Mode-Setting* (UMS). Almost all open-source drivers now provide the
KMS kernel API to do this in the kernel, however, many non-open-source binary
drivers from different vendors still do not support this. You can use
**drmModeSettingSupported**\ (3) to check whether your driver supports this. To
understand how KMS works, we need to introduce 5 objects: *CRTCs*, *Planes*,
*Encoders*, *Connectors* and *Framebuffers*.

CRTCs
   A *CRTC* short for *CRT Controller* is an abstraction representing a part of
   the chip that contains a pointer to a scanout buffer.  Therefore, the number
   of CRTCs available determines how many independent scanout buffers can be
   active at any given time. The CRTC structure contains several fields to
   support this: a pointer to some video memory (abstracted as a frame-buffer
   object), a list of driven connectors, a display mode and an (x, y) offset
   into the video memory to support panning or configurations where one piece
   of video memory spans multiple CRTCs. A CRTC is the central point where
   configuration of displays happens. You select which objects to use, which
   modes and which parameters and then configure each CRTC via
   **drmModeCrtcSet**\ (3) to drive the display devices.

Planes
   A *plane* respresents an image source that can be blended with or overlayed
   on top of a CRTC during the scanout process. Planes are associated with a
   frame-buffer to crop a portion of the image memory (source) and optionally
   scale it to a destination size. The result is then blended with or overlayed
   on top of a CRTC. Planes are not provided by all hardware and the number of
   available planes is limited. If planes are not available or if not enough
   planes are available, the user should fall back to normal software blending
   (via GPU or CPU).

Encoders
   An *encoder* takes pixel data from a CRTC and converts it to a format
   suitable for any attached connectors. On some devices, it may be possible to
   have a CRTC send data to more than one encoder. In that case, both encoders
   would receive data from the same scanout buffer, resulting in a *cloned*
   display configuration across the connectors attached to each encoder.

Connectors
   A *connector* is the final destination of pixel-data on a device, and
   usually connects directly to an external display device like a monitor or
   laptop panel. A connector can only be attached to one encoder at a time. The
   connector is also the structure where information about the attached display
   is kept, so it contains fields for display data, *EDID* data, *DPMS* and
   *connection status*, and information about modes supported on the attached
   displays.

Framebuffers
   *Framebuffers* are abstract memory objects that provide a source of pixel
   data to scanout to a CRTC. Applications explicitly request the creation of
   framebuffers and can control their behavior. Framebuffers rely on the
   underneath memory manager for low-level memory operations. When creating a
   framebuffer, applications pass a memory handle through the API which is used
   as backing storage. The framebuffer itself is only an abstract object with
   no data. It just refers to memory buffers that must be created with the
   **drm-memory**\ (7) API.

Mode-Setting
------------

Before mode-setting can be performed, an application needs to call
**drmSetMaster**\ (3) to become *DRM-Master*. It then has exclusive access to
the KMS API. A call to **drmModeGetResources**\ (3) returns a list of *CRTCs*,
*Connectors*, *Encoders* and *Planes*.

Normal procedure now includes: First, you select which connectors you want to
use. Users are mostly interested in which monitor or display-panel is active so
you need to make sure to arrange them in the correct logical order and select
the correct ones to use. For each connector, you need to find a CRTC to drive
this connector. If you want to clone output to two or more connectors, you may
use a single CRTC for all cloned connectors (if the hardware supports this). To
find a suitable CRTC, you need to iterate over the list of encoders that are
available for each connector. Each encoder contains a list of CRTCs that it can
work with and you simply select one of these CRTCs. If you later program the
CRTC to control a connector, it automatically selects the best encoder.
However, this procedure is needed so your CRTC has at least one working encoder
for the selected connector. See the *Examples* section below for more
information.

All valid modes for a connector can be retrieved with a call to
**drmModeGetConnector**\ (3) You need to select the mode you want to use and save it.
The first mode in the list is the default mode with the highest resolution
possible and often a suitable choice.

After you have a working connector+CRTC+mode combination, you need to create a
framebuffer that is used for scanout. Memory buffer allocation is
driver-dependent and described in **drm-memory**\ (7). You need to create a
buffer big enough for your selected mode. Now you can create a framebuffer
object that uses your memory-buffer as scanout buffer. You can do this with
**drmModeAddFB**\ (3) and **drmModeAddFB2**\ (3).

As a last step, you want to program your CRTC to drive your selected connector.
You can do this with a call to **drmModeSetCrtc**\ (3).

Page-Flipping
-------------

A call to **drmModeSetCrtc**\ (3) is executed immediately and forces the CRTC
to use the new scanout buffer. If you want smooth-transitions without tearing,
you probably use double-buffering. You need to create one framebuffer object
for each buffer you use. You can then call **drmModeSetCrtc**\ (3) on the next
buffer to flip. If you want to synchronize your flips with *vertical-blanks*,
you can use **drmModePageFlip**\ (3) which schedules your page-flip for the
next *vblank*.

Planes
------

Planes are controlled independently from CRTCs. That is, a call to
**drmModeSetCrtc**\ (3) does not affect planes. Instead, you need to call
**drmModeSetPlane**\ (3) to configure a plane. This requires the plane ID, a
CRTC, a framebuffer and offsets into the plane-framebuffer and the
CRTC-framebuffer. The CRTC then blends the content from the plane over the CRTC
framebuffer buffer during scanout. As this does not involve any
software-blending, it is way faster than traditional blending. However, plane
resources are limited. See **drmModeGetPlaneResources**\ (3) for more
information.

Cursors
-------

Similar to planes, many hardware also supports cursors. A cursor is a very
small buffer with an image that is blended over the CRTC framebuffer. You can
set a different cursor for each CRTC with **drmModeSetCursor**\ (3) and move it
on the screen with **drmModeMoveCursor**\ (3).  This allows to move the cursor
on the screen without rerendering. If no hardware cursors are supported, you
need to rerender for each frame the cursor is moved.

Examples
========

Some examples of how basic mode-setting can be done. See the man-page of each
DRM function for more information.

CRTC/Encoder Selection
----------------------

If you retrieved all display configuration information via
**drmModeGetResources**\ (3) as ``drmModeRes *res``, selected a connector from
the list in ``res->connectors`` and retrieved the connector-information as
``drmModeConnector *conn`` via **drmModeGetConnector**\ (3) then this example
shows, how you can find a suitable CRTC id to drive this connector. This
function takes a file-descriptor to the DRM device (see **drmOpen**\ (3)) as
``fd``, a pointer to the retrieved resources as ``res`` and a pointer to the
selected connector as ``conn``. It returns an integer smaller than 0 on
failure, otherwise, a valid CRTC id is returned.

::

   static int modeset_find_crtc(int fd, drmModeRes *res, drmModeConnector *conn)
   {
       drmModeEncoder *enc;
       unsigned int i, j;

       /* iterate all encoders of this connector */
       for (i = 0; i < conn->count_encoders; ++i) {
           enc = drmModeGetEncoder(fd, conn->encoders[i]);
           if (!enc) {
               /* cannot retrieve encoder, ignoring... */
               continue;
           }

           /* iterate all global CRTCs */
           for (j = 0; j < res->count_crtcs; ++j) {
               /* check whether this CRTC works with the encoder */
               if (!(enc->possible_crtcs & (1 << j)))
                   continue;


               /* Here you need to check that no other connector
                * currently uses the CRTC with id "crtc". If you intend
                * to drive one connector only, then you can skip this
                * step. Otherwise, simply scan your list of configured
                * connectors and CRTCs whether this CRTC is already
                * used. If it is, then simply continue the search here. */
               if (res->crtcs[j] "is unused") {
                   drmModeFreeEncoder(enc);
                   return res->crtcs[j];
               }
           }

           drmModeFreeEncoder(enc);
       }

       /* cannot find a suitable CRTC */
       return -ENOENT;
   }

Reporting Bugs
==============

Bugs in this manual should be reported to
https://gitlab.freedesktop.org/mesa/drm/-/issues

See Also
========

**drm**\ (7), **drm-memory**\ (7), **drmModeGetResources**\ (3),
**drmModeGetConnector**\ (3), **drmModeGetEncoder**\ (3),
**drmModeGetCrtc**\ (3), **drmModeSetCrtc**\ (3), **drmModeGetFB**\ (3),
**drmModeAddFB**\ (3), **drmModeAddFB2**\ (3), **drmModeRmFB**\ (3),
**drmModePageFlip**\ (3), **drmModeGetPlaneResources**\ (3),
**drmModeGetPlane**\ (3), **drmModeSetPlane**\ (3), **drmModeSetCursor**\ (3),
**drmModeMoveCursor**\ (3), **drmSetMaster**\ (3), **drmAvailable**\ (3),
**drmCheckModesettingSupported**\ (3), **drmOpen**\ (3)
