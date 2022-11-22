if(WIN32)
    add_subdirectory(../OSDependent/Windows "${CMAKE_CURRENT_BINARY_DIR}/OSDependent/Windows")
elseif(UNIX)
    add_subdirectory(../OSDependent/Unix "${CMAKE_CURRENT_BINARY_DIR}/OSDependent/Unix")
else(WIN32)
    message("unknown platform")
endif(WIN32)

set(SOURCES
        ../MachineIndependent/glslang.y
        ../MachineIndependent/glslang_tab.cpp
        ../MachineIndependent/attribute.cpp
        ../MachineIndependent/Constant.cpp
        ../MachineIndependent/iomapper.cpp
        ../MachineIndependent/InfoSink.cpp
        ../MachineIndependent/Initialize.cpp
        ../MachineIndependent/IntermTraverse.cpp
        ../MachineIndependent/Intermediate.cpp
        ../MachineIndependent/ParseContextBase.cpp
        ../MachineIndependent/ParseHelper.cpp
        ../MachineIndependent/PoolAlloc.cpp
        ../MachineIndependent/RemoveTree.cpp
        ../MachineIndependent/Scan.cpp
        ../MachineIndependent/ShaderLang.cpp
        ../MachineIndependent/SpirvIntrinsics.cpp
        ../MachineIndependent/SymbolTable.cpp
        ../MachineIndependent/Versions.cpp
        ../MachineIndependent/intermOut.cpp
        ../MachineIndependent/limits.cpp
        ../MachineIndependent/linkValidate.cpp
        ../MachineIndependent/parseConst.cpp
        ../MachineIndependent/reflection.cpp
        ../MachineIndependent/preprocessor/Pp.cpp
        ../MachineIndependent/preprocessor/PpAtom.cpp
        ../MachineIndependent/preprocessor/PpContext.cpp
        ../MachineIndependent/preprocessor/PpScanner.cpp
        ../MachineIndependent/preprocessor/PpTokens.cpp
        ../MachineIndependent/propagateNoContraction.cpp
        ../GenericCodeGen/CodeGen.cpp
        ../GenericCodeGen/Link.cpp)

set(HEADERS
        ../Public/ShaderLang.h
        ../Include/arrays.h
        ../Include/BaseTypes.h
        ../Include/Common.h
        ../Include/ConstantUnion.h
        ../Include/InfoSink.h
        ../Include/InitializeGlobals.h
        ../Include/intermediate.h
        ../Include/PoolAlloc.h
        ../Include/ResourceLimits.h
        ../Include/ShHandle.h
        ../Include/Types.h
        ../MachineIndependent/attribute.h
        ../MachineIndependent/glslang_tab.cpp.h
        ../MachineIndependent/gl_types.h
        ../MachineIndependent/Initialize.h
        ../MachineIndependent/iomapper.h
        ../MachineIndependent/LiveTraverser.h
        ../MachineIndependent/localintermediate.h
        ../MachineIndependent/ParseHelper.h
        ../MachineIndependent/reflection.h
        ../MachineIndependent/RemoveTree.h
        ../MachineIndependent/Scan.h
        ../MachineIndependent/ScanContext.h
        ../MachineIndependent/SymbolTable.h
        ../MachineIndependent/Versions.h
        ../MachineIndependent/parseVersions.h
        ../MachineIndependent/propagateNoContraction.h
        ../MachineIndependent/preprocessor/PpContext.h
        ../MachineIndependent/preprocessor/PpTokens.h)

# This might be useful for making grammar changes:
#
# find_package(BISON)
# add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/MachineIndependent/glslang_tab.cpp ${CMAKE_CURRENT_SOURCE_DIR}/MachineIndependent/glslang_tab.cpp.h
#                    COMMAND ${BISON_EXECUTABLE} --defines=${CMAKE_CURRENT_SOURCE_DIR}/MachineIndependent/glslang_tab.cpp.h -t ${CMAKE_CURRENT_SOURCE_DIR}/MachineIndependent/glslang.y -o ${CMAKE_CURRENT_SOURCE_DIR}/MachineIndependent/glslang_tab.cpp
#                    MAIN_DEPENDENCY MachineIndependent/glslang.y
#                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
# set(BISON_GLSLParser_OUTPUT_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/MachineIndependent/glslang_tab.cpp)

add_library(glslang STATIC ${BISON_GLSLParser_OUTPUT_SOURCE} ${SOURCES} ${HEADERS})
set_property(TARGET glslang PROPERTY FOLDER ThirdParty/glslang)
set_property(TARGET glslang PROPERTY POSITION_INDEPENDENT_CODE ON)

glslang_add_build_info_dependency(glslang)

set(PUBLIC_HEADER_FOLDERS
        ../Include
        ../Public
        ../MachineIndependent)

target_include_directories(glslang PUBLIC ${PUBLIC_HEADER_FOLDERS})
target_link_libraries(glslang OGLCompiler OSDependent)
#if(ENABLE_HLSL)
#    target_link_libraries(glslang HLSL)
#endif()

if(WIN32)
    source_group("../Public" REGULAR_EXPRESSION "Public/*")
    source_group("../MachineIndependent" REGULAR_EXPRESSION "MachineIndependent/[^/]*")
    source_group("../Include" REGULAR_EXPRESSION "Include/[^/]*")
    source_group("../GenericCodeGen" REGULAR_EXPRESSION "GenericCodeGen/*")
    source_group("MachineIndependent\\Preprocessor" REGULAR_EXPRESSION "MachineIndependent/preprocessor/*")
endif(WIN32)

if(ENABLE_GLSLANG_INSTALL)
    install(TARGETS glslang
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif(ENABLE_GLSLANG_INSTALL)

if(ENABLE_GLSLANG_INSTALL)
    foreach(file ${HEADERS})
        get_filename_component(dir ${file} DIRECTORY)
        install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/glslang/${dir})
    endforeach()
endif(ENABLE_GLSLANG_INSTALL)
