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

#ifndef SDL_BWin_h_
#define SDL_BWin_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../SDL_internal.h"
#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_bframebuffer.h"

#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <AppKit.h>
#include <InterfaceKit.h>
#include <game/DirectWindow.h>
#if SDL_VIDEO_OPENGL
#include <opengl/GLView.h>
#endif
#include "SDL_events.h"
#include "../../main/haiku/SDL_BApp.h"


enum WinCommands {
    BWIN_MOVE_WINDOW,
    BWIN_RESIZE_WINDOW,
    BWIN_SHOW_WINDOW,
    BWIN_HIDE_WINDOW,
    BWIN_MAXIMIZE_WINDOW,
    BWIN_MINIMIZE_WINDOW,
    BWIN_RESTORE_WINDOW,
    BWIN_SET_TITLE,
    BWIN_SET_BORDERED,
    BWIN_SET_RESIZABLE,
    BWIN_FULLSCREEN
};


class SDL_BWin:public BDirectWindow
{
  public:
    /* Constructor/Destructor */
    SDL_BWin(BRect bounds, window_look look, uint32 flags)
        : BDirectWindow(bounds, "Untitled", look, B_NORMAL_WINDOW_FEEL, flags)
    {
        _last_buttons = 0;

#if SDL_VIDEO_OPENGL
        _SDL_GLView = NULL;
        _gl_type = 0;
#endif
        _shown = false;
        _inhibit_resize = false;
        _mouse_focused = false;
        _prev_frame = NULL;

        /* Handle framebuffer stuff */
        _connected = _connection_disabled = false;
        _buffer_created = _buffer_dirty = false;
        _trash_window_buffer = false;
        _buffer_locker = new BLocker();
        _bitmap = NULL;
        _clips = NULL;

#ifdef DRAWTHREAD
        _draw_thread_id = spawn_thread(BE_DrawThread, "drawing_thread",
                            B_NORMAL_PRIORITY, (void*) this);
        resume_thread(_draw_thread_id);
#endif
    }

    virtual ~ SDL_BWin()
    {
        Lock();
        _connection_disabled = true;
        int32 result;

#if SDL_VIDEO_OPENGL
        if (_SDL_GLView) {
            _SDL_GLView->UnlockGL();
            RemoveChild(_SDL_GLView);   /* Why was this outside the if
                                            statement before? */
        }

#endif
        Unlock();
#if SDL_VIDEO_OPENGL
        if (_SDL_GLView) {
            delete _SDL_GLView;
        }
#endif

        delete _prev_frame;

        /* Clean up framebuffer stuff */
        _buffer_locker->Lock();
#ifdef DRAWTHREAD
        wait_for_thread(_draw_thread_id, &result);
#endif
        free(_clips);
        delete _buffer_locker;
    }


    /* * * * * OpenGL functionality * * * * */
#if SDL_VIDEO_OPENGL
    virtual BGLView *CreateGLView(Uint32 gl_flags) {
        Lock();
        if (_SDL_GLView == NULL) {
            _SDL_GLView = new BGLView(Bounds(), "SDL GLView",
                                     B_FOLLOW_ALL_SIDES,
                                     (B_WILL_DRAW | B_FRAME_EVENTS),
                                     gl_flags);
            _gl_type = gl_flags;
        }
        AddChild(_SDL_GLView);
        _SDL_GLView->EnableDirectMode(true);
        _SDL_GLView->LockGL();  /* "New" GLViews are created */
        Unlock();
        return (_SDL_GLView);
    }

    virtual void RemoveGLView() {
        Lock();
        if(_SDL_GLView) {
            _SDL_GLView->UnlockGL();
            RemoveChild(_SDL_GLView);
        }
        Unlock();
    }

    virtual void SwapBuffers(void) {
        _SDL_GLView->UnlockGL();
        _SDL_GLView->LockGL();
        _SDL_GLView->SwapBuffers();
    }
#endif

