//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// trace_interpreter.cpp:
//   Parser and interpreter for the C-based replays.
//

#include "trace_interpreter.h"

#include "anglebase/no_destructor.h"
#include "common/gl_enum_utils.h"
#include "common/string_utils.h"
#include "trace_fixture.h"

#define USE_SYSTEM_ZLIB
#include "compression_utils_portable.h"

namespace angle
{
namespace
{
bool ShouldParseFile(const std::string &file)
{
    return EndsWith(file, ".c") || EndsWith(file, ".cpp");
}

void ReplayTraceFunction(const TraceFunction &func, const TraceFunctionMap &customFunctions)
{
    for (const CallCapture &call : func)
    {
        ReplayTraceFunctionCall(call, customFunctions);
    }
}

class Parser : angle::NonCopyable
{
  public:
    Parser(const std::string &stream,
           TraceFunctionMap &functionsIn,
           TraceStringMap &stringsIn,
           bool verboseLogging)
        : mStream(stream),
          mFunctions(functionsIn),
          mStrings(stringsIn),
          mIndex(0),
          mVerboseLogging(verboseLogging)
    {}

    void parse()
    {
        while (mIndex < mStream.size())
        {
            if (peek() == '#' || peek() == '/')
            {
                skipLine();
            }
            else if (peek() == 'v')
            {
                ASSERT(check("void "));
                readFunction();
            }
            else if (peek() == 'c')
            {
                ASSERT(check("const "));
                readMultilineString();
            }
            else
            {
                printf("Unexpected character: '%c'\n", peek());
                UNREACHABLE();
            }
        }
    }

  private:
    ANGLE_INLINE char peek() const { return mStream[mIndex]; }

    ANGLE_INLINE char look(size_t ahead) const { return mStream[mIndex + ahead]; }

    ANGLE_INLINE void advance() { mIndex++; }

    ANGLE_INLINE void advanceTo(char delim)
    {
        while (peek() != delim)
        {
            advance();
        }
    }

    bool check(const char *forString) const
    {
        return mStream.substr(mIndex, strlen(forString)) == forString;
    }

    void skipLine()
    {
        advanceTo('\n');
        skipWhitespace();
    }

    void skipWhitespace()
    {
        while (isspace(peek()))
        {
            advance();
        }
    }

    void skipNonWhitespace()
    {
        while (!isspace(peek()))
        {
            advance();
        }
    }

    // In our simplified trace C, every line that begins with a } either ends a function or a
    // string. All lines inside the function begin with whitespace. So to find the end of the
    // function we just need to scan for a line beginning with }.
    void skipFunction()
    {
        while (peek() != '}')
        {
            advanceTo('\n');
            advance();
        }
        advance();
        skipWhitespace();
    }

    void readStringAppend(std::string *stringOut, char delim)
    {
        while (peek() != delim)
        {
            if (peek() == '\\')
            {
                advance();
                switch (peek())
                {
                    case 'n':
                        *stringOut += '\n';
                        break;
                    case '\"':
                        *stringOut += '\"';
                        break;
                    case '\\':
                        *stringOut += '\\';
                        break;
                    default:
                        printf("Unrecognized escape character: \\%c\n", peek());
                        UNREACHABLE();
                        break;
                }
            }
            else
            {
                *stringOut += peek();
            }
            advance();
        }
    }

    void readToken(Token &token, char delim)
    {
        size_t startIndex = mIndex;
        advanceTo(delim);
        size_t tokenSize = mIndex - startIndex;
        ASSERT(tokenSize < kMaxTokenSize);
        memcpy(token, &mStream[startIndex], tokenSize);
        token[mIndex - startIndex] = 0;
    }

    void skipCast()
    {
        if (peek() == '(')
        {
            advanceTo(')');
            advance();
        }
    }

    void skipComments()
    {
        while (peek() == '/')
        {
            skipLine();
        }
    }

