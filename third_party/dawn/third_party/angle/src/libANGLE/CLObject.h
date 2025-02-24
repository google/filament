//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLObject.h: Defines the cl::Object class, which is the base class of all ANGLE CL objects.

#ifndef LIBANGLE_CLOBJECT_H_
#define LIBANGLE_CLOBJECT_H_

#include "libANGLE/cl_types.h"
#include "libANGLE/renderer/cl_types.h"

#include <atomic>

namespace cl
{

class Object
{
  public:
    Object();
    virtual ~Object();

    cl_uint getRefCount() const noexcept { return mRefCount; }

    void retain() noexcept { ++mRefCount; }

    bool release()
    {
        if (mRefCount == 0u)
        {
            WARN() << "Unreferenced object without references";
            return true;
        }
        return --mRefCount == 0u;
    }

    template <typename T, typename... Args>
    static T *Create(Args &&...args)
    {
        T *object = new T(std::forward<Args>(args)...);
        return object;
    }

  private:
    std::atomic<cl_uint> mRefCount;
};

}  // namespace cl

#endif  // LIBANGLE_CLCONTEXT_H_
