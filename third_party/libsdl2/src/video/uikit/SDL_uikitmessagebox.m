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

#if SDL_VIDEO_DRIVER_UIKIT

#include "SDL.h"
#include "SDL_uikitvideo.h"
#include "SDL_uikitwindow.h"

/* Display a UIKit message box */

static SDL_bool s_showingMessageBox = SDL_FALSE;

SDL_bool
UIKit_ShowingMessageBox(void)
{
    return s_showingMessageBox;
}

static void
UIKit_WaitUntilMessageBoxClosed(const SDL_MessageBoxData *messageboxdata, int *clickedindex)
{
    *clickedindex = messageboxdata->numbuttons;

    @autoreleasepool {
        /* Run the main event loop until the alert has finished */
        /* Note that this needs to be done on the main thread */
        s_showingMessageBox = SDL_TRUE;
        while ((*clickedindex) == messageboxdata->numbuttons) {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
        }
        s_showingMessageBox = SDL_FALSE;
    }
}

static BOOL
UIKit_ShowMessageBoxAlertController(const SDL_MessageBoxData *messageboxdata, int *buttonid)
{
#ifdef __IPHONE_8_0
    int i;
    int __block clickedindex = messageboxdata->numbuttons;
    const SDL_MessageBoxButtonData *buttons = messageboxdata->buttons;
    UIWindow *window = nil;
    UIWindow *alertwindow = nil;

    if (![UIAlertController class]) {
        return NO;
    }

    UIAlertController *alert;
    alert = [UIAlertController alertControllerWithTitle:@(messageboxdata->title)
                                                message:@(messageboxdata->message)
                                         preferredStyle:UIAlertControllerStyleAlert];

    for (i = 0; i < messageboxdata->numbuttons; i++) {
        UIAlertAction *action;
        UIAlertActionStyle style = UIAlertActionStyleDefault;

        if (buttons[i].flags & SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT) {
            style = UIAlertActionStyleCancel;
        }

        action = [UIAlertAction actionWithTitle:@(buttons[i].text)
                                          style:style
                                        handler:^(UIAlertAction *action) {
                                            clickedindex = i;
                                        }];
        [alert addAction:action];
    }

    if (messageboxdata->window) {
        SDL_WindowData *data = (__bridge SDL_WindowData *) messageboxdata->window->driverdata;
        window = data.uiwindow;
    }

    if (window == nil || window.rootViewController == nil) {
        alertwindow = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
        alertwindow.rootViewController = [UIViewController new];
        alertwindow.windowLevel = UIWindowLevelAlert;

        window = alertwindow;

        [alertwindow makeKeyAndVisible];
    }

    [window.rootViewController presentViewController:alert animated:YES completion:nil];
    UIKit_WaitUntilMessageBoxClosed(messageboxdata, &clickedindex);

    if (alertwindow) {
        alertwindow.hidden = YES;
    }

    /* Force the main SDL window to re-evaluate home indicator state */
    SDL_Window *focus = SDL_GetFocusWindow();
    if (focus) {
        SDL_WindowData *data = (__bridge SDL_WindowData *) focus->driverdata;
        if (data != nil) {
            if (@available(iOS 11.0, *)) {
                [data.viewcontroller performSelectorOnMainThread:@selector(setNeedsUpdateOfHomeIndicatorAutoHidden) withObject:nil waitUntilDone:NO];
                [data.viewcontroller performSelectorOnMainThread:@selector(setNeedsUpdateOfScreenEdgesDeferringSystemGestures) withObject:nil waitUntilDone:NO];
            }
        }
    }

    *buttonid = messageboxdata->buttons[clickedindex].buttonid;
    return YES;
#else
    return NO;
#endif /* __IPHONE_8_0 */
}

/* UIAlertView is deprecated in iOS 8+ in favor of UIAlertController. */
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 80000
@interface SDLAlertViewDelegate : NSObject <UIAlertViewDelegate>

@property (nonatomic, assign) int *clickedIndex;

@end

@implementation SDLAlertViewDelegate

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
    if (_clickedIndex != NULL) {
        *_clickedIndex = (int) buttonIndex;
    }
}

@end
#endif /* __IPHONE_OS_VERSION_MIN_REQUIRED < 80000 */

static BOOL
UIKit_ShowMessageBoxAlertView(const SDL_MessageBoxData *messageboxdata, int *buttonid)
{
    /* UIAlertView is deprecated in iOS 8+ in favor of UIAlertController. */
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 80000
    int i;
    int clickedindex = messageboxdata->numbuttons;
    const SDL_MessageBoxButtonData *buttons = messageboxdata->buttons;
    UIAlertView *alert = [[UIAlertView alloc] init];
    SDLAlertViewDelegate *delegate = [[SDLAlertViewDelegate alloc] init];

    alert.delegate = delegate;
    alert.title = @(messageboxdata->title);
    alert.message = @(messageboxdata->message);

    for (i = 0; i < messageboxdata->numbuttons; i++) {
        [alert addButtonWithTitle:@(buttons[i].text)];
    }

    delegate.clickedIndex = &clickedindex;

    [alert show];

    UIKit_WaitUntilMessageBoxClosed(messageboxdata, &clickedindex);

    alert.delegate = nil;

    *buttonid = messageboxdata->buttons[clickedindex].buttonid;
    return YES;
#else
    return NO;
#endif /* __IPHONE_OS_VERSION_MIN_REQUIRED < 80000 */
}

int
UIKit_ShowMessageBox(const SDL_MessageBoxData *messageboxdata, int *buttonid)
{
    BOOL success = NO;

    @autoreleasepool {
        success = UIKit_ShowMessageBoxAlertController(messageboxdata, buttonid);
        if (!success) {
            success = UIKit_ShowMessageBoxAlertView(messageboxdata, buttonid);
        }
    }

    if (!success) {
        return SDL_SetError("Could not show message box.");
    }

    return 0;
}

#endif /* SDL_VIDEO_DRIVER_UIKIT */

/* vi: set ts=4 sw=4 expandtab: */
