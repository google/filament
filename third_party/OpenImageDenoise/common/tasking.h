// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "thread.h"

#define TBB_PREVIEW_LOCAL_OBSERVER 1
#include "tbb/task_scheduler_init.h"
#include "tbb/task_scheduler_observer.h"
#include "tbb/task_arena.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_reduce.h"
#include "tbb/blocked_range.h"
#include "tbb/blocked_range2d.h"

namespace oidn {

  // --------------------------------------------------------------------------
  // PinningObserver
  // --------------------------------------------------------------------------

  class PinningObserver : public tbb::task_scheduler_observer
  {
  private:
    std::shared_ptr<ThreadAffinity> affinity;

  public:
    explicit PinningObserver(const std::shared_ptr<ThreadAffinity>& affinity);
    PinningObserver(const std::shared_ptr<ThreadAffinity>& affinity, tbb::task_arena& arena);
    ~PinningObserver();

    void on_scheduler_entry(bool isWorker) override;
    void on_scheduler_exit(bool isWorker) override;
  };

} // namespace oidn
