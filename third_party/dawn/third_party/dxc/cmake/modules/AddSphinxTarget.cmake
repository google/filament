# Handy function for creating the different Sphinx targets.
#
# ``builder`` should be one of the supported builders used by
# the sphinx-build command.
#
# ``project`` should be the project name
function (add_sphinx_target builder project)
  set(SPHINX_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${builder}")
  set(SPHINX_DOC_TREE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
  set(SPHINX_TARGET_NAME docs-${project}-${builder})

  if (SPHINX_WARNINGS_AS_ERRORS)
    set(SPHINX_WARNINGS_AS_ERRORS_FLAG "-W")
  else()
    set(SPHINX_WARNINGS_AS_ERRORS_FLAG "")
  endif()

  add_custom_target(${SPHINX_TARGET_NAME}
                    COMMAND ${SPHINX_EXECUTABLE}
                            -b ${builder}
                            -d "${SPHINX_DOC_TREE_DIR}"
                            -q # Quiet: no output other than errors and warnings.
                            ${SPHINX_WARNINGS_AS_ERRORS_FLAG} # Treat warnings as errors if requested
                            "${CMAKE_CURRENT_SOURCE_DIR}" # Source
                            "${SPHINX_BUILD_DIR}" # Output
                    COMMENT
                    "Generating ${builder} Sphinx documentation for ${project} into \"${SPHINX_BUILD_DIR}\"")

  # When "clean" target is run, remove the Sphinx build directory
  set_property(DIRECTORY APPEND PROPERTY
               ADDITIONAL_MAKE_CLEAN_FILES
               "${SPHINX_BUILD_DIR}")

  # We need to remove ${SPHINX_DOC_TREE_DIR} when make clean is run
  # but we should only add this path once
  get_property(_CURRENT_MAKE_CLEAN_FILES
               DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES)
  list(FIND _CURRENT_MAKE_CLEAN_FILES "${SPHINX_DOC_TREE_DIR}" _INDEX)
  if (_INDEX EQUAL -1)
    set_property(DIRECTORY APPEND PROPERTY
                 ADDITIONAL_MAKE_CLEAN_FILES
                 "${SPHINX_DOC_TREE_DIR}")
  endif()

  if (LLVM_BUILD_DOCS)
    add_dependencies(sphinx ${SPHINX_TARGET_NAME})

    # Handle installation
    if (NOT LLVM_INSTALL_TOOLCHAIN_ONLY)
      if (builder STREQUAL man)
        # FIXME: We might not ship all the tools that these man pages describe
        install(DIRECTORY "${SPHINX_BUILD_DIR}/" # Slash indicates contents of
                DESTINATION share/man/man1)

      elseif (builder STREQUAL html)
        install(DIRECTORY "${SPHINX_BUILD_DIR}"
                DESTINATION "share/doc/${project}")
      else()
        message(WARNING Installation of ${builder} not supported)
      endif()
    endif()
  endif()
endfunction()
