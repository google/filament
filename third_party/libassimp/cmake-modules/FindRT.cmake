# Try to find real time libraries
# Once done, this will define
#
# RT_FOUND - system has rt library
# RT_LIBRARIES - rt libraries directory
#
# Source: https://gitlab.cern.ch/dss/eos/commit/44070e575faaa46bd998708ef03eedb381506ff0
#

if(RT_LIBRARIES)
    set(RT_FIND_QUIETLY TRUE)
endif(RT_LIBRARIES)

find_library(RT_LIBRARY rt)
set(RT_LIBRARIES ${RT_LIBRARY})
# handle the QUIETLY and REQUIRED arguments and set
# RT_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(rt DEFAULT_MSG RT_LIBRARY)
mark_as_advanced(RT_LIBRARY)
