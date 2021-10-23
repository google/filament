/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_VIDEO_DRIVER_UIKIT && (SDL_VIDEO_OPENGL_ES || SDL_VIDEO_OPENGL_ES2)

#include "SDL_uikitopengles.h"
#import "SDL_uikitopenglview.h"
#include "SDL_uikitmodes.h"
#include "SDL_uikitwindow.h"
#include "SDL_uikitevents.h"
#include "../SDL_sysvideo.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../events/SDL_mouse_c.h"
#include "../../power/uikit/SDL_syspower.h"
#include "SDL_loadso.h"
#include <dlfcn.h>

@interface SDLEAGLContext : EAGLContext

/* The OpenGL ES context owns a view / drawable. */
@property (nonatomic, strong) SDL_uikitopenglview *sdlView;

@end

@implementation SDLEAGLContext

- (void)dealloc
{
    /* When the context is deallocated, its view should be removed from any
     * SDL window that it's attached to. */
    [self.sdlView setSDLWindow:NULL];
}

@end

void *
UIKit_GL_GetProcAddress(_THIS, const char *proc)
{
    /* Look through all SO's for the proc symbol.  Here's why:
     * -Looking for the path to the OpenGL Library seems not to work in the iOS Simulator.
     * -We don't know that the path won't change in the future. */
    return dlsym(RTLD_DEFAULT, proc);
}

/*
  note that SDL_GL_DeleteContext makes it current without passing the window
*/
int
UIKit_GL_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context)
{
    @autoreleasepool {
        SDLEAGLContext *eaglcontext = (__bridge SDLEAGLContext *) context;

        if (![EAGLContext setCurrentContext:eaglcontext]) {
            return SDL_SetError("Could not make EAGL context current");
        }

        if (eaglcontext) {
            [eaglcontext.sdlView setSDLWindow:window];
        }
    }

    return 0;
}

void
UIKit_GL_GetDrawableSize(_THIS, SDL_Window * window, int * w, int * h)
{
    @autoreleasepool {
        SDL_WindowData *data = (__bridge SDL_WindowData *)window->driverdata;
        UIView *view = data.viewcontroller.view;
        if ([view isKindOfClass:[SDL_uikitopenglview class]]) {
            SDL_uikitopenglview *glview = (SDL_uikitopenglview *) view;
            if (w) {
                *w = glview.backingWidth;
            }
            if (h) {
                *h = glview.backingHeight;
            }
        } else {
            SDL_GetWindowSize(window, w, h);
        }
    }
}

int
UIKit_GL_LoadLibrary(_THIS, const char *path)
{
    /* We shouldn't pass a path to this function, since we've already loaded the
     * library. */
    if (path != NULL) {
        return SDL_SetError("iOS GL Load Library just here for compatibility");
    }
    return 0;
}

int UIKit_GL_SwapWindow(_THIS, SDL_Window * window)
{
    @autoreleasepool {
        SDLEAGLContext *context = (__bridge SDLEAGLContext *) SDL_GL_GetCurrentContext();

#if SDL_POWER_UIKIT
        /* Check once a frame to see if we should turn off the battery monitor. */
        SDL_UIKit_UpdateBatteryMonitoring();
#endif

        [context.sdlView swapBuffers];

        /* You need to pump events in order for the OS to make changes visible.
         * We don't pump events here because we don't want iOS application events
         * (low memory, terminate, etc.) to happen inside low level rendering. */
    }
    return 0;
}

