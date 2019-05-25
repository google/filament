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

#include "common.h"

namespace oidn {

  class Buffer;
  class Filter;

  class Device : public RefCount
  {
  private:
    // Thread-safety
    std::mutex mutex;

    // Error handling
    struct ErrorState
    {
      Error code = Error::None;
      std::string message;
    };

    static thread_local ErrorState globalError;
    ThreadLocal<ErrorState> error;
    ErrorFunction errorFunc = nullptr;
    void* errorUserPtr = nullptr;

    // Tasking
    std::shared_ptr<tbb::task_arena> arena;
    std::shared_ptr<PinningObserver> observer;
    std::shared_ptr<ThreadAffinity> affinity;

    // Parameters
    int numThreads = 0; // autodetect by default
    bool setAffinity = true;

    bool dirty = true;

  public:
    Device();
    ~Device();

    static void setError(Device* device, Error code, const std::string& message);
    static Error getError(Device* device, const char** outMessage);

    void setErrorFunction(ErrorFunction func, void* userPtr);

    int get1i(const std::string& name);
    void set1i(const std::string& name, int value);

    void commit();

    template<typename F>
    void executeTask(F& f)
    {
      arena->execute(f);
    }

    template<typename F>
    void executeTask(const F& f)
    {
      arena->execute(f);
    }

    Ref<Buffer> newBuffer(size_t byteSize);
    Ref<Buffer> newBuffer(void* ptr, size_t byteSize);
    Ref<Filter> newFilter(const std::string& type);

    Device* getDevice() { return this; }
    std::mutex& getMutex() { return mutex; }

  private:
    bool isCommitted() const { return bool(arena); }
    void checkCommitted();
  };

} // namespace oidn
