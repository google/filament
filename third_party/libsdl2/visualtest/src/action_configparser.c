/* See LICENSE.txt for the full license governing this code. */
/**
 * \file action_configparser.c
 *
 * Source file for the parser for action config files.
 */

#include <SDL_stdinc.h>
#include <SDL_test.h>
#include <string.h>
#include "SDL_visualtest_action_configparser.h"
#include "SDL_visualtest_rwhelper.h"
#include "SDL_visualtest_parsehelper.h"

static void
FreeAction(SDLVisualTest_Action* action)
{
    if(!action)
        return;
    switch(action->type)
    {
        case SDL_ACTION_LAUNCH:
        {
            char* path;
            char* args;

            path = action->extra.process.path;
            args = action->extra.process.args;

            if(path)
                SDL_free(path);
            if(args)
                SDL_free(args);

            action->extra.process.path = NULL;
            action->extra.process.args = NULL;
        }
        break;
    }
}

int
SDLVisualTest_EnqueueAction(SDLVisualTest_ActionQueue* queue,
                            SDLVisualTest_Action action)
{
    SDLVisualTest_ActionNode* node;
    if(!queue)
    {
        SDLTest_LogError("queue argument cannot be NULL");
        return 0;
    }

    node = (SDLVisualTest_ActionNode*)SDL_malloc(
                                      sizeof(SDLVisualTest_ActionNode));
    if(!node)
    {
        SDLTest_LogError("malloc() failed");
        return 0;
    }
    node->action = action;
    node->next = NULL;
    queue->size++;
    if(!queue->rear)
        queue->rear = queue->front = node;
    else
    {
        queue->rear->next = node;
        queue->rear = node;
    }
    return 1;
}

int
SDLVisualTest_DequeueAction(SDLVisualTest_ActionQueue* queue)
{
    SDLVisualTest_ActionNode* node;
    if(!queue)
    {
        SDLTest_LogError("queue argument cannot be NULL");
        return 0;
    }
    if(SDLVisualTest_IsActionQueueEmpty(queue))
    {
        SDLTest_LogError("cannot dequeue from empty queue");
        return 0;
    }
    if(queue->front == queue->rear)
    {
        FreeAction(&queue->front->action);
        SDL_free(queue->front);
        queue->front = queue->rear = NULL;
    }
    else
    {
        node = queue->front;
        queue->front = queue->front->next;
        FreeAction(&node->action);
        SDL_free(node);
    }
    queue->size--;
    return 1;
}

