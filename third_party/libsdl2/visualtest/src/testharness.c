/* See LICENSE.txt for the full license governing this code. */
/**
 *  \file testharness.c 
 *
 *  Source file for the test harness.
 */

#include <stdlib.h>
#include <SDL_test.h>
#include <SDL.h>
#include <SDL_assert.h>
#include "SDL_visualtest_harness_argparser.h"
#include "SDL_visualtest_process.h"
#include "SDL_visualtest_variators.h"
#include "SDL_visualtest_screenshot.h"
#include "SDL_visualtest_mischelper.h"

#if defined(__WIN32__) && !defined(__CYGWIN__)
#include <direct.h>
#elif defined(__WIN32__) && defined(__CYGWIN__)
#include <signal.h>
#elif defined(__LINUX__)
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#else
#error "Unsupported platform"
#endif

/** Code for the user event triggered when a new action is to be executed */
#define ACTION_TIMER_EVENT 0
/** Code for the user event triggered when the maximum timeout is reached */
#define KILL_TIMER_EVENT 1
/** FPS value used for delays in the action loop */
#define ACTION_LOOP_FPS 10

/** Value returned by RunSUTAndTest() when the test has passed */
#define TEST_PASSED 1
/** Value returned by RunSUTAndTest() when the test has failed */
#define TEST_FAILED 0
/** Value returned by RunSUTAndTest() on a fatal error */
#define TEST_ERROR -1

static SDL_ProcessInfo pinfo;
static SDL_ProcessExitStatus sut_exitstatus;
static SDLVisualTest_HarnessState state;
static SDLVisualTest_Variator variator;
static SDLVisualTest_ActionNode* current; /* the current action being performed */
static SDL_TimerID action_timer, kill_timer;

/* returns a char* to be passed as the format argument of a printf-style function. */
static char*
usage()
{
    return "Usage: \n%s --sutapp xyz"
           " [--sutargs abc | --parameter-config xyz.parameters"
           " [--variator exhaustive|random]" 
           " [--num-variations N] [--no-launch]] [--timeout hh:mm:ss]"
           " [--action-config xyz.actions]"
           " [--output-dir /path/to/output]"
           " [--verify-dir /path/to/verify]"
           " or --config app.config";
}

/* register Ctrl+C handlers */
#if defined(__LINUX__) || defined(__CYGWIN__)
static void
CtrlCHandlerCallback(int signum)
{
    SDL_Event event;
    SDLTest_Log("Ctrl+C received");
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}
#endif

static Uint32
ActionTimerCallback(Uint32 interval, void* param)
{
    SDL_Event event;
    SDL_UserEvent userevent;
    Uint32 next_action_time;

    /* push an event to handle the action */
    userevent.type = SDL_USEREVENT;
    userevent.code = ACTION_TIMER_EVENT;
    userevent.data1 = &current->action;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;
    SDL_PushEvent(&event);

    /* calculate the new interval and return it */
    if(current->next)
        next_action_time = current->next->action.time - current->action.time;
    else
    {
        next_action_time = 0;
        action_timer = 0;
    }

    current = current->next;
    return next_action_time;
}

static Uint32
KillTimerCallback(Uint32 interval, void* param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = KILL_TIMER_EVENT;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;
    SDL_PushEvent(&event);

    kill_timer = 0;
    return 0;
}

