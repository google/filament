# This file will only be executed if VMA_BUILD_REPLAY is set to ON

if (WIN32)
    set(VMA_REPLAY_SOURCE_FILES
        Common.cpp
        Constants.cpp
        VmaReplay.cpp
        VmaUsage.cpp
    )

    add_executable(VmaReplay ${VMA_REPLAY_SOURCE_FILES})

    # Enable multithreaded compiling
    target_compile_options(VmaReplay PRIVATE "/MP")

    target_link_libraries(
        VmaReplay

        PRIVATE
        Vulkan::Vulkan
    )
else()
    message(STATUS "VmaReplay is not supported on Linux")
endif()
