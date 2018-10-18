cmake_minimum_required(VERSION 2.8.12)
project(moltenvk)

set(OUTPUT_LIBVK ${CMAKE_CURRENT_BINARY_DIR}/../libvulkan.1.dylib)
set(INPUT_LIBVK  ${CMAKE_CURRENT_SOURCE_DIR}/../libvulkan.1.dylib)

set(OUTPUT_ICD ${CMAKE_CURRENT_BINARY_DIR}/../libMoltenVK.dylib)
set(INPUT_ICD  ${CMAKE_CURRENT_SOURCE_DIR}/../libMoltenVK.dylib)

set(OUTPUT_JSON ${CMAKE_CURRENT_BINARY_DIR}/../MoltenVK_icd.json)
set(INPUT_JSON  ${CMAKE_CURRENT_SOURCE_DIR}/../MoltenVK_icd.json)

# Define a build step that simply copies the dylib to the destination.
add_custom_command(
        OUTPUT ${OUTPUT_LIBVK}
        COMMAND ${CMAKE_COMMAND} -E copy ${INPUT_LIBVK} ${OUTPUT_LIBVK})

add_custom_command(
        OUTPUT ${OUTPUT_ICD}
        COMMAND ${CMAKE_COMMAND} -E copy ${INPUT_ICD} ${OUTPUT_ICD})

add_custom_command(
        OUTPUT ${OUTPUT_JSON}
        COMMAND ${CMAKE_COMMAND} -E copy ${INPUT_JSON} ${OUTPUT_JSON})

# Define a fictional artifact called "moltenvk" that depends on the dylib.
add_custom_target(moltenvk ALL DEPENDS ${OUTPUT_LIBVK} ${OUTPUT_ICD} ${OUTPUT_JSON})

# Install all the molten files alongside libfilament in /usr/local/lib/${ARCH}
# Only install on Desktop- iOS links against ../ios/libMoltenVK.dylib
if(APPLE AND NOT IOS)
    install(FILES ${OUTPUT_LIBVK} DESTINATION lib/${DIST_DIR})
    install(FILES ${OUTPUT_ICD} DESTINATION lib/${DIST_DIR})
    install(FILES ${OUTPUT_JSON} DESTINATION lib/${DIST_DIR})
endif()
