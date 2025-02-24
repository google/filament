# Introduction

This page describes how to add new extensions to ANGLE.

# Adding EGL extensions

Note: see also [anglebug.com/42261334](http://anglebug.com/42261334), linked
from the [starter project](Starter-Projects.md) doc, to simplify some
of these steps.

For extensions requiring new entry points:

* Add the extension xml to
  [scripts/egl_angle_ext.xml](../scripts/egl_angle_ext.xml) .

* Note the prototypes for the new entry points must be added to the
  top of the file, and the functions themselves grouped under the
  extension name to the bottom of the file.

* Modify [scripts/registry_xml.py](../scripts/registry_xml.py) to add
  the new extension as needed.

* Run
  [scripts/run_code_generation.py](../scripts/run_code_generation.py)
  .

* The entry point itself goes in
  [entry_points_egl_ext.h](../src/libGLESv2/entry_points_egl_ext.h)
  and
  [entry_points_egl_ext.cpp](../src/libGLESv2/entry_points_egl_ext.cpp)
  .

* Add the new function to [libEGL.cpp](../src/libEGL/libEGL.cpp) and
  [libEGL.def](../src/libEGL/libEGL.def) .

* Update [eglext_angle.h](../include/EGL/eglext_angle.h) with the new
  entry points and/or enums.

* Add members to the appropriate Extensions struct in
  [Caps.h](../src/libANGLE/Caps.h) and
  [Caps.cpp](../src/libANGLE/Caps.cpp) .

* Initialize extension availability in the `Display` subclass's
  `generateExtensions` method for displays that can support the
  extension; for example,
  [DisplayCGL](../src/libANGLE/renderer/gl/cgl/DisplayCGL.mm).
