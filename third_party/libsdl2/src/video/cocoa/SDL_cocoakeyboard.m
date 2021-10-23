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

#if SDL_VIDEO_DRIVER_COCOA

#include "SDL_cocoavideo.h"

#include "../../events/SDL_events_c.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../events/scancodes_darwin.h"

#include <Carbon/Carbon.h>

/*#define DEBUG_IME NSLog */
#define DEBUG_IME(...)

@interface SDLTranslatorResponder : NSView <NSTextInputClient> {
    NSString *_markedText;
    NSRange   _markedRange;
    NSRange   _selectedRange;
    SDL_Rect  _inputRect;
}
- (void)doCommandBySelector:(SEL)myselector;
- (void)setInputRect:(SDL_Rect *)rect;
@end

@implementation SDLTranslatorResponder

- (void)setInputRect:(SDL_Rect *)rect
{
    _inputRect = *rect;
}

- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange
{
    /* TODO: Make use of replacementRange? */

    const char *str;

    DEBUG_IME(@"insertText: %@", aString);

    /* Could be NSString or NSAttributedString, so we have
     * to test and convert it before return as SDL event */
    if ([aString isKindOfClass: [NSAttributedString class]]) {
        str = [[aString string] UTF8String];
    } else {
        str = [aString UTF8String];
    }

    SDL_SendKeyboardText(str);
}

- (void)doCommandBySelector:(SEL)myselector
{
    /* No need to do anything since we are not using Cocoa
       selectors to handle special keys, instead we use SDL
       key events to do the same job.
    */
}

- (BOOL)hasMarkedText
{
    return _markedText != nil;
}

- (NSRange)markedRange
{
    return _markedRange;
}

- (NSRange)selectedRange
{
    return _selectedRange;
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
    if ([aString isKindOfClass:[NSAttributedString class]]) {
        aString = [aString string];
    }

    if ([aString length] == 0) {
        [self unmarkText];
        return;
    }

    if (_markedText != aString) {
        [_markedText release];
        _markedText = [aString retain];
    }

    _selectedRange = selectedRange;
    _markedRange = NSMakeRange(0, [aString length]);

    SDL_SendEditingText([aString UTF8String],
                        (int) selectedRange.location, (int) selectedRange.length);

    DEBUG_IME(@"setMarkedText: %@, (%d, %d)", _markedText,
          selRange.location, selRange.length);
}

- (void)unmarkText
{
    [_markedText release];
    _markedText = nil;

    SDL_SendEditingText("", 0, 0);
}

- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
    NSWindow *window = [self window];
    NSRect contentRect = [window contentRectForFrameRect:[window frame]];
    float windowHeight = contentRect.size.height;
    NSRect rect = NSMakeRect(_inputRect.x, windowHeight - _inputRect.y - _inputRect.h,
                             _inputRect.w, _inputRect.h);

    if (actualRange) {
        *actualRange = aRange;
    }

    DEBUG_IME(@"firstRectForCharacterRange: (%d, %d): windowHeight = %g, rect = %@",
            aRange.location, aRange.length, windowHeight,
            NSStringFromRect(rect));

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1070
    if (![window respondsToSelector:@selector(convertRectToScreen:)]) {
        rect.origin = [window convertBaseToScreen:rect.origin];
    } else
#endif
    {
        rect = [window convertRectToScreen:rect];
    }

    return rect;
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
    DEBUG_IME(@"attributedSubstringFromRange: (%d, %d)", aRange.location, aRange.length);
    return nil;
}

- (NSInteger)conversationIdentifier
{
    return (NSInteger) self;
}

/* This method returns the index for character that is
 * nearest to thePoint.  thPoint is in screen coordinate system.
 */
- (NSUInteger)characterIndexForPoint:(NSPoint)thePoint
{
    DEBUG_IME(@"characterIndexForPoint: (%g, %g)", thePoint.x, thePoint.y);
    return 0;
}

