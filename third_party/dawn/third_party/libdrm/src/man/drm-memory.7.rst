==========
drm-memory
==========

---------------------
DRM Memory Management
---------------------

:Date: September 2012
:Manual section: 7
:Manual group: Direct Rendering Manager

Synopsis
========

``#include <xf86drm.h>``

Description
===========

Many modern high-end GPUs come with their own memory managers. They even
include several different caches that need to be synchronized during access.
Textures, framebuffers, command buffers and more need to be stored in memory
that can be accessed quickly by the GPU. Therefore, memory management on GPUs
is highly driver- and hardware-dependent.

However, there are several frameworks in the kernel that are used by more than
one driver. These can be used for trivial mode-setting without requiring
driver-dependent code. But for hardware-accelerated rendering you need to read
the manual pages for the driver you want to work with.

Dumb-Buffers
------------

Almost all in-kernel DRM hardware drivers support an API called *Dumb-Buffers*.
This API allows to create buffers of arbitrary size that can be used for
scanout. These buffers can be memory mapped via **mmap**\ (2) so you can render
into them on the CPU. However, GPU access to these buffers is often not
possible. Therefore, they are fine for simple tasks but not suitable for
complex compositions and renderings.

The ``DRM_IOCTL_MODE_CREATE_DUMB`` ioctl can be used to create a dumb buffer.
The kernel will return a 32-bit handle that can be used to manage the buffer
with the DRM API. You can create framebuffers with **drmModeAddFB**\ (3) and
use it for mode-setting and scanout. To access the buffer, you first need to
retrieve the offset of the buffer. The ``DRM_IOCTL_MODE_MAP_DUMB`` ioctl
requests the DRM subsystem to prepare the buffer for memory-mapping and returns
a fake-offset that can be used with **mmap**\ (2).

The ``DRM_IOCTL_MODE_CREATE_DUMB`` ioctl takes as argument a structure of type
``struct drm_mode_create_dumb``:

::

   struct drm_mode_create_dumb {
       __u32 height;
       __u32 width;
       __u32 bpp;
       __u32 flags;

       __u32 handle;
       __u32 pitch;
       __u64 size;
   };

The fields *height*, *width*, *bpp* and *flags* have to be provided by the
caller. The other fields are filled by the kernel with the return values.
*height* and *width* are the dimensions of the rectangular buffer that is
created. *bpp* is the number of bits-per-pixel and must be a multiple of 8. You
most commonly want to pass 32 here. The flags field is currently unused and
must be zeroed. Different flags to modify the behavior may be added in the
future. After calling the ioctl, the handle, pitch and size fields are filled
by the kernel. *handle* is a 32-bit gem handle that identifies the buffer. This
is used by several other calls that take a gem-handle or memory-buffer as
argument. The *pitch* field is the pitch (or stride) of the new buffer. Most
drivers use 32-bit or 64-bit aligned stride-values. The size field contains the
absolute size in bytes of the buffer. This can normally also be computed with
``(height * pitch + width) * bpp / 4``.

To prepare the buffer for **mmap**\ (2) you need to use the
``DRM_IOCTL_MODE_MAP_DUMB`` ioctl. It takes as argument a structure of type
``struct drm_mode_map_dumb``:

::

   struct drm_mode_map_dumb {
       __u32 handle;
       __u32 pad;

       __u64 offset;
   };

You need to put the gem-handle that was previously retrieved via
``DRM_IOCTL_MODE_CREATE_DUMB`` into the *handle* field. The *pad* field is
unused padding and must be zeroed. After completion, the *offset* field will
contain an offset that can be used with **mmap**\ (2) on the DRM
file-descriptor.

If you don't need your dumb-buffer, anymore, you have to destroy it with
``DRM_IOCTL_MODE_DESTROY_DUMB``. If you close the DRM file-descriptor, all open
dumb-buffers are automatically destroyed. This ioctl takes as argument a
structure of type ``struct drm_mode_destroy_dumb``:

::

   struct drm_mode_destroy_dumb {
       __u32 handle;
   };

You only need to put your handle into the *handle* field. After this call, the
handle is invalid and may be reused for new buffers by the dumb-API.

