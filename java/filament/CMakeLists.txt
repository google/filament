cmake_minimum_required(VERSION 3.10)
project(filament-java)

if (NOT ENABLE_JAVA)
    return()
endif()

find_package(Java)
if (NOT Java_FOUND)
    message(WARNING "JDK not found, skipping Java projects")
    return()
endif()

find_package(JNI)
if (NOT JNI_FOUND)
    message(WARNING "JNI not found, skipping Java projects")
    return()
endif()

if (NOT DEFINED ENV{JAVA_HOME})
    message(WARNING "The JAVA_HOME environment variable must be set to compile Java projects")
    message(WARNING "Skipping Java projects")
    return()
endif()

# ==================================================================================================
# JNI bindings
# ==================================================================================================
set(TARGET filament-jni)

set(CXX_STANDARD "-std=c++14")
if (WIN32)
    set(CXX_STANDARD "/std:c++14")
endif()

set(ANDROID_DIR ../../android)
set(FILAMENT_DIR ${ANDROID_DIR}/filament-android)
set(COMMON_DIR ${ANDROID_DIR}/common)

include_directories(${JNI_INCLUDE_DIRS} ${ANDROID_DIR})

set(JNI_SOURCE_FILES
        ${FILAMENT_DIR}/src/main/cpp/Camera.cpp
        ${FILAMENT_DIR}/src/main/cpp/Colors.cpp
        ${FILAMENT_DIR}/src/main/cpp/VertexBuffer.cpp
        ${FILAMENT_DIR}/src/main/cpp/Engine.cpp
        ${FILAMENT_DIR}/src/main/cpp/EntityManager.cpp
        ${FILAMENT_DIR}/src/main/cpp/Fence.cpp
        ${FILAMENT_DIR}/src/main/cpp/IndexBuffer.cpp
        ${FILAMENT_DIR}/src/main/cpp/IndirectLight.cpp
        ${FILAMENT_DIR}/src/main/cpp/LightManager.cpp
        ${FILAMENT_DIR}/src/main/cpp/Material.cpp
        ${FILAMENT_DIR}/src/main/cpp/MaterialInstance.cpp
        ${FILAMENT_DIR}/src/main/cpp/MathUtils.cpp
        ${FILAMENT_DIR}/src/main/cpp/NativeSurface.cpp
        ${FILAMENT_DIR}/src/main/cpp/RenderableManager.cpp
        ${FILAMENT_DIR}/src/main/cpp/Renderer.cpp
        ${FILAMENT_DIR}/src/main/cpp/RenderTarget.cpp
        ${FILAMENT_DIR}/src/main/cpp/Scene.cpp
        ${FILAMENT_DIR}/src/main/cpp/SkyBox.cpp
        ${FILAMENT_DIR}/src/main/cpp/Stream.cpp
        ${FILAMENT_DIR}/src/main/cpp/Texture.cpp
        ${FILAMENT_DIR}/src/main/cpp/TextureSampler.cpp
        ${FILAMENT_DIR}/src/main/cpp/TransformManager.cpp
        ${FILAMENT_DIR}/src/main/cpp/View.cpp
        # Private utils
        ${FILAMENT_DIR}/src/main/cpp/Filament.cpp
        # Desktop specific
        ${FILAMENT_DIR}/src/main/cpp/nativewindow/JAWTUtils.cpp
        # Common utils
        ${COMMON_DIR}/CallbackUtils.cpp
        ${COMMON_DIR}/NioUtils.cpp
)

if (LINUX)
    list(APPEND JNI_SOURCE_FILES ${FILAMENT_DIR}/src/main/cpp/nativewindow/Linux.cpp)
endif()

if (WIN32)
    list(APPEND JNI_SOURCE_FILES ${FILAMENT_DIR}/src/main/cpp/nativewindow/Win32.cpp)
endif()

if (APPLE)
    list(APPEND JNI_SOURCE_FILES ${FILAMENT_DIR}/src/main/cpp/nativewindow/Darwin.mm)
endif()

add_library(${TARGET} SHARED ${JNI_SOURCE_FILES})
target_link_libraries(${TARGET} filament ${JAVA_AWT_LIBRARY})

set(INSTALL_TYPE LIBRARY)
if (WIN32 OR CYGWIN)
    set(INSTALL_TYPE RUNTIME)
endif()
install(TARGETS ${TARGET} ${INSTALL_TYPE} DESTINATION lib/${DIST_DIR})
set(LIB_SUFFIX ".so")
if (APPLE)
    set(LIB_SUFFIX ".dylib")
elseif (WIN32)
    set(LIB_SUFFIX ".dll")
endif()
install(CODE "execute_process(COMMAND ${CMAKE_STRIP} -x ${CMAKE_INSTALL_PREFIX}/lib/${DIST_DIR}/lib${TARGET}${LIB_SUFFIX})")

# ==================================================================================================
# Java APIs
# ==================================================================================================
set(TARGET filament-java)

include(UseJava)

set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.8" "-target" "1.8")

set(FILAMENT_JAVA_DIR ${FILAMENT_DIR}/src/main/java/)
get_filename_component(FILAMENT_JAVA_DIR ${FILAMENT_JAVA_DIR} ABSOLUTE)

set(JAVA_SOURCE_FILES
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Asserts.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Box.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Camera.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Colors.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Engine.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Entity.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/EntityInstance.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/EntityManager.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Fence.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Filament.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/IndexBuffer.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/IndirectLight.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/LightManager.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Material.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/MaterialInstance.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/MathUtils.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/NativeSurface.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/NioUtils.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Platform.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/RenderableManager.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Renderer.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/RenderTarget.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Scene.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Skybox.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Stream.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/SwapChain.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Texture.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/TextureSampler.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/TransformManager.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/VertexBuffer.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/View.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/Viewport.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/proguard/UsedByNative.java
        ${FILAMENT_JAVA_DIR}/com/google/android/filament/proguard/UsedByReflection.java
        # Desktop specific
        src/java/com/google/android/filament/DesktopPlatform.java
        src/java/com/google/android/filament/FilamentCanvas.java
        src/java/com/google/android/filament/FilamentPanel.java
        src/java/com/google/android/filament/FilamentTarget.java
)

add_jar(${TARGET}
        SOURCES ${JAVA_SOURCE_FILES}
        INCLUDE_JARS ../lib/support-annotations.jar)

install_jar(${TARGET} lib)