    void readFunction()
    {
        std::string funcName;
        TraceFunction func;

        // Skip past the "void" return value.
        skipNonWhitespace();
        advance();
        readStringAppend(&funcName, '(');
        if (mVerboseLogging)
        {
            printf("function: %s\n", funcName.c_str());
        }

        // Skip this function because of the switch statements.
        if (funcName == "ReplayFrame")
        {
            skipFunction();
            return;
        }

        skipLine();
        ASSERT(peek() == '{');
        skipLine();
        while (peek() != '}')
        {
            skipComments();

            Token nameToken;
            readToken(nameToken, '(');
            advance();
            ParamBuffer params;
            Token paramTokens[kMaxParameters];
            size_t numParams = 0;
            skipCast();
            size_t tokenStart = mIndex;
            while (peek() != ';')
            {
                // Skip casts.
                if (peek() == ',' || (peek() == ')' && mIndex != tokenStart))
                {
                    ASSERT(numParams < kMaxParameters);
                    size_t tokenSize = mIndex - tokenStart;
                    ASSERT(tokenSize < kMaxTokenSize);
                    Token &token = paramTokens[numParams++];

                    memcpy(token, &mStream[tokenStart], tokenSize);
                    token[tokenSize] = 0;
                    advance();
                    skipWhitespace();
                    skipCast();
                    tokenStart = mIndex;
                }
                else
                {
                    advance();
                }
            }

            // Turn on if you want more spam.
            // if (mVerboseLogging)
            //{
            //    printf("call: %s(", nameToken);
            //    for (size_t paramIndex = 0; paramIndex < numParams; ++paramIndex)
            //    {
            //        if (paramIndex > 0)
            //        {
            //            printf(", ");
            //        }
            //        printf("%s", paramTokens[paramIndex]);
            //    }
            //    printf(")\n");
            //}

            // We pass in the strings for specific use with C string array parameters.
            CallCapture call = ParseCallCapture(nameToken, numParams, paramTokens, mStrings);
            func.push_back(std::move(call));
            skipLine();
        }
        skipLine();

        addFunction(funcName, func);
    }

    void readMultilineString()
    {
        std::string name;
        TraceString traceStr;

        while (peek() != 'g')
        {
            advance();
        }
        ASSERT(check("glShaderSource") || check("glTransformFeedbackVaryings"));

        readStringAppend(&name, '[');
        if (mVerboseLogging)
        {
            printf("string: %s\n", name.c_str());
        }
        skipLine();
        std::string str;
        while (peek() != '}')
        {
            advance();
            readStringAppend(&str, '\"');
            advance();
            if (peek() == ',')
            {
                traceStr.strings.push_back(std::move(str));
            }
            skipLine();
        }
        skipLine();

        for (const std::string &cppstr : traceStr.strings)
        {
            traceStr.pointers.push_back(cppstr.c_str());
        }

        mStrings[name] = std::move(traceStr);
    }

    void addFunction(const std::string &funcName, TraceFunction &func)
    {
        // Run initialize immediately so we can load the binary data.
        if (funcName == "InitReplay")
        {
            ReplayTraceFunction(func, {});
            func.clear();
        }
        mFunctions[funcName] = std::move(func);
    }