/* This method is the key to attribute extension.
 * We could add new attributes through this method.
 * NSInputServer examines the return value of this
 * method & constructs appropriate attributed string.
 */
- (NSArray *)validAttributesForMarkedText
{
    return [NSArray array];
}

@end


/* This is a helper function for HandleModifierSide. This
 * function reverts back to behavior before the distinction between
 * sides was made.
 */
static void
HandleNonDeviceModifier(unsigned int device_independent_mask,
                        unsigned int oldMods,
                        unsigned int newMods,
                        SDL_Scancode scancode)
{
    unsigned int oldMask, newMask;

    /* Isolate just the bits we care about in the depedent bits so we can
     * figure out what changed
     */
    oldMask = oldMods & device_independent_mask;
    newMask = newMods & device_independent_mask;

    if (oldMask && oldMask != newMask) {
        SDL_SendKeyboardKey(SDL_RELEASED, scancode);
    } else if (newMask && oldMask != newMask) {
        SDL_SendKeyboardKey(SDL_PRESSED, scancode);
    }
}

/* This is a helper function for HandleModifierSide.
 * This function sets the actual SDL_PrivateKeyboard event.
 */
static void
HandleModifierOneSide(unsigned int oldMods, unsigned int newMods,
                      SDL_Scancode scancode,
                      unsigned int sided_device_dependent_mask)
{
    unsigned int old_dep_mask, new_dep_mask;

    /* Isolate just the bits we care about in the depedent bits so we can
     * figure out what changed
     */
    old_dep_mask = oldMods & sided_device_dependent_mask;
    new_dep_mask = newMods & sided_device_dependent_mask;

    /* We now know that this side bit flipped. But we don't know if
     * it went pressed to released or released to pressed, so we must
     * find out which it is.
     */
    if (new_dep_mask && old_dep_mask != new_dep_mask) {
        SDL_SendKeyboardKey(SDL_PRESSED, scancode);
    } else {
        SDL_SendKeyboardKey(SDL_RELEASED, scancode);
    }
}

/* This is a helper function for DoSidedModifiers.
 * This function will figure out if the modifier key is the left or right side,
 * e.g. left-shift vs right-shift.
 */
static void
HandleModifierSide(int device_independent_mask,
                   unsigned int oldMods, unsigned int newMods,
                   SDL_Scancode left_scancode,
                   SDL_Scancode right_scancode,
                   unsigned int left_device_dependent_mask,
                   unsigned int right_device_dependent_mask)
{
    unsigned int device_dependent_mask = (left_device_dependent_mask |
                                         right_device_dependent_mask);
    unsigned int diff_mod;

    /* On the basis that the device independent mask is set, but there are
     * no device dependent flags set, we'll assume that we can't detect this
     * keyboard and revert to the unsided behavior.
     */
    if ((device_dependent_mask & newMods) == 0) {
        /* Revert to the old behavior */
        HandleNonDeviceModifier(device_independent_mask, oldMods, newMods, left_scancode);
        return;
    }

    /* XOR the previous state against the new state to see if there's a change */
    diff_mod = (device_dependent_mask & oldMods) ^
               (device_dependent_mask & newMods);
    if (diff_mod) {
        /* A change in state was found. Isolate the left and right bits
         * to handle them separately just in case the values can simulataneously
         * change or if the bits don't both exist.
         */
        if (left_device_dependent_mask & diff_mod) {
            HandleModifierOneSide(oldMods, newMods, left_scancode, left_device_dependent_mask);
        }
        if (right_device_dependent_mask & diff_mod) {
            HandleModifierOneSide(oldMods, newMods, right_scancode, right_device_dependent_mask);
        }
    }
}

/* This is a helper function for DoSidedModifiers.
 * This function will release a key press in the case that
 * it is clear that the modifier has been released (i.e. one side
 * can't still be down).
 */
