cmake_minimum_required(VERSION 3.4.1)
project(cfilament)

# ==================================================================================================
# C wrapper around filament
# ==================================================================================================
set(TARGET cfilament)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_STANDARD}")

set(NATIVE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Colors.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/VertexBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EntityManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Fence.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/IndexBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/IndirectLight.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/LightManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Material.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MaterialInstance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderableManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Scene.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SkyBox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Stream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TextureSampler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TransformManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/View.cpp
        )

add_library(${TARGET} SHARED ${NATIVE_SOURCES})
target_compile_definitions(${TARGET} PRIVATE BUILD_CFILAMENT)
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${TARGET} filament)
if (NOT WIN32)
    target_compile_options(${TARGET} PRIVATE -fvisibility=hidden)
    set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "-Wl,--exclude-libs,ALL")
endif ()

set(INSTALL_TYPE LIBRARY)
if (WIN32 OR CYGWIN)
    set(INSTALL_TYPE RUNTIME)
endif ()
install(TARGETS ${TARGET} ${INSTALL_TYPE} DESTINATION lib/${DIST_DIR})
set(LIB_SUFFIX ".so")
if (APPLE)
    set(LIB_SUFFIX ".dylib")
elseif (WIN32)
    set(LIB_SUFFIX ".dll")
endif ()
install(CODE "execute_process(COMMAND ${CMAKE_STRIP} -x ${CMAKE_INSTALL_PREFIX}/lib/${DIST_DIR}/lib${TARGET}${LIB_SUFFIX})")