    const std::string &mStream;
    TraceFunctionMap &mFunctions;
    TraceStringMap &mStrings;
    size_t mIndex;
    bool mVerboseLogging = false;
};

void PackResourceID(ParamBuffer &params, const Token &token)
{
    ASSERT(token[0] == 'g');
    const char *start = strrchr(token, '[');
    ASSERT(start != nullptr && EndsWith(token, "]"));
    uint32_t value = static_cast<uint32_t>(atoi(start + 1));
    if (BeginsWith(token, "gShaderProgramMap"))
    {
        gl::ShaderProgramID id = {value};
        params.addUnnamedParam(ParamType::TShaderProgramID, id);
    }
    else if (BeginsWith(token, "gBufferMap"))
    {
        gl::BufferID id = {value};
        params.addUnnamedParam(ParamType::TBufferID, id);
    }
    else if (BeginsWith(token, "gTextureMap"))
    {
        gl::TextureID id = {value};
        params.addUnnamedParam(ParamType::TTextureID, id);
    }
    else if (BeginsWith(token, "gRenderbufferMap"))
    {
        gl::RenderbufferID id = {value};
        params.addUnnamedParam(ParamType::TRenderbufferID, id);
    }
    else if (BeginsWith(token, "gFramebufferMap"))
    {
        gl::FramebufferID id = {value};
        params.addUnnamedParam(ParamType::TFramebufferID, id);
    }
    else if (BeginsWith(token, "gSyncMap"))
    {
        gl::SyncID id = {value};
        params.addUnnamedParam(ParamType::TSyncID, id);
    }
    else if (BeginsWith(token, "gTransformFeedbackMap"))
    {
        gl::TransformFeedbackID id = {value};
        params.addUnnamedParam(ParamType::TTransformFeedbackID, id);
    }
    else if (BeginsWith(token, "gVertexArrayMap"))
    {
        gl::VertexArrayID id = {value};
        params.addUnnamedParam(ParamType::TVertexArrayID, id);
    }
    else if (BeginsWith(token, "gQueryMap"))
    {
        gl::QueryID id = {value};
        params.addUnnamedParam(ParamType::TQueryID, id);
    }
    else if (BeginsWith(token, "gSamplerMap"))
    {
        gl::SamplerID id = {value};
        params.addUnnamedParam(ParamType::TSamplerID, id);
    }
    else
    {
        printf("Unknown resource map: %s\n", token);
        UNREACHABLE();
    }
}

template <typename IntT>
void PackIntParameter(ParamBuffer &params, ParamType paramType, const Token &token)
{
    IntT value;

    if (token[0] == 'G')
    {
        ASSERT(BeginsWith(token, "GL_"));
        if (strchr(token, '|') == 0)
        {
            value = static_cast<IntT>(gl::StringToGLenum(token));
        }
        else
        {
            value = static_cast<IntT>(gl::StringToGLbitfield(token));
        }
    }
    else
    {
        if (!isdigit(token[0]) && !(token[0] == '-' && isdigit(token[1])))
        {
            printf("Expected number, got %s\n", token);
            UNREACHABLE();
        }
        if (token[0] == '0' && token[1] == 'x')
        {
            value = static_cast<IntT>(strtol(token, nullptr, 16));
        }
        else
        {
            value = static_cast<IntT>(atoi(token));
        }
    }

    params.addUnnamedParam(paramType, value);
}

uint32_t GetStringArrayOffset(const Token &token, const char *prefixString)
{
    const char *offsetString = &token[strlen(prefixString)];
    return atoi(offsetString);
}

template <typename PointerT>
void PackMemPointer(ParamBuffer &params, ParamType paramType, uint32_t offset, uint8_t *mem)
{
    ASSERT(gBinaryData);
    params.addUnnamedParam(paramType, reinterpret_cast<PointerT>(&mem[offset]));
}

template <typename T>
void PackMutablePointerParameter(ParamBuffer &params, ParamType paramType, const Token &token)
{
    if (token[0] == '0' && token[1] == 0)
    {
        params.addUnnamedParam(paramType, reinterpret_cast<T *>(0));
    }
    else if (token[0] == '&')
    {
        ASSERT(BeginsWith(token, "&gReadBuffer[") && EndsWith(token, "]"));
        uint32_t offset = GetStringArrayOffset(token, "&gReadBuffer[");
        PackMemPointer<T *>(params, paramType, offset, gReadBuffer);
    }
    else if (token[0] == 'g')
    {
        ASSERT(strcmp(token, "gReadBuffer") == 0);
        params.addUnnamedParam(paramType, reinterpret_cast<T *>(gReadBuffer));
    }
    else
    {
        UNREACHABLE();
    }
}

template <typename T>
void PackConstPointerParameter(ParamBuffer &params, ParamType paramType, const Token &token)
{
    // Handle nullptr, the literal "0".
    if (token[0] == '0' && token[1] == 0)
    {
        params.addUnnamedParam(paramType, reinterpret_cast<const T *>(0));
    }
    else if (token[0] == '&')
    {
        ASSERT(BeginsWith(token, "&gBinaryData[") && EndsWith(token, "]"));
        uint32_t offset = GetStringArrayOffset(token, "&gReadBuffer[");
        PackMemPointer<const T *>(params, paramType, offset, gBinaryData);
    }
    else if (token[0] == 'g')
    {
        if (strcmp(token, "gResourceIDBuffer") == 0)
        {
            params.addUnnamedParam(paramType, reinterpret_cast<const T *>(gResourceIDBuffer));
        }
        else if (BeginsWith(token, "gClientArrays"))
        {
            uint32_t offset = GetStringArrayOffset(token, "gClientArrays[");
            params.addUnnamedParam(paramType, reinterpret_cast<const T *>(gClientArrays[offset]));
        }
        else
        {
            printf("Unexpected token: %s\n", token);
            UNREACHABLE();
        }
    }
    else
    {
        ASSERT(isdigit(token[0]));
        uint32_t offset = atoi(token);
        params.addUnnamedParam(paramType,
                               reinterpret_cast<const T *>(static_cast<uintptr_t>(offset)));
    }
}

class TraceInterpreter : angle::NonCopyable
{
  public:
    TraceInterpreter()  = default;
    ~TraceInterpreter() = default;

