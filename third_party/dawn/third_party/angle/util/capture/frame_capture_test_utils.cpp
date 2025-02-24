//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_test_utils:
//   Helper functions for capture and replay of traces.
//

#include "frame_capture_test_utils.h"

#include "common/frame_capture_utils.h"
#include "common/string_utils.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

namespace angle
{

namespace
{
bool LoadJSONFromFile(const std::string &fileName, rapidjson::Document *doc)
{
    std::ifstream ifs(fileName);
    if (!ifs.is_open())
    {
        return false;
    }

    rapidjson::IStreamWrapper inWrapper(ifs);
    doc->ParseStream(inWrapper);
    return !doc->HasParseError();
}

// Branched from:
// https://crsrc.org/c/third_party/zlib/google/compression_utils_portable.cc;drc=9fc44ce454cc889b603900ccd14b7024ea2c284c;l=167
// Unmodified other than inlining ZlibStreamWrapperType and z_stream arg to access .msg
int GzipUncompressHelperPatched(Bytef *dest,
                                uLongf *dest_length,
                                const Bytef *source,
                                uLong source_length,
                                z_stream &stream)
{
    stream.next_in  = static_cast<z_const Bytef *>(const_cast<Bytef *>(source));
    stream.avail_in = static_cast<uInt>(source_length);
    if (static_cast<uLong>(stream.avail_in) != source_length)
        return Z_BUF_ERROR;

    stream.next_out  = dest;
    stream.avail_out = static_cast<uInt>(*dest_length);
    if (static_cast<uLong>(stream.avail_out) != *dest_length)
        return Z_BUF_ERROR;

    stream.zalloc = static_cast<alloc_func>(0);
    stream.zfree  = static_cast<free_func>(0);

    int err = inflateInit2(&stream, MAX_WBITS + 16);
    if (err != Z_OK)
        return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END)
    {
        inflateEnd(&stream);
        if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
            return Z_DATA_ERROR;
        return err;
    }
    *dest_length = stream.total_out;

    err = inflateEnd(&stream);
    return err;
}

bool UncompressData(const std::vector<uint8_t> &compressedData,
                    std::vector<uint8_t> *uncompressedData)
{
    uint32_t uncompressedSize =
        zlib_internal::GetGzipUncompressedSize(compressedData.data(), compressedData.size());

    uncompressedData->resize(uncompressedSize + 1);  // +1 to make sure .data() is valid
    uLong destLen = uncompressedSize;
    z_stream stream;
    int zResult =
        GzipUncompressHelperPatched(uncompressedData->data(), &destLen, compressedData.data(),
                                    static_cast<uLong>(compressedData.size()), stream);

    if (zResult != Z_OK)
    {
        std::cerr << "Failure to decompressed binary data: " << zResult
                  << " msg=" << (stream.msg ? stream.msg : "nil") << "\n";
        fprintf(stderr,
                "next_in %p (input %p) avail_in %d total_in %lu next_out %p (output %p) avail_out "
                "%d total_out %ld adler %lX crc %lX crc_simd %lX\n",
                stream.next_in, compressedData.data(), stream.avail_in, stream.total_in,
                stream.next_out, uncompressedData->data(), stream.avail_out, stream.total_out,
                stream.adler, crc32(0, uncompressedData->data(), uncompressedSize),
                crc32(0, uncompressedData->data(), 16 * (uncompressedSize / 16)));
        return false;
    }

    return true;
}

void SaveDebugFile(const std::string &outputDir,
                   const char *baseFileName,
                   const char *suffix,
                   const std::vector<uint8_t> data)
{
    if (outputDir.empty())
    {
        return;
    }

    std::ostringstream path;
    path << outputDir << "/" << baseFileName << suffix;
    FILE *fp = fopen(path.str().c_str(), "wb");
    fwrite(data.data(), 1, data.size(), fp);
    fclose(fp);
}
}  // namespace

