/**
 * Automated SDL subsystems management test.
 *
 * Written by J�rgen Tjern� "jorgenpt"
 *
 * Released under Public Domain.
 */

#include "SDL.h"
#include "SDL_test.h"


/* !
 * \brief Tests SDL_Init() and SDL_Quit() of Joystick and Haptic subsystems
 * \sa
 * http://wiki.libsdl.org/SDL_Init
 * http://wiki.libsdl.org/SDL_Quit
 */
static int main_testInitQuitJoystickHaptic (void *arg)
{
#if defined SDL_JOYSTICK_DISABLED || defined SDL_HAPTIC_DISABLED
    return TEST_SKIPPED;
#else
    int enabled_subsystems;
    int initialized_subsystems = SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC;

    SDLTest_AssertCheck( SDL_Init(initialized_subsystems) == 0, "SDL_Init multiple systems." );

    enabled_subsystems = SDL_WasInit(initialized_subsystems);
    SDLTest_AssertCheck( enabled_subsystems == initialized_subsystems, "SDL_WasInit(SDL_INIT_EVERYTHING) contains all systems (%i)", enabled_subsystems );

    SDL_Quit();

    enabled_subsystems = SDL_WasInit(initialized_subsystems);
    SDLTest_AssertCheck( enabled_subsystems == 0, "SDL_Quit should shut down everything (%i)", enabled_subsystems );

    return TEST_COMPLETED;
#endif
}

/* !
 * \brief Tests SDL_InitSubSystem() and SDL_QuitSubSystem()
 * \sa
 * http://wiki.libsdl.org/SDL_Init
 * http://wiki.libsdl.org/SDL_Quit
 */
static int main_testInitQuitSubSystem (void *arg)
{
#if defined SDL_JOYSTICK_DISABLED || defined SDL_HAPTIC_DISABLED || defined SDL_GAMECONTROLLER_DISABLED
    return TEST_SKIPPED;
#else
    int i;
    int subsystems[] = { SDL_INIT_JOYSTICK, SDL_INIT_HAPTIC, SDL_INIT_GAMECONTROLLER };

    for (i = 0; i < SDL_arraysize(subsystems); ++i) {
        int initialized_system;
        int subsystem = subsystems[i];

        SDLTest_AssertCheck( (SDL_WasInit(subsystem) & subsystem) == 0, "SDL_WasInit(%x) before init should be false", subsystem );
        SDLTest_AssertCheck( SDL_InitSubSystem(subsystem) == 0, "SDL_InitSubSystem(%x)", subsystem );

        initialized_system = SDL_WasInit(subsystem);
        SDLTest_AssertCheck( (initialized_system & subsystem) != 0, "SDL_WasInit(%x) should be true (%x)", subsystem, initialized_system );

        SDL_QuitSubSystem(subsystem);

        SDLTest_AssertCheck( (SDL_WasInit(subsystem) & subsystem) == 0, "SDL_WasInit(%x) after shutdown should be false", subsystem );
    }

    return TEST_COMPLETED;
#endif
}

const int joy_and_controller = SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;
static int main_testImpliedJoystickInit (void *arg)
{
#if defined SDL_JOYSTICK_DISABLED || defined SDL_GAMECONTROLLER_DISABLED
    return TEST_SKIPPED;
#else
    int initialized_system;

    /* First initialize the controller */
    SDLTest_AssertCheck( (SDL_WasInit(joy_and_controller) & joy_and_controller) == 0, "SDL_WasInit() before init should be false for joystick & controller" );
    SDLTest_AssertCheck( SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0, "SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER)" );

    /* Then make sure this implicitly initialized the joystick subsystem */
    initialized_system = SDL_WasInit(joy_and_controller);
    SDLTest_AssertCheck( (initialized_system & joy_and_controller) == joy_and_controller, "SDL_WasInit() should be true for joystick & controller (%x)", initialized_system );

    /* Then quit the controller, and make sure that implicitly also quits the */
    /* joystick subsystem */
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    initialized_system = SDL_WasInit(joy_and_controller);
    SDLTest_AssertCheck( (initialized_system & joy_and_controller) == 0, "SDL_WasInit() should be false for joystick & controller (%x)", initialized_system );

    return TEST_COMPLETED;
#endif
}

static int main_testImpliedJoystickQuit (void *arg)
{
#if defined SDL_JOYSTICK_DISABLED || defined SDL_GAMECONTROLLER_DISABLED
    return TEST_SKIPPED;
#else
    int initialized_system;

    /* First initialize the controller and the joystick (explicitly) */
    SDLTest_AssertCheck( (SDL_WasInit(joy_and_controller) & joy_and_controller) == 0, "SDL_WasInit() before init should be false for joystick & controller" );
    SDLTest_AssertCheck( SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0, "SDL_InitSubSystem(SDL_INIT_JOYSTICK)" );
    SDLTest_AssertCheck( SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0, "SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER)" );

    /* Then make sure they're both initialized properly */
    initialized_system = SDL_WasInit(joy_and_controller);
    SDLTest_AssertCheck( (initialized_system & joy_and_controller) == joy_and_controller, "SDL_WasInit() should be true for joystick & controller (%x)", initialized_system );

    /* Then quit the controller, and make sure that it does NOT quit the */
    /* explicitly initialized joystick subsystem. */
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    initialized_system = SDL_WasInit(joy_and_controller);
    SDLTest_AssertCheck( (initialized_system & joy_and_controller) == SDL_INIT_JOYSTICK, "SDL_WasInit() should be false for joystick & controller (%x)", initialized_system );

    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);

    return TEST_COMPLETED;
#endif
}

static const SDLTest_TestCaseReference mainTest1 =
        { (SDLTest_TestCaseFp)main_testInitQuitJoystickHaptic, "main_testInitQuitJoystickHaptic", "Tests SDL_Init/Quit of Joystick and Haptic subsystem", TEST_ENABLED};

static const SDLTest_TestCaseReference mainTest2 =
        { (SDLTest_TestCaseFp)main_testInitQuitSubSystem, "main_testInitQuitSubSystem", "Tests SDL_InitSubSystem/QuitSubSystem", TEST_ENABLED};

static const SDLTest_TestCaseReference mainTest3 =
        { (SDLTest_TestCaseFp)main_testImpliedJoystickInit, "main_testImpliedJoystickInit", "Tests that init for gamecontroller properly implies joystick", TEST_ENABLED};

static const SDLTest_TestCaseReference mainTest4 =
        { (SDLTest_TestCaseFp)main_testImpliedJoystickQuit, "main_testImpliedJoystickQuit", "Tests that quit for gamecontroller doesn't quit joystick if you inited it explicitly", TEST_ENABLED};

/* Sequence of Main test cases */
static const SDLTest_TestCaseReference *mainTests[] =  {
    &mainTest1,
    &mainTest2,
    &mainTest3,
    &mainTest4,
    NULL
};

/* Main test suite (global) */
SDLTest_TestSuiteReference mainTestSuite = {
    "Main",
    NULL,
    mainTests,
    NULL
};

/* vi: set ts=4 sw=4 expandtab: */