    void replayFrame(uint32_t frameIndex);
    void setupReplay();
    void resetReplay();
    const char *getSerializedContextState(uint32_t frameIndex);

  private:
    void runTraceFunction(const char *name) const;
    void parseTraceUncompressed();
    void parseTraceGz();

    TraceFunctionMap mTraceFunctions;
    TraceStringMap mTraceStrings;
    bool mVerboseLogging = true;
};

void TraceInterpreter::replayFrame(uint32_t frameIndex)
{
    char funcName[kMaxTokenSize];
    snprintf(funcName, kMaxTokenSize, "ReplayFrame%u", frameIndex);
    runTraceFunction(funcName);
}

void TraceInterpreter::parseTraceUncompressed()
{
    for (const std::string &file : gTraceInfo.traceFiles)
    {
        if (!ShouldParseFile(file))
        {
            if (mVerboseLogging)
            {
                printf("Skipping function parsing for %s.\n", file.c_str());
            }
            continue;
        }

        if (mVerboseLogging)
        {
            printf("Parsing functions from %s\n", file.c_str());
        }
        std::stringstream pathStream;
        pathStream << gBinaryDataDir << GetPathSeparator() << file;
        std::string path = pathStream.str();

        std::string fileData;
        if (!ReadFileToString(path, &fileData))
        {
            UNREACHABLE();
        }

        Parser parser(fileData, mTraceFunctions, mTraceStrings, mVerboseLogging);
        parser.parse();
    }
}

void TraceInterpreter::parseTraceGz()
{
    if (mVerboseLogging)
    {
        printf("Parsing functions from %s\n", gTraceGzPath.c_str());
    }

    FILE *fp = fopen(gTraceGzPath.c_str(), "rb");
    if (fp == 0)
    {
        printf("Error loading trace (gz) from: %s\n", gTraceGzPath.c_str());
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::vector<uint8_t> compressedData(size);
    (void)fread(compressedData.data(), 1, size, fp);

    uint32_t uncompressedSize =
        zlib_internal::GetGzipUncompressedSize(compressedData.data(), compressedData.size());

    std::string uncompressedData(uncompressedSize, 0);
    uLong destLen = uncompressedSize;
    int zResult = zlib_internal::GzipUncompressHelper((uint8_t *)uncompressedData.data(), &destLen,
                                                      compressedData.data(),
                                                      static_cast<uLong>(compressedData.size()));

    if (zResult != Z_OK)
    {
        printf("Failure to decompress gz trace: %s\n", gTraceGzPath.c_str());
        exit(1);
    }

    Parser parser(uncompressedData, mTraceFunctions, mTraceStrings, mVerboseLogging);
    parser.parse();
}

void TraceInterpreter::setupReplay()
{
    if (!gTraceGzPath.empty())
    {
        parseTraceGz();
    }
    else
    {
        parseTraceUncompressed();
    }

    if (mTraceFunctions.count("SetupReplay") == 0)
    {
        printf("Did not find a SetupReplay function to run among %zu parsed functions.\n",
               mTraceFunctions.size());
        exit(1);
    }

    runTraceFunction("SetupReplay");
}

void TraceInterpreter::resetReplay()
{
    runTraceFunction("ResetReplay");
}

const char *TraceInterpreter::getSerializedContextState(uint32_t frameIndex)
{
    // TODO: Necessary for complete self-testing. http://anglebug.com/42266250
    UNREACHABLE();
    return nullptr;
}

void TraceInterpreter::runTraceFunction(const char *name) const
{
    auto iter = mTraceFunctions.find(name);
    if (iter == mTraceFunctions.end())
    {
        printf("Cannot find function: %s\n", name);
        UNREACHABLE();
    }
    const TraceFunction &func = iter->second;
    ReplayTraceFunction(func, mTraceFunctions);
}

TraceInterpreter &GetInterpreter()
{
    static angle::base::NoDestructor<std::unique_ptr<TraceInterpreter>> sTraceInterpreter(
        new TraceInterpreter());
    return *sTraceInterpreter.get()->get();
}
}  // anonymous namespace

template <>
void PackParameter<uint32_t>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    if (token[0] == 'g')
    {
        PackResourceID(params, token);
    }
    else
    {
        PackIntParameter<uint32_t>(params, ParamType::TGLuint, token);
    }
}

template <>
void PackParameter<int32_t>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    if (BeginsWith(token, "gUniformLocations"))
    {
        const char *start = strrchr(token, '[');
        ASSERT(start != nullptr && EndsWith(token, "]"));
        int32_t value           = atoi(start + 1);
        gl::UniformLocation loc = {value};
        params.addUnnamedParam(ParamType::TUniformLocation, loc);
    }
    else
    {
        PackIntParameter<int32_t>(params, ParamType::TGLint, token);
    }
}

