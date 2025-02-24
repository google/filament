//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// trace_interface:
//   Interface shared between trace libraries and the test suite.
//

#ifndef UTIL_CAPTURE_TRACE_INTERFACE_H_
#define UTIL_CAPTURE_TRACE_INTERFACE_H_

#include <string>
#include <vector>

namespace angle
{

static constexpr size_t kTraceInfoMaxNameLen = 128;

enum class ReplayResourceMode
{
    Active,
    All,
};

struct TraceInfo
{
    char name[kTraceInfoMaxNameLen];
    bool initialized = false;
    uint32_t contextClientMajorVersion;
    uint32_t contextClientMinorVersion;
    uint32_t frameStart;
    uint32_t frameEnd;
    uint32_t drawSurfaceWidth;
    uint32_t drawSurfaceHeight;
    uint32_t drawSurfaceColorSpace;
    uint32_t displayPlatformType;
    uint32_t displayDeviceType;
    int configRedBits;
    int configBlueBits;
    int configGreenBits;
    int configAlphaBits;
    int configDepthBits;
    int configStencilBits;
    bool isBinaryDataCompressed;
    bool areClientArraysEnabled;
    bool isBindGeneratesResourcesEnabled;
    bool isWebGLCompatibilityEnabled;
    bool isRobustResourceInitEnabled;
    std::vector<std::string> traceFiles;
    int windowSurfaceContextId;
    std::vector<std::string> requiredExtensions;
    std::vector<int> keyFrames;
};

// Test suite calls into the trace library (fixture).
struct TraceFunctions
{
    virtual void SetupReplay()                    = 0;
    virtual void ReplayFrame(uint32_t frameIndex) = 0;
    virtual void ResetReplay()                    = 0;
    virtual void FinishReplay()                   = 0;

    virtual void SetBinaryDataDir(const char *dataDir)                        = 0;
    virtual void SetReplayResourceMode(const ReplayResourceMode resourceMode) = 0;
    virtual void SetTraceGzPath(const std::string &traceGzPath)               = 0;
    virtual void SetTraceInfo(const TraceInfo &traceInfo)                     = 0;

    virtual ~TraceFunctions() {}
};

// Trace library (fixture) calls into the test suite.
struct TraceCallbacks
{
    virtual uint8_t *LoadBinaryData(const char *fileName) = 0;

    virtual ~TraceCallbacks() {}
};

}  // namespace angle
#endif  // UTIL_CAPTURE_TRACE_INTERFACE_H_
