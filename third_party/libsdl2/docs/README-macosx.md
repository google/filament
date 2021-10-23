Mac OS X
==============================================================================

These instructions are for people using Apple's Mac OS X (pronounced
"ten").

From the developer's point of view, OS X is a sort of hybrid Mac and
Unix system, and you have the option of using either traditional
command line tools or Apple's IDE Xcode.

Command Line Build
==================

To build SDL using the command line, use the standard configure and make
process:

    ./configure
    make
    sudo make install

You can also build SDL as a Universal library (a single binary for both
32-bit and 64-bit Intel architectures), on Mac OS X 10.7 and newer, by using
the gcc-fat.sh script in build-scripts:

    mkdir mybuild
    cd mybuild
    CC=$PWD/../build-scripts/gcc-fat.sh CXX=$PWD/../build-scripts/g++-fat.sh ../configure
    make
    sudo make install

This script builds SDL with 10.5 ABI compatibility on i386 and 10.6
ABI compatibility on x86_64 architectures.  For best compatibility you
should compile your application the same way.

Please note that building SDL requires at least Xcode 4.6 and the 10.7 SDK
(even if you target back to 10.5 systems). PowerPC support for Mac OS X has
been officially dropped as of SDL 2.0.2.

To use the library once it's built, you essential have two possibilities:
use the traditional autoconf/automake/make method, or use Xcode.

==============================================================================
Caveats for using SDL with Mac OS X
==============================================================================

Some things you have to be aware of when using SDL on Mac OS X:

- If you register your own NSApplicationDelegate (using [NSApp setDelegate:]),
  SDL will not register its own. This means that SDL will not terminate using
  SDL_Quit if it receives a termination request, it will terminate like a 
  normal app, and it will not send a SDL_DROPFILE when you request to open a
  file with the app. To solve these issues, put the following code in your 
  NSApplicationDelegate implementation:


    - (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
    {
        if (SDL_GetEventState(SDL_QUIT) == SDL_ENABLE) {
            SDL_Event event;
            event.type = SDL_QUIT;
            SDL_PushEvent(&event);
        }
    
        return NSTerminateCancel;
    }
    
    - (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
    {
        if (SDL_GetEventState(SDL_DROPFILE) == SDL_ENABLE) {
            SDL_Event event;
            event.type = SDL_DROPFILE;
            event.drop.file = SDL_strdup([filename UTF8String]);
            return (SDL_PushEvent(&event) > 0);
        }
    
        return NO;
    }

==============================================================================
Using the Simple DirectMedia Layer with a traditional Makefile
==============================================================================

An existing autoconf/automake build system for your SDL app has good chances
to work almost unchanged on OS X. However, to produce a "real" Mac OS X binary
that you can distribute to users, you need to put the generated binary into a
so called "bundle", which basically is a fancy folder with a name like
"MyCoolGame.app".

To get this build automatically, add something like the following rule to
your Makefile.am:

    bundle_contents = APP_NAME.app/Contents
    APP_NAME_bundle: EXE_NAME
    	mkdir -p $(bundle_contents)/MacOS
    	mkdir -p $(bundle_contents)/Resources
    	echo "APPL????" > $(bundle_contents)/PkgInfo
    	$(INSTALL_PROGRAM) $< $(bundle_contents)/MacOS/

You should replace EXE_NAME with the name of the executable. APP_NAME is what
will be visible to the user in the Finder. Usually it will be the same
as EXE_NAME but capitalized. E.g. if EXE_NAME is "testgame" then APP_NAME 
usually is "TestGame". You might also want to use `@PACKAGE@` to use the package
name as specified in your configure.ac file.

If your project builds more than one application, you will have to do a bit
more. For each of your target applications, you need a separate rule.

If you want the created bundles to be installed, you may want to add this
rule to your Makefile.am:

    install-exec-hook: APP_NAME_bundle
    	rm -rf $(DESTDIR)$(prefix)/Applications/APP_NAME.app
    	mkdir -p $(DESTDIR)$(prefix)/Applications/
    	cp -r $< /$(DESTDIR)$(prefix)Applications/

This rule takes the Bundle created by the rule from step 3 and installs them
into "$(DESTDIR)$(prefix)/Applications/".

Again, if you want to install multiple applications, you will have to augment
the make rule accordingly.


But beware! That is only part of the story! With the above, you end up with
a bare bone .app bundle, which is double clickable from the Finder. But
there are some more things you should do before shipping your product...

1) The bundle right now probably is dynamically linked against SDL. That 
   means that when you copy it to another computer, *it will not run*,
   unless you also install SDL on that other computer. A good solution
   for this dilemma is to static link against SDL. On OS X, you can
   achieve that by linking against the libraries listed by

       sdl-config --static-libs

   instead of those listed by

       sdl-config --libs

   Depending on how exactly SDL is integrated into your build systems, the
   way to achieve that varies, so I won't describe it here in detail

2) Add an 'Info.plist' to your application. That is a special XML file which
   contains some meta-information about your application (like some copyright
   information, the version of your app, the name of an optional icon file,
   and other things). Part of that information is displayed by the Finder
   when you click on the .app, or if you look at the "Get Info" window.
   More information about Info.plist files can be found on Apple's homepage.


