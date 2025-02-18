===
drm
===

------------------------
Direct Rendering Manager
------------------------

:Date: September 2012
:Manual section: 7
:Manual group: Direct Rendering Manager

Synopsis
========

``#include <xf86drm.h>``

Description
===========

The *Direct Rendering Manager* (DRM) is a framework to manage *Graphics
Processing Units* (GPUs). It is designed to support the needs of complex
graphics devices, usually containing programmable pipelines well suited
to 3D graphics acceleration. Furthermore, it is responsible for memory
management, interrupt handling and DMA to provide a uniform interface to
applications.

In earlier days, the kernel framework was solely used to provide raw
hardware access to privileged user-space processes which implement all
the hardware abstraction layers. But more and more tasks were moved into
the kernel. All these interfaces are based on **ioctl**\ (2) commands on
the DRM character device. The *libdrm* library provides wrappers for these
system-calls and many helpers to simplify the API.

When a GPU is detected, the DRM system loads a driver for the detected
hardware type. Each connected GPU is then presented to user-space via a
character-device that is usually available as ``/dev/dri/card0`` and can
be accessed with **open**\ (2) and **close**\ (2). However, it still
depends on the graphics driver which interfaces are available on these
devices. If an interface is not available, the syscalls will fail with
``EINVAL``.

Authentication
--------------

All DRM devices provide authentication mechanisms. Only a DRM master is
allowed to perform mode-setting or modify core state and only one user
can be DRM master at a time. See **drmSetMaster**\ (3) for information
on how to become DRM master and what the limitations are. Other DRM users
can be authenticated to the DRM-Master via **drmAuthMagic**\ (3) so they
can perform buffer allocations and rendering.

Mode-Setting
------------

Managing connected monitors and displays and changing the current modes
is called *Mode-Setting*. This is restricted to the current DRM master.
Historically, this was implemented in user-space, but new DRM drivers
implement a kernel interface to perform mode-setting called *Kernel Mode
Setting* (KMS). If your hardware-driver supports it, you can use the KMS
API provided by DRM. This includes allocating framebuffers, selecting
modes and managing CRTCs and encoders. See **drm-kms**\ (7) for more.

Memory Management
-----------------

The most sophisticated tasks for GPUs today is managing memory objects.
Textures, framebuffers, command-buffers and all other kinds of commands
for the GPU have to be stored in memory. The DRM driver takes care of
managing all memory objects, flushing caches, synchronizing access and
providing CPU access to GPU memory. All memory management is hardware
driver dependent. However, two generic frameworks are available that are
used by most DRM drivers. These are the *Translation Table Manager*
(TTM) and the *Graphics Execution Manager* (GEM). They provide generic
APIs to create, destroy and access buffers from user-space. However,
there are still many differences between the drivers so driver-dependent
code is still needed. Many helpers are provided in *libgbm* (Graphics
Buffer Manager) from the *Mesa* project. For more information on DRM
memory management, see **drm-memory**\ (7).

Reporting Bugs
==============

Bugs in this manual should be reported to
https://gitlab.freedesktop.org/mesa/drm/-/issues.

See Also
========

**drm-kms**\ (7), **drm-memory**\ (7), **drmSetMaster**\ (3),
**drmAuthMagic**\ (3), **drmAvailable**\ (3), **drmOpen**\ (3)
