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
    int i;
    int __block clickedindex = messageboxdata->numbuttons;
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
        const SDL_MessageBoxButtonData *sdlButton;

        if (messageboxdata->flags & SDL_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT) {
            sdlButton = &messageboxdata->buttons[messageboxdata->numbuttons - 1 - i];
        } else {
            sdlButton = &messageboxdata->buttons[i];
        }

        if (sdlButton->flags & SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT) {
            style = UIAlertActionStyleCancel;
        }

        action = [UIAlertAction actionWithTitle:@(sdlButton->text)
                                style:style
                                handler:^(UIAlertAction *action) {
                                    clickedindex = (int)(sdlButton - messageboxdata->buttons);
                                }];
        [alert addAction:action];

        if (sdlButton->flags & SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT) {
            alert.preferredAction = action;
        }
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

    UIKit_ForceUpdateHomeIndicator();

    *buttonid = messageboxdata->buttons[clickedindex].buttonid;
    return YES;
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
    UIAlertView *alert = [[UIAlertView alloc] init];
    SDLAlertViewDelegate *delegate = [[SDLAlertViewDelegate alloc] init];

    alert.delegate = delegate;
    alert.title = @(messageboxdata->title);
    alert.message = @(messageboxdata->message);

    for (i = 0; i < messageboxdata->numbuttons; i++) {
        const SDL_MessageBoxButtonData *sdlButton;
        if (messageboxdata->flags & SDL_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT) {
            sdlButton = &messageboxdata->buttons[messageboxdata->numbuttons - 1 - i];
        } else {
            sdlButton = &messageboxdata->buttons[i];
        }
        [alert addButtonWithTitle:@(sdlButton->text)];
    }

    delegate.clickedIndex = &clickedindex;

    [alert show];

    UIKit_WaitUntilMessageBoxClosed(messageboxdata, &clickedindex);

    alert.delegate = nil;

    if (messageboxdata->flags & SDL_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT) {
        clickedindex = messageboxdata->numbuttons - 1 - clickedindex;
    }
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