bool LoadTraceNamesFromJSON(const std::string jsonFilePath, std::vector<std::string> *namesOut)
{
    rapidjson::Document doc;
    if (!LoadJSONFromFile(jsonFilePath, &doc))
    {
        return false;
    }

    if (!doc.IsArray())
    {
        return false;
    }

    // Read trace json into a list of trace names.
    std::vector<std::string> traces;

    rapidjson::Document::Array traceArray = doc.GetArray();
    for (rapidjson::SizeType arrayIndex = 0; arrayIndex < traceArray.Size(); ++arrayIndex)
    {
        const rapidjson::Document::ValueType &arrayElement = traceArray[arrayIndex];

        if (!arrayElement.IsString())
        {
            return false;
        }

        traces.push_back(arrayElement.GetString());
    }

    *namesOut = std::move(traces);
    return true;
}

bool LoadTraceInfoFromJSON(const std::string &traceName,
                           const std::string &traceJsonPath,
                           TraceInfo *traceInfoOut)
{
    rapidjson::Document doc;
    if (!LoadJSONFromFile(traceJsonPath, &doc))
    {
        return false;
    }

    if (!doc.IsObject() || !doc.HasMember("TraceMetadata"))
    {
        return false;
    }

    const rapidjson::Document::Object &meta = doc["TraceMetadata"].GetObj();

    strncpy(traceInfoOut->name, traceName.c_str(), kTraceInfoMaxNameLen);
    traceInfoOut->contextClientMajorVersion = meta["ContextClientMajorVersion"].GetInt();
    traceInfoOut->contextClientMinorVersion = meta["ContextClientMinorVersion"].GetInt();
    traceInfoOut->frameEnd                  = meta["FrameEnd"].GetInt();
    traceInfoOut->frameStart                = meta["FrameStart"].GetInt();
    traceInfoOut->drawSurfaceHeight         = meta["DrawSurfaceHeight"].GetInt();
    traceInfoOut->drawSurfaceWidth          = meta["DrawSurfaceWidth"].GetInt();

    angle::HexStringToUInt(meta["DrawSurfaceColorSpace"].GetString(),
                           &traceInfoOut->drawSurfaceColorSpace);
    angle::HexStringToUInt(meta["DisplayPlatformType"].GetString(),
                           &traceInfoOut->displayPlatformType);
    angle::HexStringToUInt(meta["DisplayDeviceType"].GetString(), &traceInfoOut->displayDeviceType);

    traceInfoOut->configRedBits     = meta["ConfigRedBits"].GetInt();
    traceInfoOut->configGreenBits   = meta["ConfigGreenBits"].GetInt();
    traceInfoOut->configBlueBits    = meta["ConfigBlueBits"].GetInt();
    traceInfoOut->configAlphaBits   = meta["ConfigAlphaBits"].GetInt();
    traceInfoOut->configDepthBits   = meta["ConfigDepthBits"].GetInt();
    traceInfoOut->configStencilBits = meta["ConfigStencilBits"].GetInt();

    traceInfoOut->isBinaryDataCompressed = meta["IsBinaryDataCompressed"].GetBool();
    traceInfoOut->areClientArraysEnabled = meta["AreClientArraysEnabled"].GetBool();
    traceInfoOut->isBindGeneratesResourcesEnabled =
        meta["IsBindGeneratesResourcesEnabled"].GetBool();
    traceInfoOut->isWebGLCompatibilityEnabled = meta["IsWebGLCompatibilityEnabled"].GetBool();
    traceInfoOut->isRobustResourceInitEnabled = meta["IsRobustResourceInitEnabled"].GetBool();
    traceInfoOut->windowSurfaceContextId      = doc["WindowSurfaceContextID"].GetInt();

    if (doc.HasMember("RequiredExtensions"))
    {
        const rapidjson::Value &requiredExtensions = doc["RequiredExtensions"];
        if (!requiredExtensions.IsArray())
        {
            return false;
        }
        for (rapidjson::SizeType i = 0; i < requiredExtensions.Size(); i++)
        {
            std::string ext = std::string(requiredExtensions[i].GetString());
            traceInfoOut->requiredExtensions.push_back(ext);
        }
    }

    if (meta.HasMember("KeyFrames"))
    {
        const rapidjson::Value &keyFrames = meta["KeyFrames"];
        if (!keyFrames.IsArray())
        {
            return false;
        }
        for (rapidjson::SizeType i = 0; i < keyFrames.Size(); i++)
        {
            int frame = keyFrames[i].GetInt();
            traceInfoOut->keyFrames.push_back(frame);
        }
    }

    const rapidjson::Document::Array &traceFiles = doc["TraceFiles"].GetArray();
    for (const rapidjson::Value &value : traceFiles)
    {
        traceInfoOut->traceFiles.push_back(value.GetString());
    }

    traceInfoOut->initialized = true;
    return true;
}

