function(glmCreateTestGTC NAME)
	file(GLOB_RECURSE CORE_SOURCE ../gli/core/*.cpp)
	file(GLOB_RECURSE CORE_INLINE ../gli/core/*.inl)
	file(GLOB_RECURSE CORE_HEADER ../gli/core/*.hpp)

	file(GLOB_RECURSE GTX_SOURCE ../gli/gtx/*.cpp)
	file(GLOB_RECURSE GTX_INLINE ../gli/gtx/*.inl)
	file(GLOB_RECURSE GTX_HEADER ../gli/gtx/*.hpp)

	source_group("KMG Spec" FILES  ${ROOT_KMG})
	source_group("Text Files" FILES ${ROOT_TEXT} ${ROOT_MD})
	source_group("Core Files" FILES ${CORE_SOURCE})
	source_group("Core Files" FILES ${CORE_INLINE})
	source_group("Core Files" FILES ${CORE_HEADER})
	source_group("GTX Files" FILES ${GTX_SOURCE})
	source_group("GTX Files" FILES ${GTX_INLINE})
	source_group("GTX Files" FILES ${GTX_HEADER})

	set(SAMPLE_NAME test-${NAME})
	add_executable(${SAMPLE_NAME} ${NAME}.cpp)
	target_link_libraries(${SAMPLE_NAME} gli)
	add_test(
		NAME ${SAMPLE_NAME}
		COMMAND $<TARGET_FILE:${SAMPLE_NAME}> )
endfunction()

if(GLI_TEST_ENABLE)
	add_subdirectory(bug)
	add_subdirectory(core)
endif()
