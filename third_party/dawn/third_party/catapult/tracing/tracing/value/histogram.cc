// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tracing/tracing/value/histogram.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <map>
#include <random>

#include "tracing/tracing/value/running_statistics.h"

namespace catapult {

namespace proto = tracing::tracing::proto;

static constexpr std::pair<const char*, proto::Unit> kJsonUnitToProtoUnit[] = {
    {"ms", proto::MS},
    {"msBestFitFormat", proto::MS_BEST_FIT_FORMAT},
    {"tsMs", proto::TS_MS},
    {"n%", proto::N_PERCENT},
    {"sizeInBytes", proto::SIZE_IN_BYTES},
    {"bytesPerSecond", proto::BYTES_PER_SECOND},
    {"J", proto::J},
    {"W", proto::W},
    {"A", proto::A},
    {"Ah", proto::AH},
    {"V", proto::V},
    {"Hz", proto::HERTZ},
    {"unitless", proto::UNITLESS},
    {"count", proto::COUNT},
    {"sigma", proto::SIGMA}};

// Assume a single bin. The default num sample values is num bins * 10.
static constexpr int kDefaultNumSampleValues = 10;

class HistogramBuilder::Resampler {
 public:
  Resampler() : distribution_(0.0, 1.0) {}

  // When processing a stream of samples, call this method for each new sample
  // in order to decide whether to keep it in |samples|.
  // Modifies |samples| in-place such that its length never exceeds
  // |max_num_samples|. After |stream_length| samples have been processed, each
  // sample has equal probability of being retained in |samples|. The order of
  // samples is not preserved after |stream_length| exceeds |num_samples|.
  void UniformlySampleStream(std::vector<double>* samples,
                             uint32_t stream_length,
                             double new_element,
                             uint32_t max_num_samples) {
    assert(max_num_samples > 0);

    if (stream_length <= max_num_samples) {
      if (samples->size() >= stream_length) {
        (*samples)[stream_length - 1] = new_element;
      } else {
        samples->push_back(new_element);
      }
      return;
    }
    double prob_keep = static_cast<double>(max_num_samples) / stream_length;
    if (random() > prob_keep) {
      // Reject new sample.
      return;
    }

    // Replace a random element.
    int victim = static_cast<int>(std::floor(random() * max_num_samples));
    (*samples)[victim] = new_element;
  }

 private:
  double random() { return distribution_(generator_); }

  std::default_random_engine generator_;
  std::uniform_real_distribution<double> distribution_;
};

HistogramBuilder::HistogramBuilder(
    const std::string& name, proto::UnitAndDirection unit)
    : resampler_(std::make_unique<Resampler>()),
      running_statistics_(std::make_unique<RunningStatistics>()),
      name_(name),
      unit_(unit),
      num_nans_(0) {
  max_num_sample_values_ = kDefaultNumSampleValues;
}

HistogramBuilder::~HistogramBuilder() = default;

void HistogramBuilder::AddSample(double value) {
  if (std::isnan(value)) {
    num_nans_++;
  } else {
    running_statistics_->Add(value);
    int num_values = running_statistics_->count();
    resampler_->UniformlySampleStream(&sample_values_, num_nans_ + num_values,
                                      value, max_num_sample_values_);
  }
}

void HistogramBuilder::AddDiagnostic(
    const std::string& key,
    tracing::tracing::proto::Diagnostic diagnostic) {
  diagnostics_[key] = diagnostic;
}

void HistogramBuilder::SetSummaryOptions(proto::SummaryOptions options) {
  options_ = options;
}

std::unique_ptr<proto::Histogram> HistogramBuilder::toProto() const {
  auto histogram = std::make_unique<proto::Histogram>();
  histogram->set_name(name_);
  *histogram->mutable_unit() = unit_;
  histogram->set_description(description_);

  proto::DiagnosticMap* diagnostics = histogram->mutable_diagnostics();
  for (const auto& pair : diagnostics_) {
    auto* diagnostic_map = diagnostics->mutable_diagnostic_map();
    (*diagnostic_map)[pair.first] = pair.second;
  }

  for (double sample: sample_values_) {
    histogram->add_sample_values(sample);
  }

  histogram->set_max_num_sample_values(max_num_sample_values_);

  histogram->set_num_nans(num_nans_);

  proto::RunningStatistics* running = histogram->mutable_running();
  running->set_count(running_statistics_->count());
  running->set_max(running_statistics_->max());
  if (running_statistics_->meanlogs_valid()) {
    running->set_meanlogs(running_statistics_->meanlogs());
  }
  running->set_mean(running_statistics_->mean());
  running->set_min(running_statistics_->min());
  running->set_sum(running_statistics_->sum());
  running->set_variance(running_statistics_->variance());

  proto::SummaryOptions* options = histogram->mutable_summary_options();
  *options = options_;

  return histogram;
}

tracing::tracing::proto::Unit UnitFromJsonUnit(std::string unit) {
  unit.erase(std::find(unit.begin(), unit.end(), '_'), unit.end());

  for (const auto& pair : kJsonUnitToProtoUnit) {
    if (unit == pair.first) {
      return pair.second;
    }
  }

  return proto::UNITLESS;
}

}  // namespace catapult
