//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLEventCL.h: Defines the class interface for CLEventCL, implementing CLEventImpl.

#ifndef LIBANGLE_RENDERER_CL_CLEVENTCL_H_
#define LIBANGLE_RENDERER_CL_CLEVENTCL_H_

#include "libANGLE/renderer/cl/cl_types.h"

#include "libANGLE/renderer/CLEventImpl.h"

namespace rx
{

class CLEventCL : public CLEventImpl
{
  public:
    CLEventCL(const cl::Event &event, cl_event native);
    ~CLEventCL() override;

    cl_event getNative() const;

    angle::Result getCommandExecutionStatus(cl_int &executionStatus) override;

    angle::Result setUserEventStatus(cl_int executionStatus) override;

    angle::Result setCallback(cl::Event &event, cl_int commandExecCallbackType) override;

    angle::Result getProfilingInfo(cl::ProfilingInfo name,
                                   size_t valueSize,
                                   void *value,
                                   size_t *valueSizeRet) override;

    static std::vector<cl_event> Cast(const cl::EventPtrs &events);

  private:
    static void CL_CALLBACK Callback(cl_event event, cl_int commandStatus, void *userData);

    const cl_event mNative;
};

inline cl_event CLEventCL::getNative() const
{
    return mNative;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CL_CLEVENTCL_H_
