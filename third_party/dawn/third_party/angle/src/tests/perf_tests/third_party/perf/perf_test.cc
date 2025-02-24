// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "perf_test.h"

#include "common/base/anglebase/no_destructor.h"

#include <stdarg.h>
#include <stdio.h>
#include <vector>

namespace
{

std::string FormatString(const char *fmt, va_list vararg)
{
    static angle::base::NoDestructor<std::vector<char>> buffer(512);

    // Attempt to just print to the current buffer
    int len = vsnprintf(buffer->data(), buffer->size(), fmt, vararg);
    if (len < 0 || static_cast<size_t>(len) >= buffer->size())
    {
        // Buffer was not large enough, calculate the required size and resize the buffer
        len = vsnprintf(NULL, 0, fmt, vararg);
        buffer->resize(len + 1);

        // Print again
        vsnprintf(buffer->data(), buffer->size(), fmt, vararg);
    }

    return std::string(buffer->data(), len);
}

std::string StringPrintf(const char *fmt, ...)
{
    va_list vararg;
    va_start(vararg, fmt);
    std::string result = FormatString(fmt, vararg);
    va_end(vararg);
    return result;
}

std::string NumberToString(size_t value)
{
    return StringPrintf("%u", value);
}

std::string NumberToString(double value)
{
    return StringPrintf("%.10lf", value);
}

std::string ResultsToString(const std::string &measurement,
                            const std::string &modifier,
                            const std::string &trace,
                            const std::string &values,
                            const std::string &prefix,
                            const std::string &suffix,
                            const std::string &units,
                            bool important)
{
    // <*>RESULT <graph_name>: <trace_name>= <value> <units>
    // <*>RESULT <graph_name>: <trace_name>= {<mean>, <std deviation>} <units>
    // <*>RESULT <graph_name>: <trace_name>= [<value>,value,value,...,] <units>
    return StringPrintf("%sRESULT %s%s: %s= %s%s%s %s\n", important ? "*" : "", measurement.c_str(),
                        modifier.c_str(), trace.c_str(), prefix.c_str(), values.c_str(),
                        suffix.c_str(), units.c_str());
}

void PrintResultsImpl(const std::string &measurement,
                      const std::string &modifier,
                      const std::string &trace,
                      const std::string &values,
                      const std::string &prefix,
                      const std::string &suffix,
                      const std::string &units,
                      bool important)
{
    fflush(stdout);
    printf("%s",
           ResultsToString(measurement, modifier, trace, values, prefix, suffix, units, important)
               .c_str());
    fflush(stdout);
}

}  // namespace

namespace perf_test
{

void PrintResult(const std::string &measurement,
                 const std::string &modifier,
                 const std::string &trace,
                 size_t value,
                 const std::string &units,
                 bool important)
{
    PrintResultsImpl(measurement, modifier, trace, NumberToString(value), std::string(),
                     std::string(), units, important);
}

void PrintResult(const std::string &measurement,
                 const std::string &modifier,
                 const std::string &trace,
                 double value,
                 const std::string &units,
                 bool important)
{
    PrintResultsImpl(measurement, modifier, trace, NumberToString(value), std::string(),
                     std::string(), units, important);
}

void AppendResult(std::string &output,
                  const std::string &measurement,
                  const std::string &modifier,
                  const std::string &trace,
                  size_t value,
                  const std::string &units,
                  bool important)
{
    output += ResultsToString(measurement, modifier, trace, NumberToString(value), std::string(),
                              std::string(), units, important);
}

void PrintResult(const std::string &measurement,
                 const std::string &modifier,
                 const std::string &trace,
                 const std::string &value,
                 const std::string &units,
                 bool important)
{
    PrintResultsImpl(measurement, modifier, trace, value, std::string(), std::string(), units,
                     important);
}

void AppendResult(std::string &output,
                  const std::string &measurement,
                  const std::string &modifier,
                  const std::string &trace,
                  const std::string &value,
                  const std::string &units,
                  bool important)
{
    output += ResultsToString(measurement, modifier, trace, value, std::string(), std::string(),
                              units, important);
}

void PrintResultMeanAndError(const std::string &measurement,
                             const std::string &modifier,
                             const std::string &trace,
                             const std::string &mean_and_error,
                             const std::string &units,
                             bool important)
{
    PrintResultsImpl(measurement, modifier, trace, mean_and_error, "{", "}", units, important);
}

void AppendResultMeanAndError(std::string &output,
                              const std::string &measurement,
                              const std::string &modifier,
                              const std::string &trace,
                              const std::string &mean_and_error,
                              const std::string &units,
                              bool important)
{
    output +=
        ResultsToString(measurement, modifier, trace, mean_and_error, "{", "}", units, important);
}

void PrintResultList(const std::string &measurement,
                     const std::string &modifier,
                     const std::string &trace,
                     const std::string &values,
                     const std::string &units,
                     bool important)
{
    PrintResultsImpl(measurement, modifier, trace, values, "[", "]", units, important);
}

void AppendResultList(std::string &output,
                      const std::string &measurement,
                      const std::string &modifier,
                      const std::string &trace,
                      const std::string &values,
                      const std::string &units,
                      bool important)
{
    output += ResultsToString(measurement, modifier, trace, values, "[", "]", units, important);
}

void PrintSystemCommitCharge(const std::string &test_name, size_t charge, bool important)
{
    PrintSystemCommitCharge(stdout, test_name, charge, important);
}

void PrintSystemCommitCharge(FILE *target,
                             const std::string &test_name,
                             size_t charge,
                             bool important)
{
    fprintf(target, "%s", SystemCommitChargeToString(test_name, charge, important).c_str());
}

std::string SystemCommitChargeToString(const std::string &test_name, size_t charge, bool important)
{
    std::string trace_name(test_name);
    std::string output;
    AppendResult(output, "commit_charge", std::string(), "cc" + trace_name, charge, "kb",
                 important);
    return output;
}

}  // namespace perf_test
