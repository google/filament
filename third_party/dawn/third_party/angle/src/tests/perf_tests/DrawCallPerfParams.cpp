//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawCallPerfParams.cpp:
//   Parametrization for performance tests for ANGLE draw call overhead.
//

#include "DrawCallPerfParams.h"

#include <sstream>

DrawCallPerfParams::DrawCallPerfParams()
{
    majorVersion = 2;
    minorVersion = 0;
    windowWidth  = 64;
    windowHeight = 64;

// Lower the iteration count in debug.
#if !defined(NDEBUG)
    iterationsPerStep = 100;
#else
    iterationsPerStep = 20000;
#endif
    runTimeSeconds = 10.0;
    numTris        = 1;
}

DrawCallPerfParams::~DrawCallPerfParams() = default;

std::string DrawCallPerfParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    return strstr.str();
}