As a final remark, let me add that I use some of the techniques (and some
variations of them) in Exult and ScummVM; both are available in source on
the net, so feel free to take a peek at them for inspiration!


==============================================================================
Using the Simple DirectMedia Layer with Xcode
==============================================================================

These instructions are for using Apple's Xcode IDE to build SDL applications.

- First steps

The first thing to do is to unpack the Xcode.tar.gz archive in the
top level SDL directory (where the Xcode.tar.gz archive resides).
Because Stuffit Expander will unpack the archive into a subdirectory,
you should unpack the archive manually from the command line:

    cd [path_to_SDL_source]
    tar zxf Xcode.tar.gz

This will create a new folder called Xcode, which you can browse
normally from the Finder.

- Building the Framework

The SDL Library is packaged as a framework bundle, an organized
relocatable folder hierarchy of executable code, interface headers,
and additional resources. For practical purposes, you can think of a 
framework as a more user and system-friendly shared library, whose library
file behaves more or less like a standard UNIX shared library.

To build the framework, simply open the framework project and build it. 
By default, the framework bundle "SDL.framework" is installed in 
/Library/Frameworks. Therefore, the testers and project stationary expect
it to be located there. However, it will function the same in any of the
following locations:

    ~/Library/Frameworks
    /Local/Library/Frameworks
    /System/Library/Frameworks

- Build Options
    There are two "Build Styles" (See the "Targets" tab) for SDL.
    "Deployment" should be used if you aren't tweaking the SDL library.
    "Development" should be used to debug SDL apps or the library itself.

- Building the Testers
    Open the SDLTest project and build away!

- Using the Project Stationary
    Copy the stationary to the indicated folders to access it from
    the "New Project" and "Add target" menus. What could be easier?

- Setting up a new project by hand
    Some of you won't want to use the Stationary so I'll give some tips:
    * Create a new "Cocoa Application"
    * Add src/main/macosx/SDLMain.m , .h and .nib to your project
    * Remove "main.c" from your project
    * Remove "MainMenu.nib" from your project
    * Add "$(HOME)/Library/Frameworks/SDL.framework/Headers" to include path
    * Add "$(HOME)/Library/Frameworks" to the frameworks search path
    * Add "-framework SDL -framework Foundation -framework AppKit" to "OTHER_LDFLAGS"
    * Set the "Main Nib File" under "Application Settings" to "SDLMain.nib"
    * Add your files
    * Clean and build

- Building from command line
    Use pbxbuild in the same directory as your .pbproj file

- Running your app
    You can send command line args to your app by either invoking it from
    the command line (in *.app/Contents/MacOS) or by entering them in the
    "Executables" panel of the target settings.
    
- Implementation Notes
    Some things that may be of interest about how it all works...
    * Working directory
        As defined in the SDL_main.m file, the working directory of your SDL app
        is by default set to its parent. You may wish to change this to better
        suit your needs.
    * You have a Cocoa App!
        Your SDL app is essentially a Cocoa application. When your app
        starts up and the libraries finish loading, a Cocoa procedure is called,
        which sets up the working directory and calls your main() method.
        You are free to modify your Cocoa app with generally no consequence 
        to SDL. You cannot, however, easily change the SDL window itself.
        Functionality may be added in the future to help this.


Known bugs are listed in the file "BUGS.txt".
