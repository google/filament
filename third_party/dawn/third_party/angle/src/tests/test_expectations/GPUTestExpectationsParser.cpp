//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "GPUTestExpectationsParser.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/string_utils.h"

namespace angle
{

namespace
{

enum LineParserStage
{
    kLineParserBegin = 0,
    kLineParserBugID,
    kLineParserConfigs,
    kLineParserColon,
    kLineParserTestName,
    kLineParserEqual,
    kLineParserExpectations,
};

enum Token
{
    // os
    kConfigWinXP = 0,
    kConfigWinVista,
    kConfigWin7,
    kConfigWin8,
    kConfigWin10,
    kConfigWin,
    kConfigMacLeopard,
    kConfigMacSnowLeopard,
    kConfigMacLion,
    kConfigMacMountainLion,
    kConfigMacMavericks,
    kConfigMacYosemite,
    kConfigMacElCapitan,
    kConfigMacSierra,
    kConfigMacHighSierra,
    kConfigMacMojave,
    kConfigMac,
    kConfigIOS,
    kConfigLinux,
    kConfigChromeOS,
    kConfigAndroid,
    // gpu vendor
    kConfigNVIDIA,
    kConfigAMD,
    kConfigIntel,
    kConfigVMWare,
    kConfigApple,
    // build type
    kConfigRelease,
    kConfigDebug,
    // ANGLE renderer
    kConfigD3D9,
    kConfigD3D11,
    kConfigGLDesktop,
    kConfigGLES,
    kConfigVulkan,
    kConfigSwiftShader,
    kConfigMetal,
    kConfigWgpu,
    // Android devices
    kConfigNexus5X,
    kConfigPixel2,
    kConfigPixel4,
    kConfigPixel6,
    kConfigPixel7,
    kConfigFlipN2,
    kConfigMaliG710,
    kConfigGalaxyA23,
    kConfigGalaxyA34,
    kConfigGalaxyA54,
    kConfigGalaxyS22,
    kConfigGalaxyS23,
    kConfigGalaxyS24Exynos,
    kConfigGalaxyS24Qualcomm,
    kConfigFindX6,
    kConfigPineapple,
    // GPU devices
    kConfigNVIDIAQuadroP400,
    kConfigNVIDIAGTX1660,
    // PreRotation
    kConfigPreRotation,
    kConfigPreRotation90,
    kConfigPreRotation180,
    kConfigPreRotation270,
    // Sanitizers
    kConfigNoSan,
    kConfigASan,
    kConfigTSan,
    kConfigUBSan,
    // expectation
    kExpectationPass,
    kExpectationFail,
    kExpectationFlaky,
    kExpectationTimeout,
    kExpectationSkip,
    // separator
    kSeparatorColon,
    kSeparatorEqual,

    kNumberOfExactMatchTokens,

    // others
    kTokenComment,
    kTokenWord,

    kNumberOfTokens,
};

enum ErrorType
{
    kErrorFileIO = 0,
    kErrorIllegalEntry,
    kErrorInvalidEntry,
    kErrorEntryWithExpectationConflicts,
    kErrorEntryWithDisallowedExpectation,
    kErrorEntriesOverlap,

    kNumberOfErrors,
};

struct TokenInfo
{
    constexpr TokenInfo()
        : name(nullptr),
          condition(GPUTestConfig::kConditionNone),
          expectation(GPUTestExpectationsParser::kGpuTestPass)
    {}

    constexpr TokenInfo(const char *nameIn,
                        GPUTestConfig::Condition conditionIn,
                        GPUTestExpectationsParser::GPUTestExpectation expectationIn)
        : name(nameIn), condition(conditionIn), expectation(expectationIn)
    {}

    constexpr TokenInfo(const char *nameIn, GPUTestConfig::Condition conditionIn)
        : TokenInfo(nameIn, conditionIn, GPUTestExpectationsParser::kGpuTestPass)
    {}