    /* * * * * Framebuffering* * * * */
    virtual void DirectConnected(direct_buffer_info *info) {
        if(!_connected && _connection_disabled) {
            return;
        }

        /* Determine if the pixel buffer is usable after this update */
        _trash_window_buffer =      _trash_window_buffer
                                || ((info->buffer_state & B_BUFFER_RESIZED)
                                || (info->buffer_state & B_BUFFER_RESET)
                                || (info->driver_state == B_MODE_CHANGED));
        LockBuffer();

        switch(info->buffer_state & B_DIRECT_MODE_MASK) {
        case B_DIRECT_START:
            _connected = true;

        case B_DIRECT_MODIFY:
            if(_clips) {
                free(_clips);
                _clips = NULL;
            }

            _num_clips = info->clip_list_count;
            _clips = (clipping_rect *)malloc(_num_clips*sizeof(clipping_rect));
            if(_clips) {
                memcpy(_clips, info->clip_list,
                    _num_clips*sizeof(clipping_rect));

                _bits = (uint8*) info->bits;
                _row_bytes = info->bytes_per_row;
                _bounds = info->window_bounds;
                _bytes_per_px = info->bits_per_pixel / 8;
                _buffer_dirty = true;
            }
            break;

        case B_DIRECT_STOP:
            _connected = false;
            break;
        }
#if SDL_VIDEO_OPENGL
        if(_SDL_GLView) {
            _SDL_GLView->DirectConnected(info);
        }
#endif


        /* Call the base object directconnected */
        BDirectWindow::DirectConnected(info);

        UnlockBuffer();

    }




    /* * * * * Event sending * * * * */
    /* Hook functions */
    virtual void FrameMoved(BPoint origin) {
        /* Post a message to the BApp so that it can handle the window event */
        BMessage msg(BAPP_WINDOW_MOVED);
        msg.AddInt32("window-x", (int)origin.x);
        msg.AddInt32("window-y", (int)origin.y);
        _PostWindowEvent(msg);

        /* Perform normal hook operations */
        BDirectWindow::FrameMoved(origin);
    }

    virtual void FrameResized(float width, float height) {
        /* Post a message to the BApp so that it can handle the window event */
        BMessage msg(BAPP_WINDOW_RESIZED);

        msg.AddInt32("window-w", (int)width + 1);
        msg.AddInt32("window-h", (int)height + 1);
        _PostWindowEvent(msg);

        /* Perform normal hook operations */
        BDirectWindow::FrameResized(width, height);
    }

    virtual bool QuitRequested() {
        BMessage msg(BAPP_WINDOW_CLOSE_REQUESTED);
        _PostWindowEvent(msg);

        /* We won't allow a quit unless asked by DestroyWindow() */
        return false;
    }

    virtual void WindowActivated(bool active) {
        BMessage msg(BAPP_KEYBOARD_FOCUS);  /* Mouse focus sold separately */
        msg.AddBool("focusGained", active);
        _PostWindowEvent(msg);
    }

    virtual void Zoom(BPoint origin,
                float width,
                float height) {
        BMessage msg(BAPP_MAXIMIZE);    /* Closest thing to maximization Haiku has */
        _PostWindowEvent(msg);

        /* Before the window zooms, record its size */
        if( !_prev_frame )
            _prev_frame = new BRect(Frame());

        /* Perform normal hook operations */
        BDirectWindow::Zoom(origin, width, height);
    }

    /* Member functions */
    virtual void Show() {
        while(IsHidden()) {
            BDirectWindow::Show();
        }
        _shown = true;

        BMessage msg(BAPP_SHOW);
        _PostWindowEvent(msg);
    }

    virtual void Hide() {
        BDirectWindow::Hide();
        _shown = false;

        BMessage msg(BAPP_HIDE);
        _PostWindowEvent(msg);
    }

    virtual void Minimize(bool minimize) {
        BDirectWindow::Minimize(minimize);
        int32 minState = (minimize ? BAPP_MINIMIZE : BAPP_RESTORE);

        BMessage msg(minState);
        _PostWindowEvent(msg);
    }


