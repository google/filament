// Copyright 2016 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "draco/core/cycle_timer.h"

namespace draco {
void DracoTimer::Start() {
#ifdef _WIN32
  QueryPerformanceCounter(&tv_start);
#else
  gettimeofday(&tv_start, nullptr);
#endif
}

void DracoTimer::Stop() {
#ifdef _WIN32
  QueryPerformanceCounter(&tv_end);
#else
  gettimeofday(&tv_end, nullptr);
#endif
}

int64_t DracoTimer::GetInMs() {
#ifdef _WIN32
  LARGE_INTEGER elapsed = {0};
  elapsed.QuadPart = tv_end.QuadPart - tv_start.QuadPart;

  LARGE_INTEGER frequency = {0};
  QueryPerformanceFrequency(&frequency);
  return elapsed.QuadPart * 1000 / frequency.QuadPart;
#else
  const int64_t seconds = (tv_end.tv_sec - tv_start.tv_sec) * 1000;
  const int64_t milliseconds = (tv_end.tv_usec - tv_start.tv_usec) / 1000;
  return seconds + milliseconds;
#endif
}

}  // namespace draco