template <>
void PackParameter<void *>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    void *value = 0;
    params.addUnnamedParam(ParamType::TvoidPointer, value);
}

template <>
void PackParameter<const int32_t *>(ParamBuffer &params,
                                    const Token &token,
                                    const TraceStringMap &strings)
{
    PackConstPointerParameter<int32_t>(params, ParamType::TGLintConstPointer, token);
}

template <>
void PackParameter<void **>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<int32_t *>(ParamBuffer &params,
                              const Token &token,
                              const TraceStringMap &strings)
{
    PackMutablePointerParameter<int32_t>(params, ParamType::TGLintPointer, token);
}

template <>
void PackParameter<uint64_t>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    params.addUnnamedParam(ParamType::TGLuint64,
                           static_cast<GLuint64>(std::strtoull(token, nullptr, 10)));
}

template <>
void PackParameter<int64_t>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    params.addUnnamedParam(ParamType::TGLint64,
                           static_cast<GLint64>(std::strtoll(token, nullptr, 10)));
}

template <>
void PackParameter<const int64_t *>(ParamBuffer &params,
                                    const Token &token,
                                    const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<int64_t *>(ParamBuffer &params,
                              const Token &token,
                              const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<uint64_t *>(ParamBuffer &params,
                               const Token &token,
                               const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<const char *>(ParamBuffer &params,
                                 const Token &token,
                                 const TraceStringMap &strings)
{
    if (token[0] == '"')
    {
        ASSERT(EndsWith(token, "\""));

        ParamCapture param(params.getNextParamName(), ParamType::TGLcharConstPointer);
        std::vector<uint8_t> data(&token[1], &token[strlen(token) - 1]);
        data.push_back(0);
        param.data.push_back(std::move(data));
        param.value.GLcharConstPointerVal = reinterpret_cast<const char *>(param.data[0].data());
        params.addParam(std::move(param));
    }
    else
    {
        PackConstPointerParameter<char>(params, ParamType::TGLcharConstPointer, token);
    }
}

template <>
void PackParameter<const void *>(ParamBuffer &params,
                                 const Token &token,
                                 const TraceStringMap &strings)
{
    PackConstPointerParameter<void>(params, ParamType::TvoidConstPointer, token);
}

template <>
void PackParameter<uint32_t *>(ParamBuffer &params,
                               const Token &token,
                               const TraceStringMap &strings)
{
    PackMutablePointerParameter<uint32_t>(params, ParamType::TGLuintPointer, token);
}

template <>
void PackParameter<const uint32_t *>(ParamBuffer &params,
                                     const Token &token,
                                     const TraceStringMap &strings)
{
    PackConstPointerParameter<uint32_t>(params, ParamType::TGLuintConstPointer, token);
}

template <>
void PackParameter<float>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    params.addUnnamedParam(ParamType::TGLfloat, std::stof(token));
}

template <>
void PackParameter<uint8_t>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    PackIntParameter<uint8_t>(params, ParamType::TGLubyte, token);
}

template <>
void PackParameter<float *>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    PackMutablePointerParameter<float>(params, ParamType::TGLfloatPointer, token);
}

