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

#include "device.h"
#include "autoencoder.h"

namespace oidn {

  thread_local Device::ErrorState Device::globalError;

  Device::Device()
  {
    if (!mayiuse(sse41))
      throw Exception(Error::UnsupportedHardware, "SSE4.1 support is required at minimum");
  }

  Device::~Device()
  {
    observer.reset();
  }

  void Device::setError(Device* device, Error code, const std::string& message)
  {
    // Update the stored error only if the previous error was queried
    if (device)
    {
      ErrorState& curError = device->error.get();

      if (curError.code == Error::None)
      {
        curError.code = code;
        curError.message = message;
      }

      // Call the error callback function
      ErrorFunction errorFunc;
      void* errorUserPtr;

      {
        std::lock_guard<std::mutex> lock(device->mutex);
        errorFunc = device->errorFunc;
        errorUserPtr = device->errorUserPtr;
      }

      if (errorFunc)
        errorFunc(errorUserPtr, code, (code == Error::None) ? nullptr : message.c_str());
    }
    else
    {
      if (globalError.code == Error::None)
      {
        globalError.code = code;
        globalError.message = message;
      }
    }
  }

  Error Device::getError(Device* device, const char** outMessage)
  {
    // Return and clear the stored error code, but keep the error message so pointers to it will
    // remain valid until the next getError call
    if (device)
    {
      ErrorState& curError = device->error.get();
      const Error code = curError.code;
      if (outMessage)
        *outMessage = (code == Error::None) ? nullptr : curError.message.c_str();
      curError.code = Error::None;
      return code;
    }
    else
    {
      const Error code = globalError.code;
      if (outMessage)
        *outMessage = (code == Error::None) ? nullptr : globalError.message.c_str();
      globalError.code = Error::None;
      return code;
    }
  }

  void Device::setErrorFunction(ErrorFunction func, void* userPtr)
  {
    errorFunc = func;
    errorUserPtr = userPtr;
  }

  int Device::get1i(const std::string& name)
  {
    if (name == "numThreads")
      return numThreads;
    else if (name == "setAffinity")
      return setAffinity;
    else if (name == "version")
      return OIDN_VERSION;
    else if (name == "versionMajor")
      return OIDN_VERSION_MAJOR;
    else if (name == "versionMinor")
      return OIDN_VERSION_MINOR;
    else if (name == "versionPatch")
      return OIDN_VERSION_PATCH;
    else
      throw Exception(Error::InvalidArgument, "invalid parameter");
  }

  void Device::set1i(const std::string& name, int value)
  {
    if (name == "numThreads")
      numThreads = value;
    else if (name == "setAffinity")
      setAffinity = value;

    dirty = true;
  }

  void Device::commit()
  {
    if (isCommitted())
      throw Exception(Error::InvalidOperation, "device can be committed only once");

    // Get the optimal thread affinities
    if (setAffinity)
    {
      affinity = std::make_shared<ThreadAffinity>(1); // one thread per core
      if (affinity->getNumThreads() == 0)
        affinity.reset();
    }

    // Create the task arena
    const int maxNumThreads = affinity ? affinity->getNumThreads() : tbb::this_task_arena::max_concurrency();
    numThreads = (numThreads > 0) ? min(numThreads, maxNumThreads) : maxNumThreads;
    arena = std::make_shared<tbb::task_arena>(numThreads);

    // Automatically set the thread affinities
    if (affinity)
      observer = std::make_shared<PinningObserver>(affinity, *arena);

    dirty = false;
  }

  void Device::checkCommitted()
  {
    if (dirty)
      throw Exception(Error::InvalidOperation, "changes to the device are not committed");
  }

  Ref<Buffer> Device::newBuffer(size_t byteSize)
  {
    checkCommitted();
    return makeRef<Buffer>(Ref<Device>(this), byteSize);
  }

  Ref<Buffer> Device::newBuffer(void* ptr, size_t byteSize)
  {
    checkCommitted();
    return makeRef<Buffer>(Ref<Device>(this), ptr, byteSize);
  }

  Ref<Filter> Device::newFilter(const std::string& type)
  {
    checkCommitted();

    Ref<Filter> filter;

    if (type == "RT")
      filter = makeRef<RTFilter>(Ref<Device>(this));
    else
      throw Exception(Error::InvalidArgument, "unknown filter type");

    return filter;
  }

} // namespace oidn
