cmake_minimum_required(VERSION 3.15...3.26)

project(VMA LANGUAGES CXX)

add_library(VulkanMemoryAllocator INTERFACE)
add_library(GPUOpen::VulkanMemoryAllocator ALIAS VulkanMemoryAllocator)

target_include_directories(VulkanMemoryAllocator INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if (CMAKE_VERSION VERSION_LESS "3.21")
    # https://cmake.org/cmake/help/latest/variable/PROJECT_IS_TOP_LEVEL.html
    string(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} PROJECT_IS_TOP_LEVEL)
endif()

if (PROJECT_IS_TOP_LEVEL)
    include(GNUInstallDirs)

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(TARGETS VulkanMemoryAllocator EXPORT VulkanMemoryAllocatorConfig INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT VulkanMemoryAllocatorConfig NAMESPACE "GPUOpen::" DESTINATION "share/cmake/VulkanMemoryAllocator")

    option(VMA_BUILD_DOCUMENTATION "Create and install the HTML based API documentation")
    if(VMA_BUILD_DOCUMENTATION)
        find_package(Doxygen REQUIRED)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        # note the option ALL which allows to build the docs together with the application
        add_custom_target(doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
        install(DIRECTORY docs/ DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/VulkanMemoryAllocator" PATTERN ".nojekyll" EXCLUDE)
    endif()

    option(VMA_BUILD_SAMPLES "Build samples")
    if (VMA_BUILD_SAMPLES)
        add_subdirectory(src)
    endif()
endif()