template <>
void PackParameter<const float *>(ParamBuffer &params,
                                  const Token &token,
                                  const TraceStringMap &strings)
{
    PackConstPointerParameter<float>(params, ParamType::TGLfloatConstPointer, token);
}

template <>
void PackParameter<GLsync>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    PackResourceID(params, token);
}

template <>
void PackParameter<const char *const *>(ParamBuffer &params,
                                        const Token &token,
                                        const TraceStringMap &strings)
{
    // Find the string that corresponds to "token". Currently we only support string arrays.
    auto iter = strings.find(token);
    if (iter == strings.end())
    {
        printf("Could not find string: %s\n", token);
        UNREACHABLE();
    }
    const TraceString &traceStr = iter->second;
    params.addUnnamedParam(ParamType::TGLcharConstPointerPointer, traceStr.pointers.data());
}

template <>
void PackParameter<const char **>(ParamBuffer &params,
                                  const Token &token,
                                  const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<GLDEBUGPROCKHR>(ParamBuffer &params,
                                   const Token &token,
                                   const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<EGLDEBUGPROCKHR>(ParamBuffer &params,
                                    const Token &token,
                                    const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<const struct AHardwareBuffer *>(ParamBuffer &params,
                                                   const Token &token,
                                                   const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<EGLSetBlobFuncANDROID>(ParamBuffer &params,
                                          const Token &token,
                                          const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<EGLGetBlobFuncANDROID>(ParamBuffer &params,
                                          const Token &token,
                                          const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<int16_t>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    PackIntParameter<int16_t>(params, ParamType::TGLshort, token);
}

template <>
void PackParameter<const int16_t *>(ParamBuffer &params,
                                    const Token &token,
                                    const TraceStringMap &strings)
{
    PackConstPointerParameter<int16_t>(params, ParamType::TGLshortConstPointer, token);
}

template <>
void PackParameter<char *>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<unsigned char *>(ParamBuffer &params,
                                    const Token &token,
                                    const TraceStringMap &strings)
{
    PackMutablePointerParameter<GLubyte>(params, ParamType::TGLubytePointer, token);
}

template <>
void PackParameter<const void *const *>(ParamBuffer &params,
                                        const Token &token,
                                        const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<const uint64_t *>(ParamBuffer &params,
                                     const Token &token,
                                     const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<GLGETBLOBPROCANGLE>(ParamBuffer &params,
                                       const Token &token,
                                       const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<GLSETBLOBPROCANGLE>(ParamBuffer &params,
                                       const Token &token,
                                       const TraceStringMap &strings)
{
    UNREACHABLE();
}

#if defined(ANGLE_PLATFORM_WINDOWS)
template <>
void PackParameter<EGLNativeDisplayType>(ParamBuffer &params,
                                         const Token &token,
                                         const TraceStringMap &strings)
{
    UNREACHABLE();
}
#endif  // defined(ANGLE_PLATFORM_WINDOWS)

#if defined(ANGLE_PLATFORM_WINDOWS) || defined(ANGLE_PLATFORM_ANDROID)
template <>
void PackParameter<EGLNativeWindowType>(ParamBuffer &params,
                                        const Token &token,
                                        const TraceStringMap &strings)
{
    UNREACHABLE();
}

template <>
void PackParameter<EGLNativePixmapType>(ParamBuffer &params,
                                        const Token &token,
                                        const TraceStringMap &strings)
{
    UNREACHABLE();
}
#endif  // defined(ANGLE_PLATFORM_WINDOWS) || defined(ANGLE_PLATFORM_ANDROID)

#if defined(ANGLE_PLATFORM_APPLE) || !defined(ANGLE_IS_64_BIT_CPU)
template <>
void PackParameter<const long *>(ParamBuffer &params,
                                 const Token &token,
                                 const TraceStringMap &strings)
{
    PackConstPointerParameter<int64_t>(params, ParamType::TGLuint64ConstPointer, token);
}

template <>
void PackParameter<long *>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    PackMutablePointerParameter<int64_t>(params, ParamType::TGLint64Pointer, token);
}

template <>
void PackParameter<long>(ParamBuffer &params, const Token &token, const TraceStringMap &strings)
{
    PackIntParameter<int64_t>(params, ParamType::TGLint64, token);
}

template <>
void PackParameter<unsigned long>(ParamBuffer &params,
                                  const Token &token,
                                  const TraceStringMap &strings)
{
    PackIntParameter<uint64_t>(params, ParamType::TGLuint64, token);
}
#endif  // defined(ANGLE_PLATFORM_APPLE) || !defined(ANGLE_IS_64_BIT_CPU)

GLuint GetResourceIDMapValue(ResourceIDType resourceIDType, GLuint key)
{
    switch (resourceIDType)
    {
        case ResourceIDType::Buffer:
            return gBufferMap[key];
        case ResourceIDType::FenceNV:
            return gFenceNVMap[key];
        case ResourceIDType::Framebuffer:
            return gFramebufferMap[key];
        case ResourceIDType::ProgramPipeline:
            return gProgramPipelineMap[key];
        case ResourceIDType::Query:
            return gQueryMap[key];
        case ResourceIDType::Renderbuffer:
            return gRenderbufferMap[key];
        case ResourceIDType::Sampler:
            return gSamplerMap[key];
        case ResourceIDType::Semaphore:
            return gSemaphoreMap[key];
        case ResourceIDType::ShaderProgram:
            return gShaderProgramMap[key];
        case ResourceIDType::Texture:
            return gTextureMap[key];
        case ResourceIDType::TransformFeedback:
            return gTransformFeedbackMap[key];
        case ResourceIDType::VertexArray:
            return gVertexArrayMap[key];
        default:
            printf("Incompatible resource ID type: %d\n", static_cast<int>(resourceIDType));
            UNREACHABLE();
            return 0;
    }
}

}  // namespace angle

extern "C" {
void SetupReplay()
{
    angle::GetInterpreter().setupReplay();
}

void ReplayFrame(uint32_t frameIndex)
{
    angle::GetInterpreter().replayFrame(frameIndex);
}

void ResetReplay()
{
    angle::GetInterpreter().resetReplay();
}

const char *GetSerializedContextState(uint32_t frameIndex)
{
    return angle::GetInterpreter().getSerializedContextState(frameIndex);
}
}  // extern "C"
