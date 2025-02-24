//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// HistogramWriter:
//   Helper class for writing histogram-json-set-format files to JSON.

#ifndef ANGLE_TESTS_TEST_UTILS_HISTOGRAM_WRITER_H_
#define ANGLE_TESTS_TEST_UTILS_HISTOGRAM_WRITER_H_

#if !defined(ANGLE_HAS_HISTOGRAMS)
#    error "Requires ANGLE_HAS_HISTOGRAMS, see angle_maybe_has_histograms"
#endif  // !defined(ANGLE_HAS_HISTOGRAMS)

#include <map>
#include <memory>
#include <string>

// Include forward delcarations for rapidjson types.
#include <rapidjson/fwd.h>

namespace catapult
{
class HistogramBuilder;
}  // namespace catapult

namespace angle
{
class HistogramWriter
{
  public:
    HistogramWriter();
    ~HistogramWriter();

    void addSample(const std::string &measurement,
                   const std::string &story,
                   double value,
                   const std::string &units);

    void getAsJSON(rapidjson::Document *doc) const;

  private:
#if ANGLE_HAS_HISTOGRAMS
    std::map<std::string, std::unique_ptr<catapult::HistogramBuilder>> mHistograms;
#endif  // ANGLE_HAS_HISTOGRAMS
};

// Define a stub implementation when histograms are compiled out.
#if !ANGLE_HAS_HISTOGRAMS
inline HistogramWriter::HistogramWriter()  = default;
inline HistogramWriter::~HistogramWriter() = default;
inline void HistogramWriter::addSample(const std::string &measurement,
                                       const std::string &story,
                                       double value,
                                       const std::string &units)
{}
inline void HistogramWriter::getAsJSON(rapidjson::Document *doc) const {}
#endif  // !ANGLE_HAS_HISTOGRAMS

}  // namespace angle

#endif  // ANGLE_TESTS_TEST_UTILS_HISTOGRAM_WRITER_H_
