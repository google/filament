//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Timer.cpp: Implementation of a high precision timer class.
//

#include "util/Timer.h"

#include "common/system_utils.h"

Timer::Timer() : mRunning(false), mStartTime(0), mStopTime(0) {}

void Timer::start()
{
    mStartTime    = angle::GetCurrentSystemTime();
    mStartCpuTime = angle::GetCurrentProcessCpuTime();
    mRunning      = true;
}

void Timer::stop()
{
    mStopTime    = angle::GetCurrentSystemTime();
    mStopCpuTime = angle::GetCurrentProcessCpuTime();
    mRunning     = false;
}

double Timer::getElapsedWallClockTime() const
{
    double endTime;
    if (mRunning)
    {
        endTime = angle::GetCurrentSystemTime();
    }
    else
    {
        endTime = mStopTime;
    }

    return endTime - mStartTime;
}

double Timer::getElapsedCpuTime() const
{
    double endTime;
    if (mRunning)
    {
        endTime = angle::GetCurrentProcessCpuTime();
    }
    else
    {
        endTime = mStopCpuTime;
    }

    return endTime - mStartCpuTime;
}