static void
ReleaseModifierSide(unsigned int device_independent_mask,
                    unsigned int oldMods, unsigned int newMods,
                    SDL_Scancode left_scancode,
                    SDL_Scancode right_scancode,
                    unsigned int left_device_dependent_mask,
                    unsigned int right_device_dependent_mask)
{
    unsigned int device_dependent_mask = (left_device_dependent_mask |
                                          right_device_dependent_mask);

    /* On the basis that the device independent mask is set, but there are
     * no device dependent flags set, we'll assume that we can't detect this
     * keyboard and revert to the unsided behavior.
     */
    if ((device_dependent_mask & oldMods) == 0) {
        /* In this case, we can't detect the keyboard, so use the left side
         * to represent both, and release it.
         */
        SDL_SendKeyboardKey(SDL_RELEASED, left_scancode);
        return;
    }

    /*
     * This could have been done in an if-else case because at this point,
     * we know that all keys have been released when calling this function.
     * But I'm being paranoid so I want to handle each separately,
     * so I hope this doesn't cause other problems.
     */
    if ( left_device_dependent_mask & oldMods ) {
        SDL_SendKeyboardKey(SDL_RELEASED, left_scancode);
    }
    if ( right_device_dependent_mask & oldMods ) {
        SDL_SendKeyboardKey(SDL_RELEASED, right_scancode);
    }
}

/* This function will handle the modifier keys and also determine the
 * correct side of the key.
 */
static void
DoSidedModifiers(unsigned short scancode,
                 unsigned int oldMods, unsigned int newMods)
{
    /* Set up arrays for the key syms for the left and right side. */
    const SDL_Scancode left_mapping[]  = {
        SDL_SCANCODE_LSHIFT,
        SDL_SCANCODE_LCTRL,
        SDL_SCANCODE_LALT,
        SDL_SCANCODE_LGUI
    };
    const SDL_Scancode right_mapping[] = {
        SDL_SCANCODE_RSHIFT,
        SDL_SCANCODE_RCTRL,
        SDL_SCANCODE_RALT,
        SDL_SCANCODE_RGUI
    };
    /* Set up arrays for the device dependent masks with indices that
     * correspond to the _mapping arrays
     */
    const unsigned int left_device_mapping[]  = { NX_DEVICELSHIFTKEYMASK, NX_DEVICELCTLKEYMASK, NX_DEVICELALTKEYMASK, NX_DEVICELCMDKEYMASK };
    const unsigned int right_device_mapping[] = { NX_DEVICERSHIFTKEYMASK, NX_DEVICERCTLKEYMASK, NX_DEVICERALTKEYMASK, NX_DEVICERCMDKEYMASK };

    unsigned int i, bit;

    /* Iterate through the bits, testing each against the old modifiers */
    for (i = 0, bit = NSEventModifierFlagShift; bit <= NSEventModifierFlagCommand; bit <<= 1, ++i) {
        unsigned int oldMask, newMask;

        oldMask = oldMods & bit;
        newMask = newMods & bit;

        /* If the bit is set, we must always examine it because the left
         * and right side keys may alternate or both may be pressed.
         */
        if (newMask) {
            HandleModifierSide(bit, oldMods, newMods,
                               left_mapping[i], right_mapping[i],
                               left_device_mapping[i], right_device_mapping[i]);
        }
        /* If the state changed from pressed to unpressed, we must examine
            * the device dependent bits to release the correct keys.
            */
        else if (oldMask && oldMask != newMask) {
            ReleaseModifierSide(bit, oldMods, newMods,
                              left_mapping[i], right_mapping[i],
                              left_device_mapping[i], right_device_mapping[i]);
        }
    }
}

static void
HandleModifiers(_THIS, unsigned short scancode, unsigned int modifierFlags)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    if (modifierFlags == data->modifierFlags) {
        return;
    }

    DoSidedModifiers(scancode, data->modifierFlags, modifierFlags);
    data->modifierFlags = modifierFlags;
}

