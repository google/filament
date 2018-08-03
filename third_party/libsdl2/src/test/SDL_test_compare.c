/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

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

/*

 Based on automated SDL_Surface tests originally written by Edgar Simo 'bobbens'.

 Rewritten for test lib by Andreas Schiffler.

*/

#include "SDL_config.h"

#include "SDL_test.h"


/* Counter for _CompareSurface calls; used for filename creation when comparisons fail */
static int _CompareSurfaceCount = 0;

/* Compare surfaces */
int SDLTest_CompareSurfaces(SDL_Surface *surface, SDL_Surface *referenceSurface, int allowable_error)
{
   int ret;
   int i,j;
   int bpp, bpp_reference;
   Uint8 *p, *p_reference;
   int dist;
   int sampleErrorX = 0, sampleErrorY = 0, sampleDist = 0;
   Uint8 R, G, B, A;
   Uint8 Rd, Gd, Bd, Ad;
   char imageFilename[128];
   char referenceFilename[128];

   /* Validate input surfaces */
   if (surface == NULL || referenceSurface == NULL) {
      return -1;
   }

   /* Make sure surface size is the same. */
   if ((surface->w != referenceSurface->w) || (surface->h != referenceSurface->h)) {
      return -2;
   }

   /* Sanitize input value */
   if (allowable_error<0) {
      allowable_error = 0;
   }

   SDL_LockSurface( surface );
   SDL_LockSurface( referenceSurface );

   ret = 0;
   bpp = surface->format->BytesPerPixel;
   bpp_reference = referenceSurface->format->BytesPerPixel;
   /* Compare image - should be same format. */
   for (j=0; j<surface->h; j++) {
      for (i=0; i<surface->w; i++) {
         p  = (Uint8 *)surface->pixels + j * surface->pitch + i * bpp;
         p_reference = (Uint8 *)referenceSurface->pixels + j * referenceSurface->pitch + i * bpp_reference;

         SDL_GetRGBA(*(Uint32*)p, surface->format, &R, &G, &B, &A);
         SDL_GetRGBA(*(Uint32*)p_reference, referenceSurface->format, &Rd, &Gd, &Bd, &Ad);

         dist = 0;
         dist += (R-Rd)*(R-Rd);
         dist += (G-Gd)*(G-Gd);
         dist += (B-Bd)*(B-Bd);

         /* Allow some difference in blending accuracy */
         if (dist > allowable_error) {
            ret++;
            if (ret == 1) {
               sampleErrorX = i;
               sampleErrorY = j;
               sampleDist = dist;
            }
         }
      }
   }

   SDL_UnlockSurface( surface );
   SDL_UnlockSurface( referenceSurface );

   /* Save test image and reference for analysis on failures */
   _CompareSurfaceCount++;
   if (ret != 0) {
      SDLTest_LogError("Comparison of pixels with allowable error of %i failed %i times.", allowable_error, ret);
      SDLTest_LogError("First detected occurrence at position %i,%i with a squared RGB-difference of %i.", sampleErrorX, sampleErrorY, sampleDist); 
      SDL_snprintf(imageFilename, 127, "CompareSurfaces%04d_TestOutput.bmp", _CompareSurfaceCount);
      SDL_SaveBMP(surface, imageFilename);
      SDL_snprintf(referenceFilename, 127, "CompareSurfaces%04d_Reference.bmp", _CompareSurfaceCount);
      SDL_SaveBMP(referenceSurface, referenceFilename);
      SDLTest_LogError("Surfaces from failed comparison saved as '%s' and '%s'", imageFilename, referenceFilename);
   }

   return ret;
}

/* vi: set ts=4 sw=4 expandtab: */
