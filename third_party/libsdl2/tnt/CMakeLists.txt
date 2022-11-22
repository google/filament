cmake_minimum_required(VERSION 3.19)
project(sdl2 C)

if (FILAMENT_SKIP_SDL2)
    return()
endif()

# This is necessary on Windows, so the sdl2.dll lives next to the sample apps that use it
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${FILAMENT_SAMPLES_BINARY_DIR})

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              sdl2)
set(SRC_DIR             ${OUR_DIR}/src)
set(PUBLIC_HDR_DIR      ${OUR_DIR}/include)


# General source files
file(GLOB SRCS
    ${SRC_DIR}/*.c
    ${SRC_DIR}/atomic/*.c
    ${SRC_DIR}/audio/*.c
    ${SRC_DIR}/audio/disk/*.c
    ${SRC_DIR}/audio/dsp/*.c
    ${SRC_DIR}/audio/dummy/*.c
    ${SRC_DIR}/cpuinfo/*.c
    ${SRC_DIR}/dynapi/*.c
    ${SRC_DIR}/events/*.c
    ${SRC_DIR}/file/*.c
    ${SRC_DIR}/joystick/*.c
    ${SRC_DIR}/joystick/steam/*.c
    ${SRC_DIR}/haptic/*.c
    ${SRC_DIR}/libm/*.c
    ${SRC_DIR}/power/*.c
    ${SRC_DIR}/render/*.c
    ${SRC_DIR}/render/*/*.c
    ${SRC_DIR}/stdlib/*.c
    ${SRC_DIR}/thread/*.c
    ${SRC_DIR}/timer/*.c
    ${SRC_DIR}/video/*.c
    ${SRC_DIR}/video/yuv2rgb/*.c
    ${SRC_DIR}/video/dummy/*.c
)

###################################################################################################
# Darwin specific files
if (APPLE)
    include_directories(${SRC_DIR}/video/khronos)
    file(GLOB SRCS_DARWIN
        ${SRC_DIR}/audio/coreaudio/*.c
        ${SRC_DIR}/file/cocoa/*.m
        ${SRC_DIR}/filesystem/cocoa/*.m
        ${SRC_DIR}/video/cocoa/*.m
        ${SRC_DIR}/joystick/darwin/*.c
        ${SRC_DIR}/haptic/darwin/*.c
        ${SRC_DIR}/power/macosx/*.c
        ${SRC_DIR}/timer/unix/*.c
        ${SRC_DIR}/thread/pthread/*.c
        ${SRC_DIR}/loadso/dlopen/*.c
        ${SRC_DIR}/render/metal/*.m
    )

    set(SRCS ${SRCS} ${SRCS_DARWIN})

    # Darwin frameworks
    find_library(COCOA_LIBRARY Cocoa)
    list(APPEND EXTRA_LIBS ${COCOA_LIBRARY})
    find_library(IOKIT IOKit)
    list(APPEND EXTRA_LIBS ${IOKIT})
    find_library(FORCEFEEDBACK ForceFeedback)
    list(APPEND EXTRA_LIBS ${FORCEFEEDBACK})
    find_library(CARBON_LIBRARY Carbon)
    list(APPEND EXTRA_LIBS ${CARBON_LIBRARY})
    find_library(COREAUDIO CoreAudio)
    list(APPEND EXTRA_LIBS ${COREAUDIO})
    find_library(AUDIOUNIT AudioUnit)
    list(APPEND EXTRA_LIBS ${AUDIOUNIT})

    find_library(CORE_VIDEO_LIBRARY CoreVideo)
    list(APPEND EXTRA_LIBS ${CORE_VIDEO_LIBRARY})

    find_library(METAL_LIBRARY Metal)
    list(APPEND EXTRA_LIBS ${METAL_LIBRARY})

endif()
###################################################################################################

###################################################################################################
# Linux specific files
if (LINUX)
    file(GLOB SRCS_LINUX
            ${SRC_DIR}/core/unix/*.c
            ${SRC_DIR}/thread/pthread/*.c
            ${SRC_DIR}/power/linux/*.c
            ${SRC_DIR}/filesystem/unix/*.c
            ${SRC_DIR}/timer/unix/*.c
            ${SRC_DIR}/joystick/linux/*.c
            ${SRC_DIR}/haptic/linux/*.c
            ${SRC_DIR}/loadso/dlopen/*.c
    )
    set(SRCS ${SRCS} ${SRCS_LINUX})

    if (FILAMENT_SUPPORTS_WAYLAND)
        file(GLOB SRCS_LINUX_WAYLAND ${SRC_DIR}/video/wayland/*.c)
        list(APPEND SRCS ${SRCS_LINUX_WAYLAND})
    elseif(FILAMENT_SUPPORTS_X11)
        file(GLOB SRCS_LINUX_X11 ${SRC_DIR}/video/x11/*.c)
        list(APPEND SRCS ${SRCS_LINUX_X11})
    endif()

    if (FILAMENT_SUPPORTS_WAYLAND)
        find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

        find_package(PkgConfig REQUIRED)
        pkg_check_modules(WAYLAND REQUIRED xkbcommon wayland-egl wayland-cursor wayland-client)
        pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols>=1.13)
        pkg_get_variable(WAYLAND_PROTOCOLS_BASE wayland-protocols pkgdatadir)
        pkg_get_variable(WAYLAND_CLIENT_PKGDATADIR wayland-client pkgdatadir)

        macro(wayland_generate protocol_file output_file)
            add_custom_command(OUTPUT "${output_file}.h"
                    COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header "${protocol_file}" "${output_file}.h"
                    DEPENDS "${protocol_file}"
                    VERBATIM)
            add_custom_command(OUTPUT "${output_file}.c"
                    COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${protocol_file}" "${output_file}.c"
                    DEPENDS "${protocol_file}"
                    VERBATIM)
            list(APPEND SRCS "${output_file}.h" "${output_file}.c")
        endmacro()

        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/autogen/wayland)
        include_directories(${CMAKE_BINARY_DIR}/autogen/wayland)

        list(APPEND EXTRA_LIBS ${WAYLAND_LIBRARIES})

        wayland_generate(
                "${WAYLAND_CLIENT_PKGDATADIR}/wayland.xml"
                "${CMAKE_BINARY_DIR}/autogen/wayland/wayland-client-protocol")
        wayland_generate(
                "${WAYLAND_PROTOCOLS_BASE}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml"
                "${CMAKE_BINARY_DIR}/autogen/wayland/pointer-constraints-unstable-v1-client-protocol")
        wayland_generate(
                "${WAYLAND_PROTOCOLS_BASE}/unstable/relative-pointer/relative-pointer-unstable-v1.xml"
                "${CMAKE_BINARY_DIR}/autogen/wayland/relative-pointer-unstable-v1-client-protocol")
        wayland_generate(
                "${WAYLAND_PROTOCOLS_BASE}/unstable/xdg-shell/xdg-shell-unstable-v6.xml"
                "${CMAKE_BINARY_DIR}/autogen/wayland/xdg-shell-unstable-v6-client-protocol")
    endif()
endif()
###################################################################################################

###################################################################################################
# Windows specific files
if (WIN32)
    file(GLOB SRCS_WIN32
            ${SRC_DIR}/core/windows/*.c
            ${SRC_DIR}/video/windows/*.c
            ${SRC_DIR}/thread/windows/*.c
            ${SRC_DIR}/thread/generic/SDL_syscond.c # Weird but this is how they do it on windows.
            ${SRC_DIR}/power/windows/*.c
            ${SRC_DIR}/filesystem/windows/*.c
            ${SRC_DIR}/timer/windows/*.c
            ${SRC_DIR}/joystick/windows/*.c
            ${SRC_DIR}/haptic/windows/*.c
            ${SRC_DIR}/loadso/windows/*.c
            ${SRC_DIR}/audio/winmm/*.c
            ${SRC_DIR}/audio/wasapi/*.c
    )
    set(SRCS ${SRCS} ${SRCS_WIN32})

    list(APPEND EXTRA_LIBS dinput8)
    list(APPEND EXTRA_LIBS dxguid)
    list(APPEND EXTRA_LIBS gdi32)
    list(APPEND EXTRA_LIBS imagehlp)
    list(APPEND EXTRA_LIBS imm32)
    list(APPEND EXTRA_LIBS ole32)
    list(APPEND EXTRA_LIBS oleaut32)
    list(APPEND EXTRA_LIBS shell32)
    list(APPEND EXTRA_LIBS user32)
    list(APPEND EXTRA_LIBS uuid)
    list(APPEND EXTRA_LIBS version)
    list(APPEND EXTRA_LIBS winmm)

endif()
###################################################################################################

# include directories...
include_directories(
    ${PUBLIC_HDR_DIR}
)

add_definitions(-D_THREAD_SAFE)

# we're building a library
add_library(${TARGET} STATIC ${SRCS})

# libraries we're linking against
target_link_libraries(${TARGET} ${EXTRA_LIBS})
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)

if(NOT MSVC)
    target_compile_options(${TARGET}
        PRIVATE -Wno-unused-variable
        PRIVATE -Wno-deprecated-declarations
        PRIVATE -Wno-switch
        PRIVATE -Wno-tautological-pointer-compare
        PRIVATE -Wno-parentheses-equality
    )
endif()

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

if (WIN32)
    # Windows needs a WinMain static library.
    add_library(${TARGET}main ${SRC_DIR}/main/windows/SDL_windows_main.c)
    if(NOT MSVC)
        # Making this option public because the warning occurs in public headers
        target_compile_options(${TARGET} PUBLIC -Wno-pragma-pack)
        target_compile_options(${TARGET} PRIVATE -Wno-unused-local-typedef)
        target_compile_options(${TARGET}main PRIVATE -Wno-pragma-pack)
    endif()

    # Set sdl2main's folder here
    set_target_properties(sdl2main PROPERTIES FOLDER ThirdParty)
else()
    target_compile_options(${TARGET} PRIVATE -fvisibility=hidden)

    # FIXME: the way SDL does atomics is insane, just disable all atomics because there are no
    #        implementation for aarch64 on certain targets. ideally, we would just std::atomic
    target_compile_definitions(${TARGET} PRIVATE SDL_ATOMIC_DISABLED=1)
endif()
