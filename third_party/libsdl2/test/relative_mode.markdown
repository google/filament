Relative mode testing
=====================

See test program at the bottom of this file.

Initial tests:

 - When in relative mode, the mouse shouldn't be moveable outside of the window.
 - When the cursor is outside the window when relative mode is enabled, mouse
   clicks should not go to whatever app was under the cursor previously.
 - When alt/cmd-tabbing between a relative mode app and another app, clicks when
   in the relative mode app should also not go to whatever app was under the
   cursor previously.


Code
====

    #include <SDL.h>

    int PollEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    return 1;
                default:
                    break;
            }
        }

        return 0;
    }

    int main(int argc, char *argv[])
    {
        SDL_Window *win;

        SDL_Init(SDL_INIT_VIDEO);

        win = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
        SDL_SetRelativeMouseMode(SDL_TRUE);

        while (1)
        {
            if (PollEvents())
                break;
        }

        SDL_DestroyWindow(win);

        SDL_Quit();

        return 0;
    }