static int
ProcessAction(SDLVisualTest_Action* action, int* sut_running, char* args)
{
    if(!action || !sut_running)
        return TEST_ERROR;

    switch(action->type)
    {
        case SDL_ACTION_KILL:
            SDLTest_Log("Action: Kill SUT");
            if(SDL_IsProcessRunning(&pinfo) == 1 &&
               !SDL_KillProcess(&pinfo, &sut_exitstatus))
            {
                SDLTest_LogError("SDL_KillProcess() failed");
                return TEST_ERROR;
            }
            *sut_running = 0;
        break;

        case SDL_ACTION_QUIT:
            SDLTest_Log("Action: Quit SUT");
            if(SDL_IsProcessRunning(&pinfo) == 1 &&
               !SDL_QuitProcess(&pinfo, &sut_exitstatus))
            {
                SDLTest_LogError("SDL_QuitProcess() failed");
                return TEST_FAILED;
            }
            *sut_running = 0;
        break;

        case SDL_ACTION_LAUNCH:
        {
            char* path;
            char* args;
            SDL_ProcessInfo action_process;
            SDL_ProcessExitStatus ps;

            path = action->extra.process.path;
            args = action->extra.process.args;
            if(args)
            {
                SDLTest_Log("Action: Launch process: %s with arguments: %s",
                            path, args);
            }
            else
                SDLTest_Log("Action: Launch process: %s", path);
            if(!SDL_LaunchProcess(path, args, &action_process))
            {
                SDLTest_LogError("SDL_LaunchProcess() failed");
                return TEST_ERROR;
            }

            /* small delay so that the process can do its job */
            SDL_Delay(1000);

            if(SDL_IsProcessRunning(&action_process) > 0)
            {
                SDLTest_LogError("Process %s took too long too complete."
                                    " Force killing...", action->extra);
                if(!SDL_KillProcess(&action_process, &ps))
                {
                    SDLTest_LogError("SDL_KillProcess() failed");
                    return TEST_ERROR;
                }
            }
        }
        break;

        case SDL_ACTION_SCREENSHOT:
        {
            char path[MAX_PATH_LEN], hash[33];

            SDLTest_Log("Action: Take screenshot");
            /* can't take a screenshot if the SUT isn't running */
            if(SDL_IsProcessRunning(&pinfo) != 1)
            {
                SDLTest_LogError("SUT has quit.");
                *sut_running = 0;
                return TEST_FAILED;
            }

            /* file name for the screenshot image */
            SDLVisualTest_HashString(args, hash);
            SDL_snprintf(path, MAX_PATH_LEN, "%s/%s", state.output_dir, hash);
            if(!SDLVisualTest_ScreenshotProcess(&pinfo, path))
            {
                SDLTest_LogError("SDLVisualTest_ScreenshotProcess() failed");
                return TEST_ERROR;
            }
        }
        break;

        case SDL_ACTION_VERIFY:
        {
            int ret;

            SDLTest_Log("Action: Verify screenshot");
            ret = SDLVisualTest_VerifyScreenshots(args, state.output_dir,
                                                  state.verify_dir);

            if(ret == -1)
            {
                SDLTest_LogError("SDLVisualTest_VerifyScreenshots() failed");
                return TEST_ERROR;
            }
            else if(ret == 0)
            {
                SDLTest_Log("Verification failed: Images were not equal.");
                return TEST_FAILED;
            }
            else if(ret == 1)
                SDLTest_Log("Verification successful.");
            else
            {
                SDLTest_Log("Verfication skipped.");
                return TEST_FAILED;
            }
        }
        break;

        default:
            SDLTest_LogError("Invalid action type");
            return TEST_ERROR;
        break;
    }

    return TEST_PASSED;
}

static int
RunSUTAndTest(char* sutargs, int variation_num)
{
    int success, sut_running, return_code;
    char hash[33];
    SDL_Event event;

    return_code = TEST_PASSED;

    if(!sutargs)
    {
        SDLTest_LogError("sutargs argument cannot be NULL");
        return_code = TEST_ERROR;
        goto runsutandtest_cleanup_generic;
    }

    SDLVisualTest_HashString(sutargs, hash);
    SDLTest_Log("Hash: %s", hash);

    success = SDL_LaunchProcess(state.sutapp, sutargs, &pinfo);
    if(!success)
    {
        SDLTest_Log("Could not launch SUT.");
        return_code = TEST_ERROR;
        goto runsutandtest_cleanup_generic;
    }
    SDLTest_Log("SUT launch successful.");
    SDLTest_Log("Process will be killed in %d milliseconds", state.timeout);
    sut_running = 1;

    /* launch the timers */
    SDLTest_Log("Performing actions..");
    current = state.action_queue.front;
    action_timer = 0;
    kill_timer = 0;
    if(current)
    {
        action_timer = SDL_AddTimer(current->action.time, ActionTimerCallback, NULL);
        if(!action_timer)
        {
            SDLTest_LogError("SDL_AddTimer() failed");
            return_code = TEST_ERROR;
            goto runsutandtest_cleanup_timer;
        }
    }
    kill_timer = SDL_AddTimer(state.timeout, KillTimerCallback, NULL);
    if(!kill_timer)
    {
        SDLTest_LogError("SDL_AddTimer() failed");
        return_code = TEST_ERROR;
        goto runsutandtest_cleanup_timer;
    }

    /* the timer stops running if the actions queue is empty, and the
       SUT stops running if it crashes or if we encounter a KILL/QUIT action */
    while(sut_running)
    {
        /* process the actions by using an event queue */
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_USEREVENT)
            {
                if(event.user.code == ACTION_TIMER_EVENT)
                {
                    SDLVisualTest_Action* action;

                    action = (SDLVisualTest_Action*)event.user.data1;

                    switch(ProcessAction(action, &sut_running, sutargs))
                    {
                        case TEST_PASSED:
                        break;

                        case TEST_FAILED:
                            return_code = TEST_FAILED;
                            goto runsutandtest_cleanup_timer;
                        break;

                        default:
                            SDLTest_LogError("ProcessAction() failed");
                            return_code = TEST_ERROR;
                            goto runsutandtest_cleanup_timer;
                    }
                }
                else if(event.user.code == KILL_TIMER_EVENT)
                {
                    SDLTest_LogError("Maximum timeout reached. Force killing..");
                    return_code = TEST_FAILED;
                    goto runsutandtest_cleanup_timer;
                }
            }
            else if(event.type == SDL_QUIT)
            {
                SDLTest_LogError("Received QUIT event. Testharness is quitting..");
                return_code = TEST_ERROR;
                goto runsutandtest_cleanup_timer;
            }
        }
        SDL_Delay(1000/ACTION_LOOP_FPS);
    }

    SDLTest_Log("SUT exit code was: %d", sut_exitstatus.exit_status);
    if(sut_exitstatus.exit_status == 0)
    {
        return_code = TEST_PASSED;
        goto runsutandtest_cleanup_timer;
    }
    else
    {
        return_code = TEST_FAILED;
        goto runsutandtest_cleanup_timer;
    }

    return_code = TEST_ERROR;
    goto runsutandtest_cleanup_generic;