TTM
---

*TTM* stands for *Translation Table Manager* and is a generic memory-manager
provided by the kernel. It does not provide a common user-space API so you need
to look at each driver interface if you want to use it. See for instance the
radeon man pages for more information on memory-management with radeon and TTM.

GEM
---

*GEM* stands for *Graphics Execution Manager* and is a generic DRM
memory-management framework in the kernel, that is used by many different
drivers. GEM is designed to manage graphics memory, control access to the
graphics device execution context and handle essentially NUMA environment
unique to modern graphics hardware. GEM allows multiple applications to share
graphics device resources without the need to constantly reload the entire
graphics card. Data may be shared between multiple applications with gem
ensuring that the correct memory synchronization occurs.

GEM provides simple mechanisms to manage graphics data and control execution
flow within the linux DRM subsystem. However, GEM is not a complete framework
that is fully driver independent. Instead, if provides many functions that are
shared between many drivers, but each driver has to implement most of
memory-management with driver-dependent ioctls. This manpage tries to describe
the semantics (and if it applies, the syntax) that is shared between all
drivers that use GEM.

All GEM APIs are defined as **ioctl**\ (2) on the DRM file descriptor. An
application must be authorized via **drmAuthMagic**\ (3) to the current
DRM-Master to access the GEM subsystem. A driver that does not support GEM will
return ``ENODEV`` for all these ioctls. Invalid object handles return
``EINVAL`` and invalid object names return ``ENOENT``.

Gem provides explicit memory management primitives. System pages are allocated
when the object is created, either as the fundamental storage for hardware
where system memory is used by the graphics processor directly, or as backing
store for graphics-processor resident memory.

Objects are referenced from user-space using handles. These are, for all
intents and purposes, equivalent to file descriptors but avoid the overhead.
Newer kernel drivers also support the **drm-prime** (7) infrastructure which
can return real file-descriptor for GEM-handles using the linux DMA-BUF API.
Objects may be published with a name so that other applications and processes
can access them. The name remains valid as long as the object exists.
GEM-objects are reference counted in the kernel. The object is only destroyed
when all handles from user-space were closed.

GEM-buffers cannot be created with a generic API. Each driver provides its own
API to create GEM-buffers. See for example ``DRM_I915_GEM_CREATE``,
``DRM_NOUVEAU_GEM_NEW`` or ``DRM_RADEON_GEM_CREATE``. Each of these ioctls
returns a GEM-handle that can be passed to different generic ioctls. The
*libgbm* library from the *mesa3D* distribution tries to provide a
driver-independent API to create GBM buffers and retrieve a GBM-handle to them.
It allows to create buffers for different use-cases including scanout,
rendering, cursors and CPU-access. See the libgbm library for more information
or look at the driver-dependent man-pages (for example **drm-intel**\ (7) or
**drm-radeon**\ (7)).

GEM-buffers can be closed with **drmCloseBufferHandle**\ (3). It takes as
argument the GEM-handle to be closed. After this call the GEM handle cannot be
used by this process anymore and may be reused for new GEM objects by the GEM
API.

If you want to share GEM-objects between different processes, you can create a
name for them and pass this name to other processes which can then open this
GEM-object. Names are currently 32-bit integer IDs and have no special
protection. That is, if you put a name on your GEM-object, every other client
that has access to the DRM device and is authenticated via
**drmAuthMagic**\ (3) to the current DRM-Master, can *guess* the name and open
or access the GEM-object. If you want more fine-grained access control, you can
use the new **drm-prime**\ (7) API to retrieve file-descriptors for
GEM-handles. To create a name for a GEM-handle, you use the
``DRM_IOCTL_GEM_FLINK`` ioctl. It takes as argument a structure of type
``struct drm_gem_flink``:

::

   struct drm_gem_flink {
       __u32 handle;
       __u32 name;
   };

You have to put your handle into the *handle* field. After completion, the
kernel has put the new unique name into the name field. You can now pass
this name to other processes which can then import the name with the
``DRM_IOCTL_GEM_OPEN`` ioctl. It takes as argument a structure of type
``struct drm_gem_open``:

::

   struct drm_gem_open {
       __u32 name;

       __u32 handle;
       __u32 size;
   };

You have to fill in the *name* field with the name of the GEM-object that you
want to open. The kernel will fill in the *handle* and *size* fields with the
new handle and size of the GEM-object. You can now access the GEM-object via
the handle as if you created it with the GEM API.

Besides generic buffer management, the GEM API does not provide any generic
access. Each driver implements its own functionality on top of this API. This
includes execution-buffers, GTT management, context creation, CPU access, GPU
I/O and more. The next higher-level API is *OpenGL*. So if you want to use more
GPU features, you should use the *mesa3D* library to create OpenGL contexts on
DRM devices. This does *not* require any windowing-system like X11, but can
also be done on raw DRM devices. However, this is beyond the scope of this
man-page. You may have a look at other mesa3D man pages, including libgbm and
libEGL. 2D software-rendering (rendering with the CPU) can be achieved with the
dumb-buffer-API in a driver-independent fashion, however, for
hardware-accelerated 2D or 3D rendering you must use OpenGL. Any other API that
tries to abstract the driver-internals to access GEM-execution-buffers and
other GPU internals, would simply reinvent OpenGL so it is not provided. But if
you need more detailed information for a specific driver, you may have a look
into the driver-manpages, including **drm-intel**\ (7), **drm-radeon**\ (7) and
**drm-nouveau**\ (7). However, the **drm-prime**\ (7) infrastructure and the
generic GEM API as described here allow display-managers to handle
graphics-buffers and render-clients without any deeper knowledge of the GPU
that is used. Moreover, it allows to move objects between GPUs and implement
complex display-servers that don't do any rendering on their own. See its
man-page for more information.

Examples
========

This section includes examples for basic memory-management tasks.

Dumb-Buffers
------------

This examples shows how to create a dumb-buffer via the generic DRM API.
This is driver-independent (as long as the driver supports dumb-buffers)
and provides memory-mapped buffers that can be used for scanout. This
example creates a full-HD 1920x1080 buffer with 32 bits-per-pixel and a
color-depth of 24 bits. The buffer is then bound to a framebuffer which
can be used for scanout with the KMS API (see **drm-kms**\ (7)).

::

   struct drm_mode_create_dumb creq;
   struct drm_mode_destroy_dumb dreq;
   struct drm_mode_map_dumb mreq;
   uint32_t fb;
   int ret;
   void *map;

   /* create dumb buffer */
   memset(&creq, 0, sizeof(creq));
   creq.width = 1920;
   creq.height = 1080;
   creq.bpp = 32;
   ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
   if (ret < 0) {
       /* buffer creation failed; see "errno" for more error codes */
       ...
   }
   /* creq.pitch, creq.handle and creq.size are filled by this ioctl with
    * the requested values and can be used now. */

   /* create framebuffer object for the dumb-buffer */
   ret = drmModeAddFB(fd, 1920, 1080, 24, 32, creq.pitch, creq.handle, &fb);
   if (ret) {
       /* frame buffer creation failed; see "errno" */
       ...
   }
   /* the framebuffer "fb" can now used for scanout with KMS */

   /* prepare buffer for memory mapping */
   memset(&mreq, 0, sizeof(mreq));
   mreq.handle = creq.handle;
   ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
   if (ret) {
       /* DRM buffer preparation failed; see "errno" */
       ...
   }
   /* mreq.offset now contains the new offset that can be used with mmap() */

   /* perform actual memory mapping */
   map = mmap(0, creq.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);
   if (map == MAP_FAILED) {
       /* memory-mapping failed; see "errno" */
       ...
   }

   /* clear the framebuffer to 0 */
   memset(map, 0, creq.size);

Reporting Bugs
==============

Bugs in this manual should be reported to
https://gitlab.freedesktop.org/mesa/drm/-/issues

See Also
========

**drm**\ (7), **drm-kms**\ (7), **drm-prime**\ (7), **drmAvailable**\ (3),
**drmOpen**\ (3), **drm-intel**\ (7), **drm-radeon**\ (7), **drm-nouveau**\ (7)
