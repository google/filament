#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

##################################################################
# Provides some common functionality for the FindPackage modules
##################################################################

# Begin processing of package
macro(findpkg_begin PREFIX)
  if (NOT ${PREFIX}_FIND_QUIETLY)
    message(STATUS "Looking for ${PREFIX}...")
  endif ()
endmacro(findpkg_begin)

# Display a status message unless FIND_QUIETLY is set
macro(pkg_message PREFIX)
  if (NOT ${PREFIX}_FIND_QUIETLY)
    message(STATUS ${ARGN})
  endif ()
endmacro(pkg_message)

# Get environment variable, define it as ENV_$var and make sure backslashes are converted to forward slashes
macro(getenv_path VAR)
   set(ENV_${VAR} $ENV{${VAR}})
   # replace won't work if var is blank
   if (ENV_${VAR})
     string( REGEX REPLACE "\\\\" "/" ENV_${VAR} ${ENV_${VAR}} )
   endif ()
endmacro(getenv_path)

# Construct search paths for includes and libraries from a PREFIX_PATH
macro(create_search_paths PREFIX)
  foreach(dir ${${PREFIX}_PREFIX_PATH})
    set(${PREFIX}_INC_SEARCH_PATH ${${PREFIX}_INC_SEARCH_PATH}
      ${dir}/include ${dir}/include/${PREFIX} ${dir}/Headers)
    set(${PREFIX}_LIB_SEARCH_PATH ${${PREFIX}_LIB_SEARCH_PATH}
      ${dir}/lib ${dir}/lib/${PREFIX} ${dir}/Libs)
  endforeach(dir)
  set(${PREFIX}_FRAMEWORK_SEARCH_PATH ${${PREFIX}_PREFIX_PATH})
endmacro(create_search_paths)

# clear cache variables if a certain variable changed
macro(clear_if_changed TESTVAR)
  # test against internal check variable
  if (NOT "${${TESTVAR}}" STREQUAL "${${TESTVAR}_INT_CHECK}")
    message(STATUS "${TESTVAR} changed.")
    foreach(var ${ARGN})
      set(${var} "NOTFOUND" CACHE STRING "x" FORCE)
    endforeach(var)
  endif ()
  set(${TESTVAR}_INT_CHECK ${${TESTVAR}} CACHE INTERNAL "x" FORCE)
endmacro(clear_if_changed)

# Try to get some hints from pkg-config, if available
macro(use_pkgconfig PREFIX PKGNAME)
  # Android does not support PKG_CONFIG so we disable it
  IF ( NOT ANDROID )
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(${PREFIX} ${PKGNAME})
    endif ()
  ENDIF ( NOT ANDROID )
endmacro (use_pkgconfig)

# Couple a set of release AND debug libraries (or frameworks)
macro(make_library_set PREFIX)
  if (${PREFIX}_FWK)
    set(${PREFIX} ${${PREFIX}_FWK})
  elseif (${PREFIX}_REL AND ${PREFIX}_DBG)
    set(${PREFIX} optimized ${${PREFIX}_REL} debug ${${PREFIX}_DBG})
  elseif (${PREFIX}_REL)
    set(${PREFIX} ${${PREFIX}_REL})
  elseif (${PREFIX}_DBG)
    set(${PREFIX} ${${PREFIX}_DBG})
  endif ()
endmacro(make_library_set)

# Generate debug names from given release names
macro(get_debug_names PREFIX)
  foreach(i ${${PREFIX}})
    set(${PREFIX}_DBG ${${PREFIX}_DBG} ${i}d ${i}D ${i}_d ${i}_D ${i}_debug ${i})
  endforeach(i)
endmacro(get_debug_names)

# Add the parent dir from DIR to VAR
macro(add_parent_dir VAR DIR)
  get_filename_component(${DIR}_TEMP "${${DIR}}/.." ABSOLUTE)
  set(${VAR} ${${VAR}} ${${DIR}_TEMP})
endmacro(add_parent_dir)

# Do the final processing for the package find.
macro(findpkg_finish PREFIX)
  # skip if already processed during this run
  if (NOT ${PREFIX}_FOUND)
    if (${PREFIX}_INCLUDE_DIR AND ${PREFIX}_LIBRARY)
      set(${PREFIX}_FOUND TRUE)
      set(${PREFIX}_INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIR})
      set(${PREFIX}_LIBRARIES ${${PREFIX}_LIBRARY})
      if (NOT ${PREFIX}_FIND_QUIETLY)
        message(STATUS "Found ${PREFIX}: ${${PREFIX}_LIBRARIES}")
      endif ()
    else ()
      if (NOT ${PREFIX}_FIND_QUIETLY)
        message(STATUS "Could not locate ${PREFIX}")
      endif ()
      if (${PREFIX}_FIND_REQUIRED)
        message(FATAL_ERROR "Required library ${PREFIX} not found! Install the library (including dev packages) and try again. If the library is already installed, set the missing variables manually in cmake.")
      endif ()
    endif ()

    mark_as_advanced(${PREFIX}_INCLUDE_DIR ${PREFIX}_LIBRARY ${PREFIX}_LIBRARY_REL ${PREFIX}_LIBRARY_DBG ${PREFIX}_LIBRARY_FWK)
  endif ()
endmacro(findpkg_finish)


# Slightly customised framework finder
MACRO(findpkg_framework fwk)
  IF(APPLE)
    SET(${fwk}_FRAMEWORK_PATH
      ${${fwk}_FRAMEWORK_SEARCH_PATH}
      ${CMAKE_FRAMEWORK_PATH}
      ~/Library/Frameworks
      /Library/Frameworks
      /System/Library/Frameworks
      /Network/Library/Frameworks
      /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS3.0.sdk/System/Library/Frameworks/
      /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS3.0.sdk/System/Library/Frameworks/
    )
    FOREACH(dir ${${fwk}_FRAMEWORK_PATH})
      SET(fwkpath ${dir}/${fwk}.framework)
      IF(EXISTS ${fwkpath})
        SET(${fwk}_FRAMEWORK_INCLUDES ${${fwk}_FRAMEWORK_INCLUDES}
          ${fwkpath}/Headers ${fwkpath}/PrivateHeaders)
        if (NOT ${fwk}_LIBRARY_FWK)
          SET(${fwk}_LIBRARY_FWK "-framework ${fwk}")
        endif ()
      ENDIF(EXISTS ${fwkpath})
    ENDFOREACH(dir)
  ENDIF(APPLE)
ENDMACRO(findpkg_framework)