void
SDLVisualTest_InitActionQueue(SDLVisualTest_ActionQueue* queue)
{
    if(!queue)
    {
        SDLTest_LogError("queue argument cannot be NULL");
        return;
    }
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

SDLVisualTest_Action*
SDLVisualTest_GetQueueFront(SDLVisualTest_ActionQueue* queue)
{
    if(!queue)
    {
        SDLTest_LogError("queue argument cannot be NULL");
        return NULL;
    }
    if(!queue->front)
    {
        SDLTest_LogError("cannot get front of empty queue");
        return NULL;
    }

    return &queue->front->action;
}

int
SDLVisualTest_IsActionQueueEmpty(SDLVisualTest_ActionQueue* queue)
{
    if(!queue)
    {
        SDLTest_LogError("queue argument cannot be NULL");
        return 1;
    }

    if(queue->size > 0)
        return 0;
    return 1;
}

void
SDLVisualTest_EmptyActionQueue(SDLVisualTest_ActionQueue* queue)
{
    if(queue)
    {
        while(!SDLVisualTest_IsActionQueueEmpty(queue))
            SDLVisualTest_DequeueAction(queue);
    }
}

/* Since the size of the queue is not likely to be larger than 100 elements
   we can get away with using insertion sort. */
static void
SortQueue(SDLVisualTest_ActionQueue* queue)
{
    SDLVisualTest_ActionNode* head;
    SDLVisualTest_ActionNode* tail;

    if(!queue || SDLVisualTest_IsActionQueueEmpty(queue))
        return;

    head = queue->front;
    for(tail = head; tail && tail->next;)
    {
        SDLVisualTest_ActionNode* pos;
        SDLVisualTest_ActionNode* element = tail->next;

        if(element->action.time < head->action.time)
        {
            tail->next = tail->next->next;
            element->next = head;
            head = element;
        }
        else if(element->action.time >= tail->action.time)
        {
            tail = tail->next;
        }
        else
        {
            for(pos = head;
                (pos->next->action.time < element->action.time);
                pos = pos->next);
            tail->next = tail->next->next;
            element->next = pos->next;
            pos->next = element;
        }
    }

    queue->front = head;
    queue->rear = tail;
}

int
SDLVisualTest_InsertIntoActionQueue(SDLVisualTest_ActionQueue* queue,
                                    SDLVisualTest_Action action)
{
    SDLVisualTest_ActionNode* n;
    SDLVisualTest_ActionNode* prev;
    SDLVisualTest_ActionNode* newnode;
    if(!queue)
    {
        SDLTest_LogError("queue argument cannot be NULL");
        return 0;
    }

    if(SDLVisualTest_IsActionQueueEmpty(queue))
    {
        if(!SDLVisualTest_EnqueueAction(queue, action))
        {
            SDLTest_LogError("SDLVisualTest_EnqueueAction() failed");
            return 0;
        }
        return 1;
    }

    newnode = (SDLVisualTest_ActionNode*)malloc(sizeof(SDLVisualTest_ActionNode));
    if(!newnode)
    {
        SDLTest_LogError("malloc() failed");
        return 0;
    }
    newnode->action = action;

    queue->size++;
    for(n = queue->front, prev = NULL; n; n = n->next)
    {
        if(action.time < n->action.time)
        {
            if(prev)
            {
                prev->next = newnode;
                newnode->next = n;
            }
            else
            {
                newnode->next = queue->front;
                queue->front = newnode;
            }
            return 1;
        }
        prev = n;
    }

    queue->rear->next = newnode;
    newnode->next = NULL;
    queue->rear = newnode;

    return 1;
}

int
SDLVisualTest_ParseActionConfig(char* file, SDLVisualTest_ActionQueue* queue)
{
    char line[MAX_ACTION_LINE_LENGTH];
    SDLVisualTest_RWHelperBuffer buffer;
    char* token_ptr;
    int linenum;
    SDL_RWops* rw;

    if(!file)
    {
        SDLTest_LogError("file argument cannot be NULL");
        return 0;
    }
    if(!queue)
    {
        SDLTest_LogError("queue argument cannot be NULL");
        return 0;
    }

    rw = SDL_RWFromFile(file, "r");
    if(!rw)
    {
        SDLTest_LogError("SDL_RWFromFile() failed");
        return 0;
    }

    SDLVisualTest_RWHelperResetBuffer(&buffer);
    SDLVisualTest_InitActionQueue(queue);
    linenum = 0;
    while(SDLVisualTest_RWHelperReadLine(rw, line, MAX_ACTION_LINE_LENGTH,
                                         &buffer, '#'))
    {
        SDLVisualTest_Action action;
        int hr, min, sec;

        /* parse time */
        token_ptr = strtok(line, " ");
        if(!token_ptr ||
           (SDL_sscanf(token_ptr, "%d:%d:%d", &hr, &min, &sec) != 3))
        {
            SDLTest_LogError("Could not parse time token at line: %d",
                             linenum);
            SDLVisualTest_EmptyActionQueue(queue);
            SDL_RWclose(rw);
            return 0;
        }
        action.time = (((hr * 60 + min) * 60) + sec) * 1000;

        /* parse type */
        token_ptr = strtok(NULL, " ");
        if(SDL_strcasecmp(token_ptr, "launch") == 0)
            action.type = SDL_ACTION_LAUNCH;
        else if(SDL_strcasecmp(token_ptr, "kill") == 0)
            action.type = SDL_ACTION_KILL;
        else if(SDL_strcasecmp(token_ptr, "quit") == 0)
            action.type = SDL_ACTION_QUIT;
        else if(SDL_strcasecmp(token_ptr, "screenshot") == 0)
            action.type = SDL_ACTION_SCREENSHOT;
        else if(SDL_strcasecmp(token_ptr, "verify") == 0)
            action.type = SDL_ACTION_VERIFY;
        else
        {
            SDLTest_LogError("Could not parse type token at line: %d",
                             linenum);
            SDLVisualTest_EmptyActionQueue(queue);
            SDL_RWclose(rw);
            return 0;
        }

        /* parse the extra field */
        if(action.type == SDL_ACTION_LAUNCH)
        {
            int len;
            char* args;
            char* path;
            token_ptr = strtok(NULL, " ");
            len = token_ptr ? SDL_strlen(token_ptr) : 0;
            if(len <= 0)
            {
                SDLTest_LogError("Please specify the process to launch at line: %d",
                                 linenum);
                SDLVisualTest_EmptyActionQueue(queue);
                SDL_RWclose(rw);
                return 0;
            }
            path = (char*)SDL_malloc(sizeof(char) * (len + 1));
            if(!path)
            {
                SDLTest_LogError("malloc() failed");
                SDLVisualTest_EmptyActionQueue(queue);
                SDL_RWclose(rw);
                return 0;
            }
            SDL_strlcpy(path, token_ptr, len + 1);

            token_ptr = strtok(NULL, "");
            len = token_ptr ? SDL_strlen(token_ptr) : 0;
            if(len > 0)
            {
                args = (char*)SDL_malloc(sizeof(char) * (len + 1));
                if(!args)
                {
                    SDLTest_LogError("malloc() failed");
                    SDL_free(path);
                    SDLVisualTest_EmptyActionQueue(queue);
                    SDL_RWclose(rw);
                    return 0;
                }
                SDL_strlcpy(args, token_ptr, len + 1);
            }
            else
                args = NULL;

            action.extra.process.path = path;
            action.extra.process.args = args;
        }

        /* add the action to the queue */
        if(!SDLVisualTest_EnqueueAction(queue, action))
        {
            SDLTest_LogError("SDLVisualTest_EnqueueAction() failed");
            if(action.type == SDL_ACTION_LAUNCH)
            {
                SDL_free(action.extra.process.path);
                if(action.extra.process.args)
                    SDL_free(action.extra.process.args);
            }
            SDLVisualTest_EmptyActionQueue(queue);
            SDL_RWclose(rw);
            return 0;
        }
    }
    /* sort the queue of actions */
    SortQueue(queue);

    SDL_RWclose(rw);
    return 1;
}