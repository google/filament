==============
drmHandleEvent
==============

-----------------------------------
read and process pending DRM events
-----------------------------------

:Date: September 2012
:Manual section: 3
:Manual group: Direct Rendering Manager

Synopsis
========

``#include <xf86drm.h>``

``int drmHandleEvent(int fd, drmEventContextPtr evctx);``

Description
===========

``drmHandleEvent`` processes outstanding DRM events on the DRM
file-descriptor passed as ``fd``. This function should be called after
the DRM file-descriptor has polled readable; it will read the events and
use the passed-in ``evctx`` structure to call function pointers with the
parameters noted below:

::

   typedef struct _drmEventContext {
       int version;
       void (*vblank_handler) (int fd,
                               unsigned int sequence,
                               unsigned int tv_sec,
                               unsigned int tv_usec,
                               void *user_data)
       void (*page_flip_handler) (int fd,
                                  unsigned int sequence,
                                  unsigned int tv_sec,
                                  unsigned int tv_usec,
                                  void *user_data)
   } drmEventContext, *drmEventContextPtr;

Return Value
============

``drmHandleEvent`` returns 0 on success, or if there is no data to
read from the file-descriptor. Returns -1 if the read on the
file-descriptor fails or returns less than a full event record.

Reporting Bugs
==============

Bugs in this function should be reported to
https://gitlab.freedesktop.org/mesa/drm/-/issues

See Also
========

**drm**\ (7), **drm-kms**\ (7), **drmModePageFlip**\ (3),
**drmWaitVBlank**\ (3)
