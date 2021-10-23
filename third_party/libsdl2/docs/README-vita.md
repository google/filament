PS Vita
=======
SDL port for the Sony Playstation Vita and Sony Playstation TV

Credit to
* xerpi and rsn8887 for initial (vita2d) port
* vitasdk/dolcesdk devs
* CBPS discord (Namely Graphene and SonicMastr)

Building
--------
To build for the PSVita, make sure you have vitasdk and cmake installed and run:
```
   cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=${VITASDK}/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   cmake --install build
```


Notes
-----
* gles2 support is disabled by default and can be enabled by configuring with `-DVIDEO_VITA_PIB=ON`
* By default SDL emits mouse events for touch events on every touchscreen.  
  Vita has two touchscreens, so it's recommended to use `SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");` and handle touch events instead.
* Support for L2/R2/R3/R3 buttons, haptic feedback and gamepad led only available on PSTV, or when using external ds4 gamepad on vita.