TraceLibrary::TraceLibrary(const std::string &traceName,
                           const TraceInfo &traceInfo,
                           const std::string &baseDir)
{
    std::stringstream libNameStr;
    SearchType searchType = SearchType::ModuleDir;

#if defined(ANGLE_TRACE_EXTERNAL_BINARIES)
    // This means we are using the binary build of traces on Android, which are
    // not bundled in the APK, but located in the app's home directory.
    searchType = SearchType::SystemDir;
    libNameStr << baseDir;
#endif  // defined(ANGLE_TRACE_EXTERNAL_BINARIES)
#if !defined(ANGLE_PLATFORM_WINDOWS)
    libNameStr << "lib";
#endif  // !defined(ANGLE_PLATFORM_WINDOWS)
    libNameStr << traceName;
    std::string libName = libNameStr.str();
    std::string loadError;

    mTraceLibrary.reset(OpenSharedLibraryAndGetError(libName.c_str(), searchType, &loadError));
    if (mTraceLibrary->getNative() == nullptr)
    {
        FATAL() << "Failed to load trace library (" << libName << "): " << loadError;
    }

    callFunc<SetupEntryPoints>("SetupEntryPoints", static_cast<angle::TraceCallbacks *>(this),
                               &mTraceFunctions);
    mTraceFunctions->SetTraceInfo(traceInfo);
    mTraceInfo = traceInfo;
}

uint8_t *TraceLibrary::LoadBinaryData(const char *fileName)
{
    std::ostringstream pathBuffer;
    pathBuffer << mBinaryDataDir << "/" << fileName;
    FILE *fp = fopen(pathBuffer.str().c_str(), "rb");
    if (fp == 0)
    {
        fprintf(stderr, "Error loading binary data file: %s\n", fileName);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (mTraceInfo.isBinaryDataCompressed)
    {
        if (!strstr(fileName, ".gz"))
        {
            fprintf(stderr, "Filename does not end in .gz");
            exit(1);
        }

        std::vector<uint8_t> compressedData(size);
        size_t bytesRead = fread(compressedData.data(), 1, size, fp);
        if (bytesRead != static_cast<size_t>(size))
        {
            std::cerr << "Failed to read binary data: " << bytesRead << " != " << size << "\n";
            exit(1);
        }

        if (!UncompressData(compressedData, &mBinaryData))
        {
            // Workaround for sporadic failures https://issuetracker.google.com/296921272
            SaveDebugFile(mDebugOutputDir, fileName, ".gzdbg_input.gz", compressedData);
            SaveDebugFile(mDebugOutputDir, fileName, ".gzdbg_attempt1", mBinaryData);
            std::vector<uint8_t> uncompressedData;
            bool secondResult = UncompressData(compressedData, &uncompressedData);
            SaveDebugFile(mDebugOutputDir, fileName, ".gzdbg_attempt2", uncompressedData);
            if (!secondResult)
            {
                std::cerr << "Uncompress retry failed\n";
                exit(1);
            }
            std::cerr << "Uncompress retry succeeded, moving to mBinaryData\n";
            mBinaryData = std::move(uncompressedData);
        }
    }
    else
    {
        if (!strstr(fileName, ".angledata"))
        {
            fprintf(stderr, "Filename does not end in .angledata");
            exit(1);
        }
        mBinaryData.resize(size + 1);
        (void)fread(mBinaryData.data(), 1, size, fp);
    }
    fclose(fp);

    return mBinaryData.data();
}

}  // namespace angle
