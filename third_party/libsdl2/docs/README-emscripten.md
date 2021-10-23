Emscripten
================================================================================

Build:

    $ mkdir build
    $ cd build
    $ emconfigure ../configure --host=asmjs-unknown-emscripten --disable-assembly --disable-threads --disable-cpuinfo CFLAGS="-O2"
    $ emmake make

Or with cmake:

    $ mkdir build
    $ cd build
    $ emcmake cmake ..
    $ emmake make

To build one of the tests:

    $ cd test/
    $ emcc -O2 --js-opts 0 -g4 testdraw2.c -I../include ../build/.libs/libSDL2.a ../build/libSDL2_test.a -o a.html

Uses GLES2 renderer or software

Some other SDL2 libraries can be easily built (assuming SDL2 is installed somewhere):

SDL_mixer (http://www.libsdl.org/projects/SDL_mixer/):

    $ EMCONFIGURE_JS=1 emconfigure ../configure
    build as usual...

SDL_gfx (http://cms.ferzkopp.net/index.php/software/13-sdl-gfx):

    $ EMCONFIGURE_JS=1 emconfigure ../configure --disable-mmx
    build as usual...
