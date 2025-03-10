# This file caches variables which are platform specific.
if (WIN32)
    set(DAWN_USE_BUILT_DXC ON CACHE BOOL "")
endif ()
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "")
set(DAWN_ENABLE_INSTALL ON CACHE BOOL "")

if (CMAKE_SYTEM_NAME STREQUAL "Linux")
    # `sccache` seems effective only on linux.
    # for windows, we could look into `buildcache`
    # for macos, `sccache` causes an argument parse error for clang
    # similar to https://github.com/fastbuild/fastbuild/issues/1041
    # maybe we could use `ccache` in macos.
    set(CMAKE_C_COMPILER_LAUNCHER "sccache" CACHE STRING "")
    set(CMAKE_CXX_COMPILER_LAUNCHER "sccache" CACHE STRING "")
endif ()
