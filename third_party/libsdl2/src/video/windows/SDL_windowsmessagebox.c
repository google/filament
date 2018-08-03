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

#if SDL_VIDEO_DRIVER_WINDOWS

#include "../../core/windows/SDL_windows.h"

#include "SDL_assert.h"
#include "SDL_windowsvideo.h"


#ifndef SS_EDITCONTROL
#define SS_EDITCONTROL  0x2000
#endif

/* Display a Windows message box */

#pragma pack(push, 1)

typedef struct
{
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATEEX;

typedef struct
{
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    short x;
    short y;
    short cx;
    short cy;
    DWORD id;
} DLGITEMTEMPLATEEX;

#pragma pack(pop)

typedef struct
{
    DLGTEMPLATEEX* lpDialog;
    Uint8 *data;
    size_t size;
    size_t used;
} WIN_DialogData;


static INT_PTR MessageBoxDialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    switch ( iMessage ) {
    case WM_COMMAND:
        /* Return the ID of the button that was pushed */
        EndDialog(hDlg, LOWORD(wParam));
        return TRUE;

    default:
        break;
    }
    return FALSE;
}

static SDL_bool ExpandDialogSpace(WIN_DialogData *dialog, size_t space)
{
    size_t size = dialog->size;

    if (size == 0) {
        size = space;
    } else {
        while ((dialog->used + space) > size) {
            size *= 2;
        }
    }
    if (size > dialog->size) {
        void *data = SDL_realloc(dialog->data, size);
        if (!data) {
            SDL_OutOfMemory();
            return SDL_FALSE;
        }
        dialog->data = data;
        dialog->size = size;
        dialog->lpDialog = (DLGTEMPLATEEX*)dialog->data;
    }
    return SDL_TRUE;
}

static SDL_bool AlignDialogData(WIN_DialogData *dialog, size_t size)
{
    size_t padding = (dialog->used % size);

    if (!ExpandDialogSpace(dialog, padding)) {
        return SDL_FALSE;
    }

    dialog->used += padding;

    return SDL_TRUE;
}

static SDL_bool AddDialogData(WIN_DialogData *dialog, const void *data, size_t size)
{
    if (!ExpandDialogSpace(dialog, size)) {
        return SDL_FALSE;
    }

    SDL_memcpy(dialog->data+dialog->used, data, size);
    dialog->used += size;

    return SDL_TRUE;
}

static SDL_bool AddDialogString(WIN_DialogData *dialog, const char *string)
{
    WCHAR *wstring;
    WCHAR *p;
    size_t count;
    SDL_bool status;

    if (!string) {
        string = "";
    }

    wstring = WIN_UTF8ToString(string);
    if (!wstring) {
        return SDL_FALSE;
    }

    /* Find out how many characters we have, including null terminator */
    count = 0;
    for (p = wstring; *p; ++p) {
        ++count;
    }
    ++count;

    status = AddDialogData(dialog, wstring, count*sizeof(WCHAR));
    SDL_free(wstring);
    return status;
}

static int s_BaseUnitsX;
static int s_BaseUnitsY;
static void Vec2ToDLU(short *x, short *y)
{
    SDL_assert(s_BaseUnitsX != 0); /* we init in WIN_ShowMessageBox(), which is the only public function... */

    *x = MulDiv(*x, 4, s_BaseUnitsX);
    *y = MulDiv(*y, 8, s_BaseUnitsY);
}


static SDL_bool AddDialogControl(WIN_DialogData *dialog, WORD type, DWORD style, DWORD exStyle, int x, int y, int w, int h, int id, const char *caption)
{
    DLGITEMTEMPLATEEX item;
    WORD marker = 0xFFFF;
    WORD extraData = 0;

    SDL_zero(item);
    item.style = style;
    item.exStyle = exStyle;
    item.x = x;
    item.y = y;
    item.cx = w;
    item.cy = h;
    item.id = id;

    Vec2ToDLU(&item.x, &item.y);
    Vec2ToDLU(&item.cx, &item.cy);

    if (!AlignDialogData(dialog, sizeof(DWORD))) {
        return SDL_FALSE;
    }
    if (!AddDialogData(dialog, &item, sizeof(item))) {
        return SDL_FALSE;
    }
    if (!AddDialogData(dialog, &marker, sizeof(marker))) {
        return SDL_FALSE;
    }
    if (!AddDialogData(dialog, &type, sizeof(type))) {
        return SDL_FALSE;
    }
    if (!AddDialogString(dialog, caption)) {
        return SDL_FALSE;
    }
    if (!AddDialogData(dialog, &extraData, sizeof(extraData))) {
        return SDL_FALSE;
    }
    ++dialog->lpDialog->cDlgItems;

    return SDL_TRUE;
}

