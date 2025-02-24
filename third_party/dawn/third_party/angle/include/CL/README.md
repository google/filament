# ANGLE OpenCL Headers

The OpenCL headers ANGLE uses are the original headers from Khronos.

### Updating headers

1. Clone [https://github.com/KhronosGroup/OpenCL-Headers.git](https://github.com/KhronosGroup/OpenCL-Headers.git).
1. Inspect the differences between all headers from `OpenCL-Headers/CL/` and this folder.
   * Changes of supported enums have to be updated in `src/common/packed_cl_enums.json`.
   * Changes of supported entry points have to be updated in `src/libGLESv2/cl_stubs.cpp`.
1. Copy all headers from `OpenCL-Headers/CL/` over to this folder.
