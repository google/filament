if(WIN32)
    add_subdirectory(OSDependent/Windows)
elseif(UNIX)
    add_subdirectory(OSDependent/Unix)
else(WIN32)
    message("unknown platform")
endif(WIN32)

if(EMSCRIPTEN OR ENABLE_GLSLANG_JS)
    # May be enabled on non-Emscripten builds for binary-size testing.
    add_subdirectory(OSDependent/Web)
endif(EMSCRIPTEN OR ENABLE_GLSLANG_JS)

set(SOURCES
    MachineIndependent/glslang.m4
    MachineIndependent/glslang.y
    MachineIndependent/glslang_tab.cpp
    MachineIndependent/attribute.cpp
    MachineIndependent/Constant.cpp
    MachineIndependent/iomapper.cpp
    MachineIndependent/InfoSink.cpp
    MachineIndependent/Initialize.cpp
    MachineIndependent/IntermTraverse.cpp
    MachineIndependent/Intermediate.cpp
    MachineIndependent/ParseContextBase.cpp
    MachineIndependent/ParseHelper.cpp
    MachineIndependent/PoolAlloc.cpp
    MachineIndependent/RemoveTree.cpp
    MachineIndependent/Scan.cpp
    MachineIndependent/ShaderLang.cpp
    MachineIndependent/SymbolTable.cpp
    MachineIndependent/Versions.cpp
    MachineIndependent/intermOut.cpp
    MachineIndependent/limits.cpp
    MachineIndependent/linkValidate.cpp
    MachineIndependent/parseConst.cpp
    MachineIndependent/reflection.cpp
    MachineIndependent/preprocessor/Pp.cpp
    MachineIndependent/preprocessor/PpAtom.cpp
    MachineIndependent/preprocessor/PpContext.cpp
    MachineIndependent/preprocessor/PpScanner.cpp
    MachineIndependent/preprocessor/PpTokens.cpp
    MachineIndependent/propagateNoContraction.cpp
    GenericCodeGen/CodeGen.cpp
    GenericCodeGen/Link.cpp
    CInterface/glslang_c_interface.cpp)

set(HEADERS
    Public/ShaderLang.h
    Include/arrays.h
    Include/BaseTypes.h
    Include/Common.h
    Include/ConstantUnion.h
    Include/glslang_c_interface.h
    Include/glslang_c_shader_types.h
    Include/InfoSink.h
    Include/InitializeGlobals.h
    Include/intermediate.h
    Include/PoolAlloc.h
    Include/ResourceLimits.h
    Include/revision.h
    Include/ShHandle.h
    Include/Types.h
    MachineIndependent/attribute.h
    MachineIndependent/glslang_tab.cpp.h
    MachineIndependent/gl_types.h
    MachineIndependent/Initialize.h
    MachineIndependent/iomapper.h
    MachineIndependent/LiveTraverser.h
    MachineIndependent/localintermediate.h
    MachineIndependent/ParseHelper.h
    MachineIndependent/reflection.h
    MachineIndependent/RemoveTree.h
    MachineIndependent/Scan.h
    MachineIndependent/ScanContext.h
    MachineIndependent/SymbolTable.h
    MachineIndependent/Versions.h
    MachineIndependent/parseVersions.h
    MachineIndependent/propagateNoContraction.h
    MachineIndependent/preprocessor/PpContext.h
    MachineIndependent/preprocessor/PpTokens.h)

glslang_pch(SOURCES MachineIndependent/pch.cpp)

add_library(glslang ${LIB_TYPE} ${BISON_GLSLParser_OUTPUT_SOURCE} ${SOURCES} ${HEADERS})
set_property(TARGET glslang PROPERTY FOLDER glslang)
set_property(TARGET glslang PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(glslang OGLCompiler OSDependent)
target_include_directories(glslang PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(glslang PROPERTIES PREFIX "")
endif()

if(ENABLE_HLSL)
    target_link_libraries(glslang HLSL)
endif()

if(WIN32)
    source_group("Public" REGULAR_EXPRESSION "Public/*")
    source_group("MachineIndependent" REGULAR_EXPRESSION "MachineIndependent/[^/]*")
    source_group("Include" REGULAR_EXPRESSION "Include/[^/]*")
    source_group("GenericCodeGen" REGULAR_EXPRESSION "GenericCodeGen/*")
    source_group("MachineIndependent\\Preprocessor" REGULAR_EXPRESSION "MachineIndependent/preprocessor/*")
endif(WIN32)

if(ENABLE_GLSLANG_INSTALL)
    if(BUILD_SHARED_LIBS)
        install(TARGETS glslang EXPORT glslangTargets
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    else()
        install(TARGETS glslang EXPORT glslangTargets
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
    install(EXPORT glslangTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)
endif(ENABLE_GLSLANG_INSTALL)

if(ENABLE_GLSLANG_INSTALL)
    foreach(file ${HEADERS})
        get_filename_component(dir ${file} DIRECTORY)
        install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/glslang/${dir})
    endforeach()
endif(ENABLE_GLSLANG_INSTALL)
