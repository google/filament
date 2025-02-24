//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FrameCapture.cpp:
//   ANGLE Frame capture implementation.
//

#include "libANGLE/capture/FrameCapture.h"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <queue>
#include <string>

#include "sys/stat.h"

#include "common/aligned_memory.h"
#include "common/angle_version_info.h"
#include "common/frame_capture_utils.h"
#include "common/gl_enum_utils.h"
#include "common/mathutil.h"
#include "common/serializer/JsonSerializer.h"
#include "common/string_utils.h"
#include "common/system_utils.h"
#include "gpu_info_util/SystemInfo.h"
#include "image_util/storeimage.h"
#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/Context.inl.h"
#include "libANGLE/Display.h"
#include "libANGLE/EGLSync.h"
#include "libANGLE/Fence.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/GLES1Renderer.h"
#include "libANGLE/Query.h"
#include "libANGLE/ResourceMap.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Surface.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/capture/capture_egl_autogen.h"
#include "libANGLE/capture/capture_gles_1_0_autogen.h"
#include "libANGLE/capture/capture_gles_2_0_autogen.h"
#include "libANGLE/capture/capture_gles_3_0_autogen.h"
#include "libANGLE/capture/capture_gles_3_1_autogen.h"
#include "libANGLE/capture/capture_gles_3_2_autogen.h"
#include "libANGLE/capture/capture_gles_ext_autogen.h"
#include "libANGLE/capture/serialize.h"
#include "libANGLE/entry_points_utils.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/queryutils.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/validationEGL.h"
#include "third_party/ceval/ceval.h"

#define USE_SYSTEM_ZLIB
#include "compression_utils_portable.h"

#if !ANGLE_CAPTURE_ENABLED
#    error Frame capture must be enabled to include this file.
#endif  // !ANGLE_CAPTURE_ENABLED

namespace angle
{
namespace
{

// TODO: Consolidate to C output and remove option. http://anglebug.com/42266223

constexpr char kEnabledVarName[]        = "ANGLE_CAPTURE_ENABLED";
constexpr char kOutDirectoryVarName[]   = "ANGLE_CAPTURE_OUT_DIR";
constexpr char kFrameStartVarName[]     = "ANGLE_CAPTURE_FRAME_START";
constexpr char kFrameEndVarName[]       = "ANGLE_CAPTURE_FRAME_END";
constexpr char kTriggerVarName[]        = "ANGLE_CAPTURE_TRIGGER";
constexpr char kCaptureLabelVarName[]   = "ANGLE_CAPTURE_LABEL";
constexpr char kCompressionVarName[]    = "ANGLE_CAPTURE_COMPRESSION";
constexpr char kSerializeStateVarName[] = "ANGLE_CAPTURE_SERIALIZE_STATE";
constexpr char kValidationVarName[]     = "ANGLE_CAPTURE_VALIDATION";
constexpr char kValidationExprVarName[] = "ANGLE_CAPTURE_VALIDATION_EXPR";
constexpr char kSourceExtVarName[]      = "ANGLE_CAPTURE_SOURCE_EXT";
constexpr char kSourceSizeVarName[]     = "ANGLE_CAPTURE_SOURCE_SIZE";
constexpr char kForceShadowVarName[]    = "ANGLE_CAPTURE_FORCE_SHADOW";

constexpr size_t kBinaryAlignment   = 16;
constexpr size_t kFunctionSizeLimit = 5000;

// Limit based on MSVC Compiler Error C2026
constexpr size_t kStringLengthLimit = 16380;

// Default limit to number of bytes in a capture source files.
constexpr char kDefaultSourceFileExt[]           = "cpp";
constexpr size_t kDefaultSourceFileSizeThreshold = 400000;

// Android debug properties that correspond to the above environment variables
constexpr char kAndroidEnabled[]        = "debug.angle.capture.enabled";
constexpr char kAndroidOutDir[]         = "debug.angle.capture.out_dir";
constexpr char kAndroidFrameStart[]     = "debug.angle.capture.frame_start";
constexpr char kAndroidFrameEnd[]       = "debug.angle.capture.frame_end";
constexpr char kAndroidTrigger[]        = "debug.angle.capture.trigger";
constexpr char kAndroidCaptureLabel[]   = "debug.angle.capture.label";
constexpr char kAndroidCompression[]    = "debug.angle.capture.compression";
constexpr char kAndroidValidation[]     = "debug.angle.capture.validation";
constexpr char kAndroidValidationExpr[] = "debug.angle.capture.validation_expr";
constexpr char kAndroidSourceExt[]      = "debug.angle.capture.source_ext";
constexpr char kAndroidSourceSize[]     = "debug.angle.capture.source_size";
constexpr char kAndroidForceShadow[]    = "debug.angle.capture.force_shadow";

struct FramebufferCaptureFuncs
{
    FramebufferCaptureFuncs(bool isGLES1)
    {
        if (isGLES1)
        {
            // From GL_OES_framebuffer_object
            framebufferTexture2D    = &gl::CaptureFramebufferTexture2DOES;
            framebufferRenderbuffer = &gl::CaptureFramebufferRenderbufferOES;
            bindFramebuffer         = &gl::CaptureBindFramebufferOES;
            genFramebuffers         = &gl::CaptureGenFramebuffersOES;
            bindRenderbuffer        = &gl::CaptureBindRenderbufferOES;
            genRenderbuffers        = &gl::CaptureGenRenderbuffersOES;
            renderbufferStorage     = &gl::CaptureRenderbufferStorageOES;
        }
        else
        {
            framebufferTexture2D    = &gl::CaptureFramebufferTexture2D;
            framebufferRenderbuffer = &gl::CaptureFramebufferRenderbuffer;
            bindFramebuffer         = &gl::CaptureBindFramebuffer;
            genFramebuffers         = &gl::CaptureGenFramebuffers;
            bindRenderbuffer        = &gl::CaptureBindRenderbuffer;
            genRenderbuffers        = &gl::CaptureGenRenderbuffers;
            renderbufferStorage     = &gl::CaptureRenderbufferStorage;
        }
    }

    decltype(&gl::CaptureFramebufferTexture2D) framebufferTexture2D;
    decltype(&gl::CaptureFramebufferRenderbuffer) framebufferRenderbuffer;
    decltype(&gl::CaptureBindFramebuffer) bindFramebuffer;
    decltype(&gl::CaptureGenFramebuffers) genFramebuffers;
    decltype(&gl::CaptureBindRenderbuffer) bindRenderbuffer;
    decltype(&gl::CaptureGenRenderbuffers) genRenderbuffers;
    decltype(&gl::CaptureRenderbufferStorage) renderbufferStorage;
};

struct VertexArrayCaptureFuncs
{
    VertexArrayCaptureFuncs(bool isGLES1)
    {
        if (isGLES1)
        {
            // From GL_OES_vertex_array_object
            bindVertexArray    = &gl::CaptureBindVertexArrayOES;
            deleteVertexArrays = &gl::CaptureDeleteVertexArraysOES;
            genVertexArrays    = &gl::CaptureGenVertexArraysOES;
            isVertexArray      = &gl::CaptureIsVertexArrayOES;
        }
        else
        {
            bindVertexArray    = &gl::CaptureBindVertexArray;
            deleteVertexArrays = &gl::CaptureDeleteVertexArrays;
            genVertexArrays    = &gl::CaptureGenVertexArrays;
            isVertexArray      = &gl::CaptureIsVertexArray;
        }
    }

    decltype(&gl::CaptureBindVertexArray) bindVertexArray;
    decltype(&gl::CaptureDeleteVertexArrays) deleteVertexArrays;
    decltype(&gl::CaptureGenVertexArrays) genVertexArrays;
    decltype(&gl::CaptureIsVertexArray) isVertexArray;
};

std::string GetDefaultOutDirectory()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    std::string path = "/sdcard/Android/data/";

    // Linux interface to get application id of the running process
    FILE *cmdline = fopen("/proc/self/cmdline", "r");
    char applicationId[512];
    if (cmdline)
    {
        fread(applicationId, 1, sizeof(applicationId), cmdline);
        fclose(cmdline);

        // Some package may have application id as <app_name>:<cmd_name>
        char *colonSep = strchr(applicationId, ':');
        if (colonSep)
        {
            *colonSep = '\0';
        }
    }
    else
    {
        ERR() << "not able to lookup application id";
    }

    constexpr char kAndroidOutputSubdir[] = "/angle_capture/";
    path += std::string(applicationId) + kAndroidOutputSubdir;

    // Check for existence of output path
    struct stat dir_stat;
    if (stat(path.c_str(), &dir_stat) == -1)
    {
        ERR() << "Output directory '" << path
              << "' does not exist.  Create it over adb using mkdir.";
    }

    return path;
#else
    return std::string("./");
#endif  // defined(ANGLE_PLATFORM_ANDROID)
}

std::string GetCaptureTrigger()
{
    // Use the GetAndSet variant to improve future lookup times
    return GetAndSetEnvironmentVarOrUnCachedAndroidProperty(kTriggerVarName, kAndroidTrigger);
}

std::ostream &operator<<(std::ostream &os, gl::ContextID contextId)
{
    os << static_cast<int>(contextId.value);
    return os;
}

// Used to indicate that "shared" should be used to identify the files.
constexpr gl::ContextID kSharedContextId = {0};
// Used to indicate no context ID should be output.
constexpr gl::ContextID kNoContextId = {std::numeric_limits<uint32_t>::max()};

struct FmtCapturePrefix
{
    FmtCapturePrefix(gl::ContextID contextIdIn, const std::string &captureLabelIn)
        : contextId(contextIdIn), captureLabel(captureLabelIn)
    {}
    gl::ContextID contextId;
    const std::string &captureLabel;
};

std::ostream &operator<<(std::ostream &os, const FmtCapturePrefix &fmt)
{
    if (fmt.captureLabel.empty())
    {
        os << "angle_capture";
    }
    else
    {
        os << fmt.captureLabel;
    }

    if (fmt.contextId == kSharedContextId)
    {
        os << "_shared";
    }

    return os;
}

enum class ReplayFunc
{
    Replay,
    Setup,
    SetupInactive,
    Reset,
};

constexpr uint32_t kNoPartId = std::numeric_limits<uint32_t>::max();

// In C, when you declare or define a function that takes no parameters, you must explicitly say the
// function takes "void" parameters. When you're calling the function you omit this void. It's
// therefore necessary to know how we're using a function to know if we should emi the "void".
enum FuncUsage
{
    Prototype,
    Definition,
    Call,
};

std::ostream &operator<<(std::ostream &os, FuncUsage usage)
{
    os << "(";
    if (usage != FuncUsage::Call)
    {
        os << "void";
    }
    os << ")";
    return os;
}

struct FmtReplayFunction
{
    FmtReplayFunction(gl::ContextID contextIdIn,
                      FuncUsage usageIn,
                      uint32_t frameIndexIn,
                      uint32_t partIdIn = kNoPartId)
        : contextId(contextIdIn), usage(usageIn), frameIndex(frameIndexIn), partId(partIdIn)
    {}
    gl::ContextID contextId;
    FuncUsage usage;
    uint32_t frameIndex;
    uint32_t partId;
};

std::ostream &operator<<(std::ostream &os, const FmtReplayFunction &fmt)
{
    os << "Replay";

    if (fmt.contextId == kSharedContextId)
    {
        os << "Shared";
    }

    os << "Frame" << fmt.frameIndex;

    if (fmt.partId != kNoPartId)
    {
        os << "Part" << fmt.partId;
    }
    os << fmt.usage;
    return os;
}

struct FmtSetupFunction
{
    FmtSetupFunction(uint32_t partIdIn, gl::ContextID contextIdIn, FuncUsage usageIn)
        : partId(partIdIn), contextId(contextIdIn), usage(usageIn)
    {}

    uint32_t partId;
    gl::ContextID contextId;
    FuncUsage usage;
};

std::ostream &operator<<(std::ostream &os, const FmtSetupFunction &fmt)
{
    os << "SetupReplayContext";

    if (fmt.contextId == kSharedContextId)
    {
        os << "Shared";
    }
    else
    {
        os << fmt.contextId;
    }

    if (fmt.partId != kNoPartId)
    {
        os << "Part" << fmt.partId;
    }
    os << fmt.usage;
    return os;
}

struct FmtSetupInactiveFunction
{
    FmtSetupInactiveFunction(uint32_t partIdIn, gl::ContextID contextIdIn, FuncUsage usageIn)
        : partId(partIdIn), contextId(contextIdIn), usage(usageIn)
    {}

    uint32_t partId;
    gl::ContextID contextId;
    FuncUsage usage;
};

std::ostream &operator<<(std::ostream &os, const FmtSetupInactiveFunction &fmt)
{
    if ((fmt.usage == FuncUsage::Call) && (fmt.partId == kNoPartId))
    {
        os << "if (gReplayResourceMode == angle::ReplayResourceMode::All)\n    {\n        ";
    }
    os << "SetupReplayContext";

    if (fmt.contextId == kSharedContextId)
    {
        os << "Shared";
    }
    else
    {
        os << fmt.contextId;
    }

    os << "Inactive";

    if (fmt.partId != kNoPartId)
    {
        os << "Part" << fmt.partId;
    }

    os << fmt.usage;

    if ((fmt.usage == FuncUsage::Call) && (fmt.partId == kNoPartId))
    {
        os << ";\n    }";
    }
    return os;
}

struct FmtResetFunction
{
    FmtResetFunction(uint32_t partIdIn, gl::ContextID contextIdIn, FuncUsage usageIn)
        : partId(partIdIn), contextId(contextIdIn), usage(usageIn)
    {}

    uint32_t partId;
    gl::ContextID contextId;
    FuncUsage usage;
};

std::ostream &operator<<(std::ostream &os, const FmtResetFunction &fmt)
{
    os << "ResetReplayContext";

    if (fmt.contextId == kSharedContextId)
    {
        os << "Shared";
    }
    else
    {
        os << fmt.contextId;
    }

    if (fmt.partId != kNoPartId)
    {
        os << "Part" << fmt.partId;
    }
    os << fmt.usage;
    return os;
}

struct FmtFunction
{
    FmtFunction(ReplayFunc funcTypeIn,
                gl::ContextID contextIdIn,
                FuncUsage usageIn,
                uint32_t frameIndexIn,
                uint32_t partIdIn)
        : funcType(funcTypeIn),
          contextId(contextIdIn),
          usage(usageIn),
          frameIndex(frameIndexIn),
          partId(partIdIn)
    {}

    ReplayFunc funcType;
    gl::ContextID contextId;
    FuncUsage usage;
    uint32_t frameIndex;
    uint32_t partId;
};

std::ostream &operator<<(std::ostream &os, const FmtFunction &fmt)
{
    switch (fmt.funcType)
    {
        case ReplayFunc::Replay:
            os << FmtReplayFunction(fmt.contextId, fmt.usage, fmt.frameIndex, fmt.partId);
            break;

        case ReplayFunc::Setup:
            os << FmtSetupFunction(fmt.partId, fmt.contextId, fmt.usage);
            break;

        case ReplayFunc::SetupInactive:
            os << FmtSetupInactiveFunction(fmt.partId, fmt.contextId, fmt.usage);
            break;

        case ReplayFunc::Reset:
            os << FmtResetFunction(fmt.partId, fmt.contextId, fmt.usage);
            break;

        default:
            UNREACHABLE();
            break;
    }

    return os;
}

struct FmtGetSerializedContextStateFunction
{
    FmtGetSerializedContextStateFunction(gl::ContextID contextIdIn,
                                         FuncUsage usageIn,
                                         uint32_t frameIndexIn)
        : contextId(contextIdIn), usage(usageIn), frameIndex(frameIndexIn)
    {}
    gl::ContextID contextId;
    FuncUsage usage;
    uint32_t frameIndex;
};

std::ostream &operator<<(std::ostream &os, const FmtGetSerializedContextStateFunction &fmt)
{
    os << "GetSerializedContext" << fmt.contextId << "StateFrame" << fmt.frameIndex << "Data"
       << fmt.usage;
    return os;
}

void WriteGLFloatValue(std::ostream &out, GLfloat value)
{
    // Check for non-representable values
    ASSERT(std::numeric_limits<float>::has_infinity);
    ASSERT(std::numeric_limits<float>::has_quiet_NaN);

    if (std::isinf(value))
    {
        float negativeInf = -std::numeric_limits<float>::infinity();
        if (value == negativeInf)
        {
            out << "-";
        }
        out << "INFINITY";
    }
    else if (std::isnan(value))
    {
        out << "NAN";
    }
    else
    {
        // Write a decimal point to preserve the zero sign on replay
        out << (value == 0.0 ? std::showpoint : std::noshowpoint);
        out << std::setprecision(16);
        out << value;
    }
}

template <typename T, typename CastT = T>
void WriteInlineData(const std::vector<uint8_t> &vec, std::ostream &out)
{
    const T *data = reinterpret_cast<const T *>(vec.data());
    size_t count  = vec.size() / sizeof(T);

    if (data == nullptr)
    {
        return;
    }

    out << static_cast<CastT>(data[0]);

    for (size_t dataIndex = 1; dataIndex < count; ++dataIndex)
    {
        out << ", " << static_cast<CastT>(data[dataIndex]);
    }
}

template <>
void WriteInlineData<GLchar>(const std::vector<uint8_t> &vec, std::ostream &out)
{
    const GLchar *data = reinterpret_cast<const GLchar *>(vec.data());
    size_t count       = vec.size() / sizeof(GLchar);

    if (data == nullptr || data[0] == '\0')
    {
        return;
    }

    out << "\"";

    for (size_t dataIndex = 0; dataIndex < count; ++dataIndex)
    {
        if (data[dataIndex] == '\0')
            break;

        out << static_cast<GLchar>(data[dataIndex]);
    }

    out << "\"";
}

// For compatibility with C, which does not have multi-line string literals, we break strings up
// into multiple lines like:
//
//   const char *str[] = {
//   "multiple\n"
//   "line\n"
//   "strings may have \"quotes\"\n"
//   "and \\slashes\\\n",
//   };
//
// Note we need to emit extra escapes to ensure quotes and other special characters are preserved.
struct FmtMultiLineString
{
    FmtMultiLineString(const std::string &str) : strings()
    {
        std::string str2;

        // Strip any carriage returns before splitting, for consistency
        if (str.find("\r") != std::string::npos)
        {
            // str is const, so have to make a copy of it first
            str2 = str;
            ReplaceAllSubstrings(&str2, "\r", "");
        }

        strings =
            angle::SplitString(str2.empty() ? str : str2, "\n", WhitespaceHandling::KEEP_WHITESPACE,
                               SplitResult::SPLIT_WANT_ALL);
    }

    std::vector<std::string> strings;
};

std::string EscapeString(const std::string &string)
{
    std::stringstream strstr;

    for (char c : string)
    {
        if (c == '\"' || c == '\\')
        {
            strstr << "\\";
        }
        strstr << c;
    }

    return strstr.str();
}

std::ostream &operator<<(std::ostream &ostr, const FmtMultiLineString &fmt)
{
    ASSERT(!fmt.strings.empty());
    bool first = true;
    for (const std::string &string : fmt.strings)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            ostr << "\\n\"\n";
        }

        ostr << "\"" << EscapeString(string);
    }

    ostr << "\"";

    return ostr;
}

void WriteStringParamReplay(ReplayWriter &replayWriter,
                            std::ostream &out,
                            std::ostream &header,
                            const CallCapture &call,
                            const ParamCapture &param,
                            std::vector<uint8_t> *binaryData)
{
    const std::vector<uint8_t> &data = param.data[0];
    // null terminate C style string
    ASSERT(data.size() > 0 && data.back() == '\0');
    std::string str(data.begin(), data.end() - 1);

    constexpr size_t kMaxInlineStringLength = 20000;
    if (str.size() > kMaxInlineStringLength)
    {
        // Store in binary file if the string is too long.
        // Round up to 16-byte boundary for cross ABI safety.
        size_t offset = rx::roundUpPow2(binaryData->size(), kBinaryAlignment);
        binaryData->resize(offset + str.size() + 1);
        memcpy(binaryData->data() + offset, str.data(), str.size() + 1);
        out << "(const char *)&gBinaryData[" << offset << "]";
    }
    else if (str.find('\n') != std::string::npos)
    {
        std::string varName = replayWriter.getInlineVariableName(call.entryPoint, param.name);
        header << "const char " << varName << "[] = \n" << FmtMultiLineString(str) << ";";
        out << varName;
    }
    else
    {
        out << "\"" << str << "\"";
    }
}

void WriteStringPointerParamReplay(ReplayWriter &replayWriter,
                                   std::ostream &out,
                                   std::ostream &header,
                                   const CallCapture &call,
                                   const ParamCapture &param)
{
    // Concatenate the strings to ensure we get an accurate counter
    std::vector<std::string> strings;
    for (const std::vector<uint8_t> &data : param.data)
    {
        // null terminate C style string
        ASSERT(data.size() > 0 && data.back() == '\0');
        strings.emplace_back(data.begin(), data.end() - 1);
    }

    bool isNewEntry     = false;
    std::string varName = replayWriter.getInlineStringSetVariableName(call.entryPoint, param.name,
                                                                      strings, &isNewEntry);

    if (isNewEntry)
    {
        header << "const char *const " << varName << "[] = { \n";

        for (const std::string &str : strings)
        {
            // Break up long strings for MSVC
            size_t copyLength = 0;
            std::string separator;
            for (size_t i = 0; i < str.length(); i += kStringLengthLimit)
            {
                if ((str.length() - i) <= kStringLengthLimit)
                {
                    copyLength = str.length() - i;
                    separator  = ",";
                }
                else
                {
                    copyLength = kStringLengthLimit;
                    separator  = "";
                }

                header << FmtMultiLineString(str.substr(i, copyLength)) << separator << "\n";
            }
        }

        header << "};\n";
    }

    out << varName;
}

enum class Indent
{
    Indent,
    NoIdent,
};

void UpdateResourceIDBuffer(std::ostream &out,
                            Indent indent,
                            size_t bufferIndex,
                            ResourceIDType resourceIDType,
                            gl::ContextID contextID,
                            GLuint resourceID)
{
    if (indent == Indent::Indent)
    {
        out << "    ";
    }
    out << "UpdateResourceIDBuffer(" << bufferIndex << ", g"
        << GetResourceIDTypeName(resourceIDType) << "Map";
    if (IsTrackedPerContext(resourceIDType))
    {
        out << "PerContext[" << contextID.value << "]";
    }
    out << "[" << resourceID << "]);\n";
}

template <typename ParamT>
void WriteResourceIDPointerParamReplay(ReplayWriter &replayWriter,
                                       std::ostream &out,
                                       std::ostream &header,
                                       const CallCapture &call,
                                       const ParamCapture &param,
                                       size_t *maxResourceIDBufferSize)
{
    const ResourceIDType resourceIDType = GetResourceIDTypeFromParamType(param.type);
    ASSERT(resourceIDType != ResourceIDType::InvalidEnum);

    if (param.dataNElements > 0)
    {
        ASSERT(param.data.size() == 1);

        const ParamT *returnedIDs = reinterpret_cast<const ParamT *>(param.data[0].data());
        for (GLsizei resIndex = 0; resIndex < param.dataNElements; ++resIndex)
        {
            ParamT id = returnedIDs[resIndex];
            UpdateResourceIDBuffer(header, Indent::NoIdent, resIndex, resourceIDType,
                                   call.contextID, id.value);
        }

        *maxResourceIDBufferSize = std::max<size_t>(*maxResourceIDBufferSize, param.dataNElements);
    }

    out << "gResourceIDBuffer";
}

void WriteBinaryParamReplay(ReplayWriter &replayWriter,
                            std::ostream &out,
                            std::ostream &header,
                            const CallCapture &call,
                            const ParamCapture &param,
                            std::vector<uint8_t> *binaryData)
{
    std::string varName = replayWriter.getInlineVariableName(call.entryPoint, param.name);

    ASSERT(param.data.size() == 1);
    const std::vector<uint8_t> &data = param.data[0];

    // Only inline strings (shaders) to simplify the C code.
    ParamType overrideType = param.type;
    if (param.type == ParamType::TvoidConstPointer)
    {
        overrideType = ParamType::TGLubyteConstPointer;
    }
    if (overrideType == ParamType::TGLcharPointer)
    {
        // Inline if data is of type string
        std::string paramTypeString = ParamTypeToString(param.type);
        header << paramTypeString.substr(0, paramTypeString.length() - 1) << varName << "[] = { ";
        WriteInlineData<GLchar>(data, header);
        header << " };\n";
        out << varName;
    }
    else
    {
        // Store in binary file if data are not of type string
        // Round up to 16-byte boundary for cross ABI safety
        size_t offset = rx::roundUpPow2(binaryData->size(), kBinaryAlignment);
        binaryData->resize(offset + data.size());
        memcpy(binaryData->data() + offset, data.data(), data.size());
        out << "(" << ParamTypeToString(overrideType) << ")&gBinaryData[" << offset << "]";
    }
}

void WriteComment(std::ostream &out, const CallCapture &call)
{
    // Read the string parameter
    const ParamCapture &stringParam =
        call.params.getParam("comment", ParamType::TGLcharConstPointer, 0);
    const std::vector<uint8_t> &data = stringParam.data[0];
    ASSERT(data.size() > 0 && data.back() == '\0');
    std::string str(data.begin(), data.end() - 1);

    // Write the string prefixed with single line comment
    out << "// " << str;
}

void WriteCppReplayForCall(const CallCapture &call,
                           ReplayWriter &replayWriter,
                           std::ostream &out,
                           std::ostream &header,
                           std::vector<uint8_t> *binaryData,
                           size_t *maxResourceIDBufferSize)
{
    if (call.customFunctionName == "Comment")
    {
        // Just write it directly to the file and move on
        WriteComment(out, call);
        return;
    }

    std::ostringstream callOut;

    callOut << call.name() << "(";

    bool first = true;
    for (const ParamCapture &param : call.params.getParamCaptures())
    {
        if (!first)
        {
            callOut << ", ";
        }

        if (param.arrayClientPointerIndex != -1 && param.value.voidConstPointerVal != nullptr)
        {
            callOut << "gClientArrays[" << param.arrayClientPointerIndex << "]";
        }
        else if (param.readBufferSizeBytes > 0)
        {
            callOut << "(" << ParamTypeToString(param.type) << ")gReadBuffer";
        }
        else if (param.data.empty())
        {
            if (param.type == ParamType::TGLenum)
            {
                OutputGLenumString(callOut, param.enumGroup, param.value.GLenumVal);
            }
            else if (param.type == ParamType::TGLbitfield)
            {
                OutputGLbitfieldString(callOut, param.enumGroup, param.value.GLbitfieldVal);
            }
            else if (param.type == ParamType::TGLfloat)
            {
                WriteGLFloatValue(callOut, param.value.GLfloatVal);
            }
            else if (param.type == ParamType::TGLsync)
            {
                callOut << "gSyncMap[" << FmtPointerIndex(param.value.GLsyncVal) << "]";
            }
            else if (param.type == ParamType::TGLuint64 && param.name == "timeout")
            {
                if (param.value.GLuint64Val == GL_TIMEOUT_IGNORED)
                {
                    callOut << "GL_TIMEOUT_IGNORED";
                }
                else
                {
                    WriteParamCaptureReplay(callOut, call, param);
                }
            }
            else
            {
                WriteParamCaptureReplay(callOut, call, param);
            }
        }
        else
        {
            switch (param.type)
            {
                case ParamType::TGLcharConstPointer:
                    WriteStringParamReplay(replayWriter, callOut, header, call, param, binaryData);
                    break;
                case ParamType::TGLcharConstPointerPointer:
                    WriteStringPointerParamReplay(replayWriter, callOut, header, call, param);
                    break;
                case ParamType::TBufferIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::BufferID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                case ParamType::TFenceNVIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::FenceNVID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                case ParamType::TFramebufferIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::FramebufferID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                case ParamType::TMemoryObjectIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::MemoryObjectID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                case ParamType::TProgramPipelineIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::ProgramPipelineID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                case ParamType::TQueryIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::QueryID>(replayWriter, callOut, out, call,
                                                                   param, maxResourceIDBufferSize);
                    break;
                case ParamType::TRenderbufferIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::RenderbufferID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                case ParamType::TSamplerIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::SamplerID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                case ParamType::TSemaphoreIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::SemaphoreID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                case ParamType::TTextureIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::TextureID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                case ParamType::TTransformFeedbackIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::TransformFeedbackID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                case ParamType::TVertexArrayIDConstPointer:
                    WriteResourceIDPointerParamReplay<gl::VertexArrayID>(
                        replayWriter, callOut, out, call, param, maxResourceIDBufferSize);
                    break;
                default:
                    WriteBinaryParamReplay(replayWriter, callOut, header, call, param, binaryData);
                    break;
            }
        }

        first = false;
    }

    callOut << ")";

    out << callOut.str();
}

void AddComment(std::vector<CallCapture> *outCalls, const std::string &comment)
{

    ParamBuffer commentParamBuffer;
    ParamCapture commentParam("comment", ParamType::TGLcharConstPointer);
    CaptureString(comment.c_str(), &commentParam);
    commentParamBuffer.addParam(std::move(commentParam));
    outCalls->emplace_back("Comment", std::move(commentParamBuffer));
}

size_t MaxClientArraySize(const gl::AttribArray<size_t> &clientArraySizes)
{
    size_t found = 0;
    for (size_t size : clientArraySizes)
    {
        if (size > found)
        {
            found = size;
        }
    }

    return found;
}

std::string GetBinaryDataFilePath(bool compression, const std::string &captureLabel)
{
    std::stringstream fnameStream;
    fnameStream << FmtCapturePrefix(kNoContextId, captureLabel) << ".angledata";
    if (compression)
    {
        fnameStream << ".gz";
    }
    return fnameStream.str();
}

struct SaveFileHelper
{
  public:
    // We always use ios::binary to avoid inconsistent line endings when captured on Linux vs Win.
    SaveFileHelper(const std::string &filePathIn)
        : mOfs(filePathIn, std::ios::binary | std::ios::out), mFilePath(filePathIn)
    {
        if (!mOfs.is_open())
        {
            FATAL() << "Could not open " << filePathIn;
        }
    }
    ~SaveFileHelper() { printf("Saved '%s'.\n", mFilePath.c_str()); }

    template <typename T>
    SaveFileHelper &operator<<(const T &value)
    {
        mOfs << value;
        if (mOfs.bad())
        {
            FATAL() << "Error writing to " << mFilePath;
        }
        return *this;
    }

    void write(const uint8_t *data, size_t size)
    {
        mOfs.write(reinterpret_cast<const char *>(data), size);
    }

  private:
    void checkError();

    std::ofstream mOfs;
    std::string mFilePath;
};

void SaveBinaryData(bool compression,
                    const std::string &outDir,
                    gl::ContextID contextId,
                    const std::string &captureLabel,
                    const std::vector<uint8_t> &binaryData)
{
    std::string binaryDataFileName = GetBinaryDataFilePath(compression, captureLabel);
    std::string dataFilepath       = outDir + binaryDataFileName;

    SaveFileHelper saveData(dataFilepath);

    if (compression)
    {
        // Save compressed data.
        uLong uncompressedSize       = static_cast<uLong>(binaryData.size());
        uLong expectedCompressedSize = zlib_internal::GzipExpectedCompressedSize(uncompressedSize);

        std::vector<uint8_t> compressedData(expectedCompressedSize, 0);

        uLong compressedSize = expectedCompressedSize;
        int zResult = zlib_internal::GzipCompressHelper(compressedData.data(), &compressedSize,
                                                        binaryData.data(), uncompressedSize,
                                                        nullptr, nullptr);

        if (zResult != Z_OK)
        {
            FATAL() << "Error compressing binary data: " << zResult;
        }

        saveData.write(compressedData.data(), compressedSize);
    }
    else
    {
        saveData.write(binaryData.data(), binaryData.size());
    }
}

void WriteInitReplayCall(bool compression,
                         std::ostream &out,
                         gl::ContextID contextID,
                         const std::string &captureLabel,
                         size_t maxClientArraySize,
                         size_t readBufferSize,
                         size_t resourceIDBufferSize,
                         const PackedEnumMap<ResourceIDType, uint32_t> &maxIDs)
{
    std::string binaryDataFileName = GetBinaryDataFilePath(compression, captureLabel);

    out << "    // binaryDataFileName = " << binaryDataFileName << "\n";
    out << "    // maxClientArraySize = " << maxClientArraySize << "\n";
    out << "    // maxClientArraySize = " << maxClientArraySize << "\n";
    out << "    // readBufferSize = " << readBufferSize << "\n";
    out << "    // resourceIDBufferSize = " << resourceIDBufferSize << "\n";
    out << "    // contextID = " << contextID << "\n";

    for (ResourceIDType resourceID : AllEnums<ResourceIDType>())
    {
        const char *name = GetResourceIDTypeName(resourceID);
        out << "    // max" << name << " = " << maxIDs[resourceID] << "\n";
    }

    out << "    InitializeReplay4(\"" << binaryDataFileName << "\", " << maxClientArraySize << ", "
        << readBufferSize << ", " << resourceIDBufferSize << ", " << contextID;

    for (ResourceIDType resourceID : AllEnums<ResourceIDType>())
    {
        // Sanity check for catching e.g. uninitialized memory reads like b/380296979
        ASSERT(maxIDs[resourceID] < 1000000);
        out << ", " << maxIDs[resourceID];
    }

    out << ");\n";
}

void DeleteResourcesInReset(std::stringstream &out,
                            const gl::ContextID contextID,
                            const ResourceSet &newResources,
                            const ResourceSet &resourcesToDelete,
                            const ResourceIDType resourceIDType,
                            size_t *maxResourceIDBufferSize)
{
    if (!newResources.empty() || !resourcesToDelete.empty())
    {
        size_t count = 0;

        for (GLuint oldResource : resourcesToDelete)
        {
            UpdateResourceIDBuffer(out, Indent::Indent, count++, resourceIDType, contextID,
                                   oldResource);
        }

        for (GLuint newResource : newResources)
        {
            UpdateResourceIDBuffer(out, Indent::Indent, count++, resourceIDType, contextID,
                                   newResource);
        }

        // Delete all the new and old buffers at once
        out << "    glDelete" << GetResourceIDTypeName(resourceIDType) << "s(" << count
            << ", gResourceIDBuffer);\n";

        *maxResourceIDBufferSize = std::max(*maxResourceIDBufferSize, count);
    }
}

// TODO (http://anglebug.com/42263204): Reset more state on frame loop
void MaybeResetResources(egl::Display *display,
                         gl::ContextID contextID,
                         ResourceIDType resourceIDType,
                         ReplayWriter &replayWriter,
                         std::stringstream &out,
                         std::stringstream &header,
                         ResourceTracker *resourceTracker,
                         std::vector<uint8_t> *binaryData,
                         bool &anyResourceReset,
                         size_t *maxResourceIDBufferSize)
{
    // Track the initial output position so we can detect if it has moved
    std::streampos initialOutPos = out.tellp();

    switch (resourceIDType)
    {
        case ResourceIDType::Buffer:
        {
            TrackedResource &trackedBuffers =
                resourceTracker->getTrackedResource(contextID, ResourceIDType::Buffer);
            ResourceSet &newBuffers           = trackedBuffers.getNewResources();
            ResourceSet &buffersToDelete      = trackedBuffers.getResourcesToDelete();
            ResourceSet &buffersToRegen       = trackedBuffers.getResourcesToRegen();
            ResourceCalls &bufferRegenCalls   = trackedBuffers.getResourceRegenCalls();
            ResourceCalls &bufferRestoreCalls = trackedBuffers.getResourceRestoreCalls();

            BufferCalls &bufferMapCalls   = resourceTracker->getBufferMapCalls();
            BufferCalls &bufferUnmapCalls = resourceTracker->getBufferUnmapCalls();

            DeleteResourcesInReset(out, contextID, newBuffers, buffersToDelete, resourceIDType,
                                   maxResourceIDBufferSize);

            // If any of our starting buffers were deleted during the run, recreate them
            for (GLuint id : buffersToRegen)
            {
                // Emit their regen calls
                for (CallCapture &call : bufferRegenCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }

            // If any of our starting buffers were modified during the run, restore their contents
            ResourceSet &buffersToRestore = trackedBuffers.getResourcesToRestore();
            for (GLuint id : buffersToRestore)
            {
                if (resourceTracker->getStartingBuffersMappedCurrent(id))
                {
                    // Some drivers require the buffer to be unmapped before you can update data,
                    // which violates the spec. See gl::Buffer::bufferDataImpl().
                    for (CallCapture &call : bufferUnmapCalls[id])
                    {
                        out << "    ";
                        WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                              maxResourceIDBufferSize);
                        out << ";\n";
                    }
                }

                // Emit their restore calls
                for (CallCapture &call : bufferRestoreCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";

                    // Also note that this buffer has been implicitly unmapped by this call
                    resourceTracker->setBufferUnmapped(contextID, id);
                }
            }

            // Update the map/unmap of buffers to match the starting state
            ResourceSet startingBuffers = trackedBuffers.getStartingResources();
            for (GLuint id : startingBuffers)
            {
                // If the buffer was mapped at the start, but is not mapped now, we need to map
                if (resourceTracker->getStartingBuffersMappedInitial(id) &&
                    !resourceTracker->getStartingBuffersMappedCurrent(id))
                {
                    // Emit their map calls
                    for (CallCapture &call : bufferMapCalls[id])
                    {
                        out << "    ";
                        WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                              maxResourceIDBufferSize);
                        out << ";\n";
                    }
                }
                // If the buffer was unmapped at the start, but is mapped now, we need to unmap
                if (!resourceTracker->getStartingBuffersMappedInitial(id) &&
                    resourceTracker->getStartingBuffersMappedCurrent(id))
                {
                    // Emit their unmap calls
                    for (CallCapture &call : bufferUnmapCalls[id])
                    {
                        out << "    ";
                        WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                              maxResourceIDBufferSize);
                        out << ";\n";
                    }
                }
            }
            break;
        }
        case ResourceIDType::Framebuffer:
        {
            TrackedResource &trackedFramebuffers =
                resourceTracker->getTrackedResource(contextID, ResourceIDType::Framebuffer);
            ResourceSet &newFramebuffers           = trackedFramebuffers.getNewResources();
            ResourceSet &framebuffersToDelete      = trackedFramebuffers.getResourcesToDelete();
            ResourceSet &framebuffersToRegen       = trackedFramebuffers.getResourcesToRegen();
            ResourceCalls &framebufferRegenCalls   = trackedFramebuffers.getResourceRegenCalls();
            ResourceCalls &framebufferRestoreCalls = trackedFramebuffers.getResourceRestoreCalls();

            DeleteResourcesInReset(out, contextID, newFramebuffers, framebuffersToDelete,
                                   resourceIDType, maxResourceIDBufferSize);

            for (GLuint id : framebuffersToRegen)
            {
                // Emit their regen calls
                for (CallCapture &call : framebufferRegenCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }

            // If any of our starting framebuffers were modified during the run, restore their
            // contents
            ResourceSet &framebuffersToRestore = trackedFramebuffers.getResourcesToRestore();
            for (GLuint id : framebuffersToRestore)
            {
                // Emit their restore calls
                for (CallCapture &call : framebufferRestoreCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }
            break;
        }
        case ResourceIDType::Renderbuffer:
        {
            TrackedResource &trackedRenderbuffers =
                resourceTracker->getTrackedResource(contextID, ResourceIDType::Renderbuffer);
            ResourceSet &newRenderbuffers         = trackedRenderbuffers.getNewResources();
            ResourceSet &renderbuffersToDelete    = trackedRenderbuffers.getResourcesToDelete();
            ResourceSet &renderbuffersToRegen     = trackedRenderbuffers.getResourcesToRegen();
            ResourceCalls &renderbufferRegenCalls = trackedRenderbuffers.getResourceRegenCalls();
            ResourceCalls &renderbufferRestoreCalls =
                trackedRenderbuffers.getResourceRestoreCalls();

            DeleteResourcesInReset(out, contextID, newRenderbuffers, renderbuffersToDelete,
                                   resourceIDType, maxResourceIDBufferSize);

            for (GLuint id : renderbuffersToRegen)
            {
                // Emit their regen calls
                for (CallCapture &call : renderbufferRegenCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }

            // If any of our starting renderbuffers were modified during the run, restore their
            // contents
            ResourceSet &renderbuffersToRestore = trackedRenderbuffers.getResourcesToRestore();
            for (GLuint id : renderbuffersToRestore)
            {
                // Emit their restore calls
                for (CallCapture &call : renderbufferRestoreCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }
            break;
        }
        case ResourceIDType::ShaderProgram:
        {
            TrackedResource &trackedShaderPrograms =
                resourceTracker->getTrackedResource(contextID, ResourceIDType::ShaderProgram);
            ResourceSet &newShaderPrograms         = trackedShaderPrograms.getNewResources();
            ResourceSet &shaderProgramsToDelete    = trackedShaderPrograms.getResourcesToDelete();
            ResourceSet &shaderProgramsToRegen     = trackedShaderPrograms.getResourcesToRegen();
            ResourceSet &shaderProgramsToRestore   = trackedShaderPrograms.getResourcesToRestore();
            ResourceCalls &shaderProgramRegenCalls = trackedShaderPrograms.getResourceRegenCalls();
            ResourceCalls &shaderProgramRestoreCalls =
                trackedShaderPrograms.getResourceRestoreCalls();

            // If we have any new shaders or programs created and not deleted during the run, delete
            // them now
            for (const GLuint &newShaderProgram : newShaderPrograms)
            {
                if (resourceTracker->getShaderProgramType({newShaderProgram}) ==
                    ShaderProgramType::ShaderType)
                {
                    out << "    glDeleteShader(gShaderProgramMap[" << newShaderProgram << "]);\n";
                }
                else
                {
                    ASSERT(resourceTracker->getShaderProgramType({newShaderProgram}) ==
                           ShaderProgramType::ProgramType);
                    out << "    glDeleteProgram(gShaderProgramMap[" << newShaderProgram << "]);\n";
                }
            }

            // Do the same for shaders/programs to be deleted
            for (const GLuint &shaderProgramToDelete : shaderProgramsToDelete)
            {
                if (resourceTracker->getShaderProgramType({shaderProgramToDelete}) ==
                    ShaderProgramType::ShaderType)
                {
                    out << "    glDeleteShader(gShaderProgramMap[" << shaderProgramToDelete
                        << "]);\n";
                }
                else
                {
                    ASSERT(resourceTracker->getShaderProgramType({shaderProgramToDelete}) ==
                           ShaderProgramType::ProgramType);
                    out << "    glDeleteProgram(gShaderProgramMap[" << shaderProgramToDelete
                        << "]);\n";
                }
            }

            for (const GLuint id : shaderProgramsToRegen)
            {
                // Emit their regen calls
                for (CallCapture &call : shaderProgramRegenCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }

            for (const GLuint id : shaderProgramsToRestore)
            {
                // Emit their restore calls
                for (CallCapture &call : shaderProgramRestoreCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }

            break;
        }
        case ResourceIDType::Texture:
        {
            TrackedResource &trackedTextures =
                resourceTracker->getTrackedResource(contextID, ResourceIDType::Texture);
            ResourceSet &newTextures           = trackedTextures.getNewResources();
            ResourceSet &texturesToDelete      = trackedTextures.getResourcesToDelete();
            ResourceSet &texturesToRegen       = trackedTextures.getResourcesToRegen();
            ResourceCalls &textureRegenCalls   = trackedTextures.getResourceRegenCalls();
            ResourceCalls &textureRestoreCalls = trackedTextures.getResourceRestoreCalls();

            DeleteResourcesInReset(out, contextID, newTextures, texturesToDelete, resourceIDType,
                                   maxResourceIDBufferSize);

            // If any of our starting textures were deleted, regen them
            for (GLuint id : texturesToRegen)
            {
                // Emit their regen calls
                for (CallCapture &call : textureRegenCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }

            // If any of our starting textures were modified during the run, restore their contents
            ResourceSet &texturesToRestore = trackedTextures.getResourcesToRestore();

            // Do some setup if we have any textures to restore
            if (texturesToRestore.size() != 0)
            {
                // We need to unbind PIXEL_UNPACK_BUFFER before restoring textures
                // The correct binding will be restored in context state reset
                gl::Context *context = display->getContext(contextID);
                if (context->getState().getTargetBuffer(gl::BufferBinding::PixelUnpack))
                {
                    out << "    // Clearing PIXEL_UNPACK_BUFFER binding for texture restore\n";
                    out << "    ";
                    WriteCppReplayForCall(CaptureBindBuffer(context->getState(), true,
                                                            gl::BufferBinding::PixelUnpack, {0}),
                                          replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }

            for (GLuint id : texturesToRestore)
            {
                // Emit their restore calls
                for (CallCapture &call : textureRestoreCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }
            break;
        }
        case ResourceIDType::VertexArray:
        {
            TrackedResource &trackedVertexArrays =
                resourceTracker->getTrackedResource(contextID, ResourceIDType::VertexArray);
            ResourceSet &newVertexArrays           = trackedVertexArrays.getNewResources();
            ResourceSet &vertexArraysToDelete      = trackedVertexArrays.getResourcesToDelete();
            ResourceSet &vertexArraysToRegen       = trackedVertexArrays.getResourcesToRegen();
            ResourceSet &vertexArraysToRestore     = trackedVertexArrays.getResourcesToRestore();
            ResourceCalls &vertexArrayRegenCalls   = trackedVertexArrays.getResourceRegenCalls();
            ResourceCalls &vertexArrayRestoreCalls = trackedVertexArrays.getResourceRestoreCalls();

            DeleteResourcesInReset(out, contextID, newVertexArrays, vertexArraysToDelete,
                                   resourceIDType, maxResourceIDBufferSize);

            // If any of our starting vertex arrays were deleted during the run, recreate them
            for (GLuint id : vertexArraysToRegen)
            {
                // Emit their regen calls
                for (CallCapture &call : vertexArrayRegenCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }

            // If any of our starting vertex arrays were modified during the run, restore their
            // contents
            for (GLuint id : vertexArraysToRestore)
            {
                // Emit their restore calls
                for (CallCapture &call : vertexArrayRestoreCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }
            break;
        }
        case ResourceIDType::egl_Sync:
        {
            TrackedResource &trackedEGLSyncs =
                resourceTracker->getTrackedResource(contextID, ResourceIDType::egl_Sync);
            ResourceSet &newEGLSyncs         = trackedEGLSyncs.getNewResources();
            ResourceSet &eglSyncsToDelete    = trackedEGLSyncs.getResourcesToDelete();
            ResourceSet &eglSyncsToRegen     = trackedEGLSyncs.getResourcesToRegen();
            ResourceCalls &eglSyncRegenCalls = trackedEGLSyncs.getResourceRegenCalls();

            if (!newEGLSyncs.empty() || !eglSyncsToDelete.empty())
            {
                for (GLuint oldResource : eglSyncsToDelete)
                {
                    out << "    eglDestroySyncKHR(gEGLDisplay, gEGLSyncMap[" << oldResource
                        << "]);\n";
                }

                for (GLuint newResource : newEGLSyncs)
                {
                    out << "    eglDestroySyncKHR(gEGLDisplay, gEGLSyncMap[" << newResource
                        << "]);\n";
                }
            }

            // If any of our starting EGLsyncs were deleted during the run, recreate them
            for (GLuint id : eglSyncsToRegen)
            {
                // Emit their regen calls
                for (CallCapture &call : eglSyncRegenCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }
            break;
        }
        case ResourceIDType::Image:
        {
            TrackedResource &trackedEGLImages =
                resourceTracker->getTrackedResource(contextID, ResourceIDType::Image);
            ResourceSet &newEGLImages         = trackedEGLImages.getNewResources();
            ResourceSet &eglImagesToDelete    = trackedEGLImages.getResourcesToDelete();
            ResourceSet &eglImagesToRegen     = trackedEGLImages.getResourcesToRegen();
            ResourceCalls &eglImageRegenCalls = trackedEGLImages.getResourceRegenCalls();

            if (!newEGLImages.empty() || !eglImagesToDelete.empty())
            {
                for (GLuint oldResource : eglImagesToDelete)
                {
                    out << "    DestroyEGLImageKHR(gEGLDisplay, gEGLImageMap2[" << oldResource
                        << "], " << oldResource << ");\n";
                }

                for (GLuint newResource : newEGLImages)
                {
                    out << "    DestroyEGLImageKHR(gEGLDisplay, gEGLImageMap2[" << newResource
                        << "], " << newResource << ");\n";
                }
            }
            // If any of our starting EGLImages were deleted during the run, recreate them
            for (GLuint id : eglImagesToRegen)
            {
                // Emit their regen calls
                for (CallCapture &call : eglImageRegenCalls[id])
                {
                    out << "    ";
                    WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                          maxResourceIDBufferSize);
                    out << ";\n";
                }
            }
            break;
        }
        default:
            // TODO (http://anglebug.com/42263204): Reset more resource types
            break;
    }

    // If the output position has moved, we Reset something
    anyResourceReset = (initialOutPos != out.tellp());
}

void MaybeResetFenceSyncObjects(std::stringstream &out,
                                ReplayWriter &replayWriter,
                                std::stringstream &header,
                                ResourceTracker *resourceTracker,
                                std::vector<uint8_t> *binaryData,
                                size_t *maxResourceIDBufferSize)
{
    FenceSyncCalls &fenceSyncRegenCalls = resourceTracker->getFenceSyncRegenCalls();

    // If any of our starting fence sync objects were deleted during the run, recreate them
    FenceSyncSet &fenceSyncsToRegen = resourceTracker->getFenceSyncsToRegen();
    for (const gl::SyncID syncID : fenceSyncsToRegen)
    {
        // Emit their regen calls
        for (CallCapture &call : fenceSyncRegenCalls[syncID])
        {
            out << "    ";
            WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                  maxResourceIDBufferSize);
            out << ";\n";
        }
    }
}

void Capture(std::vector<CallCapture> *setupCalls, CallCapture &&call)
{
    setupCalls->emplace_back(std::move(call));
}

void CaptureUpdateCurrentProgram(const CallCapture &call,
                                 int programParamPos,
                                 std::vector<CallCapture> *callsOut)
{
    const ParamCapture &param =
        call.params.getParam("programPacked", ParamType::TShaderProgramID, programParamPos);
    gl::ShaderProgramID programID = param.value.ShaderProgramIDVal;

    ParamBuffer paramBuffer;
    paramBuffer.addValueParam("program", ParamType::TGLuint, programID.value);

    callsOut->emplace_back("UpdateCurrentProgram", std::move(paramBuffer));
}

bool ProgramNeedsReset(const gl::Context *context,
                       ResourceTracker *resourceTracker,
                       gl::ShaderProgramID programID)
{
    // Check whether the program is listed in programs to regen or restore
    TrackedResource &trackedShaderPrograms =
        resourceTracker->getTrackedResource(context->id(), ResourceIDType::ShaderProgram);

    ResourceSet &shaderProgramsToRegen = trackedShaderPrograms.getResourcesToRegen();
    if (shaderProgramsToRegen.count(programID.value) != 0)
    {
        return true;
    }

    ResourceSet &shaderProgramsToRestore = trackedShaderPrograms.getResourcesToRestore();
    if (shaderProgramsToRestore.count(programID.value) != 0)
    {
        return true;
    }

    // Deferred linked programs will also update their own uniforms
    FrameCaptureShared *frameCaptureShared = context->getShareGroup()->getFrameCaptureShared();
    if (frameCaptureShared->isDeferredLinkProgram(programID))
    {
        return true;
    }

    return false;
}

void MaybeResetDefaultUniforms(std::stringstream &out,
                               ReplayWriter &replayWriter,
                               std::stringstream &header,
                               const gl::Context *context,
                               ResourceTracker *resourceTracker,
                               std::vector<uint8_t> *binaryData,
                               size_t *maxResourceIDBufferSize)
{
    DefaultUniformLocationsPerProgramMap &defaultUniformsToReset =
        resourceTracker->getDefaultUniformsToReset();

    for (const auto &uniformIter : defaultUniformsToReset)
    {
        gl::ShaderProgramID programID               = uniformIter.first;
        const DefaultUniformLocationsSet &locations = uniformIter.second;

        if (ProgramNeedsReset(context, resourceTracker, programID))
        {
            // Skip programs marked for reset as they will update their own uniforms
            return;
        }

        // Bind the program to update its uniforms
        std::vector<CallCapture> bindCalls;
        Capture(&bindCalls, CaptureUseProgram(context->getState(), true, programID));
        CaptureUpdateCurrentProgram((&bindCalls)->back(), 0, &bindCalls);
        for (CallCapture &call : bindCalls)
        {
            out << "    ";
            WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                  maxResourceIDBufferSize);
            out << ";\n";
        }

        DefaultUniformCallsPerLocationMap &defaultUniformResetCalls =
            resourceTracker->getDefaultUniformResetCalls(programID);

        // Uniform arrays might have been modified in the middle (i.e. location 5 out of 10)
        // We only have Reset calls for the entire array, so emit them once for the entire array
        std::set<gl::UniformLocation> alreadyReset;

        // Emit the reset calls per modified location
        for (const gl::UniformLocation &location : locations)
        {
            gl::UniformLocation baseLocation =
                resourceTracker->getDefaultUniformBaseLocation(programID, location);
            if (alreadyReset.find(baseLocation) != alreadyReset.end())
            {
                // We've already Reset this array
                continue;
            }
            alreadyReset.insert(baseLocation);

            ASSERT(defaultUniformResetCalls.find(baseLocation) != defaultUniformResetCalls.end());
            std::vector<CallCapture> &callsPerLocation = defaultUniformResetCalls[baseLocation];

            for (CallCapture &call : callsPerLocation)
            {
                out << "    ";
                WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                      maxResourceIDBufferSize);
                out << ";\n";
            }
        }
    }
}

void MaybeResetOpaqueTypeObjects(ReplayWriter &replayWriter,
                                 std::stringstream &out,
                                 std::stringstream &header,
                                 const gl::Context *context,
                                 ResourceTracker *resourceTracker,
                                 std::vector<uint8_t> *binaryData,
                                 size_t *maxResourceIDBufferSize)
{
    MaybeResetFenceSyncObjects(out, replayWriter, header, resourceTracker, binaryData,
                               maxResourceIDBufferSize);

    MaybeResetDefaultUniforms(out, replayWriter, header, context, resourceTracker, binaryData,
                              maxResourceIDBufferSize);
}

void MaybeResetContextState(ReplayWriter &replayWriter,
                            std::stringstream &out,
                            std::stringstream &header,
                            ResourceTracker *resourceTracker,
                            const gl::Context *context,
                            std::vector<uint8_t> *binaryData,
                            StateResetHelper &stateResetHelper,
                            size_t *maxResourceIDBufferSize)
{
    // Check dirty states per entrypoint
    for (const EntryPoint &entryPoint : stateResetHelper.getDirtyEntryPoints())
    {
        const CallResetMap *resetCalls = &stateResetHelper.getResetCalls();

        // Create the default reset call for this entrypoint
        if (resetCalls->find(entryPoint) == resetCalls->end())
        {
            // If we don't have any reset calls for these entrypoints, that means we started capture
            // from the beginning, amd mid-execution capture was not invoked.
            stateResetHelper.setDefaultResetCalls(context, entryPoint);
        }

        // Emit the calls, if we added any
        if (resetCalls->find(entryPoint) != resetCalls->end())
        {
            for (const auto &call : resetCalls->at(entryPoint))
            {
                out << "    ";
                WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                      maxResourceIDBufferSize);
                out << ";\n";
            }
        }
    }

    // Reset buffer bindings that weren't bound at the beginning
    for (const gl::BufferBinding &dirtyBufferBinding : stateResetHelper.getDirtyBufferBindings())
    {
        // Check to see if dirty binding was part of starting set
        bool dirtyStartingBinding = false;
        for (const BufferBindingPair &startingBufferBinding :
             stateResetHelper.getStartingBufferBindings())
        {
            gl::BufferBinding startingBinding = startingBufferBinding.first;
            if (startingBinding == dirtyBufferBinding)
            {
                dirtyStartingBinding = true;
            }
        }

        // If the dirty binding was not part of starting bindings, clear it
        if (!dirtyStartingBinding)
        {
            out << "    ";
            WriteCppReplayForCall(
                CaptureBindBuffer(context->getState(), true, dirtyBufferBinding, {0}), replayWriter,
                out, header, binaryData, maxResourceIDBufferSize);
            out << ";\n";
        }
    }

    // Restore starting buffer bindings to initial state
    std::vector<CallCapture> &bufferBindingCalls = resourceTracker->getBufferBindingCalls();
    for (CallCapture &call : bufferBindingCalls)
    {
        out << "    ";
        WriteCppReplayForCall(call, replayWriter, out, header, binaryData, maxResourceIDBufferSize);
        out << ";\n";
    }

    // Restore texture bindings to initial state
    size_t activeTexture                 = context->getState().getActiveSampler();
    const TextureResetMap &resetBindings = stateResetHelper.getResetTextureBindings();
    for (const auto &textureBinding : stateResetHelper.getDirtyTextureBindings())
    {
        TextureResetMap::const_iterator id = resetBindings.find(textureBinding);
        if (id != resetBindings.end())
        {
            const auto &[unit, target] = textureBinding;

            // Set active texture unit if necessary
            if (unit != activeTexture)
            {
                out << "    ";
                WriteCppReplayForCall(CaptureActiveTexture(context->getState(), true,
                                                           GL_TEXTURE0 + static_cast<GLenum>(unit)),
                                      replayWriter, out, header, binaryData,
                                      maxResourceIDBufferSize);
                out << ";\n";
                activeTexture = unit;
            }

            // Bind texture for this target
            out << "    ";
            WriteCppReplayForCall(CaptureBindTexture(context->getState(), true, target, id->second),
                                  replayWriter, out, header, binaryData, maxResourceIDBufferSize);
            out << ";\n";
        }
    }

    // Restore active texture unit to initial state if necessary
    if (activeTexture != stateResetHelper.getResetActiveTexture())
    {
        out << "    ";
        WriteCppReplayForCall(
            CaptureActiveTexture(
                context->getState(), true,
                GL_TEXTURE0 + static_cast<GLenum>(stateResetHelper.getResetActiveTexture())),
            replayWriter, out, header, binaryData, maxResourceIDBufferSize);
        out << ";\n";
    }
}

void MarkResourceIDActive(ResourceIDType resourceType,
                          GLuint id,
                          std::vector<CallCapture> *setupCalls,
                          const ResourceIDToSetupCallsMap *resourceIDToSetupCallsMap)
{
    const std::map<GLuint, gl::Range<size_t>> &resourceSetupCalls =
        (*resourceIDToSetupCallsMap)[resourceType];
    const auto iter = resourceSetupCalls.find(id);
    if (iter == resourceSetupCalls.end())
    {
        return;
    }

    // Mark all of the calls that were used to initialize this resource as ACTIVE
    const gl::Range<size_t> &calls = iter->second;
    for (size_t index : calls)
    {
        (*setupCalls)[index].isActive = true;
    }
}

// Some replay functions can get quite large. If over a certain size, this method breaks up the
// function into parts to avoid overflowing the stack and causing slow compilation.
void WriteCppReplayFunctionWithParts(const gl::ContextID contextID,
                                     ReplayFunc replayFunc,
                                     ReplayWriter &replayWriter,
                                     uint32_t frameIndex,
                                     std::vector<uint8_t> *binaryData,
                                     const std::vector<CallCapture> &calls,
                                     std::stringstream &header,
                                     std::stringstream &out,
                                     size_t *maxResourceIDBufferSize)
{
    int callCount = 0;
    int partCount = 0;

    if (calls.size() > kFunctionSizeLimit)
    {
        out << "void "
            << FmtFunction(replayFunc, contextID, FuncUsage::Definition, frameIndex, ++partCount)
            << "\n";
    }
    else
    {
        out << "void "
            << FmtFunction(replayFunc, contextID, FuncUsage::Definition, frameIndex, kNoPartId)
            << "\n";
    }

    out << "{\n";

    for (const CallCapture &call : calls)
    {
        // Process active calls for Setup and inactive calls for SetupInactive
        if ((call.isActive && replayFunc != ReplayFunc::SetupInactive) ||
            (!call.isActive && replayFunc == ReplayFunc::SetupInactive))
        {
            out << "    ";
            WriteCppReplayForCall(call, replayWriter, out, header, binaryData,
                                  maxResourceIDBufferSize);
            out << ";\n";

            if (partCount > 0 && ++callCount % kFunctionSizeLimit == 0)
            {
                out << "}\n";
                out << "\n";
                out << "void "
                    << FmtFunction(replayFunc, contextID, FuncUsage::Definition, frameIndex,
                                   ++partCount)
                    << "\n";
                out << "{\n";
            }
        }
    }
    out << "}\n";

    if (partCount > 0)
    {
        out << "\n";
        out << "void "
            << FmtFunction(replayFunc, contextID, FuncUsage::Definition, frameIndex, kNoPartId)
            << "\n";
        out << "{\n";

        // Write out the main call which calls all the parts.
        for (int i = 1; i <= partCount; i++)
        {
            out << "    " << FmtFunction(replayFunc, contextID, FuncUsage::Call, frameIndex, i)
                << ";\n";
        }

        out << "}\n";
    }
}

// Performance can be gained by reordering traced calls and grouping them by context.
// Side context calls (as opposed to main context) can be grouped together paying attention
// to synchronization points in the original call stream.
void WriteCppReplayFunctionWithPartsMultiContext(const gl::ContextID contextID,
                                                 ReplayFunc replayFunc,
                                                 ReplayWriter &replayWriter,
                                                 uint32_t frameIndex,
                                                 std::vector<uint8_t> *binaryData,
                                                 std::vector<CallCapture> &calls,
                                                 std::stringstream &header,
                                                 std::stringstream &out,
                                                 size_t *maxResourceIDBufferSize)
{
    int callCount = 0;
    int partCount = 0;

    if (calls.size() > kFunctionSizeLimit)
    {
        out << "void "
            << FmtFunction(replayFunc, contextID, FuncUsage::Definition, frameIndex, ++partCount)
            << "\n";
    }
    else
    {
        out << "void "
            << FmtFunction(replayFunc, contextID, FuncUsage::Definition, frameIndex, kNoPartId)
            << "\n";
    }

    out << "{\n";

    std::map<gl::ContextID, std::queue<int>> sideContextCallIndices;

    // Helper lambda to write a context change command to the call stream
    auto writeMakeCurrentCall = [&](gl::ContextID cID) {
        CallCapture makeCurrentCall =
            egl::CaptureMakeCurrent(nullptr, true, nullptr, {0}, {0}, cID, EGL_TRUE);
        out << "    ";
        WriteCppReplayForCall(makeCurrentCall, replayWriter, out, header, binaryData,
                              maxResourceIDBufferSize);
        out << ";\n";
        callCount++;
    };

    // Helper lambda to write a call to the call stream
    auto writeCall = [&](CallCapture &outCall, gl::ContextID cID) {
        out << "    ";
        WriteCppReplayForCall(outCall, replayWriter, out, header, binaryData,
                              maxResourceIDBufferSize);
        out << ";\n";
        if (cID != contextID)
        {
            sideContextCallIndices[cID].pop();
        }
        callCount++;
    };

    int callIndex = 0;
    // Iterate through calls saving side context call indices in a per-side-context queue
    for (CallCapture &call : calls)
    {
        if (call.contextID != contextID)
        {
            sideContextCallIndices[call.contextID].push(callIndex);
        }
        callIndex++;
    }

    // At the beginning of the frame, output all side context calls occuring before a sync point.
    // If no sync points are present, all calls in that side context are written at this time
    for (auto const &sideContext : sideContextCallIndices)
    {
        gl::ContextID sideContextID = sideContext.first;

        // Make sidecontext current if there are commands before the first syncpoint
        if (!calls[sideContextCallIndices[sideContextID].front()].isSyncPoint)
        {
            writeMakeCurrentCall(sideContextID);
        }
        // Output all commands in sidecontext until a syncpoint is reached
        while (!sideContextCallIndices[sideContextID].empty() &&
               !calls[sideContextCallIndices[sideContextID].front()].isSyncPoint)
        {
            writeCall(calls[sideContextCallIndices[sideContextID].front()], sideContextID);
        }
    }

    // Make mainContext current
    writeMakeCurrentCall(contextID);

    // Iterate through calls writing out main context calls. When a sync point is reached, write the
    // next queued sequence of side context calls until another sync point is reached.
    for (CallCapture &call : calls)
    {
        if (call.contextID == contextID)
        {
            writeCall(call, call.contextID);
        }
        else
        {
            if (call.isSyncPoint)
            {
                // Make sideContext current
                writeMakeCurrentCall(call.contextID);

                do
                {
                    writeCall(calls[sideContextCallIndices[call.contextID].front()],
                              call.contextID);
                } while (!sideContextCallIndices[call.contextID].empty() &&
                         !calls[sideContextCallIndices[call.contextID].front()].isSyncPoint);

                // Make mainContext current
                writeMakeCurrentCall(contextID);

                if (partCount > 0 && ++callCount % kFunctionSizeLimit == 0)
                {
                    out << "}\n";
                    out << "\n";
                    out << "void "
                        << FmtFunction(replayFunc, contextID, FuncUsage::Definition, frameIndex,
                                       ++partCount)
                        << "\n";
                    out << "{\n";
                }
            }
        }
    }
    out << "}\n";

    if (partCount > 0)
    {
        out << "\n";
        out << "void "
            << FmtFunction(replayFunc, contextID, FuncUsage::Definition, frameIndex, kNoPartId)
            << "\n";
        out << "{\n";

        // Write out the main call which calls all the parts.
        for (int i = 1; i <= partCount; i++)
        {
            out << "    " << FmtFunction(replayFunc, contextID, FuncUsage::Call, frameIndex, i)
                << ";\n";
        }

        out << "}\n";
    }
}

// Auxiliary contexts are other contexts in the share group that aren't the context calling
// eglSwapBuffers().
void WriteAuxiliaryContextCppSetupReplay(ReplayWriter &replayWriter,
                                         bool compression,
                                         const std::string &outDir,
                                         const gl::Context *context,
                                         const std::string &captureLabel,
                                         uint32_t frameIndex,
                                         const std::vector<CallCapture> &setupCalls,
                                         std::vector<uint8_t> *binaryData,
                                         bool serializeStateEnabled,
                                         const FrameCaptureShared &frameCaptureShared,
                                         size_t *maxResourceIDBufferSize)
{
    ASSERT(frameCaptureShared.getWindowSurfaceContextID() != context->id());

    {
        std::stringstream filenameStream;
        filenameStream << outDir << FmtCapturePrefix(context->id(), captureLabel);
        std::string filenamePattern = filenameStream.str();
        replayWriter.setFilenamePattern(filenamePattern);
    }

    {
        std::stringstream include;
        include << "#include \""
                << FmtCapturePrefix(frameCaptureShared.getWindowSurfaceContextID(), captureLabel)
                << ".h\"\n";
        include << "#include \"angle_trace_gl.h\"\n";

        std::string frameIncludes = include.str();
        replayWriter.setSourcePrologue(frameIncludes);
        replayWriter.setHeaderPrologue(frameIncludes);
    }

    {
        std::stringstream protoStream;
        std::stringstream headerStream;
        std::stringstream bodyStream;

        protoStream << "void " << FmtSetupFunction(kNoPartId, context->id(), FuncUsage::Prototype);
        std::string proto = protoStream.str();

        WriteCppReplayFunctionWithParts(context->id(), ReplayFunc::Setup, replayWriter, frameIndex,
                                        binaryData, setupCalls, headerStream, bodyStream,
                                        maxResourceIDBufferSize);

        replayWriter.addPrivateFunction(proto, headerStream, bodyStream);
    }

    replayWriter.saveFrame();
}

void WriteShareGroupCppSetupReplay(ReplayWriter &replayWriter,
                                   bool compression,
                                   const std::string &outDir,
                                   const std::string &captureLabel,
                                   uint32_t frameIndex,
                                   uint32_t frameCount,
                                   const std::vector<CallCapture> &setupCalls,
                                   ResourceTracker *resourceTracker,
                                   std::vector<uint8_t> *binaryData,
                                   bool serializeStateEnabled,
                                   gl::ContextID windowSurfaceContextID,
                                   size_t *maxResourceIDBufferSize)
{
    {

        std::stringstream include;

        include << "#include \"angle_trace_gl.h\"\n";
        include << "#include \"" << FmtCapturePrefix(windowSurfaceContextID, captureLabel)
                << ".h\"\n";

        std::string includeString = include.str();

        replayWriter.setSourcePrologue(includeString);
    }

    {
        std::stringstream protoStream;
        std::stringstream headerStream;
        std::stringstream bodyStream;

        protoStream << "void "
                    << FmtSetupFunction(kNoPartId, kSharedContextId, FuncUsage::Prototype);
        std::string proto = protoStream.str();

        WriteCppReplayFunctionWithParts(kSharedContextId, ReplayFunc::Setup, replayWriter,
                                        frameIndex, binaryData, setupCalls, headerStream,
                                        bodyStream, maxResourceIDBufferSize);

        replayWriter.addPrivateFunction(proto, headerStream, bodyStream);

        protoStream.str("");
        headerStream.str("");
        bodyStream.str("");
        protoStream << "void "
                    << FmtSetupInactiveFunction(kNoPartId, kSharedContextId, FuncUsage::Prototype);
        proto = protoStream.str();

        WriteCppReplayFunctionWithParts(kSharedContextId, ReplayFunc::SetupInactive, replayWriter,
                                        frameIndex, binaryData, setupCalls, headerStream,
                                        bodyStream, maxResourceIDBufferSize);
        replayWriter.addPrivateFunction(proto, headerStream, bodyStream);
    }

    {
        std::stringstream filenameStream;
        filenameStream << outDir << FmtCapturePrefix(kSharedContextId, captureLabel);

        std::string filenamePattern = filenameStream.str();

        replayWriter.setFilenamePattern(filenamePattern);
    }

    replayWriter.saveSetupFile();
}

ProgramSources GetAttachedProgramSources(const gl::Context *context, const gl::Program *program)
{
    ProgramSources sources;
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Shader *shader = program->getAttachedShader(shaderType);
        if (shader)
        {
            sources[shaderType] = shader->getSourceString();
        }
    }
    return sources;
}

template <typename IDType>
void CaptureUpdateResourceIDs(const gl::Context *context,
                              const CallCapture &call,
                              const ParamCapture &param,
                              ResourceTracker *resourceTracker,
                              std::vector<CallCapture> *callsOut)
{
    GLsizei n = call.params.getParamFlexName("n", "count", ParamType::TGLsizei, 0).value.GLsizeiVal;
    ASSERT(param.data.size() == 1);
    ResourceIDType resourceIDType = GetResourceIDTypeFromParamType(param.type);
    ASSERT(resourceIDType != ResourceIDType::InvalidEnum &&
           resourceIDType != ResourceIDType::ShaderProgram);
    const char *resourceName = GetResourceIDTypeName(resourceIDType);

    std::stringstream updateFuncNameStr;
    updateFuncNameStr << "Update" << resourceName << "ID";
    bool trackedPerContext = IsTrackedPerContext(resourceIDType);
    if (trackedPerContext)
    {
        // TODO (https://issuetracker.google.com/169868803) The '2' version can be removed after all
        // context-local objects are tracked per-context
        updateFuncNameStr << "2";
    }
    std::string updateFuncName = updateFuncNameStr.str();

    const IDType *returnedIDs = reinterpret_cast<const IDType *>(param.data[0].data());

    ResourceSet &startingSet =
        resourceTracker->getTrackedResource(context->id(), resourceIDType).getStartingResources();

    for (GLsizei idIndex = 0; idIndex < n; ++idIndex)
    {
        IDType id                = returnedIDs[idIndex];
        GLsizei readBufferOffset = idIndex * sizeof(gl::RenderbufferID);
        ParamBuffer params;
        if (trackedPerContext)
        {
            params.addValueParam("contextId", ParamType::TGLuint, context->id().value);
        }
        params.addValueParam("id", ParamType::TGLuint, id.value);
        params.addValueParam("readBufferOffset", ParamType::TGLsizei, readBufferOffset);
        callsOut->emplace_back(updateFuncName, std::move(params));

        // Add only if not in starting resources.
        if (startingSet.find(id.value) == startingSet.end())
        {
            resourceTracker->getTrackedResource(context->id(), resourceIDType)
                .getNewResources()
                .insert(id.value);
        }
    }
}

void CaptureUpdateUniformLocations(const gl::Program *program, std::vector<CallCapture> *callsOut)
{
    const gl::ProgramExecutable &executable            = program->getExecutable();
    const std::vector<gl::LinkedUniform> &uniforms     = executable.getUniforms();
    const std::vector<gl::VariableLocation> &locations = executable.getUniformLocations();

    for (GLint location = 0; location < static_cast<GLint>(locations.size()); ++location)
    {
        const gl::VariableLocation &locationVar = locations[location];

        // This handles the case where the application calls glBindUniformLocationCHROMIUM
        // on an unused uniform. We must still store a -1 into gUniformLocations in case the
        // application attempts to call a glUniform* call. To do this we'll pass in a blank name to
        // force glGetUniformLocation to return -1.
        std::string name;
        int count = 1;
        ParamBuffer params;
        params.addValueParam("program", ParamType::TGLuint, program->id().value);

        if (locationVar.index >= uniforms.size())
        {
            name = "";
        }
        else
        {
            const gl::LinkedUniform &uniform = uniforms[locationVar.index];

            name = executable.getUniformNameByIndex(locationVar.index);

            if (uniform.isArray())
            {
                if (locationVar.arrayIndex > 0)
                {
                    // Non-sequential array uniform locations are not currently handled.
                    // In practice array locations shouldn't ever be non-sequential.
                    ASSERT(uniform.getLocation() == -1 ||
                           location ==
                               uniform.getLocation() + static_cast<int>(locationVar.arrayIndex));
                    continue;
                }

                name  = gl::StripLastArrayIndex(name);
                count = uniform.getBasicTypeElementCount();
            }
        }

        ParamCapture nameParam("name", ParamType::TGLcharConstPointer);
        CaptureString(name.c_str(), &nameParam);
        params.addParam(std::move(nameParam));
        params.addValueParam("location", ParamType::TGLint, location);
        params.addValueParam("count", ParamType::TGLint, static_cast<GLint>(count));
        callsOut->emplace_back("UpdateUniformLocation", std::move(params));
    }
}

void CaptureValidateSerializedState(const gl::Context *context, std::vector<CallCapture> *callsOut)
{
    INFO() << "Capturing validation checkpoint at position " << callsOut->size();

    context->finishImmutable();

    std::string serializedState;
    angle::Result result = angle::SerializeContextToString(context, &serializedState);
    if (result != angle::Result::Continue)
    {
        ERR() << "Internal error serializing context state.";
        return;
    }
    ParamCapture serializedStateParam("serializedState", ParamType::TGLcharConstPointer);
    CaptureString(serializedState.c_str(), &serializedStateParam);

    ParamBuffer params;
    params.addParam(std::move(serializedStateParam));

    callsOut->emplace_back("VALIDATE_CHECKPOINT", std::move(params));
}

void CaptureUpdateUniformBlockIndexes(const gl::Program *program,
                                      std::vector<CallCapture> *callsOut)
{
    const std::vector<gl::InterfaceBlock> &uniformBlocks =
        program->getExecutable().getUniformBlocks();

    for (GLuint index = 0; index < uniformBlocks.size(); ++index)
    {
        ParamBuffer params;

        std::string name;
        params.addValueParam("program", ParamType::TShaderProgramID, program->id());

        const std::string fullName = uniformBlocks[index].nameWithArrayIndex();
        ParamCapture nameParam("name", ParamType::TGLcharConstPointer);
        CaptureString(fullName.c_str(), &nameParam);
        params.addParam(std::move(nameParam));

        params.addValueParam("index", ParamType::TGLuint, index);
        callsOut->emplace_back("UpdateUniformBlockIndex", std::move(params));
    }
}

void CaptureDeleteUniformLocations(gl::ShaderProgramID program, std::vector<CallCapture> *callsOut)
{
    ParamBuffer params;
    params.addValueParam("program", ParamType::TShaderProgramID, program);
    callsOut->emplace_back("DeleteUniformLocations", std::move(params));
}

void MaybeCaptureUpdateResourceIDs(const gl::Context *context,
                                   ResourceTracker *resourceTracker,
                                   std::vector<CallCapture> *callsOut)
{
    const CallCapture &call = callsOut->back();

    switch (call.entryPoint)
    {
        case EntryPoint::GLGenBuffers:
        {
            const ParamCapture &buffers =
                call.params.getParam("buffersPacked", ParamType::TBufferIDPointer, 1);
            CaptureUpdateResourceIDs<gl::BufferID>(context, call, buffers, resourceTracker,
                                                   callsOut);
            break;
        }

        case EntryPoint::GLGenFencesNV:
        {
            const ParamCapture &fences =
                call.params.getParam("fencesPacked", ParamType::TFenceNVIDPointer, 1);
            CaptureUpdateResourceIDs<gl::FenceNVID>(context, call, fences, resourceTracker,
                                                    callsOut);
            break;
        }

        case EntryPoint::GLGenFramebuffers:
        case EntryPoint::GLGenFramebuffersOES:
        {
            const ParamCapture &framebuffers =
                call.params.getParam("framebuffersPacked", ParamType::TFramebufferIDPointer, 1);
            CaptureUpdateResourceIDs<gl::FramebufferID>(context, call, framebuffers,
                                                        resourceTracker, callsOut);
            break;
        }

        case EntryPoint::GLGenProgramPipelines:
        {
            const ParamCapture &pipelines =
                call.params.getParam("pipelinesPacked", ParamType::TProgramPipelineIDPointer, 1);
            CaptureUpdateResourceIDs<gl::ProgramPipelineID>(context, call, pipelines,
                                                            resourceTracker, callsOut);
            break;
        }

        case EntryPoint::GLGenQueries:
        case EntryPoint::GLGenQueriesEXT:
        {
            const ParamCapture &queries =
                call.params.getParam("idsPacked", ParamType::TQueryIDPointer, 1);
            CaptureUpdateResourceIDs<gl::QueryID>(context, call, queries, resourceTracker,
                                                  callsOut);
            break;
        }

        case EntryPoint::GLGenRenderbuffers:
        case EntryPoint::GLGenRenderbuffersOES:
        {
            const ParamCapture &renderbuffers =
                call.params.getParam("renderbuffersPacked", ParamType::TRenderbufferIDPointer, 1);
            CaptureUpdateResourceIDs<gl::RenderbufferID>(context, call, renderbuffers,
                                                         resourceTracker, callsOut);
            break;
        }

        case EntryPoint::GLGenSamplers:
        {
            const ParamCapture &samplers =
                call.params.getParam("samplersPacked", ParamType::TSamplerIDPointer, 1);
            CaptureUpdateResourceIDs<gl::SamplerID>(context, call, samplers, resourceTracker,
                                                    callsOut);
            break;
        }

        case EntryPoint::GLGenSemaphoresEXT:
        {
            const ParamCapture &semaphores =
                call.params.getParam("semaphoresPacked", ParamType::TSemaphoreIDPointer, 1);
            CaptureUpdateResourceIDs<gl::SemaphoreID>(context, call, semaphores, resourceTracker,
                                                      callsOut);
            break;
        }

        case EntryPoint::GLGenTextures:
        {
            const ParamCapture &textures =
                call.params.getParam("texturesPacked", ParamType::TTextureIDPointer, 1);
            CaptureUpdateResourceIDs<gl::TextureID>(context, call, textures, resourceTracker,
                                                    callsOut);
            break;
        }

        case EntryPoint::GLGenTransformFeedbacks:
        {
            const ParamCapture &xfbs =
                call.params.getParam("idsPacked", ParamType::TTransformFeedbackIDPointer, 1);
            CaptureUpdateResourceIDs<gl::TransformFeedbackID>(context, call, xfbs, resourceTracker,
                                                              callsOut);
            break;
        }

        case EntryPoint::GLGenVertexArrays:
        case EntryPoint::GLGenVertexArraysOES:
        {
            const ParamCapture &vertexArrays =
                call.params.getParam("arraysPacked", ParamType::TVertexArrayIDPointer, 1);
            CaptureUpdateResourceIDs<gl::VertexArrayID>(context, call, vertexArrays,
                                                        resourceTracker, callsOut);
            break;
        }

        case EntryPoint::GLCreateMemoryObjectsEXT:
        {
            const ParamCapture &memoryObjects =
                call.params.getParam("memoryObjectsPacked", ParamType::TMemoryObjectIDPointer, 1);
            CaptureUpdateResourceIDs<gl::MemoryObjectID>(context, call, memoryObjects,
                                                         resourceTracker, callsOut);
            break;
        }

        default:
            break;
    }
}

bool IsDefaultCurrentValue(const gl::VertexAttribCurrentValueData &currentValue)
{
    if (currentValue.Type != gl::VertexAttribType::Float)
        return false;

    return currentValue.Values.FloatValues[0] == 0.0f &&
           currentValue.Values.FloatValues[1] == 0.0f &&
           currentValue.Values.FloatValues[2] == 0.0f && currentValue.Values.FloatValues[3] == 1.0f;
}

bool IsQueryActive(const gl::State &glState, gl::QueryID &queryID)
{
    const gl::ActiveQueryMap &activeQueries = glState.getActiveQueriesForCapture();
    for (const auto &activeQueryIter : activeQueries)
    {
        const gl::Query *activeQuery = activeQueryIter.get();
        if (activeQuery && activeQuery->id() == queryID)
        {
            return true;
        }
    }

    return false;
}

bool IsTextureUpdate(CallCapture &call)
{
    switch (call.entryPoint)
    {
        case EntryPoint::GLCompressedCopyTextureCHROMIUM:
        case EntryPoint::GLCompressedTexImage2D:
        case EntryPoint::GLCompressedTexImage2DRobustANGLE:
        case EntryPoint::GLCompressedTexImage3D:
        case EntryPoint::GLCompressedTexImage3DOES:
        case EntryPoint::GLCompressedTexImage3DRobustANGLE:
        case EntryPoint::GLCompressedTexSubImage2D:
        case EntryPoint::GLCompressedTexSubImage2DRobustANGLE:
        case EntryPoint::GLCompressedTexSubImage3D:
        case EntryPoint::GLCompressedTexSubImage3DOES:
        case EntryPoint::GLCompressedTexSubImage3DRobustANGLE:
        case EntryPoint::GLCopyTexImage2D:
        case EntryPoint::GLCopyTexSubImage2D:
        case EntryPoint::GLCopyTexSubImage3D:
        case EntryPoint::GLCopyTexSubImage3DOES:
        case EntryPoint::GLCopyTexture3DANGLE:
        case EntryPoint::GLCopyTextureCHROMIUM:
        case EntryPoint::GLTexImage2D:
        case EntryPoint::GLTexImage2DExternalANGLE:
        case EntryPoint::GLTexImage2DRobustANGLE:
        case EntryPoint::GLTexImage3D:
        case EntryPoint::GLTexImage3DOES:
        case EntryPoint::GLTexImage3DRobustANGLE:
        case EntryPoint::GLTexSubImage2D:
        case EntryPoint::GLTexSubImage2DRobustANGLE:
        case EntryPoint::GLTexSubImage3D:
        case EntryPoint::GLTexSubImage3DOES:
        case EntryPoint::GLTexSubImage3DRobustANGLE:
        case EntryPoint::GLCopyImageSubData:
        case EntryPoint::GLCopyImageSubDataEXT:
        case EntryPoint::GLCopyImageSubDataOES:
            return true;
        default:
            return false;
    }
}

bool IsImageUpdate(CallCapture &call)
{
    switch (call.entryPoint)
    {
        case EntryPoint::GLDispatchCompute:
        case EntryPoint::GLDispatchComputeIndirect:
            return true;
        default:
            return false;
    }
}

bool IsVertexArrayUpdate(CallCapture &call)
{
    switch (call.entryPoint)
    {
        case EntryPoint::GLVertexAttribFormat:
        case EntryPoint::GLVertexAttribIFormat:
        case EntryPoint::GLBindVertexBuffer:
        case EntryPoint::GLVertexAttribBinding:
        case EntryPoint::GLVertexAttribPointer:
        case EntryPoint::GLVertexAttribIPointer:
        case EntryPoint::GLEnableVertexAttribArray:
        case EntryPoint::GLDisableVertexAttribArray:
        case EntryPoint::GLVertexBindingDivisor:
        case EntryPoint::GLVertexAttribDivisor:
            return true;
        default:
            return false;
    }
}

bool IsSharedObjectResource(ResourceIDType type)
{
    // This helper function informs us which objects are shared vs. per context
    //
    //   OpenGL ES Version 3.2 (October 22, 2019)
    //   Chapter 5 Shared Objects and Multiple Contexts:
    //
    //   - Objects that can be shared between contexts include buffer objects, program
    //     and shader objects, renderbuffer objects, sampler objects, sync objects, and texture
    //     objects (except for the texture objects named zero).
    //   - Objects which contain references to other objects include framebuffer, program
    //     pipeline, transform feedback, and vertex array objects. Such objects are called
    //     container objects and are not shared.
    //
    // Notably absent from this list are Sync objects, which are not ResourceIDType, are handled
    // elsewhere, and are shared:
    //   - 2.6.13 Sync Objects: Sync objects may be shared.

    switch (type)
    {
        case ResourceIDType::Buffer:
            // 2.6.2 Buffer Objects: Buffer objects may be shared.
            return true;

        case ResourceIDType::Framebuffer:
            // 2.6.9 Framebuffer Objects: Framebuffer objects are container objects including
            // references to renderbuffer and / or texture objects, and are not shared.
            return false;

        case ResourceIDType::ProgramPipeline:
            // 2.6.5 Program Pipeline Objects: Program pipeline objects are container objects
            // including references to program objects, and are not shared.
            return false;

        case ResourceIDType::TransformFeedback:
            // 2.6.11 Transform Feedback Objects: Transform feedback objects are container objects
            // including references to buffer objects, and are not shared
            return false;

        case ResourceIDType::VertexArray:
            // 2.6.10 Vertex Array Objects: Vertex array objects are container objects including
            // references to buffer objects, and are not shared
            return false;

        case ResourceIDType::FenceNV:
            // From https://registry.khronos.org/OpenGL/extensions/NV/NV_fence.txt
            //  Are the fences sharable between multiple contexts?
            //   RESOLUTION: No.
            return false;

        case ResourceIDType::Renderbuffer:
            // 2.6.8 Renderbuffer Objects: Renderbuffer objects may be shared.
            return true;

        case ResourceIDType::ShaderProgram:
            // 2.6.3 Shader Objects: Shader objects may be shared.
            // 2.6.4 Program Objects: Program objects may be shared.
            return true;

        case ResourceIDType::Sampler:
            // 2.6.7 Sampler Objects: Sampler objects may be shared
            return true;

        case ResourceIDType::Sync:
            // 2.6.13 Sync Objects: Sync objects may be shared.
            return true;

        case ResourceIDType::Texture:
            // 2.6.6 Texture Objects: Texture objects may be shared
            return true;

        case ResourceIDType::Query:
            // 2.6.12 Query Objects: Query objects are not shared
            return false;

        case ResourceIDType::Semaphore:
            // From https://registry.khronos.org/OpenGL/extensions/EXT/EXT_external_objects.txt
            // 2.6.14 Semaphore Objects: Semaphore objects may be shared.
            return true;

        case ResourceIDType::MemoryObject:
            // From https://registry.khronos.org/OpenGL/extensions/EXT/EXT_external_objects.txt
            // 2.6.15 Memory Objects: Memory objects may be shared.
            return true;

        case ResourceIDType::Context:
        case ResourceIDType::Image:
        case ResourceIDType::Surface:
        case ResourceIDType::egl_Sync:
            // EGL types are associated with a display and not bound to a context
            // For the way this function is used, we can treat them as shared.
            return true;

        case ResourceIDType::EnumCount:
        default:
            ERR() << "Unhandled ResourceIDType= " << static_cast<int>(type);
            UNREACHABLE();
            return false;
    }
}

enum class DefaultUniformType
{
    None,
    CurrentProgram,
    SpecifiedProgram,
};

DefaultUniformType GetDefaultUniformType(const CallCapture &call)
{
    switch (call.entryPoint)
    {
        case EntryPoint::GLProgramUniform1f:
        case EntryPoint::GLProgramUniform1fEXT:
        case EntryPoint::GLProgramUniform1fv:
        case EntryPoint::GLProgramUniform1fvEXT:
        case EntryPoint::GLProgramUniform1i:
        case EntryPoint::GLProgramUniform1iEXT:
        case EntryPoint::GLProgramUniform1iv:
        case EntryPoint::GLProgramUniform1ivEXT:
        case EntryPoint::GLProgramUniform1ui:
        case EntryPoint::GLProgramUniform1uiEXT:
        case EntryPoint::GLProgramUniform1uiv:
        case EntryPoint::GLProgramUniform1uivEXT:
        case EntryPoint::GLProgramUniform2f:
        case EntryPoint::GLProgramUniform2fEXT:
        case EntryPoint::GLProgramUniform2fv:
        case EntryPoint::GLProgramUniform2fvEXT:
        case EntryPoint::GLProgramUniform2i:
        case EntryPoint::GLProgramUniform2iEXT:
        case EntryPoint::GLProgramUniform2iv:
        case EntryPoint::GLProgramUniform2ivEXT:
        case EntryPoint::GLProgramUniform2ui:
        case EntryPoint::GLProgramUniform2uiEXT:
        case EntryPoint::GLProgramUniform2uiv:
        case EntryPoint::GLProgramUniform2uivEXT:
        case EntryPoint::GLProgramUniform3f:
        case EntryPoint::GLProgramUniform3fEXT:
        case EntryPoint::GLProgramUniform3fv:
        case EntryPoint::GLProgramUniform3fvEXT:
        case EntryPoint::GLProgramUniform3i:
        case EntryPoint::GLProgramUniform3iEXT:
        case EntryPoint::GLProgramUniform3iv:
        case EntryPoint::GLProgramUniform3ivEXT:
        case EntryPoint::GLProgramUniform3ui:
        case EntryPoint::GLProgramUniform3uiEXT:
        case EntryPoint::GLProgramUniform3uiv:
        case EntryPoint::GLProgramUniform3uivEXT:
        case EntryPoint::GLProgramUniform4f:
        case EntryPoint::GLProgramUniform4fEXT:
        case EntryPoint::GLProgramUniform4fv:
        case EntryPoint::GLProgramUniform4fvEXT:
        case EntryPoint::GLProgramUniform4i:
        case EntryPoint::GLProgramUniform4iEXT:
        case EntryPoint::GLProgramUniform4iv:
        case EntryPoint::GLProgramUniform4ivEXT:
        case EntryPoint::GLProgramUniform4ui:
        case EntryPoint::GLProgramUniform4uiEXT:
        case EntryPoint::GLProgramUniform4uiv:
        case EntryPoint::GLProgramUniform4uivEXT:
        case EntryPoint::GLProgramUniformMatrix2fv:
        case EntryPoint::GLProgramUniformMatrix2fvEXT:
        case EntryPoint::GLProgramUniformMatrix2x3fv:
        case EntryPoint::GLProgramUniformMatrix2x3fvEXT:
        case EntryPoint::GLProgramUniformMatrix2x4fv:
        case EntryPoint::GLProgramUniformMatrix2x4fvEXT:
        case EntryPoint::GLProgramUniformMatrix3fv:
        case EntryPoint::GLProgramUniformMatrix3fvEXT:
        case EntryPoint::GLProgramUniformMatrix3x2fv:
        case EntryPoint::GLProgramUniformMatrix3x2fvEXT:
        case EntryPoint::GLProgramUniformMatrix3x4fv:
        case EntryPoint::GLProgramUniformMatrix3x4fvEXT:
        case EntryPoint::GLProgramUniformMatrix4fv:
        case EntryPoint::GLProgramUniformMatrix4fvEXT:
        case EntryPoint::GLProgramUniformMatrix4x2fv:
        case EntryPoint::GLProgramUniformMatrix4x2fvEXT:
        case EntryPoint::GLProgramUniformMatrix4x3fv:
        case EntryPoint::GLProgramUniformMatrix4x3fvEXT:
            return DefaultUniformType::SpecifiedProgram;

        case EntryPoint::GLUniform1f:
        case EntryPoint::GLUniform1fv:
        case EntryPoint::GLUniform1i:
        case EntryPoint::GLUniform1iv:
        case EntryPoint::GLUniform1ui:
        case EntryPoint::GLUniform1uiv:
        case EntryPoint::GLUniform2f:
        case EntryPoint::GLUniform2fv:
        case EntryPoint::GLUniform2i:
        case EntryPoint::GLUniform2iv:
        case EntryPoint::GLUniform2ui:
        case EntryPoint::GLUniform2uiv:
        case EntryPoint::GLUniform3f:
        case EntryPoint::GLUniform3fv:
        case EntryPoint::GLUniform3i:
        case EntryPoint::GLUniform3iv:
        case EntryPoint::GLUniform3ui:
        case EntryPoint::GLUniform3uiv:
        case EntryPoint::GLUniform4f:
        case EntryPoint::GLUniform4fv:
        case EntryPoint::GLUniform4i:
        case EntryPoint::GLUniform4iv:
        case EntryPoint::GLUniform4ui:
        case EntryPoint::GLUniform4uiv:
        case EntryPoint::GLUniformMatrix2fv:
        case EntryPoint::GLUniformMatrix2x3fv:
        case EntryPoint::GLUniformMatrix2x4fv:
        case EntryPoint::GLUniformMatrix3fv:
        case EntryPoint::GLUniformMatrix3x2fv:
        case EntryPoint::GLUniformMatrix3x4fv:
        case EntryPoint::GLUniformMatrix4fv:
        case EntryPoint::GLUniformMatrix4x2fv:
        case EntryPoint::GLUniformMatrix4x3fv:
            return DefaultUniformType::CurrentProgram;

        default:
            return DefaultUniformType::None;
    }
}

void CaptureFramebufferAttachment(std::vector<CallCapture> *setupCalls,
                                  const gl::State &replayState,
                                  const FramebufferCaptureFuncs &framebufferFuncs,
                                  const gl::FramebufferAttachment &attachment,
                                  std::vector<CallCapture> *shareGroupSetupCalls,
                                  ResourceIDToSetupCallsMap *resourceIDToSetupCalls)
{
    GLuint resourceID = attachment.getResource()->getId();

    if (attachment.type() == GL_TEXTURE)
    {
        gl::ImageIndex index = attachment.getTextureImageIndex();

        if (index.usesTex3D())
        {
            Capture(setupCalls, CaptureFramebufferTextureLayer(
                                    replayState, true, GL_FRAMEBUFFER, attachment.getBinding(),
                                    {resourceID}, index.getLevelIndex(), index.getLayerIndex()));
        }
        else
        {
            Capture(setupCalls,
                    framebufferFuncs.framebufferTexture2D(
                        replayState, true, GL_FRAMEBUFFER, attachment.getBinding(),
                        index.getTargetOrFirstCubeFace(), {resourceID}, index.getLevelIndex()));
        }

        std::vector<gl::TextureID> textureIDs;
        const CallCapture &call = setupCalls->back();
        if (FindResourceIDsInCall<gl::TextureID>(call, textureIDs))
        {
            // We skip the is active check on the assumption this call is made during MEC
            for (gl::TextureID textureID : textureIDs)
            {
                // Track that this call referenced a Texture, setting it active for Setup
                MarkResourceIDActive(ResourceIDType::Texture, textureID.value, shareGroupSetupCalls,
                                     resourceIDToSetupCalls);
            }
        }
    }
    else
    {
        ASSERT(attachment.type() == GL_RENDERBUFFER);
        Capture(setupCalls, framebufferFuncs.framebufferRenderbuffer(
                                replayState, true, GL_FRAMEBUFFER, attachment.getBinding(),
                                GL_RENDERBUFFER, {resourceID}));
    }
}

void CaptureUpdateUniformValues(const gl::State &replayState,
                                const gl::Context *context,
                                gl::Program *program,
                                ResourceTracker *resourceTracker,
                                std::vector<CallCapture> *callsOut)
{
    if (!program->isLinked())
    {
        // We can't populate uniforms if the program hasn't been linked
        return;
    }

    // We need to bind the program and update its uniforms
    if (!replayState.getProgram() || replayState.getProgram()->id() != program->id())
    {
        Capture(callsOut, CaptureUseProgram(replayState, true, program->id()));
        CaptureUpdateCurrentProgram(callsOut->back(), 0, callsOut);
    }

    const gl::ProgramExecutable &executable = program->getExecutable();

    for (GLuint uniformIndex = 0;
         uniformIndex < static_cast<GLuint>(executable.getUniforms().size()); uniformIndex++)
    {
        std::string uniformName          = executable.getUniformNameByIndex(uniformIndex);
        const gl::LinkedUniform &uniform = executable.getUniformByIndex(uniformIndex);

        int uniformCount = 1;
        if (uniform.isArray())
        {
            uniformCount = uniform.getBasicTypeElementCount();
            uniformName  = gl::StripLastArrayIndex(uniformName);
        }

        gl::UniformLocation uniformLoc      = executable.getUniformLocation(uniformName);
        const gl::UniformTypeInfo &typeInfo = gl::GetUniformTypeInfo(uniform.getType());
        int componentCount                  = typeInfo.componentCount;
        int uniformSize                     = uniformCount * componentCount;

        // For arrayed uniforms, we'll need to increment a read location
        gl::UniformLocation readLoc = uniformLoc;

        // If the uniform is unused, just continue
        if (readLoc.value == -1)
        {
            continue;
        }

        // Image uniforms are special and cannot be set this way
        if (typeInfo.isImageType)
        {
            continue;
        }

        DefaultUniformCallsPerLocationMap &resetCalls =
            resourceTracker->getDefaultUniformResetCalls(program->id());

        // Create two lists of calls for uniforms, one for Setup, one for Reset
        CallVector defaultUniformCalls({callsOut, &resetCalls[uniformLoc]});

        // Samplers should be populated with GL_INT, regardless of return type
        if (typeInfo.isSampler)
        {
            std::vector<GLint> uniformBuffer(uniformSize);
            for (int index = 0; index < uniformCount; index++, readLoc.value++)
            {
                executable.getUniformiv(context, readLoc,
                                        uniformBuffer.data() + index * componentCount);
                resourceTracker->setDefaultUniformBaseLocation(program->id(), readLoc, uniformLoc);
            }

            for (std::vector<CallCapture> *calls : defaultUniformCalls)
            {
                Capture(calls, CaptureUniform1iv(replayState, true, uniformLoc, uniformCount,
                                                 uniformBuffer.data()));
            }

            continue;
        }

        switch (typeInfo.componentType)
        {
            case GL_FLOAT:
            {
                std::vector<GLfloat> uniformBuffer(uniformSize);
                for (int index = 0; index < uniformCount; index++, readLoc.value++)
                {
                    executable.getUniformfv(context, readLoc,
                                            uniformBuffer.data() + index * componentCount);
                    resourceTracker->setDefaultUniformBaseLocation(program->id(), readLoc,
                                                                   uniformLoc);
                }
                switch (typeInfo.type)
                {
                    // Note: All matrix uniforms are populated without transpose
                    case GL_FLOAT_MAT4x3:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniformMatrix4x3fv(replayState, true, uniformLoc,
                                                                     uniformCount, false,
                                                                     uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_MAT4x2:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniformMatrix4x2fv(replayState, true, uniformLoc,
                                                                     uniformCount, false,
                                                                     uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_MAT4:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniformMatrix4fv(replayState, true, uniformLoc,
                                                                   uniformCount, false,
                                                                   uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_MAT3x4:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniformMatrix3x4fv(replayState, true, uniformLoc,
                                                                     uniformCount, false,
                                                                     uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_MAT3x2:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniformMatrix3x2fv(replayState, true, uniformLoc,
                                                                     uniformCount, false,
                                                                     uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_MAT3:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniformMatrix3fv(replayState, true, uniformLoc,
                                                                   uniformCount, false,
                                                                   uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_MAT2x4:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniformMatrix2x4fv(replayState, true, uniformLoc,
                                                                     uniformCount, false,
                                                                     uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_MAT2x3:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniformMatrix2x3fv(replayState, true, uniformLoc,
                                                                     uniformCount, false,
                                                                     uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_MAT2:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniformMatrix2fv(replayState, true, uniformLoc,
                                                                   uniformCount, false,
                                                                   uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_VEC4:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform4fv(replayState, true, uniformLoc,
                                                             uniformCount, uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_VEC3:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform3fv(replayState, true, uniformLoc,
                                                             uniformCount, uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT_VEC2:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform2fv(replayState, true, uniformLoc,
                                                             uniformCount, uniformBuffer.data()));
                        }
                        break;
                    case GL_FLOAT:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform1fv(replayState, true, uniformLoc,
                                                             uniformCount, uniformBuffer.data()));
                        }
                        break;
                    default:
                        UNIMPLEMENTED();
                        break;
                }
                break;
            }
            case GL_INT:
            {
                std::vector<GLint> uniformBuffer(uniformSize);
                for (int index = 0; index < uniformCount; index++, readLoc.value++)
                {
                    executable.getUniformiv(context, readLoc,
                                            uniformBuffer.data() + index * componentCount);
                    resourceTracker->setDefaultUniformBaseLocation(program->id(), readLoc,
                                                                   uniformLoc);
                }
                switch (componentCount)
                {
                    case 4:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform4iv(replayState, true, uniformLoc,
                                                             uniformCount, uniformBuffer.data()));
                        }
                        break;
                    case 3:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform3iv(replayState, true, uniformLoc,
                                                             uniformCount, uniformBuffer.data()));
                        }
                        break;
                    case 2:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform2iv(replayState, true, uniformLoc,
                                                             uniformCount, uniformBuffer.data()));
                        }
                        break;
                    case 1:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform1iv(replayState, true, uniformLoc,
                                                             uniformCount, uniformBuffer.data()));
                        }
                        break;
                    default:
                        UNIMPLEMENTED();
                        break;
                }
                break;
            }
            case GL_BOOL:
            case GL_UNSIGNED_INT:
            {
                std::vector<GLuint> uniformBuffer(uniformSize);
                for (int index = 0; index < uniformCount; index++, readLoc.value++)
                {
                    executable.getUniformuiv(context, readLoc,
                                             uniformBuffer.data() + index * componentCount);
                    resourceTracker->setDefaultUniformBaseLocation(program->id(), readLoc,
                                                                   uniformLoc);
                }
                switch (componentCount)
                {
                    case 4:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform4uiv(replayState, true, uniformLoc,
                                                              uniformCount, uniformBuffer.data()));
                        }
                        break;
                    case 3:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform3uiv(replayState, true, uniformLoc,
                                                              uniformCount, uniformBuffer.data()));
                        }
                        break;
                    case 2:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform2uiv(replayState, true, uniformLoc,
                                                              uniformCount, uniformBuffer.data()));
                        }
                        break;
                    case 1:
                        for (std::vector<CallCapture> *calls : defaultUniformCalls)
                        {
                            Capture(calls, CaptureUniform1uiv(replayState, true, uniformLoc,
                                                              uniformCount, uniformBuffer.data()));
                        }
                        break;
                    default:
                        UNIMPLEMENTED();
                        break;
                }
                break;
            }
            default:
                UNIMPLEMENTED();
                break;
        }
    }
}

void CaptureVertexPointerES1(std::vector<CallCapture> *setupCalls,
                             gl::State *replayState,
                             GLuint attribIndex,
                             const gl::VertexAttribute &attrib,
                             const gl::VertexBinding &binding)
{
    switch (gl::GLES1Renderer::VertexArrayType(attribIndex))
    {
        case gl::ClientVertexArrayType::Vertex:
            Capture(setupCalls,
                    CaptureVertexPointer(*replayState, true, attrib.format->channelCount,
                                         attrib.format->vertexAttribType, binding.getStride(),
                                         attrib.pointer));
            break;
        case gl::ClientVertexArrayType::Normal:
            Capture(setupCalls,
                    CaptureNormalPointer(*replayState, true, attrib.format->vertexAttribType,
                                         binding.getStride(), attrib.pointer));
            break;
        case gl::ClientVertexArrayType::Color:
            Capture(setupCalls, CaptureColorPointer(*replayState, true, attrib.format->channelCount,
                                                    attrib.format->vertexAttribType,
                                                    binding.getStride(), attrib.pointer));
            break;
        case gl::ClientVertexArrayType::PointSize:
            Capture(setupCalls,
                    CapturePointSizePointerOES(*replayState, true, attrib.format->vertexAttribType,
                                               binding.getStride(), attrib.pointer));
            break;
        case gl::ClientVertexArrayType::TextureCoord:
            Capture(setupCalls,
                    CaptureTexCoordPointer(*replayState, true, attrib.format->channelCount,
                                           attrib.format->vertexAttribType, binding.getStride(),
                                           attrib.pointer));
            break;
        default:
            UNREACHABLE();
    }
}

void CaptureTextureEnvironmentState(std::vector<CallCapture> *setupCalls,
                                    gl::State *replayState,
                                    const gl::State *apiState,
                                    unsigned int unit)
{
    const gl::TextureEnvironmentParameters &currentEnv = apiState->gles1().textureEnvironment(unit);
    const gl::TextureEnvironmentParameters &defaultEnv =
        replayState->gles1().textureEnvironment(unit);

    if (currentEnv == defaultEnv)
    {
        return;
    }

    auto capIfNe = [setupCalls](auto currentState, auto defaultState, CallCapture &&call) {
        if (currentState != defaultState)
        {
            setupCalls->emplace_back(std::move(call));
        }
    };

    // When the texture env state differs on a non-default sampler unit, emit an ActiveTexture call.
    // The default sampler unit is GL_TEXTURE0.
    GLenum currentUnit = GL_TEXTURE0 + static_cast<GLenum>(unit);
    GLenum defaultUnit = GL_TEXTURE0 + static_cast<GLenum>(replayState->getActiveSampler());
    capIfNe(currentUnit, defaultUnit, CaptureActiveTexture(*replayState, true, currentUnit));

    auto capEnum = [capIfNe, replayState](gl::TextureEnvParameter pname, auto currentState,
                                          auto defaultState) {
        capIfNe(currentState, defaultState,
                CaptureTexEnvi(*replayState, true, gl::TextureEnvTarget::Env, pname,
                               ToGLenum(currentState)));
    };

    capEnum(gl::TextureEnvParameter::Mode, currentEnv.mode, defaultEnv.mode);

    capEnum(gl::TextureEnvParameter::CombineRgb, currentEnv.combineRgb, defaultEnv.combineRgb);
    capEnum(gl::TextureEnvParameter::CombineAlpha, currentEnv.combineAlpha,
            defaultEnv.combineAlpha);

    capEnum(gl::TextureEnvParameter::Src0Rgb, currentEnv.src0Rgb, defaultEnv.src0Rgb);
    capEnum(gl::TextureEnvParameter::Src1Rgb, currentEnv.src1Rgb, defaultEnv.src1Rgb);
    capEnum(gl::TextureEnvParameter::Src2Rgb, currentEnv.src2Rgb, defaultEnv.src2Rgb);

    capEnum(gl::TextureEnvParameter::Src0Alpha, currentEnv.src0Alpha, defaultEnv.src0Alpha);
    capEnum(gl::TextureEnvParameter::Src1Alpha, currentEnv.src1Alpha, defaultEnv.src1Alpha);
    capEnum(gl::TextureEnvParameter::Src2Alpha, currentEnv.src2Alpha, defaultEnv.src2Alpha);

    capEnum(gl::TextureEnvParameter::Op0Rgb, currentEnv.op0Rgb, defaultEnv.op0Rgb);
    capEnum(gl::TextureEnvParameter::Op1Rgb, currentEnv.op1Rgb, defaultEnv.op1Rgb);
    capEnum(gl::TextureEnvParameter::Op2Rgb, currentEnv.op2Rgb, defaultEnv.op2Rgb);

    capEnum(gl::TextureEnvParameter::Op0Alpha, currentEnv.op0Alpha, defaultEnv.op0Alpha);
    capEnum(gl::TextureEnvParameter::Op1Alpha, currentEnv.op1Alpha, defaultEnv.op1Alpha);
    capEnum(gl::TextureEnvParameter::Op2Alpha, currentEnv.op2Alpha, defaultEnv.op2Alpha);

    auto capFloat = [capIfNe, replayState](gl::TextureEnvParameter pname, auto currentState,
                                           auto defaultState) {
        capIfNe(currentState, defaultState,
                CaptureTexEnvf(*replayState, true, gl::TextureEnvTarget::Env, pname, currentState));
    };

    capFloat(gl::TextureEnvParameter::RgbScale, currentEnv.rgbScale, defaultEnv.rgbScale);
    capFloat(gl::TextureEnvParameter::AlphaScale, currentEnv.alphaScale, defaultEnv.alphaScale);

    capIfNe(currentEnv.color, defaultEnv.color,
            CaptureTexEnvfv(*replayState, true, gl::TextureEnvTarget::Env,
                            gl::TextureEnvParameter::Color, currentEnv.color.data()));

    // PointCoordReplace is the only parameter that uses the PointSprite TextureEnvTarget.
    capIfNe(currentEnv.pointSpriteCoordReplace, defaultEnv.pointSpriteCoordReplace,
            CaptureTexEnvi(*replayState, true, gl::TextureEnvTarget::PointSprite,
                           gl::TextureEnvParameter::PointCoordReplace,
                           currentEnv.pointSpriteCoordReplace));

    // In case of non-default sampler units, the default unit must be set back here.
    capIfNe(currentUnit, defaultUnit, CaptureActiveTexture(*replayState, true, defaultUnit));
}

bool VertexBindingMatchesAttribStride(const gl::VertexAttribute &attrib,
                                      const gl::VertexBinding &binding)
{
    if (attrib.vertexAttribArrayStride == 0 &&
        binding.getStride() == ComputeVertexAttributeTypeSize(attrib))
    {
        return true;
    }

    return attrib.vertexAttribArrayStride == binding.getStride();
}

void CaptureVertexArrayState(std::vector<CallCapture> *setupCalls,
                             const gl::Context *context,
                             const gl::VertexArray *vertexArray,
                             gl::State *replayState)
{
    const std::vector<gl::VertexAttribute> &vertexAttribs = vertexArray->getVertexAttributes();
    const std::vector<gl::VertexBinding> &vertexBindings  = vertexArray->getVertexBindings();

    gl::AttributesMask vertexPointerBindings;

    ASSERT(vertexAttribs.size() <= vertexBindings.size());
    for (GLuint attribIndex = 0; attribIndex < vertexAttribs.size(); ++attribIndex)
    {
        const gl::VertexAttribute defaultAttrib(attribIndex);
        const gl::VertexBinding defaultBinding;

        const gl::VertexAttribute &attrib = vertexAttribs[attribIndex];
        const gl::VertexBinding &binding  = vertexBindings[attrib.bindingIndex];

        if (attrib.enabled != defaultAttrib.enabled)
        {
            if (context->isGLES1())
            {
                Capture(setupCalls,
                        CaptureEnableClientState(*replayState, false,
                                                 gl::GLES1Renderer::VertexArrayType(attribIndex)));
            }
            else
            {
                Capture(setupCalls,
                        CaptureEnableVertexAttribArray(*replayState, false, attribIndex));
            }
        }

        // Don't capture CaptureVertexAttribPointer calls when a non-default VAO is bound, the array
        // buffer is null and a non-null attrib pointer is used.
        bool skipInvalidAttrib = vertexArray->id().value != 0 &&
                                 binding.getBuffer().get() == nullptr && attrib.pointer != nullptr;

        if (!skipInvalidAttrib &&
            (attrib.format != defaultAttrib.format || attrib.pointer != defaultAttrib.pointer ||
             binding.getStride() != defaultBinding.getStride() ||
             attrib.bindingIndex != defaultAttrib.bindingIndex ||
             binding.getBuffer().get() != nullptr))
        {
            // Each attribute can pull from a separate buffer, so check the binding
            gl::Buffer *buffer = binding.getBuffer().get();
            if (buffer != replayState->getArrayBuffer())
            {
                replayState->setBufferBinding(context, gl::BufferBinding::Array, buffer);

                gl::BufferID bufferID = {0};
                if (buffer)
                {
                    bufferID = buffer->id();
                }
                Capture(setupCalls,
                        CaptureBindBuffer(*replayState, true, gl::BufferBinding::Array, bufferID));
            }

            // Establish the relationship between currently bound buffer and the VAO
            if (context->isGLES1())
            {
                // Track indexes that used ES1 calls
                vertexPointerBindings.set(attribIndex);

                CaptureVertexPointerES1(setupCalls, replayState, attribIndex, attrib, binding);
            }
            else if (attrib.bindingIndex == attribIndex &&
                     VertexBindingMatchesAttribStride(attrib, binding) &&
                     (!buffer || binding.getOffset() == reinterpret_cast<GLintptr>(attrib.pointer)))
            {
                // Check if we can use strictly ES2 semantics, and track indexes that do.
                vertexPointerBindings.set(attribIndex);

                if (attrib.format->isPureInt())
                {
                    Capture(setupCalls, CaptureVertexAttribIPointer(*replayState, true, attribIndex,
                                                                    attrib.format->channelCount,
                                                                    attrib.format->vertexAttribType,
                                                                    attrib.vertexAttribArrayStride,
                                                                    attrib.pointer));
                }
                else
                {
                    Capture(setupCalls,
                            CaptureVertexAttribPointer(
                                *replayState, true, attribIndex, attrib.format->channelCount,
                                attrib.format->vertexAttribType, attrib.format->isNorm(),
                                attrib.vertexAttribArrayStride, attrib.pointer));
                }

                if (binding.getDivisor() != 0)
                {
                    Capture(setupCalls, CaptureVertexAttribDivisor(*replayState, true, attribIndex,
                                                                   binding.getDivisor()));
                }
            }
            else
            {
                ASSERT(context->getClientVersion() >= gl::ES_3_1);

                if (attrib.format->isPureInt())
                {
                    Capture(setupCalls, CaptureVertexAttribIFormat(*replayState, true, attribIndex,
                                                                   attrib.format->channelCount,
                                                                   attrib.format->vertexAttribType,
                                                                   attrib.relativeOffset));
                }
                else
                {
                    Capture(setupCalls, CaptureVertexAttribFormat(*replayState, true, attribIndex,
                                                                  attrib.format->channelCount,
                                                                  attrib.format->vertexAttribType,
                                                                  attrib.format->isNorm(),
                                                                  attrib.relativeOffset));
                }

                Capture(setupCalls, CaptureVertexAttribBinding(*replayState, true, attribIndex,
                                                               attrib.bindingIndex));
            }
        }
    }

    // The loop below expects attribs and bindings to have equal counts
    static_assert(gl::MAX_VERTEX_ATTRIBS == gl::MAX_VERTEX_ATTRIB_BINDINGS,
                  "Max vertex attribs and bindings count mismatch");

    // Loop through binding indices that weren't used by VertexAttribPointer
    for (size_t bindingIndex : vertexPointerBindings.flip())
    {
        const gl::VertexBinding &binding = vertexBindings[bindingIndex];

        if (binding.getBuffer().id().value != 0)
        {
            Capture(setupCalls,
                    CaptureBindVertexBuffer(*replayState, true, static_cast<GLuint>(bindingIndex),
                                            binding.getBuffer().id(), binding.getOffset(),
                                            binding.getStride()));
        }

        if (binding.getDivisor() != 0)
        {
            Capture(setupCalls, CaptureVertexBindingDivisor(*replayState, true,
                                                            static_cast<GLuint>(bindingIndex),
                                                            binding.getDivisor()));
        }
    }

    // The element array buffer is not per attribute, but per VAO
    gl::Buffer *elementArrayBuffer = vertexArray->getElementArrayBuffer();
    if (elementArrayBuffer)
    {
        Capture(setupCalls, CaptureBindBuffer(*replayState, true, gl::BufferBinding::ElementArray,
                                              elementArrayBuffer->id()));
    }
}

void CaptureTextureStorage(std::vector<CallCapture> *setupCalls,
                           gl::State *replayState,
                           const gl::Texture *texture)
{
    // Use mip-level 0 for the base dimensions
    gl::ImageIndex imageIndex = gl::ImageIndex::MakeFromType(texture->getType(), 0);
    const gl::ImageDesc &desc = texture->getTextureState().getImageDesc(imageIndex);

    switch (texture->getType())
    {
        case gl::TextureType::_2D:
        case gl::TextureType::CubeMap:
        {
            Capture(setupCalls, CaptureTexStorage2D(*replayState, true, texture->getType(),
                                                    texture->getImmutableLevels(),
                                                    desc.format.info->internalFormat,
                                                    desc.size.width, desc.size.height));
            break;
        }
        case gl::TextureType::_3D:
        case gl::TextureType::_2DArray:
        case gl::TextureType::CubeMapArray:
        {
            Capture(setupCalls, CaptureTexStorage3D(
                                    *replayState, true, texture->getType(),
                                    texture->getImmutableLevels(), desc.format.info->internalFormat,
                                    desc.size.width, desc.size.height, desc.size.depth));
            break;
        }
        case gl::TextureType::Buffer:
        {
            // Do nothing. This will already be captured as a buffer.
            break;
        }
        default:
            UNIMPLEMENTED();
            break;
    }
}

void CaptureTextureContents(std::vector<CallCapture> *setupCalls,
                            gl::State *replayState,
                            const gl::Texture *texture,
                            const gl::ImageIndex &index,
                            const gl::ImageDesc &desc,
                            GLuint size,
                            const void *data)
{
    const gl::InternalFormat &format = *desc.format.info;

    if (index.getType() == gl::TextureType::Buffer)
    {
        // Zero binding size indicates full buffer bound
        if (texture->getBuffer().getSize() == 0)
        {
            Capture(setupCalls,
                    CaptureTexBufferEXT(*replayState, true, index.getType(), format.internalFormat,
                                        texture->getBuffer().get()->id()));
        }
        else
        {
            Capture(setupCalls, CaptureTexBufferRangeEXT(*replayState, true, index.getType(),
                                                         format.internalFormat,
                                                         texture->getBuffer().get()->id(),
                                                         texture->getBuffer().getOffset(),
                                                         texture->getBuffer().getSize()));
        }

        // For buffers, we're done
        return;
    }

    bool is3D =
        (index.getType() == gl::TextureType::_3D || index.getType() == gl::TextureType::_2DArray ||
         index.getType() == gl::TextureType::CubeMapArray);

    if (format.compressed || format.paletted)
    {
        if (is3D)
        {
            if (texture->getImmutableFormat())
            {
                Capture(setupCalls,
                        CaptureCompressedTexSubImage3D(
                            *replayState, true, index.getTarget(), index.getLevelIndex(), 0, 0, 0,
                            desc.size.width, desc.size.height, desc.size.depth,
                            format.internalFormat, size, data));
            }
            else
            {
                Capture(setupCalls,
                        CaptureCompressedTexImage3D(*replayState, true, index.getTarget(),
                                                    index.getLevelIndex(), format.internalFormat,
                                                    desc.size.width, desc.size.height,
                                                    desc.size.depth, 0, size, data));
            }
        }
        else
        {
            if (texture->getImmutableFormat())
            {
                Capture(setupCalls,
                        CaptureCompressedTexSubImage2D(
                            *replayState, true, index.getTarget(), index.getLevelIndex(), 0, 0,
                            desc.size.width, desc.size.height, format.internalFormat, size, data));
            }
            else
            {
                Capture(setupCalls, CaptureCompressedTexImage2D(
                                        *replayState, true, index.getTarget(),
                                        index.getLevelIndex(), format.internalFormat,
                                        desc.size.width, desc.size.height, 0, size, data));
            }
        }
    }
    else
    {
        if (is3D)
        {
            if (texture->getImmutableFormat())
            {
                Capture(setupCalls,
                        CaptureTexSubImage3D(*replayState, true, index.getTarget(),
                                             index.getLevelIndex(), 0, 0, 0, desc.size.width,
                                             desc.size.height, desc.size.depth, format.format,
                                             format.type, data));
            }
            else
            {
                Capture(
                    setupCalls,
                    CaptureTexImage3D(*replayState, true, index.getTarget(), index.getLevelIndex(),
                                      format.internalFormat, desc.size.width, desc.size.height,
                                      desc.size.depth, 0, format.format, format.type, data));
            }
        }
        else
        {
            if (texture->getImmutableFormat())
            {
                Capture(setupCalls,
                        CaptureTexSubImage2D(*replayState, true, index.getTarget(),
                                             index.getLevelIndex(), 0, 0, desc.size.width,
                                             desc.size.height, format.format, format.type, data));
            }
            else
            {
                Capture(setupCalls, CaptureTexImage2D(*replayState, true, index.getTarget(),
                                                      index.getLevelIndex(), format.internalFormat,
                                                      desc.size.width, desc.size.height, 0,
                                                      format.format, format.type, data));
            }
        }
    }
}

void CaptureCustomUniformBlockBinding(const CallCapture &callIn, std::vector<CallCapture> &callsOut)
{
    const ParamBuffer &paramsIn = callIn.params;

    const ParamCapture &programID =
        paramsIn.getParam("programPacked", ParamType::TShaderProgramID, 0);
    const ParamCapture &blockIndex =
        paramsIn.getParam("uniformBlockIndexPacked", ParamType::TUniformBlockIndex, 1);
    const ParamCapture &blockBinding =
        paramsIn.getParam("uniformBlockBinding", ParamType::TGLuint, 2);

    ParamBuffer params;
    params.addValueParam("program", ParamType::TGLuint, programID.value.ShaderProgramIDVal.value);
    params.addValueParam("uniformBlockIndex", ParamType::TGLuint,
                         blockIndex.value.UniformBlockIndexVal.value);
    params.addValueParam("uniformBlockBinding", ParamType::TGLuint, blockBinding.value.GLuintVal);

    callsOut.emplace_back("UniformBlockBinding", std::move(params));
}

void CaptureCustomMapBuffer(const char *entryPointName,
                            CallCapture &call,
                            std::vector<CallCapture> &callsOut,
                            gl::BufferID mappedBufferID)
{
    call.params.addValueParam("buffer", ParamType::TGLuint, mappedBufferID.value);
    callsOut.emplace_back(entryPointName, std::move(call.params));
}

void CaptureCustomShaderProgram(const char *name,
                                CallCapture &call,
                                std::vector<CallCapture> &callsOut)
{
    call.params.addValueParam("shaderProgram", ParamType::TGLuint,
                              call.params.getReturnValue().value.GLuintVal);
    call.customFunctionName = name;
    callsOut.emplace_back(std::move(call));
}

void CaptureCustomFenceSync(CallCapture &call, std::vector<CallCapture> &callsOut)
{
    ParamBuffer &&params = std::move(call.params);
    params.addValueParam("fenceSync", ParamType::TGLuint64,
                         params.getReturnValue().value.GLuint64Val);
    call.customFunctionName = "FenceSync2";
    call.isSyncPoint        = true;
    callsOut.emplace_back(std::move(call));
}

const egl::Image *GetImageFromParam(const gl::Context *context, const ParamCapture &param)
{
    const egl::ImageID eglImageID = egl::PackParam<egl::ImageID>(param.value.EGLImageVal);
    const egl::Image *eglImage    = context->getDisplay()->getImage(eglImageID);
    ASSERT(eglImage != nullptr);
    return eglImage;
}

void CaptureCustomCreateEGLImage(const gl::Context *context,
                                 const char *name,
                                 size_t width,
                                 size_t height,
                                 CallCapture &call,
                                 std::vector<CallCapture> &callsOut)
{
    ParamBuffer &&params    = std::move(call.params);
    EGLImage returnVal      = params.getReturnValue().value.EGLImageVal;
    egl::ImageID imageID    = egl::PackParam<egl::ImageID>(returnVal);
    call.customFunctionName = name;

    // Clear client buffer value if it is a pointer to a hardware buffer. It is
    // not used by replay and will not be portable to 32-bit builds
    if (params.getParam("target", ParamType::TEGLenum, 2).value.EGLenumVal ==
        EGL_NATIVE_BUFFER_ANDROID)
    {
        params.setValueParamAtIndex("buffer", ParamType::TEGLClientBuffer,
                                    reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(0)),
                                    3);
    }

    // Record image dimensions in case a backing resource needs to be created during replay
    params.addValueParam("width", ParamType::TGLsizei, static_cast<GLsizei>(width));
    params.addValueParam("height", ParamType::TGLsizei, static_cast<GLsizei>(height));

    params.addValueParam("image", ParamType::TGLuint, imageID.value);
    callsOut.emplace_back(std::move(call));
}

void CaptureCustomDestroyEGLImage(const char *name,
                                  CallCapture &call,
                                  std::vector<CallCapture> &callsOut)
{
    call.customFunctionName = name;
    ParamBuffer &&params    = std::move(call.params);

    const ParamCapture &imageID = params.getParam("imagePacked", ParamType::TImageID, 1);
    params.addValueParam("imageID", ParamType::TGLuint, imageID.value.ImageIDVal.value);

    callsOut.emplace_back(std::move(call));
}

void CaptureCustomCreateEGLSync(const char *name,
                                CallCapture &call,
                                std::vector<CallCapture> &callsOut)
{
    ParamBuffer &&params = std::move(call.params);
    EGLSync returnVal    = params.getReturnValue().value.EGLSyncVal;
    egl::SyncID syncID   = egl::PackParam<egl::SyncID>(returnVal);
    params.addValueParam("sync", ParamType::TGLuint, syncID.value);
    call.customFunctionName = name;
    callsOut.emplace_back(std::move(call));
}

void CaptureCustomCreatePbufferSurface(CallCapture &call, std::vector<CallCapture> &callsOut)
{
    ParamBuffer &&params     = std::move(call.params);
    EGLSurface returnVal     = params.getReturnValue().value.EGLSurfaceVal;
    egl::SurfaceID surfaceID = egl::PackParam<egl::SurfaceID>(returnVal);

    params.addValueParam("surface", ParamType::TGLuint, surfaceID.value);
    call.customFunctionName = "CreatePbufferSurface";
    callsOut.emplace_back(std::move(call));
}

void CaptureCustomCreateNativeClientbuffer(CallCapture &call, std::vector<CallCapture> &callsOut)
{
    ParamBuffer &&params = std::move(call.params);
    params.addValueParam("clientBuffer", ParamType::TEGLClientBuffer,
                         params.getReturnValue().value.EGLClientBufferVal);
    call.customFunctionName = "CreateNativeClientBufferANDROID";
    callsOut.emplace_back(std::move(call));
}

void GenerateLinkedProgram(const gl::Context *context,
                           const gl::State &replayState,
                           ResourceTracker *resourceTracker,
                           std::vector<CallCapture> *setupCalls,
                           gl::Program *program,
                           gl::ShaderProgramID id,
                           gl::ShaderProgramID tempIDStart,
                           const ProgramSources &linkedSources)
{
    // A map to store the gShaderProgram map lookup index of the temp shaders we attached below. We
    // need this map to retrieve the lookup index to pass to CaptureDetachShader calls at the end of
    // GenerateLinkedProgram.
    PackedEnumMap<gl::ShaderType, gl::ShaderProgramID> tempShaderIDTracker;

    const gl::ProgramExecutable &executable = program->getExecutable();

    // Compile with last linked sources.
    for (gl::ShaderType shaderType : executable.getLinkedShaderStages())
    {
        // Bump the max shader program id for each new tempIDStart we use to create, compile, and
        // attach the temp shader object.
        resourceTracker->onShaderProgramAccess(tempIDStart);
        // Store the tempIDStart in the tempShaderIDTracker to retrieve for CaptureDetachShader
        // calls later.
        tempShaderIDTracker[shaderType] = tempIDStart;
        const std::string &sourceString = linkedSources[shaderType];
        const char *sourcePointer       = sourceString.c_str();

        if (sourceString.empty())
        {
            // If we don't have source for this shader, that means it was populated by the app
            // using glProgramBinary.  We need to look it up from our cached copy.
            const ProgramSources &cachedLinkedSources =
                context->getShareGroup()->getFrameCaptureShared()->getProgramSources(id);

            const std::string &cachedSourceString = cachedLinkedSources[shaderType];
            sourcePointer                         = cachedSourceString.c_str();
            ASSERT(!cachedSourceString.empty());
        }

        // Compile and attach the temporary shader. Then free it immediately.
        CallCapture createShader =
            CaptureCreateShader(replayState, true, shaderType, tempIDStart.value);
        CaptureCustomShaderProgram("CreateShader", createShader, *setupCalls);
        Capture(setupCalls,
                CaptureShaderSource(replayState, true, tempIDStart, 1, &sourcePointer, nullptr));
        Capture(setupCalls, CaptureCompileShader(replayState, true, tempIDStart));
        Capture(setupCalls, CaptureAttachShader(replayState, true, id, tempIDStart));
        // Increment tempIDStart to get a new gShaderProgram map index for the next linked stage
        // shader object. We can't reuse the same tempIDStart as we need to retrieve the index of
        // each attached shader object later to pass to CaptureDetachShader calls.
        tempIDStart.value += 1;
    }

    // Gather XFB varyings
    std::vector<std::string> xfbVaryings;
    for (const gl::TransformFeedbackVarying &xfbVarying :
         executable.getLinkedTransformFeedbackVaryings())
    {
        xfbVaryings.push_back(xfbVarying.nameWithArrayIndex());
    }

    if (!xfbVaryings.empty())
    {
        std::vector<const char *> varyingsStrings;
        for (const std::string &varyingString : xfbVaryings)
        {
            varyingsStrings.push_back(varyingString.data());
        }

        GLenum xfbMode = executable.getTransformFeedbackBufferMode();
        Capture(setupCalls, CaptureTransformFeedbackVaryings(replayState, true, id,
                                                             static_cast<GLint>(xfbVaryings.size()),
                                                             varyingsStrings.data(), xfbMode));
    }

    // Force the attributes to be bound the same way as in the existing program.
    // This can affect attributes that are optimized out in some implementations.
    for (const gl::ProgramInput &attrib : executable.getProgramInputs())
    {
        if (gl::IsBuiltInName(attrib.name))
        {
            // Don't try to bind built-in attributes
            continue;
        }

        // Separable programs may not have a VS, meaning it may not have attributes.
        if (executable.hasLinkedShaderStage(gl::ShaderType::Vertex))
        {
            ASSERT(attrib.getLocation() != -1);
            Capture(setupCalls, CaptureBindAttribLocation(replayState, true, id,
                                                          static_cast<GLuint>(attrib.getLocation()),
                                                          attrib.name.c_str()));
        }
    }

    if (program->isSeparable())
    {
        // MEC manually recreates separable programs, rather than attempting to recreate a call
        // to glCreateShaderProgramv(), so insert a call to mark it separable.
        Capture(setupCalls,
                CaptureProgramParameteri(replayState, true, id, GL_PROGRAM_SEPARABLE, GL_TRUE));
    }

    Capture(setupCalls, CaptureLinkProgram(replayState, true, id));
    CaptureUpdateUniformLocations(program, setupCalls);
    CaptureUpdateUniformValues(replayState, context, program, resourceTracker, setupCalls);
    CaptureUpdateUniformBlockIndexes(program, setupCalls);

    // Capture uniform block bindings for each program
    for (uint32_t uniformBlockIndex = 0;
         uniformBlockIndex < static_cast<uint32_t>(executable.getUniformBlocks().size());
         uniformBlockIndex++)
    {
        GLuint blockBinding = executable.getUniformBlocks()[uniformBlockIndex].pod.inShaderBinding;
        CallCapture updateCallCapture =
            CaptureUniformBlockBinding(replayState, true, id, {uniformBlockIndex}, blockBinding);
        CaptureCustomUniformBlockBinding(updateCallCapture, *setupCalls);
    }

    // Add DetachShader call if that's what the app does, so that the
    // ResourceManagerBase::mHandleAllocator can release the ShaderProgramID handle assigned to the
    // shader object when glDeleteShader is called. This ensures the ShaderProgramID handles used in
    // SetupReplayContextShared() are consistent with the ShaderProgramID handles used by the app.
    for (gl::ShaderType shaderType : executable.getLinkedShaderStages())
    {
        gl::Shader *attachedShader = program->getAttachedShader(shaderType);
        if (attachedShader == nullptr)
        {
            Capture(setupCalls,
                    CaptureDetachShader(replayState, true, id, tempShaderIDTracker[shaderType]));
        }
        Capture(setupCalls,
                CaptureDeleteShader(replayState, true, tempShaderIDTracker[shaderType]));
    }
}

// TODO(http://anglebug.com/42263204): Improve reset/restore call generation
// There are multiple ways to track reset calls for individual resources. For now, we are tracking
// separate lists of instructions that mirror the calls created during mid-execution setup. Other
// methods could involve passing the original CallCaptures to this function, or tracking the
// indices of original setup calls.
void CaptureBufferResetCalls(const gl::Context *context,
                             const gl::State &replayState,
                             ResourceTracker *resourceTracker,
                             gl::BufferID *id,
                             const gl::Buffer *buffer)
{
    GLuint bufferID = (*id).value;

    // Track this as a starting resource that may need to be restored.
    TrackedResource &trackedBuffers =
        resourceTracker->getTrackedResource(context->id(), ResourceIDType::Buffer);

    // Track calls to regenerate a given buffer
    ResourceCalls &bufferRegenCalls = trackedBuffers.getResourceRegenCalls();
    Capture(&bufferRegenCalls[bufferID], CaptureGenBuffers(replayState, true, 1, id));
    MaybeCaptureUpdateResourceIDs(context, resourceTracker, &bufferRegenCalls[bufferID]);

    // Call glBufferStorageEXT when regenerating immutable buffers,
    // as we can't call glBufferData on restore.
    if (buffer->isImmutable())
    {
        Capture(&bufferRegenCalls[bufferID],
                CaptureBindBuffer(replayState, true, gl::BufferBinding::Array, *id));
        Capture(
            &bufferRegenCalls[bufferID],
            CaptureBufferStorageEXT(replayState, true, gl::BufferBinding::Array,
                                    static_cast<GLsizeiptr>(buffer->getSize()),
                                    buffer->getMapPointer(), buffer->getStorageExtUsageFlags()));
    }

    // Track calls to restore a given buffer's contents
    ResourceCalls &bufferRestoreCalls = trackedBuffers.getResourceRestoreCalls();
    Capture(&bufferRestoreCalls[bufferID],
            CaptureBindBuffer(replayState, true, gl::BufferBinding::Array, *id));

    // Mutable buffers will be restored here using glBufferData.
    // Immutable buffers need to be restored below, after maping.
    if (!buffer->isImmutable())
    {
        Capture(&bufferRestoreCalls[bufferID],
                CaptureBufferData(replayState, true, gl::BufferBinding::Array,
                                  static_cast<GLsizeiptr>(buffer->getSize()),
                                  buffer->getMapPointer(), buffer->getUsage()));
    }

    if (buffer->isMapped())
    {
        // Track calls to remap a buffer that started as mapped
        BufferCalls &bufferMapCalls = resourceTracker->getBufferMapCalls();

        Capture(&bufferMapCalls[bufferID],
                CaptureBindBuffer(replayState, true, gl::BufferBinding::Array, *id));

        void *dontCare             = nullptr;
        CallCapture mapBufferRange = CaptureMapBufferRange(
            replayState, true, gl::BufferBinding::Array,
            static_cast<GLsizeiptr>(buffer->getMapOffset()),
            static_cast<GLsizeiptr>(buffer->getMapLength()), buffer->getAccessFlags(), dontCare);
        CaptureCustomMapBuffer("MapBufferRange", mapBufferRange, bufferMapCalls[bufferID],
                               buffer->id());

        // Restore immutable mapped buffers. Needs to happen after mapping.
        if (buffer->isImmutable())
        {
            ParamBuffer dataParamBuffer;
            dataParamBuffer.addValueParam("dest", ParamType::TGLuint, buffer->id().value);
            ParamCapture captureData("source", ParamType::TvoidConstPointer);
            CaptureMemory(buffer->getMapPointer(), static_cast<GLsizeiptr>(buffer->getSize()),
                          &captureData);
            dataParamBuffer.addParam(std::move(captureData));
            dataParamBuffer.addValueParam<GLsizeiptr>("size", ParamType::TGLsizeiptr,
                                                      static_cast<GLsizeiptr>(buffer->getSize()));
            bufferMapCalls[bufferID].emplace_back("UpdateClientBufferData",
                                                  std::move(dataParamBuffer));
        }
    }

    // Track calls unmap a buffer that started as unmapped
    BufferCalls &bufferUnmapCalls = resourceTracker->getBufferUnmapCalls();
    Capture(&bufferUnmapCalls[bufferID],
            CaptureBindBuffer(replayState, true, gl::BufferBinding::Array, *id));
    Capture(&bufferUnmapCalls[bufferID],
            CaptureUnmapBuffer(replayState, true, gl::BufferBinding::Array, GL_TRUE));
}

void CaptureFenceSyncResetCalls(const gl::Context *context,
                                const gl::State &replayState,
                                ResourceTracker *resourceTracker,
                                gl::SyncID syncID,
                                GLsync syncObject,
                                const gl::Sync *sync)
{
    // Track calls to regenerate a given fence sync
    FenceSyncCalls &fenceSyncRegenCalls = resourceTracker->getFenceSyncRegenCalls();
    CallCapture fenceSync =
        CaptureFenceSync(replayState, true, sync->getCondition(), sync->getFlags(), syncObject);
    CaptureCustomFenceSync(fenceSync, fenceSyncRegenCalls[syncID]);
    MaybeCaptureUpdateResourceIDs(context, resourceTracker, &fenceSyncRegenCalls[syncID]);
}

void CaptureEGLSyncResetCalls(const gl::Context *context,
                              const gl::State &replayState,
                              ResourceTracker *resourceTracker,
                              egl::SyncID eglSyncID,
                              EGLSync eglSyncObject,
                              const egl::Sync *eglSync)
{
    // Track this as a starting resource that may need to be restored.
    TrackedResource &trackedEGLSyncs =
        resourceTracker->getTrackedResource(context->id(), ResourceIDType::egl_Sync);

    // Track calls to regenerate a given buffer
    ResourceCalls &eglSyncRegenCalls = trackedEGLSyncs.getResourceRegenCalls();

    CallCapture createEGLSync =
        CaptureCreateSyncKHR(nullptr, true, context->getDisplay(), eglSync->getType(),
                             eglSync->getAttributeMap(), eglSyncObject);
    CaptureCustomCreateEGLSync("CreateEGLSyncKHR", createEGLSync,
                               eglSyncRegenCalls[eglSyncID.value]);
    MaybeCaptureUpdateResourceIDs(context, resourceTracker, &eglSyncRegenCalls[eglSyncID.value]);
}

void CaptureBufferBindingResetCalls(const gl::State &replayState,
                                    ResourceTracker *resourceTracker,
                                    gl::BufferBinding binding,
                                    gl::BufferID id)
{
    // Track the calls to reset it
    std::vector<CallCapture> &bufferBindingCalls = resourceTracker->getBufferBindingCalls();
    Capture(&bufferBindingCalls, CaptureBindBuffer(replayState, true, binding, id));
}

void CaptureIndexedBuffers(const gl::State &glState,
                           const gl::BufferVector &indexedBuffers,
                           gl::BufferBinding binding,
                           std::vector<CallCapture> *setupCalls)
{
    for (unsigned int index = 0; index < indexedBuffers.size(); ++index)
    {
        const gl::OffsetBindingPointer<gl::Buffer> &buffer = indexedBuffers[index];

        if (buffer.get() == nullptr)
        {
            continue;
        }

        GLintptr offset       = buffer.getOffset();
        GLsizeiptr size       = buffer.getSize();
        gl::BufferID bufferID = buffer.get()->id();

        // Context::bindBufferBase() calls Context::bindBufferRange() with size and offset = 0.
        if ((offset == 0) && (size == 0))
        {
            Capture(setupCalls, CaptureBindBufferBase(glState, true, binding, index, bufferID));
        }
        else
        {
            Capture(setupCalls,
                    CaptureBindBufferRange(glState, true, binding, index, bufferID, offset, size));
        }
    }
}

void CaptureDefaultVertexAttribs(const gl::State &replayState,
                                 const gl::State &apiState,
                                 std::vector<CallCapture> *setupCalls)
{
    const std::vector<gl::VertexAttribCurrentValueData> &currentValues =
        apiState.getVertexAttribCurrentValues();

    for (GLuint attribIndex = 0; attribIndex < currentValues.size(); ++attribIndex)
    {
        const gl::VertexAttribCurrentValueData &defaultValue = currentValues[attribIndex];
        if (!IsDefaultCurrentValue(defaultValue))
        {
            Capture(setupCalls, CaptureVertexAttrib4fv(replayState, true, attribIndex,
                                                       defaultValue.Values.FloatValues));
        }
    }
}

void CompressPalettedTexture(angle::MemoryBuffer &data,
                             angle::MemoryBuffer &tmp,
                             const gl::InternalFormat &compressedFormat,
                             const gl::Extents &extents)
{
    constexpr int uncompressedChannelCount = 4;

    uint32_t indexBits = 0, redBlueBits = 0, greenBits = 0, alphaBits = 0;
    switch (compressedFormat.internalFormat)
    {
        case GL_PALETTE4_RGB8_OES:
            indexBits   = 4;
            redBlueBits = 8;
            greenBits   = 8;
            alphaBits   = 0;
            break;
        case GL_PALETTE4_RGBA8_OES:
            indexBits   = 4;
            redBlueBits = 8;
            greenBits   = 8;
            alphaBits   = 8;
            break;
        case GL_PALETTE4_R5_G6_B5_OES:
            indexBits   = 4;
            redBlueBits = 5;
            greenBits   = 6;
            alphaBits   = 0;
            break;
        case GL_PALETTE4_RGBA4_OES:
            indexBits   = 4;
            redBlueBits = 4;
            greenBits   = 4;
            alphaBits   = 4;
            break;
        case GL_PALETTE4_RGB5_A1_OES:
            indexBits   = 4;
            redBlueBits = 5;
            greenBits   = 5;
            alphaBits   = 1;
            break;
        case GL_PALETTE8_RGB8_OES:
            indexBits   = 8;
            redBlueBits = 8;
            greenBits   = 8;
            alphaBits   = 0;
            break;
        case GL_PALETTE8_RGBA8_OES:
            indexBits   = 8;
            redBlueBits = 8;
            greenBits   = 8;
            alphaBits   = 8;
            break;
        case GL_PALETTE8_R5_G6_B5_OES:
            indexBits   = 8;
            redBlueBits = 5;
            greenBits   = 6;
            alphaBits   = 0;
            break;
        case GL_PALETTE8_RGBA4_OES:
            indexBits   = 8;
            redBlueBits = 4;
            greenBits   = 4;
            alphaBits   = 4;
            break;
        case GL_PALETTE8_RGB5_A1_OES:
            indexBits   = 8;
            redBlueBits = 5;
            greenBits   = 5;
            alphaBits   = 1;
            break;

        default:
            UNREACHABLE();
            break;
    }

    bool result = data.resize(
        // Palette size
        (1 << indexBits) * (2 * redBlueBits + greenBits + alphaBits) / 8 +
        // Texels size
        indexBits * extents.width * extents.height * extents.depth / 8);
    ASSERT(result);

    angle::StoreRGBA8ToPalettedImpl(
        extents.width, extents.height, extents.depth, indexBits, redBlueBits, greenBits, alphaBits,
        tmp.data(),
        uncompressedChannelCount * extents.width,                   // inputRowPitch
        uncompressedChannelCount * extents.width * extents.height,  // inputDepthPitch
        data.data(),                                                // output
        indexBits * extents.width / 8,                              // outputRowPitch
        indexBits * extents.width * extents.height / 8              // outputDepthPitch
    );
}

// Capture the setup of the state that's shared by all of the contexts in the share group
// See IsSharedObjectResource for the list of objects covered here.
void CaptureShareGroupMidExecutionSetup(
    gl::Context *context,
    std::vector<CallCapture> *setupCalls,
    ResourceTracker *resourceTracker,
    gl::State &replayState,
    const PackedEnumMap<ResourceIDType, uint32_t> &maxAccessedResourceIDs)
{
    FrameCaptureShared *frameCaptureShared = context->getShareGroup()->getFrameCaptureShared();
    const gl::State &apiState              = context->getState();

    // Small helper function to make the code more readable.
    auto cap = [setupCalls](CallCapture &&call) { setupCalls->emplace_back(std::move(call)); };

    // Capture Buffer data.
    const gl::BufferManager &buffers = apiState.getBufferManagerForCapture();
    for (const auto &bufferIter : gl::UnsafeResourceMapIter(buffers.getResourcesForCapture()))
    {
        gl::BufferID id    = {bufferIter.first};
        gl::Buffer *buffer = bufferIter.second;

        if (id.value == 0)
        {
            continue;
        }

        // Generate binding.
        cap(CaptureGenBuffers(replayState, true, 1, &id));

        resourceTracker->getTrackedResource(context->id(), ResourceIDType::Buffer)
            .getStartingResources()
            .insert(id.value);

        MaybeCaptureUpdateResourceIDs(context, resourceTracker, setupCalls);

        // glBufferData. Would possibly be better implemented using a getData impl method.
        // Saving buffers that are mapped during a swap is not yet handled.
        if (buffer->getSize() == 0)
        {
            resourceTracker->setStartingBufferMapped(buffer->id().value, false);
            continue;
        }

        // Remember if the buffer was already mapped
        GLboolean bufferMapped = buffer->isMapped();

        // If needed, map the buffer so we can capture its contents
        if (!bufferMapped)
        {
            (void)buffer->mapRange(context, 0, static_cast<GLsizeiptr>(buffer->getSize()),
                                   GL_MAP_READ_BIT);
        }

        // Always use the array buffer binding point to upload data to keep things simple.
        if (buffer != replayState.getArrayBuffer())
        {
            replayState.setBufferBinding(context, gl::BufferBinding::Array, buffer);
            cap(CaptureBindBuffer(replayState, true, gl::BufferBinding::Array, id));
        }

        if (buffer->isImmutable())
        {
            cap(CaptureBufferStorageEXT(replayState, true, gl::BufferBinding::Array,
                                        static_cast<GLsizeiptr>(buffer->getSize()),
                                        buffer->getMapPointer(),
                                        buffer->getStorageExtUsageFlags()));
        }
        else
        {
            cap(CaptureBufferData(replayState, true, gl::BufferBinding::Array,
                                  static_cast<GLsizeiptr>(buffer->getSize()),
                                  buffer->getMapPointer(), buffer->getUsage()));
        }

        if (bufferMapped)
        {
            void *dontCare = nullptr;
            CallCapture mapBufferRange =
                CaptureMapBufferRange(replayState, true, gl::BufferBinding::Array,
                                      static_cast<GLsizeiptr>(buffer->getMapOffset()),
                                      static_cast<GLsizeiptr>(buffer->getMapLength()),
                                      buffer->getAccessFlags(), dontCare);
            CaptureCustomMapBuffer("MapBufferRange", mapBufferRange, *setupCalls, buffer->id());

            resourceTracker->setStartingBufferMapped(buffer->id().value, true);

            frameCaptureShared->trackBufferMapping(
                context, &setupCalls->back(), buffer->id(), buffer,
                static_cast<GLsizeiptr>(buffer->getMapOffset()),
                static_cast<GLsizeiptr>(buffer->getMapLength()),
                (buffer->getAccessFlags() & GL_MAP_WRITE_BIT) != 0,
                (buffer->getStorageExtUsageFlags() & GL_MAP_COHERENT_BIT_EXT) != 0);
        }
        else
        {
            resourceTracker->setStartingBufferMapped(buffer->id().value, false);
        }

        // Generate the calls needed to restore this buffer to original state for frame looping
        CaptureBufferResetCalls(context, replayState, resourceTracker, &id, buffer);

        // Unmap the buffer if it wasn't already mapped
        if (!bufferMapped)
        {
            GLboolean dontCare;
            (void)buffer->unmap(context, &dontCare);
        }
    }

    // Clear the array buffer binding.
    if (replayState.getTargetBuffer(gl::BufferBinding::Array))
    {
        cap(CaptureBindBuffer(replayState, true, gl::BufferBinding::Array, {0}));
        replayState.setBufferBinding(context, gl::BufferBinding::Array, nullptr);
    }

    // Set a unpack alignment of 1. Otherwise, computeRowPitch() will compute the wrong value,
    // leading to a crash in memcpy() when capturing the texture contents.
    gl::PixelUnpackState &currentUnpackState = replayState.getUnpackState();
    if (currentUnpackState.alignment != 1)
    {
        cap(CapturePixelStorei(replayState, true, GL_UNPACK_ALIGNMENT, 1));
        replayState.getMutablePrivateStateForCapture()->setUnpackAlignment(1);
    }

    const egl::ImageMap eglImageMap = context->getDisplay()->getImagesForCapture();
    for (const auto &[eglImageID, eglImage] : eglImageMap)
    {
        // Track this as a starting resource that may need to be restored.
        TrackedResource &trackedImages =
            resourceTracker->getTrackedResource(context->id(), ResourceIDType::Image);
        trackedImages.getStartingResources().insert(eglImageID);

        ResourceCalls &imageRegenCalls = trackedImages.getResourceRegenCalls();
        CallVector imageGenCalls({setupCalls, &imageRegenCalls[eglImageID]});

        auto eglImageAttribIter = resourceTracker->getImageToAttribTable().find(
            reinterpret_cast<EGLImage>(static_cast<uintptr_t>(eglImageID)));
        ASSERT(eglImageAttribIter != resourceTracker->getImageToAttribTable().end());
        const egl::AttributeMap &attribs = eglImageAttribIter->second;

        for (std::vector<CallCapture> *calls : imageGenCalls)
        {
            // Create the image on demand with the same attrib retrieved above
            CallCapture eglCreateImageKHRCall = egl::CaptureCreateImageKHR(
                nullptr, true, nullptr, context->id(), EGL_GL_TEXTURE_2D,
                reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(0)), attribs,
                reinterpret_cast<EGLImage>(static_cast<uintptr_t>(eglImageID)));

            // Convert the CaptureCreateImageKHR CallCapture to the customized CallCapture
            CaptureCustomCreateEGLImage(context, "CreateEGLImageKHR", eglImage->getWidth(),
                                        eglImage->getHeight(), eglCreateImageKHRCall, *calls);
        }
    }

    // Capture Texture setup and data.
    const gl::TextureManager &textures = apiState.getTextureManagerForCapture();

    for (const auto &textureIter : gl::UnsafeResourceMapIter(textures.getResourcesForCapture()))
    {
        gl::TextureID id     = {textureIter.first};
        gl::Texture *texture = textureIter.second;

        if (id.value == 0)
        {
            continue;
        }

        size_t textureSetupStart = setupCalls->size();

        // Track this as a starting resource that may need to be restored.
        TrackedResource &trackedTextures =
            resourceTracker->getTrackedResource(context->id(), ResourceIDType::Texture);
        ResourceSet &startingTextures = trackedTextures.getStartingResources();
        startingTextures.insert(id.value);

        // For the initial texture creation calls, track in the generate list
        ResourceCalls &textureRegenCalls = trackedTextures.getResourceRegenCalls();
        CallVector texGenCalls({setupCalls, &textureRegenCalls[id.value]});

        // Gen the Texture.
        for (std::vector<CallCapture> *calls : texGenCalls)
        {
            Capture(calls, CaptureGenTextures(replayState, true, 1, &id));
            MaybeCaptureUpdateResourceIDs(context, resourceTracker, calls);
        }

        // For the remaining texture setup calls, track in the restore list
        ResourceCalls &textureRestoreCalls = trackedTextures.getResourceRestoreCalls();
        CallVector texSetupCalls({setupCalls, &textureRestoreCalls[id.value]});

        // For each texture, set the correct active texture and binding
        // There is similar code in CaptureMidExecutionSetup for per-context setup
        const gl::TextureBindingMap &currentBoundTextures = apiState.getBoundTexturesForCapture();
        const gl::TextureBindingVector &currentBindings = currentBoundTextures[texture->getType()];
        const gl::TextureBindingVector &replayBindings =
            replayState.getBoundTexturesForCapture()[texture->getType()];
        ASSERT(currentBindings.size() == replayBindings.size());

        // Look up the replay binding
        size_t replayActiveTexture = replayState.getActiveSampler();
        // If there ends up being no binding, just use the replay binding
        // TODO: We may want to start using index 0 for everything, mark it dirty, and restore it
        size_t currentActiveTexture = replayActiveTexture;

        // Iterate through current bindings and find the correct index for this texture ID
        for (size_t bindingIndex = 0; bindingIndex < currentBindings.size(); ++bindingIndex)
        {
            gl::TextureID currentTextureID = currentBindings[bindingIndex].id();
            gl::TextureID replayTextureID  = replayBindings[bindingIndex].id();

            // Only check the texture we care about
            if (currentTextureID == texture->id())
            {
                // If the binding doesn't match, track it
                if (currentTextureID != replayTextureID)
                {
                    currentActiveTexture = bindingIndex;
                }

                break;
            }
        }

        // Set the correct active texture before performing any state changes, including binding
        if (currentActiveTexture != replayActiveTexture)
        {
            for (std::vector<CallCapture> *calls : texSetupCalls)
            {
                Capture(calls, CaptureActiveTexture(
                                   replayState, true,
                                   GL_TEXTURE0 + static_cast<GLenum>(currentActiveTexture)));
            }
            replayState.getMutablePrivateStateForCapture()->setActiveSampler(
                static_cast<unsigned int>(currentActiveTexture));
        }

        for (std::vector<CallCapture> *calls : texSetupCalls)
        {
            Capture(calls, CaptureBindTexture(replayState, true, texture->getType(), id));
        }
        replayState.setSamplerTexture(context, texture->getType(), texture);

        // Capture sampler parameter states.
        // TODO(jmadill): More sampler / texture states. http://anglebug.com/42262323
        gl::SamplerState defaultSamplerState =
            gl::SamplerState::CreateDefaultForTarget(texture->getType());
        const gl::SamplerState &textureSamplerState = texture->getSamplerState();

        auto capTexParam = [&replayState, texture, &texSetupCalls](GLenum pname, GLint param) {
            for (std::vector<CallCapture> *calls : texSetupCalls)
            {
                Capture(calls,
                        CaptureTexParameteri(replayState, true, texture->getType(), pname, param));
            }
        };

        auto capTexParamf = [&replayState, texture, &texSetupCalls](GLenum pname, GLfloat param) {
            for (std::vector<CallCapture> *calls : texSetupCalls)
            {
                Capture(calls,
                        CaptureTexParameterf(replayState, true, texture->getType(), pname, param));
            }
        };

        if (textureSamplerState.getMinFilter() != defaultSamplerState.getMinFilter())
        {
            capTexParam(GL_TEXTURE_MIN_FILTER, textureSamplerState.getMinFilter());
        }

        if (textureSamplerState.getMagFilter() != defaultSamplerState.getMagFilter())
        {
            capTexParam(GL_TEXTURE_MAG_FILTER, textureSamplerState.getMagFilter());
        }

        if (textureSamplerState.getWrapR() != defaultSamplerState.getWrapR())
        {
            capTexParam(GL_TEXTURE_WRAP_R, textureSamplerState.getWrapR());
        }

        if (textureSamplerState.getWrapS() != defaultSamplerState.getWrapS())
        {
            capTexParam(GL_TEXTURE_WRAP_S, textureSamplerState.getWrapS());
        }

        if (textureSamplerState.getWrapT() != defaultSamplerState.getWrapT())
        {
            capTexParam(GL_TEXTURE_WRAP_T, textureSamplerState.getWrapT());
        }

        if (textureSamplerState.getMinLod() != defaultSamplerState.getMinLod())
        {
            capTexParamf(GL_TEXTURE_MIN_LOD, textureSamplerState.getMinLod());
        }

        if (textureSamplerState.getMaxLod() != defaultSamplerState.getMaxLod())
        {
            capTexParamf(GL_TEXTURE_MAX_LOD, textureSamplerState.getMaxLod());
        }

        if (textureSamplerState.getCompareMode() != defaultSamplerState.getCompareMode())
        {
            capTexParam(GL_TEXTURE_COMPARE_MODE, textureSamplerState.getCompareMode());
        }

        if (textureSamplerState.getCompareFunc() != defaultSamplerState.getCompareFunc())
        {
            capTexParam(GL_TEXTURE_COMPARE_FUNC, textureSamplerState.getCompareFunc());
        }

        // Texture parameters
        if (texture->getSwizzleRed() != GL_RED)
        {
            capTexParam(GL_TEXTURE_SWIZZLE_R, texture->getSwizzleRed());
        }

        if (texture->getSwizzleGreen() != GL_GREEN)
        {
            capTexParam(GL_TEXTURE_SWIZZLE_G, texture->getSwizzleGreen());
        }

        if (texture->getSwizzleBlue() != GL_BLUE)
        {
            capTexParam(GL_TEXTURE_SWIZZLE_B, texture->getSwizzleBlue());
        }

        if (texture->getSwizzleAlpha() != GL_ALPHA)
        {
            capTexParam(GL_TEXTURE_SWIZZLE_A, texture->getSwizzleAlpha());
        }

        if (texture->getBaseLevel() != 0)
        {
            capTexParam(GL_TEXTURE_BASE_LEVEL, texture->getBaseLevel());
        }

        if (texture->getMaxLevel() != 1000)
        {
            capTexParam(GL_TEXTURE_MAX_LEVEL, texture->getMaxLevel());
        }

        // If the texture is immutable, initialize it with TexStorage
        if (texture->getImmutableFormat())
        {
            // We can only call TexStorage *once* on an immutable texture, so it needs special
            // handling. To solve this, immutable textures will have a BindTexture and TexStorage as
            // part of their textureRegenCalls. The resulting regen sequence will be:
            //
            //    const GLuint glDeleteTextures_texturesPacked_0[] = { gTextureMap[52] };
            //    glDeleteTextures(1, glDeleteTextures_texturesPacked_0);
            //    glGenTextures(1, reinterpret_cast<GLuint *>(gReadBuffer));
            //    UpdateTextureID(52, 0);
            //    glBindTexture(GL_TEXTURE_2D, gTextureMap[52]);
            //    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, 256, 512);

            // Bind the texture first just for textureRegenCalls
            Capture(&textureRegenCalls[id.value],
                    CaptureBindTexture(replayState, true, texture->getType(), id));

            // Then add TexStorage to texGenCalls instead of texSetupCalls
            for (std::vector<CallCapture> *calls : texGenCalls)
            {
                CaptureTextureStorage(calls, &replayState, texture);
            }
        }

        // Iterate texture levels and layers.
        gl::ImageIndexIterator imageIter = gl::ImageIndexIterator::MakeGeneric(
            texture->getType(), 0, texture->getMipmapMaxLevel() + 1, gl::ImageIndex::kEntireLevel,
            gl::ImageIndex::kEntireLevel);
        while (imageIter.hasNext())
        {
            gl::ImageIndex index = imageIter.next();

            const gl::ImageDesc &desc = texture->getTextureState().getImageDesc(index);

            if (desc.size.empty())
            {
                continue;
            }

            const gl::InternalFormat &format = *desc.format.info;

            bool supportedType = (index.getType() == gl::TextureType::_2D ||
                                  index.getType() == gl::TextureType::_3D ||
                                  index.getType() == gl::TextureType::_2DArray ||
                                  index.getType() == gl::TextureType::Buffer ||
                                  index.getType() == gl::TextureType::CubeMap ||
                                  index.getType() == gl::TextureType::CubeMapArray ||
                                  index.getType() == gl::TextureType::External);

            // Check for supported textures
            if (!supportedType)
            {
                ERR() << "Unsupported texture type: " << index.getType();
                UNREACHABLE();
            }

            if (index.getType() == gl::TextureType::Buffer)
            {
                // The buffer contents are already backed up, but we need to emit the TexBuffer
                // binding calls
                for (std::vector<CallCapture> *calls : texSetupCalls)
                {
                    CaptureTextureContents(calls, &replayState, texture, index, desc, 0, 0);
                }
                continue;
            }

            if (index.getType() == gl::TextureType::External)
            {
                // Lookup the eglImage ID associated with this texture when the app issued
                // glEGLImageTargetTexture2DOES()
                auto eglImageIter = resourceTracker->getTextureIDToImageTable().find(id.value);
                egl::ImageID eglImageID;
                if (eglImageIter != resourceTracker->getTextureIDToImageTable().end())
                {
                    eglImageID = eglImageIter->second;
                }
                else
                {
                    // Original image was deleted and needs to be recreated first
                    eglImageID = {maxAccessedResourceIDs[ResourceIDType::Image] + 1};
                    for (std::vector<CallCapture> *calls : texSetupCalls)
                    {
                        egl::AttributeMap attribs = egl::AttributeMap::CreateFromIntArray(nullptr);
                        CallCapture eglCreateImageKHRCall = egl::CaptureCreateImageKHR(
                            nullptr, true, nullptr, context->id(), EGL_GL_TEXTURE_2D,
                            reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(0)), attribs,
                            reinterpret_cast<EGLImage>(static_cast<uintptr_t>(eglImageID.value)));
                        CaptureCustomCreateEGLImage(context, "CreateEGLImageKHR", desc.size.width,
                                                    desc.size.height, eglCreateImageKHRCall,
                                                    *calls);
                    }
                }
                // Pass the eglImage to the texture that is bound to GL_TEXTURE_EXTERNAL_OES target
                for (std::vector<CallCapture> *calls : texSetupCalls)
                {
                    Capture(calls, CaptureEGLImageTargetTexture2DOES(
                                       replayState, true, gl::TextureType::External, eglImageID));
                }
            }
            else if (context->getExtensions().getImageANGLE)
            {
                // Use ANGLE_get_image to read back pixel data.
                angle::MemoryBuffer data;

                const gl::Extents extents(desc.size.width, desc.size.height, desc.size.depth);

                gl::PixelPackState packState;
                packState.alignment = 1;

                if (format.paletted)
                {
                    // Read back the uncompressed texture, then re-compress it
                    // to store in the trace.

                    angle::MemoryBuffer tmp;

                    // The uncompressed format (R8G8B8A8) is 4 bytes per texel
                    bool result = tmp.resize(4 * extents.width * extents.height * extents.depth);
                    ASSERT(result);

                    (void)texture->getTexImage(context, packState, nullptr, index.getTarget(),
                                               index.getLevelIndex(), GL_RGBA, GL_UNSIGNED_BYTE,
                                               tmp.data());

                    CompressPalettedTexture(data, tmp, format, extents);
                }
                else if (format.compressed)
                {
                    // Calculate the size needed to store the compressed level
                    GLuint sizeInBytes;
                    bool result = format.computeCompressedImageSize(extents, &sizeInBytes);
                    ASSERT(result);

                    result = data.resize(sizeInBytes);
                    ASSERT(result);

                    (void)texture->getCompressedTexImage(context, packState, nullptr,
                                                         index.getTarget(), index.getLevelIndex(),
                                                         data.data());
                }
                else
                {
                    GLenum getFormat = format.format;
                    GLenum getType   = format.type;

                    const gl::PixelUnpackState &unpack = apiState.getUnpackState();

                    GLuint endByte = 0;
                    bool unpackSize =
                        format.computePackUnpackEndByte(getType, extents, unpack, true, &endByte);
                    ASSERT(unpackSize);

                    bool result = data.resize(endByte);
                    ASSERT(result);

                    (void)texture->getTexImage(context, packState, nullptr, index.getTarget(),
                                               index.getLevelIndex(), getFormat, getType,
                                               data.data());
                }

                for (std::vector<CallCapture> *calls : texSetupCalls)
                {
                    CaptureTextureContents(calls, &replayState, texture, index, desc,
                                           static_cast<GLuint>(data.size()), data.data());
                }
            }
            else
            {
                for (std::vector<CallCapture> *calls : texSetupCalls)
                {
                    CaptureTextureContents(calls, &replayState, texture, index, desc, 0, nullptr);
                }
            }
        }

        size_t textureSetupEnd = setupCalls->size();

        // Mark the range of calls used to setup this texture
        frameCaptureShared->markResourceSetupCallsInactive(
            setupCalls, ResourceIDType::Texture, id.value,
            gl::Range<size_t>(textureSetupStart, textureSetupEnd));
    }

    // Capture Renderbuffers.
    const gl::RenderbufferManager &renderbuffers = apiState.getRenderbufferManagerForCapture();
    FramebufferCaptureFuncs framebufferFuncs(context->isGLES1());

    for (const auto &renderbufIter :
         gl::UnsafeResourceMapIter(renderbuffers.getResourcesForCapture()))
    {
        gl::RenderbufferID id                = {renderbufIter.first};
        const gl::Renderbuffer *renderbuffer = renderbufIter.second;

        // Track this as a starting resource that may need to be restored.
        TrackedResource &trackedRenderbuffers =
            resourceTracker->getTrackedResource(context->id(), ResourceIDType::Renderbuffer);
        ResourceSet &startingRenderbuffers = trackedRenderbuffers.getStartingResources();
        startingRenderbuffers.insert(id.value);

        // For the initial renderbuffer creation calls, track in the generate list
        ResourceCalls &renderbufferRegenCalls = trackedRenderbuffers.getResourceRegenCalls();
        CallVector rbGenCalls({setupCalls, &renderbufferRegenCalls[id.value]});

        // Generate renderbuffer id.
        for (std::vector<CallCapture> *calls : rbGenCalls)
        {
            Capture(calls, framebufferFuncs.genRenderbuffers(replayState, true, 1, &id));
            MaybeCaptureUpdateResourceIDs(context, resourceTracker, calls);
            Capture(calls,
                    framebufferFuncs.bindRenderbuffer(replayState, true, GL_RENDERBUFFER, id));
        }

        GLenum internalformat = renderbuffer->getFormat().info->internalFormat;

        if (renderbuffer->getSamples() > 0)
        {
            // Note: We could also use extensions if available.
            for (std::vector<CallCapture> *calls : rbGenCalls)
            {
                Capture(calls,
                        CaptureRenderbufferStorageMultisample(
                            replayState, true, GL_RENDERBUFFER, renderbuffer->getSamples(),
                            internalformat, renderbuffer->getWidth(), renderbuffer->getHeight()));
            }
        }
        else
        {
            for (std::vector<CallCapture> *calls : rbGenCalls)
            {
                Capture(calls, framebufferFuncs.renderbufferStorage(
                                   replayState, true, GL_RENDERBUFFER, internalformat,
                                   renderbuffer->getWidth(), renderbuffer->getHeight()));
            }
        }

        // TODO: Capture renderbuffer contents. http://anglebug.com/42262323
    }

    // Capture Shaders and Programs.
    const gl::ShaderProgramManager &shadersAndPrograms =
        apiState.getShaderProgramManagerForCapture();
    const gl::ResourceMap<gl::Shader, gl::ShaderProgramID> &shaders =
        shadersAndPrograms.getShadersForCapture();
    const gl::ResourceMap<gl::Program, gl::ShaderProgramID> &programs =
        shadersAndPrograms.getProgramsForCaptureAndPerf();

    TrackedResource &trackedShaderPrograms =
        resourceTracker->getTrackedResource(context->id(), ResourceIDType::ShaderProgram);

    // Capture Program binary state.
    gl::ShaderProgramID tempShaderStartID = {resourceTracker->getMaxShaderPrograms()};
    std::map<gl::ShaderProgramID, std::vector<gl::ShaderProgramID>> deferredAttachCalls;
    for (const auto &programIter : gl::UnsafeResourceMapIter(programs))
    {
        gl::ShaderProgramID id = {programIter.first};
        gl::Program *program   = programIter.second;

        // Unlinked programs don't have an executable so track in case linking is deferred
        // Programs are shared by contexts in the share group and only need to be captured once.
        if (!program->isLinked())
        {
            frameCaptureShared->setDeferredLinkProgram(id);

            if (program->getAttachedShadersCount() > 0)
            {
                // AttachShader calls will be generated at shader-handling time
                for (gl::ShaderType shaderType : gl::AllShaderTypes())
                {
                    gl::Shader *shader = program->getAttachedShader(shaderType);
                    if (shader != nullptr)
                    {
                        deferredAttachCalls[shader->getHandle()].push_back(id);
                    }
                }
            }
            else
            {
                WARN() << "Deferred attachment of shaders is not yet supported";
            }
        }

        size_t programSetupStart = setupCalls->size();

        // Create two lists for program regen calls
        ResourceCalls &shaderProgramRegenCalls = trackedShaderPrograms.getResourceRegenCalls();
        CallVector programRegenCalls({setupCalls, &shaderProgramRegenCalls[id.value]});

        for (std::vector<CallCapture> *calls : programRegenCalls)
        {
            CallCapture createProgram = CaptureCreateProgram(replayState, true, id.value);
            CaptureCustomShaderProgram("CreateProgram", createProgram, *calls);
        }

        if (program->isLinked())
        {
            // Get last linked shader source.
            const ProgramSources &linkedSources =
                context->getShareGroup()->getFrameCaptureShared()->getProgramSources(id);

            // Create two lists for program restore calls
            ResourceCalls &shaderProgramRestoreCalls =
                trackedShaderPrograms.getResourceRestoreCalls();
            CallVector programRestoreCalls({setupCalls, &shaderProgramRestoreCalls[id.value]});

            for (std::vector<CallCapture> *calls : programRestoreCalls)
            {
                GenerateLinkedProgram(context, replayState, resourceTracker, calls, program, id,
                                      tempShaderStartID, linkedSources);
            }

            // Update the program in replayState
            if (!replayState.getProgram() || replayState.getProgram()->id() != program->id())
            {
                // Note: We don't do this in GenerateLinkedProgram because it can't modify state
                (void)replayState.setProgram(context, program);
            }
        }

        resourceTracker->getTrackedResource(context->id(), ResourceIDType::ShaderProgram)
            .getStartingResources()
            .insert(id.value);
        resourceTracker->setShaderProgramType(id, ShaderProgramType::ProgramType);

        // Mark linked programs/shaders as inactive, leaving deferred-linked programs/shaders marked
        // as active
        if (!frameCaptureShared->isDeferredLinkProgram(id))
        {
            size_t programSetupEnd = setupCalls->size();

            // Mark the range of calls used to setup this program
            frameCaptureShared->markResourceSetupCallsInactive(
                setupCalls, ResourceIDType::ShaderProgram, id.value,
                gl::Range<size_t>(programSetupStart, programSetupEnd));
        }
    }

    // Handle shaders.
    for (const auto &shaderIter : gl::UnsafeResourceMapIter(shaders))
    {
        gl::ShaderProgramID id = {shaderIter.first};
        gl::Shader *shader     = shaderIter.second;

        // Skip shaders scheduled for deletion.
        // Shaders are shared by contexts in the share group and only need to be captured once.
        if (shader->hasBeenDeleted())
        {
            continue;
        }

        size_t shaderSetupStart = setupCalls->size();

        // Create two lists for shader regen calls
        ResourceCalls &shaderProgramRegenCalls = trackedShaderPrograms.getResourceRegenCalls();
        CallVector shaderRegenCalls({setupCalls, &shaderProgramRegenCalls[id.value]});

        for (std::vector<CallCapture> *calls : shaderRegenCalls)
        {
            CallCapture createShader =
                CaptureCreateShader(replayState, true, shader->getType(), id.value);
            CaptureCustomShaderProgram("CreateShader", createShader, *calls);

            // If unlinked programs have been created which reference this shader emit corresponding
            // attach calls
            for (const auto deferredAttachedProgramID : deferredAttachCalls[id])
            {
                CallCapture attachShader =
                    CaptureAttachShader(replayState, true, deferredAttachedProgramID, id);
                calls->emplace_back(std::move(attachShader));
            }
        }

        std::string shaderSource  = shader->getSourceString();
        const char *sourcePointer = shaderSource.empty() ? nullptr : shaderSource.c_str();

        // Create two lists for shader restore calls
        ResourceCalls &shaderProgramRestoreCalls = trackedShaderPrograms.getResourceRestoreCalls();
        CallVector shaderRestoreCalls({setupCalls, &shaderProgramRestoreCalls[id.value]});

        // This does not handle some more tricky situations like attaching and then deleting a
        // shader.
        // TODO(jmadill): Handle trickier program uses. http://anglebug.com/42262323
        if (shader->isCompiled(context))
        {
            const std::string &capturedSource =
                context->getShareGroup()->getFrameCaptureShared()->getShaderSource(id);
            if (capturedSource != shaderSource)
            {
                ASSERT(!capturedSource.empty());
                sourcePointer = capturedSource.c_str();
            }

            for (std::vector<CallCapture> *calls : shaderRestoreCalls)
            {
                Capture(calls,
                        CaptureShaderSource(replayState, true, id, 1, &sourcePointer, nullptr));
                Capture(calls, CaptureCompileShader(replayState, true, id));
            }
        }

        if (sourcePointer &&
            (!shader->isCompiled(context) || sourcePointer != shaderSource.c_str()))
        {
            for (std::vector<CallCapture> *calls : shaderRestoreCalls)
            {
                Capture(calls,
                        CaptureShaderSource(replayState, true, id, 1, &sourcePointer, nullptr));
            }
        }

        // Deferred-linked programs/shaders must be left marked as active
        if (deferredAttachCalls[id].empty())
        {
            // Mark the range of calls used to setup this shader
            frameCaptureShared->markResourceSetupCallsInactive(
                setupCalls, ResourceIDType::ShaderProgram, id.value,
                gl::Range<size_t>(shaderSetupStart, setupCalls->size()));
        }

        resourceTracker->getTrackedResource(context->id(), ResourceIDType::ShaderProgram)
            .getStartingResources()
            .insert(id.value);
        resourceTracker->setShaderProgramType(id, ShaderProgramType::ShaderType);
    }

    // Capture Sampler Objects
    const gl::SamplerManager &samplers = apiState.getSamplerManagerForCapture();
    for (const auto &samplerIter : gl::UnsafeResourceMapIter(samplers.getResourcesForCapture()))
    {
        gl::SamplerID samplerID = {samplerIter.first};

        // Don't gen the sampler if we've seen it before, since they are shared across the context
        // share group.
        cap(CaptureGenSamplers(replayState, true, 1, &samplerID));
        MaybeCaptureUpdateResourceIDs(context, resourceTracker, setupCalls);

        gl::Sampler *sampler = samplerIter.second;
        if (!sampler)
        {
            continue;
        }

        gl::SamplerState defaultSamplerState;
        if (sampler->getMinFilter() != defaultSamplerState.getMinFilter())
        {
            cap(CaptureSamplerParameteri(replayState, true, samplerID, GL_TEXTURE_MIN_FILTER,
                                         sampler->getMinFilter()));
        }
        if (sampler->getMagFilter() != defaultSamplerState.getMagFilter())
        {
            cap(CaptureSamplerParameteri(replayState, true, samplerID, GL_TEXTURE_MAG_FILTER,
                                         sampler->getMagFilter()));
        }
        if (sampler->getWrapS() != defaultSamplerState.getWrapS())
        {
            cap(CaptureSamplerParameteri(replayState, true, samplerID, GL_TEXTURE_WRAP_S,
                                         sampler->getWrapS()));
        }
        if (sampler->getWrapR() != defaultSamplerState.getWrapR())
        {
            cap(CaptureSamplerParameteri(replayState, true, samplerID, GL_TEXTURE_WRAP_R,
                                         sampler->getWrapR()));
        }
        if (sampler->getWrapT() != defaultSamplerState.getWrapT())
        {
            cap(CaptureSamplerParameteri(replayState, true, samplerID, GL_TEXTURE_WRAP_T,
                                         sampler->getWrapT()));
        }
        if (sampler->getMinLod() != defaultSamplerState.getMinLod())
        {
            cap(CaptureSamplerParameterf(replayState, true, samplerID, GL_TEXTURE_MIN_LOD,
                                         sampler->getMinLod()));
        }
        if (sampler->getMaxLod() != defaultSamplerState.getMaxLod())
        {
            cap(CaptureSamplerParameterf(replayState, true, samplerID, GL_TEXTURE_MAX_LOD,
                                         sampler->getMaxLod()));
        }
        if (sampler->getCompareMode() != defaultSamplerState.getCompareMode())
        {
            cap(CaptureSamplerParameteri(replayState, true, samplerID, GL_TEXTURE_COMPARE_MODE,
                                         sampler->getCompareMode()));
        }
        if (sampler->getCompareFunc() != defaultSamplerState.getCompareFunc())
        {
            cap(CaptureSamplerParameteri(replayState, true, samplerID, GL_TEXTURE_COMPARE_FUNC,
                                         sampler->getCompareFunc()));
        }
    }

    // Capture Sync Objects
    const gl::SyncManager &syncs = apiState.getSyncManagerForCapture();
    for (const auto &syncIter : gl::UnsafeResourceMapIter(syncs.getResourcesForCapture()))
    {
        gl::SyncID syncID    = {syncIter.first};
        const gl::Sync *sync = syncIter.second;
        GLsync syncObject    = gl::unsafe_int_to_pointer_cast<GLsync>(syncID.value);

        if (!sync)
        {
            continue;
        }
        CallCapture fenceSync =
            CaptureFenceSync(replayState, true, sync->getCondition(), sync->getFlags(), syncObject);
        CaptureCustomFenceSync(fenceSync, *setupCalls);
        CaptureFenceSyncResetCalls(context, replayState, resourceTracker, syncID, syncObject, sync);
        resourceTracker->getStartingFenceSyncs().insert(syncID);
    }

    // Capture EGL Sync Objects
    const egl::SyncMap &eglSyncMap = context->getDisplay()->getSyncsForCapture();
    for (const auto &eglSyncIter : eglSyncMap)
    {
        egl::SyncID eglSyncID    = {eglSyncIter.first};
        const egl::Sync *eglSync = eglSyncIter.second.get();
        EGLSync eglSyncObject    = gl::unsafe_int_to_pointer_cast<EGLSync>(eglSyncID.value);

        if (!eglSync)
        {
            continue;
        }
        CallCapture createEGLSync =
            CaptureCreateSync(nullptr, true, context->getDisplay(), eglSync->getType(),
                              eglSync->getAttributeMap(), eglSyncObject);
        CaptureCustomCreateEGLSync("CreateEGLSyncKHR", createEGLSync, *setupCalls);
        CaptureEGLSyncResetCalls(context, replayState, resourceTracker, eglSyncID, eglSyncObject,
                                 eglSync);
        resourceTracker->getTrackedResource(context->id(), ResourceIDType::egl_Sync)
            .getStartingResources()
            .insert(eglSyncID.value);
    }

    GLint contextUnpackAlignment = context->getState().getUnpackState().alignment;
    if (currentUnpackState.alignment != contextUnpackAlignment)
    {
        cap(CapturePixelStorei(replayState, true, GL_UNPACK_ALIGNMENT, contextUnpackAlignment));
        replayState.getMutablePrivateStateForCapture()->setUnpackAlignment(contextUnpackAlignment);
    }
}

void CaptureMidExecutionSetup(const gl::Context *context,
                              std::vector<CallCapture> *setupCalls,
                              StateResetHelper &resetHelper,
                              std::vector<CallCapture> *shareGroupSetupCalls,
                              ResourceIDToSetupCallsMap *resourceIDToSetupCalls,
                              ResourceTracker *resourceTracker,
                              gl::State &replayState,
                              bool validationEnabled)
{
    const gl::State &apiState = context->getState();

    // Small helper function to make the code more readable.
    auto cap = [setupCalls](CallCapture &&call) { setupCalls->emplace_back(std::move(call)); };

    cap(egl::CaptureMakeCurrent(nullptr, true, nullptr, {0}, {0}, context->id(), EGL_TRUE));

    // Vertex input states. Must happen after buffer data initialization. Do not capture on GLES1.
    if (!context->isGLES1())
    {
        CaptureDefaultVertexAttribs(replayState, apiState, setupCalls);
    }

    // Capture vertex array objects
    VertexArrayCaptureFuncs vertexArrayFuncs(context->isGLES1());

    const gl::VertexArrayMap &vertexArrayMap = context->getVertexArraysForCapture();
    gl::VertexArrayID boundVertexArrayID     = {0};
    for (const auto &vertexArrayIter : gl::UnsafeResourceMapIter(vertexArrayMap))
    {
        TrackedResource &trackedVertexArrays =
            resourceTracker->getTrackedResource(context->id(), ResourceIDType::VertexArray);

        gl::VertexArrayID vertexArrayID = {vertexArrayIter.first};

        // Track this as a starting resource that may need to be restored
        resourceTracker->getTrackedResource(context->id(), ResourceIDType::VertexArray)
            .getStartingResources()
            .insert(vertexArrayID.value);

        // Create two lists of calls for initial setup
        ResourceCalls &vertexArrayRegenCalls = trackedVertexArrays.getResourceRegenCalls();
        CallVector vertexArrayGenCalls({setupCalls, &vertexArrayRegenCalls[vertexArrayID.value]});

        if (vertexArrayID.value != 0)
        {
            // Gen the vertex array
            for (std::vector<CallCapture> *calls : vertexArrayGenCalls)
            {
                Capture(calls,
                        vertexArrayFuncs.genVertexArrays(replayState, true, 1, &vertexArrayID));
                MaybeCaptureUpdateResourceIDs(context, resourceTracker, calls);
            }
        }

        // Create two lists of calls for populating the vertex array
        ResourceCalls &vertexArrayRestoreCalls = trackedVertexArrays.getResourceRestoreCalls();
        CallVector vertexArraySetupCalls(
            {setupCalls, &vertexArrayRestoreCalls[vertexArrayID.value]});

        if (vertexArrayIter.second)
        {
            const gl::VertexArray *vertexArray = vertexArrayIter.second;

            // Populate the vertex array
            for (std::vector<CallCapture> *calls : vertexArraySetupCalls)
            {
                // Bind the vertexArray (if needed) and populate it
                if (vertexArrayID != boundVertexArrayID)
                {
                    Capture(calls,
                            vertexArrayFuncs.bindVertexArray(replayState, true, vertexArrayID));
                }
                CaptureVertexArrayState(calls, context, vertexArray, &replayState);
            }
            boundVertexArrayID = vertexArrayID;
        }
    }

    // Bind the current vertex array
    const gl::VertexArray *currentVertexArray = apiState.getVertexArray();
    if (currentVertexArray->id() != boundVertexArrayID)
    {
        cap(vertexArrayFuncs.bindVertexArray(replayState, true, currentVertexArray->id()));
    }

    // Track the calls necessary to bind the vertex array back to initial state
    CallResetMap &resetCalls = resetHelper.getResetCalls();
    Capture(&resetCalls[angle::EntryPoint::GLBindVertexArray],
            vertexArrayFuncs.bindVertexArray(replayState, true, currentVertexArray->id()));

    // Capture indexed buffer bindings.
    const gl::BufferVector &uniformIndexedBuffers =
        apiState.getOffsetBindingPointerUniformBuffers();
    const gl::BufferVector &atomicCounterIndexedBuffers =
        apiState.getOffsetBindingPointerAtomicCounterBuffers();
    const gl::BufferVector &shaderStorageIndexedBuffers =
        apiState.getOffsetBindingPointerShaderStorageBuffers();
    CaptureIndexedBuffers(replayState, uniformIndexedBuffers, gl::BufferBinding::Uniform,
                          setupCalls);
    CaptureIndexedBuffers(replayState, atomicCounterIndexedBuffers,
                          gl::BufferBinding::AtomicCounter, setupCalls);
    CaptureIndexedBuffers(replayState, shaderStorageIndexedBuffers,
                          gl::BufferBinding::ShaderStorage, setupCalls);

    // Capture Buffer bindings.
    const gl::BoundBufferMap &boundBuffers = apiState.getBoundBuffersForCapture();
    for (gl::BufferBinding binding : angle::AllEnums<gl::BufferBinding>())
    {
        gl::BufferID bufferID = boundBuffers[binding].id();

        // Filter out redundant buffer binding commands. Note that the code in the previous section
        // only binds to ARRAY_BUFFER. Therefore we only check the array binding against the binding
        // we set earlier.
        const gl::Buffer *arrayBuffer = replayState.getArrayBuffer();
        bool isArray                  = binding == gl::BufferBinding::Array;
        bool isArrayBufferChanging    = isArray && arrayBuffer && arrayBuffer->id() != bufferID;
        if (isArrayBufferChanging || (!isArray && bufferID.value != 0))
        {
            cap(CaptureBindBuffer(replayState, true, binding, bufferID));
            replayState.setBufferBinding(context, binding, boundBuffers[binding].get());
        }

        // Restore all buffer bindings for Reset
        if (bufferID.value != 0 || isArrayBufferChanging)
        {
            CaptureBufferBindingResetCalls(replayState, resourceTracker, binding, bufferID);

            // Track this as a starting binding
            resetHelper.setStartingBufferBinding(binding, bufferID);
        }
    }

    // Set a unpack alignment of 1. Otherwise, computeRowPitch() will compute the wrong value,
    // leading to a crash in memcpy() when capturing the texture contents.
    gl::PixelUnpackState &currentUnpackState = replayState.getUnpackState();
    if (currentUnpackState.alignment != 1)
    {
        cap(CapturePixelStorei(replayState, true, GL_UNPACK_ALIGNMENT, 1));
        replayState.getMutablePrivateStateForCapture()->setUnpackAlignment(1);
    }

    // Capture Texture setup and data.
    const gl::TextureBindingMap &apiBoundTextures = apiState.getBoundTexturesForCapture();
    resetHelper.setResetActiveTexture(apiState.getActiveSampler());

    // Set Texture bindings.
    for (gl::TextureType textureType : angle::AllEnums<gl::TextureType>())
    {
        const gl::TextureBindingVector &apiBindings = apiBoundTextures[textureType];
        const gl::TextureBindingVector &replayBindings =
            replayState.getBoundTexturesForCapture()[textureType];
        ASSERT(apiBindings.size() == replayBindings.size());
        for (size_t bindingIndex = 0; bindingIndex < apiBindings.size(); ++bindingIndex)
        {
            gl::TextureID apiTextureID    = apiBindings[bindingIndex].id();
            gl::TextureID replayTextureID = replayBindings[bindingIndex].id();

            if (apiTextureID != replayTextureID)
            {
                if (replayState.getActiveSampler() != bindingIndex)
                {
                    cap(CaptureActiveTexture(replayState, true,
                                             GL_TEXTURE0 + static_cast<GLenum>(bindingIndex)));
                    replayState.getMutablePrivateStateForCapture()->setActiveSampler(
                        static_cast<unsigned int>(bindingIndex));
                }

                cap(CaptureBindTexture(replayState, true, textureType, apiTextureID));
                replayState.setSamplerTexture(context, textureType,
                                              apiBindings[bindingIndex].get());
            }

            if (apiTextureID.value)
            {
                // Set this texture as active so it will be generated in Setup
                MarkResourceIDActive(ResourceIDType::Texture, apiTextureID.value,
                                     shareGroupSetupCalls, resourceIDToSetupCalls);
                // Save currently bound textures for reset
                resetHelper.getResetTextureBindings().emplace(
                    std::make_pair(bindingIndex, textureType), apiTextureID);
            }
        }
    }

    // Capture Texture Environment
    if (context->isGLES1())
    {
        const gl::Caps &caps = context->getCaps();
        for (GLuint unit = 0; unit < caps.maxMultitextureUnits; ++unit)
        {
            CaptureTextureEnvironmentState(setupCalls, &replayState, &apiState, unit);
        }
    }

    // Set active Texture.
    if (replayState.getActiveSampler() != apiState.getActiveSampler())
    {
        cap(CaptureActiveTexture(replayState, true,
                                 GL_TEXTURE0 + static_cast<GLenum>(apiState.getActiveSampler())));
        replayState.getMutablePrivateStateForCapture()->setActiveSampler(
            apiState.getActiveSampler());
    }

    // Set Renderbuffer binding.
    FramebufferCaptureFuncs framebufferFuncs(context->isGLES1());

    const gl::RenderbufferManager &renderbuffers = apiState.getRenderbufferManagerForCapture();
    gl::RenderbufferID currentRenderbuffer       = {0};
    for (const auto &renderbufIter :
         gl::UnsafeResourceMapIter(renderbuffers.getResourcesForCapture()))
    {
        currentRenderbuffer = renderbufIter.second->id();
    }

    if (currentRenderbuffer != apiState.getRenderbufferId())
    {
        cap(framebufferFuncs.bindRenderbuffer(replayState, true, GL_RENDERBUFFER,
                                              apiState.getRenderbufferId()));
    }

    // Capture Framebuffers.
    const gl::FramebufferManager &framebuffers = apiState.getFramebufferManagerForCapture();

    gl::FramebufferID currentDrawFramebuffer = {0};
    gl::FramebufferID currentReadFramebuffer = {0};

    for (const auto &framebufferIter :
         gl::UnsafeResourceMapIter(framebuffers.getResourcesForCapture()))
    {
        gl::FramebufferID id               = {framebufferIter.first};
        const gl::Framebuffer *framebuffer = framebufferIter.second;

        // The default Framebuffer exists (by default).
        if (framebuffer->isDefault())
        {
            continue;
        }

        // Track this as a starting resource that may need to be restored
        TrackedResource &trackedFramebuffers =
            resourceTracker->getTrackedResource(context->id(), ResourceIDType::Framebuffer);
        ResourceSet &startingFramebuffers = trackedFramebuffers.getStartingResources();
        startingFramebuffers.insert(id.value);

        // Create two lists of calls for initial setup
        ResourceCalls &framebufferRegenCalls = trackedFramebuffers.getResourceRegenCalls();
        CallVector framebufferGenCalls({setupCalls, &framebufferRegenCalls[id.value]});

        // Gen the framebuffer
        for (std::vector<CallCapture> *calls : framebufferGenCalls)
        {
            Capture(calls, framebufferFuncs.genFramebuffers(replayState, true, 1, &id));
            MaybeCaptureUpdateResourceIDs(context, resourceTracker, calls);
        }

        // Create two lists of calls for remaining setup calls.  One for setup, and one for restore
        // during reset.
        ResourceCalls &framebufferRestoreCalls = trackedFramebuffers.getResourceRestoreCalls();
        CallVector framebufferSetupCalls({setupCalls, &framebufferRestoreCalls[id.value]});

        for (std::vector<CallCapture> *calls : framebufferSetupCalls)
        {
            Capture(calls, framebufferFuncs.bindFramebuffer(replayState, true, GL_FRAMEBUFFER, id));
            // Set current context for this CallCapture
            calls->back().contextID = context->id();
        }
        currentDrawFramebuffer = currentReadFramebuffer = id;

        // Color Attachments.
        for (const gl::FramebufferAttachment &colorAttachment : framebuffer->getColorAttachments())
        {
            if (!colorAttachment.isAttached())
            {
                continue;
            }

            for (std::vector<CallCapture> *calls : framebufferSetupCalls)
            {
                CaptureFramebufferAttachment(calls, replayState, framebufferFuncs, colorAttachment,
                                             shareGroupSetupCalls, resourceIDToSetupCalls);
            }
        }

        const gl::FramebufferAttachment *depthAttachment = framebuffer->getDepthAttachment();
        if (depthAttachment)
        {
            ASSERT(depthAttachment->getBinding() == GL_DEPTH_ATTACHMENT ||
                   depthAttachment->getBinding() == GL_DEPTH_STENCIL_ATTACHMENT);
            for (std::vector<CallCapture> *calls : framebufferSetupCalls)
            {
                CaptureFramebufferAttachment(calls, replayState, framebufferFuncs, *depthAttachment,
                                             shareGroupSetupCalls, resourceIDToSetupCalls);
            }
        }

        const gl::FramebufferAttachment *stencilAttachment = framebuffer->getStencilAttachment();
        if (stencilAttachment)
        {
            ASSERT(stencilAttachment->getBinding() == GL_STENCIL_ATTACHMENT ||
                   depthAttachment->getBinding() == GL_DEPTH_STENCIL_ATTACHMENT);
            for (std::vector<CallCapture> *calls : framebufferSetupCalls)
            {
                CaptureFramebufferAttachment(calls, replayState, framebufferFuncs,
                                             *stencilAttachment, shareGroupSetupCalls,
                                             resourceIDToSetupCalls);
            }
        }

        gl::FramebufferState defaultFramebufferState(
            context->getCaps(), framebuffer->getState().id(),
            framebuffer->getState().getFramebufferSerial());
        const gl::DrawBuffersVector<GLenum> &defaultDrawBufferStates =
            defaultFramebufferState.getDrawBufferStates();
        const gl::DrawBuffersVector<GLenum> &drawBufferStates = framebuffer->getDrawBufferStates();

        if (drawBufferStates != defaultDrawBufferStates)
        {
            for (std::vector<CallCapture> *calls : framebufferSetupCalls)
            {
                Capture(calls, CaptureDrawBuffers(replayState, true,
                                                  static_cast<GLsizei>(drawBufferStates.size()),
                                                  drawBufferStates.data()));
            }
        }
    }

    // Capture framebuffer bindings.
    if (apiState.getDrawFramebuffer())
    {
        ASSERT(apiState.getReadFramebuffer());
        gl::FramebufferID stateReadFramebuffer = apiState.getReadFramebuffer()->id();
        gl::FramebufferID stateDrawFramebuffer = apiState.getDrawFramebuffer()->id();
        if (stateDrawFramebuffer == stateReadFramebuffer)
        {
            if (currentDrawFramebuffer != stateDrawFramebuffer ||
                currentReadFramebuffer != stateReadFramebuffer)
            {
                cap(framebufferFuncs.bindFramebuffer(replayState, true, GL_FRAMEBUFFER,
                                                     stateDrawFramebuffer));
                currentDrawFramebuffer = currentReadFramebuffer = stateDrawFramebuffer;
            }
        }
        else
        {
            if (currentDrawFramebuffer != stateDrawFramebuffer)
            {
                cap(framebufferFuncs.bindFramebuffer(replayState, true, GL_DRAW_FRAMEBUFFER,
                                                     stateDrawFramebuffer));
                currentDrawFramebuffer = stateDrawFramebuffer;
            }

            if (currentReadFramebuffer != stateReadFramebuffer)
            {
                cap(framebufferFuncs.bindFramebuffer(replayState, true, GL_READ_FRAMEBUFFER,
                                                     stateReadFramebuffer));
                currentReadFramebuffer = stateReadFramebuffer;
            }
        }
    }

    // Capture Program Pipelines
    const gl::ProgramPipelineManager *programPipelineManager =
        apiState.getProgramPipelineManagerForCapture();

    for (const auto &ppoIterator :
         gl::UnsafeResourceMapIter(programPipelineManager->getResourcesForCapture()))
    {
        gl::ProgramPipeline *pipeline = ppoIterator.second;
        gl::ProgramPipelineID id      = {ppoIterator.first};
        cap(CaptureGenProgramPipelines(replayState, true, 1, &id));
        MaybeCaptureUpdateResourceIDs(context, resourceTracker, setupCalls);

        // PPOs can contain graphics and compute programs, so loop through all shader types rather
        // than just the linked ones since getLinkedShaderStages() will return either only graphics
        // or compute stages.
        for (gl::ShaderType shaderType : gl::AllShaderTypes())
        {
            const gl::Program *program = pipeline->getShaderProgram(shaderType);
            if (!program)
            {
                continue;
            }
            ASSERT(program->isLinked());
            GLbitfield gLbitfield = GetBitfieldFromShaderType(shaderType);
            cap(CaptureUseProgramStages(replayState, true, pipeline->id(), gLbitfield,
                                        program->id()));

            // Set this program as active so it will be generated in Setup
            // Note: We aren't filtering ProgramPipelines, so this could be setting programs
            // active that aren't actually used.
            MarkResourceIDActive(ResourceIDType::ShaderProgram, program->id().value,
                                 shareGroupSetupCalls, resourceIDToSetupCalls);
        }

        gl::Program *program = pipeline->getActiveShaderProgram();
        if (program)
        {
            cap(CaptureActiveShaderProgram(replayState, true, id, program->id()));
        }
    }

    // For now we assume the installed program executable is the same as the current program.
    // TODO(jmadill): Handle installed program executable. http://anglebug.com/42262323
    if (!context->isGLES1())
    {
        // If we have a program bound in the API, or if there is no program bound to the API at
        // time of capture and we bound a program for uniform updates during MEC, we must add
        // a set program call to replay the correct states.
        GLuint currentProgram = 0;
        if (apiState.getProgram())
        {
            cap(CaptureUseProgram(replayState, true, apiState.getProgram()->id()));
            CaptureUpdateCurrentProgram(setupCalls->back(), 0, setupCalls);
            (void)replayState.setProgram(context, apiState.getProgram());

            // Set this program as active so it will be generated in Setup
            MarkResourceIDActive(ResourceIDType::ShaderProgram, apiState.getProgram()->id().value,
                                 shareGroupSetupCalls, resourceIDToSetupCalls);

            currentProgram = apiState.getProgram()->id().value;
        }
        else if (replayState.getProgram())
        {
            cap(CaptureUseProgram(replayState, true, {0}));
            CaptureUpdateCurrentProgram(setupCalls->back(), 0, setupCalls);
            (void)replayState.setProgram(context, nullptr);
        }

        // Track the calls necessary to reset active program back to initial state
        Capture(&resetCalls[angle::EntryPoint::GLUseProgram],
                CaptureUseProgram(replayState, true, {currentProgram}));
        CaptureUpdateCurrentProgram((&resetCalls[angle::EntryPoint::GLUseProgram])->back(), 0,
                                    &resetCalls[angle::EntryPoint::GLUseProgram]);

        // Same for program pipelines as for programs, see comment above.
        if (apiState.getProgramPipeline())
        {
            cap(CaptureBindProgramPipeline(replayState, true, apiState.getProgramPipeline()->id()));
        }
        else if (replayState.getProgramPipeline())
        {
            cap(CaptureBindProgramPipeline(replayState, true, {0}));
        }
    }

    // Capture Queries
    const gl::QueryMap &queryMap = context->getQueriesForCapture();

    // Create existing queries. Note that queries may be genned and not yet started. In that
    // case the queries will exist in the query map as nullptr entries.
    // If any queries are active between frames, we want to defer creation and do them last,
    // otherwise you'll get GL errors about starting a query while one is already active.
    for (gl::QueryMap::Iterator queryIter = gl::UnsafeResourceMapIter(queryMap).beginWithNull(),
                                endIter   = gl::UnsafeResourceMapIter(queryMap).endWithNull();
         queryIter != endIter; ++queryIter)
    {
        ASSERT(queryIter->first);
        gl::QueryID queryID = {queryIter->first};

        cap(CaptureGenQueries(replayState, true, 1, &queryID));
        MaybeCaptureUpdateResourceIDs(context, resourceTracker, setupCalls);

        gl::Query *query = queryIter->second;
        if (query)
        {
            gl::QueryType queryType = query->getType();

            // Defer active queries until we've created them all
            if (IsQueryActive(apiState, queryID))
            {
                continue;
            }

            // Begin the query to generate the object
            cap(CaptureBeginQuery(replayState, true, queryType, queryID));

            // End the query if it was not active
            cap(CaptureEndQuery(replayState, true, queryType));
        }
    }

    const gl::ActiveQueryMap &activeQueries = apiState.getActiveQueriesForCapture();
    for (const auto &activeQueryIter : activeQueries)
    {
        const gl::Query *activeQuery = activeQueryIter.get();
        if (activeQuery)
        {
            cap(CaptureBeginQuery(replayState, true, activeQuery->getType(), activeQuery->id()));
        }
    }

    // Transform Feedback
    const gl::TransformFeedbackMap &xfbMap = context->getTransformFeedbacksForCapture();
    for (const auto &xfbIter : gl::UnsafeResourceMapIter(xfbMap))
    {
        gl::TransformFeedbackID xfbID = {xfbIter.first};

        // Do not capture the default XFB object.
        if (xfbID.value == 0)
        {
            continue;
        }

        cap(CaptureGenTransformFeedbacks(replayState, true, 1, &xfbID));
        MaybeCaptureUpdateResourceIDs(context, resourceTracker, setupCalls);

        gl::TransformFeedback *xfb = xfbIter.second;
        if (!xfb)
        {
            // The object was never created
            continue;
        }

        // Bind XFB to create the object
        cap(CaptureBindTransformFeedback(replayState, true, GL_TRANSFORM_FEEDBACK, xfbID));

        // Bind the buffers associated with this XFB object
        for (size_t i = 0; i < xfb->getIndexedBufferCount(); ++i)
        {
            const gl::OffsetBindingPointer<gl::Buffer> &xfbBuffer = xfb->getIndexedBuffer(i);

            // Note: Buffers bound with BindBufferBase can be used with BindBuffer
            cap(CaptureBindBufferRange(replayState, true, gl::BufferBinding::TransformFeedback, 0,
                                       xfbBuffer.id(), xfbBuffer.getOffset(), xfbBuffer.getSize()));
        }

        if (xfb->isActive() || xfb->isPaused())
        {
            // We don't support active XFB in MEC yet
            UNIMPLEMENTED();
        }
    }

    // Bind the current XFB buffer after populating XFB objects
    gl::TransformFeedback *currentXFB = apiState.getCurrentTransformFeedback();
    if (currentXFB)
    {
        cap(CaptureBindTransformFeedback(replayState, true, GL_TRANSFORM_FEEDBACK,
                                         currentXFB->id()));
    }

    // Bind samplers
    const gl::SamplerBindingVector &samplerBindings = apiState.getSamplers();
    for (GLuint bindingIndex = 0; bindingIndex < static_cast<GLuint>(samplerBindings.size());
         ++bindingIndex)
    {
        gl::SamplerID samplerID = samplerBindings[bindingIndex].id();
        if (samplerID.value != 0)
        {
            cap(CaptureBindSampler(replayState, true, bindingIndex, samplerID));
        }
    }

    // Capture Image Texture bindings
    const std::vector<gl::ImageUnit> &imageUnits = apiState.getImageUnits();
    for (GLuint bindingIndex = 0; bindingIndex < static_cast<GLuint>(imageUnits.size());
         ++bindingIndex)
    {
        const gl::ImageUnit &imageUnit = imageUnits[bindingIndex];

        if (imageUnit.texture == 0)
        {
            continue;
        }

        cap(CaptureBindImageTexture(replayState, true, bindingIndex, imageUnit.texture.id(),
                                    imageUnit.level, imageUnit.layered, imageUnit.layer,
                                    imageUnit.access, imageUnit.format));
    }

    // Capture GL Context states.
    auto capCap = [cap, &replayState](GLenum capEnum, bool capValue) {
        if (capValue)
        {
            cap(CaptureEnable(replayState, true, capEnum));
        }
        else
        {
            cap(CaptureDisable(replayState, true, capEnum));
        }
    };

    // Capture GLES1 context states.
    if (context->isGLES1())
    {
        const bool currentTextureState = apiState.getEnableFeature(GL_TEXTURE_2D);
        const bool defaultTextureState = replayState.getEnableFeature(GL_TEXTURE_2D);
        if (currentTextureState != defaultTextureState)
        {
            capCap(GL_TEXTURE_2D, currentTextureState);
        }

        cap(CaptureMatrixMode(replayState, true, gl::MatrixType::Projection));
        for (angle::Mat4 projectionMatrix :
             apiState.gles1().getMatrixStack(gl::MatrixType::Projection))
        {
            cap(CapturePushMatrix(replayState, true));
            cap(CaptureLoadMatrixf(replayState, true, projectionMatrix.elements().data()));
        }

        cap(CaptureMatrixMode(replayState, true, gl::MatrixType::Modelview));
        for (angle::Mat4 modelViewMatrix :
             apiState.gles1().getMatrixStack(gl::MatrixType::Modelview))
        {
            cap(CapturePushMatrix(replayState, true));
            cap(CaptureLoadMatrixf(replayState, true, modelViewMatrix.elements().data()));
        }

        gl::MatrixType currentMatrixMode = apiState.gles1().getMatrixMode();
        if (currentMatrixMode != gl::MatrixType::Modelview)
        {
            cap(CaptureMatrixMode(replayState, true, currentMatrixMode));
        }

        // Alpha Test state
        const bool currentAlphaTestState = apiState.getEnableFeature(GL_ALPHA_TEST);
        const bool defaultAlphaTestState = replayState.getEnableFeature(GL_ALPHA_TEST);

        if (currentAlphaTestState != defaultAlphaTestState)
        {
            capCap(GL_ALPHA_TEST, currentAlphaTestState);
        }

        const gl::AlphaTestParameters currentAlphaTestParameters =
            apiState.gles1().getAlphaTestParameters();
        const gl::AlphaTestParameters defaultAlphaTestParameters =
            replayState.gles1().getAlphaTestParameters();

        if (currentAlphaTestParameters != defaultAlphaTestParameters)
        {
            cap(CaptureAlphaFunc(replayState, true, currentAlphaTestParameters.func,
                                 currentAlphaTestParameters.ref));
        }
    }

    // Rasterizer state. Missing ES 3.x features.
    const gl::RasterizerState &defaultRasterState = replayState.getRasterizerState();
    const gl::RasterizerState &currentRasterState = apiState.getRasterizerState();
    if (currentRasterState.cullFace != defaultRasterState.cullFace)
    {
        capCap(GL_CULL_FACE, currentRasterState.cullFace);
    }

    if (currentRasterState.cullMode != defaultRasterState.cullMode)
    {
        cap(CaptureCullFace(replayState, true, currentRasterState.cullMode));
    }

    if (currentRasterState.frontFace != defaultRasterState.frontFace)
    {
        cap(CaptureFrontFace(replayState, true, currentRasterState.frontFace));
    }

    if (currentRasterState.polygonMode != defaultRasterState.polygonMode)
    {
        if (context->getExtensions().polygonModeNV)
        {
            cap(CapturePolygonModeNV(replayState, true, GL_FRONT_AND_BACK,
                                     currentRasterState.polygonMode));
        }
        else if (context->getExtensions().polygonModeANGLE)
        {
            cap(CapturePolygonModeANGLE(replayState, true, GL_FRONT_AND_BACK,
                                        currentRasterState.polygonMode));
        }
        else
        {
            UNREACHABLE();
        }
    }

    if (currentRasterState.polygonOffsetPoint != defaultRasterState.polygonOffsetPoint)
    {
        capCap(GL_POLYGON_OFFSET_POINT_NV, currentRasterState.polygonOffsetPoint);
    }

    if (currentRasterState.polygonOffsetLine != defaultRasterState.polygonOffsetLine)
    {
        capCap(GL_POLYGON_OFFSET_LINE_NV, currentRasterState.polygonOffsetLine);
    }

    if (currentRasterState.polygonOffsetFill != defaultRasterState.polygonOffsetFill)
    {
        capCap(GL_POLYGON_OFFSET_FILL, currentRasterState.polygonOffsetFill);
    }

    if (currentRasterState.polygonOffsetFactor != defaultRasterState.polygonOffsetFactor ||
        currentRasterState.polygonOffsetUnits != defaultRasterState.polygonOffsetUnits ||
        currentRasterState.polygonOffsetClamp != defaultRasterState.polygonOffsetClamp)
    {
        if (currentRasterState.polygonOffsetClamp == 0.0f)
        {
            cap(CapturePolygonOffset(replayState, true, currentRasterState.polygonOffsetFactor,
                                     currentRasterState.polygonOffsetUnits));
        }
        else
        {
            cap(CapturePolygonOffsetClampEXT(
                replayState, true, currentRasterState.polygonOffsetFactor,
                currentRasterState.polygonOffsetUnits, currentRasterState.polygonOffsetClamp));
        }
    }

    if (currentRasterState.depthClamp != defaultRasterState.depthClamp)
    {
        capCap(GL_DEPTH_CLAMP_EXT, currentRasterState.depthClamp);
    }

    // pointDrawMode/multiSample are only used in the D3D back-end right now.

    if (currentRasterState.rasterizerDiscard != defaultRasterState.rasterizerDiscard)
    {
        capCap(GL_RASTERIZER_DISCARD, currentRasterState.rasterizerDiscard);
    }

    if (currentRasterState.dither != defaultRasterState.dither)
    {
        capCap(GL_DITHER, currentRasterState.dither);
    }

    // Depth/stencil state.
    const gl::DepthStencilState &defaultDSState = replayState.getDepthStencilState();
    const gl::DepthStencilState &currentDSState = apiState.getDepthStencilState();
    if (defaultDSState.depthFunc != currentDSState.depthFunc)
    {
        cap(CaptureDepthFunc(replayState, true, currentDSState.depthFunc));
    }

    if (defaultDSState.depthMask != currentDSState.depthMask)
    {
        cap(CaptureDepthMask(replayState, true, gl::ConvertToGLBoolean(currentDSState.depthMask)));
    }

    if (defaultDSState.depthTest != currentDSState.depthTest)
    {
        capCap(GL_DEPTH_TEST, currentDSState.depthTest);
    }

    if (defaultDSState.stencilTest != currentDSState.stencilTest)
    {
        capCap(GL_STENCIL_TEST, currentDSState.stencilTest);
    }

    if (currentDSState.stencilFunc == currentDSState.stencilBackFunc &&
        currentDSState.stencilMask == currentDSState.stencilBackMask)
    {
        // Front and back are equal
        if (defaultDSState.stencilFunc != currentDSState.stencilFunc ||
            defaultDSState.stencilMask != currentDSState.stencilMask ||
            apiState.getStencilRef() != 0)
        {
            cap(CaptureStencilFunc(replayState, true, currentDSState.stencilFunc,
                                   apiState.getStencilRef(), currentDSState.stencilMask));
        }
    }
    else
    {
        // Front and back are separate
        if (defaultDSState.stencilFunc != currentDSState.stencilFunc ||
            defaultDSState.stencilMask != currentDSState.stencilMask ||
            apiState.getStencilRef() != 0)
        {
            cap(CaptureStencilFuncSeparate(replayState, true, GL_FRONT, currentDSState.stencilFunc,
                                           apiState.getStencilRef(), currentDSState.stencilMask));
        }

        if (defaultDSState.stencilBackFunc != currentDSState.stencilBackFunc ||
            defaultDSState.stencilBackMask != currentDSState.stencilBackMask ||
            apiState.getStencilBackRef() != 0)
        {
            cap(CaptureStencilFuncSeparate(
                replayState, true, GL_BACK, currentDSState.stencilBackFunc,
                apiState.getStencilBackRef(), currentDSState.stencilBackMask));
        }
    }

    if (currentDSState.stencilFail == currentDSState.stencilBackFail &&
        currentDSState.stencilPassDepthFail == currentDSState.stencilBackPassDepthFail &&
        currentDSState.stencilPassDepthPass == currentDSState.stencilBackPassDepthPass)
    {
        // Front and back are equal
        if (defaultDSState.stencilFail != currentDSState.stencilFail ||
            defaultDSState.stencilPassDepthFail != currentDSState.stencilPassDepthFail ||
            defaultDSState.stencilPassDepthPass != currentDSState.stencilPassDepthPass)
        {
            cap(CaptureStencilOp(replayState, true, currentDSState.stencilFail,
                                 currentDSState.stencilPassDepthFail,
                                 currentDSState.stencilPassDepthPass));
        }
    }
    else
    {
        // Front and back are separate
        if (defaultDSState.stencilFail != currentDSState.stencilFail ||
            defaultDSState.stencilPassDepthFail != currentDSState.stencilPassDepthFail ||
            defaultDSState.stencilPassDepthPass != currentDSState.stencilPassDepthPass)
        {
            cap(CaptureStencilOpSeparate(replayState, true, GL_FRONT, currentDSState.stencilFail,
                                         currentDSState.stencilPassDepthFail,
                                         currentDSState.stencilPassDepthPass));
        }

        if (defaultDSState.stencilBackFail != currentDSState.stencilBackFail ||
            defaultDSState.stencilBackPassDepthFail != currentDSState.stencilBackPassDepthFail ||
            defaultDSState.stencilBackPassDepthPass != currentDSState.stencilBackPassDepthPass)
        {
            cap(CaptureStencilOpSeparate(replayState, true, GL_BACK, currentDSState.stencilBackFail,
                                         currentDSState.stencilBackPassDepthFail,
                                         currentDSState.stencilBackPassDepthPass));
        }
    }

    if (currentDSState.stencilWritemask == currentDSState.stencilBackWritemask)
    {
        // Front and back are equal
        if (defaultDSState.stencilWritemask != currentDSState.stencilWritemask)
        {
            cap(CaptureStencilMask(replayState, true, currentDSState.stencilWritemask));
        }
    }
    else
    {
        // Front and back are separate
        if (defaultDSState.stencilWritemask != currentDSState.stencilWritemask)
        {
            cap(CaptureStencilMaskSeparate(replayState, true, GL_FRONT,
                                           currentDSState.stencilWritemask));
        }

        if (defaultDSState.stencilBackWritemask != currentDSState.stencilBackWritemask)
        {
            cap(CaptureStencilMaskSeparate(replayState, true, GL_BACK,
                                           currentDSState.stencilBackWritemask));
        }
    }

    // Blend state.
    const gl::BlendState &defaultBlendState = replayState.getBlendState();
    const gl::BlendState &currentBlendState = apiState.getBlendState();

    if (currentBlendState.blend != defaultBlendState.blend)
    {
        capCap(GL_BLEND, currentBlendState.blend);
    }

    if (currentBlendState.sourceBlendRGB != defaultBlendState.sourceBlendRGB ||
        currentBlendState.destBlendRGB != defaultBlendState.destBlendRGB ||
        currentBlendState.sourceBlendAlpha != defaultBlendState.sourceBlendAlpha ||
        currentBlendState.destBlendAlpha != defaultBlendState.destBlendAlpha)
    {
        if (context->isGLES1())
        {
            // Even though their states are tracked independently, in GLES1 blendAlpha
            // and blendRGB cannot be set separately and are always equal
            cap(CaptureBlendFunc(replayState, true, currentBlendState.sourceBlendRGB,
                                 currentBlendState.destBlendRGB));
            Capture(&resetCalls[angle::EntryPoint::GLBlendFunc],
                    CaptureBlendFunc(replayState, true, currentBlendState.sourceBlendRGB,
                                     currentBlendState.destBlendRGB));
        }
        else
        {
            // Always use BlendFuncSeparate for non-GLES1 as it covers all cases
            cap(CaptureBlendFuncSeparate(
                replayState, true, currentBlendState.sourceBlendRGB, currentBlendState.destBlendRGB,
                currentBlendState.sourceBlendAlpha, currentBlendState.destBlendAlpha));
            Capture(&resetCalls[angle::EntryPoint::GLBlendFuncSeparate],
                    CaptureBlendFuncSeparate(replayState, true, currentBlendState.sourceBlendRGB,
                                             currentBlendState.destBlendRGB,
                                             currentBlendState.sourceBlendAlpha,
                                             currentBlendState.destBlendAlpha));
        }
    }

    if (currentBlendState.blendEquationRGB != defaultBlendState.blendEquationRGB ||
        currentBlendState.blendEquationAlpha != defaultBlendState.blendEquationAlpha)
    {
        // Similarly to BlendFunc, using BlendEquation in some cases complicates Reset.
        cap(CaptureBlendEquationSeparate(replayState, true, currentBlendState.blendEquationRGB,
                                         currentBlendState.blendEquationAlpha));
        Capture(&resetCalls[angle::EntryPoint::GLBlendEquationSeparate],
                CaptureBlendEquationSeparate(replayState, true, currentBlendState.blendEquationRGB,
                                             currentBlendState.blendEquationAlpha));
    }

    if (currentBlendState.colorMaskRed != defaultBlendState.colorMaskRed ||
        currentBlendState.colorMaskGreen != defaultBlendState.colorMaskGreen ||
        currentBlendState.colorMaskBlue != defaultBlendState.colorMaskBlue ||
        currentBlendState.colorMaskAlpha != defaultBlendState.colorMaskAlpha)
    {
        cap(CaptureColorMask(replayState, true,
                             gl::ConvertToGLBoolean(currentBlendState.colorMaskRed),
                             gl::ConvertToGLBoolean(currentBlendState.colorMaskGreen),
                             gl::ConvertToGLBoolean(currentBlendState.colorMaskBlue),
                             gl::ConvertToGLBoolean(currentBlendState.colorMaskAlpha)));
        Capture(&resetCalls[angle::EntryPoint::GLColorMask],
                CaptureColorMask(replayState, true,
                                 gl::ConvertToGLBoolean(currentBlendState.colorMaskRed),
                                 gl::ConvertToGLBoolean(currentBlendState.colorMaskGreen),
                                 gl::ConvertToGLBoolean(currentBlendState.colorMaskBlue),
                                 gl::ConvertToGLBoolean(currentBlendState.colorMaskAlpha)));
    }

    const gl::ColorF &currentBlendColor = apiState.getBlendColor();
    if (currentBlendColor != gl::ColorF())
    {
        cap(CaptureBlendColor(replayState, true, currentBlendColor.red, currentBlendColor.green,
                              currentBlendColor.blue, currentBlendColor.alpha));
        Capture(&resetCalls[angle::EntryPoint::GLBlendColor],
                CaptureBlendColor(replayState, true, currentBlendColor.red, currentBlendColor.green,
                                  currentBlendColor.blue, currentBlendColor.alpha));
    }

    // Pixel storage states.
    gl::PixelPackState &currentPackState = replayState.getPackState();
    if (currentPackState.alignment != apiState.getPackAlignment())
    {
        cap(CapturePixelStorei(replayState, true, GL_PACK_ALIGNMENT, apiState.getPackAlignment()));
        currentPackState.alignment = apiState.getPackAlignment();
    }

    if (currentPackState.rowLength != apiState.getPackRowLength())
    {
        cap(CapturePixelStorei(replayState, true, GL_PACK_ROW_LENGTH, apiState.getPackRowLength()));
        currentPackState.rowLength = apiState.getPackRowLength();
    }

    if (currentPackState.skipRows != apiState.getPackSkipRows())
    {
        cap(CapturePixelStorei(replayState, true, GL_PACK_SKIP_ROWS, apiState.getPackSkipRows()));
        currentPackState.skipRows = apiState.getPackSkipRows();
    }

    if (currentPackState.skipPixels != apiState.getPackSkipPixels())
    {
        cap(CapturePixelStorei(replayState, true, GL_PACK_SKIP_PIXELS,
                               apiState.getPackSkipPixels()));
        currentPackState.skipPixels = apiState.getPackSkipPixels();
    }

    // We set unpack alignment above, no need to change it here
    ASSERT(currentUnpackState.alignment == 1);
    if (currentUnpackState.rowLength != apiState.getUnpackRowLength())
    {
        cap(CapturePixelStorei(replayState, true, GL_UNPACK_ROW_LENGTH,
                               apiState.getUnpackRowLength()));
        currentUnpackState.rowLength = apiState.getUnpackRowLength();
    }

    if (currentUnpackState.skipRows != apiState.getUnpackSkipRows())
    {
        cap(CapturePixelStorei(replayState, true, GL_UNPACK_SKIP_ROWS,
                               apiState.getUnpackSkipRows()));
        currentUnpackState.skipRows = apiState.getUnpackSkipRows();
    }

    if (currentUnpackState.skipPixels != apiState.getUnpackSkipPixels())
    {
        cap(CapturePixelStorei(replayState, true, GL_UNPACK_SKIP_PIXELS,
                               apiState.getUnpackSkipPixels()));
        currentUnpackState.skipPixels = apiState.getUnpackSkipPixels();
    }

    if (currentUnpackState.imageHeight != apiState.getUnpackImageHeight())
    {
        cap(CapturePixelStorei(replayState, true, GL_UNPACK_IMAGE_HEIGHT,
                               apiState.getUnpackImageHeight()));
        currentUnpackState.imageHeight = apiState.getUnpackImageHeight();
    }

    if (currentUnpackState.skipImages != apiState.getUnpackSkipImages())
    {
        cap(CapturePixelStorei(replayState, true, GL_UNPACK_SKIP_IMAGES,
                               apiState.getUnpackSkipImages()));
        currentUnpackState.skipImages = apiState.getUnpackSkipImages();
    }

    // Clear state. Missing ES 3.x features.
    // TODO(http://anglebug.com/42262323): Complete state capture.
    const gl::ColorF &currentClearColor = apiState.getColorClearValue();
    if (currentClearColor != gl::ColorF())
    {
        cap(CaptureClearColor(replayState, true, currentClearColor.red, currentClearColor.green,
                              currentClearColor.blue, currentClearColor.alpha));
    }

    if (apiState.getDepthClearValue() != 1.0f)
    {
        cap(CaptureClearDepthf(replayState, true, apiState.getDepthClearValue()));
    }

    if (apiState.getStencilClearValue() != 0)
    {
        cap(CaptureClearStencil(replayState, true, apiState.getStencilClearValue()));
    }

    // Viewport / scissor / clipping planes.
    const gl::Rectangle &currentViewport = apiState.getViewport();
    if (currentViewport != gl::Rectangle())
    {
        cap(CaptureViewport(replayState, true, currentViewport.x, currentViewport.y,
                            currentViewport.width, currentViewport.height));
    }

    if (apiState.getNearPlane() != 0.0f || apiState.getFarPlane() != 1.0f)
    {
        cap(CaptureDepthRangef(replayState, true, apiState.getNearPlane(), apiState.getFarPlane()));
    }

    if (apiState.getClipOrigin() != gl::ClipOrigin::LowerLeft ||
        apiState.getClipDepthMode() != gl::ClipDepthMode::NegativeOneToOne)
    {
        cap(CaptureClipControlEXT(replayState, true, apiState.getClipOrigin(),
                                  apiState.getClipDepthMode()));
    }

    if (apiState.isScissorTestEnabled())
    {
        capCap(GL_SCISSOR_TEST, apiState.isScissorTestEnabled());
    }

    const gl::Rectangle &currentScissor = apiState.getScissor();
    if (currentScissor != gl::Rectangle())
    {
        cap(CaptureScissor(replayState, true, currentScissor.x, currentScissor.y,
                           currentScissor.width, currentScissor.height));
    }

    // Allow the replayState object to be destroyed conveniently.
    replayState.setBufferBinding(context, gl::BufferBinding::Array, nullptr);

    // Clean up the replay state.
    replayState.reset(context);

    GLint contextUnpackAlignment = context->getState().getUnpackState().alignment;
    if (currentUnpackState.alignment != contextUnpackAlignment)
    {
        cap(CapturePixelStorei(replayState, true, GL_UNPACK_ALIGNMENT, contextUnpackAlignment));
        replayState.getMutablePrivateStateForCapture()->setUnpackAlignment(contextUnpackAlignment);
    }

    if (validationEnabled)
    {
        CaptureValidateSerializedState(context, setupCalls);
    }
}

bool SkipCall(EntryPoint entryPoint)
{
    switch (entryPoint)
    {
        case EntryPoint::GLDebugMessageCallback:
        case EntryPoint::GLDebugMessageCallbackKHR:
        case EntryPoint::GLDebugMessageControl:
        case EntryPoint::GLDebugMessageControlKHR:
        case EntryPoint::GLDebugMessageInsert:
        case EntryPoint::GLDebugMessageInsertKHR:
        case EntryPoint::GLGetDebugMessageLog:
        case EntryPoint::GLGetDebugMessageLogKHR:
        case EntryPoint::GLGetObjectLabel:
        case EntryPoint::GLGetObjectLabelEXT:
        case EntryPoint::GLGetObjectLabelKHR:
        case EntryPoint::GLGetObjectPtrLabelKHR:
        case EntryPoint::GLGetPointervKHR:
        case EntryPoint::GLInsertEventMarkerEXT:
        case EntryPoint::GLLabelObjectEXT:
        case EntryPoint::GLObjectLabel:
        case EntryPoint::GLObjectLabelKHR:
        case EntryPoint::GLObjectPtrLabelKHR:
        case EntryPoint::GLPopDebugGroupKHR:
        case EntryPoint::GLPopGroupMarkerEXT:
        case EntryPoint::GLPushDebugGroupKHR:
        case EntryPoint::GLPushGroupMarkerEXT:
            // Purposefully skip entry points from:
            // - KHR_debug
            // - EXT_debug_label
            // - EXT_debug_marker
            // There is no need to capture these for replaying a trace in our harness
            return true;

        case EntryPoint::GLGetActiveUniform:
        case EntryPoint::GLGetActiveUniformsiv:
            // Skip these calls because:
            // - We don't use the return values.
            // - Active uniform counts can vary between platforms due to cross stage optimizations
            //   and asking about uniforms above GL_ACTIVE_UNIFORMS triggers errors.
            return true;

        case EntryPoint::GLGetActiveAttrib:
            // Skip these calls because:
            // - We don't use the return values.
            // - Same as uniforms, the value can vary, asking above GL_ACTIVE_ATTRIBUTES is an error
            return true;

        case EntryPoint::GLGetActiveUniformBlockiv:
        case EntryPoint::GLGetActiveUniformBlockName:
            // Skip these calls because:
            // - We don't use the return values.
            // - It reduces the number of references to the uniform block index map.
            return true;

        case EntryPoint::EGLChooseConfig:
        case EntryPoint::EGLGetProcAddress:
        case EntryPoint::EGLGetConfigAttrib:
        case EntryPoint::EGLGetConfigs:
        case EntryPoint::EGLGetSyncAttrib:
        case EntryPoint::EGLGetSyncAttribKHR:
        case EntryPoint::EGLQueryContext:
        case EntryPoint::EGLQuerySurface:
            // Skip these calls because:
            // - We don't use the return values.
            // - Some EGL types and pointer parameters aren't yet implemented in EGL capture.
            return true;

        case EntryPoint::EGLPrepareSwapBuffersANGLE:
            // Skip this call because:
            // - eglPrepareSwapBuffersANGLE is automatically called by eglSwapBuffers
            return true;

        case EntryPoint::EGLSwapBuffers:
            // Skip these calls because:
            // - Swap is handled specially by the trace harness.
            return true;

        default:
            break;
    }

    return false;
}

std::string GetBaseName(const std::string &nameWithPath)
{
    std::vector<std::string> result = angle::SplitString(
        nameWithPath, "/\\", WhitespaceHandling::TRIM_WHITESPACE, SplitResult::SPLIT_WANT_NONEMPTY);
    ASSERT(!result.empty());
    return result.back();
}
}  // namespace

FrameCapture::FrameCapture()  = default;
FrameCapture::~FrameCapture() = default;

void FrameCapture::reset()
{
    mSetupCalls.clear();
}

FrameCaptureShared::FrameCaptureShared()
    : mEnabled(true),
      mSerializeStateEnabled(false),
      mCompression(true),
      mClientVertexArrayMap{},
      mFrameIndex(1),
      mCaptureStartFrame(1),
      mCaptureEndFrame(0),
      mClientArraySizes{},
      mReadBufferSize(0),
      mResourceIDBufferSize(0),
      mHasResourceType{},
      mResourceIDToSetupCalls{},
      mMaxAccessedResourceIDs{},
      mCaptureTrigger(0),
      mCaptureActive(false),
      mWindowSurfaceContextID({0})
{
    reset();

    std::string enabledFromEnv =
        GetEnvironmentVarOrUnCachedAndroidProperty(kEnabledVarName, kAndroidEnabled);
    if (enabledFromEnv == "0")
    {
        mEnabled = false;
    }

    std::string startFromEnv =
        GetEnvironmentVarOrUnCachedAndroidProperty(kFrameStartVarName, kAndroidFrameStart);
    if (!startFromEnv.empty())
    {
        mCaptureStartFrame = atoi(startFromEnv.c_str());
    }
    if (mCaptureStartFrame < 1)
    {
        WARN() << "Cannot use a capture start frame less than 1.";
        mCaptureStartFrame = 1;
    }

    std::string endFromEnv =
        GetEnvironmentVarOrUnCachedAndroidProperty(kFrameEndVarName, kAndroidFrameEnd);
    if (!endFromEnv.empty())
    {
        mCaptureEndFrame = atoi(endFromEnv.c_str());
    }

    std::string captureTriggerFromEnv =
        GetEnvironmentVarOrUnCachedAndroidProperty(kTriggerVarName, kAndroidTrigger);
    if (!captureTriggerFromEnv.empty())
    {
        mCaptureTrigger = atoi(captureTriggerFromEnv.c_str());

        // If the trigger has been populated, ignore the other frame range variables by setting them
        // to unreasonable values. This isn't perfect, but it is effective.
        mCaptureStartFrame = mCaptureEndFrame = std::numeric_limits<uint32_t>::max();
        INFO() << "Capture trigger detected, disabling capture start/end frame.";
    }

    std::string labelFromEnv =
        GetEnvironmentVarOrUnCachedAndroidProperty(kCaptureLabelVarName, kAndroidCaptureLabel);
    // --angle-per-test-capture-label sets the env var, not properties
    if (labelFromEnv.empty())
    {
        labelFromEnv = GetEnvironmentVar(kCaptureLabelVarName);
    }
    if (!labelFromEnv.empty())
    {
        // Optional label to provide unique file names and namespaces
        mCaptureLabel = labelFromEnv;
    }

    std::string compressionFromEnv =
        GetEnvironmentVarOrUnCachedAndroidProperty(kCompressionVarName, kAndroidCompression);
    if (compressionFromEnv == "0")
    {
        mCompression = false;
    }
    std::string serializeStateFromEnv = angle::GetEnvironmentVar(kSerializeStateVarName);
    if (serializeStateFromEnv == "1")
    {
        mSerializeStateEnabled = true;
    }

    std::string validateSerialiedStateFromEnv =
        GetEnvironmentVarOrUnCachedAndroidProperty(kValidationVarName, kAndroidValidation);
    if (validateSerialiedStateFromEnv == "1")
    {
        mValidateSerializedState = true;
    }

    mValidationExpression =
        GetEnvironmentVarOrUnCachedAndroidProperty(kValidationExprVarName, kAndroidValidationExpr);

    if (!mValidationExpression.empty())
    {
        INFO() << "Validation expression is " << kValidationExprVarName;
    }

    // TODO: Remove. http://anglebug.com/42266223
    std::string sourceExtFromEnv =
        GetEnvironmentVarOrUnCachedAndroidProperty(kSourceExtVarName, kAndroidSourceExt);
    if (!sourceExtFromEnv.empty())
    {
        if (sourceExtFromEnv == "c" || sourceExtFromEnv == "cpp")
        {
            mReplayWriter.setSourceFileExtension(sourceExtFromEnv.c_str());
        }
        else
        {
            WARN() << "Invalid capture source extension: " << sourceExtFromEnv;
        }
    }

    std::string sourceSizeFromEnv =
        GetEnvironmentVarOrUnCachedAndroidProperty(kSourceSizeVarName, kAndroidSourceSize);
    if (!sourceSizeFromEnv.empty())
    {
        int sourceSize = atoi(sourceSizeFromEnv.c_str());
        if (sourceSize < 0)
        {
            WARN() << "Invalid capture source size: " << sourceSize;
        }
        else
        {
            mReplayWriter.setSourceFileSizeThreshold(sourceSize);
        }
    }

    std::string forceShadowFromEnv =
        GetEnvironmentVarOrUnCachedAndroidProperty(kForceShadowVarName, kAndroidForceShadow);
    if (forceShadowFromEnv == "1")
    {
        INFO() << "Force enabling shadow memory for coherent buffer tracking.";
        mCoherentBufferTracker.enableShadowMemory();
    }

    if (mFrameIndex == mCaptureStartFrame)
    {
        // Capture is starting from the first frame, so set the capture active to ensure all GLES
        // commands issued are handled correctly by maybeCapturePreCallUpdates() and
        // maybeCapturePostCallUpdates().
        setCaptureActive();
    }

    if (mCaptureEndFrame < mCaptureStartFrame)
    {
        // If we're still in a situation where start frame is after end frame,
        // capture cannot happen. Consider this a disabled state.
        // Note: We won't get here if trigger is in use, as it sets them equal but huge.
        mEnabled = false;
    }

    mReplayWriter.setCaptureLabel(mCaptureLabel);

    // Special case the output directory
    if (mEnabled)
    {
        // Only perform output directory checks if enabled
        // - This can avoid some expensive process name and filesystem checks
        // - We want to emit errors if the directory doesn't exist
        std::string pathFromEnv =
            GetEnvironmentVarOrUnCachedAndroidProperty(kOutDirectoryVarName, kAndroidOutDir);
        if (pathFromEnv.empty())
        {
            mOutDirectory = GetDefaultOutDirectory();
        }
        else
        {
            mOutDirectory = pathFromEnv;
        }

        // Ensure the capture path ends with a slash.
        if (mOutDirectory.back() != '\\' && mOutDirectory.back() != '/')
        {
            mOutDirectory += '/';
        }
    }
}

FrameCaptureShared::~FrameCaptureShared() = default;

PageRange::PageRange(size_t start, size_t end) : start(start), end(end) {}
PageRange::~PageRange() = default;

AddressRange::AddressRange() {}
AddressRange::AddressRange(uintptr_t start, size_t size) : start(start), size(size) {}
AddressRange::~AddressRange() = default;

uintptr_t AddressRange::end()
{
    return start + size;
}

bool IsTrackedPerContext(ResourceIDType type)
{
    // This helper function informs us which context-local (not shared) objects are tracked
    // with per-context object maps.
    if (IsSharedObjectResource(type))
    {
        return false;
    }

    // TODO (https://issuetracker.google.com/169868803): Remaining context-local resources (VAOs,
    // PPOs, Transform Feedback Objects, and Query Objects) must also tracked per-context. Once all
    // per-context resource handling is correctly updated then this function can be replaced with
    // !IsSharedObjectResource().
    switch (type)
    {
        case ResourceIDType::Framebuffer:
            return true;

        default:
            return false;
    }
}

CoherentBuffer::CoherentBuffer(uintptr_t start,
                               size_t size,
                               size_t pageSize,
                               bool isShadowMemoryEnabled)
    : mPageSize(pageSize),
      mShadowMemoryEnabled(isShadowMemoryEnabled),
      mBufferStart(start),
      mShadowMemory(nullptr),
      mShadowDirty(false)
{
    if (mShadowMemoryEnabled)
    {
        // Shadow memory needs to have at least the size of one page, to not protect outside.
        size_t numShadowPages = (size / pageSize) + 1;
        mShadowMemory         = AlignedAlloc(numShadowPages * pageSize, pageSize);
        ASSERT(mShadowMemory != nullptr);
        start = reinterpret_cast<uintptr_t>(mShadowMemory);
    }

    mRange.start           = start;
    mRange.size            = size;
    mProtectionRange.start = rx::roundDownPow2(start, pageSize);

    uintptr_t protectionEnd = rx::roundUpPow2(start + size, pageSize);

    mProtectionRange.size = protectionEnd - mProtectionRange.start;
    mPageCount            = mProtectionRange.size / pageSize;

    mProtectionStartPage = mProtectionRange.start / mPageSize;
    mProtectionEndPage   = mProtectionStartPage + mPageCount;

    mDirtyPages = std::vector<bool>(mPageCount);
    mDirtyPages.assign(mPageCount, true);
}

std::vector<PageRange> CoherentBuffer::getDirtyPageRanges()
{
    std::vector<PageRange> dirtyPageRanges;

    bool inDirty = false;
    for (size_t i = 0; i < mPageCount; i++)
    {
        if (!inDirty && mDirtyPages[i])
        {
            // Found start of a dirty range
            inDirty = true;
            // Set end page as last page initially
            dirtyPageRanges.push_back(PageRange(i, mPageCount));
        }
        else if (inDirty && !mDirtyPages[i])
        {
            // Found end of a dirty range
            inDirty                    = false;
            dirtyPageRanges.back().end = i;
        }
    }

    return dirtyPageRanges;
}

AddressRange CoherentBuffer::getRange()
{
    return mRange;
}

AddressRange CoherentBuffer::getDirtyAddressRange(const PageRange &dirtyPageRange)
{
    AddressRange range;

    if (dirtyPageRange.start == 0)
    {
        // First page, use non page aligned buffer start.
        range.start = mRange.start;
    }
    else
    {
        range.start = mProtectionRange.start + dirtyPageRange.start * mPageSize;
    }

    if (dirtyPageRange.end == mPageCount)
    {
        // Last page, use non page aligned buffer end.
        range.size = mRange.end() - range.start;
    }
    else
    {
        range.size = (dirtyPageRange.end - dirtyPageRange.start) * mPageSize;
        // This occurs when a buffer occupies 2 pages, but is smaller than a page.
        if (mRange.end() < range.end())
        {
            range.size = mRange.end() - range.start;
        }
    }

    // Dirty range must be in buffer
    ASSERT(range.start >= mRange.start && mRange.end() >= range.end());

    return range;
}

CoherentBuffer::~CoherentBuffer()
{
    if (mShadowMemory != nullptr)
    {
        AlignedFree(mShadowMemory);
    }
}

bool CoherentBuffer::isDirty()
{
    return std::find(mDirtyPages.begin(), mDirtyPages.end(), true) != mDirtyPages.end();
}

bool CoherentBuffer::contains(size_t page, size_t *relativePage)
{
    bool isInProtectionRange = page >= mProtectionStartPage && page < mProtectionEndPage;
    if (!isInProtectionRange)
    {
        return false;
    }

    *relativePage = page - mProtectionStartPage;

    ASSERT(page >= mProtectionStartPage);

    return true;
}

void CoherentBuffer::protectPageRange(const PageRange &pageRange)
{
    for (size_t i = pageRange.start; i < pageRange.end; i++)
    {
        setDirty(i, false);
    }
}

void CoherentBuffer::protectAll()
{
    for (size_t i = 0; i < mPageCount; i++)
    {
        setDirty(i, false);
    }
}

void CoherentBuffer::updateBufferMemory()
{
    memcpy(reinterpret_cast<void *>(mBufferStart), reinterpret_cast<void *>(mRange.start),
           mRange.size);
}

void CoherentBuffer::updateShadowMemory()
{
    memcpy(reinterpret_cast<void *>(mRange.start), reinterpret_cast<void *>(mBufferStart),
           mRange.size);
    mShadowDirty = false;
}

void CoherentBuffer::setDirty(size_t relativePage, bool dirty)
{
    if (mDirtyPages[relativePage] == dirty)
    {
        // The page is already set.
        // This can happen when tracked buffers overlap in a page.
        return;
    }

    uintptr_t pageStart = mProtectionRange.start + relativePage * mPageSize;

    // Last page end must be the same as protection end
    if (relativePage + 1 == mPageCount)
    {
        ASSERT(mProtectionRange.end() == pageStart + mPageSize);
    }

    bool ret;
    if (dirty)
    {
        ret = UnprotectMemory(pageStart, mPageSize);
    }
    else
    {
        ret = ProtectMemory(pageStart, mPageSize);
    }

    if (!ret)
    {
        ERR() << "Could not set protection for buffer page " << relativePage << " at "
              << reinterpret_cast<void *>(pageStart) << " with size " << mPageSize;
    }
    mDirtyPages[relativePage] = dirty;
}

void CoherentBuffer::removeProtection(PageSharingType sharingType)
{
    uintptr_t start = mProtectionRange.start;
    size_t size     = mProtectionRange.size;

    switch (sharingType)
    {
        case PageSharingType::FirstShared:
        case PageSharingType::FirstAndLastShared:
            start += mPageSize;
            break;
        default:
            break;
    }

    switch (sharingType)
    {
        case PageSharingType::FirstShared:
        case PageSharingType::LastShared:
            size -= mPageSize;
            break;
        case PageSharingType::FirstAndLastShared:
            size -= (2 * mPageSize);
            break;
        default:
            break;
    }

    if (size == 0)
    {
        return;
    }

    if (!UnprotectMemory(start, size))
    {
        ERR() << "Could not remove protection for buffer at " << start << " with size " << size;
    }
}

bool CoherentBufferTracker::canProtectDirectly(gl::Context *context)
{
    gl::BufferID bufferId = context->createBuffer();

    gl::BufferBinding targetPacked = gl::BufferBinding::Array;
    context->bindBuffer(targetPacked, bufferId);

    // Allocate 2 pages so we will always have a full aligned page to protect
    GLsizei size = static_cast<GLsizei>(mPageSize * 2);

    context->bufferStorage(targetPacked, size, nullptr,
                           GL_DYNAMIC_STORAGE_BIT_EXT | GL_MAP_WRITE_BIT |
                               GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

    gl::Buffer *buffer = context->getBuffer(bufferId);

    angle::Result result = buffer->mapRange(
        context, 0, size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
    if (result != angle::Result::Continue)
    {
        ERR() << "Failed to mapRange of buffer.";
    }

    void *map = buffer->getMapPointer();
    if (map == nullptr)
    {
        ERR() << "Failed to getMapPointer of buffer.";
    }

    // Test mprotect
    auto start = reinterpret_cast<uintptr_t>(map);

    // Only protect a whole page inside the allocated memory
    uintptr_t protectionStart = rx::roundUpPow2(start, mPageSize);
    uintptr_t protectionEnd   = protectionStart + mPageSize;

    ASSERT(protectionStart < protectionEnd);

    angle::PageFaultCallback callback = [](uintptr_t address) {
        return angle::PageFaultHandlerRangeType::InRange;
    };

    std::unique_ptr<angle::PageFaultHandler> handler(CreatePageFaultHandler(callback));

    if (!handler->enable())
    {
        GLboolean unmapResult;
        if (buffer->unmap(context, &unmapResult) != angle::Result::Continue)
        {
            ERR() << "Could not unmap buffer.";
        }
        context->bindBuffer(targetPacked, {0});

        // Page fault handler could not be enabled, memory can't be protected directly.
        return false;
    }

    size_t protectionSize = protectionEnd - protectionStart;

    ASSERT(protectionSize == mPageSize);

    bool canProtect = angle::ProtectMemory(protectionStart, protectionSize);
    if (canProtect)
    {
        angle::UnprotectMemory(protectionStart, protectionSize);
    }

    // Clean up
    handler->disable();

    GLboolean unmapResult;
    if (buffer->unmap(context, &unmapResult) != angle::Result::Continue)
    {
        ERR() << "Could not unmap buffer.";
    }
    context->bindBuffer(targetPacked, {0});
    context->deleteBuffer(buffer->id());

    return canProtect;
}

CoherentBufferTracker::CoherentBufferTracker() : mEnabled(false), mShadowMemoryEnabled(false)
{
    mPageSize = GetPageSize();
}

CoherentBufferTracker::~CoherentBufferTracker()
{
    disable();
}

PageFaultHandlerRangeType CoherentBufferTracker::handleWrite(uintptr_t address)
{
    std::lock_guard<angle::SimpleMutex> lock(mMutex);
    auto pagesInBuffers = getBufferPagesForAddress(address);

    if (pagesInBuffers.empty())
    {
        ERR() << "Didn't find a tracked buffer containing " << reinterpret_cast<void *>(address);
    }

    for (const auto &page : pagesInBuffers)
    {
        std::shared_ptr<CoherentBuffer> buffer = page.first;
        size_t relativePage                    = page.second;
        buffer->setDirty(relativePage, true);
    }

    return pagesInBuffers.empty() ? PageFaultHandlerRangeType::OutOfRange
                                  : PageFaultHandlerRangeType::InRange;
}

HashMap<std::shared_ptr<CoherentBuffer>, size_t> CoherentBufferTracker::getBufferPagesForAddress(
    uintptr_t address)
{
    HashMap<std::shared_ptr<CoherentBuffer>, size_t> foundPages;

#if defined(ANGLE_PLATFORM_ANDROID)
    size_t page;
    if (mShadowMemoryEnabled)
    {
        // Starting with Android 11 heap pointers get a tag which is stripped by the POSIX mprotect
        // callback. We need to add this tag manually to the untagged pointer in order to determine
        // the corresponding page.
        // See: https://source.android.com/docs/security/test/tagged-pointers
        // TODO(http://anglebug.com/42265874): Determine when heap pointer tagging is not enabled.
        constexpr unsigned long long POINTER_TAG = 0xb400000000000000;
        unsigned long long taggedAddress         = address | POINTER_TAG;
        page                                     = static_cast<size_t>(taggedAddress / mPageSize);
    }
    else
    {
        // VMA allocated memory pointers are not tagged.
        page = address / mPageSize;
    }
#else
    size_t page = address / mPageSize;
#endif

    for (const auto &pair : mBuffers)
    {
        std::shared_ptr<CoherentBuffer> buffer = pair.second;
        size_t relativePage;
        if (buffer->contains(page, &relativePage))
        {
            foundPages.insert(std::make_pair(buffer, relativePage));
        }
    }

    return foundPages;
}

bool CoherentBufferTracker::isDirty(gl::BufferID id)
{
    return mBuffers[id.value]->isDirty();
}

void CoherentBufferTracker::enable()
{
    if (mEnabled)
    {
        return;
    }

    PageFaultCallback callback = [this](uintptr_t address) { return handleWrite(address); };

    // This needs to be initialized after canProtectDirectly ran and can only be initialized once.
    if (!mPageFaultHandler)
    {
        mPageFaultHandler = std::unique_ptr<PageFaultHandler>(CreatePageFaultHandler(callback));
    }

    bool ret = mPageFaultHandler->enable();
    if (ret)
    {
        mEnabled = true;
    }
    else
    {
        ERR() << "Could not enable page fault handler.";
    }
}

bool CoherentBufferTracker::haveBuffer(gl::BufferID id)
{
    return mBuffers.find(id.value) != mBuffers.end();
}

void CoherentBufferTracker::onEndFrame()
{
    std::lock_guard<angle::SimpleMutex> lock(mMutex);

    if (!mEnabled)
    {
        return;
    }

    // Remove protection from all buffers
    for (const auto &pair : mBuffers)
    {
        std::shared_ptr<CoherentBuffer> buffer = pair.second;
        buffer->removeProtection(PageSharingType::NoneShared);
    }

    disable();
}

void CoherentBufferTracker::disable()
{
    if (!mEnabled)
    {
        return;
    }

    if (mPageFaultHandler->disable())
    {
        mEnabled = false;
    }
    else
    {
        ERR() << "Could not disable page fault handler.";
    }

    if (mShadowMemoryEnabled && mBuffers.size() > 0)
    {
        WARN() << "Disabling coherent buffer tracking while leaving shadow memory without "
                  "synchronization. Expect rendering artifacts after capture ends.";
    }
}

uintptr_t CoherentBufferTracker::addBuffer(gl::BufferID id, uintptr_t start, size_t size)
{
    std::lock_guard<angle::SimpleMutex> lock(mMutex);

    if (haveBuffer(id))
    {
        auto buffer = mBuffers[id.value];
        return buffer->getRange().start;
    }

    auto buffer = std::make_shared<CoherentBuffer>(start, size, mPageSize, mShadowMemoryEnabled);
    uintptr_t realOrShadowStart = buffer->getRange().start;

    mBuffers.insert(std::make_pair(id.value, std::move(buffer)));

    return realOrShadowStart;
}

void CoherentBufferTracker::maybeUpdateShadowMemory()
{
    for (const auto &pair : mBuffers)
    {
        std::shared_ptr<CoherentBuffer> cb = pair.second;
        if (cb->isShadowDirty())
        {
            cb->removeProtection(PageSharingType::NoneShared);
            cb->updateShadowMemory();
            cb->protectAll();
        }
    }
}

void CoherentBufferTracker::markAllShadowDirty()
{
    for (const auto &pair : mBuffers)
    {
        std::shared_ptr<CoherentBuffer> cb = pair.second;
        cb->markShadowDirty();
    }
}

PageSharingType CoherentBufferTracker::doesBufferSharePage(gl::BufferID id)
{
    bool firstPageShared = false;
    bool lastPageShared  = false;

    std::shared_ptr<CoherentBuffer> buffer = mBuffers[id.value];

    AddressRange range = buffer->getRange();

    size_t firstPage = range.start / mPageSize;
    size_t lastPage  = range.end() / mPageSize;

    for (const auto &pair : mBuffers)
    {
        gl::BufferID otherId = {pair.first};
        if (otherId != id)
        {
            std::shared_ptr<CoherentBuffer> otherBuffer = pair.second;
            size_t relativePage;
            if (otherBuffer->contains(firstPage, &relativePage))
            {
                firstPageShared = true;
            }
            else if (otherBuffer->contains(lastPage, &relativePage))
            {
                lastPageShared = true;
            }
        }
    }

    if (firstPageShared && !lastPageShared)
    {
        return PageSharingType::FirstShared;
    }
    else if (!firstPageShared && lastPageShared)
    {
        return PageSharingType::LastShared;
    }
    else if (firstPageShared && lastPageShared)
    {
        return PageSharingType::FirstAndLastShared;
    }
    else
    {
        return PageSharingType::NoneShared;
    }
}

void CoherentBufferTracker::removeBuffer(gl::BufferID id)
{
    std::lock_guard<angle::SimpleMutex> lock(mMutex);

    if (!haveBuffer(id))
    {
        return;
    }

    // Synchronize graphics buffer memory before the buffer is removed from the tracker.
    if (mShadowMemoryEnabled)
    {
        mBuffers[id.value]->updateBufferMemory();
    }

    // If the buffer shares pages with other tracked buffers,
    // don't unprotect the overlapping pages.
    PageSharingType sharingType = doesBufferSharePage(id);
    mBuffers[id.value]->removeProtection(sharingType);
    mBuffers.erase(id.value);
}

void *FrameCaptureShared::maybeGetShadowMemoryPointer(gl::Buffer *buffer,
                                                      GLsizeiptr length,
                                                      GLbitfield access)
{
    if (!(access & GL_MAP_COHERENT_BIT_EXT) || !mCoherentBufferTracker.isShadowMemoryEnabled())
    {
        return buffer->getMapPointer();
    }

    mCoherentBufferTracker.enable();
    uintptr_t realMapPointer = reinterpret_cast<uintptr_t>(buffer->getMapPointer());
    return (void *)mCoherentBufferTracker.addBuffer(buffer->id(), realMapPointer, length);
}

void FrameCaptureShared::determineMemoryProtectionSupport(gl::Context *context)
{
    // Skip this test if shadow memory was force enabled or shadow memory requirement was detected
    // previously
    if (mCoherentBufferTracker.isShadowMemoryEnabled())
    {
        return;
    }

    // These known devices must use shadow memory
    HashMap<std::string, std::vector<std::string>> denyList = {
        {"Google", {"Pixel 6", "Pixel 6 Pro", "Pixel 6a", "Pixel 7", "Pixel 7 Pro"}},
    };

    angle::SystemInfo info;
    angle::GetSystemInfo(&info);
    bool isDeviceDenyListed = false;

    if (rx::GetAndroidSDKVersion() < 34)
    {
        // Before Android 14, there was a bug in Mali based Pixel preventing mprotect
        // on Vulkan surfaces. (https://b.corp.google.com/issues/269535398)
        // Check the denylist in this case.
        if (denyList.find(info.machineManufacturer) != denyList.end())
        {
            const std::vector<std::string> &models = denyList[info.machineManufacturer];
            isDeviceDenyListed =
                std::find(models.begin(), models.end(), info.machineModelName) != models.end();
        }
    }

    if (isDeviceDenyListed)
    {
        WARN() << "Direct memory protection not possible on deny listed device '"
               << info.machineModelName
               << "', enabling shadow memory for coherent buffer tracking.";
        mCoherentBufferTracker.enableShadowMemory();
    }
    else
    {
        // Device is not on deny listed. Run a test if we actually can protect directly. Do this
        // only on assertion enabled builds.
        ASSERT(mCoherentBufferTracker.canProtectDirectly(context));
    }
}

void FrameCaptureShared::trackBufferMapping(const gl::Context *context,
                                            CallCapture *call,
                                            gl::BufferID id,
                                            gl::Buffer *buffer,
                                            GLintptr offset,
                                            GLsizeiptr length,
                                            bool writable,
                                            bool coherent)
{
    // Track that the buffer was mapped
    mResourceTracker.setBufferMapped(context->id(), id.value);

    if (writable)
    {
        // If this buffer was mapped writable, we don't have any visibility into what
        // happens to it. Therefore, remember the details about it, and we'll read it back
        // on Unmap to repopulate it during replay.
        mBufferDataMap[id] = std::make_pair(offset, length);

        // Track that this buffer was potentially modified
        mResourceTracker.getTrackedResource(context->id(), ResourceIDType::Buffer)
            .setModifiedResource(id.value);

        // Track coherent buffer
        // Check if capture is active to not initialize the coherent buffer tracker on the
        // first coherent glMapBufferRange call.
        if (coherent && isCaptureActive())
        {
            mCoherentBufferTracker.enable();
            // When not using shadow memory, adding buffers to the tracking happens here instead of
            // during mapping
            if (!mCoherentBufferTracker.isShadowMemoryEnabled())
            {
                uintptr_t data = reinterpret_cast<uintptr_t>(buffer->getMapPointer());
                mCoherentBufferTracker.addBuffer(id, data, length);
            }
        }
    }
}

void FrameCaptureShared::trackTextureUpdate(const gl::Context *context, const CallCapture &call)
{
    int index             = 0;
    std::string paramName = "targetPacked";
    ParamType paramType   = ParamType::TTextureTarget;

    // Some calls provide the textureID directly
    // For the rest, look it up based on the currently bound texture
    switch (call.entryPoint)
    {
        case EntryPoint::GLCompressedCopyTextureCHROMIUM:
            index     = 1;
            paramName = "destIdPacked";
            paramType = ParamType::TTextureID;
            break;
        case EntryPoint::GLCopyTextureCHROMIUM:
        case EntryPoint::GLCopySubTextureCHROMIUM:
        case EntryPoint::GLCopyTexture3DANGLE:
            index     = 3;
            paramName = "destIdPacked";
            paramType = ParamType::TTextureID;
            break;
        case EntryPoint::GLCopyImageSubData:
        case EntryPoint::GLCopyImageSubDataEXT:
        case EntryPoint::GLCopyImageSubDataOES:
            index     = 7;
            paramName = "dstTarget";
            paramType = ParamType::TGLenum;
            break;
        default:
            break;
    }

    GLuint id = 0;
    switch (paramType)
    {
        case ParamType::TTextureTarget:
        {
            gl::TextureTarget targetPacked =
                call.params.getParam(paramName.c_str(), ParamType::TTextureTarget, index)
                    .value.TextureTargetVal;
            gl::TextureType textureType = gl::TextureTargetToType(targetPacked);
            gl::Texture *texture        = context->getState().getTargetTexture(textureType);
            id                          = texture->id().value;
            break;
        }
        case ParamType::TTextureID:
        {
            gl::TextureID destIDPacked =
                call.params.getParam(paramName.c_str(), ParamType::TTextureID, index)
                    .value.TextureIDVal;
            id = destIDPacked.value;
            break;
        }
        case ParamType::TGLenum:
        {
            GLenum target =
                call.params.getParam(paramName.c_str(), ParamType::TGLenum, index).value.GLenumVal;

            if (target == GL_TEXTURE_CUBE_MAP)
            {
                // CopyImageSubData doesn't support cube faces, but PackedParams requires one
                target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            }

            gl::TextureTarget targetPacked = gl::PackParam<gl::TextureTarget>(target);
            gl::TextureType textureType    = gl::TextureTargetToType(targetPacked);
            gl::Texture *texture           = context->getState().getTargetTexture(textureType);
            id                             = texture->id().value;
            break;
        }
        default:
            ERR() << "Unhandled paramType= " << static_cast<int>(paramType);
            UNREACHABLE();
            break;
    }

    // Mark it as modified
    mResourceTracker.getTrackedResource(context->id(), ResourceIDType::Texture)
        .setModifiedResource(id);
}

// Identify and mark writeable shader image textures as modified
void FrameCaptureShared::trackImageUpdate(const gl::Context *context, const CallCapture &call)
{
    const gl::ProgramExecutable *executable = context->getState().getProgramExecutable();
    for (const gl::ImageBinding &imageBinding : executable->getImageBindings())
    {
        for (GLuint binding : imageBinding.boundImageUnits)
        {
            const gl::ImageUnit &imageUnit = context->getState().getImageUnit(binding);
            if (imageUnit.access != GL_READ_ONLY)
            {
                // Get image binding texture id and mark it as modified
                GLuint id = imageUnit.texture.id().value;
                mResourceTracker.getTrackedResource(context->id(), ResourceIDType::Texture)
                    .setModifiedResource(id);
            }
        }
    }
}

void FrameCaptureShared::trackDefaultUniformUpdate(const gl::Context *context,
                                                   const CallCapture &call)
{
    DefaultUniformType defaultUniformType = GetDefaultUniformType(call);

    GLuint programID = 0;
    int location     = 0;

    // We track default uniform updates by program and location, so look them up in parameters
    if (defaultUniformType == DefaultUniformType::CurrentProgram)
    {
        programID = context->getActiveLinkedProgram()->id().value;

        location = call.params.getParam("locationPacked", ParamType::TUniformLocation, 0)
                       .value.UniformLocationVal.value;
    }
    else
    {
        ASSERT(defaultUniformType == DefaultUniformType::SpecifiedProgram);

        programID = call.params.getParam("programPacked", ParamType::TShaderProgramID, 0)
                        .value.ShaderProgramIDVal.value;

        location = call.params.getParam("locationPacked", ParamType::TUniformLocation, 1)
                       .value.UniformLocationVal.value;
    }

    const TrackedResource &trackedShaderProgram =
        mResourceTracker.getTrackedResource(context->id(), ResourceIDType::ShaderProgram);
    const ResourceSet &startingPrograms = trackedShaderProgram.getStartingResources();
    const ResourceSet &programsToRegen  = trackedShaderProgram.getResourcesToRegen();

    // If this program was in our starting set, track its uniform updates. Unless it was deleted,
    // then its uniforms will all be regenned along wih with the program.
    if (startingPrograms.find(programID) != startingPrograms.end() &&
        programsToRegen.find(programID) == programsToRegen.end())
    {
        // Track that we need to set this default uniform value again
        mResourceTracker.setModifiedDefaultUniform({programID}, {location});
    }
}

void FrameCaptureShared::trackVertexArrayUpdate(const gl::Context *context, const CallCapture &call)
{
    // Look up the currently bound vertex array
    gl::VertexArrayID id = context->getState().getVertexArray()->id();

    // Mark it as modified
    mResourceTracker.getTrackedResource(context->id(), ResourceIDType::VertexArray)
        .setModifiedResource(id.value);
}

void FrameCaptureShared::updateCopyImageSubData(CallCapture &call)
{
    // This call modifies srcName and dstName to no longer be object IDs (GLuint), but actual
    // packed types that can remapped using gTextureMap and gRenderbufferMap

    GLint srcName    = call.params.getParam("srcName", ParamType::TGLuint, 0).value.GLuintVal;
    GLenum srcTarget = call.params.getParam("srcTarget", ParamType::TGLenum, 1).value.GLenumVal;
    switch (srcTarget)
    {
        case GL_RENDERBUFFER:
        {
            // Convert the GLuint to RenderbufferID
            gl::RenderbufferID srcRenderbufferID = {static_cast<GLuint>(srcName)};
            call.params.setValueParamAtIndex("srcName", ParamType::TRenderbufferID,
                                             srcRenderbufferID, 0);
            break;
        }
        case GL_TEXTURE_2D:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_EXTERNAL_OES:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        {
            // Convert the GLuint to TextureID
            gl::TextureID srcTextureID = {static_cast<GLuint>(srcName)};
            call.params.setValueParamAtIndex("srcName", ParamType::TTextureID, srcTextureID, 0);
            break;
        }
        default:
            ERR() << "Unhandled srcTarget = " << srcTarget;
            UNREACHABLE();
            break;
    }

    // Change dstName to the appropriate type based on dstTarget
    GLint dstName    = call.params.getParam("dstName", ParamType::TGLuint, 6).value.GLuintVal;
    GLenum dstTarget = call.params.getParam("dstTarget", ParamType::TGLenum, 7).value.GLenumVal;
    switch (dstTarget)
    {
        case GL_RENDERBUFFER:
        {
            // Convert the GLuint to RenderbufferID
            gl::RenderbufferID dstRenderbufferID = {static_cast<GLuint>(dstName)};
            call.params.setValueParamAtIndex("dstName", ParamType::TRenderbufferID,
                                             dstRenderbufferID, 6);
            break;
        }
        case GL_TEXTURE_2D:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_EXTERNAL_OES:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        {
            // Convert the GLuint to TextureID
            gl::TextureID dstTextureID = {static_cast<GLuint>(dstName)};
            call.params.setValueParamAtIndex("dstName", ParamType::TTextureID, dstTextureID, 6);
            break;
        }
        default:
            ERR() << "Unhandled dstTarget = " << dstTarget;
            UNREACHABLE();
            break;
    }
}

void FrameCaptureShared::overrideProgramBinary(const gl::Context *context,
                                               CallCapture &inCall,
                                               std::vector<CallCapture> &outCalls)
{
    // Program binaries are inherently non-portable, even between two ANGLE builds.
    // If an application is using glProgramBinary in the middle of a trace, we need to replace
    // those calls with an equivalent sequence of portable calls.
    //
    // For example, here is a sequence an app could use for glProgramBinary:
    //
    //   gShaderProgramMap[42] = glCreateProgram();
    //   glProgramBinary(gShaderProgramMap[42], GL_PROGRAM_BINARY_ANGLE, gBinaryData[x], 1000);
    //   glGetProgramiv(gShaderProgramMap[42], GL_LINK_STATUS, gReadBuffer);
    //   glGetProgramiv(gShaderProgramMap[42], GL_PROGRAM_BINARY_LENGTH, gReadBuffer);
    //
    // With this override, the glProgramBinary call will be replaced like so:
    //
    //   gShaderProgramMap[42] = glCreateProgram();
    //   === Begin override ===
    //   gShaderProgramMap[43] = glCreateShader(GL_VERTEX_SHADER);
    //   glShaderSource(gShaderProgramMap[43], 1, string_0, &gBinaryData[100]);
    //   glCompileShader(gShaderProgramMap[43]);
    //   glAttachShader(gShaderProgramMap[42], gShaderProgramMap[43]);
    //   glDeleteShader(gShaderProgramMap[43]);
    //   gShaderProgramMap[43] = glCreateShader(GL_FRAGMENT_SHADER);
    //   glShaderSource(gShaderProgramMap[43], 1, string_1, &gBinaryData[200]);
    //   glCompileShader(gShaderProgramMap[43]);
    //   glAttachShader(gShaderProgramMap[42], gShaderProgramMap[43]);
    //   glDeleteShader(gShaderProgramMap[43]);
    //   glBindAttribLocation(gShaderProgramMap[42], 0, "attrib1");
    //   glBindAttribLocation(gShaderProgramMap[42], 1, "attrib2");
    //   glLinkProgram(gShaderProgramMap[42]);
    //   UpdateUniformLocation(gShaderProgramMap[42], "foo", 0, 20);
    //   UpdateUniformLocation(gShaderProgramMap[42], "bar", 72, 1);
    //   glUseProgram(gShaderProgramMap[42]);
    //   UpdateCurrentProgram(gShaderProgramMap[42]);
    //   glUniform4fv(gUniformLocations[gCurrentProgram][0], 20, &gBinaryData[300]);
    //   glUniform1iv(gUniformLocations[gCurrentProgram][72], 1, &gBinaryData[400]);
    //   === End override ===
    //   glGetProgramiv(gShaderProgramMap[42], GL_LINK_STATUS, gReadBuffer);
    //   glGetProgramiv(gShaderProgramMap[42], GL_PROGRAM_BINARY_LENGTH, gReadBuffer);
    //
    // To facilitate this override, we are serializing each shader stage source into the binary
    // itself.  See Program::serialize and Program::deserialize.  Once extracted from the binary,
    // they will be available via getProgramSources.

    gl::ShaderProgramID id = inCall.params.getParam("programPacked", ParamType::TShaderProgramID, 0)
                                 .value.ShaderProgramIDVal;

    gl::Program *program = context->getProgramResolveLink(id);
    ASSERT(program);

    mResourceTracker.onShaderProgramAccess(id);
    gl::ShaderProgramID tempShaderStartID = {mResourceTracker.getMaxShaderPrograms()};
    GenerateLinkedProgram(context, context->getState(), &mResourceTracker, &outCalls, program, id,
                          tempShaderStartID, getProgramSources(id));
}

void FrameCaptureShared::captureCustomMapBufferFromContext(const gl::Context *context,
                                                           const char *entryPointName,
                                                           CallCapture &call,
                                                           std::vector<CallCapture> &callsOut)
{
    gl::BufferBinding binding =
        call.params.getParam("targetPacked", ParamType::TBufferBinding, 0).value.BufferBindingVal;
    gl::Buffer *buffer = context->getState().getTargetBuffer(binding);

    if (call.entryPoint == EntryPoint::GLMapBufferRange ||
        call.entryPoint == EntryPoint::GLMapBufferRangeEXT)
    {
        GLintptr offset = call.params.getParam("offset", ParamType::TGLintptr, 1).value.GLintptrVal;
        GLsizeiptr length =
            call.params.getParam("length", ParamType::TGLsizeiptr, 2).value.GLsizeiptrVal;
        GLbitfield access =
            call.params.getParam("access", ParamType::TGLbitfield, 3).value.GLbitfieldVal;

        trackBufferMapping(context, &call, buffer->id(), buffer, offset, length,
                           access & GL_MAP_WRITE_BIT, access & GL_MAP_COHERENT_BIT_EXT);
    }
    else
    {
        ASSERT(call.entryPoint == EntryPoint::GLMapBufferOES);
        GLenum access = call.params.getParam("access", ParamType::TGLenum, 1).value.GLenumVal;
        bool writeAccess =
            (access == GL_WRITE_ONLY_OES || access == GL_WRITE_ONLY || access == GL_READ_WRITE);
        trackBufferMapping(context, &call, buffer->id(), buffer, 0,
                           static_cast<GLsizeiptr>(buffer->getSize()), writeAccess, false);
    }

    CaptureCustomMapBuffer(entryPointName, call, callsOut, buffer->id());
}

void FrameCaptureShared::maybeOverrideEntryPoint(const gl::Context *context,
                                                 CallCapture &inCall,
                                                 std::vector<CallCapture> &outCalls)
{
    switch (inCall.entryPoint)
    {
        case EntryPoint::GLCopyImageSubData:
        case EntryPoint::GLCopyImageSubDataEXT:
        case EntryPoint::GLCopyImageSubDataOES:
        {
            // We must look at the src and dst target types to determine which remap table to use
            updateCopyImageSubData(inCall);
            outCalls.emplace_back(std::move(inCall));
            break;
        }
        case EntryPoint::GLProgramBinary:
        case EntryPoint::GLProgramBinaryOES:
        {
            // Binary formats are not portable at all, so replace the calls with full linking
            // sequence
            overrideProgramBinary(context, inCall, outCalls);
            break;
        }
        case EntryPoint::GLUniformBlockBinding:
        {
            CaptureCustomUniformBlockBinding(inCall, outCalls);
            break;
        }
        case EntryPoint::GLMapBufferRange:
        {
            captureCustomMapBufferFromContext(context, "MapBufferRange", inCall, outCalls);
            break;
        }
        case EntryPoint::GLMapBufferRangeEXT:
        {
            captureCustomMapBufferFromContext(context, "MapBufferRangeEXT", inCall, outCalls);
            break;
        }
        case EntryPoint::GLMapBufferOES:
        {
            captureCustomMapBufferFromContext(context, "MapBufferOES", inCall, outCalls);
            break;
        }
        case EntryPoint::GLCreateShader:
        {
            CaptureCustomShaderProgram("CreateShader", inCall, outCalls);
            break;
        }
        case EntryPoint::GLCreateProgram:
        {
            CaptureCustomShaderProgram("CreateProgram", inCall, outCalls);
            break;
        }
        case EntryPoint::GLCreateShaderProgramv:
        {
            CaptureCustomShaderProgram("CreateShaderProgramv", inCall, outCalls);
            break;
        }
        case EntryPoint::GLFenceSync:
        {
            CaptureCustomFenceSync(inCall, outCalls);
            break;
        }
        case EntryPoint::EGLCreateImage:
        {
            const egl::Image *eglImage = GetImageFromParam(context, inCall.params.getReturnValue());
            CaptureCustomCreateEGLImage(context, "CreateEGLImage", eglImage->getWidth(),
                                        eglImage->getHeight(), inCall, outCalls);
            break;
        }
        case EntryPoint::EGLCreateImageKHR:
        {
            const egl::Image *eglImage = GetImageFromParam(context, inCall.params.getReturnValue());
            CaptureCustomCreateEGLImage(context, "CreateEGLImageKHR", eglImage->getWidth(),
                                        eglImage->getHeight(), inCall, outCalls);
            break;
        }
        case EntryPoint::EGLDestroyImage:
        {
            CaptureCustomDestroyEGLImage("DestroyEGLImage", inCall, outCalls);
            break;
        }
        case EntryPoint::EGLDestroyImageKHR:
        {
            CaptureCustomDestroyEGLImage("DestroyEGLImageKHR", inCall, outCalls);
            break;
        }
        case EntryPoint::EGLCreateSync:
        {
            CaptureCustomCreateEGLSync("CreateEGLSync", inCall, outCalls);
            break;
        }
        case EntryPoint::EGLCreateSyncKHR:
        {
            CaptureCustomCreateEGLSync("CreateEGLSyncKHR", inCall, outCalls);
            break;
        }
        case EntryPoint::EGLCreatePbufferSurface:
        {
            CaptureCustomCreatePbufferSurface(inCall, outCalls);
            break;
        }
        case EntryPoint::EGLCreateNativeClientBufferANDROID:
        {
            CaptureCustomCreateNativeClientbuffer(inCall, outCalls);
            break;
        }

        default:
        {
            // Pass the single call through
            outCalls.emplace_back(std::move(inCall));
            break;
        }
    }
}

void FrameCaptureShared::maybeCaptureCoherentBuffers(const gl::Context *context)
{
    if (!isCaptureActive())
    {
        return;
    }

    std::lock_guard<angle::SimpleMutex> lock(mCoherentBufferTracker.mMutex);

    for (const auto &pair : mCoherentBufferTracker.mBuffers)
    {
        gl::BufferID id = {pair.first};
        if (mCoherentBufferTracker.isDirty(id))
        {
            captureCoherentBufferSnapshot(context, id);
        }
    }
}

void FrameCaptureShared::maybeCaptureDrawArraysClientData(const gl::Context *context,
                                                          CallCapture &call,
                                                          size_t instanceCount)
{
    if (!context->getStateCache().hasAnyActiveClientAttrib())
    {
        return;
    }

    // Get counts from paramBuffer.
    GLint firstVertex =
        call.params.getParamFlexName("first", "start", ParamType::TGLint, 1).value.GLintVal;
    GLsizei drawCount = call.params.getParam("count", ParamType::TGLsizei, 2).value.GLsizeiVal;
    captureClientArraySnapshot(context, firstVertex + drawCount, instanceCount);
}

void FrameCaptureShared::maybeCaptureDrawElementsClientData(const gl::Context *context,
                                                            CallCapture &call,
                                                            size_t instanceCount)
{
    if (!context->getStateCache().hasAnyActiveClientAttrib())
    {
        return;
    }

    // if the count is zero then the index evaluation is not valid and we wouldn't be drawing
    // anything anyway, so skip capturing
    GLsizei count = call.params.getParam("count", ParamType::TGLsizei, 1).value.GLsizeiVal;
    if (count == 0)
    {
        return;
    }

    gl::DrawElementsType drawElementsType =
        call.params.getParam("typePacked", ParamType::TDrawElementsType, 2)
            .value.DrawElementsTypeVal;
    const void *indices =
        call.params.getParam("indices", ParamType::TvoidConstPointer, 3).value.voidConstPointerVal;

    gl::IndexRange indexRange;

    bool restart = context->getState().isPrimitiveRestartEnabled();

    gl::Buffer *elementArrayBuffer = context->getState().getVertexArray()->getElementArrayBuffer();
    if (elementArrayBuffer)
    {
        size_t offset = reinterpret_cast<size_t>(indices);
        (void)elementArrayBuffer->getIndexRange(context, drawElementsType, offset, count, restart,
                                                &indexRange);
    }
    else
    {
        ASSERT(indices);
        indexRange = gl::ComputeIndexRange(drawElementsType, indices, count, restart);
    }

    // index starts from 0
    captureClientArraySnapshot(context, indexRange.end + 1, instanceCount);
}

template <typename AttribT, typename FactoryT>
void CreateEGLImagePreCallUpdate(const CallCapture &call,
                                 ResourceTracker &resourceTracker,
                                 ParamType paramType,
                                 FactoryT factory)
{
    EGLImage image            = call.params.getReturnValue().value.EGLImageVal;
    const ParamCapture &param = call.params.getParam("attrib_list", paramType, 4);
    const AttribT *attribs =
        param.data.empty() ? nullptr : reinterpret_cast<const AttribT *>(param.data[0].data());
    egl::AttributeMap attributeMap = factory(attribs);
    attributeMap.initializeWithoutValidation();
    resourceTracker.getImageToAttribTable().insert(
        std::pair<EGLImage, egl::AttributeMap>(image, attributeMap));
}

void FrameCaptureShared::maybeCapturePreCallUpdates(
    const gl::Context *context,
    CallCapture &call,
    std::vector<CallCapture> *shareGroupSetupCalls,
    ResourceIDToSetupCallsMap *resourceIDToSetupCalls)
{
    switch (call.entryPoint)
    {
        case EntryPoint::GLVertexAttribPointer:
        case EntryPoint::GLVertexPointer:
        case EntryPoint::GLColorPointer:
        case EntryPoint::GLTexCoordPointer:
        case EntryPoint::GLNormalPointer:
        case EntryPoint::GLPointSizePointerOES:
        {
            // Get array location
            GLuint index = 0;
            if (call.entryPoint == EntryPoint::GLVertexAttribPointer)
            {
                index = call.params.getParam("index", ParamType::TGLuint, 0).value.GLuintVal;
            }
            else
            {
                gl::ClientVertexArrayType type;
                switch (call.entryPoint)
                {
                    case EntryPoint::GLVertexPointer:
                        type = gl::ClientVertexArrayType::Vertex;
                        break;
                    case EntryPoint::GLColorPointer:
                        type = gl::ClientVertexArrayType::Color;
                        break;
                    case EntryPoint::GLTexCoordPointer:
                        type = gl::ClientVertexArrayType::TextureCoord;
                        break;
                    case EntryPoint::GLNormalPointer:
                        type = gl::ClientVertexArrayType::Normal;
                        break;
                    case EntryPoint::GLPointSizePointerOES:
                        type = gl::ClientVertexArrayType::PointSize;
                        break;
                    default:
                        UNREACHABLE();
                        type = gl::ClientVertexArrayType::InvalidEnum;
                }
                index = gl::GLES1Renderer::VertexArrayIndex(type, context->getState().gles1());
            }

            if (call.params.hasClientArrayData())
            {
                mClientVertexArrayMap[index] = static_cast<int>(mFrameCalls.size());
            }
            else
            {
                mClientVertexArrayMap[index] = -1;
            }
            break;
        }

        case EntryPoint::GLGenFramebuffers:
        case EntryPoint::GLGenFramebuffersOES:
        {
            GLsizei count = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
            const gl::FramebufferID *framebufferIDs =
                call.params.getParam("framebuffersPacked", ParamType::TFramebufferIDPointer, 1)
                    .value.FramebufferIDPointerVal;
            for (GLsizei i = 0; i < count; i++)
            {
                handleGennedResource(context, framebufferIDs[i]);
            }
            break;
        }

        case EntryPoint::GLBindFramebuffer:
        case EntryPoint::GLBindFramebufferOES:
            maybeGenResourceOnBind<gl::FramebufferID>(context, call);
            break;

        case EntryPoint::GLGenRenderbuffers:
        case EntryPoint::GLGenRenderbuffersOES:
        {
            GLsizei count = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
            const gl::RenderbufferID *renderbufferIDs =
                call.params.getParam("renderbuffersPacked", ParamType::TRenderbufferIDPointer, 1)
                    .value.RenderbufferIDPointerVal;
            for (GLsizei i = 0; i < count; i++)
            {
                handleGennedResource(context, renderbufferIDs[i]);
            }
            break;
        }

        case EntryPoint::GLBindRenderbuffer:
        case EntryPoint::GLBindRenderbufferOES:
            maybeGenResourceOnBind<gl::RenderbufferID>(context, call);
            break;

        case EntryPoint::GLDeleteRenderbuffers:
        case EntryPoint::GLDeleteRenderbuffersOES:
        {
            // Look up how many renderbuffers are being deleted
            GLsizei n = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;

            // Look up the pointer to list of renderbuffers
            const gl::RenderbufferID *renderbufferIDs =
                call.params
                    .getParam("renderbuffersPacked", ParamType::TRenderbufferIDConstPointer, 1)
                    .value.RenderbufferIDConstPointerVal;

            // For each renderbuffer listed for deletion
            for (int32_t i = 0; i < n; ++i)
            {
                // If we're capturing, track what renderbuffers have been deleted
                handleDeletedResource(context, renderbufferIDs[i]);
            }
            break;
        }

        case EntryPoint::GLGenTextures:
        {
            GLsizei count = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
            const gl::TextureID *textureIDs =
                call.params.getParam("texturesPacked", ParamType::TTextureIDPointer, 1)
                    .value.TextureIDPointerVal;
            for (GLsizei i = 0; i < count; i++)
            {
                // If we're capturing, track what new textures have been genned
                handleGennedResource(context, textureIDs[i]);
            }
            break;
        }

        case EntryPoint::GLBindTexture:
            maybeGenResourceOnBind<gl::TextureID>(context, call);
            if (isCaptureActive())
            {
                gl::TextureType target =
                    call.params.getParam("targetPacked", ParamType::TTextureType, 0)
                        .value.TextureTypeVal;
                context->getFrameCapture()->getStateResetHelper().setTextureBindingDirty(
                    context->getState().getActiveSampler(), target);
            }
            break;

        case EntryPoint::GLDeleteBuffers:
        {
            GLsizei count = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
            const gl::BufferID *bufferIDs =
                call.params.getParam("buffersPacked", ParamType::TBufferIDConstPointer, 1)
                    .value.BufferIDConstPointerVal;
            for (GLsizei i = 0; i < count; i++)
            {
                // For each buffer being deleted, check our backup of data and remove it
                const auto &bufferDataInfo = mBufferDataMap.find(bufferIDs[i]);
                if (bufferDataInfo != mBufferDataMap.end())
                {
                    mBufferDataMap.erase(bufferDataInfo);
                }
                // If we're capturing, track what buffers have been deleted
                handleDeletedResource(context, bufferIDs[i]);
            }
            break;
        }

        case EntryPoint::GLGenBuffers:
        {
            GLsizei count = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
            const gl::BufferID *bufferIDs =
                call.params.getParam("buffersPacked", ParamType::TBufferIDPointer, 1)
                    .value.BufferIDPointerVal;
            for (GLsizei i = 0; i < count; i++)
            {
                handleGennedResource(context, bufferIDs[i]);
            }
            break;
        }

        case EntryPoint::GLBindBuffer:
            maybeGenResourceOnBind<gl::BufferID>(context, call);
            if (isCaptureActive())
            {
                gl::BufferBinding binding =
                    call.params.getParam("targetPacked", ParamType::TBufferBinding, 0)
                        .value.BufferBindingVal;

                context->getFrameCapture()->getStateResetHelper().setBufferBindingDirty(binding);
            }
            break;

        case EntryPoint::GLBindBufferBase:
        case EntryPoint::GLBindBufferRange:
            if (isCaptureActive())
            {
                WARN() << "Indexed buffer binding changed during capture, Reset doesn't handle it "
                          "yet.";
            }
            break;

        case EntryPoint::GLDeleteProgramPipelines:
        case EntryPoint::GLDeleteProgramPipelinesEXT:
        {
            GLsizei count = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
            const gl::ProgramPipelineID *pipelineIDs =
                call.params
                    .getParam("pipelinesPacked", ParamType::TProgramPipelineIDConstPointer, 1)
                    .value.ProgramPipelineIDPointerVal;
            for (GLsizei i = 0; i < count; i++)
            {
                handleDeletedResource(context, pipelineIDs[i]);
            }
            break;
        }

        case EntryPoint::GLGenProgramPipelines:
        case EntryPoint::GLGenProgramPipelinesEXT:
        {
            GLsizei count = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
            const gl::ProgramPipelineID *pipelineIDs =
                call.params.getParam("pipelinesPacked", ParamType::TProgramPipelineIDPointer, 1)
                    .value.ProgramPipelineIDPointerVal;
            for (GLsizei i = 0; i < count; i++)
            {
                handleGennedResource(context, pipelineIDs[i]);
            }
            break;
        }

        case EntryPoint::GLDeleteSync:
        {
            gl::SyncID sync =
                call.params.getParam("syncPacked", ParamType::TSyncID, 0).value.SyncIDVal;
            FrameCaptureShared *frameCaptureShared =
                context->getShareGroup()->getFrameCaptureShared();
            // If we're capturing, track which fence sync has been deleted
            if (frameCaptureShared->isCaptureActive())
            {
                mResourceTracker.setDeletedFenceSync(sync);
            }
            break;
        }

        case EntryPoint::GLDrawArrays:
        {
            maybeCaptureDrawArraysClientData(context, call, 1);
            maybeCaptureCoherentBuffers(context);
            break;
        }

        case EntryPoint::GLDrawArraysInstanced:
        case EntryPoint::GLDrawArraysInstancedANGLE:
        case EntryPoint::GLDrawArraysInstancedEXT:
        {
            GLsizei instancecount =
                call.params.getParamFlexName("instancecount", "primcount", ParamType::TGLsizei, 3)
                    .value.GLsizeiVal;

            maybeCaptureDrawArraysClientData(context, call, instancecount);
            maybeCaptureCoherentBuffers(context);
            break;
        }

        case EntryPoint::GLDrawElements:
        {
            maybeCaptureDrawElementsClientData(context, call, 1);
            maybeCaptureCoherentBuffers(context);
            break;
        }

        case EntryPoint::GLDrawElementsInstanced:
        case EntryPoint::GLDrawElementsInstancedANGLE:
        case EntryPoint::GLDrawElementsInstancedEXT:
        {
            GLsizei instancecount =
                call.params.getParamFlexName("instancecount", "primcount", ParamType::TGLsizei, 4)
                    .value.GLsizeiVal;

            maybeCaptureDrawElementsClientData(context, call, instancecount);
            maybeCaptureCoherentBuffers(context);
            break;
        }

        case EntryPoint::GLCreateShaderProgramv:
        {
            // Refresh the cached shader sources.
            // The command CreateShaderProgramv() creates a stand-alone program from an array of
            // null-terminated source code strings for a single shader type, so we need update the
            // Shader and Program sources, similar to GLCompileShader + GLLinkProgram handling.
            gl::ShaderProgramID programID = {call.params.getReturnValue().value.GLuintVal};
            const ParamCapture &paramCapture =
                call.params.getParam("typePacked", ParamType::TShaderType, 0);
            const ParamCapture &lineCount = call.params.getParam("count", ParamType::TGLsizei, 1);
            const ParamCapture &strings =
                call.params.getParam("strings", ParamType::TGLcharConstPointerPointer, 2);

            std::ostringstream sourceString;
            for (int i = 0; i < lineCount.value.GLsizeiVal; ++i)
            {
                sourceString << strings.value.GLcharConstPointerPointerVal[i];
            }

            gl::ShaderType shaderType = paramCapture.value.ShaderTypeVal;
            ProgramSources source;
            source[shaderType] = sourceString.str();
            setProgramSources(programID, source);
            handleGennedResource(context, programID);
            mResourceTracker.setShaderProgramType(programID, ShaderProgramType::ProgramType);
            break;
        }

        case EntryPoint::GLCreateProgram:
        {
            // If we're capturing, track which programs have been created
            gl::ShaderProgramID programID = {call.params.getReturnValue().value.GLuintVal};
            handleGennedResource(context, programID);

            mResourceTracker.setShaderProgramType(programID, ShaderProgramType::ProgramType);
            break;
        }

        case EntryPoint::GLDeleteProgram:
        {
            // If we're capturing, track which programs have been deleted
            const ParamCapture &param =
                call.params.getParam("programPacked", ParamType::TShaderProgramID, 0);
            handleDeletedResource(context, param.value.ShaderProgramIDVal);

            // If this assert fires, it means a ShaderProgramID has changed from program to shader
            // which is unsupported
            ASSERT(mResourceTracker.getShaderProgramType(param.value.ShaderProgramIDVal) ==
                   ShaderProgramType::ProgramType);

            break;
        }

        case EntryPoint::GLCreateShader:
        {
            // If we're capturing, track which shaders have been created
            gl::ShaderProgramID shaderID = {call.params.getReturnValue().value.GLuintVal};
            handleGennedResource(context, shaderID);

            mResourceTracker.setShaderProgramType(shaderID, ShaderProgramType::ShaderType);
            break;
        }

        case EntryPoint::GLDeleteShader:
        {
            // If we're capturing, track which shaders have been deleted
            const ParamCapture &param =
                call.params.getParam("shaderPacked", ParamType::TShaderProgramID, 0);
            handleDeletedResource(context, param.value.ShaderProgramIDVal);

            // If this assert fires, it means a ShaderProgramID has changed from shader to program
            // which is unsupported
            ASSERT(mResourceTracker.getShaderProgramType(param.value.ShaderProgramIDVal) ==
                   ShaderProgramType::ShaderType);
            break;
        }

        case EntryPoint::GLCompileShader:
        {
            // Refresh the cached shader sources.
            gl::ShaderProgramID shaderID =
                call.params.getParam("shaderPacked", ParamType::TShaderProgramID, 0)
                    .value.ShaderProgramIDVal;
            const gl::Shader *shader = context->getShaderNoResolveCompile(shaderID);
            // Shaders compiled for ProgramBinary will not have a shader created
            if (shader)
            {
                setShaderSource(shaderID, shader->getSourceString());
            }
            break;
        }

        case EntryPoint::GLLinkProgram:
        {
            // Refresh the cached program sources.
            gl::ShaderProgramID programID =
                call.params.getParam("programPacked", ParamType::TShaderProgramID, 0)
                    .value.ShaderProgramIDVal;
            const gl::Program *program = context->getProgramResolveLink(programID);
            // Programs linked in support of ProgramBinary will not have attached shaders
            if (program->getState().hasAnyAttachedShader())
            {
                setProgramSources(programID, GetAttachedProgramSources(context, program));
            }
            break;
        }

        case EntryPoint::GLDeleteTextures:
        {
            // Free any TextureLevelDataMap entries being tracked for this texture
            // This is to cover the scenario where a texture has been created, its
            // levels cached, then texture deleted and recreated, receiving the same ID

            // Look up how many textures are being deleted
            GLsizei n = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;

            // Look up the pointer to list of textures
            const gl::TextureID *textureIDs =
                call.params.getParam("texturesPacked", ParamType::TTextureIDConstPointer, 1)
                    .value.TextureIDConstPointerVal;

            // For each texture listed for deletion
            for (int32_t i = 0; i < n; ++i)
            {
                // If we're capturing, track what textures have been deleted
                handleDeletedResource(context, textureIDs[i]);
            }
            break;
        }

        case EntryPoint::GLMapBufferOES:
        {
            gl::BufferBinding target =
                call.params.getParam("targetPacked", ParamType::TBufferBinding, 0)
                    .value.BufferBindingVal;

            GLbitfield access =
                call.params.getParam("access", ParamType::TGLenum, 1).value.GLenumVal;

            gl::Buffer *buffer = context->getState().getTargetBuffer(target);

            GLintptr offset   = 0;
            GLsizeiptr length = static_cast<GLsizeiptr>(buffer->getSize());

            bool writable =
                access == GL_WRITE_ONLY_OES || access == GL_WRITE_ONLY || access == GL_READ_WRITE;

            FrameCaptureShared *frameCaptureShared =
                context->getShareGroup()->getFrameCaptureShared();
            frameCaptureShared->trackBufferMapping(context, &call, buffer->id(), buffer, offset,
                                                   length, writable, false);
            break;
        }

        case EntryPoint::GLUnmapBuffer:
        case EntryPoint::GLUnmapBufferOES:
        {
            // See if we need to capture the buffer contents
            captureMappedBufferSnapshot(context, call);

            // Track that the buffer was unmapped, for use during state reset
            gl::BufferBinding target =
                call.params.getParam("targetPacked", ParamType::TBufferBinding, 0)
                    .value.BufferBindingVal;
            gl::Buffer *buffer = context->getState().getTargetBuffer(target);
            mResourceTracker.setBufferUnmapped(context->id(), buffer->id().value);

            // Remove from CoherentBufferTracker
            mCoherentBufferTracker.removeBuffer(buffer->id());
            break;
        }

        case EntryPoint::GLBufferData:
        case EntryPoint::GLBufferSubData:
        {
            gl::BufferBinding target =
                call.params.getParam("targetPacked", ParamType::TBufferBinding, 0)
                    .value.BufferBindingVal;

            gl::Buffer *buffer = context->getState().getTargetBuffer(target);

            // Track that this buffer's contents have been modified
            mResourceTracker.getTrackedResource(context->id(), ResourceIDType::Buffer)
                .setModifiedResource(buffer->id().value);

            // BufferData is equivalent to UnmapBuffer, for what we're tracking.
            // From the ES 3.1 spec in BufferData section:
            //     If any portion of the buffer object is mapped in the current context or any
            //     context current to another thread, it is as though UnmapBuffer (see section
            //     6.3.1) is executed in each such context prior to deleting the existing data
            //     store.
            // Track that the buffer was unmapped, for use during state reset
            mResourceTracker.setBufferUnmapped(context->id(), buffer->id().value);

            break;
        }

        case EntryPoint::GLCopyBufferSubData:
        {
            maybeCaptureCoherentBuffers(context);
            break;
        }
        case EntryPoint::GLFinish:
        {
            // When using shadow memory we might need to synchronize it here.
            if (mCoherentBufferTracker.isShadowMemoryEnabled())
            {
                mCoherentBufferTracker.maybeUpdateShadowMemory();
            }
            break;
        }
        case EntryPoint::GLDeleteFramebuffers:
        case EntryPoint::GLDeleteFramebuffersOES:
        {
            // Look up how many framebuffers are being deleted
            GLsizei n = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;

            // Look up the pointer to list of framebuffers
            const gl::FramebufferID *framebufferIDs =
                call.params.getParam("framebuffersPacked", ParamType::TFramebufferIDConstPointer, 1)
                    .value.FramebufferIDConstPointerVal;

            // For each framebuffer listed for deletion
            for (int32_t i = 0; i < n; ++i)
            {
                // If we're capturing, track what framebuffers have been deleted
                handleDeletedResource(context, framebufferIDs[i]);
            }
            break;
        }

        case EntryPoint::GLUseProgram:
        {
            if (isCaptureActive())
            {
                context->getFrameCapture()->getStateResetHelper().setEntryPointDirty(
                    EntryPoint::GLUseProgram);
            }
            break;
        }

        case EntryPoint::GLGenVertexArrays:
        case EntryPoint::GLGenVertexArraysOES:
        {
            GLsizei count = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
            const gl::VertexArrayID *arrayIDs =
                call.params.getParam("arraysPacked", ParamType::TVertexArrayIDPointer, 1)
                    .value.VertexArrayIDPointerVal;
            for (GLsizei i = 0; i < count; i++)
            {
                handleGennedResource(context, arrayIDs[i]);
            }
            break;
        }

        case EntryPoint::GLDeleteVertexArrays:
        case EntryPoint::GLDeleteVertexArraysOES:
        {
            GLsizei count = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
            const gl::VertexArrayID *arrayIDs =
                call.params.getParam("arraysPacked", ParamType::TVertexArrayIDConstPointer, 1)
                    .value.VertexArrayIDConstPointerVal;
            for (GLsizei i = 0; i < count; i++)
            {
                // If we're capturing, track which vertex arrays have been deleted
                handleDeletedResource(context, arrayIDs[i]);
            }
            break;
        }

        case EntryPoint::GLBindVertexArray:
        case EntryPoint::GLBindVertexArrayOES:
        {
            if (isCaptureActive())
            {
                context->getFrameCapture()->getStateResetHelper().setEntryPointDirty(
                    EntryPoint::GLBindVertexArray);
            }
            break;
        }
        case EntryPoint::GLBlendFunc:
        {
            if (isCaptureActive())
            {
                context->getFrameCapture()->getStateResetHelper().setEntryPointDirty(
                    EntryPoint::GLBlendFunc);
            }
            break;
        }
        case EntryPoint::GLBlendFuncSeparate:
        {
            if (isCaptureActive())
            {
                context->getFrameCapture()->getStateResetHelper().setEntryPointDirty(
                    EntryPoint::GLBlendFuncSeparate);
            }
            break;
        }
        case EntryPoint::GLBlendEquation:
        case EntryPoint::GLBlendEquationSeparate:
        {
            if (isCaptureActive())
            {
                context->getFrameCapture()->getStateResetHelper().setEntryPointDirty(
                    EntryPoint::GLBlendEquationSeparate);
            }
            break;
        }
        case EntryPoint::GLColorMask:
        {
            if (isCaptureActive())
            {
                context->getFrameCapture()->getStateResetHelper().setEntryPointDirty(
                    EntryPoint::GLColorMask);
            }
            break;
        }
        case EntryPoint::GLBlendColor:
        {
            if (isCaptureActive())
            {
                context->getFrameCapture()->getStateResetHelper().setEntryPointDirty(
                    EntryPoint::GLBlendColor);
            }
            break;
        }

        case EntryPoint::GLEGLImageTargetTexture2DOES:
        {
            gl::TextureType target =
                call.params.getParam("targetPacked", ParamType::TTextureType, 0)
                    .value.TextureTypeVal;
            egl::ImageID imageID =
                call.params.getParam("imagePacked", ParamType::TImageID, 1).value.ImageIDVal;
            mResourceTracker.getTextureIDToImageTable().insert(std::pair<GLuint, egl::ImageID>(
                context->getState().getTargetTexture(target)->getId(), imageID));
            break;
        }

        case EntryPoint::EGLCreateImage:
        {
            CreateEGLImagePreCallUpdate<EGLAttrib>(call, mResourceTracker,
                                                   ParamType::TEGLAttribPointer,
                                                   egl::AttributeMap::CreateFromAttribArray);
            if (isCaptureActive())
            {
                EGLImage eglImage    = call.params.getReturnValue().value.EGLImageVal;
                egl::ImageID imageID = egl::PackParam<egl::ImageID>(eglImage);
                handleGennedResource(context, imageID);
            }
            break;
        }
        case EntryPoint::EGLCreateImageKHR:
        {
            CreateEGLImagePreCallUpdate<EGLint>(call, mResourceTracker, ParamType::TEGLintPointer,
                                                egl::AttributeMap::CreateFromIntArray);
            if (isCaptureActive())
            {
                EGLImageKHR eglImage = call.params.getReturnValue().value.EGLImageKHRVal;
                egl::ImageID imageID = egl::PackParam<egl::ImageID>(eglImage);
                handleGennedResource(context, imageID);
            }
            break;
        }
        case EntryPoint::EGLDestroyImage:
        case EntryPoint::EGLDestroyImageKHR:
        {
            egl::ImageID eglImageID =
                call.params.getParam("imagePacked", ParamType::TImageID, 1).value.ImageIDVal;

            // Clear any texture->image mappings that involve this image
            for (auto texImageIter = mResourceTracker.getTextureIDToImageTable().begin();
                 texImageIter != mResourceTracker.getTextureIDToImageTable().end();)
            {
                if (texImageIter->second == eglImageID)
                {
                    texImageIter = mResourceTracker.getTextureIDToImageTable().erase(texImageIter);
                }
                else
                {
                    ++texImageIter;
                }
            }

            FrameCaptureShared *frameCaptureShared =
                context->getShareGroup()->getFrameCaptureShared();
            if (frameCaptureShared->isCaptureActive())
            {
                handleDeletedResource(context, eglImageID);
            }
            break;
        }
        case EntryPoint::EGLCreateSync:
        case EntryPoint::EGLCreateSyncKHR:
        {
            egl::SyncID eglSyncID = call.params.getReturnValue().value.egl_SyncIDVal;
            FrameCaptureShared *frameCaptureShared =
                context->getShareGroup()->getFrameCaptureShared();
            // If we're capturing, track which egl sync has been created
            if (frameCaptureShared->isCaptureActive())
            {
                handleGennedResource(context, eglSyncID);
            }
            break;
        }
        case EntryPoint::EGLDestroySync:
        case EntryPoint::EGLDestroySyncKHR:
        {
            egl::SyncID eglSyncID =
                call.params.getParam("syncPacked", ParamType::Tegl_SyncID, 1).value.egl_SyncIDVal;
            FrameCaptureShared *frameCaptureShared =
                context->getShareGroup()->getFrameCaptureShared();
            // If we're capturing, track which EGL sync has been deleted
            if (frameCaptureShared->isCaptureActive())
            {
                handleDeletedResource(context, eglSyncID);
            }
            break;
        }
        case EntryPoint::GLDispatchCompute:
        {
            // When using shadow memory we need to update the real memory here
            if (mCoherentBufferTracker.isShadowMemoryEnabled())
            {
                maybeCaptureCoherentBuffers(context);
            }
            break;
        }
        default:
            break;
    }

    if (IsTextureUpdate(call))
    {
        // If this call modified texture contents, track it for possible reset
        trackTextureUpdate(context, call);
    }

    if (IsImageUpdate(call))
    {
        // If this call modified shader image contents, track it for possible reset
        trackImageUpdate(context, call);
    }

    if (isCaptureActive() && GetDefaultUniformType(call) != DefaultUniformType::None)
    {
        trackDefaultUniformUpdate(context, call);
    }

    if (IsVertexArrayUpdate(call))
    {
        trackVertexArrayUpdate(context, call);
    }

    updateReadBufferSize(call.params.getReadBufferSize());

    std::vector<gl::ShaderProgramID> shaderProgramIDs;
    if (FindResourceIDsInCall<gl::ShaderProgramID>(call, shaderProgramIDs))
    {
        for (gl::ShaderProgramID shaderProgramID : shaderProgramIDs)
        {
            mResourceTracker.onShaderProgramAccess(shaderProgramID);

            if (isCaptureActive())
            {
                // Track that this call referenced a ShaderProgram, setting it active for Setup
                MarkResourceIDActive(ResourceIDType::ShaderProgram, shaderProgramID.value,
                                     shareGroupSetupCalls, resourceIDToSetupCalls);
            }
        }
    }

    std::vector<gl::TextureID> textureIDs;
    if (FindResourceIDsInCall<gl::TextureID>(call, textureIDs))
    {
        for (gl::TextureID textureID : textureIDs)
        {
            if (isCaptureActive())
            {
                // Track that this call referenced a Texture, setting it active for Setup
                MarkResourceIDActive(ResourceIDType::Texture, textureID.value, shareGroupSetupCalls,
                                     resourceIDToSetupCalls);
            }
        }
    }
}

template <typename ParamValueType>
void FrameCaptureShared::maybeGenResourceOnBind(const gl::Context *context, CallCapture &call)
{
    const char *paramName     = ParamValueTrait<ParamValueType>::name;
    const ParamType paramType = ParamValueTrait<ParamValueType>::typeID;

    const ParamCapture &param = call.params.getParam(paramName, paramType, 1);
    const ParamValueType id   = AccessParamValue<ParamValueType>(paramType, param.value);

    // Don't inject the default resource or resources that are already generated
    if (id.value != 0 && !resourceIsGenerated(context, id))
    {
        handleGennedResource(context, id);

        ResourceIDType resourceIDType = GetResourceIDTypeFromParamType(param.type);
        const char *resourceName      = GetResourceIDTypeName(resourceIDType);

        std::stringstream updateFuncNameStr;
        updateFuncNameStr << "Set" << resourceName << "ID";
        ParamBuffer params;
        if (IsTrackedPerContext(resourceIDType))
        {
            // TODO (https://issuetracker.google.com/169868803) The '2' version can be removed after
            // all context-local objects are tracked per-context
            updateFuncNameStr << "2";
            params.addValueParam("contextID", ParamType::TGLuint, context->id().value);
        }
        std::string updateFuncName = updateFuncNameStr.str();
        params.addValueParam("id", ParamType::TGLuint, id.value);
        mFrameCalls.emplace_back(updateFuncName, std::move(params));
    }
}

void FrameCaptureShared::updateResourceCountsFromParamCapture(const ParamCapture &param,
                                                              ResourceIDType idType)
{
    if (idType != ResourceIDType::InvalidEnum)
    {
        mHasResourceType.set(idType);

        // Capture resource IDs for non-pointer types.
        if (strcmp(ParamTypeToString(param.type), "GLuint") == 0)
        {
            mMaxAccessedResourceIDs[idType] =
                std::max(mMaxAccessedResourceIDs[idType], param.value.GLuintVal);
        }
        // Capture resource IDs for pointer types.
        if (strstr(ParamTypeToString(param.type), "GLuint *") != nullptr)
        {
            if (param.data.size() == 1u)
            {
                const GLuint *dataPtr = reinterpret_cast<const GLuint *>(param.data[0].data());
                size_t numHandles     = param.data[0].size() / sizeof(GLuint);
                for (size_t handleIndex = 0; handleIndex < numHandles; ++handleIndex)
                {
                    mMaxAccessedResourceIDs[idType] =
                        std::max(mMaxAccessedResourceIDs[idType], dataPtr[handleIndex]);
                }
            }
        }
        if (idType == ResourceIDType::Sync)
        {
            mMaxAccessedResourceIDs[idType] =
                std::max(mMaxAccessedResourceIDs[idType], param.value.GLuintVal);
        }
    }
}

void FrameCaptureShared::updateResourceCountsFromCallCapture(const CallCapture &call)
{
    for (const ParamCapture &param : call.params.getParamCaptures())
    {
        ResourceIDType idType = GetResourceIDTypeFromParamType(param.type);
        updateResourceCountsFromParamCapture(param, idType);
    }

    // Update resource IDs in the return value. Return values types are not stored as resource IDs,
    // but instead are stored as GLuints. Therefore we need to explicitly label the resource ID type
    // when we call update. Currently only shader and program creation are explicitly tracked.
    switch (call.entryPoint)
    {
        case EntryPoint::GLCreateShader:
        case EntryPoint::GLCreateProgram:
            updateResourceCountsFromParamCapture(call.params.getReturnValue(),
                                                 ResourceIDType::ShaderProgram);
            break;

        case EntryPoint::GLFenceSync:
            updateResourceCountsFromParamCapture(call.params.getReturnValue(),
                                                 ResourceIDType::Sync);
            break;
        case EntryPoint::EGLCreateSync:
        case EntryPoint::EGLCreateSyncKHR:
            updateResourceCountsFromParamCapture(call.params.getReturnValue(),
                                                 ResourceIDType::egl_Sync);
            break;
        default:
            break;
    }
}

void FrameCaptureShared::captureCall(gl::Context *context, CallCapture &&inCall, bool isCallValid)
{
    if (SkipCall(inCall.entryPoint))
    {
        return;
    }

    if (isCallValid)
    {
        // Save the call's contextID
        inCall.contextID = context->id();

        // Update resource counts before we override entry points with custom calls.
        updateResourceCountsFromCallCapture(inCall);

        size_t j = mFrameCalls.size();

        std::vector<CallCapture> outCalls;
        maybeOverrideEntryPoint(context, inCall, outCalls);

        // Need to loop on any new calls we added during override
        for (CallCapture &call : outCalls)
        {
            // During capture, consider all frame calls active
            if (isCaptureActive())
            {
                call.isActive = true;
            }

            maybeCapturePreCallUpdates(context, call, &mShareGroupSetupCalls,
                                       &mResourceIDToSetupCalls);
            mFrameCalls.emplace_back(std::move(call));
            maybeCapturePostCallUpdates(context);
        }

        // Tag all 'added' commands with this context
        for (size_t k = j; k < mFrameCalls.size(); k++)
        {
            mFrameCalls[k].contextID = context->id();
        }

        // Evaluate the validation expression to determine if we insert a validation checkpoint.
        // This lets the user pick a subset of calls to check instead of checking every call.
        if (mValidateSerializedState && !mValidationExpression.empty())
        {
            // Example substitution for frame #2, call #110:
            // Before: (call == 2) && (frame >= 100) && (frame <= 120) && ((frame % 10) == 0)
            // After:  (2 == 2) && (110 >= 100) && (110 <= 120) && ((110 % 10) == 0)
            // Evaluates to 1.0.
            std::string expression = mValidationExpression;

            angle::ReplaceAllSubstrings(&expression, "frame", std::to_string(mFrameIndex));
            angle::ReplaceAllSubstrings(&expression, "call", std::to_string(mFrameCalls.size()));

            double result = ceval_result(expression);
            if (result > 0)
            {
                CaptureValidateSerializedState(context, &mFrameCalls);
            }
        }
    }
    else
    {
        const int maxInvalidCallLogs = 3;
        size_t &callCount = isCaptureActive() ? mInvalidCallCountsActive[inCall.entryPoint]
                                              : mInvalidCallCountsInactive[inCall.entryPoint];
        callCount++;
        if (callCount <= maxInvalidCallLogs)
        {
            std::ostringstream msg;
            msg << "FrameCapture (capture " << (isCaptureActive() ? "active" : "inactive")
                << "): Not capturing invalid call to " << GetEntryPointName(inCall.entryPoint);
            if (callCount == maxInvalidCallLogs)
            {
                msg << " (will no longer repeat for this entry point)";
            }
            INFO() << msg.str();
        }

        std::stringstream skipCall;
        skipCall << "Skipping invalid call to " << GetEntryPointName(inCall.entryPoint)
                 << " with error: "
                 << gl::GLenumToString(gl::GLESEnum::ErrorCode, context->getErrorForCapture());
        AddComment(&mFrameCalls, skipCall.str());
    }
}

void FrameCaptureShared::maybeCapturePostCallUpdates(const gl::Context *context)
{
    // Process resource ID updates.
    if (isCaptureActive())
    {
        MaybeCaptureUpdateResourceIDs(context, &mResourceTracker, &mFrameCalls);
    }

    CallCapture &lastCall = mFrameCalls.back();
    switch (lastCall.entryPoint)
    {
        case EntryPoint::GLCreateShaderProgramv:
        {
            gl::ShaderProgramID programId;
            programId.value            = lastCall.params.getReturnValue().value.GLuintVal;
            const gl::Program *program = context->getProgramResolveLink(programId);
            CaptureUpdateUniformLocations(program, &mFrameCalls);
            CaptureUpdateUniformBlockIndexes(program, &mFrameCalls);
            break;
        }
        case EntryPoint::GLLinkProgram:
        {
            const ParamCapture &param =
                lastCall.params.getParam("programPacked", ParamType::TShaderProgramID, 0);
            const gl::Program *program =
                context->getProgramResolveLink(param.value.ShaderProgramIDVal);
            CaptureUpdateUniformLocations(program, &mFrameCalls);
            CaptureUpdateUniformBlockIndexes(program, &mFrameCalls);
            break;
        }
        case EntryPoint::GLUseProgram:
            CaptureUpdateCurrentProgram(lastCall, 0, &mFrameCalls);
            break;
        case EntryPoint::GLActiveShaderProgram:
            CaptureUpdateCurrentProgram(lastCall, 1, &mFrameCalls);
            break;
        case EntryPoint::GLDeleteProgram:
        {
            const ParamCapture &param =
                lastCall.params.getParam("programPacked", ParamType::TShaderProgramID, 0);
            CaptureDeleteUniformLocations(param.value.ShaderProgramIDVal, &mFrameCalls);
            break;
        }
        case EntryPoint::GLShaderSource:
        {
            lastCall.params.setValueParamAtIndex("count", ParamType::TGLsizei, 1, 1);

            ParamCapture &paramLength =
                lastCall.params.getParam("length", ParamType::TGLintConstPointer, 3);
            paramLength.data.resize(1);
            // Set the length parameter to {-1} to signal that the actual string length
            // is to be used. Since we store the parameter blob as an array of four uint8_t
            // values, we have to pass the binary equivalent of -1.
            paramLength.data[0] = {0xff, 0xff, 0xff, 0xff};
            break;
        }
        case EntryPoint::GLBufferData:
        case EntryPoint::GLBufferSubData:
        {
            // When using shadow memory we need to update it from real memory here
            if (mCoherentBufferTracker.isShadowMemoryEnabled())
            {
                gl::BufferBinding target =
                    lastCall.params.getParam("targetPacked", ParamType::TBufferBinding, 0)
                        .value.BufferBindingVal;

                gl::Buffer *buffer = context->getState().getTargetBuffer(target);
                if (mCoherentBufferTracker.haveBuffer(buffer->id()))
                {
                    std::shared_ptr<CoherentBuffer> cb =
                        mCoherentBufferTracker.mBuffers[buffer->id().value];
                    cb->removeProtection(PageSharingType::NoneShared);
                    cb->updateShadowMemory();
                    cb->protectAll();
                }
            }
            break;
        }

        case EntryPoint::GLCopyBufferSubData:
        {
            // When using shadow memory, we need to mark the buffer shadowDirty bit to true
            // so it will be synchronized with real memory on the next glFinish call.
            if (mCoherentBufferTracker.isShadowMemoryEnabled())
            {
                gl::BufferBinding target =
                    lastCall.params.getParam("writeTargetPacked", ParamType::TBufferBinding, 1)
                        .value.BufferBindingVal;

                gl::Buffer *buffer = context->getState().getTargetBuffer(target);
                if (mCoherentBufferTracker.haveBuffer(buffer->id()))
                {
                    std::shared_ptr<CoherentBuffer> cb =
                        mCoherentBufferTracker.mBuffers[buffer->id().value];
                    // This needs to be synced on glFinish
                    cb->markShadowDirty();
                }
            }
            break;
        }
        case EntryPoint::GLDispatchCompute:
        {
            // When using shadow memory, we need to mark all buffer's shadowDirty bit to true
            // so they will be synchronized with real memory on the next glFinish call.
            if (mCoherentBufferTracker.isShadowMemoryEnabled())
            {
                mCoherentBufferTracker.markAllShadowDirty();
            }
            break;
        }
        default:
            break;
    }
}

void FrameCaptureShared::captureClientArraySnapshot(const gl::Context *context,
                                                    size_t vertexCount,
                                                    size_t instanceCount)
{
    if (vertexCount == 0)
    {
        // Nothing to capture
        return;
    }

    const gl::VertexArray *vao = context->getState().getVertexArray();

    // Capture client array data.
    for (size_t attribIndex : context->getStateCache().getActiveClientAttribsMask())
    {
        const gl::VertexAttribute &attrib = vao->getVertexAttribute(attribIndex);
        const gl::VertexBinding &binding  = vao->getVertexBinding(attrib.bindingIndex);

        int callIndex = mClientVertexArrayMap[attribIndex];

        if (callIndex != -1)
        {
            size_t count = vertexCount;

            if (binding.getDivisor() > 0)
            {
                count = rx::UnsignedCeilDivide(static_cast<uint32_t>(instanceCount),
                                               binding.getDivisor());
            }

            // The last capture element doesn't take up the full stride.
            size_t bytesToCapture = (count - 1) * binding.getStride() + attrib.format->pixelBytes;

            CallCapture &call   = mFrameCalls[callIndex];
            ParamCapture &param = call.params.getClientArrayPointerParameter();
            ASSERT(param.type == ParamType::TvoidConstPointer);

            ParamBuffer updateParamBuffer;
            updateParamBuffer.addValueParam<GLint>("arrayIndex", ParamType::TGLint,
                                                   static_cast<uint32_t>(attribIndex));

            ParamCapture updateMemory("pointer", ParamType::TvoidConstPointer);
            CaptureMemory(param.value.voidConstPointerVal, bytesToCapture, &updateMemory);
            updateParamBuffer.addParam(std::move(updateMemory));

            updateParamBuffer.addValueParam<GLuint64>("size", ParamType::TGLuint64, bytesToCapture);

            mFrameCalls.emplace_back("UpdateClientArrayPointer", std::move(updateParamBuffer));

            mClientArraySizes[attribIndex] =
                std::max(mClientArraySizes[attribIndex], bytesToCapture);
        }
    }
}

void FrameCaptureShared::captureCoherentBufferSnapshot(const gl::Context *context, gl::BufferID id)
{
    if (!hasBufferData(id))
    {
        // This buffer was not marked writable
        return;
    }

    const gl::State &apiState        = context->getState();
    const gl::BufferManager &buffers = apiState.getBufferManagerForCapture();
    gl::Buffer *buffer               = buffers.getBuffer(id);
    if (!buffer)
    {
        // Could not find buffer binding
        return;
    }

    ASSERT(buffer->isMapped());

    std::shared_ptr<angle::CoherentBuffer> coherentBuffer =
        mCoherentBufferTracker.mBuffers[id.value];

    std::vector<PageRange> dirtyPageRanges = coherentBuffer->getDirtyPageRanges();

    if (mCoherentBufferTracker.isShadowMemoryEnabled() && !dirtyPageRanges.empty())
    {
        coherentBuffer->updateBufferMemory();
    }

    AddressRange wholeRange = coherentBuffer->getRange();

    for (PageRange &pageRange : dirtyPageRanges)
    {
        // Write protect the memory already, so the app is blocked on writing during our capture
        coherentBuffer->protectPageRange(pageRange);

        // Create the parameters to our helper for use during replay
        ParamBuffer dataParamBuffer;

        // Pass in the target buffer ID
        dataParamBuffer.addValueParam("dest", ParamType::TGLuint, buffer->id().value);

        // Capture the current buffer data with a binary param
        ParamCapture captureData("source", ParamType::TvoidConstPointer);

        AddressRange dirtyRange = coherentBuffer->getDirtyAddressRange(pageRange);
        CaptureMemory(reinterpret_cast<void *>(dirtyRange.start), dirtyRange.size, &captureData);
        dataParamBuffer.addParam(std::move(captureData));

        // Also track its size for use with memcpy
        dataParamBuffer.addValueParam<GLsizeiptr>("size", ParamType::TGLsizeiptr,
                                                  static_cast<GLsizeiptr>(dirtyRange.size));

        if (wholeRange.start != dirtyRange.start)
        {
            // Capture with offset
            GLsizeiptr offset = dirtyRange.start - wholeRange.start;

            ASSERT(offset > 0);

            // The dirty page range is not at the start of the buffer, track the offset.
            dataParamBuffer.addValueParam<GLsizeiptr>("offset", ParamType::TGLsizeiptr, offset);

            // Call the helper that populates the buffer with captured data
            mFrameCalls.emplace_back("UpdateClientBufferDataWithOffset",
                                     std::move(dataParamBuffer));
        }
        else
        {
            // Call the helper that populates the buffer with captured data
            mFrameCalls.emplace_back("UpdateClientBufferData", std::move(dataParamBuffer));
        }
    }
}

void FrameCaptureShared::captureMappedBufferSnapshot(const gl::Context *context,
                                                     const CallCapture &call)
{
    // If the buffer was mapped writable, we need to restore its data, since we have no
    // visibility into what the client did to the buffer while mapped.
    // This sequence will result in replay calls like this:
    //   ...
    //   gMappedBufferData[gBufferMap[42]] = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 65536,
    //                                                        GL_MAP_WRITE_BIT);
    //   ...
    //   UpdateClientBufferData(42, &gBinaryData[164631024], 65536);
    //   glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    //   ...

    // Re-map the buffer, using the info we tracked about the buffer
    gl::BufferBinding target =
        call.params.getParam("targetPacked", ParamType::TBufferBinding, 0).value.BufferBindingVal;

    FrameCaptureShared *frameCaptureShared = context->getShareGroup()->getFrameCaptureShared();
    gl::Buffer *buffer                     = context->getState().getTargetBuffer(target);
    if (!frameCaptureShared->hasBufferData(buffer->id()))
    {
        // This buffer was not marked writable, so we did not back it up
        return;
    }

    std::pair<GLintptr, GLsizeiptr> bufferDataOffsetAndLength =
        frameCaptureShared->getBufferDataOffsetAndLength(buffer->id());
    GLintptr offset   = bufferDataOffsetAndLength.first;
    GLsizeiptr length = bufferDataOffsetAndLength.second;

    // Map the buffer so we can copy its contents out
    ASSERT(!buffer->isMapped());
    angle::Result result = buffer->mapRange(context, offset, length, GL_MAP_READ_BIT);
    if (result != angle::Result::Continue)
    {
        ERR() << "Failed to mapRange of buffer" << std::endl;
    }
    const uint8_t *data = reinterpret_cast<const uint8_t *>(buffer->getMapPointer());

    // Create the parameters to our helper for use during replay
    ParamBuffer dataParamBuffer;

    // Pass in the target buffer ID
    dataParamBuffer.addValueParam("dest", ParamType::TGLuint, buffer->id().value);

    // Capture the current buffer data with a binary param
    ParamCapture captureData("source", ParamType::TvoidConstPointer);
    CaptureMemory(data, length, &captureData);
    dataParamBuffer.addParam(std::move(captureData));

    // Also track its size for use with memcpy
    dataParamBuffer.addValueParam<GLsizeiptr>("size", ParamType::TGLsizeiptr, length);

    // Call the helper that populates the buffer with captured data
    mFrameCalls.emplace_back("UpdateClientBufferData", std::move(dataParamBuffer));

    // Unmap the buffer and move on
    GLboolean dontCare;
    (void)buffer->unmap(context, &dontCare);
}

void FrameCaptureShared::checkForCaptureTrigger()
{
    // If the capture trigger has not been set, move on
    if (mCaptureTrigger == 0)
    {
        return;
    }

    // Otherwise, poll the value for a change
    std::string captureTriggerStr = GetCaptureTrigger();
    if (captureTriggerStr.empty())
    {
        return;
    }

    // If the value has changed, use the original value as the frame count
    // TODO (anglebug.com/42263521): Improve capture at unknown frame time. It is good to
    // avoid polling if the feature is not enabled, but not entirely intuitive to set
    // a value to zero when you want to trigger it.
    uint32_t captureTrigger = atoi(captureTriggerStr.c_str());
    if (captureTrigger != mCaptureTrigger)
    {
        // Start mid-execution capture for the current frame
        mCaptureStartFrame = mFrameIndex + 1;

        // Use the original trigger value as the frame count
        mCaptureEndFrame = mCaptureStartFrame + mCaptureTrigger - 1;

        INFO() << "Capture triggered after frame " << mFrameIndex << " for " << mCaptureTrigger
               << " frames";

        // Stop polling
        mCaptureTrigger = 0;
    }
}

void FrameCaptureShared::scanSetupCalls(std::vector<CallCapture> &setupCalls)
{
    // Scan all the instructions in the list for tracking
    for (CallCapture &call : setupCalls)
    {
        updateReadBufferSize(call.params.getReadBufferSize());
        updateResourceCountsFromCallCapture(call);
    }
}

void FrameCaptureShared::runMidExecutionCapture(gl::Context *mainContext)
{
    // Set the capture active to ensure all GLES commands issued by the next frame are
    // handled correctly by maybeCapturePreCallUpdates() and maybeCapturePostCallUpdates().
    setCaptureActive();

    // Make sure all pending work for every Context in the share group has completed so all data
    // (buffers, textures, etc.) has been updated and no resources are in use.
    egl::ShareGroup *shareGroup = mainContext->getShareGroup();
    shareGroup->finishAllContexts();

    const gl::State &contextState = mainContext->getState();
    gl::State mainContextReplayState(
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, contextState.getClientVersion(),
        false, true, true, true, false, EGL_CONTEXT_PRIORITY_MEDIUM_IMG,
        contextState.hasRobustAccess(), contextState.hasProtectedContent(), false);
    mainContextReplayState.initializeForCapture(mainContext);

    CaptureShareGroupMidExecutionSetup(mainContext, &mShareGroupSetupCalls, &mResourceTracker,
                                       mainContextReplayState, mMaxAccessedResourceIDs);

    scanSetupCalls(mShareGroupSetupCalls);

    egl::Display *display = mainContext->getDisplay();
    egl::Surface *draw    = mainContext->getCurrentDrawSurface();
    egl::Surface *read    = mainContext->getCurrentReadSurface();

    for (auto shareContext : shareGroup->getContexts())
    {
        FrameCapture *frameCapture = shareContext.second->getFrameCapture();
        ASSERT(frameCapture->getSetupCalls().empty());

        if (shareContext.second->id() == mainContext->id())
        {
            CaptureMidExecutionSetup(shareContext.second, &frameCapture->getSetupCalls(),
                                     frameCapture->getStateResetHelper(), &mShareGroupSetupCalls,
                                     &mResourceIDToSetupCalls, &mResourceTracker,
                                     mainContextReplayState, mValidateSerializedState);
            scanSetupCalls(frameCapture->getSetupCalls());

            std::stringstream protoStream;
            std::stringstream headerStream;
            std::stringstream bodyStream;

            protoStream << "void "
                        << FmtSetupFunction(kNoPartId, mainContext->id(), FuncUsage::Prototype);
            std::string proto = protoStream.str();

            WriteCppReplayFunctionWithParts(mainContext->id(), ReplayFunc::Setup, mReplayWriter, 1,
                                            &mBinaryData, frameCapture->getSetupCalls(),
                                            headerStream, bodyStream, &mResourceIDBufferSize);

            mReplayWriter.addPrivateFunction(proto, headerStream, bodyStream);
        }
        else
        {
            const gl::State &shareContextState = shareContext.second->getState();
            gl::State auxContextReplayState(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                            shareContextState.getClientVersion(), false, true, true,
                                            true, false, EGL_CONTEXT_PRIORITY_MEDIUM_IMG,
                                            shareContextState.hasRobustAccess(),
                                            shareContextState.hasProtectedContent(), false);
            auxContextReplayState.initializeForCapture(shareContext.second);

            egl::Error error = shareContext.second->makeCurrent(display, draw, read);
            if (error.isError())
            {
                INFO() << "MEC unable to make secondary context current";
            }

            CaptureMidExecutionSetup(shareContext.second, &frameCapture->getSetupCalls(),
                                     frameCapture->getStateResetHelper(), &mShareGroupSetupCalls,
                                     &mResourceIDToSetupCalls, &mResourceTracker,
                                     auxContextReplayState, mValidateSerializedState);

            scanSetupCalls(frameCapture->getSetupCalls());

            WriteAuxiliaryContextCppSetupReplay(
                mReplayWriter, mCompression, mOutDirectory, shareContext.second, mCaptureLabel, 1,
                frameCapture->getSetupCalls(), &mBinaryData, mSerializeStateEnabled, *this,
                &mResourceIDBufferSize);
        }
        // Track that this context was created before MEC started
        mActiveContexts.insert(shareContext.first);
    }

    egl::Error error = mainContext->makeCurrent(display, draw, read);
    if (error.isError())
    {
        INFO() << "MEC unable to make main context current again";
    }
}

void FrameCaptureShared::onEndFrame(gl::Context *context)
{
    if (!enabled() || mFrameIndex > mCaptureEndFrame)
    {
        setCaptureInactive();
        mCoherentBufferTracker.onEndFrame();
        return;
    }

    FrameCapture *frameCapture = context->getFrameCapture();

    // Count resource IDs. This is also done on every frame. It could probably be done by
    // checking the GL state instead of the calls.
    for (const CallCapture &call : mFrameCalls)
    {
        for (const ParamCapture &param : call.params.getParamCaptures())
        {
            ResourceIDType idType = GetResourceIDTypeFromParamType(param.type);
            if (idType != ResourceIDType::InvalidEnum)
            {
                mHasResourceType.set(idType);
            }
        }
    }

    mWindowSurfaceContextID = context->id();

    // On Android, we can trigger a capture during the run
    checkForCaptureTrigger();

    // Check for MEC. Done after checkForCaptureTrigger(), since that can modify mCaptureStartFrame.
    if (mFrameIndex < mCaptureStartFrame)
    {
        if (mFrameIndex == mCaptureStartFrame - 1)
        {
            // Trigger MEC.
            runMidExecutionCapture(context);
        }
        mFrameIndex++;
        reset();
        return;
    }

    ASSERT(isCaptureActive());

    if (!mFrameCalls.empty())
    {
        mActiveFrameIndices.push_back(getReplayFrameIndex());
    }

    // Make sure all pending work for every Context in the share group has completed so all data
    // (buffers, textures, etc.) has been updated and no resources are in use.
    egl::ShareGroup *shareGroup = context->getShareGroup();
    shareGroup->finishAllContexts();

    // Only validate the first frame for now to save on retracing time.
    if (mValidateSerializedState && mFrameIndex == mCaptureStartFrame)
    {
        CaptureValidateSerializedState(context, &mFrameCalls);
    }

    writeMainContextCppReplay(context, frameCapture->getSetupCalls(),
                              frameCapture->getStateResetHelper());

    if (mFrameIndex == mCaptureEndFrame)
    {
        // Write shared MEC after frame sequence so we can eliminate unused assets like programs
        WriteShareGroupCppSetupReplay(mReplayWriter, mCompression, mOutDirectory, mCaptureLabel, 1,
                                      1, mShareGroupSetupCalls, &mResourceTracker, &mBinaryData,
                                      mSerializeStateEnabled, mWindowSurfaceContextID,
                                      &mResourceIDBufferSize);

        // Save the index files after the last frame.
        writeCppReplayIndexFiles(context, false);
        SaveBinaryData(mCompression, mOutDirectory, kSharedContextId, mCaptureLabel, mBinaryData);
        mBinaryData.clear();
        mWroteIndexFile = true;
        INFO() << "Finished recording graphics API capture";
    }

    reset();
    mFrameIndex++;
}

void FrameCaptureShared::onDestroyContext(const gl::Context *context)
{
    if (!mEnabled)
    {
        return;
    }
    if (!mWroteIndexFile && mFrameIndex > mCaptureStartFrame)
    {
        // If context is destroyed before end frame is reached and at least
        // 1 frame has been recorded, then write the index files.
        // It doesn't make sense to write the index files when no frame has been recorded
        mFrameIndex -= 1;
        mCaptureEndFrame = mFrameIndex;
        writeCppReplayIndexFiles(context, true);
        SaveBinaryData(mCompression, mOutDirectory, kSharedContextId, mCaptureLabel, mBinaryData);
        mBinaryData.clear();
        mWroteIndexFile = true;
    }
}

void FrameCaptureShared::onMakeCurrent(const gl::Context *context, const egl::Surface *drawSurface)
{
    if (!drawSurface)
    {
        return;
    }

    // Track the width, height and color space of the draw surface as provided to makeCurrent
    SurfaceParams &params = mDrawSurfaceParams[context->id()];
    params.extents        = gl::Extents(drawSurface->getWidth(), drawSurface->getHeight(), 1);
    params.colorSpace     = egl::FromEGLenum<egl::ColorSpace>(drawSurface->getGLColorspace());
}

DataCounters::DataCounters() = default;

DataCounters::~DataCounters() = default;

int DataCounters::getAndIncrement(EntryPoint entryPoint, const std::string &paramName)
{
    Counter counterKey = {entryPoint, paramName};
    return mData[counterKey]++;
}

DataTracker::DataTracker() = default;

DataTracker::~DataTracker() = default;

StringCounters::StringCounters() = default;

StringCounters::~StringCounters() = default;

int StringCounters::getStringCounter(const std::vector<std::string> &strings)
{
    const auto &id = mStringCounterMap.find(strings);
    if (id == mStringCounterMap.end())
    {
        return kStringsNotFound;
    }
    else
    {
        return mStringCounterMap[strings];
    }
}

void StringCounters::setStringCounter(const std::vector<std::string> &strings, int &counter)
{
    ASSERT(counter >= 0);
    mStringCounterMap[strings] = counter;
}

TrackedResource::TrackedResource() = default;

TrackedResource::~TrackedResource() = default;

ResourceTracker::ResourceTracker() = default;

ResourceTracker::~ResourceTracker() = default;

StateResetHelper::StateResetHelper() = default;

StateResetHelper::~StateResetHelper() = default;

void StateResetHelper::setDefaultResetCalls(const gl::Context *context,
                                            angle::EntryPoint entryPoint)
{
    static const gl::BlendState kDefaultBlendState;

    // Populate default reset calls for entrypoints to support looping to beginning
    switch (entryPoint)
    {
        case angle::EntryPoint::GLUseProgram:
        {
            if (context->getActiveLinkedProgram() &&
                context->getActiveLinkedProgram()->id().value != 0)
            {
                Capture(&mResetCalls[angle::EntryPoint::GLUseProgram],
                        gl::CaptureUseProgram(context->getState(), true, {0}));
            }
            break;
        }
        case angle::EntryPoint::GLBindVertexArray:
        {
            if (context->getState().getVertexArray()->id().value != 0)
            {
                VertexArrayCaptureFuncs vertexArrayFuncs(context->isGLES1());
                Capture(&mResetCalls[angle::EntryPoint::GLBindVertexArray],
                        vertexArrayFuncs.bindVertexArray(context->getState(), true, {0}));
            }
            break;
        }
        case angle::EntryPoint::GLBlendFunc:
        {
            Capture(&mResetCalls[angle::EntryPoint::GLBlendFunc],
                    CaptureBlendFunc(context->getState(), true, kDefaultBlendState.sourceBlendRGB,
                                     kDefaultBlendState.destBlendRGB));
            break;
        }
        case angle::EntryPoint::GLBlendFuncSeparate:
        {
            Capture(&mResetCalls[angle::EntryPoint::GLBlendFuncSeparate],
                    CaptureBlendFuncSeparate(
                        context->getState(), true, kDefaultBlendState.sourceBlendRGB,
                        kDefaultBlendState.destBlendRGB, kDefaultBlendState.sourceBlendAlpha,
                        kDefaultBlendState.destBlendAlpha));
            break;
        }
        case angle::EntryPoint::GLBlendEquation:
        {
            UNREACHABLE();  // GLBlendEquationSeparate is always used instead
            break;
        }
        case angle::EntryPoint::GLBlendEquationSeparate:
        {
            Capture(&mResetCalls[angle::EntryPoint::GLBlendEquationSeparate],
                    CaptureBlendEquationSeparate(context->getState(), true,
                                                 kDefaultBlendState.blendEquationRGB,
                                                 kDefaultBlendState.blendEquationAlpha));
            break;
        }
        case angle::EntryPoint::GLColorMask:
        {
            Capture(&mResetCalls[angle::EntryPoint::GLColorMask],
                    CaptureColorMask(context->getState(), true,
                                     gl::ConvertToGLBoolean(kDefaultBlendState.colorMaskRed),
                                     gl::ConvertToGLBoolean(kDefaultBlendState.colorMaskGreen),
                                     gl::ConvertToGLBoolean(kDefaultBlendState.colorMaskBlue),
                                     gl::ConvertToGLBoolean(kDefaultBlendState.colorMaskAlpha)));
            break;
        }
        case angle::EntryPoint::GLBlendColor:
        {
            Capture(&mResetCalls[angle::EntryPoint::GLBlendColor],
                    CaptureBlendColor(context->getState(), true, 0, 0, 0, 0));
            break;
        }
        default:
            ERR() << "Unhandled entry point in setDefaultResetCalls: "
                  << GetEntryPointName(entryPoint);
            UNREACHABLE();
            break;
    }
}

void ResourceTracker::setDeletedFenceSync(gl::SyncID sync)
{
    ASSERT(sync.value != 0);
    if (mStartingFenceSyncs.find(sync) == mStartingFenceSyncs.end())
    {
        // This is a fence sync created after MEC was initialized. Ignore it.
        return;
    }

    // In this case, the app is deleting a fence sync we started with, we need to regen on loop.
    mFenceSyncsToRegen.insert(sync);
}

void ResourceTracker::setModifiedDefaultUniform(gl::ShaderProgramID programID,
                                                gl::UniformLocation location)
{
    // Pull up or create the list of uniform locations for this program and mark one dirty
    mDefaultUniformsToReset[programID].insert(location);
}

void ResourceTracker::setDefaultUniformBaseLocation(gl::ShaderProgramID programID,
                                                    gl::UniformLocation location,
                                                    gl::UniformLocation baseLocation)
{
    // Track the base location used to populate arrayed uniforms in Setup
    mDefaultUniformBaseLocations[{programID, location}] = baseLocation;
}

TrackedResource &ResourceTracker::getTrackedResource(gl::ContextID contextID, ResourceIDType type)
{
    if (IsSharedObjectResource(type))
    {
        // No need to index with context if shared
        return mTrackedResourcesShared[static_cast<uint32_t>(type)];
    }
    else
    {
        // For per-context objects, track the resource per-context
        return mTrackedResourcesPerContext[contextID][static_cast<uint32_t>(type)];
    }
}

void ResourceTracker::getContextIDs(std::set<gl::ContextID> &idsOut)
{
    for (const auto &trackedResourceIterator : mTrackedResourcesPerContext)
    {
        gl::ContextID contextID = trackedResourceIterator.first;
        idsOut.insert(contextID);
    }
}

void TrackedResource::setGennedResource(GLuint id)
{
    if (mStartingResources.find(id) == mStartingResources.end())
    {
        // This is a resource created after MEC was initialized, track it
        mNewResources.insert(id);
    }
    else
    {
        // In this case, the app is genning a resource with starting ID after previously deleting it
        ASSERT(mResourcesToRegen.find(id) != mResourcesToRegen.end());

        // For this, we need to delete it again to recreate it.
        mResourcesToDelete.insert(id);
    }
}

bool TrackedResource::resourceIsGenerated(GLuint id)
{
    return mStartingResources.find(id) != mStartingResources.end() ||
           mNewResources.find(id) != mNewResources.end();
}

void TrackedResource::setDeletedResource(GLuint id)
{
    if (id == 0)
    {
        // Ignore ID 0
        return;
    }

    if (mNewResources.find(id) != mNewResources.end())
    {
        // This is a resource created after MEC was initialized, just clear it, since there will be
        // no actions required for it to return to starting state.
        mNewResources.erase(id);
        return;
    }

    if (mStartingResources.find(id) != mStartingResources.end())
    {
        // In this case, the app is deleting a resource we started with, we need to regen on loop

        // Mark that we don't need to delete this
        mResourcesToDelete.erase(id);

        // Generate the resource again
        mResourcesToRegen.insert(id);

        // Also restore its contents
        mResourcesToRestore.insert(id);
    }

    // If none of the above is true, the app is deleting a resource that was never genned.
}

void TrackedResource::setModifiedResource(GLuint id)
{
    // If this was a starting resource, we need to track it for restore
    if (mStartingResources.find(id) != mStartingResources.end())
    {
        mResourcesToRestore.insert(id);
    }
}

void ResourceTracker::setBufferMapped(gl::ContextID contextID, GLuint id)
{
    // If this was a starting buffer, we may need to restore it to original state during Reset.
    // Skip buffers that were deleted after the starting point.
    const TrackedResource &trackedBuffers = getTrackedResource(contextID, ResourceIDType::Buffer);
    const ResourceSet &startingBuffers    = trackedBuffers.getStartingResources();
    const ResourceSet &buffersToRegen     = trackedBuffers.getResourcesToRegen();
    if (startingBuffers.find(id) != startingBuffers.end() &&
        buffersToRegen.find(id) == buffersToRegen.end())
    {
        // Track that its current state is mapped (true)
        mStartingBuffersMappedCurrent[id] = true;
    }
}

void ResourceTracker::setBufferUnmapped(gl::ContextID contextID, GLuint id)
{
    // If this was a starting buffer, we may need to restore it to original state during Reset.
    // Skip buffers that were deleted after the starting point.
    const TrackedResource &trackedBuffers = getTrackedResource(contextID, ResourceIDType::Buffer);
    const ResourceSet &startingBuffers    = trackedBuffers.getStartingResources();
    const ResourceSet &buffersToRegen     = trackedBuffers.getResourcesToRegen();
    if (startingBuffers.find(id) != startingBuffers.end() &&
        buffersToRegen.find(id) == buffersToRegen.end())
    {
        // Track that its current state is unmapped (false)
        mStartingBuffersMappedCurrent[id] = false;
    }
}

bool ResourceTracker::getStartingBuffersMappedCurrent(GLuint id) const
{
    const auto &foundBool = mStartingBuffersMappedCurrent.find(id);
    ASSERT(foundBool != mStartingBuffersMappedCurrent.end());
    return foundBool->second;
}

bool ResourceTracker::getStartingBuffersMappedInitial(GLuint id) const
{
    const auto &foundBool = mStartingBuffersMappedInitial.find(id);
    ASSERT(foundBool != mStartingBuffersMappedInitial.end());
    return foundBool->second;
}

void ResourceTracker::onShaderProgramAccess(gl::ShaderProgramID shaderProgramID)
{
    mMaxShaderPrograms = std::max(mMaxShaderPrograms, shaderProgramID.value + 1);
}

bool FrameCaptureShared::isCapturing() const
{
    // Currently we will always do a capture up until the last frame. In the future we could improve
    // mid execution capture by only capturing between the start and end frames. The only necessary
    // reason we need to capture before the start is for attached program and shader sources.
    return mEnabled && mFrameIndex <= mCaptureEndFrame;
}

uint32_t FrameCaptureShared::getFrameCount() const
{
    return mCaptureEndFrame - mCaptureStartFrame + 1;
}

uint32_t FrameCaptureShared::getReplayFrameIndex() const
{
    return mFrameIndex - mCaptureStartFrame + 1;
}

// Serialize trace metadata into a JSON file. The JSON file will be named "trace_prefix.json".
//
// As of writing, it will have the format like so:
// {
//     "TraceMetadata":
//     {
//         "AreClientArraysEnabled" : 1, "CaptureRevision" : 16631, "ConfigAlphaBits" : 8,
//             "ConfigBlueBits" : 8, "ConfigDepthBits" : 24, "ConfigGreenBits" : 8,
// ... etc ...
void FrameCaptureShared::writeJSON(const gl::Context *context)
{
    const gl::ContextID contextId           = context->id();
    const SurfaceParams &surfaceParams      = mDrawSurfaceParams.at(contextId);
    const gl::State &glState                = context->getState();
    const egl::Config *config               = context->getConfig();
    const egl::AttributeMap &displayAttribs = context->getDisplay()->getAttributeMap();

    unsigned int frameCount = getFrameCount();

    JsonSerializer json;
    json.startGroup("TraceMetadata");
    json.addScalar("CaptureRevision", GetANGLERevision());
    json.addScalar("ContextClientMajorVersion", context->getClientMajorVersion());
    json.addScalar("ContextClientMinorVersion", context->getClientMinorVersion());
    json.addHexValue("DisplayPlatformType", displayAttribs.getAsInt(EGL_PLATFORM_ANGLE_TYPE_ANGLE));
    json.addHexValue("DisplayDeviceType",
                     displayAttribs.getAsInt(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE));
    json.addScalar("FrameStart", 1);
    json.addScalar("FrameEnd", frameCount);
    json.addScalar("DrawSurfaceWidth", surfaceParams.extents.width);
    json.addScalar("DrawSurfaceHeight", surfaceParams.extents.height);
    json.addHexValue("DrawSurfaceColorSpace", ToEGLenum(surfaceParams.colorSpace));
    if (config)
    {
        json.addScalar("ConfigRedBits", config->redSize);
        json.addScalar("ConfigGreenBits", config->greenSize);
        json.addScalar("ConfigBlueBits", config->blueSize);
        json.addScalar("ConfigAlphaBits", config->alphaSize);
        json.addScalar("ConfigDepthBits", config->depthSize);
        json.addScalar("ConfigStencilBits", config->stencilSize);
    }
    else
    {
        json.addScalar("ConfigRedBits", EGL_DONT_CARE);
        json.addScalar("ConfigGreenBits", EGL_DONT_CARE);
        json.addScalar("ConfigBlueBits", EGL_DONT_CARE);
        json.addScalar("ConfigAlphaBits", EGL_DONT_CARE);
        json.addScalar("ConfigDepthBits", EGL_DONT_CARE);
        json.addScalar("ConfigStencilBits", EGL_DONT_CARE);
    }
    json.addBool("IsBinaryDataCompressed", mCompression);
    json.addBool("AreClientArraysEnabled", glState.areClientArraysEnabled());
    json.addBool("IsBindGeneratesResourcesEnabled", glState.isBindGeneratesResourceEnabled());
    json.addBool("IsWebGLCompatibilityEnabled", glState.isWebGL());
    json.addBool("IsRobustResourceInitEnabled", glState.isRobustResourceInitEnabled());
    json.endGroup();

    {
        const std::vector<std::string> &traceFiles = mReplayWriter.getAndResetWrittenFiles();
        json.addVectorOfStrings("TraceFiles", traceFiles);
    }

    json.addScalar("WindowSurfaceContextID", contextId.value);

    {
        std::stringstream jsonFileNameStream;
        jsonFileNameStream << mOutDirectory << FmtCapturePrefix(kNoContextId, mCaptureLabel)
                           << ".json";
        std::string jsonFileName = jsonFileNameStream.str();

        SaveFileHelper saveData(jsonFileName);
        saveData.write(reinterpret_cast<const uint8_t *>(json.data()), json.length());
    }
}

void FrameCaptureShared::writeCppReplayIndexFiles(const gl::Context *context,
                                                  bool writeResetContextCall)
{
    // Ensure the last frame is written. This will no-op if the frame is already written.
    mReplayWriter.saveFrame();

    const gl::ContextID contextId = context->id();

    {
        std::stringstream header;

        header << "#pragma once\n";
        header << "\n";
        header << "#include <EGL/egl.h>\n";
        header << "#include <stdint.h>\n";

        std::string includes = header.str();
        mReplayWriter.setHeaderPrologue(includes);
    }

    {
        std::stringstream source;

        source << "#include \"" << FmtCapturePrefix(contextId, mCaptureLabel) << ".h\"\n";
        source << "#include \"trace_fixture.h\"\n";
        source << "#include \"angle_trace_gl.h\"\n";

        std::string sourcePrologue = source.str();
        mReplayWriter.setSourcePrologue(sourcePrologue);
    }

    {
        std::string proto = "void InitReplay(void)";

        std::stringstream source;
        source << proto << "\n";
        source << "{\n";
        WriteInitReplayCall(mCompression, source, context->id(), mCaptureLabel,
                            MaxClientArraySize(mClientArraySizes), mReadBufferSize,
                            mResourceIDBufferSize, mMaxAccessedResourceIDs);
        source << "}\n";

        mReplayWriter.addPrivateFunction(proto, std::stringstream(), source);
    }

    {
        std::string proto = "void ReplayFrame(uint32_t frameIndex)";

        std::stringstream source;

        source << proto << "\n";
        source << "{\n";
        source << "    switch (frameIndex)\n";
        source << "    {\n";
        for (uint32_t frameIndex : mActiveFrameIndices)
        {
            source << "        case " << frameIndex << ":\n";
            source << "            " << FmtReplayFunction(contextId, FuncUsage::Call, frameIndex)
                   << ";\n";
            source << "            break;\n";
        }
        source << "        default:\n";
        source << "            break;\n";
        source << "    }\n";
        source << "}\n";

        mReplayWriter.addPublicFunction(proto, std::stringstream(), source);
    }

    if (writeResetContextCall)
    {
        std::string proto = "void ResetReplay(void)";

        std::stringstream source;

        source << proto << "\n";
        source << "{\n";
        source << "    // Reset context is empty because context is destroyed before end "
                  "frame is reached\n";
        source << "}\n";

        mReplayWriter.addPublicFunction(proto, std::stringstream(), source);
    }

    if (mSerializeStateEnabled)
    {
        std::string proto = "const char *GetSerializedContextState(uint32_t frameIndex)";

        std::stringstream source;

        source << proto << "\n";
        source << "{\n";
        source << "    switch (frameIndex)\n";
        source << "    {\n";
        for (uint32_t frameIndex = 1; frameIndex <= getFrameCount(); ++frameIndex)
        {
            source << "        case " << frameIndex << ":\n";
            source << "            return "
                   << FmtGetSerializedContextStateFunction(contextId, FuncUsage::Call, frameIndex)
                   << ";\n";
        }
        source << "        default:\n";
        source << "            return NULL;\n";
        source << "    }\n";
        source << "}\n";

        mReplayWriter.addPublicFunction(proto, std::stringstream(), source);
    }

    {
        std::stringstream fnameStream;
        fnameStream << mOutDirectory << FmtCapturePrefix(contextId, mCaptureLabel);
        std::string fnamePattern = fnameStream.str();

        mReplayWriter.setFilenamePattern(fnamePattern);
    }

    mReplayWriter.saveIndexFilesAndHeader();

    writeJSON(context);
}

void FrameCaptureShared::writeMainContextCppReplay(const gl::Context *context,
                                                   const std::vector<CallCapture> &setupCalls,
                                                   StateResetHelper &stateResetHelper)
{
    ASSERT(mWindowSurfaceContextID == context->id());

    {
        std::stringstream header;

        header << "#include \"" << FmtCapturePrefix(context->id(), mCaptureLabel) << ".h\"\n";
        header << "#include \"angle_trace_gl.h\"\n";

        std::string headerString = header.str();
        mReplayWriter.setSourcePrologue(headerString);
    }

    uint32_t frameCount = getFrameCount();
    uint32_t frameIndex = getReplayFrameIndex();

    if (frameIndex == 1)
    {
        {
            std::string proto = "void SetupReplay(void)";

            std::stringstream out;

            out << proto << "\n";
            out << "{\n";

            // Setup all of the shared objects.
            out << "    InitReplay();\n";
            if (usesMidExecutionCapture())
            {
                out << "    " << FmtSetupFunction(kNoPartId, kSharedContextId, FuncUsage::Call)
                    << ";\n";
                out << "    "
                    << FmtSetupInactiveFunction(kNoPartId, kSharedContextId, FuncUsage::Call)
                    << "\n";
                // Make sure that the current context is mapped correctly
                out << "    SetCurrentContextID(" << context->id() << ");\n";
            }

            // Setup each of the auxiliary contexts.
            egl::ShareGroup *shareGroup            = context->getShareGroup();
            const egl::ContextMap &shareContextMap = shareGroup->getContexts();
            for (auto shareContext : shareContextMap)
            {
                if (shareContext.first == context->id().value)
                {
                    if (usesMidExecutionCapture())
                    {
                        // Setup the presentation (this) context first.
                        out << "    " << FmtSetupFunction(kNoPartId, context->id(), FuncUsage::Call)
                            << ";\n";
                        out << "\n";
                    }

                    continue;
                }

                // The SetupReplayContextXX() calls only exist if this is a mid-execution capture
                // and we can only call them if they exist, so only output the calls if this is a
                // MEC.
                if (usesMidExecutionCapture())
                {
                    // Only call SetupReplayContext for secondary contexts that were current before
                    // MEC started
                    if (mActiveContexts.find(shareContext.first) != mActiveContexts.end())
                    {
                        // TODO(http://anglebug.com/42264418): Support capture/replay of
                        // eglCreateContext() so this block can be moved into SetupReplayContextXX()
                        // by injecting them into the beginning of the setup call stream.
                        out << "    CreateContext(" << shareContext.first << ");\n";

                        out << "    "
                            << FmtSetupFunction(kNoPartId, shareContext.second->id(),
                                                FuncUsage::Call)
                            << ";\n";
                    }
                }
            }

            // If there are other contexts that were initialized, we need to make the main context
            // current again.
            if (shareContextMap.size() > 1)
            {
                out << "\n";
                out << "    eglMakeCurrent(NULL, NULL, NULL, gContextMap2[" << context->id()
                    << "]);\n";
            }

            out << "}\n";

            mReplayWriter.addPublicFunction(proto, std::stringstream(), out);
        }
    }

    // Emit code to reset back to starting state
    if (frameIndex == frameCount)
    {
        std::stringstream resetProtoStream;
        std::stringstream resetHeaderStream;
        std::stringstream resetBodyStream;

        resetProtoStream << "void ResetReplay(void)";

        resetBodyStream << resetProtoStream.str() << "\n";
        resetBodyStream << "{\n";

        // Grab the list of contexts to be reset
        std::set<gl::ContextID> contextIDs;
        mResourceTracker.getContextIDs(contextIDs);

        // TODO(http://anglebug.com/42264418): Look at moving this into the shared context file
        // since it's resetting shared objects.

        // TODO(http://anglebug.com/42263204): Support function parts when writing Reset functions

        // Track whether anything was written during Reset
        bool anyResourceReset = false;

        // Track whether we changed contexts during Reset
        bool contextChanged = false;

        // First emit shared object reset, including opaque and context state
        {
            std::stringstream protoStream;
            std::stringstream headerStream;
            std::stringstream bodyStream;

            protoStream << "void "
                        << FmtResetFunction(kNoPartId, kSharedContextId, FuncUsage::Prototype);
            bodyStream << protoStream.str() << "\n";
            bodyStream << "{\n";

            for (ResourceIDType resourceType : AllEnums<ResourceIDType>())
            {
                if (!IsSharedObjectResource(resourceType))
                {
                    continue;
                }
                // Use current context for shared reset
                MaybeResetResources(context->getDisplay(), context->id(), resourceType,
                                    mReplayWriter, bodyStream, headerStream, &mResourceTracker,
                                    &mBinaryData, anyResourceReset, &mResourceIDBufferSize);
            }

            // Reset opaque type objects that don't have IDs, so are not ResourceIDTypes.
            MaybeResetOpaqueTypeObjects(mReplayWriter, bodyStream, headerStream, context,
                                        &mResourceTracker, &mBinaryData, &mResourceIDBufferSize);

            bodyStream << "}\n";

            mReplayWriter.addPrivateFunction(protoStream.str(), headerStream, bodyStream);
        }

        // Emit the call to shared object reset
        resetBodyStream << "    " << FmtResetFunction(kNoPartId, kSharedContextId, FuncUsage::Call)
                        << ";\n";

        // Reset our output tracker (Note: This was unused during shared reset)
        anyResourceReset = false;

        // Walk through all contexts that need Reset
        for (const gl::ContextID &contextID : contextIDs)
        {
            // Create a function to reset each context's non-shared objects
            {
                std::stringstream protoStream;
                std::stringstream headerStream;
                std::stringstream bodyStream;

                protoStream << "void "
                            << FmtResetFunction(kNoPartId, contextID, FuncUsage::Prototype);
                bodyStream << protoStream.str() << "\n";
                bodyStream << "{\n";

                // Build the Reset calls in a separate stream so we can insert before them
                std::stringstream resetStream;

                for (ResourceIDType resourceType : AllEnums<ResourceIDType>())
                {
                    if (IsSharedObjectResource(resourceType))
                    {
                        continue;
                    }
                    MaybeResetResources(context->getDisplay(), contextID, resourceType,
                                        mReplayWriter, resetStream, headerStream, &mResourceTracker,
                                        &mBinaryData, anyResourceReset, &mResourceIDBufferSize);
                }

                // Only call eglMakeCurrent if anything was actually reset in the function and the
                // context differs from current
                if (anyResourceReset && contextID != context->id())
                {
                    contextChanged = true;
                    bodyStream << "    eglMakeCurrent(NULL, NULL, NULL, gContextMap2["
                               << contextID.value << "]);\n\n";
                }

                // Then append the Reset calls
                bodyStream << resetStream.str();

                bodyStream << "}\n";
                mReplayWriter.addPrivateFunction(protoStream.str(), headerStream, bodyStream);
            }

            // Emit a call to reset each context's non-shared objects
            resetBodyStream << "    " << FmtResetFunction(kNoPartId, contextID, FuncUsage::Call)
                            << ";\n";
        }

        // Bind the main context again if we bound any additional contexts
        if (contextChanged)
        {
            resetBodyStream << "    eglMakeCurrent(NULL, NULL, NULL, gContextMap2["
                            << context->id().value << "]);\n";
        }

        // Now that we're back on the main context, reset any additional state
        resetBodyStream << "\n    // Reset main context state\n";
        MaybeResetContextState(mReplayWriter, resetBodyStream, resetHeaderStream, &mResourceTracker,
                               context, &mBinaryData, stateResetHelper, &mResourceIDBufferSize);

        resetBodyStream << "}\n";

        mReplayWriter.addPublicFunction(resetProtoStream.str(), resetHeaderStream, resetBodyStream);
    }

    if (!mFrameCalls.empty())
    {
        std::stringstream protoStream;
        protoStream << "void "
                    << FmtReplayFunction(context->id(), FuncUsage::Prototype, frameIndex);
        std::string proto = protoStream.str();
        std::stringstream headerStream;
        std::stringstream bodyStream;

        if (context->getShareGroup()->getContexts().size() > 1)
        {
            // Only ReplayFunc::Replay trace file output functions are affected by multi-context
            // call grouping so they can safely be special-cased here.
            WriteCppReplayFunctionWithPartsMultiContext(
                context->id(), ReplayFunc::Replay, mReplayWriter, frameIndex, &mBinaryData,
                mFrameCalls, headerStream, bodyStream, &mResourceIDBufferSize);
        }
        else
        {
            WriteCppReplayFunctionWithParts(context->id(), ReplayFunc::Replay, mReplayWriter,
                                            frameIndex, &mBinaryData, mFrameCalls, headerStream,
                                            bodyStream, &mResourceIDBufferSize);
        }
        mReplayWriter.addPrivateFunction(proto, headerStream, bodyStream);
    }

    if (mSerializeStateEnabled)
    {
        std::string serializedContextString;
        if (SerializeContextToString(const_cast<gl::Context *>(context),
                                     &serializedContextString) == Result::Continue)
        {
            std::stringstream protoStream;
            protoStream << "const char *"
                        << FmtGetSerializedContextStateFunction(context->id(), FuncUsage::Prototype,
                                                                frameIndex);
            std::string proto = protoStream.str();

            std::stringstream bodyStream;
            bodyStream << proto << "\n";
            bodyStream << "{\n";
            bodyStream << "    return " << FmtMultiLineString(serializedContextString) << ";\n";
            bodyStream << "}\n";

            mReplayWriter.addPrivateFunction(proto, std::stringstream(), bodyStream);
        }
    }

    {
        std::stringstream fnamePatternStream;
        fnamePatternStream << mOutDirectory << FmtCapturePrefix(context->id(), mCaptureLabel);
        std::string fnamePattern = fnamePatternStream.str();

        mReplayWriter.setFilenamePattern(fnamePattern);
    }

    if (mFrameIndex == mCaptureEndFrame)
    {
        mReplayWriter.saveFrame();
    }
    else
    {
        mReplayWriter.saveFrameIfFull();
    }
}

void FrameCaptureShared::reset()
{
    mFrameCalls.clear();
    mClientVertexArrayMap.fill(-1);

    // Do not reset replay-specific settings like the maximum read buffer size, client array sizes,
    // or the 'has seen' type map. We could refine this into per-frame and per-capture maximums if
    // necessary.
}

const std::string &FrameCaptureShared::getShaderSource(gl::ShaderProgramID id) const
{
    const auto &foundSources = mCachedShaderSource.find(id);
    ASSERT(foundSources != mCachedShaderSource.end());
    return foundSources->second;
}

void FrameCaptureShared::setShaderSource(gl::ShaderProgramID id, std::string source)
{
    mCachedShaderSource[id] = source;
}

const ProgramSources &FrameCaptureShared::getProgramSources(gl::ShaderProgramID id) const
{
    const auto &foundSources = mCachedProgramSources.find(id);
    ASSERT(foundSources != mCachedProgramSources.end());
    return foundSources->second;
}

void FrameCaptureShared::setProgramSources(gl::ShaderProgramID id, ProgramSources sources)
{
    mCachedProgramSources[id] = sources;
}

void FrameCaptureShared::markResourceSetupCallsInactive(std::vector<CallCapture> *setupCalls,
                                                        ResourceIDType type,
                                                        GLuint id,
                                                        gl::Range<size_t> range)
{
    ASSERT(mResourceIDToSetupCalls[type].find(id) == mResourceIDToSetupCalls[type].end());

    // Mark all of the calls that were used to initialize this resource as INACTIVE
    for (size_t index : range)
    {
        (*setupCalls)[index].isActive = false;
    }

    mResourceIDToSetupCalls[type][id] = range;
}

void CaptureMemory(const void *source, size_t size, ParamCapture *paramCapture)
{
    std::vector<uint8_t> data(size);
    memcpy(data.data(), source, size);
    paramCapture->data.emplace_back(std::move(data));
}

void CaptureString(const GLchar *str, ParamCapture *paramCapture)
{
    // include the '\0' suffix
    CaptureMemory(str, strlen(str) + 1, paramCapture);
}

void CaptureStringLimit(const GLchar *str, uint32_t limit, ParamCapture *paramCapture)
{
    // Write the incoming string up to limit, including null terminator
    size_t length = strlen(str) + 1;

    if (length > limit)
    {
        // If too many characters, resize the string to fit in the limit
        std::string newStr = str;
        newStr.resize(limit - 1);
        CaptureString(newStr.c_str(), paramCapture);
    }
    else
    {
        CaptureMemory(str, length, paramCapture);
    }
}

void CaptureVertexPointerGLES1(const gl::State &glState,
                               gl::ClientVertexArrayType type,
                               const void *pointer,
                               ParamCapture *paramCapture)
{
    paramCapture->value.voidConstPointerVal = pointer;
    if (!glState.getTargetBuffer(gl::BufferBinding::Array))
    {
        paramCapture->arrayClientPointerIndex =
            gl::GLES1Renderer::VertexArrayIndex(type, glState.gles1());
    }
}

gl::Program *GetProgramForCapture(const gl::State &glState, gl::ShaderProgramID handle)
{
    gl::Program *program = glState.getShaderProgramManagerForCapture().getProgram(handle);
    return program;
}

void CaptureGetActiveUniformBlockivParameters(const gl::State &glState,
                                              gl::ShaderProgramID handle,
                                              gl::UniformBlockIndex uniformBlockIndex,
                                              GLenum pname,
                                              ParamCapture *paramCapture)
{
    int numParams = 1;

    // From the OpenGL ES 3.0 spec:
    // If pname is UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, then a list of the
    // active uniform indices for the uniform block identified by uniformBlockIndex is
    // returned. The number of elements that will be written to params is the value of
    // UNIFORM_BLOCK_ACTIVE_UNIFORMS for uniformBlockIndex
    if (pname == GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES)
    {
        gl::Program *program = GetProgramForCapture(glState, handle);
        if (program)
        {
            gl::QueryActiveUniformBlockiv(program, uniformBlockIndex,
                                          GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numParams);
        }
    }

    paramCapture->readBufferSizeBytes = sizeof(GLint) * numParams;
}

void CaptureGetParameter(const gl::State &glState,
                         GLenum pname,
                         size_t typeSize,
                         ParamCapture *paramCapture)
{
    // kMaxReportedCapabilities is the biggest array we'll need to hold data from glGet calls.
    // This value needs to be updated if any new extensions are introduced that would allow for
    // more compressed texture formats. The current value is taken from:
    // http://opengles.gpuinfo.org/displaycapability.php?name=GL_NUM_COMPRESSED_TEXTURE_FORMATS&esversion=2
    constexpr unsigned int kMaxReportedCapabilities = 69;
    paramCapture->readBufferSizeBytes               = typeSize * kMaxReportedCapabilities;
}

void CaptureGenHandlesImpl(GLsizei n, GLuint *handles, ParamCapture *paramCapture)
{
    paramCapture->readBufferSizeBytes = sizeof(GLuint) * n;
    CaptureMemory(handles, paramCapture->readBufferSizeBytes, paramCapture);
}

void CaptureShaderStrings(GLsizei count,
                          const GLchar *const *strings,
                          const GLint *length,
                          ParamCapture *paramCapture)
{
    // Concat the array elements of the string into one data vector,
    // append the terminating zero and use this as the captured shader
    // string. The string count and the length array are adjusted
    // accordingly in the capture post-processing

    std::vector<uint8_t> data;
    size_t offset = 0;
    for (GLsizei index = 0; index < count; ++index)
    {
        size_t len = ((length && length[index] >= 0) ? length[index] : strlen(strings[index]));

        // Count trailing zeros
        uint32_t i = 1;
        while (i < len && strings[index][len - i] == 0)
        {
            i++;
        }

        // Don't copy trailing zeros
        len -= (i - 1);

        data.resize(offset + len);
        std::copy(strings[index], strings[index] + len, data.begin() + offset);
        offset += len;
    }

    data.push_back(0);
    paramCapture->data.emplace_back(std::move(data));
}

// ReplayWriter implementation.
ReplayWriter::ReplayWriter()
    : mSourceFileExtension(kDefaultSourceFileExt),
      mSourceFileSizeThreshold(kDefaultSourceFileSizeThreshold),
      mFrameIndex(1)
{}

ReplayWriter::~ReplayWriter()
{
    ASSERT(mPrivateFunctionPrototypes.empty());
    ASSERT(mPublicFunctionPrototypes.empty());
    ASSERT(mPrivateFunctions.empty());
    ASSERT(mPublicFunctions.empty());
    ASSERT(mGlobalVariableDeclarations.empty());
    ASSERT(mReplayHeaders.empty());
}

void ReplayWriter::setSourceFileExtension(const char *ext)
{
    mSourceFileExtension = ext;
}

void ReplayWriter::setSourceFileSizeThreshold(size_t sourceFileSizeThreshold)
{
    mSourceFileSizeThreshold = sourceFileSizeThreshold;
}

void ReplayWriter::setFilenamePattern(const std::string &pattern)
{
    if (mFilenamePattern != pattern)
    {
        mFilenamePattern = pattern;
    }
}

void ReplayWriter::setCaptureLabel(const std::string &label)
{
    mCaptureLabel = label;
}

void ReplayWriter::setSourcePrologue(const std::string &prologue)
{
    mSourcePrologue = prologue;
}

void ReplayWriter::setHeaderPrologue(const std::string &prologue)
{
    mHeaderPrologue = prologue;
}

void ReplayWriter::addPublicFunction(const std::string &functionProto,
                                     const std::stringstream &headerStream,
                                     const std::stringstream &bodyStream)
{
    mPublicFunctionPrototypes.push_back(functionProto);

    std::string header = headerStream.str();
    std::string body   = bodyStream.str();

    if (!header.empty())
    {
        mReplayHeaders.emplace_back(header);
    }

    if (!body.empty())
    {
        mPublicFunctions.emplace_back(body);
    }
}

void ReplayWriter::addPrivateFunction(const std::string &functionProto,
                                      const std::stringstream &headerStream,
                                      const std::stringstream &bodyStream)
{
    mPrivateFunctionPrototypes.push_back(functionProto);

    std::string header = headerStream.str();
    std::string body   = bodyStream.str();

    if (!header.empty())
    {
        mReplayHeaders.emplace_back(header);
    }

    if (!body.empty())
    {
        mPrivateFunctions.emplace_back(body);
    }
}

std::string ReplayWriter::getInlineVariableName(EntryPoint entryPoint, const std::string &paramName)
{
    int counter = mDataTracker.getCounters().getAndIncrement(entryPoint, paramName);
    return GetVarName(entryPoint, paramName, counter);
}

std::string ReplayWriter::getInlineStringSetVariableName(EntryPoint entryPoint,
                                                         const std::string &paramName,
                                                         const std::vector<std::string> &strings,
                                                         bool *isNewEntryOut)
{
    int counter    = mDataTracker.getStringCounters().getStringCounter(strings);
    *isNewEntryOut = (counter == kStringsNotFound);
    if (*isNewEntryOut)
    {
        // This is a unique set of strings, so set up their declaration and update the counter
        counter = mDataTracker.getCounters().getAndIncrement(entryPoint, paramName);
        mDataTracker.getStringCounters().setStringCounter(strings, counter);

        std::string varName = GetVarName(entryPoint, paramName, counter);

        std::stringstream declStream;
        declStream << "const char *const " << varName << "[]";
        std::string decl = declStream.str();

        mGlobalVariableDeclarations.push_back(decl);

        return varName;
    }
    else
    {
        return GetVarName(entryPoint, paramName, counter);
    }
}

size_t ReplayWriter::getStoredReplaySourceSize() const
{
    size_t sum = 0;
    for (const std::string &header : mReplayHeaders)
    {
        sum += header.size();
    }
    for (const std::string &publicFunc : mPublicFunctions)
    {
        sum += publicFunc.size();
    }
    for (const std::string &privateFunc : mPrivateFunctions)
    {
        sum += privateFunc.size();
    }
    return sum;
}

// static
std::string ReplayWriter::GetVarName(EntryPoint entryPoint,
                                     const std::string &paramName,
                                     int counter)
{
    std::stringstream strstr;
    strstr << GetEntryPointName(entryPoint) << "_" << paramName << "_" << counter;
    return strstr.str();
}

void ReplayWriter::saveFrame()
{
    if (mReplayHeaders.empty() && mPublicFunctions.empty() && mPrivateFunctions.empty())
    {
        return;
    }

    ASSERT(!mSourceFileExtension.empty());

    std::stringstream strstr;
    strstr << mFilenamePattern << "_" << std::setfill('0') << std::setw(3) << mFrameIndex++ << "."
           << mSourceFileExtension;

    std::string frameFilePath = strstr.str();

    writeReplaySource(frameFilePath);
}

void ReplayWriter::saveFrameIfFull()
{
    if (getStoredReplaySourceSize() < mSourceFileSizeThreshold)
    {
        INFO() << "Merging captured frame: " << getStoredReplaySourceSize()
               << " less than threshold of " << mSourceFileSizeThreshold << " bytes";
        return;
    }

    saveFrame();
}

void ReplayWriter::saveHeader()
{
    std::stringstream headerPathStream;
    headerPathStream << mFilenamePattern << ".h";
    std::string headerPath = headerPathStream.str();

    SaveFileHelper saveH(headerPath);

    saveH << mHeaderPrologue << "\n";

    saveH << "// Public functions are declared in trace_fixture.h.\n";
    saveH << "\n";
    saveH << "// Private Functions\n";
    saveH << "\n";

    for (const std::string &proto : mPrivateFunctionPrototypes)
    {
        saveH << proto << ";\n";
    }

    saveH << "\n";
    saveH << "// Global variables\n";
    saveH << "\n";

    for (const std::string &globalVar : mGlobalVariableDeclarations)
    {
        saveH << "extern " << globalVar << ";\n";
    }

    mPublicFunctionPrototypes.clear();
    mPrivateFunctionPrototypes.clear();
    mGlobalVariableDeclarations.clear();

    addWrittenFile(headerPath);
}

void ReplayWriter::saveIndexFilesAndHeader()
{
    ASSERT(!mSourceFileExtension.empty());

    std::stringstream sourcePathStream;
    sourcePathStream << mFilenamePattern << "." << mSourceFileExtension;
    std::string sourcePath = sourcePathStream.str();

    writeReplaySource(sourcePath);
    saveHeader();
}

void ReplayWriter::saveSetupFile()
{
    ASSERT(!mSourceFileExtension.empty());

    std::stringstream strstr;
    strstr << mFilenamePattern << "." << mSourceFileExtension;

    std::string frameFilePath = strstr.str();

    writeReplaySource(frameFilePath);
}

void ReplayWriter::writeReplaySource(const std::string &filename)
{
    SaveFileHelper saveCpp(filename);

    saveCpp << mSourcePrologue << "\n";
    for (const std::string &header : mReplayHeaders)
    {
        saveCpp << header << "\n";
    }

    saveCpp << "// Private Functions\n";
    saveCpp << "\n";

    for (const std::string &func : mPrivateFunctions)
    {
        saveCpp << func << "\n";
    }

    saveCpp << "// Public Functions\n";
    saveCpp << "\n";

    if (mFilenamePattern == "cpp")
    {
        saveCpp << "extern \"C\"\n";
        saveCpp << "{\n";
    }

    for (const std::string &func : mPublicFunctions)
    {
        saveCpp << func << "\n";
    }

    if (mFilenamePattern == "cpp")
    {
        saveCpp << "}  // extern \"C\"\n";
    }

    mReplayHeaders.clear();
    mPrivateFunctions.clear();
    mPublicFunctions.clear();

    addWrittenFile(filename);
}

void ReplayWriter::addWrittenFile(const std::string &filename)
{
    std::string writtenFile = GetBaseName(filename);
    ASSERT(std::find(mWrittenFiles.begin(), mWrittenFiles.end(), writtenFile) ==
           mWrittenFiles.end());
    mWrittenFiles.push_back(writtenFile);
}

std::vector<std::string> ReplayWriter::getAndResetWrittenFiles()
{
    std::vector<std::string> results = std::move(mWrittenFiles);
    std::sort(results.begin(), results.end());
    ASSERT(mWrittenFiles.empty());
    return results;
}
}  // namespace angle

namespace egl
{
angle::ParamCapture CaptureAttributeMap(const egl::AttributeMap &attribMap)
{
    switch (attribMap.getType())
    {
        case AttributeMapType::Attrib:
        {
            angle::ParamCapture paramCapture("attrib_list", angle::ParamType::TEGLAttribPointer);
            if (attribMap.isEmpty())
            {
                paramCapture.value.EGLAttribPointerVal = nullptr;
            }
            else
            {
                std::vector<EGLAttrib> attribs;
                for (const auto &[key, value] : attribMap)
                {
                    attribs.push_back(key);
                    attribs.push_back(value);
                }
                attribs.push_back(EGL_NONE);

                angle::CaptureMemory(attribs.data(), attribs.size() * sizeof(EGLAttrib),
                                     &paramCapture);
            }
            return paramCapture;
        }

        case AttributeMapType::Int:
        {
            angle::ParamCapture paramCapture("attrib_list", angle::ParamType::TEGLintPointer);
            if (attribMap.isEmpty())
            {
                paramCapture.value.EGLintPointerVal = nullptr;
            }
            else
            {
                std::vector<EGLint> attribs;
                for (const auto &[key, value] : attribMap)
                {
                    attribs.push_back(static_cast<EGLint>(key));
                    attribs.push_back(static_cast<EGLint>(value));
                }
                attribs.push_back(EGL_NONE);

                angle::CaptureMemory(attribs.data(), attribs.size() * sizeof(EGLint),
                                     &paramCapture);
            }
            return paramCapture;
        }

        default:
            UNREACHABLE();
            return angle::ParamCapture();
    }
}
}  // namespace egl
