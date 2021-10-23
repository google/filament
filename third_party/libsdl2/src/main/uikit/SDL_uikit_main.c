/*
    SDL_uikit_main.c, placed in the public domain by Sam Lantinga  3/18/2019
*/
#include "../../SDL_internal.h"

/* Include the SDL main definition header */
#include "SDL_main.h"

#ifndef SDL_MAIN_HANDLED
#ifdef main
#undef main
#endif

int
main(int argc, char *argv[])
{
    return SDL_UIKitRunApp(argc, argv, SDL_main);
}
#endif /* !SDL_MAIN_HANDLED */

/* vi: set ts=4 sw=4 expandtab: */
