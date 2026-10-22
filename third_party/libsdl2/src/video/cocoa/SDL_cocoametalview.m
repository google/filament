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

#import "SDL_cocoametalview.h"

#if SDL_VIDEO_DRIVER_COCOA && (SDL_VIDEO_VULKAN || SDL_VIDEO_RENDER_METAL)

#include "SDL_assert.h"

@implementation SDL_cocoametalview

/* The synthesized getter should be called by super's viewWithTag. */
@synthesize tag = _tag;

/* Return a Metal-compatible layer. */
+ (Class)layerClass
{
	return NSClassFromString(@"CAMetalLayer");
}

/* Indicate the view wants to draw using a backing layer instead of drawRect. */
- (BOOL)wantsUpdateLayer
{
    return YES;
}

/* When the wantsLayer property is set to YES, this method will be invoked to
 * return a layer instance.
 */
- (CALayer*)makeBackingLayer
{
    return [self.class.layerClass layer];
}

- (instancetype)initWithFrame:(NSRect)frame
                        scale:(CGFloat)scale
{
	if ((self = [super initWithFrame:frame])) {
        _tag = METALVIEW_TAG;
        self.wantsLayer = YES;

        /* Allow resize. */
        self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

        /* Set the desired scale. The default drawableSize of a CAMetalLayer
         * is its bounds x its scale so nothing further needs to be done.
         */
        self.layer.contentsScale = scale;
	}
  
	return self;
}

/* Set the size of the metal drawables when the view is resized. */
- (void)resizeWithOldSuperviewSize:(NSSize)oldSize
{
    [super resizeWithOldSuperviewSize:oldSize];
}

@end

SDL_cocoametalview*
Cocoa_Mtl_AddMetalView(SDL_Window* window)
{
    SDL_WindowData* data = (__bridge SDL_WindowData *)window->driverdata;
    NSView *view = data->nswindow.contentView;
    CGFloat scale = 1.0;

    if (window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
        /* Set the scale to the natural scale factor of the screen - then
         * the backing dimensions of the Metal view will match the pixel
         * dimensions of the screen rather than the dimensions in points
         * yielding high resolution on retine displays.
         *
         * N.B. In order for backingScaleFactor to be > 1,
         * NSHighResolutionCapable must be set to true in the app's Info.plist.
         */
        NSWindow* nswindow = data->nswindow;
        if ([nswindow.screen respondsToSelector:@selector(backingScaleFactor)])
            scale = data->nswindow.screen.backingScaleFactor;
    }
        
    SDL_cocoametalview *metalview
        = [[SDL_cocoametalview alloc] initWithFrame:view.frame scale:scale];
    [view addSubview:metalview];
    return metalview;
}

void
Cocoa_Mtl_GetDrawableSize(SDL_Window * window, int * w, int * h)
{
    SDL_WindowData *data = (__bridge SDL_WindowData *)window->driverdata;
    NSView *view = data->nswindow.contentView;
    SDL_cocoametalview* metalview = [view viewWithTag:METALVIEW_TAG];
    if (metalview) {
        CAMetalLayer *layer = (CAMetalLayer*)metalview.layer;
        assert(layer != NULL);
        if (w) {
            *w = layer.drawableSize.width;
        }
        if (h) {
            *h = layer.drawableSize.height;
        }
    } else {
        SDL_GetWindowSize(window, w, h);
    }
}

#endif /* SDL_VIDEO_DRIVER_COCOA && (SDL_VIDEO_VULKAN || SDL_VIDEO_RENDER_METAL) */

/* vi: set ts=4 sw=4 expandtab: */
