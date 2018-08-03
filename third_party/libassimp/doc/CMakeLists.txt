find_package( Doxygen REQUIRED )

set( HTML_OUTPUT "AssimpDoc_Html" CACHE STRING "Output directory for generated HTML documentation. Defaults to AssimpDoc_Html." )

# Enable Microsoft CHM help style only on Windows
set( MICROSOFT_HELP_WORKSHOP "NO")
if( MSVC )
    set( MICROSOFT_HELP_WORKSHOP "YES" )
endif( MSVC )

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
)

add_custom_target(
    docs ALL
    DEPENDS docs.done
)

add_custom_command(
    OUTPUT docs.done
    COMMAND ${DOXYGEN_EXECUTABLE}
    COMMAND ${CMAKE_COMMAND} -E touch docs.done
    COMMENT "Generating assimp documentation"
    VERBATIM
    )

if( DEFINED CMAKE_INSTALL_DOCDIR )
    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${HTML_OUTPUT}
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
    )
    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/AssimpDoc_Html/AnimationOverview.png
        ${CMAKE_CURRENT_SOURCE_DIR}/AssimpDoc_Html/AnimationOverview.svg
        ${CMAKE_CURRENT_SOURCE_DIR}/AssimpDoc_Html/dragonsplash.png
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/${HTML_OUTPUT}
    )
endif( DEFINED CMAKE_INSTALL_DOCDIR )

