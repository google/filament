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
 * @author Mark Callow, www.edgewise-consulting.com.
 *
 * Thanks to Alex Szpakowski, @slime73 on GitHub, for his gist showing
 * how to add a CAMetalLayer backed view.
 */

#ifndef SDL_cocoametalview_h_
#define SDL_cocoametalview_h_

#import "../SDL_sysvideo.h"
#import "SDL_cocoawindow.h"

#if SDL_VIDEO_DRIVER_COCOA && (SDL_VIDEO_VULKAN || SDL_VIDEO_RENDER_METAL)

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#define METALVIEW_TAG 255

@interface SDL_cocoametalview : NSView {
    NSInteger _tag;
}

- (instancetype)initWithFrame:(NSRect)frame
                        scale:(CGFloat)scale;

/* Override superclass tag so this class can set it. */
@property (assign, readonly) NSInteger tag;

@end

SDL_cocoametalview* Cocoa_Mtl_AddMetalView(SDL_Window* window);

void Cocoa_Mtl_GetDrawableSize(SDL_Window * window, int * w, int * h);

#endif /* SDL_VIDEO_DRIVER_COCOA && (SDL_VIDEO_VULKAN || SDL_VIDEO_RENDER_METAL) */

#endif /* SDL_cocoametalview_h_ */

/* vi: set ts=4 sw=4 expandtab: */

