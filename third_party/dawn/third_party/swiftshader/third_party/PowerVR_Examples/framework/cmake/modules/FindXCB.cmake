# - FindXCB
#
# Copyright (C) 2015 Valve Corporation

find_package(PkgConfig)

if(NOT XCB_FIND_COMPONENTS)
    set(XCB_FIND_COMPONENTS xcb)
endif()

include(FindPackageHandleStandardArgs)
set(XCB_FOUND true)
set(XCB_INCLUDE_DIRS "")
set(XCB_LIBRARIES "")

foreach(comp ${XCB_FIND_COMPONENTS})
    # component name
    string(TOUPPER ${comp} compname)
    # header name
    set(headername xcb/${comp}.h)
    # library name
    set(libname ${comp})

    pkg_check_modules(PKG_${compname} QUIET ${comp})

    find_path(${compname}_INCLUDE_DIR NAMES ${headername}
        HINTS
        ${PKG_${comp}_INCLUDEDIR}
        ${PKG_${comp}_INCLUDE_DIRS}
        )

    find_library(${compname}_LIBRARY NAMES ${libname}
        HINTS
        ${PKG_${comp}_LIBDIR}
        ${PKG_${comp}_LIBRARY_DIRS}
        )

    find_package_handle_standard_args(${comp}
        FOUND_VAR ${comp}_FOUND
        REQUIRED_VARS ${compname}_INCLUDE_DIR ${compname}_LIBRARY)
    mark_as_advanced(${compname}_INCLUDE_DIR ${compname}_LIBRARY)

    list(APPEND XCB_INCLUDE_DIRS ${${compname}_INCLUDE_DIR})
    list(APPEND XCB_LIBRARIES ${${compname}_LIBRARY})

    if(NOT ${comp}_FOUND)
        set(XCB_FOUND false)
    endif()
endforeach()

if(XCB_INCLUDE_DIRS)
	list(REMOVE_DUPLICATES XCB_INCLUDE_DIRS)
endif()