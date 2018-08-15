cmake_minimum_required(VERSION 2.8.12)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Adhere to GNU filesystem layout conventions
include(GNUInstallDirs)

option(ENABLE_AMD_EXTENSIONS "Enables support of AMD-specific extensions" ON)
option(ENABLE_NV_EXTENSIONS "Enables support of Nvidia-specific extensions" ON)
option(ENABLE_OPT "Enables spirv-opt capability if present" OFFENABLE_OPT)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND WIN32)
    set(CMAKE_INSTALL_PREFIX "install" CACHE STRING "..." FORCE)
endif()

project(../glslang)
# make testing optional
include(CTest)

#if (LINUX)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
#endif()

if(ENABLE_AMD_EXTENSIONS)
    add_definitions(-DAMD_EXTENSIONS)
endif(ENABLE_AMD_EXTENSIONS)

if(ENABLE_NV_EXTENSIONS)
    add_definitions(-DNV_EXTENSIONS)
endif(ENABLE_NV_EXTENSIONS)

#if(ENABLE_HLSL)
#    add_definitions(-DENABLE_HLSL)
#endif(ENABLE_HLSL)

if(WIN32)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-DGLSLANG_OSINCLUDE_WIN32)
elseif(UNIX)
    add_definitions(-DGLSLANG_OSINCLUDE_UNIX)
else(WIN32)
    message("unknown platform")
endif(WIN32)

function(glslang_set_link_args TARGET)
    # For MinGW compiles, statically link against the GCC and C++ runtimes.
    # This avoids the need to ship those runtimes as DLLs.
    if (WIN32 AND ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
        set_target_properties(${TARGET} PROPERTIES
                              LINK_FLAGS "-static -static-libgcc -static-libstdc++")
    endif()
endfunction(glslang_set_link_args)

# We keep the optimizer disabled, we call it ourselves
# if(ENABLE_OPT)
#     message(STATUS "SPIR-V optimizer enabled")
#     add_definitions(-DENABLE_OPT)
# elseif(ENABLE_HLSL)
#     message(STATUS "spirv-tools not linked - illegal SPIRV may be generated for HLSL")
# endif()

add_subdirectory(../glslang/tnt "${CMAKE_CURRENT_BINARY_DIR}/glslang")
add_subdirectory(../OGLCompilersDLL/tnt "${CMAKE_CURRENT_BINARY_DIR}/OGLCompilersDLL")
add_subdirectory(../SPIRV/tnt "${CMAKE_CURRENT_BINARY_DIR}/SPIRV")
