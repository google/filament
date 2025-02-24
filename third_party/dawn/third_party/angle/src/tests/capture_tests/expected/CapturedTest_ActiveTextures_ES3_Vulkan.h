#pragma once

#include <EGL/egl.h>
#include <stdint.h>

// Public functions are declared in trace_fixture.h.

// Private Functions

void SetupReplayContext3(void);
void ReplayFrame1(void);
void ReplayFrame2(void);
void ReplayFrame3(void);
void ResetReplayContextShared(void);
void ResetReplayContext3(void);
void ReplayFrame4(void);
void SetupReplayContextShared(void);
void SetupReplayContextSharedInactive(void);
void InitReplay(void);

// Global variables

extern const char *const glShaderSource_string_0[];
extern const char *const glShaderSource_string_1[];
