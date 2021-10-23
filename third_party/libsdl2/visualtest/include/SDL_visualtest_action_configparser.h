/* See LICENSE.txt for the full license governing this code. */
/**
 * \file SDL_visualtest_action_configparser.h
 *
 * Header file for the parser for action config files.
 */

#ifndef SDL_visualtest_action_configparser_h_
#define SDL_visualtest_action_configparser_h_

/** The maximum length of one line in the actions file */
#define MAX_ACTION_LINE_LENGTH 300

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Type of the action.
 */
typedef enum
{
    /*! Launch an application with some given arguments */
    SDL_ACTION_LAUNCH = 0,
    /*! Kill the SUT process */
    SDL_ACTION_KILL,
    /*! Quit (Gracefully exit) the SUT process */
    SDL_ACTION_QUIT,
    /*! Take a screenshot of the SUT window */
    SDL_ACTION_SCREENSHOT,
    /*! Verify a previously taken screenshot */
    SDL_ACTION_VERIFY
} SDLVisualTest_ActionType;

/**
 * Struct that defines an action that will be performed on the SUT process at
 * a specific time.
 */
typedef struct SDLVisualTest_Action
{
    /*! The type of action to be performed */
    SDLVisualTest_ActionType type;
    /*! The time, in milliseconds from the launch of the SUT, when the action
        will be performed */
    int time;
    /*! Any additional information needed to perform the action. */
    union
    {
        /*! The path and arguments to the process to be launched */
        struct
        {
            char* path;
            char* args;
        } process;
    } extra;
} SDLVisualTest_Action;

/**
 * Struct for a node in the action queue. 
 */
typedef struct SDLVisualTest_ActionNode
{
    /*! The action in this node */
    SDLVisualTest_Action action;
    /*! Pointer to the next element in the queue */
    struct SDLVisualTest_ActionNode* next;
} SDLVisualTest_ActionNode;

/**
 * Queue structure for actions loaded from the actions config file. 
 */
typedef struct SDLVisualTest_ActionQueue
{
    /*! Pointer to the front of the queue */
    SDLVisualTest_ActionNode* front;
    /*! Pointer to the rear of the queue */
    SDLVisualTest_ActionNode* rear;
    /*! Number of nodes in the queue */
    int size;
} SDLVisualTest_ActionQueue;

/**
 * Add an action pointed to by \c action to the rear of the action queue pointed
 * to by \c queue.
 *
 * \return 1 on success, 0 on failure.
 */
int SDLVisualTest_EnqueueAction(SDLVisualTest_ActionQueue* queue,
                                SDLVisualTest_Action action);

/**
 * Remove an action from the front of the action queue pointed to by \c queue.
 *
 * \return 1 on success, 0 on failure.
 */
int SDLVisualTest_DequeueAction(SDLVisualTest_ActionQueue* queue);

/**
 * Initialize the action queue pointed to by \c queue.
 */
void SDLVisualTest_InitActionQueue(SDLVisualTest_ActionQueue* queue);

/**
 * Get the action at the front of the action queue pointed to by \c queue.
 * The returned action pointer may become invalid after subsequent dequeues.
 *
 * \return pointer to the action on success, NULL on failure.
 */
SDLVisualTest_Action* SDLVisualTest_GetQueueFront(SDLVisualTest_ActionQueue* queue);

/**
 * Check if the queue pointed to by \c queue is empty or not.
 *
 * \return 1 if the queue is empty, 0 otherwise.
 */
int SDLVisualTest_IsActionQueueEmpty(SDLVisualTest_ActionQueue* queue);

/**
 * Dequeues all the elements in the queque pointed to by \c queue.
 */
void SDLVisualTest_EmptyActionQueue(SDLVisualTest_ActionQueue* queue);

/**
 * Inserts an action \c action into the queue pointed to by \c queue such that
 * the times of actions in the queue increase as we move from the front to the
 * rear.
 *
 * \return 1 on success, 0 on failure.
 */
int SDLVisualTest_InsertIntoActionQueue(SDLVisualTest_ActionQueue* queue,
                                        SDLVisualTest_Action action);

/**
 * Parses an action config file with path \c file and populates an action queue
 * pointed to by \c queue with actions.
 *
 * \return 1 on success, 0 on failure.
 */
int SDLVisualTest_ParseActionConfig(char* file, SDLVisualTest_ActionQueue* queue);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_action_configparser_h_ */

/* vi: set ts=4 sw=4 expandtab: */
