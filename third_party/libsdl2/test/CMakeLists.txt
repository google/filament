cmake_minimum_required(VERSION 3.0.0)
project(SDL2 C)

# Global settings for all of the test targets
# FIXME: is this wrong?
remove_definitions(-DUSING_GENERATED_CONFIG_H)
link_libraries(SDL2_test SDL2-static)

# FIXME: Parent directory CMakeLists.txt only sets these for mingw/cygwin,
# but we need them for VS as well.
if(WINDOWS)
    link_libraries(SDL2main)
    add_definitions(-Dmain=SDL_main)
endif()

find_package(OpenGL)

if (OPENGL_FOUND)
add_definitions(-DHAVE_OPENGL)
endif()

add_executable(checkkeys checkkeys.c)
add_executable(checkkeysthreads checkkeysthreads.c)
add_executable(loopwave loopwave.c)
add_executable(loopwavequeue loopwavequeue.c)
add_executable(testresample testresample.c)
add_executable(testaudioinfo testaudioinfo.c)

file(GLOB TESTAUTOMATION_SOURCE_FILES testautomation*.c)
add_executable(testautomation ${TESTAUTOMATION_SOURCE_FILES})

add_executable(testmultiaudio testmultiaudio.c)
add_executable(testaudiohotplug testaudiohotplug.c)
add_executable(testaudiocapture testaudiocapture.c)
add_executable(testatomic testatomic.c)
add_executable(testintersections testintersections.c)
add_executable(testrelative testrelative.c)
add_executable(testhittesting testhittesting.c)
add_executable(testdraw2 testdraw2.c)
add_executable(testdrawchessboard testdrawchessboard.c)
add_executable(testdropfile testdropfile.c)
add_executable(testerror testerror.c)
add_executable(testfile testfile.c)
add_executable(testgamecontroller testgamecontroller.c)
add_executable(testgesture testgesture.c)
add_executable(testgl2 testgl2.c)
add_executable(testgles testgles.c)
add_executable(testgles2 testgles2.c)
add_executable(testhaptic testhaptic.c)
add_executable(testhotplug testhotplug.c)
add_executable(testrumble testrumble.c)
add_executable(testthread testthread.c)
add_executable(testiconv testiconv.c)
add_executable(testime testime.c)
add_executable(testjoystick testjoystick.c)
add_executable(testkeys testkeys.c)
add_executable(testloadso testloadso.c)
add_executable(testlock testlock.c)

if(APPLE)
    add_executable(testnative testnative.c
                              testnativecocoa.m
                              testnativex11.c)
elseif(WINDOWS)
    add_executable(testnative testnative.c testnativew32.c)
elseif(UNIX)
    add_executable(testnative testnative.c testnativex11.c)
endif()

add_executable(testoverlay2 testoverlay2.c testyuv_cvt.c)
add_executable(testplatform testplatform.c)
add_executable(testpower testpower.c)
add_executable(testfilesystem testfilesystem.c)
add_executable(testrendertarget testrendertarget.c)
add_executable(testscale testscale.c)
add_executable(testsem testsem.c)
add_executable(testshader testshader.c)
add_executable(testshape testshape.c)
add_executable(testsprite2 testsprite2.c)
add_executable(testspriteminimal testspriteminimal.c)
add_executable(teststreaming teststreaming.c)
add_executable(testtimer testtimer.c)
add_executable(testver testver.c)
add_executable(testviewport testviewport.c)
add_executable(testwm2 testwm2.c)
add_executable(testyuv testyuv.c testyuv_cvt.c)
add_executable(torturethread torturethread.c)
add_executable(testrendercopyex testrendercopyex.c)
add_executable(testmessage testmessage.c)
add_executable(testdisplayinfo testdisplayinfo.c)
add_executable(testqsort testqsort.c)
add_executable(testbounds testbounds.c)
add_executable(testcustomcursor testcustomcursor.c)
add_executable(controllermap controllermap.c)
add_executable(testvulkan testvulkan.c)
add_executable(testoffscreen testoffscreen.c)

if(OPENGL_FOUND)
add_dependencies(testshader OpenGL::GL)
add_dependencies(testgl2 OpenGL::GL)
target_link_libraries(testshader OpenGL::GL)
target_link_libraries(testgl2 OpenGL::GL)
endif()

# HACK: Dummy target to cause the resource files to be copied to the build directory.
# Need to make it an executable so we can use the TARGET_FILE_DIR generator expression.
# This is needed so they get copied to the correct Debug/Release subdirectory in Xcode.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/resources_dummy.c "int main(int argc, const char **argv){ return 1; }\n")
add_executable(SDL2_test_resources ${CMAKE_CURRENT_BINARY_DIR}/resources_dummy.c)

file(GLOB RESOURCE_FILES *.bmp *.wav *.hex moose.dat utf8.txt)
foreach(RESOURCE_FILE ${RESOURCE_FILES})
    add_custom_command(TARGET SDL2_test_resources POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${RESOURCE_FILE} $<TARGET_FILE_DIR:SDL2_test_resources>)
endforeach(RESOURCE_FILE)

file(COPY ${RESOURCE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# TODO: Might be easier to make all targets depend on the resources...?

set(NEEDS_RESOURCES
    testscale
    testrendercopyex
    controllermap
    testyuv
    testgamecontroller
    testshape
    testshader
    testnative
    testspriteminimal
    testautomation
    testcustomcursor
    testrendertarget
    testsprite2
    loopwave
    loopwavequeue
    testresample
    testaudiohotplug
    testmultiaudio
)
foreach(APP IN LISTS NEEDS_RESOURCES)
    add_dependencies(${APP} SDL2_test_resources)
    if(APPLE)
        # Make sure resource files get installed into macOS/iOS .app bundles.  
        target_sources(${APP} PRIVATE "${RESOURCE_FILES}")
        set_target_properties(${APP} PROPERTIES RESOURCE "${RESOURCE_FILES}")
    endif()
endforeach()

# Set Apple App ID / Bundle ID.  This is needed to launch apps on some Apple
# platforms (iOS, for example).
if(APPLE)
    if(${CMAKE_VERSION} VERSION_LESS "3.7.0")
        # CMake's 'BUILDSYSTEM_TARGETS' property is only available in
        # CMake 3.7 and above.
        message(WARNING "Unable to set Bundle ID for Apple .app builds due to old CMake (pre 3.7).")
    else()
        get_property(TARGETS DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY BUILDSYSTEM_TARGETS)
        foreach(CURRENT_TARGET IN LISTS TARGETS)
            get_property(TARGET_TYPE TARGET ${CURRENT_TARGET} PROPERTY TYPE)
            if(TARGET_TYPE STREQUAL "EXECUTABLE")
                set_target_properties("${CURRENT_TARGET}" PROPERTIES
                    MACOSX_BUNDLE_GUI_IDENTIFIER "org.libsdl.${CURRENT_TARGET}"
                    MACOSX_BUNDLE_BUNDLE_VERSION "${SDL_VERSION}"
                    MACOSX_BUNDLE_SHORT_VERSION_STRING "${SDL_VERSION}"
                )
            endif()
        endforeach()
    endif()
endif()
