//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResultPerf:
//   Performance test for ANGLE's Error result class.
//

#include "ANGLEPerfTest.h"
#include "libANGLE/Error.h"

volatile int gThing = 0;

namespace
{
constexpr int kIterationsPerStep = 1000;

class ResultPerfTest : public ANGLEPerfTest
{
  public:
    ResultPerfTest();
    void step() override;
};

ResultPerfTest::ResultPerfTest() : ANGLEPerfTest("ResultPerf", "", "_run", kIterationsPerStep) {}

ANGLE_NOINLINE angle::Result ExternalCall()
{
    if (gThing != 0)
    {
        printf("Something very slow");
        return angle::Result::Stop;
    }
    else
    {
        return angle::Result::Continue;
    }
}

angle::Result CallReturningResult(int depth)
{
    ANGLE_TRY(ExternalCall());
    ANGLE_TRY(ExternalCall());
    ANGLE_TRY(ExternalCall());
    ANGLE_TRY(ExternalCall());
    ANGLE_TRY(ExternalCall());
    ANGLE_TRY(ExternalCall());
    ANGLE_TRY(ExternalCall());
    ANGLE_TRY(ExternalCall());
    ANGLE_TRY(ExternalCall());
    return ExternalCall();
}

void ResultPerfTest::step()
{
    for (int i = 0; i < kIterationsPerStep; i++)
    {
        (void)CallReturningResult(0);
        (void)CallReturningResult(0);
        (void)CallReturningResult(0);
        (void)CallReturningResult(0);
        (void)CallReturningResult(0);
    }
}

TEST_F(ResultPerfTest, Run)
{
    run();
}
}  // anonymous namespace
