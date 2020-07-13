cmake_minimum_required(VERSION 3.10)
project(filament-java)

if (NOT FILAMENT_ENABLE_JAVA)
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
set(TARGET gltfio-jni)

set(CXX_STANDARD "-std=c++14")
if (WIN32)
    set(CXX_STANDARD "/std:c++14")
endif()

set(ANDROID_DIR ../../android)
set(GLTFIO_DIR ${ANDROID_DIR}/gltfio-android)
set(COMMON_DIR ${ANDROID_DIR}/common)

include_directories(${JNI_INCLUDE_DIRS} ${ANDROID_DIR})

set(JNI_SOURCE_FILES
    ${GLTFIO_DIR}/src/main/cpp/Animator.cpp
    ${GLTFIO_DIR}/src/main/cpp/AssetLoader.cpp
    ${GLTFIO_DIR}/src/main/cpp/FilamentAsset.cpp
    ${GLTFIO_DIR}/src/main/cpp/FilamentInstance.cpp
    ${GLTFIO_DIR}/src/main/cpp/MaterialProvider.cpp
    ${GLTFIO_DIR}/src/main/cpp/ResourceLoader.cpp
    ${COMMON_DIR}/CallbackUtils.cpp
    ${COMMON_DIR}/NioUtils.cpp
)

add_library(${TARGET} SHARED ${JNI_SOURCE_FILES})
target_link_libraries(${TARGET} gltfio_core gltfio_resources)

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
set(TARGET gltfio-java)

include(UseJava)

set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.8" "-target" "1.8")

set(GLTFIO_JAVA_DIR ${GLTFIO_DIR}/src/main/java/)
get_filename_component(GLTFIO_JAVA_DIR ${GLTFIO_JAVA_DIR} ABSOLUTE)

set(JAVA_SOURCE_FILES
    ${GLTFIO_JAVA_DIR}/com/google/android/filament/gltfio/Animator.java
    ${GLTFIO_JAVA_DIR}/com/google/android/filament/gltfio/AssetLoader.java
    ${GLTFIO_JAVA_DIR}/com/google/android/filament/gltfio/FilamentAsset.java
    ${GLTFIO_JAVA_DIR}/com/google/android/filament/gltfio/FilamentInstance.java
    ${GLTFIO_JAVA_DIR}/com/google/android/filament/gltfio/Gltfio.java
    ${GLTFIO_JAVA_DIR}/com/google/android/filament/gltfio/MaterialProvider.java
    ${GLTFIO_JAVA_DIR}/com/google/android/filament/gltfio/ResourceLoader.java
)

add_jar(${TARGET}
        SOURCES ${JAVA_SOURCE_FILES}
        INCLUDE_JARS
            ../lib/annotation-1.0.0.jar
            filament-java
)

install_jar(${TARGET} lib)
