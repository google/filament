/* See LICENSE.txt for the full license governing this code. */
/**
 * \file windows_screenshot.c 
 *
 * Source file for the screenshot API on windows.
 */

#include "SDL_visualtest_process.h"
#include <SDL.h>
#include <SDL_test.h>

#if defined(__CYGWIN__)
#include <sys/stat.h>
#endif

#if defined(__WIN32__)
#include <Windows.h>

void LogLastError(char* str);

static int img_num;
static SDL_ProcessInfo screenshot_pinfo;

/* Saves a bitmap to a file using hdc as a device context */
static int
SaveBitmapToFile(HDC hdc, HBITMAP hbitmap, char* filename)
{
    BITMAP bitmap;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    DWORD bmpsize, bytes_written;
    HANDLE hdib, hfile;
    char* bmpdata;
    int return_code = 1;

    if(!hdc)
    {
        SDLTest_LogError("hdc argument is NULL");
        return 0;
    }
    if(!hbitmap)
    {
        SDLTest_LogError("hbitmap argument is NULL");
        return 0;
    }
    if(!filename)
    {
        SDLTest_LogError("filename argument is NULL");
        return 0;
    }

    if(!GetObject(hbitmap, sizeof(BITMAP), (void*)&bitmap))
    {
        SDLTest_LogError("GetObject() failed");
        return_code = 0;
        goto savebitmaptofile_cleanup_generic;
    }
    
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = bitmap.bmWidth;
    bih.biHeight = bitmap.bmHeight;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = 0;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    bmpsize = ((bitmap.bmWidth * bih.biBitCount + 31) / 32) * 4 * bitmap.bmHeight;

    hdib = GlobalAlloc(GHND, bmpsize);
    if(!hdib)
    {
        LogLastError("GlobalAlloc() failed");
        return_code = 0;
        goto savebitmaptofile_cleanup_generic;
    }
    bmpdata = (char*)GlobalLock(hdib);
    if(!bmpdata)
    {
        LogLastError("GlobalLock() failed");
        return_code = 0;
        goto savebitmaptofile_cleanup_hdib;
    }

    if(!GetDIBits(hdc, hbitmap, 0, (UINT)bitmap.bmHeight, bmpdata,
                  (LPBITMAPINFO)&bih, DIB_RGB_COLORS))
    {
        SDLTest_LogError("GetDIBits() failed");
        return_code = 0;
        goto savebitmaptofile_cleanup_unlockhdib;
    }

    hfile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    if(hfile == INVALID_HANDLE_VALUE)
    {
        LogLastError("CreateFile()");
        return_code = 0;
        goto savebitmaptofile_cleanup_unlockhdib;
    }
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bmpsize + bfh.bfOffBits;
    bfh.bfType = 0x4D42;

    bytes_written = 0;
    if(!WriteFile(hfile, (void*)&bfh, sizeof(BITMAPFILEHEADER), &bytes_written, NULL) ||
       !WriteFile(hfile, (void*)&bih, sizeof(BITMAPINFOHEADER), &bytes_written, NULL) ||
       !WriteFile(hfile, (void*)bmpdata, bmpsize, &bytes_written, NULL))
    {
        LogLastError("WriteFile() failed");
        return_code = 0;
        goto savebitmaptofile_cleanup_hfile;
    }

savebitmaptofile_cleanup_hfile:
    CloseHandle(hfile);

/* make the screenshot file writable on cygwin, since it could be overwritten later */
#if defined(__CYGWIN__)
    if(chmod(filename, 0777) == -1)
    {
        SDLTest_LogError("chmod() failed");
        return_code = 0;
    }
#endif

savebitmaptofile_cleanup_unlockhdib:
    GlobalUnlock(hdib);

savebitmaptofile_cleanup_hdib:
    GlobalFree(hdib);

savebitmaptofile_cleanup_generic:
    return return_code;
}

/* Takes the screenshot of a window and saves it to a file. If only_client_area
   is true, then only the client area of the window is considered */
