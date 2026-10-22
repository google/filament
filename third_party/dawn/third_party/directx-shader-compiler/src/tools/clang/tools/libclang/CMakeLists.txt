set(LLVM_OPTIONAL_SOURCES ARCMigrate.cpp) # HLSL Change - ignore file

if (WIN32)
set (HLSL_IGNORE_SOURCES
  BuildSystem.cpp
  )

set(SOURCES
#  ARCMigrate.cpp # HLSL Change - remove ARCMigrate
  CIndex.cpp
  CIndexCXX.cpp
  CIndexCodeCompletion.cpp
  CIndexDiagnostic.cpp
  CIndexHigh.cpp
  CIndexInclusionStack.cpp
  CIndexUSRs.cpp
  CIndexer.cpp
  CXComment.cpp
  CXCursor.cpp
  CXCompilationDatabase.cpp
  CXLoadedDiagnostic.cpp
  CXSourceLocation.cpp
  CXStoredDiagnostic.cpp
  CXString.cpp
  CXType.cpp
  IndexBody.cpp
  IndexDecl.cpp
  IndexTypeSourceInfo.cpp
  Indexing.cpp
  IndexingContext.cpp
  dxcisenseimpl.cpp # HLSL Change
  dxcrewriteunused.cpp # HLSL Change

  ADDITIONAL_HEADERS
  CIndexDiagnostic.h
  CIndexer.h
  CXCursor.h
  CXLoadedDiagnostic.h
  CXSourceLocation.h
  CXString.h
  CXTranslationUnit.h
  CXType.h
  Index_Internal.h
  IndexingContext.h
  ../../include/clang-c/Index.h
  )
else ()
set(SOURCES
#  ARCMigrate.cpp # HLSL Change - remove ARCMigrate
  CIndex.cpp
  CIndexCXX.cpp
  CIndexCodeCompletion.cpp
  CIndexDiagnostic.cpp
  CIndexHigh.cpp
  CIndexInclusionStack.cpp
  CIndexUSRs.cpp
  CIndexer.cpp
  CXComment.cpp
  CXCursor.cpp
  CXCompilationDatabase.cpp
  CXLoadedDiagnostic.cpp
  CXSourceLocation.cpp
  CXStoredDiagnostic.cpp
  CXString.cpp
  CXType.cpp
  IndexBody.cpp
  IndexDecl.cpp
  IndexTypeSourceInfo.cpp
  Indexing.cpp
  IndexingContext.cpp
  dxcisenseimpl.cpp # HLSL Change
  dxcrewriteunused.cpp # HLSL Change

  ADDITIONAL_HEADERS
  CIndexDiagnostic.h
  CIndexer.h
  CXCursor.h
  CXLoadedDiagnostic.h
  CXSourceLocation.h
  CXString.h
  CXTranslationUnit.h
  CXType.h
  Index_Internal.h
  IndexingContext.h
  ../../include/clang-c/Index.h
  )

set (HLSL_IGNORE_SOURCES
  BuildSystem.cpp
)
endif(WIN32)

set(LIBS
  clangAST
  clangBasic
  clangFrontend
  clangIndex
  clangLex
  clangSema
  clangTooling
)

if (0 AND CLANG_ENABLE_ARCMT) # HLSL Change - remove ARC support
  list(APPEND LIBS clangARCMigrate)
endif ()

find_library(DL_LIBRARY_PATH dl)
if (DL_LIBRARY_PATH)
  list(APPEND LIBS dl)
endif()

option(LIBCLANG_BUILD_STATIC
  "Build libclang as a static library (in addition to a shared one)" OFF)

set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/libclang.exports)

if(MSVC)
  # Avoid LNK4197 not to spceify libclang.def here.
  # Each functions is exported as "dllexport" in include/clang-c.
  # KB835326
  set(LLVM_EXPORTED_SYMBOL_FILE)
  add_compile_options(/bigobj)
endif()

# HLSL Change Starts
# Add a definition for the link type.
add_definitions(-DCINDEX_LINKAGE=)
add_definitions(-DLIBCLANG_CC=)
set(ENABLE_STATIC STATIC)
# HLSL Change Ends

if( 0 AND LLVM_ENABLE_PIC ) # HLSL Change - disable DLL, we only use the static lib
  set(ENABLE_SHARED SHARED)
endif()

if((NOT LLVM_ENABLE_PIC OR LIBCLANG_BUILD_STATIC) AND NOT WIN32)
  set(ENABLE_STATIC STATIC)
endif()

if(WIN32)
  set(output_name "libclang")
else()
  set(output_name "clang")
endif()

if(1) # HLSL Change don't build the static target like this
add_clang_library(libclang ${ENABLE_SHARED} ${ENABLE_STATIC}
  OUTPUT_NAME ${output_name}
  ${SOURCES}
#  DEPENDS clang-headers - HLSL Change

  LINK_LIBS
  ${LIBS}

  LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Core
  Support
  MSSupport
  )

if(ENABLE_SHARED)
  set_target_properties(libclang
    PROPERTIES
    VERSION ${LIBCLANG_LIBRARY_VERSION}
    DEFINE_SYMBOL _CINDEX_LIB_)

  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(LIBCLANG_LINK_FLAGS " -Wl,-compatibility_version -Wl,1")
    if (DEFINED ${LLVM_SUBMIT_VERSION})
      set(LIBCLANG_LINK_FLAGS
        "${LIBCLANG_LINK_FLAGS} -Wl,-current_version -Wl,${LLVM_SUBMIT_VERSION}.${LLVM_SUBMIT_SUBVERSION}")
    endif()

    set_property(TARGET libclang APPEND_STRING PROPERTY
                 LINK_FLAGS ${LIBCLANG_LINK_FLAGS})
  endif()
endif()
endif() # HLSL Change

add_dependencies(libclang TablegenHLSLOptions)  # HLSL Change

# HLSL Change Starts
# add_clang_library(${LIBCLANG_STATIC_TARGET_NAME} STATIC ${SOURCES})
# target_link_libraries(${LIBCLANG_STATIC_TARGET_NAME} ${LIBS})
# add_dependencies(${LIBCLANG_STATIC_TARGET_NAME} ${GENERATED_HEADERS} clang-headers)
# HLSL Change Ends
