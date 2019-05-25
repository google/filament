Version History
---------------

### Changes in v0.9.0:

-   Reduced memory consumption by about 38%
-   Added support for progress monitor callback functions
-   Enabled fully concurrent execution when using multiple devices
-   Clamp LDR input and output colors to 1
-   Fixed issue where some memory allocation errors were not reported

### Changes in v0.8.2:

-   Fixed wrong HDR output when the input contains infinities/NaNs
-   Fixed wrong output when multiple filters were executed concurrently on
    separate devices with AVX-512 support. Currently the filter executions are
    serialized as a temporary workaround, and a full fix will be included in a
    future release.
-   Added OIDN_STATIC_LIB CMake option for building as a static library
    (requires CMake 3.13.0 or later)
-   Fixed CMake error when adding the library with add_subdirectory() to a project

### Changes in v0.8.1:

-   Fixed wrong path to TBB in the generated CMake configs
-   Fixed wrong rpath in the binaries
-   Fixed compile error on some macOS systems
-   Fixed minor compile issues with Visual Studio
-   Lowered the CPU requirement to SSE4.1
-   Minor example update

### Changes in v0.8.0:

-   Initial beta release
