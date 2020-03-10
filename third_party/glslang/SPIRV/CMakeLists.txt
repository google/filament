set(SOURCES
    GlslangToSpv.cpp
    InReadableOrder.cpp
    Logger.cpp
    SpvBuilder.cpp
    SpvPostProcess.cpp
    doc.cpp
    SpvTools.cpp
    disassemble.cpp)

set(SPVREMAP_SOURCES
    SPVRemapper.cpp
    doc.cpp)

set(HEADERS
    bitutils.h
    spirv.hpp
    GLSL.std.450.h
    GLSL.ext.EXT.h
    GLSL.ext.KHR.h
    GlslangToSpv.h
    hex_float.h
    Logger.h
    SpvBuilder.h
    spvIR.h
    doc.h
    SpvTools.h
    disassemble.h
    GLSL.ext.AMD.h
    GLSL.ext.NV.h
    NonSemanticDebugPrintf.h)

set(SPVREMAP_HEADERS
    SPVRemapper.h
    doc.h)

add_library(SPIRV ${LIB_TYPE} ${SOURCES} ${HEADERS})
set_property(TARGET SPIRV PROPERTY FOLDER glslang)
set_property(TARGET SPIRV PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(SPIRV PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if (ENABLE_SPVREMAPPER)
    add_library(SPVRemapper ${LIB_TYPE} ${SPVREMAP_SOURCES} ${SPVREMAP_HEADERS})
    set_property(TARGET SPVRemapper PROPERTY FOLDER glslang)
    set_property(TARGET SPVRemapper PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(SPIRV PROPERTIES PREFIX "")
    if (ENABLE_SPVREMAPPER)
        set_target_properties(SPVRemapper PROPERTIES PREFIX "")
    endif()
endif()

if(ENABLE_OPT)
    target_include_directories(SPIRV
        PRIVATE ${spirv-tools_SOURCE_DIR}/include
        PRIVATE ${spirv-tools_SOURCE_DIR}/source
    )
    target_link_libraries(SPIRV glslang SPIRV-Tools-opt)
    target_include_directories(SPIRV PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../External>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/External>)
else()
    target_link_libraries(SPIRV glslang)
endif(ENABLE_OPT)

if(WIN32)
    source_group("Source" FILES ${SOURCES} ${HEADERS})
    source_group("Source" FILES ${SPVREMAP_SOURCES} ${SPVREMAP_HEADERS})
endif(WIN32)

if(ENABLE_GLSLANG_INSTALL)
    if(BUILD_SHARED_LIBS)
        if (ENABLE_SPVREMAPPER)
            install(TARGETS SPVRemapper EXPORT SPVRemapperTargets
                    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endif()
        install(TARGETS SPIRV EXPORT SPIRVTargets
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    else()
        if (ENABLE_SPVREMAPPER)
            install(TARGETS SPVRemapper EXPORT SPVRemapperTargets
                    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endif()
        install(TARGETS SPIRV EXPORT SPIRVTargets
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()

    if (ENABLE_SPVREMAPPER)
        install(EXPORT SPVRemapperTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)
    endif()

    install(EXPORT SPIRVTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)

    install(FILES ${HEADERS} ${SPVREMAP_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SPIRV/)
    install(FILES ${HEADERS} ${SPVREMAP_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/glslang/SPIRV/)
endif(ENABLE_GLSLANG_INSTALL)
