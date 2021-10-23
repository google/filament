WinRT
=====

This port allows SDL applications to run on Microsoft's platforms that require
use of "Windows Runtime", aka. "WinRT", APIs.  Microsoft may, in some cases,
refer to them as either "Windows Store", or for Windows 10, "UWP" apps.

Some of the operating systems that include WinRT, are:

* Windows 10, via its Universal Windows Platform (UWP) APIs
* Windows 8.x
* Windows RT 8.x (aka. Windows 8.x for ARM processors)
* Windows Phone 8.x


Requirements
------------

* Microsoft Visual C++ (aka Visual Studio), either 2017, 2015, 2013, or 2012
  - Free, "Community" or "Express" editions may be used, so long as they
    include  support for either "Windows Store" or "Windows Phone" apps.
    "Express" versions marked as supporting "Windows Desktop" development
    typically do not include support for creating WinRT apps, to note.
    (The "Community" editions of Visual C++ do, however, support both
    desktop/Win32 and WinRT development).
  - Visual Studio 2017 can be used, however it is recommended that you install
    the Visual C++ 2015 build tools.  These build tools can be installed
    using VS 2017's installer.  Be sure to also install the workload for
    "Universal Windows Platform development", its optional component, the
    "C++ Universal Windows Platform tools", and for UWP / Windows 10
    development, the "Windows 10 SDK (10.0.10240.0)".  Please note that
    targeting UWP / Windows 10 apps from development machine(s) running
    earlier versions of Windows, such as Windows 7, is not always supported
    by Visual Studio, and you may get error(s) when attempting to do so.
  - Visual C++ 2012 can only build apps that target versions 8.0 of Windows,
    or  Windows Phone.  8.0-targeted apps will run on devices running 8.1
    editions of Windows, however they will not be able to take advantage of
    8.1-specific features.
  - Visual C++ 2013 cannot create app projects that target Windows 8.0.
    Visual C++ 2013 Update 4, can create app projects for Windows Phone 8.0,
    Windows Phone 8.1, and Windows 8.1, but not Windows 8.0.  An optional
    Visual Studio add-in, "Tools for Maintaining Store apps for Windows 8",
    allows Visual C++ 2013 to load and build Windows 8.0 projects that were
    created with Visual C++ 2012, so long as Visual C++ 2012 is installed
    on the same machine.  More details on targeting different versions of
    Windows can found at the following web pages:
      - [Develop apps by using Visual Studio 2013](http://msdn.microsoft.com/en-us/library/windows/apps/br211384.aspx)
      - [To add the Tools for Maintaining Store apps for Windows 8](http://msdn.microsoft.com/en-us/library/windows/apps/dn263114.aspx#AddMaintenanceTools)
* A valid Microsoft account - This requirement is not imposed by SDL, but
  rather by Microsoft's Visual C++ toolchain.  This is required to launch or 
  debug apps.


Status
------

Here is a rough list of what works, and what doesn't:

* What works:
  * compilation via Visual C++ 2012 through 2015
  * compile-time platform detection for SDL programs.  The C/C++ #define,
    `__WINRT__`, will be set to 1 (by SDL) when compiling for WinRT.
  * GPU-accelerated 2D rendering, via SDL_Renderer.
  * OpenGL ES 2, via the ANGLE library (included separately from SDL)
  * software rendering, via either SDL_Surface (optionally in conjunction with
    SDL_GetWindowSurface() and SDL_UpdateWindowSurface()) or via the
    SDL_Renderer APIs
  * threads
  * timers (via SDL_GetTicks(), SDL_AddTimer(), SDL_GetPerformanceCounter(),
    SDL_GetPerformanceFrequency(), etc.)
  * file I/O via SDL_RWops
  * mouse input  (unsupported on Windows Phone)
  * audio, via SDL's WASAPI backend (if you want to record, your app must 
    have "Microphone" capabilities enabled in its manifest, and the user must 
    not have blocked access. Otherwise, capture devices will fail to work,
    presenting as a device disconnect shortly after opening it.)
  * .DLL file loading.  Libraries *MUST* be packaged inside applications.  Loading
    anything outside of the app is not supported.
  * system path retrieval via SDL's filesystem APIs
  * game controllers.  Support is provided via the SDL_Joystick and
    SDL_GameController APIs, and is backed by Microsoft's XInput API.  Please
    note, however, that Windows limits game-controller support in UWP apps to,
    "Xbox compatible controllers" (many controllers that work in Win32 apps,
    do not work in UWP, due to restrictions in UWP itself.) 
  * multi-touch input
  * app events.  SDL_APP_WILLENTER* and SDL_APP_DIDENTER* events get sent out as
    appropriate.
  * window events
  * using Direct3D 11.x APIs outside of SDL.  Non-XAML / Direct3D-only apps can
    choose to render content directly via Direct3D, using SDL to manage the
    internal WinRT window, as well as input and audio.  (Use
    SDL_GetWindowWMInfo() to get the WinRT 'CoreWindow', and pass it into
    IDXGIFactory2::CreateSwapChainForCoreWindow() as appropriate.)

* What partially works:
  * keyboard input.  Most of WinRT's documented virtual keys are supported, as
    well as many keys with documented hardware scancodes.  Converting
    SDL_Scancodes to or from SDL_Keycodes may not work, due to missing APIs
    (MapVirtualKey()) in Microsoft's Windows Store / UWP APIs.
  * SDLmain.  WinRT uses a different signature for each app's main() function.
    SDL-based apps that use this port must compile in SDL_winrt_main_NonXAML.cpp
    (in `SDL\src\main\winrt\`) directly in order for their C-style main()
    functions to be called.

* What doesn't work:
  * compilation with anything other than Visual C++
  * programmatically-created custom cursors.  These don't appear to be supported
    by WinRT.  Different OS-provided cursors can, however, be created via
    SDL_CreateSystemCursor() (unsupported on Windows Phone)
  * SDL_WarpMouseInWindow() or SDL_WarpMouseGlobal().  This are not currently
    supported by WinRT itself.
  * joysticks and game controllers that either are not supported by
    Microsoft's XInput API, or are not supported within UWP apps (many
    controllers that work in Win32, do not work in UWP, due to restrictions in
    UWP itself).
  * turning off VSync when rendering on Windows Phone.  Attempts to turn VSync
    off on Windows Phone result either in Direct3D not drawing anything, or it
    forcing VSync back on.  As such, SDL_RENDERER_PRESENTVSYNC will always get
    turned-on on Windows Phone.  This limitation is not present in non-Phone
    WinRT (such as Windows 8.x), where turning off VSync appears to work.
  * probably anything else that's not listed as supported



Upgrade Notes
-------------

#### SDL_GetPrefPath() usage when upgrading WinRT apps from SDL 2.0.3

SDL 2.0.4 fixes two bugs found in the WinRT version of SDL_GetPrefPath().
The fixes may affect older, SDL 2.0.3-based apps' save data.  Please note
that these changes only apply to SDL-based WinRT apps, and not to apps for
any other platform.

1. SDL_GetPrefPath() would return an invalid path, one in which the path's
   directory had not been created.  Attempts to create files there
   (via fopen(), for example), would fail, unless that directory was
   explicitly created beforehand.

2. SDL_GetPrefPath(), for non-WinPhone-based apps, would return a path inside
   a WinRT 'Roaming' folder, the contents of which get automatically
   synchronized across multiple devices.  This process can occur while an
   application runs, and can cause existing save-data to be overwritten
   at unexpected times, with data from other devices.  (Windows Phone apps
   written with SDL 2.0.3 did not utilize a Roaming folder, due to API
   restrictions in Windows Phone 8.0).


SDL_GetPrefPath(), starting with SDL 2.0.4, addresses these by:

1. making sure that SDL_GetPrefPath() returns a directory in which data
   can be written to immediately, without first needing to create directories.

2. basing SDL_GetPrefPath() off of a different, non-Roaming folder, the
   contents of which do not automatically get synchronized across devices
   (and which require less work to use safely, in terms of data integrity).

Apps that wish to get their Roaming folder's path can do so either by using
SDL_WinRTGetFSPathUTF8(), SDL_WinRTGetFSPathUNICODE() (which returns a
UCS-2/wide-char string), or directly through the WinRT class,
Windows.Storage.ApplicationData.



Setup, High-Level Steps
-----------------------

The steps for setting up a project for an SDL/WinRT app looks like the
following, at a high-level:

1. create a new Visual C++ project using Microsoft's template for a,
   "Direct3D App".
2. remove most of the files from the project.
3. make your app's project directly reference SDL/WinRT's own Visual C++
   project file, via use of Visual C++'s "References" dialog.  This will setup
   the linker, and will copy SDL's .dll files to your app's final output.
4. adjust your app's build settings, at minimum, telling it where to find SDL's
   header files.
5. add files that contains a WinRT-appropriate main function, along with some
   data to make sure mouse-cursor-hiding (via SDL_ShowCursor(SDL_DISABLE) calls)
   work properly.
6. add SDL-specific app code.
7. build and run your app.


Setup, Detailed Steps
---------------------

### 1. Create a new project ###

Create a new project using one of Visual C++'s templates for a plain, non-XAML,
"Direct3D App" (XAML support for SDL/WinRT is not yet ready for use).  If you
don't see one of these templates, in Visual C++'s 'New Project' dialog, try
using the textbox titled, 'Search Installed Templates' to look for one.


### 2. Remove unneeded files from the project ###

In the new project, delete any file that has one of the following extensions:

- .cpp
- .h
- .hlsl

When you are done, you should be left with a few files, each of which will be a
necessary part of your app's project.  These files will consist of:

- an .appxmanifest file, which contains metadata on your WinRT app.  This is
  similar to an Info.plist file on iOS, or an AndroidManifest.xml on Android.
- a few .png files, one of which is a splash screen (displayed when your app
  launches), others are app icons.
- a .pfx file, used for code signing purposes.


### 3. Add references to SDL's project files ###

SDL/WinRT can be built in multiple variations, spanning across three different
CPU architectures (x86, x64, and ARM) and two different configurations
(Debug and Release).  WinRT and Visual C++ do not currently provide a means
for combining multiple variations of one library into a single file.
Furthermore, it does not provide an easy means for copying pre-built .dll files
into your app's final output (via Post-Build steps, for example).  It does,
however, provide a system whereby an app can reference the MSVC projects of
libraries such that, when the app is built:

1. each library gets built for the appropriate CPU architecture(s) and WinRT
   platform(s).
2. each library's output, such as .dll files, get copied to the app's build 
   output.

To set this up for SDL/WinRT, you'll need to run through the following steps:

1. open up the Solution Explorer inside Visual C++ (under the "View" menu, then
   "Solution Explorer")
2. right click on your app's solution.
3. navigate to "Add", then to "Existing Project..."
4. find SDL/WinRT's Visual C++ project file and open it.  Different project
   files exist for different WinRT platforms.  All of them are in SDL's
   source distribution, in the following directories:
    * `VisualC-WinRT/UWP_VS2015/`        - for Windows 10 / UWP apps
    * `VisualC-WinRT/WinPhone81_VS2013/` - for Windows Phone 8.1 apps
    * `VisualC-WinRT/WinRT80_VS2012/`    - for Windows 8.0 apps
    * `VisualC-WinRT/WinRT81_VS2013/`    - for Windows 8.1 apps
5. once the project has been added, right-click on your app's project and
   select, "References..."
6. click on the button titled, "Add New Reference..."
7. check the box next to SDL
8. click OK to close the dialog
9. SDL will now show up in the list of references.  Click OK to close that
   dialog.

Your project is now linked to SDL's project, insofar that when the app is
built, SDL will be built as well, with its build output getting included with
your app.


### 4. Adjust Your App's Build Settings ###

Some build settings need to be changed in your app's project.  This guide will
outline the following:

- making sure that the compiler knows where to find SDL's header files
- **Optional for C++, but NECESSARY for compiling C code:** telling the
  compiler not to use Microsoft's C++ extensions for WinRT development.
- **Optional:** telling the compiler not generate errors due to missing
  precompiled header files.

To change these settings:

1. right-click on the project
2. choose "Properties"
3. in the drop-down box next to "Configuration", choose, "All Configurations"
4. in the drop-down box next to "Platform", choose, "All Platforms"
5. in the left-hand list, expand the "C/C++" section
6. select "General"
7. edit the "Additional Include Directories" setting, and add a path to SDL's
   "include" directory
8. **Optional: to enable compilation of C code:** change the setting for
   "Consume Windows Runtime Extension" from "Yes (/ZW)" to "No".  If you're 
   working with a completely C++ based project, this step can usually be 
   omitted.
9. **Optional: to disable precompiled headers (which can produce 
   'stdafx.h'-related build errors, if setup incorrectly:** in the left-hand 
   list, select "Precompiled Headers", then change the setting for "Precompiled 
   Header" from "Use (/Yu)" to "Not Using Precompiled Headers".
10. close the dialog, saving settings, by clicking the "OK" button


### 5. Add a WinRT-appropriate main function, and a blank-cursor image, to the app. ###

A few files should be included directly in your app's MSVC project, specifically:
1. a WinRT-appropriate main function (which is different than main() functions on
   other platforms)
2. a Win32-style cursor resource, used by SDL_ShowCursor() to hide the mouse cursor
   (if and when the app needs to do so).  *If this cursor resource is not
   included, mouse-position reporting may fail if and when the cursor is
   hidden, due to possible bugs/design-oddities in Windows itself.*

To include these files for C/C++ projects:

1. right-click on your project (again, in Visual C++'s Solution Explorer), 
   navigate to "Add", then choose "Existing Item...".
2. navigate to the directory containing SDL's source code, then into its
   subdirectory, 'src/main/winrt/'.  Select, then add, the following files:
   - `SDL_winrt_main_NonXAML.cpp`
   - `SDL2-WinRTResources.rc`
   - `SDL2-WinRTResource_BlankCursor.cur`
3. right-click on the file `SDL_winrt_main_NonXAML.cpp` (as listed in your
   project), then click on "Properties...".
4. in the drop-down box next to "Configuration", choose, "All Configurations"
5. in the drop-down box next to "Platform", choose, "All Platforms"
6. in the left-hand list, click on "C/C++"
7. change the setting for "Consume Windows Runtime Extension" to "Yes (/ZW)".
8. click the OK button.  This will close the dialog.

**NOTE: C++/CX compilation is currently required in at least one file of your 
app's project.  This is to make sure that Visual C++'s linker builds a 'Windows 
Metadata' file (.winmd) for your app.  Not doing so can lead to build errors.**

For non-C++ projects, you will need to call SDL_WinRTRunApp from your language's
main function, and generate SDL2-WinRTResources.res manually by using `rc` via
the Developer Command Prompt and including it as a <Win32Resource> within the
first <PropertyGroup> block in your Visual Studio project file.

### 6. Add app code and assets ###

At this point, you can add in SDL-specific source code.  Be sure to include a 
C-style main function (ie: `int main(int argc, char *argv[])`).  From there you 
should be able to create a single `SDL_Window` (WinRT apps can only have one 
window, at present), as well as an `SDL_Renderer`.  Direct3D will be used to 
draw content.  Events are received via SDL's usual event functions 
(`SDL_PollEvent`, etc.)  If you have a set of existing source files and assets, 
you can start adding them to the project now.  If not, or if you would like to 
make sure that you're setup correctly, some short and simple sample code is 
provided below.


#### 6.A. ... when creating a new app ####

If you are creating a new app (rather than porting an existing SDL-based app), 
or if you would just like a simple app to test SDL/WinRT with before trying to 
get existing code working, some working SDL/WinRT code is provided below.  To 
set this up:

1. right click on your app's project
2. select Add, then New Item.  An "Add New Item" dialog will show up.
3. from the left-hand list, choose "Visual C++"
4. from the middle/main list, choose "C++ File (.cpp)"
5. near the bottom of the dialog, next to "Name:", type in a name for your 
source file, such as, "main.cpp".
6. click on the Add button.  This will close the dialog, add the new file to 
your project, and open the file in Visual C++'s text editor.
7. Copy and paste the following code into the new file, then save it.

```c
#include <SDL.h>
    
int main(int argc, char **argv)
{
    SDL_DisplayMode mode;
    SDL_Window * window = NULL;
    SDL_Renderer * renderer = NULL;
    SDL_Event evt;
    SDL_bool keep_going = SDL_TRUE;
  
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    } else if (SDL_GetCurrentDisplayMode(0, &mode) != 0) {
        return 1;
    } else if (SDL_CreateWindowAndRenderer(mode.w, mode.h, SDL_WINDOW_FULLSCREEN, &window, &renderer) != 0) {
        return 1;
    }
    
    while (keep_going) {
        while (SDL_PollEvent(&evt)) {
            if ((evt.type == SDL_KEYDOWN) && (evt.key.keysym.sym == SDLK_ESCAPE)) {
                keep_going = SDL_FALSE;
            } 
        }
    
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_Quit();
    return 0;
}
```

#### 6.B. Adding code and assets ####

If you have existing code and assets that you'd like to add, you should be able 
to add them now.  The process for adding a set of files is as such.

1. right click on the app's project
2. select Add, then click on "New Item..."
3. open any source, header, or asset files as appropriate.  Support for C and 
C++ is available.

Do note that WinRT only supports a subset of the APIs that are available to 
Win32-based apps.  Many portions of the Win32 API and the C runtime are not 
available.

A list of unsupported C APIs can be found at 
<http://msdn.microsoft.com/en-us/library/windows/apps/jj606124.aspx>

General information on using the C runtime in WinRT can be found at 
<https://msdn.microsoft.com/en-us/library/hh972425.aspx>

A list of supported Win32 APIs for WinRT apps can be found at 
<http://msdn.microsoft.com/en-us/library/windows/apps/br205757.aspx>.  To note, 
the list of supported Win32 APIs for Windows Phone 8.0 is different.  
That list can be found at 
<http://msdn.microsoft.com/en-us/library/windowsphone/develop/jj662956(v=vs.105).aspx>


### 7. Build and run your app ###

Your app project should now be setup, and you should be ready to build your app.  
To run it on the local machine, open the Debug menu and choose "Start 
Debugging".  This will build your app, then run your app full-screen.  To switch 
out of your app, press the Windows key.  Alternatively, you can choose to run 
your app in a window.  To do this, before building and running your app, find 
the drop-down menu in Visual C++'s toolbar that says, "Local Machine".  Expand 
this by clicking on the arrow on the right side of the list, then click on 
Simulator.  Once you do that, any time you build and run the app, the app will 
launch in window, rather than full-screen.


#### 7.A. Running apps on older, ARM-based, "Windows RT" devices ####

**These instructions do not include Windows Phone, despite Windows Phone
typically running on ARM processors.**  They are specifically for devices
that use the "Windows RT" operating system, which was a modified version of
Windows 8.x that ran primarily on ARM-based tablet computers.

To build and run the app on ARM-based, "Windows RT" devices, you'll need to:

- install Microsoft's "Remote Debugger" on the device.  Visual C++ installs and 
  debugs ARM-based apps via IP networks.
- change a few options on the development machine, both to make sure it builds 
  for ARM (rather than x86 or x64), and to make sure it knows how to find the 
  Windows RT device (on the network).

Microsoft's Remote Debugger can be found at 
<https://msdn.microsoft.com/en-us/library/hh441469.aspx>.  Please note 
that separate versions of this debugger exist for different versions of Visual 
C++, one each for MSVC 2015, 2013, and 2012.

To setup Visual C++ to launch your app on an ARM device:

1. make sure the Remote Debugger is running on your ARM device, and that it's on 
   the same IP network as your development machine.
2. from Visual C++'s toolbar, find a drop-down menu that says, "Win32".  Click 
   it, then change the value to "ARM".
3. make sure Visual C++ knows the hostname or IP address of the ARM device.  To 
   do this:
    1. open the app project's properties
    2. select "Debugging"
    3. next to "Machine Name", enter the hostname or IP address of the ARM 
       device
    4. if, and only if, you've turned off authentication in the Remote Debugger,
       then change the setting for "Require Authentication" to No
    5. click "OK"
4. build and run the app (from Visual C++).  The first time you do this, a 
   prompt will show up on the ARM device, asking for a Microsoft Account.  You 
   do, unfortunately, need to log in here, and will need to follow the 
   subsequent registration steps in order to launch the app.  After you do so, 
   if the app didn't already launch, try relaunching it again from within Visual 
   C++.


Troubleshooting
---------------

#### Build fails with message, "error LNK2038: mismatch detected for 'vccorlib_lib_should_be_specified_before_msvcrt_lib_to_linker'"

Try adding the following to your linker flags.  In MSVC, this can be done by
right-clicking on the app project, navigating to Configuration Properties ->
Linker -> Command Line, then adding them to the Additional Options
section.

* For Release builds / MSVC-Configurations, add:

    /nodefaultlib:vccorlib /nodefaultlib:msvcrt vccorlib.lib msvcrt.lib

* For Debug builds / MSVC-Configurations, add:

    /nodefaultlib:vccorlibd /nodefaultlib:msvcrtd vccorlibd.lib msvcrtd.lib


#### Mouse-motion events fail to get sent, or SDL_GetMouseState() fails to return updated values

This may be caused by a bug in Windows itself, whereby hiding the mouse
cursor can cause mouse-position reporting to fail.

SDL provides a workaround for this, but it requires that an app links to a
set of Win32-style cursor image-resource files.  A copy of suitable resource
files can be found in `src/main/winrt/`.  Adding them to an app's Visual C++
project file should be sufficient to get the app to use them.


#### SDL's Visual Studio project file fails to open, with message, "The system can't find the file specified."

This can be caused for any one of a few reasons, which Visual Studio can
report, but won't always do so in an up-front manner.

To help determine why this error comes up:

1. open a copy of Visual Studio without opening a project file.  This can be
   accomplished via Windows' Start Menu, among other means.
2. show Visual Studio's Output window.  This can be done by going to VS'
   menu bar, then to View, and then to Output.
3. try opening the SDL project file directly by going to VS' menu bar, then
   to File, then to Open, then to Project/Solution.  When a File-Open dialog
   appears, open the SDL project (such as the one in SDL's source code, in its
   directory, VisualC-WinRT/UWP_VS2015/).
4. after attempting to open SDL's Visual Studio project file, additional error
   information will be output to the Output window.

If Visual Studio reports (via its Output window) that the project:

"could not be loaded because it's missing install components. To fix this launch Visual Studio setup with the following selections:
Microsoft.VisualStudio.ComponentGroup.UWP.VC"

... then you will need to re-launch Visual Studio's installer, and make sure that
the workflow for "Universal Windows Platform development" is checked, and that its
optional component, "C++ Universal Windows Platform tools" is also checked.  While
you are there, if you are planning on targeting UWP / Windows 10, also make sure
that you check the optional component, "Windows 10 SDK (10.0.10240.0)".  After
making sure these items are checked as-appropriate, install them.

Once you install these components, try re-launching Visual Studio, and re-opening
the SDL project file.  If you still get the error dialog, try using the Output
window, again, seeing what Visual Studio says about it.


#### Game controllers / joysticks aren't working!

Windows only permits certain game controllers and joysticks to work within
WinRT / UWP apps.  Even if a game controller or joystick works in a Win32
app, that device is not guaranteed to work inside a WinRT / UWP app.

According to Microsoft, "Xbox compatible controllers" should work inside
UWP apps, potentially with more working in the future.  This includes, but
may not be limited to, Microsoft-made Xbox controllers and USB adapters.
(Source: https://social.msdn.microsoft.com/Forums/en-US/9064838b-e8c3-4c18-8a83-19bf0dfe150d/xinput-fails-to-detect-game-controllers?forum=wpdevelop)


