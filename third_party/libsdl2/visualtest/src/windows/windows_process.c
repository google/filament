/* See LICENSE.txt for the full license governing this code. */
/**
 * \file windows_process.c 
 *
 * Source file for the process API on windows.
 */


#include <SDL.h>
#include <SDL_test.h>
#include <string.h>
#include <stdlib.h>

#include "SDL_visualtest_process.h"

#if defined(__WIN32__)

void
LogLastError(char* str)
{
    LPVOID buffer;
    DWORD dw = GetLastError();
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|
                    FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer,
                    0, NULL);
    SDLTest_LogError("%s: %s", str, (char*)buffer);
    LocalFree(buffer);
}

int
SDL_LaunchProcess(char* file, char* args, SDL_ProcessInfo* pinfo)
{
    BOOL success;
    char* working_directory;
    char* command_line;
    int path_length, args_length;
    STARTUPINFO sui = {0};
    sui.cb = sizeof(sui);

    if(!file)
    {
        SDLTest_LogError("Path to executable to launched cannot be NULL.");
        return 0;
    }
    if(!pinfo)
    {
        SDLTest_LogError("pinfo cannot be NULL.");
        return 0;
    }

    /* get the working directory of the process being launched, so that
        the process can load any resources it has in it's working directory */
    path_length = SDL_strlen(file);
    if(path_length == 0)
    {
        SDLTest_LogError("Length of the file parameter is zero.");
        return 0;
    }

    working_directory = (char*)SDL_malloc(path_length + 1);
    if(!working_directory)
    {
        SDLTest_LogError("Could not allocate working_directory - malloc() failed.");
        return 0;
    }

    SDL_memcpy(working_directory, file, path_length + 1);
    PathRemoveFileSpec(working_directory);
    if(SDL_strlen(working_directory) == 0)
    {
        SDL_free(working_directory);
        working_directory = NULL;
    }

    /* join the file path and the args string together */
    if(!args)
        args = "";
    args_length = SDL_strlen(args);
    command_line = (char*)SDL_malloc(path_length + args_length + 2);
    if(!command_line)
    {
        SDLTest_LogError("Could not allocate command_line - malloc() failed.");
        return 0;
    }
    SDL_memcpy(command_line, file, path_length);
    command_line[path_length] = ' ';
    SDL_memcpy(command_line + path_length + 1, args, args_length + 1);

    /* create the process */
    success = CreateProcess(NULL, command_line, NULL, NULL, FALSE,
                            NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                            NULL, working_directory, &sui, &pinfo->pi);
    if(working_directory)
    {
        SDL_free(working_directory);
        working_directory = NULL;
    }
    SDL_free(command_line);
    if(!success)
    {
        LogLastError("CreateProcess() failed");
        return 0;
    }

    return 1;
}

int
SDL_GetProcessExitStatus(SDL_ProcessInfo* pinfo, SDL_ProcessExitStatus* ps)
{
    DWORD exit_status;
    BOOL success;

    if(!pinfo)
    {
        SDLTest_LogError("pinfo cannot be NULL");
        return 0;
    }
    if(!ps)
    {
        SDLTest_LogError("ps cannot be NULL");
        return 0;
    }

    /* get the exit code */
    success = GetExitCodeProcess(pinfo->pi.hProcess, &exit_status);
    if(!success)
    {
        LogLastError("GetExitCodeProcess() failed");
        return 0;
    }

    if(exit_status == STILL_ACTIVE)
        ps->exit_status = -1;
    else
        ps->exit_status = exit_status;
    ps->exit_success = 1;
    return 1;
}


int
SDL_IsProcessRunning(SDL_ProcessInfo* pinfo)
{
    DWORD exit_status;
    BOOL success;

    if(!pinfo)
    {
        SDLTest_LogError("pinfo cannot be NULL");
        return -1;
    }

    success = GetExitCodeProcess(pinfo->pi.hProcess, &exit_status);
    if(!success)
    {
        LogLastError("GetExitCodeProcess() failed");
        return -1;
    }
    
    if(exit_status == STILL_ACTIVE)
        return 1;
    return 0;
}

static BOOL CALLBACK
CloseWindowCallback(HWND hwnd, LPARAM lparam)
{
    DWORD pid;
    SDL_ProcessInfo* pinfo;

    pinfo = (SDL_ProcessInfo*)lparam;

    GetWindowThreadProcessId(hwnd, &pid);
    if(pid == pinfo->pi.dwProcessId)
    {
        DWORD result;
        if(!SendMessageTimeout(hwnd, WM_CLOSE, 0, 0, SMTO_BLOCK,
                               1000, &result))
        {
            if(GetLastError() != ERROR_TIMEOUT)
            {
                LogLastError("SendMessageTimeout() failed");
                return FALSE;
            }
        }
    }
    return TRUE;
}

int
SDL_QuitProcess(SDL_ProcessInfo* pinfo, SDL_ProcessExitStatus* ps)
{
    DWORD wait_result;
    if(!pinfo)
    {
        SDLTest_LogError("pinfo argument cannot be NULL");
        return 0;
    }
    if(!ps)
    {
        SDLTest_LogError("ps argument cannot be NULL");
        return 0;
    }

    /* enumerate through all the windows, trying to close each one */
    if(!EnumWindows(CloseWindowCallback, (LPARAM)pinfo))
    {
        SDLTest_LogError("EnumWindows() failed");
        return 0;
    }

    /* wait until the process terminates */
    wait_result = WaitForSingleObject(pinfo->pi.hProcess, 1000);
    if(wait_result == WAIT_FAILED)
    {
        LogLastError("WaitForSingleObject() failed");
        return 0;
    }
    if(wait_result != WAIT_OBJECT_0)
    {
        SDLTest_LogError("Process did not quit.");
        return 0;
    }

    /* get the exit code */
    if(!SDL_GetProcessExitStatus(pinfo, ps))
    {
        SDLTest_LogError("SDL_GetProcessExitStatus() failed");
        return 0;
    }

    return 1;
}

int
SDL_KillProcess(SDL_ProcessInfo* pinfo, SDL_ProcessExitStatus* ps)
{
    BOOL success;
    DWORD exit_status, wait_result;

    if(!pinfo)
    {
        SDLTest_LogError("pinfo argument cannot be NULL");
        return 0;
    }
    if(!ps)
    {
        SDLTest_LogError("ps argument cannot be NULL");
        return 0;
    }

    /* initiate termination of the process */
    success = TerminateProcess(pinfo->pi.hProcess, 0);
    if(!success)
    {
        LogLastError("TerminateProcess() failed");
        return 0;
    }

    /* wait until the process terminates */
    wait_result = WaitForSingleObject(pinfo->pi.hProcess, INFINITE);
    if(wait_result == WAIT_FAILED)
    {
        LogLastError("WaitForSingleObject() failed");
        return 0;
    }

    /* get the exit code */
    success = GetExitCodeProcess(pinfo->pi.hProcess, &exit_status);
    if(!success)
    {
        LogLastError("GetExitCodeProcess() failed");
        return 0;
    }

    ps->exit_status = exit_status;
    ps->exit_success = 1;

    return 1;
}

#endif
