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

/**
 *  \file SDL_mouse.h
 *
 *  Include file for SDL mouse event handling.
 */

#ifndef SDL_mouse_h_
#define SDL_mouse_h_

#include "SDL_stdinc.h"
#include "SDL_error.h"
#include "SDL_video.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct SDL_Cursor SDL_Cursor;   /**< Implementation dependent */

/**
 * \brief Cursor types for SDL_CreateSystemCursor().
 */
typedef enum
{
    SDL_SYSTEM_CURSOR_ARROW,     /**< Arrow */
    SDL_SYSTEM_CURSOR_IBEAM,     /**< I-beam */
    SDL_SYSTEM_CURSOR_WAIT,      /**< Wait */
    SDL_SYSTEM_CURSOR_CROSSHAIR, /**< Crosshair */
    SDL_SYSTEM_CURSOR_WAITARROW, /**< Small wait cursor (or Wait if not available) */
    SDL_SYSTEM_CURSOR_SIZENWSE,  /**< Double arrow pointing northwest and southeast */
    SDL_SYSTEM_CURSOR_SIZENESW,  /**< Double arrow pointing northeast and southwest */
    SDL_SYSTEM_CURSOR_SIZEWE,    /**< Double arrow pointing west and east */
    SDL_SYSTEM_CURSOR_SIZENS,    /**< Double arrow pointing north and south */
    SDL_SYSTEM_CURSOR_SIZEALL,   /**< Four pointed arrow pointing north, south, east, and west */
    SDL_SYSTEM_CURSOR_NO,        /**< Slashed circle or crossbones */
    SDL_SYSTEM_CURSOR_HAND,      /**< Hand */
    SDL_NUM_SYSTEM_CURSORS
} SDL_SystemCursor;

/**
 * \brief Scroll direction types for the Scroll event
 */
typedef enum
{
    SDL_MOUSEWHEEL_NORMAL,    /**< The scroll direction is normal */
    SDL_MOUSEWHEEL_FLIPPED    /**< The scroll direction is flipped / natural */
} SDL_MouseWheelDirection;

/* Function prototypes */

/**
 *  \brief Get the window which currently has mouse focus.
 */
extern DECLSPEC SDL_Window * SDLCALL SDL_GetMouseFocus(void);

/**
 *  \brief Retrieve the current state of the mouse.
 *
 *  The current button state is returned as a button bitmask, which can
 *  be tested using the SDL_BUTTON(X) macros, and x and y are set to the
 *  mouse cursor position relative to the focus window for the currently
 *  selected mouse.  You can pass NULL for either x or y.
 */
extern DECLSPEC Uint32 SDLCALL SDL_GetMouseState(int *x, int *y);

/**
 *  \brief Get the current state of the mouse, in relation to the desktop
 *
 *  This works just like SDL_GetMouseState(), but the coordinates will be
 *  reported relative to the top-left of the desktop. This can be useful if
 *  you need to track the mouse outside of a specific window and
 *  SDL_CaptureMouse() doesn't fit your needs. For example, it could be
 *  useful if you need to track the mouse while dragging a window, where
 *  coordinates relative to a window might not be in sync at all times.
 *
 *  \note SDL_GetMouseState() returns the mouse position as SDL understands
 *        it from the last pump of the event queue. This function, however,
 *        queries the OS for the current mouse position, and as such, might
 *        be a slightly less efficient function. Unless you know what you're
 *        doing and have a good reason to use this function, you probably want
 *        SDL_GetMouseState() instead.
 *
 *  \param x Returns the current X coord, relative to the desktop. Can be NULL.
 *  \param y Returns the current Y coord, relative to the desktop. Can be NULL.
 *  \return The current button state as a bitmask, which can be tested using the SDL_BUTTON(X) macros.
 *
 *  \sa SDL_GetMouseState
 */
extern DECLSPEC Uint32 SDLCALL SDL_GetGlobalMouseState(int *x, int *y);

/**
 *  \brief Retrieve the relative state of the mouse.
 *
 *  The current button state is returned as a button bitmask, which can
 *  be tested using the SDL_BUTTON(X) macros, and x and y are set to the
 *  mouse deltas since the last call to SDL_GetRelativeMouseState().
 */
extern DECLSPEC Uint32 SDLCALL SDL_GetRelativeMouseState(int *x, int *y);

/**
 *  \brief Moves the mouse to the given position within the window.
 *
 *  \param window The window to move the mouse into, or NULL for the current mouse focus
 *  \param x The x coordinate within the window
 *  \param y The y coordinate within the window
 *
 *  \note This function generates a mouse motion event
 */
extern DECLSPEC void SDLCALL SDL_WarpMouseInWindow(SDL_Window * window,
                                                   int x, int y);

/**
 *  \brief Moves the mouse to the given position in global screen space.
 *
 *  \param x The x coordinate
 *  \param y The y coordinate
 *  \return 0 on success, -1 on error (usually: unsupported by a platform).
 *
 *  \note This function generates a mouse motion event
 */
extern DECLSPEC int SDLCALL SDL_WarpMouseGlobal(int x, int y);

/**
 *  \brief Set relative mouse mode.
 *
 *  \param enabled Whether or not to enable relative mode
 *
 *  \return 0 on success, or -1 if relative mode is not supported.
 *
 *  While the mouse is in relative mode, the cursor is hidden, and the
 *  driver will try to report continuous motion in the current window.
 *  Only relative motion events will be delivered, the mouse position
 *  will not change.
 *
 *  \note This function will flush any pending mouse motion.
 *
 *  \sa SDL_GetRelativeMouseMode()
 */