    /* BView message interruption */
    virtual void DispatchMessage(BMessage * msg, BHandler * target)
    {
        BPoint where;   /* Used by mouse moved */
        int32 buttons;  /* Used for mouse button events */
        int32 key;      /* Used for key events */

        switch (msg->what) {
        case B_MOUSE_MOVED:
            int32 transit;
            if (msg->FindPoint("where", &where) == B_OK
                && msg->FindInt32("be:transit", &transit) == B_OK) {
                _MouseMotionEvent(where, transit);
            }

            /* FIXME: Apparently a button press/release event might be dropped
               if made before before a different button is released.  Does
               B_MOUSE_MOVED have the data needed to check if a mouse button
               state has changed? */
            if (msg->FindInt32("buttons", &buttons) == B_OK) {
                _MouseButtonEvent(buttons);
            }
            break;

        case B_MOUSE_DOWN:
        case B_MOUSE_UP:
            /* _MouseButtonEvent() detects any and all buttons that may have
               changed state, as well as that button's new state */
            if (msg->FindInt32("buttons", &buttons) == B_OK) {
                _MouseButtonEvent(buttons);
            }
            break;

        case B_MOUSE_WHEEL_CHANGED:
            float x, y;
            if (msg->FindFloat("be:wheel_delta_x", &x) == B_OK
                && msg->FindFloat("be:wheel_delta_y", &y) == B_OK) {
                    _MouseWheelEvent((int)x, (int)y);
            }
            break;

        case B_KEY_DOWN:
            {
                int32 i = 0;
                int8 byte;
                int8 bytes[4] = { 0, 0, 0, 0 };
                while (i < 4 && msg->FindInt8("byte", i, &byte) == B_OK) {
                    bytes[i] = byte;
                    i++;
                }
                if (msg->FindInt32("key", &key) == B_OK) {
                    _KeyEvent((SDL_Scancode)key, &bytes[0], i, SDL_PRESSED);
                }
            }
            break;
            
        case B_UNMAPPED_KEY_DOWN:      /* modifier keys are unmapped */
            if (msg->FindInt32("key", &key) == B_OK) {
                _KeyEvent((SDL_Scancode)key, NULL, 0, SDL_PRESSED);
            }
            break;

        case B_KEY_UP:
        case B_UNMAPPED_KEY_UP:        /* modifier keys are unmapped */
            if (msg->FindInt32("key", &key) == B_OK) {
                _KeyEvent(key, NULL, 0, SDL_RELEASED);
            }
            break;

        default:
            /* move it after switch{} so it's always handled
               that way we keep Haiku features like:
               - CTRL+Q to close window (and other shortcuts)
               - PrintScreen to make screenshot into /boot/home
               - etc.. */
            /* BDirectWindow::DispatchMessage(msg, target); */
            break;
        }

        BDirectWindow::DispatchMessage(msg, target);
    }

    /* Handle command messages */
    virtual void MessageReceived(BMessage* message) {
        switch (message->what) {
            /* Handle commands from SDL */
            case BWIN_SET_TITLE:
                _SetTitle(message);
                break;
            case BWIN_MOVE_WINDOW:
                _MoveTo(message);
                break;
            case BWIN_RESIZE_WINDOW:
                _ResizeTo(message);
                break;
            case BWIN_SET_BORDERED:
                _SetBordered(message);
                break;
            case BWIN_SET_RESIZABLE:
                _SetResizable(message);
                break;
            case BWIN_SHOW_WINDOW:
                Show();
                break;
            case BWIN_HIDE_WINDOW:
                Hide();
                break;
            case BWIN_MAXIMIZE_WINDOW:
                BWindow::Zoom();
                break;
            case BWIN_MINIMIZE_WINDOW:
                Minimize(true);
                break;
            case BWIN_RESTORE_WINDOW:
                _Restore();
                break;
            case BWIN_FULLSCREEN:
                _SetFullScreen(message);
                break;
            default:
                /* Perform normal message handling */
                BDirectWindow::MessageReceived(message);
                break;
        }

    }



