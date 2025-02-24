//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef TEST_EXPECTATIONS_GPU_TEST_EXPECTATIONS_PARSER_H_
#define TEST_EXPECTATIONS_GPU_TEST_EXPECTATIONS_PARSER_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "GPUTestConfig.h"

namespace angle
{
struct GPUTestConfig;

class GPUTestExpectationsParser
{
  public:
    enum GPUTestExpectation
    {
        kGpuTestPass    = 1 << 0,
        kGpuTestFail    = 1 << 1,
        kGpuTestFlaky   = 1 << 2,
        kGpuTestTimeout = 1 << 3,
        kGpuTestSkip    = 1 << 4,
    };

    GPUTestExpectationsParser();
    ~GPUTestExpectationsParser();

    // Parse the text expectations, and if no error is encountered,
    // save all the entries. Otherwise, generate error messages.
    // Return true if parsing succeeds.
    bool loadTestExpectations(const GPUTestConfig &config, const std::string &data);
    bool loadTestExpectationsFromFile(const GPUTestConfig &config, const std::string &path);
    bool loadAllTestExpectations(const std::string &data);
    bool loadAllTestExpectationsFromFile(const std::string &path);

    // Query error messages from the last LoadTestExpectations() call.
    const std::vector<std::string> &getErrorMessages() const;

    // Query error messages from any expectations that weren't used before being queried.
    std::vector<std::string> getUnusedExpectationsMessages() const;

    // Get the test expectation of a given test on a given bot.
    int32_t getTestExpectation(const std::string &testName);
    int32_t getTestExpectationWithConfig(const GPUTestConfig &config, const std::string &testName);
    void setTestExpectationsAllowMask(uint32_t mask) { mExpectationsAllowMask = mask; }

  private:
    struct GPUTestExpectationEntry
    {
        GPUTestExpectationEntry();

        std::string testName;
        int32_t testExpectation;
        size_t lineNumber;
        bool used;
        GPUTestConfig::ConditionArray conditions;
    };

    // Parse a line of text. If we have a valid entry, save it; otherwise,
    // generate error messages.
    bool parseLine(const GPUTestConfig *config, const std::string &lineData, size_t lineNumber);

    // Check a the condition assigned to a particular token.
    bool checkTokenCondition(const GPUTestConfig &config,
                             bool &err,
                             int32_t token,
                             size_t lineNumber);

    // Check if two entries' config overlap with each other. May generate an
    // error message.
    bool detectConflictsBetweenEntries();

    // Query a list of any expectations that were's used before being queried.
    std::vector<GPUTestExpectationEntry> getUnusedExpectations() const;

    // Save an error message, which can be queried later.
    void pushErrorMessage(const std::string &message, size_t lineNumber);
    void pushErrorMessage(const std::string &message,
                          size_t entry1LineNumber,
                          size_t entry2LineNumber);

    // Config is optional.
    bool loadTestExpectationsFromFileImpl(const GPUTestConfig *config, const std::string &path);
    bool loadTestExpectationsImpl(const GPUTestConfig *config, const std::string &data);

    int32_t getTestExpectationImpl(const GPUTestConfig *config, const std::string &testName);

    std::vector<GPUTestExpectationEntry> mEntries;
    std::vector<std::string> mErrorMessages;

    uint32_t mExpectationsAllowMask;
};

const char *GetConditionName(uint32_t condition);

}  // namespace angle

#endif  // TEST_EXPECTATIONS_GPU_TEST_EXPECTATIONS_PARSER_H_
