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
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_HAIKU

#include "SDL_bframebuffer.h"

#include <AppKit.h>
#include <InterfaceKit.h>
#include "SDL_bmodes.h"
#include "SDL_BWin.h"

#include "../../main/haiku/SDL_BApp.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DRAWTHREAD
static int32 BE_UpdateOnce(SDL_Window *window);
#endif

static SDL_INLINE SDL_BWin *_ToBeWin(SDL_Window *window) {
	return ((SDL_BWin*)(window->driverdata));
}

static SDL_INLINE SDL_BApp *_GetBeApp() {
	return ((SDL_BApp*)be_app);
}

int BE_CreateWindowFramebuffer(_THIS, SDL_Window * window,
                                       Uint32 * format,
                                       void ** pixels, int *pitch) {
	SDL_BWin *bwin = _ToBeWin(window);
	BScreen bscreen;
	if(!bscreen.IsValid()) {
		return -1;
	}

	while(!bwin->Connected()) { snooze(100); }
	
	/* Make sure we have exclusive access to frame buffer data */
	bwin->LockBuffer();

	/* format */
	display_mode bmode;
	bscreen.GetMode(&bmode);
	int32 bpp = BE_ColorSpaceToBitsPerPixel(bmode.space);
	*format = BE_BPPToSDLPxFormat(bpp);

	/* Create the new bitmap object */
	BBitmap *bitmap = bwin->GetBitmap();

	if(bitmap) {
		delete bitmap;
	}
	bitmap = new BBitmap(bwin->Bounds(), (color_space)bmode.space,
			false,	/* Views not accepted */
			true);	/* Contiguous memory required */
			
	if(bitmap->InitCheck() != B_OK) {
		delete bitmap;
		return SDL_SetError("Could not initialize back buffer!");
	}


	bwin->SetBitmap(bitmap);
	
	/* Set the pixel pointer */
	*pixels = bitmap->Bits();

	/* pitch = width of window, in bytes */
	*pitch = bitmap->BytesPerRow();

	bwin->SetBufferExists(true);
	bwin->SetTrashBuffer(false);
	bwin->UnlockBuffer();
	return 0;
}



int BE_UpdateWindowFramebuffer(_THIS, SDL_Window * window,
                                      const SDL_Rect * rects, int numrects) {
	if(!window)
		return 0;

	SDL_BWin *bwin = _ToBeWin(window);

#ifdef DRAWTHREAD	
	bwin->LockBuffer();
	bwin->SetBufferDirty(true);
	bwin->UnlockBuffer();
#else
	bwin->SetBufferDirty(true);
	BE_UpdateOnce(window);
#endif

	return 0;
}

int32 BE_DrawThread(void *data) {
	SDL_BWin *bwin = (SDL_BWin*)data;
	
	BScreen bscreen;
	if(!bscreen.IsValid()) {
		return -1;
	}

	while(bwin->ConnectionEnabled()) {
		if( bwin->Connected() && bwin->BufferExists() && bwin->BufferIsDirty() ) {
			bwin->LockBuffer();
			BBitmap *bitmap = NULL;
			bitmap = bwin->GetBitmap();
			int32 windowPitch = bitmap->BytesPerRow();
			int32 bufferPitch = bwin->GetRowBytes();
			uint8 *windowpx;
			uint8 *bufferpx;

			int32 BPP = bwin->GetBytesPerPx();
			int32 windowSub = bwin->GetFbX() * BPP +
						  bwin->GetFbY() * windowPitch;
			clipping_rect *clips = bwin->GetClips();
			int32 numClips = bwin->GetNumClips();
			int i, y;

			/* Blit each clipping rectangle */
			bscreen.WaitForRetrace();
			for(i = 0; i < numClips; ++i) {
				/* Get addresses of the start of each clipping rectangle */
				int32 width = clips[i].right - clips[i].left + 1;
				int32 height = clips[i].bottom - clips[i].top + 1;
				bufferpx = bwin->GetBufferPx() + 
					clips[i].top * bufferPitch + clips[i].left * BPP;
				windowpx = (uint8*)bitmap->Bits() + 
					clips[i].top * windowPitch + clips[i].left * BPP -
					windowSub;

				/* Copy each row of pixels from the window buffer into the frame
				   buffer */
				for(y = 0; y < height; ++y)
				{

					if(bwin->CanTrashWindowBuffer()) {
						goto escape;	/* Break out before the buffer is killed */
					}

					memcpy(bufferpx, windowpx, width * BPP);
					bufferpx += bufferPitch;
					windowpx += windowPitch;
				}
			}

			bwin->SetBufferDirty(false);
escape:
			bwin->UnlockBuffer();
		} else {
			snooze(16000);
		}
	}
	
	return B_OK;
}

void BE_DestroyWindowFramebuffer(_THIS, SDL_Window * window) {
	SDL_BWin *bwin = _ToBeWin(window);
	
	bwin->LockBuffer();
	
	/* Free and clear the window buffer */
	BBitmap *bitmap = bwin->GetBitmap();
	delete bitmap;
	bwin->SetBitmap(NULL);
	bwin->SetBufferExists(false);
	bwin->UnlockBuffer();
}


/*
 * TODO:
 * This was written to test if certain errors were caused by threading issues.
 * The specific issues have since become rare enough that they may have been
 * solved, but I doubt it- they were pretty sporadic before now.
 */
#ifndef DRAWTHREAD
static int32 BE_UpdateOnce(SDL_Window *window) {
	SDL_BWin *bwin = _ToBeWin(window);
	BScreen bscreen;
	if(!bscreen.IsValid()) {
		return -1;
	}

	if(bwin->ConnectionEnabled() && bwin->Connected()) {
		bwin->LockBuffer();
		int32 windowPitch = window->surface->pitch;
		int32 bufferPitch = bwin->GetRowBytes();
		uint8 *windowpx;
		uint8 *bufferpx;

		int32 BPP = bwin->GetBytesPerPx();
		uint8 *windowBaseAddress = (uint8*)window->surface->pixels;
		int32 windowSub = bwin->GetFbX() * BPP +
						  bwin->GetFbY() * windowPitch;
		clipping_rect *clips = bwin->GetClips();
		int32 numClips = bwin->GetNumClips();
		int i, y;

		/* Blit each clipping rectangle */
		bscreen.WaitForRetrace();
		for(i = 0; i < numClips; ++i) {
			/* Get addresses of the start of each clipping rectangle */
			int32 width = clips[i].right - clips[i].left + 1;
			int32 height = clips[i].bottom - clips[i].top + 1;
			bufferpx = bwin->GetBufferPx() + 
				clips[i].top * bufferPitch + clips[i].left * BPP;
			windowpx = windowBaseAddress + 
				clips[i].top * windowPitch + clips[i].left * BPP - windowSub;

			/* Copy each row of pixels from the window buffer into the frame
			   buffer */
			for(y = 0; y < height; ++y)
			{
				memcpy(bufferpx, windowpx, width * BPP);
				bufferpx += bufferPitch;
				windowpx += windowPitch;
			}
		}
		bwin->UnlockBuffer();
	}
	return 0;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* SDL_VIDEO_DRIVER_HAIKU */

/* vi: set ts=4 sw=4 expandtab: */