extern DECLSPEC int SDLCALL SDL_SetRelativeMouseMode(SDL_bool enabled);

/**
 *  \brief Capture the mouse, to track input outside an SDL window.
 *
 *  \param enabled Whether or not to enable capturing
 *
 *  Capturing enables your app to obtain mouse events globally, instead of
 *  just within your window. Not all video targets support this function.
 *  When capturing is enabled, the current window will get all mouse events,
 *  but unlike relative mode, no change is made to the cursor and it is
 *  not restrained to your window.
 *
 *  This function may also deny mouse input to other windows--both those in
 *  your application and others on the system--so you should use this
 *  function sparingly, and in small bursts. For example, you might want to
 *  track the mouse while the user is dragging something, until the user
 *  releases a mouse button. It is not recommended that you capture the mouse
 *  for long periods of time, such as the entire time your app is running.
 *
 *  While captured, mouse events still report coordinates relative to the
 *  current (foreground) window, but those coordinates may be outside the
 *  bounds of the window (including negative values). Capturing is only
 *  allowed for the foreground window. If the window loses focus while
 *  capturing, the capture will be disabled automatically.
 *
 *  While capturing is enabled, the current window will have the
 *  SDL_WINDOW_MOUSE_CAPTURE flag set.
 *
 *  \return 0 on success, or -1 if not supported.
 */
extern DECLSPEC int SDLCALL SDL_CaptureMouse(SDL_bool enabled);

/**
 *  \brief Query whether relative mouse mode is enabled.
 *
 *  \sa SDL_SetRelativeMouseMode()
 */
extern DECLSPEC SDL_bool SDLCALL SDL_GetRelativeMouseMode(void);

/**
 *  \brief Create a cursor, using the specified bitmap data and
 *         mask (in MSB format).
 *
 *  The cursor width must be a multiple of 8 bits.
 *
 *  The cursor is created in black and white according to the following:
 *  <table>
 *  <tr><td> data </td><td> mask </td><td> resulting pixel on screen </td></tr>
 *  <tr><td>  0   </td><td>  1   </td><td> White </td></tr>
 *  <tr><td>  1   </td><td>  1   </td><td> Black </td></tr>
 *  <tr><td>  0   </td><td>  0   </td><td> Transparent </td></tr>
 *  <tr><td>  1   </td><td>  0   </td><td> Inverted color if possible, black
 *                                         if not. </td></tr>
 *  </table>
 *
 *  \sa SDL_FreeCursor()
 */
extern DECLSPEC SDL_Cursor *SDLCALL SDL_CreateCursor(const Uint8 * data,
                                                     const Uint8 * mask,
                                                     int w, int h, int hot_x,
                                                     int hot_y);

/**
 *  \brief Create a color cursor.
 *
 *  \sa SDL_FreeCursor()
 */
extern DECLSPEC SDL_Cursor *SDLCALL SDL_CreateColorCursor(SDL_Surface *surface,
                                                          int hot_x,
                                                          int hot_y);

/**
 *  \brief Create a system cursor.
 *
 *  \sa SDL_FreeCursor()
 */
extern DECLSPEC SDL_Cursor *SDLCALL SDL_CreateSystemCursor(SDL_SystemCursor id);

/**
 *  \brief Set the active cursor.
 */
extern DECLSPEC void SDLCALL SDL_SetCursor(SDL_Cursor * cursor);

/**
 *  \brief Return the active cursor.
 */
extern DECLSPEC SDL_Cursor *SDLCALL SDL_GetCursor(void);

/**
 *  \brief Return the default cursor.
 */
extern DECLSPEC SDL_Cursor *SDLCALL SDL_GetDefaultCursor(void);

/**
 *  \brief Frees a cursor created with SDL_CreateCursor() or similar functions.
 *
 *  \sa SDL_CreateCursor()
 *  \sa SDL_CreateColorCursor()
 *  \sa SDL_CreateSystemCursor()
 */
extern DECLSPEC void SDLCALL SDL_FreeCursor(SDL_Cursor * cursor);

/**
 *  \brief Toggle whether or not the cursor is shown.
 *
 *  \param toggle 1 to show the cursor, 0 to hide it, -1 to query the current
 *                state.
 *
 *  \return 1 if the cursor is shown, or 0 if the cursor is hidden.
 */
extern DECLSPEC int SDLCALL SDL_ShowCursor(int toggle);

/**
 *  Used as a mask when testing buttons in buttonstate.
 *   - Button 1:  Left mouse button
 *   - Button 2:  Middle mouse button
 *   - Button 3:  Right mouse button
 */
#define SDL_BUTTON(X)       (1 << ((X)-1))
#define SDL_BUTTON_LEFT     1
#define SDL_BUTTON_MIDDLE   2
#define SDL_BUTTON_RIGHT    3
#define SDL_BUTTON_X1       4
#define SDL_BUTTON_X2       5
#define SDL_BUTTON_LMASK    SDL_BUTTON(SDL_BUTTON_LEFT)
#define SDL_BUTTON_MMASK    SDL_BUTTON(SDL_BUTTON_MIDDLE)
#define SDL_BUTTON_RMASK    SDL_BUTTON(SDL_BUTTON_RIGHT)
#define SDL_BUTTON_X1MASK   SDL_BUTTON(SDL_BUTTON_X1)
#define SDL_BUTTON_X2MASK   SDL_BUTTON(SDL_BUTTON_X2)


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* SDL_mouse_h_ */

/* vi: set ts=4 sw=4 expandtab: */
