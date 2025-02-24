//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_test_utils:
//   Helper functions for capture and replay of traces.
//

#ifndef UTIL_CAPTURE_FRAME_CAPTURE_TEST_UTILS_H_
#define UTIL_CAPTURE_FRAME_CAPTURE_TEST_UTILS_H_

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/frame_capture_utils.h"
#include "common/system_utils.h"
#include "trace_interface.h"

#define USE_SYSTEM_ZLIB
#include "compression_utils_portable.h"

#define ANGLE_MACRO_STRINGIZE_AUX(a) #a
#define ANGLE_MACRO_STRINGIZE(a) ANGLE_MACRO_STRINGIZE_AUX(a)
#define ANGLE_MACRO_CONCAT_AUX(a, b) a##b
#define ANGLE_MACRO_CONCAT(a, b) ANGLE_MACRO_CONCAT_AUX(a, b)

namespace angle
{

using ValidateSerializedStateCallback = void (*)(const char *, const char *, uint32_t);

using GetSerializedContextStateFunc          = const char *(*)(uint32_t);
using SetValidateSerializedStateCallbackFunc = void (*)(ValidateSerializedStateCallback);
using SetupEntryPoints = void (*)(angle::TraceCallbacks *, angle::TraceFunctions **);

class TraceLibrary : angle::NonCopyable, angle::TraceCallbacks
{
  public:
    TraceLibrary(const std::string &traceName,
                 const TraceInfo &traceInfo,
                 const std::string &baseDir);

    bool valid() const
    {
        return (mTraceLibrary != nullptr) && (mTraceLibrary->getNative() != nullptr);
    }

    void setReplayResourceMode(const bool resourceMode)
    {
        mTraceFunctions->SetReplayResourceMode(
            (resourceMode ? ReplayResourceMode::All : ReplayResourceMode::Active));
    }

    void setBinaryDataDir(const char *dataDir)
    {
        mBinaryDataDir = dataDir;
        mTraceFunctions->SetBinaryDataDir(dataDir);
    }

    void setDebugOutputDir(const char *dataDir) { mDebugOutputDir = dataDir; }

    void replayFrame(uint32_t frameIndex) { mTraceFunctions->ReplayFrame(frameIndex); }

    void setupReplay() { mTraceFunctions->SetupReplay(); }

    void resetReplay() { mTraceFunctions->ResetReplay(); }

    void finishReplay()
    {
        mTraceFunctions->FinishReplay();
        mBinaryData = {};  // set to empty vector to release memory.
    }

    const char *getSerializedContextState(uint32_t frameIndex)
    {
        return callFunc<GetSerializedContextStateFunc>("GetSerializedContextState", frameIndex);
    }

    void setValidateSerializedStateCallback(ValidateSerializedStateCallback callback)
    {
        return callFunc<SetValidateSerializedStateCallbackFunc>(
            "SetValidateSerializedStateCallback", callback);
    }

    void setTraceGzPath(const std::string &traceGzPath)
    {
        mTraceFunctions->SetTraceGzPath(traceGzPath);
    }

  private:
    template <typename FuncT, typename... ArgsT>
    typename std::invoke_result<FuncT, ArgsT...>::type callFunc(const char *funcName, ArgsT... args)
    {
        void *untypedFunc = mTraceLibrary->getSymbol(funcName);
        if (!untypedFunc)
        {
            fprintf(stderr, "Error loading function: %s\n", funcName);
            ASSERT(untypedFunc);
        }
        auto typedFunc = reinterpret_cast<FuncT>(untypedFunc);
        return typedFunc(args...);
    }

    uint8_t *LoadBinaryData(const char *fileName) override;

