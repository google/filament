cmake_minimum_required(VERSION 3.10)
project(webdocs)

# We do not use CMake's find_package facility because it can't find homebrew-installed python3,
# or the python3 executable on some corporate Linux machines.
find_program(PYTHON3 python3)

if(${PYTHON3} STREQUAL "PYTHON3-NOTFOUND")
    message(WARNING "Python 3 not found, skipping web doc generation.")
else()

    file(STRINGS tutorials.txt TUTORIALS)

    set(CUSTOM_OUTPUTS)
    set(CUSTOM_INPUTS)
    foreach (NAME ${TUTORIALS})
        list(APPEND CUSTOM_OUTPUTS ${PROJECT_BINARY_DIR}/demo_${NAME}.html)
        list(APPEND CUSTOM_OUTPUTS ${PROJECT_BINARY_DIR}/tutorial_${NAME}.html)
        list(APPEND CUSTOM_OUTPUTS ${PROJECT_BINARY_DIR}/tutorial_${NAME}.js)
        list(APPEND CUSTOM_INPUTS ${CMAKE_CURRENT_SOURCE_DIR}/tutorial_${NAME}.md)
    endforeach()

    add_custom_command(
        OUTPUT ${CUSTOM_OUTPUTS}
        COMMAND ${PYTHON3} ${CMAKE_CURRENT_SOURCE_DIR}/build.py
            --output-folder ${PROJECT_BINARY_DIR}
            --build-folder ${PROJECT_BINARY_DIR}/../..
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/build.py
        DEPENDS ${CUSTOM_INPUTS} filament-js websamples)

    add_custom_target(${PROJECT_NAME} ALL DEPENDS ${CUSTOM_OUTPUTS})

endif()
