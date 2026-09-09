# TODO: Add -maltivec when ARCH is PowerPC.

set(LLVM_LINK_COMPONENTS support)

# ----------------------------------------------------------------------------
# Embedded HLSL headers.
#
# Each file under tools/clang/lib/Headers/hlsl/ that we want to ship as
# compiled-in data is converted at build time to a small C++ snippet (an
# llvm::StringRef "Data" definition) by utils/embed_header.py.  A second
# script (utils/generate_hlsl_embedded_headers.py) then aggregates those
# snippets into two ``.inc`` files:
#
#   * HLSLEmbeddedHeadersDecls.inc -- one ``namespace <ns> { #include ... }``
#     per header so each ``Data`` lives in its own namespace.
#   * HLSLEmbeddedHeadersEntries.inc -- an X-macro list of (rel_path, ns)
#     pairs.
#
# Those ``.inc`` files are textually included by the hand-written
# HLSLEmbeddedHeaders.cpp in this directory, which exposes the whole
# collection through clang::hlsl::getEmbeddedHeaders() as a
# StringMap<StringRef>.  This compiled-in data lets the preprocessor
# satisfy #includes of the bundled headers without consulting the
# filesystem.
#
# The list of embedded inputs is intentionally enumerated explicitly
# below: file(GLOB ...) is evaluated at configuration time, so a glob
# would silently miss files added between configure and build.
# Vulkan-specific headers under hlsl/vk/ are only embedded when SPIR-V
# codegen is enabled, to keep the DXC-only build free of unused data.
# ----------------------------------------------------------------------------

set(hlsl_headers_src_dir
    "${CMAKE_CURRENT_SOURCE_DIR}/../Headers/hlsl")
set(hlsl_embedded_dir
    "${CMAKE_CURRENT_BINARY_DIR}/HLSLEmbeddedHeaders")
set(embed_header_script
    "${LLVM_MAIN_SRC_DIR}/utils/embed_header.py")
set(generate_embedded_headers_script
    "${LLVM_MAIN_SRC_DIR}/utils/generate_hlsl_embedded_headers.py")

set(hlsl_embedded_inputs
    enable_if
    type_traits
    vector_utils
    dx/linalg.h
    )

if (ENABLE_SPIRV_CODEGEN)
  list(APPEND hlsl_embedded_inputs
       vk/khr/cooperative_matrix.h
       vk/khr/cooperative_matrix.impl
       vk/opcode_selector.h
       vk/spirv.h
       )
endif()

set(hlsl_embedded_inc_files)
set(hlsl_embedded_entries)
foreach(rel_path ${hlsl_embedded_inputs})
  set(src "${hlsl_headers_src_dir}/${rel_path}")
  set(inc "${hlsl_embedded_dir}/${rel_path}.inc")
  add_custom_command(
    OUTPUT "${inc}"
    DEPENDS "${src}" "${embed_header_script}"
    COMMAND "${Python3_EXECUTABLE}" "${embed_header_script}" "${src}" "${inc}"
    COMMENT "Embedding HLSL header ${rel_path}")
  list(APPEND hlsl_embedded_inc_files "${inc}")
  list(APPEND hlsl_embedded_entries "--entry" "${rel_path}=${inc}")
endforeach()

set(hlsl_embedded_decls "${hlsl_embedded_dir}/HLSLEmbeddedHeadersDecls.inc")
set(hlsl_embedded_entries_inc
    "${hlsl_embedded_dir}/HLSLEmbeddedHeadersEntries.inc")
add_custom_command(
  OUTPUT "${hlsl_embedded_decls}" "${hlsl_embedded_entries_inc}"
  DEPENDS ${hlsl_embedded_inc_files} "${generate_embedded_headers_script}"
  COMMAND "${Python3_EXECUTABLE}" "${generate_embedded_headers_script}"
          --decls-output "${hlsl_embedded_decls}"
          --entries-output "${hlsl_embedded_entries_inc}"
          ${hlsl_embedded_entries}
  COMMENT "Generating HLSL embedded headers includes")

set_source_files_properties("${hlsl_embedded_decls}"
                            "${hlsl_embedded_entries_inc}"
                            PROPERTIES GENERATED TRUE)

# Make the generated .inc files visible to HLSLEmbeddedHeaders.cpp.
include_directories("${hlsl_embedded_dir}")

# Add the generated .inc files as inputs to the target so that they're treated
# as dependencies of the target and get built before the .cpp file that includes
# them.
set(hlsl_embedded_generated_files
    "${hlsl_embedded_decls}"
    "${hlsl_embedded_entries_inc}")

add_clang_library(clangLex
  HeaderMap.cpp
  HeaderSearch.cpp
  HLSLEmbeddedHeaders.cpp
  HLSLMacroExpander.cpp
  Lexer.cpp
  LiteralSupport.cpp
  MacroArgs.cpp
  MacroInfo.cpp
  ModuleMap.cpp
  PPCaching.cpp
  PPCallbacks.cpp
  PPConditionalDirectiveRecord.cpp
  PPDirectives.cpp
  PPExpressions.cpp
  PPLexerChange.cpp
  PPMacroExpansion.cpp
  PTHLexer.cpp
  Pragma.cpp
  PreprocessingRecord.cpp
  Preprocessor.cpp
  PreprocessorLexer.cpp
  ScratchBuffer.cpp
  TokenConcatenation.cpp
  TokenLexer.cpp
  ${hlsl_embedded_generated_files}

  LINK_LIBS
  clangBasic
  )