    const char *name;
    GPUTestConfig::Condition condition;
    GPUTestExpectationsParser::GPUTestExpectation expectation;
};

constexpr TokenInfo kTokenData[kNumberOfTokens] = {
    {"xp", GPUTestConfig::kConditionWinXP},
    {"vista", GPUTestConfig::kConditionWinVista},
    {"win7", GPUTestConfig::kConditionWin7},
    {"win8", GPUTestConfig::kConditionWin8},
    {"win10", GPUTestConfig::kConditionWin10},
    {"win", GPUTestConfig::kConditionWin},
    {"leopard", GPUTestConfig::kConditionMacLeopard},
    {"snowleopard", GPUTestConfig::kConditionMacSnowLeopard},
    {"lion", GPUTestConfig::kConditionMacLion},
    {"mountainlion", GPUTestConfig::kConditionMacMountainLion},
    {"mavericks", GPUTestConfig::kConditionMacMavericks},
    {"yosemite", GPUTestConfig::kConditionMacYosemite},
    {"elcapitan", GPUTestConfig::kConditionMacElCapitan},
    {"sierra", GPUTestConfig::kConditionMacSierra},
    {"highsierra", GPUTestConfig::kConditionMacHighSierra},
    {"mojave", GPUTestConfig::kConditionMacMojave},
    {"mac", GPUTestConfig::kConditionMac},
    {"ios", GPUTestConfig::kConditionIOS},
    {"linux", GPUTestConfig::kConditionLinux},
    {"chromeos",
     GPUTestConfig::kConditionNone},  // https://anglebug.com/42262032 CrOS not supported
    {"android", GPUTestConfig::kConditionAndroid},
    {"nvidia", GPUTestConfig::kConditionNVIDIA},
    {"amd", GPUTestConfig::kConditionAMD},
    {"intel", GPUTestConfig::kConditionIntel},
    {"vmware", GPUTestConfig::kConditionVMWare},
    {"apple", GPUTestConfig::kConditionApple},
    {"release", GPUTestConfig::kConditionRelease},
    {"debug", GPUTestConfig::kConditionDebug},
    {"d3d9", GPUTestConfig::kConditionD3D9},
    {"d3d11", GPUTestConfig::kConditionD3D11},
    {"opengl", GPUTestConfig::kConditionGLDesktop},
    {"gles", GPUTestConfig::kConditionGLES},
    {"vulkan", GPUTestConfig::kConditionVulkan},
    {"swiftshader", GPUTestConfig::kConditionSwiftShader},
    {"metal", GPUTestConfig::kConditionMetal},
    {"wgpu", GPUTestConfig::kConditionWgpu},
    {"nexus5x", GPUTestConfig::kConditionNexus5X},
    {"pixel2orxl", GPUTestConfig::kConditionPixel2OrXL},
    {"pixel4orxl", GPUTestConfig::kConditionPixel4OrXL},
    {"pixel6", GPUTestConfig::kConditionPixel6},
    {"pixel7", GPUTestConfig::kConditionPixel7},
    {"flipn2", GPUTestConfig::kConditionFlipN2},
    {"malig710", GPUTestConfig::kConditionMaliG710},
    {"galaxya23", GPUTestConfig::kConditionGalaxyA23},
    {"galaxya34", GPUTestConfig::kConditionGalaxyA34},
    {"galaxya54", GPUTestConfig::kConditionGalaxyA54},
    {"galaxys22", GPUTestConfig::kConditionGalaxyS22},
    {"galaxys23", GPUTestConfig::kConditionGalaxyS23},
    {"galaxys24exynos", GPUTestConfig::kConditionGalaxyS24Exynos},
    {"galaxys24qualcomm", GPUTestConfig::kConditionGalaxyS24Qualcomm},
    {"findx6", GPUTestConfig::kConditionFindX6},
    {"pineapple", GPUTestConfig::kConditionPineapple},
    {"quadrop400", GPUTestConfig::kConditionNVIDIAQuadroP400},
    {"gtx1660", GPUTestConfig::kConditionNVIDIAGTX1660},
    {"prerotation", GPUTestConfig::kConditionPreRotation},
    {"prerotation90", GPUTestConfig::kConditionPreRotation90},
    {"prerotation180", GPUTestConfig::kConditionPreRotation180},
    {"prerotation270", GPUTestConfig::kConditionPreRotation270},
    {"nosan", GPUTestConfig::kConditionNoSan},
    {"asan", GPUTestConfig::kConditionASan},
    {"tsan", GPUTestConfig::kConditionTSan},
    {"ubsan", GPUTestConfig::kConditionUBSan},
    {"pass", GPUTestConfig::kConditionNone, GPUTestExpectationsParser::kGpuTestPass},
    {"fail", GPUTestConfig::kConditionNone, GPUTestExpectationsParser::kGpuTestFail},
    {"flaky", GPUTestConfig::kConditionNone, GPUTestExpectationsParser::kGpuTestFlaky},
    {"timeout", GPUTestConfig::kConditionNone, GPUTestExpectationsParser::kGpuTestTimeout},
    {"skip", GPUTestConfig::kConditionNone, GPUTestExpectationsParser::kGpuTestSkip},
    {":", GPUTestConfig::kConditionNone},  // kSeparatorColon
    {"=", GPUTestConfig::kConditionNone},  // kSeparatorEqual
    {},                                    // kNumberOfExactMatchTokens
    {},                                    // kTokenComment
    {},                                    // kTokenWord
};

const char *kErrorMessage[kNumberOfErrors] = {
    "file IO failed",
    "entry with wrong format",
    "entry invalid, likely unimplemented modifiers",
    "entry with expectation modifier conflicts",
    "entry with unsupported expectation",
    "two entries' configs overlap",
};

inline bool StartsWithASCII(const std::string &str, const std::string &search, bool caseSensitive)
{
    ASSERT(!caseSensitive);
    return str.compare(0, search.length(), search) == 0;
}

template <class Char>
inline Char ToLowerASCII(Char c)
{
    return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

template <typename Iter>
inline bool DoLowerCaseEqualsASCII(Iter a_begin, Iter a_end, const char *b)
{
    for (Iter it = a_begin; it != a_end; ++it, ++b)
    {
        if (!*b || ToLowerASCII(*it) != *b)
            return false;
    }
    return *b == 0;
}

inline bool LowerCaseEqualsASCII(const std::string &a, const char *b)
{
    return DoLowerCaseEqualsASCII(a.begin(), a.end(), b);
}

inline Token ParseToken(const std::string &word)
{
    if (StartsWithASCII(word, "//", false))
        return kTokenComment;

    for (int32_t i = 0; i < kNumberOfExactMatchTokens; ++i)
    {
        if (LowerCaseEqualsASCII(word, kTokenData[i].name))
            return static_cast<Token>(i);
    }
    return kTokenWord;
}

bool ConditionArrayIsSubset(const GPUTestConfig::ConditionArray &subset,
                            const GPUTestConfig::ConditionArray &superset)
{
    for (size_t subsetCondition : subset)
    {
        bool foundCondition = false;
        for (size_t supersetCondition : superset)
        {
            if (subsetCondition == supersetCondition)
            {
                foundCondition = true;
                break;
            }
        }

        if (!foundCondition)
        {
            return false;
        }
    }

    return true;
}

// If one array is completely contained within the other, then we say the conditions overlap.
bool ConditionsOverlap(const GPUTestConfig::ConditionArray &conditionsI,
                       const GPUTestConfig::ConditionArray &conditionsJ)
{
    return ConditionArrayIsSubset(conditionsI, conditionsJ) ||
           ConditionArrayIsSubset(conditionsJ, conditionsI);
}
}  // anonymous namespace

const char *GetConditionName(uint32_t condition)
{
    if (condition == GPUTestConfig::kConditionNone)
    {
        return nullptr;
    }

    for (const TokenInfo &info : kTokenData)
    {
        if (info.condition == condition)
        {
            // kConditionNone is used to tag tokens that aren't conditions, but this case has been
            // handled above.
            ASSERT(info.condition != GPUTestConfig::kConditionNone);
            return info.name;
        }
    }

    return nullptr;
}

GPUTestExpectationsParser::GPUTestExpectationsParser()
    : mExpectationsAllowMask(
          GPUTestExpectationsParser::kGpuTestPass | GPUTestExpectationsParser::kGpuTestFail |
          GPUTestExpectationsParser::kGpuTestFlaky | GPUTestExpectationsParser::kGpuTestTimeout |
          GPUTestExpectationsParser::kGpuTestSkip)
{
    // Some initial checks.
    ASSERT((static_cast<unsigned int>(kNumberOfTokens)) ==
           (sizeof(kTokenData) / sizeof(kTokenData[0])));
    ASSERT((static_cast<unsigned int>(kNumberOfErrors)) ==
           (sizeof(kErrorMessage) / sizeof(kErrorMessage[0])));
}

GPUTestExpectationsParser::~GPUTestExpectationsParser() = default;

bool GPUTestExpectationsParser::loadTestExpectationsImpl(const GPUTestConfig *config,
                                                         const std::string &data)
{
    mEntries.clear();
    mErrorMessages.clear();

    std::vector<std::string> lines = SplitString(data, "\n", TRIM_WHITESPACE, SPLIT_WANT_ALL);
    bool rt                        = true;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (!parseLine(config, lines[i], i + 1))
            rt = false;
    }
    if (detectConflictsBetweenEntries())
    {
        mEntries.clear();
        rt = false;
    }

