/* See LICENSE.txt for the full license governing this code. */
/**
 * \file SDL_visualtest_process.h
 *
 * Provides cross-platfrom process launching and termination functionality.
 */

#include <SDL_platform.h>

#if defined(__WIN32__)
#include <Windows.h>
#include <Shlwapi.h>
#elif defined(__LINUX__)
#include <unistd.h>
#else
#error "Unsupported platform."
#endif

#ifndef SDL_visualtest_process_h_
#define SDL_visualtest_process_h_

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Struct to store a platform specific handle to a process.
 */
typedef struct SDL_ProcessInfo
{
//#if defined(_WIN32) || defined(__WIN32__)
#if defined(__WIN32__)
    PROCESS_INFORMATION pi;
//#elif defined(__linux__)
#elif defined(__LINUX__)
    int pid;
#endif
} SDL_ProcessInfo;

/**
 * This structure stores the exit status (value returned by main()) and
 * whether the process exited sucessfully or not.
 */
typedef struct SDL_ProcessExitStatus
{
    int exit_success;   /*!< Zero if the process exited successfully */
    int exit_status;    /*!< The exit status of the process. 8-bit value. */
} SDL_ProcessExitStatus;

/**
 * Launches a process with the given commandline arguments.
 *
 * \param file  The path to the executable to be launched.
 * \param args  The command line arguments to be passed to the process.
 * \param pinfo Pointer to an SDL_ProcessInfo object to be populated with
 *              platform specific information about the launched process.
 *
 * \return Non-zero on success, zero on failure.
 */
int SDL_LaunchProcess(char* file, char* args, SDL_ProcessInfo* pinfo);

/**
 * Checks if a process is running or not.
 *
 * \param pinfo Pointer to SDL_ProcessInfo object of the process that needs to be
 *              checked.
 *
 * \return 1 if the process is still running; zero if it is not and -1 if the
 *         status could not be retrieved.
 */
int SDL_IsProcessRunning(SDL_ProcessInfo* pinfo);

/**
 * Kills a currently running process.
 *
 * \param pinfo Pointer to a SDL_ProcessInfo object of the process to be terminated.
 * \param ps Pointer to a SDL_ProcessExitStatus object which will be populated
 *           with the exit status.
 *
 * \return 1 on success, 0 on failure.
 */
int SDL_KillProcess(SDL_ProcessInfo* pinfo, SDL_ProcessExitStatus* ps);

/**
 * Cleanly exits the process represented by \c pinfo and stores the exit status
 * in the exit status object pointed to by \c ps.
 *
 * \return 1 on success, 0 on failure.
 */
int SDL_QuitProcess(SDL_ProcessInfo* pinfo, SDL_ProcessExitStatus* ps);

/**
 * Gets the exit status of a process. If the exit status is -1, the process is
 * still running.
 *
 * \param pinfo Pointer to a SDL_ProcessInfo object of the process to be checked.
 * \param ps Pointer to a SDL_ProcessExitStatus object which will be populated
 *           with the exit status.
 *
 * \return 1 on success, 0 on failure.
 */
int SDL_GetProcessExitStatus(SDL_ProcessInfo* pinfo, SDL_ProcessExitStatus* ps);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_process_h_ */

/* vi: set ts=4 sw=4 expandtab: */