static void
UpdateKeymap(SDL_VideoData *data, SDL_bool send_event)
{
    TISInputSourceRef key_layout;
    const void *chr_data;
    int i;
    SDL_Scancode scancode;
    SDL_Keycode keymap[SDL_NUM_SCANCODES];

    /* See if the keymap needs to be updated */
    key_layout = TISCopyCurrentKeyboardLayoutInputSource();
    if (key_layout == data->key_layout) {
        return;
    }
    data->key_layout = key_layout;

    SDL_GetDefaultKeymap(keymap);

    /* Try Unicode data first */
    CFDataRef uchrDataRef = TISGetInputSourceProperty(key_layout, kTISPropertyUnicodeKeyLayoutData);
    if (uchrDataRef) {
        chr_data = CFDataGetBytePtr(uchrDataRef);
    } else {
        goto cleanup;
    }

    if (chr_data) {
        UInt32 keyboard_type = LMGetKbdType();
        OSStatus err;

        for (i = 0; i < SDL_arraysize(darwin_scancode_table); i++) {
            UniChar s[8];
            UniCharCount len;
            UInt32 dead_key_state;

            /* Make sure this scancode is a valid character scancode */
            scancode = darwin_scancode_table[i];
            if (scancode == SDL_SCANCODE_UNKNOWN ||
                (keymap[scancode] & SDLK_SCANCODE_MASK)) {
                continue;
            }

            dead_key_state = 0;
            err = UCKeyTranslate ((UCKeyboardLayout *) chr_data,
                                  i, kUCKeyActionDown,
                                  0, keyboard_type,
                                  kUCKeyTranslateNoDeadKeysMask,
                                  &dead_key_state, 8, &len, s);
            if (err != noErr) {
                continue;
            }

            if (len > 0 && s[0] != 0x10) {
                keymap[scancode] = s[0];
            }
        }
        SDL_SetKeymap(0, keymap, SDL_NUM_SCANCODES);
        if (send_event) {
            SDL_SendKeymapChangedEvent();
        }
        return;
    }

cleanup:
    CFRelease(key_layout);
}

void
Cocoa_InitKeyboard(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    UpdateKeymap(data, SDL_FALSE);

    /* Set our own names for the platform-dependent but layout-independent keys */
    /* This key is NumLock on the MacBook keyboard. :) */
    /*SDL_SetScancodeName(SDL_SCANCODE_NUMLOCKCLEAR, "Clear");*/
    SDL_SetScancodeName(SDL_SCANCODE_LALT, "Left Option");
    SDL_SetScancodeName(SDL_SCANCODE_LGUI, "Left Command");
    SDL_SetScancodeName(SDL_SCANCODE_RALT, "Right Option");
    SDL_SetScancodeName(SDL_SCANCODE_RGUI, "Right Command");

    data->modifierFlags = (unsigned int)[NSEvent modifierFlags];
    SDL_ToggleModState(KMOD_CAPS, (data->modifierFlags & NSEventModifierFlagCapsLock) != 0);
}

void
Cocoa_StartTextInput(_THIS)
{ @autoreleasepool
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    SDL_Window *window = SDL_GetKeyboardFocus();
    NSWindow *nswindow = nil;
    if (window) {
        nswindow = ((SDL_WindowData*)window->driverdata)->nswindow;
    }

    NSView *parentView = [nswindow contentView];

    /* We only keep one field editor per process, since only the front most
     * window can receive text input events, so it make no sense to keep more
     * than one copy. When we switched to another window and requesting for
     * text input, simply remove the field editor from its superview then add
     * it to the front most window's content view */
    if (!data->fieldEdit) {
        data->fieldEdit =
            [[SDLTranslatorResponder alloc] initWithFrame: NSMakeRect(0.0, 0.0, 0.0, 0.0)];
    }

    if (![[data->fieldEdit superview] isEqual:parentView]) {
        /* DEBUG_IME(@"add fieldEdit to window contentView"); */
        [data->fieldEdit removeFromSuperview];
        [parentView addSubview: data->fieldEdit];
        [nswindow makeFirstResponder: data->fieldEdit];
    }
}}

