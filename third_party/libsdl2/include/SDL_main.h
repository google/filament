/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SDL_main_h_
#define SDL_main_h_

#include "SDL_stdinc.h"

/**
 *  \file SDL_main.h
 *
 *  Redefine main() on some platforms so that it is called by SDL.
 */

#ifndef SDL_MAIN_HANDLED
#if defined(__WIN32__)
/* On Windows SDL provides WinMain(), which parses the command line and passes
   the arguments to your main function.

   If you provide your own WinMain(), you may define SDL_MAIN_HANDLED
 */
#define SDL_MAIN_AVAILABLE

#elif defined(__WINRT__)
/* On WinRT, SDL provides a main function that initializes CoreApplication,
   creating an instance of IFrameworkView in the process.

   Please note that #include'ing SDL_main.h is not enough to get a main()
   function working.  In non-XAML apps, the file,
   src/main/winrt/SDL_WinRT_main_NonXAML.cpp, or a copy of it, must be compiled
   into the app itself.  In XAML apps, the function, SDL_WinRTRunApp must be
   called, with a pointer to the Direct3D-hosted XAML control passed in.
*/
#define SDL_MAIN_NEEDED

#elif defined(__IPHONEOS__)
/* On iOS SDL provides a main function that creates an application delegate
   and starts the iOS application run loop.

   If you link with SDL dynamically on iOS, the main function can't be in a
   shared library, so you need to link with libSDLmain.a, which includes a
   stub main function that calls into the shared library to start execution.

   See src/video/uikit/SDL_uikitappdelegate.m for more details.
 */
#define SDL_MAIN_NEEDED

#elif defined(__ANDROID__)
/* On Android SDL provides a Java class in SDLActivity.java that is the
   main activity entry point.

   See docs/README-android.md for more details on extending that class.
 */
#define SDL_MAIN_NEEDED

/* We need to export SDL_main so it can be launched from Java */
#define SDLMAIN_DECLSPEC    DECLSPEC

#elif defined(__NACL__)
/* On NACL we use ppapi_simple to set up the application helper code,
   then wait for the first PSE_INSTANCE_DIDCHANGEVIEW event before 
   starting the user main function.
   All user code is run in a separate thread by ppapi_simple, thus 
   allowing for blocking io to take place via nacl_io
*/
#define SDL_MAIN_NEEDED

#endif
#endif /* SDL_MAIN_HANDLED */

#ifndef SDLMAIN_DECLSPEC
#define SDLMAIN_DECLSPEC
#endif

/**
 *  \file SDL_main.h
 *
 *  The application's main() function must be called with C linkage,
 *  and should be declared like this:
 *  \code
 *  #ifdef __cplusplus
 *  extern "C"
 *  #endif
 *  int main(int argc, char *argv[])
 *  {
 *  }
 *  \endcode
 */

#if defined(SDL_MAIN_NEEDED) || defined(SDL_MAIN_AVAILABLE)
#define main    SDL_main
#endif

#include "begin_code.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 *  The prototype for the application's main() function
 */
typedef int (*SDL_main_func)(int argc, char *argv[]);
extern SDLMAIN_DECLSPEC int SDL_main(int argc, char *argv[]);


/**
 * Circumvent failure of SDL_Init() when not using SDL_main() as an entry
 * point.
 *
 * This function is defined in SDL_main.h, along with the preprocessor rule to
 * redefine main() as SDL_main(). Thus to ensure that your main() function
 * will not be changed it is necessary to define SDL_MAIN_HANDLED before
 * including SDL.h.
 *
 * \sa SDL_Init
 */
extern DECLSPEC void SDLCALL SDL_SetMainReady(void);

#ifdef __WIN32__

/**
 * This can be called to set the application class at startup
 */
extern DECLSPEC int SDLCALL SDL_RegisterApp(char *name, Uint32 style, void *hInst);
extern DECLSPEC void SDLCALL SDL_UnregisterApp(void);

#endif /* __WIN32__ */


#ifdef __WINRT__

/**
 * Initialize and launch an SDL/WinRT application.
 *
 * \param mainFunction the SDL app's C-style main(), an SDL_main_func
 * \param reserved reserved for future use; should be NULL
 * \returns 0 on success or -1 on failure; call SDL_GetError() to retrieve
 *          more information on the failure.
 *
 * \since This function is available since SDL 2.0.3.
 */
extern DECLSPEC int SDLCALL SDL_WinRTRunApp(SDL_main_func mainFunction, void * reserved);

#endif /* __WINRT__ */

#if defined(__IPHONEOS__)

/**
 * Initializes and launches an SDL application.
 *
 * \param argc The argc parameter from the application's main() function
 * \param argv The argv parameter from the application's main() function
 * \param mainFunction The SDL app's C-style main(), an SDL_main_func
 * \return the return value from mainFunction
 */
extern DECLSPEC int SDLCALL SDL_UIKitRunApp(int argc, char *argv[], SDL_main_func mainFunction);

#endif /* __IPHONEOS__ */


#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* SDL_main_h_ */

/* vi: set ts=4 sw=4 expandtab: */