static SDL_bool AddDialogStatic(WIN_DialogData *dialog, int x, int y, int w, int h, const char *text)
{
    DWORD style = WS_VISIBLE | WS_CHILD | SS_LEFT | SS_NOPREFIX | SS_EDITCONTROL;
    return AddDialogControl(dialog, 0x0082, style, 0, x, y, w, h, -1, text);
}

static SDL_bool AddDialogButton(WIN_DialogData *dialog, int x, int y, int w, int h, const char *text, int id, SDL_bool isDefault)
{
    DWORD style = WS_VISIBLE | WS_CHILD;
    if (isDefault) {
        style |= BS_DEFPUSHBUTTON;
    } else {
        style |= BS_PUSHBUTTON;
    }
    return AddDialogControl(dialog, 0x0080, style, 0, x, y, w, h, id, text);
}

static void FreeDialogData(WIN_DialogData *dialog)
{
    SDL_free(dialog->data);
    SDL_free(dialog);
}

static WIN_DialogData *CreateDialogData(int w, int h, const char *caption)
{
    WIN_DialogData *dialog;
    DLGTEMPLATEEX dialogTemplate;
    WORD WordToPass;

    SDL_zero(dialogTemplate);
    dialogTemplate.dlgVer = 1;
    dialogTemplate.signature = 0xffff;
    dialogTemplate.style = (WS_CAPTION | DS_CENTER | DS_SHELLFONT);
    dialogTemplate.x = 0;
    dialogTemplate.y = 0;
    dialogTemplate.cx = w;
    dialogTemplate.cy = h;
    Vec2ToDLU(&dialogTemplate.cx, &dialogTemplate.cy);

    dialog = (WIN_DialogData *)SDL_calloc(1, sizeof(*dialog));
    if (!dialog) {
        return NULL;
    }

    if (!AddDialogData(dialog, &dialogTemplate, sizeof(dialogTemplate))) {
        FreeDialogData(dialog);
        return NULL;
    }

    /* No menu */
    WordToPass = 0;
    if (!AddDialogData(dialog, &WordToPass, 2)) {
        FreeDialogData(dialog);
        return NULL;
    }

    /* No custom class */
    if (!AddDialogData(dialog, &WordToPass, 2)) {
        FreeDialogData(dialog);
        return NULL;
    }

    /* title */
    if (!AddDialogString(dialog, caption)) {
        FreeDialogData(dialog);
        return NULL;
    }

    /* Font stuff */
    {
        /*
         * We want to use the system messagebox font.
         */
        BYTE ToPass;

        NONCLIENTMETRICSA NCM;
        NCM.cbSize = sizeof(NCM);
        SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, 0, &NCM, 0);

        /* Font size - convert to logical font size for dialog parameter. */
        {
            HDC ScreenDC = GetDC(NULL);
            int LogicalPixelsY = GetDeviceCaps(ScreenDC, LOGPIXELSY);
            if (!LogicalPixelsY) /* This can happen if the application runs out of GDI handles */
                LogicalPixelsY = 72;
            WordToPass = (WORD)(-72 * NCM.lfMessageFont.lfHeight / LogicalPixelsY);
            ReleaseDC(NULL, ScreenDC);
        }

        if (!AddDialogData(dialog, &WordToPass, 2)) {
            FreeDialogData(dialog);
            return NULL;
        }

        /* Font weight */
        WordToPass = (WORD)NCM.lfMessageFont.lfWeight;
        if (!AddDialogData(dialog, &WordToPass, 2)) {
            FreeDialogData(dialog);
            return NULL;
        }

        /* italic? */
        ToPass = NCM.lfMessageFont.lfItalic;
        if (!AddDialogData(dialog, &ToPass, 1)) {
            FreeDialogData(dialog);
            return NULL;
        }

        /* charset? */
        ToPass = NCM.lfMessageFont.lfCharSet;
        if (!AddDialogData(dialog, &ToPass, 1)) {
            FreeDialogData(dialog);
            return NULL;
        }

        /* font typeface. */
        if (!AddDialogString(dialog, NCM.lfMessageFont.lfFaceName)) {
            FreeDialogData(dialog);
            return NULL;
        }
    }

    return dialog;
}