void
Cocoa_StopTextInput(_THIS)
{ @autoreleasepool
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    if (data && data->fieldEdit) {
        [data->fieldEdit removeFromSuperview];
        [data->fieldEdit release];
        data->fieldEdit = nil;
    }
}}

void
Cocoa_SetTextInputRect(_THIS, SDL_Rect *rect)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    if (!rect) {
        SDL_InvalidParamError("rect");
        return;
    }

    [data->fieldEdit setInputRect:rect];
}

void
Cocoa_HandleKeyEvent(_THIS, NSEvent *event)
{
    SDL_VideoData *data = _this ? ((SDL_VideoData *) _this->driverdata) : NULL;
    if (!data) {
        return;  /* can happen when returning from fullscreen Space on shutdown */
    }

    unsigned short scancode = [event keyCode];
    SDL_Scancode code;
#if 0
    const char *text;
#endif

    if ((scancode == 10 || scancode == 50) && KBGetLayoutType(LMGetKbdType()) == kKeyboardISO) {
        /* see comments in SDL_cocoakeys.h */
        scancode = 60 - scancode;
    }

    if (scancode < SDL_arraysize(darwin_scancode_table)) {
        code = darwin_scancode_table[scancode];
    } else {
        /* Hmm, does this ever happen?  If so, need to extend the keymap... */
        code = SDL_SCANCODE_UNKNOWN;
    }

    switch ([event type]) {
    case NSEventTypeKeyDown:
        if (![event isARepeat]) {
            /* See if we need to rebuild the keyboard layout */
            UpdateKeymap(data, SDL_TRUE);
        }

        SDL_SendKeyboardKey(SDL_PRESSED, code);
#if 1
        if (code == SDL_SCANCODE_UNKNOWN) {
            fprintf(stderr, "The key you just pressed is not recognized by SDL. To help get this fixed, report this to the SDL forums/mailing list <https://discourse.libsdl.org/> or to Christian Walther <cwalther@gmx.ch>. Mac virtual key code is %d.\n", scancode);
        }
#endif
        if (SDL_EventState(SDL_TEXTINPUT, SDL_QUERY)) {
            /* FIXME CW 2007-08-16: only send those events to the field editor for which we actually want text events, not e.g. esc or function keys. Arrow keys in particular seem to produce crashes sometimes. */
            [data->fieldEdit interpretKeyEvents:[NSArray arrayWithObject:event]];
#if 0
            text = [[event characters] UTF8String];
            if(text && *text) {
                SDL_SendKeyboardText(text);
                [data->fieldEdit setString:@""];
            }
#endif
        }
        break;
    case NSEventTypeKeyUp:
        SDL_SendKeyboardKey(SDL_RELEASED, code);
        break;
    case NSEventTypeFlagsChanged:
        /* FIXME CW 2007-08-14: check if this whole mess that takes up half of this file is really necessary */
        HandleModifiers(_this, scancode, (unsigned int)[event modifierFlags]);
        break;
    default: /* just to avoid compiler warnings */
        break;
    }
}

void
Cocoa_QuitKeyboard(_THIS)
{
}

typedef int CGSConnection;
typedef enum {
    CGSGlobalHotKeyEnable = 0,
    CGSGlobalHotKeyDisable = 1,
} CGSGlobalHotKeyOperatingMode;

extern CGSConnection _CGSDefaultConnection(void);
extern CGError CGSSetGlobalHotKeyOperatingMode(CGSConnection connection, CGSGlobalHotKeyOperatingMode mode);

void
Cocoa_SetWindowKeyboardGrab(_THIS, SDL_Window * window, SDL_bool grabbed)
{
#if SDL_MAC_NO_SANDBOX
    CGSSetGlobalHotKeyOperatingMode(_CGSDefaultConnection(), grabbed ? CGSGlobalHotKeyDisable : CGSGlobalHotKeyEnable);
#endif
}

#endif /* SDL_VIDEO_DRIVER_COCOA */

/* vi: set ts=4 sw=4 expandtab: */
