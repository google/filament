//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef SAMPLE_UTIL_TIMER_H
#define SAMPLE_UTIL_TIMER_H

class Timer final
{
  public:
    Timer();
    ~Timer() {}

    // Use start() and stop() to record the duration and use getElapsedWallClockTime() to query that
    // duration.  If getElapsedWallClockTime() is called in between, it will report the elapsed time
    // since start().
    void start();
    void stop();
    double getElapsedWallClockTime() const;
    double getElapsedCpuTime() const;

  private:
    bool mRunning;
    double mStartTime;
    double mStopTime;
    double mStartCpuTime;
    double mStopCpuTime;
};

#endif  // SAMPLE_UTIL_TIMER_H