SDL_GLContext
UIKit_GL_CreateContext(_THIS, SDL_Window * window)
{
    @autoreleasepool {
        SDLEAGLContext *context = nil;
        SDL_uikitopenglview *view;
        SDL_WindowData *data = (__bridge SDL_WindowData *) window->driverdata;
        CGRect frame = UIKit_ComputeViewFrame(window, data.uiwindow.screen);
        EAGLSharegroup *sharegroup = nil;
        CGFloat scale = 1.0;
        int samples = 0;
        int major = _this->gl_config.major_version;
        int minor = _this->gl_config.minor_version;

        /* The EAGLRenderingAPI enum values currently map 1:1 to major GLES
         * versions. */
        EAGLRenderingAPI api = major;

        /* iOS currently doesn't support GLES >3.0. iOS 6 also only supports up
         * to GLES 2.0. */
        if (major > 3 || (major == 3 && (minor > 0 || !UIKit_IsSystemVersionAtLeast(7.0)))) {
            SDL_SetError("OpenGL ES %d.%d context could not be created", major, minor);
            return NULL;
        }

        if (_this->gl_config.multisamplebuffers > 0) {
            samples = _this->gl_config.multisamplesamples;
        }

        if (_this->gl_config.share_with_current_context) {
            EAGLContext *context = (__bridge EAGLContext *) SDL_GL_GetCurrentContext();
            sharegroup = context.sharegroup;
        }

        if (window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
            /* Set the scale to the natural scale factor of the screen - the
             * backing dimensions of the OpenGL view will match the pixel
             * dimensions of the screen rather than the dimensions in points. */
            if ([data.uiwindow.screen respondsToSelector:@selector(nativeScale)]) {
                scale = data.uiwindow.screen.nativeScale;
            } else {
                scale = data.uiwindow.screen.scale;
            }
        }

        context = [[SDLEAGLContext alloc] initWithAPI:api sharegroup:sharegroup];
        if (!context) {
            SDL_SetError("OpenGL ES %d context could not be created", _this->gl_config.major_version);
            return NULL;
        }

        /* construct our view, passing in SDL's OpenGL configuration data */
        view = [[SDL_uikitopenglview alloc] initWithFrame:frame
                                                    scale:scale
                                            retainBacking:_this->gl_config.retained_backing
                                                    rBits:_this->gl_config.red_size
                                                    gBits:_this->gl_config.green_size
                                                    bBits:_this->gl_config.blue_size
                                                    aBits:_this->gl_config.alpha_size
                                                depthBits:_this->gl_config.depth_size
                                              stencilBits:_this->gl_config.stencil_size
                                                     sRGB:_this->gl_config.framebuffer_srgb_capable
                                             multisamples:samples
                                                  context:context];

        if (!view) {
            return NULL;
        }

        /* The context owns the view / drawable. */
        context.sdlView = view;

        if (UIKit_GL_MakeCurrent(_this, window, (__bridge SDL_GLContext) context) < 0) {
            UIKit_GL_DeleteContext(_this, (SDL_GLContext) CFBridgingRetain(context));
            return NULL;
        }

        /* We return a +1'd context. The window's driverdata owns the view (via
         * MakeCurrent.) */
        return (SDL_GLContext) CFBridgingRetain(context);
    }
}

void
UIKit_GL_DeleteContext(_THIS, SDL_GLContext context)
{
    @autoreleasepool {
        /* The context was retained in SDL_GL_CreateContext, so we release it
         * here. The context's view will be detached from its window when the
         * context is deallocated. */
        CFRelease(context);
    }
}

void
UIKit_GL_RestoreCurrentContext(void)
{
    @autoreleasepool {
        /* Some iOS system functionality (such as Dictation on the on-screen
         keyboard) uses its own OpenGL ES context but doesn't restore the
         previous one when it's done. This is a workaround to make sure the
         expected SDL-created OpenGL ES context is active after the OS is
         finished running its own code for the frame. If this isn't done, the
         app may crash or have other nasty symptoms when Dictation is used.
         */
        EAGLContext *context = (__bridge EAGLContext *) SDL_GL_GetCurrentContext();
        if (context != NULL && [EAGLContext currentContext] != context) {
            [EAGLContext setCurrentContext:context];
        }
    }
}

#endif /* SDL_VIDEO_DRIVER_UIKIT */

/* vi: set ts=4 sw=4 expandtab: */