static int
ScreenshotWindow(HWND hwnd, char* filename, SDL_bool only_client_area)
{
    int width, height;
    RECT dimensions;
    HDC windowdc, capturedc;
    HBITMAP capturebitmap;
    HGDIOBJ select_success;
    BOOL blt_success;
    int return_code = 1;

    if(!filename)
    {
        SDLTest_LogError("filename argument cannot be NULL");
        return_code = 0;
        goto screenshotwindow_cleanup_generic;
    }
    if(!hwnd)
    {
        SDLTest_LogError("hwnd argument cannot be NULL");
        return_code = 0;
        goto screenshotwindow_cleanup_generic;
    }

    if(!GetWindowRect(hwnd, &dimensions))
    {
        LogLastError("GetWindowRect() failed");
        return_code = 0;
        goto screenshotwindow_cleanup_generic;
    }

    if(only_client_area)
    {
        RECT crect;
        if(!GetClientRect(hwnd, &crect))
        {
            SDLTest_LogError("GetClientRect() failed");
            return_code = 0;
            goto screenshotwindow_cleanup_generic;
        }

        width = crect.right;
        height = crect.bottom;
        windowdc = GetDC(hwnd);
        if(!windowdc)
        {
            SDLTest_LogError("GetDC() failed");
            return_code = 0;
            goto screenshotwindow_cleanup_generic;
        }
    }
    else
    {
        width = dimensions.right - dimensions.left;
        height = dimensions.bottom - dimensions.top;
        windowdc = GetWindowDC(hwnd);
        if(!windowdc)
        {
            SDLTest_LogError("GetWindowDC() failed");
            return_code = 0;
            goto screenshotwindow_cleanup_generic;
        }
    }
    
    capturedc = CreateCompatibleDC(windowdc);
    if(!capturedc)
    {
        SDLTest_LogError("CreateCompatibleDC() failed");
        return_code = 0;
        goto screenshotwindow_cleanup_windowdc;
    }
    capturebitmap = CreateCompatibleBitmap(windowdc, width, height);
    if(!capturebitmap)
    {
        SDLTest_LogError("CreateCompatibleBitmap() failed");
        return_code = 0;
        goto screenshotwindow_cleanup_capturedc;
    }
    select_success = SelectObject(capturedc, capturebitmap);
    if(!select_success || select_success == HGDI_ERROR)
    {
        SDLTest_LogError("SelectObject() failed");
        return_code = 0;
        goto screenshotwindow_cleanup_capturebitmap;
    }
    blt_success = BitBlt(capturedc, 0, 0, width, height, windowdc,
                         0, 0, SRCCOPY|CAPTUREBLT);
    if(!blt_success)
    {
        LogLastError("BitBlt() failed");
        return_code = 0;
        goto screenshotwindow_cleanup_capturebitmap;
    }

    /* save bitmap as file */
    if(!SaveBitmapToFile(windowdc, capturebitmap, filename))
    {
        SDLTest_LogError("SaveBitmapToFile() failed");
        return_code = 0;
        goto screenshotwindow_cleanup_capturebitmap;
    }

    /* free resources */

screenshotwindow_cleanup_capturebitmap:
    if(!DeleteObject(capturebitmap))
    {
        SDLTest_LogError("DeleteObjectFailed");
        return_code = 0;
    }

screenshotwindow_cleanup_capturedc:
    if(!DeleteDC(capturedc))
    {
        SDLTest_LogError("DeleteDC() failed");
        return_code = 0;
    }

screenshotwindow_cleanup_windowdc:
    if(!ReleaseDC(hwnd, windowdc))
    {
        SDLTest_LogError("ReleaseDC() failed");
        return_code = 0;;
    }

screenshotwindow_cleanup_generic:
    return return_code;
}

/* Takes the screenshot of the entire desktop and saves it to a file */
int SDLVisualTest_ScreenshotDesktop(char* filename)
{
    HWND hwnd;
    hwnd = GetDesktopWindow();
    return ScreenshotWindow(hwnd, filename, SDL_FALSE);
}

/* take screenshot of a window and save it to a file */
static BOOL CALLBACK
ScreenshotHwnd(HWND hwnd, LPARAM lparam)
{
    int len;
    DWORD pid;
    char* prefix;
    char* filename;

    GetWindowThreadProcessId(hwnd, &pid);
    if(pid != screenshot_pinfo.pi.dwProcessId)
        return TRUE;

    if(!IsWindowVisible(hwnd))
        return TRUE;

    prefix = (char*)lparam;
    len = SDL_strlen(prefix) + 100;
    filename = (char*)SDL_malloc(len * sizeof(char));
    if(!filename)
    {
        SDLTest_LogError("malloc() failed");
        return FALSE;
    }

    /* restore the window and bring it to the top */
    ShowWindowAsync(hwnd, SW_RESTORE);
    /* restore is not instantaneous */
    SDL_Delay(500);

    /* take a screenshot of the client area */
    if(img_num == 1)
        SDL_snprintf(filename, len, "%s.bmp", prefix);
    else
        SDL_snprintf(filename, len, "%s_%d.bmp", prefix, img_num);
    img_num++;
    ScreenshotWindow(hwnd, filename, SDL_TRUE);

    SDL_free(filename);
    return TRUE;
}


/* each window of the process will have a screenshot taken. The file name will be
   prefix-i.png for the i'th window. */
int
SDLVisualTest_ScreenshotProcess(SDL_ProcessInfo* pinfo, char* prefix)
{
    if(!pinfo)
    {
        SDLTest_LogError("pinfo argument cannot be NULL");
        return 0;
    }
    if(!prefix)
    {
        SDLTest_LogError("prefix argument cannot be NULL");
        return 0;
    }

    img_num = 1;
    screenshot_pinfo = *pinfo;
    if(!EnumWindows(ScreenshotHwnd, (LPARAM)prefix))
    {
        SDLTest_LogError("EnumWindows() failed");
        return 0;
    }

    return 1;
}

#endif