runsutandtest_cleanup_timer:
    if(action_timer && !SDL_RemoveTimer(action_timer))
    {
        SDLTest_Log("SDL_RemoveTimer() failed");
        return_code = TEST_ERROR;
    }

    if(kill_timer && !SDL_RemoveTimer(kill_timer))
    {
        SDLTest_Log("SDL_RemoveTimer() failed");
        return_code = TEST_ERROR;
    }
/* runsutandtest_cleanup_process: */
    if(SDL_IsProcessRunning(&pinfo) && !SDL_KillProcess(&pinfo, &sut_exitstatus))
    {
        SDLTest_Log("SDL_KillProcess() failed");
        return_code = TEST_ERROR;
    }
runsutandtest_cleanup_generic:
    return return_code;
}

/** Entry point for testharness */
int
main(int argc, char* argv[])
{
    int i, passed, return_code, failed;

    /* freeing resources, linux style! */
    return_code = 0;

    if(argc < 2)
    {
        SDLTest_Log(usage(), argv[0]);
        goto cleanup_generic;
    }

#if defined(__LINUX__) || defined(__CYGWIN__)
    signal(SIGINT, CtrlCHandlerCallback);
#endif

    /* parse arguments */
    if(!SDLVisualTest_ParseHarnessArgs(argv + 1, &state))
    {
        SDLTest_Log(usage(), argv[0]);
        return_code = 1;
        goto cleanup_generic;
    }
    SDLTest_Log("Parsed harness arguments successfully.");

    /* initialize SDL */
    if(SDL_Init(SDL_INIT_TIMER) == -1)
    {
        SDLTest_LogError("SDL_Init() failed.");
        SDLVisualTest_FreeHarnessState(&state);
        return_code = 1;
        goto cleanup_harness_state;
    }

    /* create an output directory if none exists */
#if defined(__LINUX__) || defined(__CYGWIN__)
    mkdir(state.output_dir, 0777);
#elif defined(__WIN32__)
    _mkdir(state.output_dir);
#else
#error "Unsupported platform"
#endif

    /* test with sutargs */
    if(SDL_strlen(state.sutargs))
    {
        SDLTest_Log("Running: %s %s", state.sutapp, state.sutargs);
        if(!state.no_launch)
        {
            switch(RunSUTAndTest(state.sutargs, 0))
            {
                case TEST_PASSED:
                    SDLTest_Log("Status: PASSED");
                break;

                case TEST_FAILED:
                    SDLTest_Log("Status: FAILED");
                break;

                case TEST_ERROR:
                    SDLTest_LogError("Some error occurred while testing.");
                    return_code = 1;
                    goto cleanup_sdl;
                break;
            }
        }
    }

    if(state.sut_config.num_options > 0)
    {
        char* variator_name = state.variator_type == SDL_VARIATOR_RANDOM ?
                              "RANDOM" : "EXHAUSTIVE";
        if(state.num_variations > 0)
            SDLTest_Log("Testing SUT with variator: %s for %d variations",
                        variator_name, state.num_variations);
        else
            SDLTest_Log("Testing SUT with variator: %s and ALL variations",
                        variator_name);
        /* initialize the variator */
        if(!SDLVisualTest_InitVariator(&variator, &state.sut_config,
                                       state.variator_type, 0))
        {
            SDLTest_LogError("Could not initialize variator");
            return_code = 1;
            goto cleanup_sdl;
        }

        /* iterate through all the variations */
        passed = 0;
        failed = 0;
        for(i = 0; state.num_variations > 0 ? (i < state.num_variations) : 1; i++)
        {
            char* args = SDLVisualTest_GetNextVariation(&variator);
            if(!args)
                break;
            SDLTest_Log("\nVariation number: %d\nArguments: %s", i + 1, args);

            if(!state.no_launch)
            {
                switch(RunSUTAndTest(args, i + 1))
                {
                    case TEST_PASSED:
                        SDLTest_Log("Status: PASSED");
                        passed++;
                    break;

                    case TEST_FAILED:
                        SDLTest_Log("Status: FAILED");
                        failed++;
                    break;

                    case TEST_ERROR:
                        SDLTest_LogError("Some error occurred while testing.");
                        goto cleanup_variator;
                    break;
                }
            }
        }
        if(!state.no_launch)
        {
            /* report stats */
            SDLTest_Log("Testing complete.");
            SDLTest_Log("%d/%d tests passed.", passed, passed + failed);
        }
        goto cleanup_variator;
    }
 
    goto cleanup_sdl;

cleanup_variator:
    SDLVisualTest_FreeVariator(&variator);
cleanup_sdl:
    SDL_Quit();
cleanup_harness_state:
    SDLVisualTest_FreeHarnessState(&state);
cleanup_generic:
    return return_code;
}
