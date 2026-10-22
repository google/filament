

if(UNIX)
    ### Copy the 64-bit ELF file into the binaries folder for later use.
    ### Also, copy the 64-bit ELF-but call it a DLL so that we can use the wrong target (ELF vs. PE) on Windows
    #
    ### NOTE: Uncomment the following to rebuild the targets
    #
    #add_library(dummy_library_elf_64 SHARED stub.cpp)
    #set_target_properties(dummy_library_elf_64 PROPERTIES COMPILE_OPTIONS "-m64" LINK_FLAGS "-m64")
    #add_custom_command(TARGET dummy_library_elf_64 POST_BUILD
    #    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy_library_elf_64> ${CMAKE_CURRENT_SOURCE_DIR}/binaries
    #    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy_library_elf_64> ${CMAKE_CURRENT_SOURCE_DIR}/binaries/dummy_library_elf_64.dll)

    ### Copy the 32-bit ELF file into the binaries folder for later use.
    ### Also, copy the 32-bit ELF-but call it a DLL so that we can use the wrong target (ELF vs. PE) on Windows
    #
    ### NOTE: Uncomment the following to rebuild the targets
    #
    #add_library(dummy_library_elf_32 SHARED stub.cpp)
    #set_target_properties(dummy_library_elf_32 PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")
    #add_custom_command(TARGET dummy_library_elf_32 POST_BUILD
    #    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy_library_elf_32> ${CMAKE_CURRENT_SOURCE_DIR}/binaries
    #    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy_library_elf_32> ${CMAKE_CURRENT_SOURCE_DIR}/binaries/dummy_library_elf_32.dll)
endif()

if(WIN32)
    ### Copy the 64-bit PE file into the binaries folder for later use.
    ### Also, copy the 64-bit PE-but call it a SO so that we can use the wrong target (ELF vs. PE) on Linux
    #
    ### NOTE: Uncomment the following to rebuild the targets
    #
    # add_library(dummy_library_pe_64 SHARED stub.cpp)
    # add_custom_command(TARGET dummy_library_pe_64 POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy_library_pe_64> ${CMAKE_CURRENT_SOURCE_DIR}/binaries
    #     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy_library_pe_64> ${CMAKE_CURRENT_SOURCE_DIR}/binaries/dummy_library_pe_64.so)

    ### Copy the 32-bit PE file into the binaries folder for later use.
    ### Also, copy the 32-bit PE-but call it a SO so that we can use the wrong target (ELF vs. PE) on Linux
    #
    ### NOTE: Uncomment the following to rebuild the targets
    #
    # add_library(dummy_library_pe_32 SHARED stub.cpp)
    # add_custom_command(TARGET dummy_library_pe_32 POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy_library_pe_32> ${CMAKE_CURRENT_SOURCE_DIR}/binaries
    #     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy_library_pe_32> ${CMAKE_CURRENT_SOURCE_DIR}/binaries/dummy_library_pe_32.so)
endif()
