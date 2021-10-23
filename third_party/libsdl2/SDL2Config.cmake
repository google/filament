include("${CMAKE_CURRENT_LIST_DIR}/SDL2Targets.cmake")

# provide ${SDL2_LIBRARIES}, ${SDL2_INCLUDE_DIRS} etc, like sdl2-config.cmake does,
# for compatibility between SDL2 built with autotools and SDL2 built with CMake

# the following seems to work on Windows for both MSVC and MINGW+MSYS and with both SDL2Config/Target.cmake
# from vcpkg and from building myself with cmake from latest git
# AND on Linux when building SDL2 (tested current git) with CMake

# the headers are easy - but note that this adds both .../include/ and .../include/SDL2/
# while the SDL2_INCLUDE_DIRS of sdl2-config.cmake only add ...include/SDL2/
# But at least if building worked with sdl2-config.cmake it will also work with this.
get_target_property(SDL2_INCLUDE_DIRS SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)

# get the paths to the files to link against (.lib or .dll.a on Windows, .so or .a on Unix, ...) for both SDL2 and SDL2main

# for the "normal"/release build they could be in lots of different properties..
set(relprops IMPORTED_IMPLIB_RELEASE IMPORTED_IMPLIB_NOCONFIG IMPORTED_IMPLIB IMPORTED_IMPLIB_MINSIZEREL IMPORTED_IMPLIB_RELWITHDEBINFO
             IMPORTED_LOCATION_RELEASE IMPORTED_LOCATION_NOCONFIG IMPORTED_LOCATION IMPORTED_LOCATION_MINSIZEREL IMPORTED_LOCATION_RELWITHDEBINFO)

# fewer possibilities for debug builds
set(dbgprops IMPORTED_IMPLIB_DEBUG IMPORTED_LOCATION_DEBUG)

foreach(prop ${relprops})
	get_target_property(sdl2implib SDL2::SDL2 ${prop})
	if(sdl2implib)
		#message("set sdl2implib from ${prop}")
		break()
	endif()
endforeach()

foreach(prop ${relprops})
	get_target_property(sdl2mainimplib SDL2::SDL2main ${prop})
	if(sdl2mainimplib)
		#message("set sdl2mainimplib from ${prop}")
		break()
	endif()
endforeach()

foreach(prop ${dbgprops})
	get_target_property(sdl2implibdbg SDL2::SDL2 ${prop})
	if(sdl2implibdbg)
		#message("set sdl2implibdbg from ${prop}")
		break()
	endif()
endforeach()

foreach(prop ${dbgprops})
	get_target_property(sdl2mainimplibdbg SDL2::SDL2main ${prop})
	if(sdl2mainimplibdbg)
		#message("set sdl2mainimplibdbg from ${prop}")
		break()
	endif()
endforeach()

if( sdl2implib AND sdl2mainimplib AND sdl2implibdbg AND sdl2mainimplibdbg )
	# we have both release and debug builds of SDL2 and SDL2main, so use this ugly
	# generator expression in SDL2_LIBRARIES to support both in MSVC, depending on build type configured there
	set(SDL2_LIBRARIES $<IF:$<CONFIG:Debug>,${sdl2mainimplibdbg},${sdl2mainimplib}>   $<IF:$<CONFIG:Debug>,${sdl2implibdbg},${sdl2implib}>)
else()
	if( (NOT sdl2implib) AND sdl2implibdbg ) # if we only have a debug version of the lib
		set(sdl2implib sdl2implibdbg)
	endif()
	if( (NOT sdl2mainimplib) AND sdl2mainimplibdbg ) # if we only have a debug version of the lib
		set(sdl2mainimplib sdl2mainimplibdbg)
	endif()

	if( sdl2implib AND sdl2mainimplib )
		set(SDL2_LIBRARIES ${sdl2mainimplib}  ${sdl2implib})
	elseif(WIN32 OR APPLE) # I think these platforms have a non-dummy SDLmain?
		message(FATAL_ERROR, "SDL2::SDL2 and/or SDL2::SDL2main don't seem to contain any kind of IMPORTED_IMPLIB* or IMPORTED_LOCATION*")
	elseif(sdl2implib) # on other platforms just libSDL2 will hopefully do?
		set(SDL2_LIBRARIES ${sdl2implib})
		message(STATUS, "No SDL2main lib not found, I hope you don't need it..")
	else()
		message(FATAL_ERROR, "SDL2::SDL2 doesn't seem to contain any kind of lib to link against in IMPORTED_IMPLIB* or IMPORTED_LOCATION*")
	endif()

	# TODO: should something like INTERFACE_LINK_LIBRARIES be appended? or wherever -mwindows and things like that
	#       might be defined (if they were defined by the CMake build at all; autotools has @SDL_RLD_FLAGS@ @SDL_LIBS@)?
	#       LINK_DEPENDS? LINK_FLAGS?

endif()

get_filename_component(SDL2_LIBDIR ${sdl2implib} PATH)

# NOTE: SDL2_LIBRARIES now looks like "c:/path/to/SDL2main.lib;c:/path/to/SDL2.lib"
#       which is different to what it looks like when coming from sdl2-config.cmake
#       (there it's more like "-L${SDL2_LIBDIR} -lSDL2main -lSDL2" - and also -lmingw32 and -mwindows)
#       This seems to work with both MSVC and MinGW though, while the other only worked with MinGW
#   On Linux it looks like "/tmp/sdl2inst/lib/libSDL2main.a;/tmp/sdl2inst/lib/libSDL2-2.0.so.0.14.1" which also seems to work

# the exec prefix is one level up from lib/ - TODO: really, always? at least on Linux there's /usr/lib/x86_64-bla-blub/libSDL2-asdf.so.0 ..
get_filename_component(SDL2_EXEC_PREFIX ${SDL2_LIBDIR} PATH)
set(SDL2_PREFIX ${SDL2_EXEC_PREFIX}) # TODO: could this be somewhere else? parent dir of include or sth?

unset(sdl2implib)
unset(sdl2mainimplib)
unset(sdl2implibdbg)
unset(sdl2mainimplibdbg)
unset(relprops)
unset(dbgprops)
