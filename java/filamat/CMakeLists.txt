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
set(TARGET filamat-jni)

include_directories(${JNI_INCLUDE_DIRS})

set(CXX_STANDARD "-std=c++14")
if (WIN32)
    set(CXX_STANDARD "/std:c++14")
endif()

set(FILAMAT_DIR ../../android/filamat-android)

set(JNI_SOURCE_FILES
    ${FILAMAT_DIR}/src/main/cpp/MaterialBuilder.cpp
)

add_library(${TARGET} SHARED ${JNI_SOURCE_FILES})
target_link_libraries(${TARGET} filamat)

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
set(TARGET filamat-java)

include(UseJava)

set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.8" "-target" "1.8")

set(FILAMAT_JAVA_DIR ${FILAMAT_DIR}/src/main/java/)
get_filename_component(FILAMAT_JAVA_DIR ${FILAMAT_JAVA_DIR} ABSOLUTE)

set(JAVA_SOURCE_FILES
    ${FILAMAT_JAVA_DIR}/com/google/android/filament/filamat/MaterialBuilder.java
    ${FILAMAT_JAVA_DIR}/com/google/android/filament/filamat/MaterialPackage.java
)

add_jar(${TARGET}
        SOURCES ${JAVA_SOURCE_FILES}
        INCLUDE_JARS ../lib/annotation-1.0.0.jar)

install_jar(${TARGET} lib)