    return rt;
}

bool GPUTestExpectationsParser::loadTestExpectations(const GPUTestConfig &config,
                                                     const std::string &data)
{
    return loadTestExpectationsImpl(&config, data);
}

bool GPUTestExpectationsParser::loadAllTestExpectations(const std::string &data)
{
    return loadTestExpectationsImpl(nullptr, data);
}

bool GPUTestExpectationsParser::loadTestExpectationsFromFileImpl(const GPUTestConfig *config,
                                                                 const std::string &path)
{
    mEntries.clear();
    mErrorMessages.clear();

    std::string data;
    if (!ReadFileToString(path, &data))
    {
        mErrorMessages.push_back(kErrorMessage[kErrorFileIO]);
        return false;
    }
    return loadTestExpectationsImpl(config, data);
}

bool GPUTestExpectationsParser::loadTestExpectationsFromFile(const GPUTestConfig &config,
                                                             const std::string &path)
{
    return loadTestExpectationsFromFileImpl(&config, path);
}

bool GPUTestExpectationsParser::loadAllTestExpectationsFromFile(const std::string &path)
{
    return loadTestExpectationsFromFileImpl(nullptr, path);
}

int32_t GPUTestExpectationsParser::getTestExpectationImpl(const GPUTestConfig *config,
                                                          const std::string &testName)
{
    for (GPUTestExpectationEntry &entry : mEntries)
    {
        if (NamesMatchWithWildcard(entry.testName.c_str(), testName.c_str()))
        {
            // Filter by condition first.
            bool satisfiesConditions = true;
            if (config)
            {
                for (size_t condition : entry.conditions)
                {
                    if (!config->getConditions()[condition])
                    {
                        satisfiesConditions = false;
                        break;
                    }
                }
            }

            // Use the first matching expectation in the file as the matching expression.
            if (satisfiesConditions)
            {
                entry.used = true;
                return entry.testExpectation;
            }
        }
    }
    return kGpuTestPass;
}

int32_t GPUTestExpectationsParser::getTestExpectation(const std::string &testName)
{
    return getTestExpectationImpl(nullptr, testName);
}

int32_t GPUTestExpectationsParser::getTestExpectationWithConfig(const GPUTestConfig &config,
                                                                const std::string &testName)
{
    return getTestExpectationImpl(&config, testName);
}

const std::vector<std::string> &GPUTestExpectationsParser::getErrorMessages() const
{
    return mErrorMessages;
}

std::vector<std::string> GPUTestExpectationsParser::getUnusedExpectationsMessages() const
{
    std::vector<std::string> messages;
    std::vector<GPUTestExpectationsParser::GPUTestExpectationEntry> unusedExpectations =
        getUnusedExpectations();
    for (size_t i = 0; i < unusedExpectations.size(); ++i)
    {
        std::string message =
            "Line " + ToString(unusedExpectations[i].lineNumber) + ": expectation was unused.";
        messages.push_back(message);
    }
    return messages;
}

bool GPUTestExpectationsParser::parseLine(const GPUTestConfig *config,
                                          const std::string &lineData,
                                          size_t lineNumber)
{
    std::vector<std::string> tokens =
        SplitString(lineData, kWhitespaceASCII, KEEP_WHITESPACE, SPLIT_WANT_NONEMPTY);
    int32_t stage = kLineParserBegin;
    GPUTestExpectationEntry entry;
    entry.lineNumber = lineNumber;
    entry.used       = false;
    bool skipLine    = false;
    for (size_t i = 0; i < tokens.size() && !skipLine; ++i)
    {
        Token token = ParseToken(tokens[i]);
        switch (token)
        {
            case kTokenComment:
                skipLine = true;
                break;
            case kConfigWinXP:
            case kConfigWinVista:
            case kConfigWin7:
            case kConfigWin8:
            case kConfigWin10:
            case kConfigWin:
            case kConfigMacLeopard:
            case kConfigMacSnowLeopard:
            case kConfigMacLion:
            case kConfigMacMountainLion:
            case kConfigMacMavericks:
            case kConfigMacYosemite:
            case kConfigMacElCapitan:
            case kConfigMacSierra:
            case kConfigMacHighSierra:
            case kConfigMacMojave:
            case kConfigMac:
            case kConfigIOS:
            case kConfigLinux:
            case kConfigChromeOS:
            case kConfigAndroid:
            case kConfigNVIDIA:
            case kConfigAMD:
            case kConfigIntel:
            case kConfigVMWare:
            case kConfigApple:
            case kConfigRelease:
            case kConfigDebug:
            case kConfigD3D9:
            case kConfigD3D11:
            case kConfigGLDesktop:
            case kConfigGLES:
            case kConfigVulkan:
            case kConfigSwiftShader:
            case kConfigMetal:
            case kConfigWgpu:
            case kConfigNexus5X:
            case kConfigPixel2:
            case kConfigPixel4:
            case kConfigPixel6:
            case kConfigPixel7:
            case kConfigFlipN2:
            case kConfigMaliG710:
            case kConfigGalaxyA23:
            case kConfigGalaxyA34:
            case kConfigGalaxyA54:
            case kConfigGalaxyS22:
            case kConfigGalaxyS23:
            case kConfigGalaxyS24Exynos:
            case kConfigGalaxyS24Qualcomm:
            case kConfigFindX6:
            case kConfigPineapple:
            case kConfigNVIDIAQuadroP400:
            case kConfigNVIDIAGTX1660:
            case kConfigPreRotation:
            case kConfigPreRotation90:
            case kConfigPreRotation180:
            case kConfigPreRotation270:
            case kConfigNoSan:
            case kConfigASan:
            case kConfigTSan:
            case kConfigUBSan:
                // MODIFIERS, check each condition and add accordingly.
                if (stage != kLineParserConfigs && stage != kLineParserBugID)
                {
                    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
                    return false;
                }
                {
                    bool err = false;
                    if (config)
                    {
                        if (!checkTokenCondition(*config, err, token, lineNumber))
                        {
                            skipLine = true;  // Move to the next line without adding this one.
                        }
                    }
                    else
                    {
                        // Store the conditions for later comparison if we don't have a config.
                        entry.conditions[kTokenData[token].condition] = true;
                    }
                    if (err)
                    {
                        return false;
                    }
                }
                if (stage == kLineParserBugID)
                {
                    stage++;
                }
                break;
            case kSeparatorColon:
                // :
                // If there are no modifiers, move straight to separator colon
                if (stage == kLineParserBugID)
                {
                    stage++;
                }
                if (stage != kLineParserConfigs)
                {
                    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
                    return false;
                }
                stage++;
                break;
            case kSeparatorEqual:
                // =
                if (stage != kLineParserTestName)
                {
                    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
                    return false;
                }
                stage++;
                break;
            case kTokenWord:
                // BUG_ID or TEST_NAME
                if (stage == kLineParserBegin)
                {
                    // Bug ID is not used for anything; ignore it.
                }
                else if (stage == kLineParserColon)
                {
                    entry.testName = tokens[i];
                }
                else
                {
                    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
                    return false;
                }
                stage++;
                break;
            case kExpectationPass:
            case kExpectationFail:
            case kExpectationFlaky:
            case kExpectationTimeout:
            case kExpectationSkip:
                // TEST_EXPECTATIONS
                if (stage != kLineParserEqual && stage != kLineParserExpectations)
                {
                    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
                    return false;
                }
                if (entry.testExpectation != 0)
                {
                    pushErrorMessage(kErrorMessage[kErrorEntryWithExpectationConflicts],
                                     lineNumber);
                    return false;
                }
                if ((mExpectationsAllowMask & kTokenData[token].expectation) == 0)
                {
                    pushErrorMessage(kErrorMessage[kErrorEntryWithDisallowedExpectation],
                                     lineNumber);
                    return false;
                }
                entry.testExpectation = kTokenData[token].expectation;
                if (stage == kLineParserEqual)
                    stage++;
                break;
            default:
                ASSERT(false);
                break;
        }
    }
    if (stage == kLineParserBegin || skipLine)
    {
        // The whole line is empty or all comments, or has been skipped to to a condition token.
        return true;
    }
    if (stage == kLineParserExpectations)
    {
        mEntries.push_back(entry);
        return true;
    }
    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
    return false;
}

bool GPUTestExpectationsParser::checkTokenCondition(const GPUTestConfig &config,
                                                    bool &err,
                                                    int32_t token,
                                                    size_t lineNumber)
{
    if (token >= kNumberOfTokens)
    {
        pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
        err = true;
        return false;
    }

    if (kTokenData[token].condition == GPUTestConfig::kConditionNone ||
        kTokenData[token].condition >= GPUTestConfig::kNumberOfConditions)
    {
        pushErrorMessage(kErrorMessage[kErrorInvalidEntry], lineNumber);
        // error on any unsupported conditions
        err = true;
        return false;
    }
    err = false;
    return config.getConditions()[kTokenData[token].condition];
}

bool GPUTestExpectationsParser::detectConflictsBetweenEntries()
{
    bool rt = false;
    for (size_t i = 0; i < mEntries.size(); ++i)
    {
        for (size_t j = i + 1; j < mEntries.size(); ++j)
        {
            const GPUTestExpectationEntry &entryI = mEntries[i];
            const GPUTestExpectationEntry &entryJ = mEntries[j];
            if (entryI.testName == entryJ.testName &&
                ConditionsOverlap(entryI.conditions, entryJ.conditions))
            {
                pushErrorMessage(kErrorMessage[kErrorEntriesOverlap], entryI.lineNumber,
                                 entryJ.lineNumber);
                rt = true;
            }
        }
    }
    return rt;
}

std::vector<GPUTestExpectationsParser::GPUTestExpectationEntry>
GPUTestExpectationsParser::getUnusedExpectations() const
{
    std::vector<GPUTestExpectationsParser::GPUTestExpectationEntry> unusedExpectations;
    for (size_t i = 0; i < mEntries.size(); ++i)
    {
        if (!mEntries[i].used)
        {
            unusedExpectations.push_back(mEntries[i]);
        }
    }
    return unusedExpectations;
}

void GPUTestExpectationsParser::pushErrorMessage(const std::string &message, size_t lineNumber)
{
    mErrorMessages.push_back("Line " + ToString(lineNumber) + " : " + message.c_str());
}

void GPUTestExpectationsParser::pushErrorMessage(const std::string &message,
                                                 size_t entry1LineNumber,
                                                 size_t entry2LineNumber)
{
    mErrorMessages.push_back("Line " + ToString(entry1LineNumber) + " and " +
                             ToString(entry2LineNumber) + " : " + message.c_str());
}

GPUTestExpectationsParser::GPUTestExpectationEntry::GPUTestExpectationEntry()
    : testExpectation(0), lineNumber(0)
{}

}  // namespace angle