    /* Accessor methods */
    bool IsShown() { return _shown; }
    int32 GetID() { return _id; }
    uint32 GetRowBytes() { return _row_bytes; }
    int32 GetFbX() { return _bounds.left; }
    int32 GetFbY() { return _bounds.top; }
    bool ConnectionEnabled() { return !_connection_disabled; }
    bool Connected() { return _connected; }
    clipping_rect *GetClips() { return _clips; }
    int32 GetNumClips() { return _num_clips; }
    uint8* GetBufferPx() { return _bits; }
    int32 GetBytesPerPx() { return _bytes_per_px; }
    bool CanTrashWindowBuffer() { return _trash_window_buffer; }
    bool BufferExists() { return _buffer_created; }
    bool BufferIsDirty() { return _buffer_dirty; }
    BBitmap *GetBitmap() { return _bitmap; }
#if SDL_VIDEO_OPENGL
    BGLView *GetGLView() { return _SDL_GLView; }
    Uint32 GetGLType() { return _gl_type; }
#endif

    /* Setter methods */
    void SetID(int32 id) { _id = id; }
    void SetBufferExists(bool bufferExists) { _buffer_created = bufferExists; }
    void LockBuffer() { _buffer_locker->Lock(); }
    void UnlockBuffer() { _buffer_locker->Unlock(); }
    void SetBufferDirty(bool bufferDirty) { _buffer_dirty = bufferDirty; }
    void SetTrashBuffer(bool trash) { _trash_window_buffer = trash;     }
    void SetBitmap(BBitmap *bitmap) { _bitmap = bitmap; }


private:
    /* Event redirection */
    void _MouseMotionEvent(BPoint &where, int32 transit) {
        if(transit == B_EXITED_VIEW) {
            /* Change mouse focus */
            if(_mouse_focused) {
                _MouseFocusEvent(false);
            }
        } else {
            /* Change mouse focus */
            if (!_mouse_focused) {
                _MouseFocusEvent(true);
            }
            BMessage msg(BAPP_MOUSE_MOVED);
            msg.AddInt32("x", (int)where.x);
            msg.AddInt32("y", (int)where.y);

            _PostWindowEvent(msg);
        }
    }

    void _MouseFocusEvent(bool focusGained) {
        _mouse_focused = focusGained;
        BMessage msg(BAPP_MOUSE_FOCUS);
        msg.AddBool("focusGained", focusGained);
        _PostWindowEvent(msg);

/* FIXME: Why were these here?
 if false: be_app->SetCursor(B_HAND_CURSOR);
 if true:  SDL_SetCursor(NULL); */
    }

    void _MouseButtonEvent(int32 buttons) {
        int32 buttonStateChange = buttons ^ _last_buttons;

        /* Make sure at least one button has changed state */
        if( !(buttonStateChange) ) {
            return;
        }

        /* Add any mouse button events */
        if(buttonStateChange & B_PRIMARY_MOUSE_BUTTON) {
            _SendMouseButton(SDL_BUTTON_LEFT, buttons &
                B_PRIMARY_MOUSE_BUTTON);
        }
        if(buttonStateChange & B_SECONDARY_MOUSE_BUTTON) {
            _SendMouseButton(SDL_BUTTON_RIGHT, buttons &
                B_PRIMARY_MOUSE_BUTTON);
        }
        if(buttonStateChange & B_TERTIARY_MOUSE_BUTTON) {
            _SendMouseButton(SDL_BUTTON_MIDDLE, buttons &
                B_PRIMARY_MOUSE_BUTTON);
        }

        _last_buttons = buttons;
    }

    void _SendMouseButton(int32 button, int32 state) {
        BMessage msg(BAPP_MOUSE_BUTTON);
        msg.AddInt32("button-id", button);
        msg.AddInt32("button-state", state);
        _PostWindowEvent(msg);
    }

    void _MouseWheelEvent(int32 x, int32 y) {
        /* Create a message to pass along to the BeApp thread */
        BMessage msg(BAPP_MOUSE_WHEEL);
        msg.AddInt32("xticks", x);
        msg.AddInt32("yticks", y);
        _PostWindowEvent(msg);
    }

    void _KeyEvent(int32 keyCode, const int8 *keyUtf8, const ssize_t & len, int32 keyState) {
        /* Create a message to pass along to the BeApp thread */
        BMessage msg(BAPP_KEY);
        msg.AddInt32("key-state", keyState);
        msg.AddInt32("key-scancode", keyCode);
        if (keyUtf8 != NULL) {
        	msg.AddData("key-utf8", B_INT8_TYPE, (const void*)keyUtf8, len);
        }
        be_app->PostMessage(&msg);
    }

