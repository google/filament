/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* These functions are designed for testing correctness, not for speed */

extern SDL_bool ConvertRGBtoYUV(Uint32 format, Uint8 *src, int pitch, Uint8 *out, int w, int h, SDL_YUV_CONVERSION_MODE mode, int monochrome, int luminance);
extern int CalculateYUVPitch(Uint32 format, int width);
