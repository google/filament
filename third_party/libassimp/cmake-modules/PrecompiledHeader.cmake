MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CFG_INTDIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
      PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
      OBJECT_OUTPUTS "${PrecompiledBinary}")

    # Do not consider .c files
    foreach(fname ${Sources}) 
      GET_FILENAME_COMPONENT(fext ${fname} EXT)
      if(fext STREQUAL ".cpp")
	SET_SOURCE_FILES_PROPERTIES(${fname}
	  PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledBinary}\" /FI\"${PrecompiledBinary}\" /Fp\"${PrecompiledBinary}\""
          OBJECT_DEPENDS "${PrecompiledBinary}")     
      endif(fext STREQUAL ".cpp")
    endforeach(fname) 

  ENDIF(MSVC)
  # Add precompiled header to SourcesVar
  LIST(APPEND ${SourcesVar} ${PrecompiledSource})

ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)
