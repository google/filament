// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "time_utils.h"

#include <sys/time.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#include "logging.h"

namespace time_utils {

uint64_t GetTimestamp() {
  struct timespec ts = {};
  CHECK(clock_gettime(CLOCK_MONOTONIC_COARSE, &ts) == 0);
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000ul;
}

PeriodicTimer::PeriodicTimer(int interval_ms) : interval_ms_(interval_ms) {
  timer_fd_ = -1;
}

PeriodicTimer::~PeriodicTimer() {
  Stop();
}

void PeriodicTimer::Start() {
  Stop();
  timer_fd_ = timerfd_create(CLOCK_MONOTONIC, 0);
  CHECK(timer_fd_ >= 0);
  int sec = interval_ms_ / 1000;
  int nsec = (interval_ms_ % 1000) * 1000000;
  struct itimerspec ts = {};
  ts.it_value.tv_nsec = nsec;
  ts.it_value.tv_sec = sec;
  ts.it_interval.tv_nsec = nsec;
  ts.it_interval.tv_sec = sec;
  CHECK(timerfd_settime(timer_fd_, 0, &ts, nullptr) == 0);
}

void PeriodicTimer::Stop() {
  if (timer_fd_ < 0)
    return;
  close(timer_fd_);
  timer_fd_ = -1;
}

bool PeriodicTimer::Wait() {
  if (timer_fd_ < 0)
    return false;  // Not started yet.
  uint64_t stub = 0;
  int res = read(timer_fd_, &stub, sizeof(stub));
  if (res < 0 && errno == EBADF)
    return false;  // Interrupted by Stop().
  CHECK(res > 0);
  return true;
}

}  // namespace time_utils