    void _RepaintEvent() {
        /* Force a repaint: Call the SDL exposed event */
        BMessage msg(BAPP_REPAINT);
        _PostWindowEvent(msg);
    }
    void _PostWindowEvent(BMessage &msg) {
        msg.AddInt32("window-id", _id);
        be_app->PostMessage(&msg);
    }

    /* Command methods (functions called upon by SDL) */
    void _SetTitle(BMessage *msg) {
        const char *title;
        if(
            msg->FindString("window-title", &title) != B_OK
        ) {
            return;
        }
        SetTitle(title);
    }

    void _MoveTo(BMessage *msg) {
        int32 x, y;
        if(
            msg->FindInt32("window-x", &x) != B_OK ||
            msg->FindInt32("window-y", &y) != B_OK
        ) {
            return;
        }
        MoveTo(x, y);
    }

    void _ResizeTo(BMessage *msg) {
        int32 w, h;
        if(
            msg->FindInt32("window-w", &w) != B_OK ||
            msg->FindInt32("window-h", &h) != B_OK
        ) {
            return;
        }
        ResizeTo(w, h);
    }

    void _SetBordered(BMessage *msg) {
        bool bEnabled;
        if(msg->FindBool("window-border", &bEnabled) != B_OK) {
            return;
        }
        SetLook(bEnabled ? B_TITLED_WINDOW_LOOK : B_NO_BORDER_WINDOW_LOOK);
    }

    void _SetResizable(BMessage *msg) {
        bool bEnabled;
        if(msg->FindBool("window-resizable", &bEnabled) != B_OK) {
            return;
        }
        if (bEnabled) {
            SetFlags(Flags() & ~(B_NOT_RESIZABLE | B_NOT_ZOOMABLE));
        } else {
            SetFlags(Flags() | (B_NOT_RESIZABLE | B_NOT_ZOOMABLE));
        }
    }

    void _Restore() {
        if(IsMinimized()) {
            Minimize(false);
        } else if(IsHidden()) {
            Show();
        } else if(_prev_frame != NULL) {    /* Zoomed */
            MoveTo(_prev_frame->left, _prev_frame->top);
            ResizeTo(_prev_frame->Width(), _prev_frame->Height());
        }
    }

    void _SetFullScreen(BMessage *msg) {
        bool fullscreen;
        if(
            msg->FindBool("fullscreen", &fullscreen) != B_OK
        ) {
            return;
        }
        SetFullScreen(fullscreen);
    }

    /* Members */
#if SDL_VIDEO_OPENGL
    BGLView * _SDL_GLView;
    Uint32 _gl_type;
#endif

    int32 _last_buttons;
    int32 _id;  /* Window id used by SDL_BApp */
    bool  _mouse_focused;       /* Does this window have mouse focus? */
    bool  _shown;
    bool  _inhibit_resize;

    BRect *_prev_frame; /* Previous position and size of the window */

    /* Framebuffer members */
    bool            _connected,
                    _connection_disabled,
                    _buffer_created,
                    _buffer_dirty,
                    _trash_window_buffer;
    uint8          *_bits;
    uint32          _row_bytes;
    clipping_rect   _bounds;
    BLocker        *_buffer_locker;
    clipping_rect  *_clips;
    int32           _num_clips;
    int32           _bytes_per_px;
    thread_id       _draw_thread_id;

    BBitmap        *_bitmap;
};


/* FIXME:
 * An explanation of framebuffer flags.
 *
 * _connected -           Original variable used to let the drawing thread know
 *                         when changes are being made to the other framebuffer
 *                         members.
 * _connection_disabled - Used to signal to the drawing thread that the window
 *                         is closing, and the thread should exit.
 * _buffer_created -      True if the current buffer is valid
 * _buffer_dirty -        True if the window should be redrawn.
 * _trash_window_buffer - True if the window buffer needs to be trashed partway
 *                         through a draw cycle.  Occurs when the previous
 *                         buffer provided by DirectConnected() is invalidated.
 */
#endif /* SDL_BWin_h_ */

/* vi: set ts=4 sw=4 expandtab: */
