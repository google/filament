//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_cl.h: Includes all necessary CL headers and definitions for ANGLE.

#ifndef ANGLECL_H_
#define ANGLECL_H_

#define CL_TARGET_OPENCL_VERSION 300
#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define CL_USE_DEPRECATED_OPENCL_2_1_APIS
#define CL_USE_DEPRECATED_OPENCL_2_2_APIS

#include "CL/cl_icd.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace cl
{

using ContextErrorCB = void(CL_CALLBACK *)(const char *errinfo,
                                           const void *private_info,
                                           size_t cb,
                                           void *user_data);

using MemoryCB  = void(CL_CALLBACK *)(cl_mem memobj, void *user_data);
using ProgramCB = void(CL_CALLBACK *)(cl_program program, void *user_data);
using EventCB   = void(CL_CALLBACK *)(cl_event event, cl_int event_command_status, void *user_data);
using UserFunc  = void(CL_CALLBACK *)(void *args);

template <typename T = void>
struct Dispatch
{
    explicit Dispatch(std::uint32_t magic) : mDispatch(sDispatch), mMagic(magic) {}

    const cl_icd_dispatch &getDispatch() const { return *mDispatch; }

    static const cl_icd_dispatch *sDispatch;

  protected:
    // This has to be the first member to be OpenCL ICD compatible
    const cl_icd_dispatch *const mDispatch;
    const std::uint32_t mMagic;
};

template <typename T>
const cl_icd_dispatch *Dispatch<T>::sDispatch = nullptr;

template <typename NativeObjectType, std::uint32_t magic>
struct NativeObject : public Dispatch<>
{
    NativeObject() : Dispatch<>(magic)
    {
        static_assert(std::is_standard_layout<NativeObjectType>::value &&
                          offsetof(NativeObjectType, mDispatch) == 0u,
                      "Not ICD compatible");
    }

    template <typename T>
    T &cast()
    {
        return static_cast<T &>(*this);
    }

    template <typename T>
    const T &cast() const
    {
        return static_cast<const T &>(*this);
    }

    NativeObjectType *getNative() { return static_cast<NativeObjectType *>(this); }

    const NativeObjectType *getNative() const
    {
        return static_cast<const NativeObjectType *>(this);
    }

    static NativeObjectType *CastNative(NativeObjectType *p) { return p; }

    static bool IsValid(const NativeObjectType *p)
    {
        return p != nullptr && p->mDispatch == sDispatch && p->mMagic == magic;
    }
};

}  // namespace cl

struct _cl_platform_id : public cl::NativeObject<_cl_platform_id, 0x12345678u>
{};

struct _cl_device_id : public cl::NativeObject<_cl_device_id, 0x23456789u>
{};

struct _cl_context : public cl::NativeObject<_cl_context, 0x3456789Au>
{};

struct _cl_command_queue : public cl::NativeObject<_cl_command_queue, 0x456789ABu>
{};

struct _cl_mem : public cl::NativeObject<_cl_mem, 0x56789ABCu>
{};

struct _cl_program : public cl::NativeObject<_cl_program, 0x6789ABCDu>
{};

struct _cl_kernel : public cl::NativeObject<_cl_kernel, 0x789ABCDEu>
{};

struct _cl_event : public cl::NativeObject<_cl_event, 0x89ABCDEFu>
{};

struct _cl_sampler : public cl::NativeObject<_cl_sampler, 0x9ABCDEF0u>
{};

#endif  // ANGLECL_H_
