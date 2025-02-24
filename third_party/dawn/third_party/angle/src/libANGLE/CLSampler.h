//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLSampler.h: Defines the cl::Sampler class, which describes how to sample an OpenCL Image.

#ifndef LIBANGLE_CLSAMPLER_H_
#define LIBANGLE_CLSAMPLER_H_

#include "libANGLE/CLObject.h"
#include "libANGLE/renderer/CLSamplerImpl.h"

namespace cl
{

class Sampler final : public _cl_sampler, public Object
{
  public:
    // Front end entry functions, only called from OpenCL entry points

    angle::Result getInfo(SamplerInfo name,
                          size_t valueSize,
                          void *value,
                          size_t *valueSizeRet) const;

  public:
    using PropArray = std::vector<cl_sampler_properties>;

    ~Sampler() override;

    const Context &getContext() const;
    const PropArray &getProperties() const;
    cl_bool getNormalizedCoords() const;
    AddressingMode getAddressingMode() const;
    FilterMode getFilterMode() const;

    template <typename T = rx::CLSamplerImpl>
    T &getImpl() const;

    static Sampler *Cast(cl_sampler sampler);

  private:
    Sampler(Context &context,
            PropArray &&properties,
            cl_bool normalizedCoords,
            AddressingMode addressingMode,
            FilterMode filterMode);

    const ContextPtr mContext;
    const PropArray mProperties;
    const cl_bool mNormalizedCoords;
    const AddressingMode mAddressingMode;
    const FilterMode mFilterMode;
    rx::CLSamplerImpl::Ptr mImpl;

    friend class Object;
};

inline const Context &Sampler::getContext() const
{
    return *mContext;
}

inline const Sampler::PropArray &Sampler::getProperties() const
{
    return mProperties;
}

inline cl_bool Sampler::getNormalizedCoords() const
{
    return mNormalizedCoords;
}

inline AddressingMode Sampler::getAddressingMode() const
{
    return mAddressingMode;
}

inline FilterMode Sampler::getFilterMode() const
{
    return mFilterMode;
}

template <typename T>
inline T &Sampler::getImpl() const
{
    return static_cast<T &>(*mImpl);
}

inline Sampler *Sampler::Cast(cl_sampler sampler)
{
    return static_cast<Sampler *>(sampler);
}

}  // namespace cl

#endif  // LIBANGLE_CLSAMPLER_H_
