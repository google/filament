//========================================================================
// Empty event test
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//
// This test is intended to verify that the posting of empty events works.
// Background colors are produced on a secondary thread, which then wakes
// the main thread with glfwPostEmptyEvent when a new one is available.
// This allows the main thread to wait for events unconditionally.
//
//========================================================================

#include "tinycthread.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct State
{
    mtx_t lock;
    bool running;
    bool needs_update;
    int width;
    int height;
    float r, g, b;
};

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void generate_color(struct State* state)
{
    const float r = (float) rand() / (float) RAND_MAX;
    const float g = (float) rand() / (float) RAND_MAX;
    const float b = (float) rand() / (float) RAND_MAX;
    const float l = sqrtf(r * r + g * g + b * b);

    mtx_lock(&state->lock);
    state->r = r / l;
    state->g = g / l;
    state->b = b / l;
    state->needs_update = true;
    mtx_unlock(&state->lock);
}

static int thread_main(void* data)
{
    struct State* const state = data;

    srand((unsigned int) time(NULL));

    while (state->running)
    {
        generate_color(state);
        glfwPostEmptyEvent();

        struct timespec time;
        clock_gettime(CLOCK_REALTIME, &time);
        time.tv_sec += 1;
        thrd_sleep(&time, NULL);
    }

    return 0;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    struct State* const state = glfwGetWindowUserPointer(window);

    mtx_lock(&state->lock);
    state->width = width;
    state->height = height;
    state->needs_update = true;
    mtx_unlock(&state->lock);
}

int main(void)
{
    struct State state = { .running = true };

    if (mtx_init(&state.lock, mtx_plain) != thrd_success)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    generate_color(&state);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Empty Event Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowUserPointer(window, &state);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwGetFramebufferSize(window, &state.width, &state.height);

    thrd_t color_thread;

    if (thrd_create(&color_thread, thread_main, &state) != thrd_success)
    {
        fprintf(stderr, "Failed to create secondary thread\n");

        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    while (!glfwWindowShouldClose(window))
    {
        if (state.needs_update)
        {
            mtx_lock(&state.lock);
            glViewport(0, 0, state.width, state.height);
            glClearColor(state.r, state.g, state.b, 1.f);
            state.needs_update = false;
            mtx_unlock(&state.lock);

            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(window);
        }

        glfwWaitEvents();
    }

    glfwHideWindow(window);
    state.running = false;
    thrd_join(color_thread, NULL);
    mtx_destroy(&state.lock);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