int
WIN_ShowMessageBox(const SDL_MessageBoxData *messageboxdata, int *buttonid)
{
    WIN_DialogData *dialog;
    int i, x, y;
    const SDL_MessageBoxButtonData *buttons = messageboxdata->buttons;
    HFONT DialogFont;
    SIZE Size;
    RECT TextSize;
    wchar_t* wmessage;
    TEXTMETRIC TM;

    HWND ParentWindow = NULL;

    const int ButtonWidth = 88;
    const int ButtonHeight = 26;
    const int TextMargin = 16;
    const int ButtonMargin = 12;


    /* Jan 25th, 2013 - dant@fleetsa.com
     *
     *
     * I've tried to make this more reasonable, but I've run in to a lot
     * of nonsense.
     *
     * The original issue is the code was written in pixels and not
     * dialog units (DLUs). All DialogBox functions use DLUs, which
     * vary based on the selected font (yay).
     *
     * According to MSDN, the most reliable way to convert is via
     * MapDialogUnits, which requires an HWND, which we don't have
     * at time of template creation.
     *
     * We do however have:
     *  The system font (DLU width 8 for me)
     *  The font we select for the dialog (DLU width 6 for me)
     *
     * Based on experimentation, *neither* of these return the value
     * actually used. Stepping in to MapDialogUnits(), the conversion
     * is fairly clear, and uses 7 for me.
     *
     * As a result, some of this is hacky to ensure the sizing is
     * somewhat correct.
     *
     * Honestly, a long term solution is to use CreateWindow, not CreateDialog.
     *

     *
     * In order to get text dimensions we need to have a DC with the desired font.
     * I'm assuming a dialog box in SDL is rare enough we can to the create.
     */
    HDC FontDC = CreateCompatibleDC(0);

    {
        /* Create a duplicate of the font used in system message boxes. */
        LOGFONT lf;
        NONCLIENTMETRICS NCM;
        NCM.cbSize = sizeof(NCM);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &NCM, 0);
        lf = NCM.lfMessageFont;
        DialogFont = CreateFontIndirect(&lf);
    }

    /* Select the font in to our DC */
    SelectObject(FontDC, DialogFont);

    {
        /* Get the metrics to try and figure our DLU conversion. */
        GetTextMetrics(FontDC, &TM);

        /* Calculation from the following documentation:
         * https://support.microsoft.com/en-gb/help/125681/how-to-calculate-dialog-base-units-with-non-system-based-font
         * This fixes bug 2137, dialog box calculation with a fixed-width system font
         */
        {
            SIZE extent;
            GetTextExtentPoint32A(FontDC, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &extent);
            s_BaseUnitsX = (extent.cx / 26 + 1) / 2;
        }
        /*s_BaseUnitsX = TM.tmAveCharWidth + 1;*/
        s_BaseUnitsY = TM.tmHeight;
    }

    /* Measure the *pixel* size of the string. */
    wmessage = WIN_UTF8ToString(messageboxdata->message);
    SDL_zero(TextSize);
    DrawText(FontDC, wmessage, -1, &TextSize, DT_CALCRECT);

    /* Add some padding for hangs, etc. */
    TextSize.right += 2;
    TextSize.bottom += 2;

    /* Done with the DC, and the string */
    DeleteDC(FontDC);
    SDL_free(wmessage);

    /* Increase the size of the dialog by some border spacing around the text. */
    Size.cx = TextSize.right - TextSize.left;
    Size.cy = TextSize.bottom - TextSize.top;
    Size.cx += TextMargin * 2;
    Size.cy += TextMargin * 2;

    /* Ensure the size is wide enough for all of the buttons. */
    if (Size.cx < messageboxdata->numbuttons * (ButtonWidth + ButtonMargin) + ButtonMargin)
        Size.cx = messageboxdata->numbuttons * (ButtonWidth + ButtonMargin) + ButtonMargin;

    /* Add vertical space for the buttons and border. */
    Size.cy += ButtonHeight + TextMargin;

    dialog = CreateDialogData(Size.cx, Size.cy, messageboxdata->title);
    if (!dialog) {
        return -1;
    }

    if (!AddDialogStatic(dialog, TextMargin, TextMargin, TextSize.right - TextSize.left, TextSize.bottom - TextSize.top, messageboxdata->message)) {
        FreeDialogData(dialog);
        return -1;
    }

    /* Align the buttons to the right/bottom. */
    x = Size.cx - (ButtonWidth + ButtonMargin) * messageboxdata->numbuttons;
    y = Size.cy - ButtonHeight - ButtonMargin;
    for (i = messageboxdata->numbuttons - 1; i >= 0; --i) {
        SDL_bool isDefault;

        if (buttons[i].flags & SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT) {
            isDefault = SDL_TRUE;
        } else {
            isDefault = SDL_FALSE;
        }
        if (!AddDialogButton(dialog, x, y, ButtonWidth, ButtonHeight, buttons[i].text, buttons[i].buttonid, isDefault)) {
            FreeDialogData(dialog);
            return -1;
        }
        x += ButtonWidth + ButtonMargin;
    }

    /* If we have a parent window, get the Instance and HWND for them
     * so that our little dialog gets exclusive focus at all times. */
    if (messageboxdata->window) {
        ParentWindow = ((SDL_WindowData*)messageboxdata->window->driverdata)->hwnd;
    }

    *buttonid = (int)DialogBoxIndirect(NULL, (DLGTEMPLATE*)dialog->lpDialog, ParentWindow, (DLGPROC)MessageBoxDialogProc);

    FreeDialogData(dialog);
    return 0;
}

#endif /* SDL_VIDEO_DRIVER_WINDOWS */

/* vi: set ts=4 sw=4 expandtab: */
