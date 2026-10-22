/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
/* Simple test of filesystem functions. */

#include <stdio.h>
#include "SDL.h"

int
main(int argc, char *argv[])
{
    char *base_path;
    char *pref_path;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_Init(0) == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init() failed: %s\n", SDL_GetError());
        return 1;
    }

    base_path = SDL_GetBasePath();
    if(base_path == NULL){
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find base path: %s\n",
                   SDL_GetError());
      return 1;
    }
    SDL_Log("base path: '%s'\n", base_path);
    SDL_free(base_path);

    pref_path = SDL_GetPrefPath("libsdl", "testfilesystem");
    if(pref_path == NULL){
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find pref path: %s\n",
                   SDL_GetError());
      return 1;
    }
    SDL_Log("pref path: '%s'\n", pref_path); 
    SDL_free(pref_path);

    pref_path = SDL_GetPrefPath(NULL, "testfilesystem");
    if(pref_path == NULL){
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find pref path without organization: %s\n",
                   SDL_GetError());
      return 1;
    }
    SDL_Log("pref path: '%s'\n", pref_path); 
    SDL_free(pref_path);

    SDL_Quit();
    return 0;
}
