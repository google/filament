# - Find X11_XCB
#
# Copyright (C) 2015 Valve Corporation

find_package(PkgConfig)

if(NOT X11_XCB_FIND_COMPONENTS)
    set(X11_XCB_FIND_COMPONENTS X11-xcb)
endif()

include(FindPackageHandleStandardArgs)
set(X11_XCB_FOUND true)
set(X11_XCB_LIBRARIES "")

foreach(comp ${X11_XCB_FIND_COMPONENTS})
    # component name
    string(TOUPPER ${comp} compname)
    string(REPLACE "-" "_" compname ${compname})
    # library name
    set(libname ${comp})

    pkg_check_modules(PKG_${compname} QUIET ${comp})

    find_library(${compname}_LIBRARY NAMES ${libname}
        HINTS
        ${PKG_${comp}_LIBDIR}
        ${PKG_${comp}_LIBRARY_DIRS}
        )

    find_package_handle_standard_args(${comp}
        FOUND_VAR ${comp}_FOUND
        REQUIRED_VARS ${compname}_LIBRARY)
    mark_as_advanced(${compname}_LIBRARY)

    list(APPEND X11_XCB_LIBRARIES ${${compname}_LIBRARY})
	
    if(NOT ${comp}_FOUND)
        set(X11_XCB_FOUND false)
    endif()
endforeach()