    std::unique_ptr<Library> mTraceLibrary;
    std::vector<uint8_t> mBinaryData;
    std::string mBinaryDataDir;
    std::string mDebugOutputDir;
    angle::TraceInfo mTraceInfo;
    angle::TraceFunctions *mTraceFunctions = nullptr;
};

bool LoadTraceNamesFromJSON(const std::string jsonFilePath, std::vector<std::string> *namesOut);
bool LoadTraceInfoFromJSON(const std::string &traceName,
                           const std::string &traceJsonPath,
                           TraceInfo *traceInfoOut);

using TraceFunction    = std::vector<CallCapture>;
using TraceFunctionMap = std::map<std::string, TraceFunction>;

void ReplayTraceFunctionCall(const CallCapture &call, const TraceFunctionMap &customFunctions);
void ReplayCustomFunctionCall(const CallCapture &call, const TraceFunctionMap &customFunctions);

template <typename T>
struct AssertFalse : std::false_type
{};

GLuint GetResourceIDMapValue(ResourceIDType resourceIDType, GLuint key);

template <typename T>
T GetParamValue(ParamType type, const ParamValue &value);

template <>
inline GLuint GetParamValue<GLuint>(ParamType type, const ParamValue &value)
{
    ResourceIDType resourceIDType = GetResourceIDTypeFromParamType(type);
    if (resourceIDType == ResourceIDType::InvalidEnum)
    {
        return value.GLuintVal;
    }
    else
    {
        return GetResourceIDMapValue(resourceIDType, value.GLuintVal);
    }
}

template <>
inline GLint GetParamValue<GLint>(ParamType type, const ParamValue &value)
{
    return value.GLintVal;
}

template <>
inline const void *GetParamValue<const void *>(ParamType type, const ParamValue &value)
{
    return value.voidConstPointerVal;
}

template <>
inline GLuint64 GetParamValue<GLuint64>(ParamType type, const ParamValue &value)
{
    return value.GLuint64Val;
}

template <>
inline GLint64 GetParamValue<GLint64>(ParamType type, const ParamValue &value)
{
    return value.GLint64Val;
}

template <>
inline const char *GetParamValue<const char *>(ParamType type, const ParamValue &value)
{
    return value.GLcharConstPointerVal;
}

template <>
inline void *GetParamValue<void *>(ParamType type, const ParamValue &value)
{
    return value.voidPointerVal;
}

#if defined(ANGLE_IS_64_BIT_CPU)
template <>
inline const EGLAttrib *GetParamValue<const EGLAttrib *>(ParamType type, const ParamValue &value)
{
    return value.EGLAttribConstPointerVal;
}
#endif  // defined(ANGLE_IS_64_BIT_CPU)

template <>
inline const EGLint *GetParamValue<const EGLint *>(ParamType type, const ParamValue &value)
{
    return value.EGLintConstPointerVal;
}

template <>
inline const GLchar *const *GetParamValue<const GLchar *const *>(ParamType type,
                                                                 const ParamValue &value)
{
    return value.GLcharConstPointerPointerVal;
}

// On Apple platforms, std::is_same<uint64_t, long> is false despite being both 8 bits.
#if defined(ANGLE_PLATFORM_APPLE) || !defined(ANGLE_IS_64_BIT_CPU)
template <>
inline long GetParamValue<long>(ParamType type, const ParamValue &value)
{
    return static_cast<long>(value.GLint64Val);
}

template <>
inline unsigned long GetParamValue<unsigned long>(ParamType type, const ParamValue &value)
{
    return static_cast<unsigned long>(value.GLuint64Val);
}
#endif  // defined(ANGLE_PLATFORM_APPLE)

template <typename T>
T GetParamValue(ParamType type, const ParamValue &value)
{
    static_assert(AssertFalse<T>::value, "No specialization for type.");
}

template <typename T>
struct Traits;

template <typename... Args>
struct Traits<void(Args...)>
{
    static constexpr size_t NArgs = sizeof...(Args);
    template <size_t Idx>
    struct Arg
    {
        typedef typename std::tuple_element<Idx, std::tuple<Args...>>::type Type;
    };
};

template <typename Fn, size_t Idx>
using FnArg = typename Traits<Fn>::template Arg<Idx>::Type;

template <typename Fn, size_t NArgs>
using EnableIfNArgs = typename std::enable_if_t<Traits<Fn>::NArgs == NArgs, int>;

template <typename Fn, size_t Idx>
FnArg<Fn, Idx> Arg(const Captures &cap)
{
    ASSERT(Idx < cap.size());
    return GetParamValue<FnArg<Fn, Idx>>(cap[Idx].type, cap[Idx].value);
}
}  // namespace angle

#endif  // UTIL_CAPTURE_FRAME_CAPTURE_TEST_UTILS_H_
