
# Because of bugs and limitations in its Markdown support, only fairly recent
# versions of Doxygen can produce acceptable output
set(MINIMUM_DOXYGEN_VERSION 1.9.8)

# NOTE: The order of this list determines the order of items in the Guides
#       (i.e. Pages) list in the generated documentation
set(source_files
    main.md
    news.md
    quick.md
    moving.md
    compile.md
    build.md
    intro.md
    context.md
    monitor.md
    window.md
    input.md
    vulkan.md
    compat.md
    internal.md)

set(extra_files DoxygenLayout.xml header.html footer.html extra.css spaces.svg)

set(header_paths
    "${GLFW_SOURCE_DIR}/include/GLFW/glfw3.h"
    "${GLFW_SOURCE_DIR}/include/GLFW/glfw3native.h")

# Format the source list into a Doxyfile INPUT value that Doxygen can parse
foreach(path IN LISTS header_paths)
    string(APPEND GLFW_DOXYGEN_INPUT " \\\n\"${path}\"")
endforeach()
foreach(file IN LISTS source_files)
    string(APPEND GLFW_DOXYGEN_INPUT " \\\n\"${CMAKE_CURRENT_SOURCE_DIR}/${file}\"")
endforeach()

set(DOXYGEN_SKIP_DOT TRUE)
find_package(Doxygen ${MINIMUM_DOXYGEN_VERSION} QUIET)

if (NOT DOXYGEN_FOUND)
    message(STATUS "Documentation generation requires Doxygen ${MINIMUM_DOXYGEN_VERSION} or later")
else()
    configure_file(Doxyfile.in Doxyfile @ONLY)
    add_custom_command(OUTPUT "html/index.html"
                       COMMAND "${DOXYGEN_EXECUTABLE}"
                       WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                       MAIN_DEPENDENCY Doxyfile
                       DEPENDS ${header_paths} ${source_files} ${extra_files}
                       COMMENT "Generating HTML documentation"
                       VERBATIM)

    add_custom_target(docs ALL SOURCES ${source_files} DEPENDS "html/index.html")
    set_target_properties(docs PROPERTIES FOLDER "GLFW3")

    if (GLFW_INSTALL)
        install(DIRECTORY "${GLFW_BINARY_DIR}/docs/html"
                DESTINATION "${CMAKE_INSTALL_DOCDIR}")
    endif()
endif()

