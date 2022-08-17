set(SOURCES
        ../GlslangToSpv.cpp
        ../InReadableOrder.cpp
        ../Logger.cpp
        ../SpvBuilder.cpp
        ../SpvPostProcess.cpp
        ../SPVRemapper.cpp
        ../doc.cpp
        ../disassemble.cpp)

set(HEADERS
        ../bitutils.h
        ../spirv.hpp
        ../GLSL.std.450.h
        ../GLSL.ext.EXT.h
        ../GLSL.ext.KHR.h
        ../GlslangToSpv.h
        ../hex_float.h
        ../Logger.h
        ../SpvBuilder.h
        ../spvIR.h
        ../doc.h
        ../SPVRemapper.h
        ../disassemble.h)

if(ENABLE_AMD_EXTENSIONS)
    list(APPEND
            HEADERS
            ../GLSL.ext.AMD.h)
endif(ENABLE_AMD_EXTENSIONS)

if(ENABLE_NV_EXTENSIONS)
    list(APPEND
            HEADERS
            ../GLSL.ext.NV.h)
endif(ENABLE_NV_EXTENSIONS)

add_library(SPIRV STATIC ${SOURCES} ${HEADERS})
target_include_directories(SPIRV PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(SPIRV PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set_property(TARGET SPIRV PROPERTY FOLDER ThirdParty/glslang)
set_property(TARGET SPIRV PROPERTY POSITION_INDEPENDENT_CODE ON)

glslang_add_build_info_dependency(SPIRV)

if(ENABLE_OPT)
    target_include_directories(SPIRV
            PRIVATE ${spirv-tools_SOURCE_DIR}/include
            PRIVATE ${spirv-tools_SOURCE_DIR}/source
            )
    target_link_libraries(SPIRV glslang SPIRV-Tools-opt)
else()
    target_link_libraries(SPIRV glslang)
endif(ENABLE_OPT)

if(WIN32)
    source_group("Source" FILES ${SOURCES} ${HEADERS})
endif(WIN32)

if(ENABLE_GLSLANG_INSTALL)
    install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SPIRV/)
endif(ENABLE_GLSLANG_INSTALL)
