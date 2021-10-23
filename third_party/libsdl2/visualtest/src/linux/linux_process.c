/* See LICENSE.txt for the full license governing this code. */
/**
 *  \file linux_process.c
 * 
 *  Source file for the process API on linux.
 */


#include <SDL.h>
#include <SDL_test.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "SDL_visualtest_process.h"
#include "SDL_visualtest_harness_argparser.h"
#include "SDL_visualtest_parsehelper.h"

#if defined(__LINUX__)

static void
LogLastError(char* str)
{
    char* error = (char*)strerror(errno);
    if(!str || !error)
    	return;
    SDLTest_LogError("%s: %s", str, error);
}

int
SDL_LaunchProcess(char* file, char* args, SDL_ProcessInfo* pinfo)
{
    pid_t pid;
    char** argv;

    if(!file)
    {
        SDLTest_LogError("file argument cannot be NULL");
        return 0;
    }
    if(!pinfo)
    {
        SDLTest_LogError("pinfo cannot be NULL");
        return 0;
    }
    pid = fork();
    if(pid == -1)
    {
        LogLastError("fork() failed");
        return 0;
    }
    else if(pid == 0)
    {
        /* parse the arguments string */
        argv = SDLVisualTest_ParseArgsToArgv(args);
        argv[0] = file;
        execv(file, argv);
        LogLastError("execv() failed");
        return 0;
    }
    else
    {
        pinfo->pid = pid;
        return 1;
    }

    /* never executed */
    return 0;
}

int
SDL_GetProcessExitStatus(SDL_ProcessInfo* pinfo, SDL_ProcessExitStatus* ps)
{
    int success, status;
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
    success = waitpid(pinfo->pid, &status, WNOHANG);
    if(success == -1)
    {
        LogLastError("waitpid() failed");
        return 0;
    }
    else if(success == 0)
    {
        ps->exit_status = -1;
        ps->exit_success = 1;
    }
    else
    {
        ps->exit_success = WIFEXITED(status);
        ps->exit_status = WEXITSTATUS(status);
    }
    return 1;
}

int
SDL_IsProcessRunning(SDL_ProcessInfo* pinfo)
{
    int success;

    if(!pinfo)
    {
        SDLTest_LogError("pinfo cannot be NULL");
        return -1;
    }

    success = kill(pinfo->pid, 0);
    if(success == -1)
    {
        if(errno == ESRCH) /* process is not running */
            return 0;
        else
        {
            LogLastError("kill() failed");
            return -1;
        }
    }
    return 1;
}

int
SDL_QuitProcess(SDL_ProcessInfo* pinfo, SDL_ProcessExitStatus* ps)
{
    int success, status;

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

    success = kill(pinfo->pid, SIGQUIT);
    if(success == -1)
    {
        LogLastError("kill() failed");
        return 0;
    }

    success = waitpid(pinfo->pid, &status, 0);
    if(success == -1)
    {
        LogLastError("waitpid() failed");
        return 0;
    }

    ps->exit_success = WIFEXITED(status);
    ps->exit_status = WEXITSTATUS(status);
    return 1;
}

int
SDL_KillProcess(SDL_ProcessInfo* pinfo, SDL_ProcessExitStatus* ps)
{
    int success, status;

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

    success = kill(pinfo->pid, SIGKILL);
    if(success == -1)
    {
        LogLastError("kill() failed");
        return 0;
    }
    success = waitpid(pinfo->pid, &status, 0);
    if(success == -1)
    {
        LogLastError("waitpid() failed");
        return 0;
    }

    ps->exit_success = WIFEXITED(status);
    ps->exit_status = WEXITSTATUS(status);
    return 1;
}

#endif
