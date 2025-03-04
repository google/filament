// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This header provides macros for adding Chromium UMA histogram stats.
// See the detailed description in the Chromium codebase:
// https://source.chromium.org/chromium/chromium/src/+/main:base/metrics/histogram_macros.h

#ifndef SRC_DAWN_PLATFORM_METRICS_HISTOGRAM_MACROS_H_
#define SRC_DAWN_PLATFORM_METRICS_HISTOGRAM_MACROS_H_

#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/dawn_platform_export.h"
#include "partition_alloc/pointers/raw_ptr.h"

// Short timings - up to 10 seconds.
#define DAWN_HISTOGRAM_TIMES(platform, name, sample_ms) \
    DAWN_HISTOGRAM_CUSTOM_TIMES(platform, name, sample_ms, 1, 10'000, 50)

// Medium timings - up to 3 minutes. Note this starts at 10ms (no good reason,
// but not worth changing).
#define DAWN_HISTOGRAM_MEDIUM_TIMES(platform, name, sample_ms) \
    DAWN_HISTOGRAM_CUSTOM_TIMES(platform, name, sample_ms, 10, 180'000, 50)

// Long timings - up to an hour.
#define DAWN_HISTOGRAM_LONG_TIMES(platform, name, sample_ms) \
    DAWN_HISTOGRAM_CUSTOM_TIMES(platform, name, sample_ms, 1, 3'600'000, 50)

// Use this macro when times can routinely be much longer than 10 seconds and
#define DAWN_HISTOGRAM_LONG_TIMES_100(platform, name, sample_ms) \
    DAWN_HISTOGRAM_CUSTOM_TIMES(platform, name, sample_ms, 1, 3'600'000, 100)

// This can be used when the default ranges are not sufficient. This macro lets
// the metric developer customize the min and max of the sampled range, as well
// as the number of buckets recorded.
#define DAWN_HISTOGRAM_CUSTOM_TIMES(platform, name, sample_ms, min, max, bucket_count) \
    DAWN_HISTOGRAM_CUSTOM_COUNTS(platform, name, sample_ms, min, max, bucket_count)

// Same as DAWN_HISTOGRAM_CUSTOM_TIMES but reports |sample| in microseconds,
// dropping the report if this client doesn't have a high-resolution clock.
#define DAWN_HISTOGRAM_CUSTOM_MICROSECOND_TIMES(platform, name, sample_us, min, max, bucket_count) \
    DAWN_HISTOGRAM_CUSTOM_COUNTS_HPC(platform, name, sample_us, min, max, bucket_count)

//------------------------------------------------------------------------------
// Count histograms. These are used for collecting numeric data. Note that we
// have macros for more specialized use cases below (memory, time, percentages).

// The number suffixes here refer to the max size of the sample, i.e. COUNT_1000
// will be able to collect samples of counts up to 1000. The default number of
// buckets in all default macros is 50. We recommend erring on the side of too
// large a range versus too short a range.
// These macros default to exponential histograms - i.e. the lengths of the
// bucket ranges exponentially increase as the sample range increases.
// These should *not* be used if you are interested in exact counts, i.e. a
// bucket range of 1. In these cases, you should use the ENUMERATION macros
// defined later. These should also not be used to capture the number of some
// event, i.e. "button X was clicked N times". In this cases, an enum should be
// used, ideally with an appropriate baseline enum entry included.
// All of these macros must be called with |name| as a runtime constant.

// Used for capturing generic numeric data, from 1 to 1 million.
#define DAWN_HISTOGRAM_COUNTS(platform, name, sample) \
    DAWN_HISTOGRAM_CUSTOM_COUNTS(platform, name, sample, 1, 1'000'000, 50)

// Used for capturing generic numeric data, from 1 to 100.
#define DAWN_HISTOGRAM_COUNTS_100(platform, name, sample) \
    DAWN_HISTOGRAM_CUSTOM_COUNTS(platform, name, sample, 1, 100, 50)

// Used for capturing generic numeric data, from 1 to 10,000.
#define DAWN_HISTOGRAM_COUNTS_10000(platform, name, sample) \
    DAWN_HISTOGRAM_CUSTOM_COUNTS(platform, name, sample, 1, 10'000, 50)

// This macro allows the min, max, and number of buckets to be customized. Any
// samples whose values are outside of [min, exclusive_max-1] are put in the
// underflow or overflow buckets. Note that |min| should be >=1 as emitted 0s go
// into the underflow bucket.
#define DAWN_HISTOGRAM_CUSTOM_COUNTS(platformObj, name, sample, min, max, bucket_count) \
    platformObj->HistogramCustomCounts(name, sample, min, max, bucket_count)

// Same as DAWN_HISTOGRAM_CUSTOM_COUNTS, but the stat will be dropped if the
// client does not support high-performance counters (HPC). Useful for logging
// microsecond-resolution timings.
#define DAWN_HISTOGRAM_CUSTOM_COUNTS_HPC(platformObj, name, sample, min, max, bucket_count) \
    platformObj->HistogramCustomCountsHPC(name, sample, min, max, bucket_count)

// Used for capturing basic percentages. This will be 100 buckets of size 1.
#define DAWN_HISTOGRAM_PERCENTAGE(platform, name, percent_as_int) \
    DAWN_HISTOGRAM_ENUMERATION(platform, name, percent_as_int, 101)

// Histogram for boolean values.
#define DAWN_HISTOGRAM_BOOLEAN(platformObj, name, true_or_false) \
    platformObj->HistogramBoolean(name, true_or_false)

// Histogram for enumeration values.
#define DAWN_HISTOGRAM_ENUMERATION(platformObj, name, enum_value, enum_boundary_value) \
    platformObj->HistogramEnumeration(name, enum_value, enum_boundary_value)

// Used to measure common KB-granularity memory stats. Range is up to 500000KB -
// approximately 500M.
#define DAWN_HISTOGRAM_MEMORY_KB(platform, name, sample_kb) \
    DAWN_HISTOGRAM_CUSTOM_COUNTS(platform, name, sample_kb, 1000, 500'000, 50)

// MB-granularity memory metric. This has a short max (1G).
#define DAWN_HISTOGRAM_MEMORY_MB(platform, name, sample_mb) \
    DAWN_HISTOGRAM_CUSTOM_COUNTS(platform, name, sample_mb, 1, 1000, 50)

// Used to measure common MB-granularity memory stats. Range is up to 4000MiB -
// approximately 4GiB.
#define DAWN_HISTOGRAM_MEMORY_MEDIUM_MB(name, sample_mb) \
    DAWN_HISTOGRAM_CUSTOM_COUNTS(name, sample_mb, 1, 4000, 100)

// Used to measure common MB-granularity memory stats. Range is up to ~64G.
#define DAWN_HISTOGRAM_MEMORY_LARGE_MB(name, sample_mb) \
    DAWN_HISTOGRAM_CUSTOM_COUNTS(name, sample_mb, 1, 64000, 100)

// Sparse histograms are well suited for recording counts of exact sample values
// that are sparsely distributed over a relatively large range, in cases where
// ultra-fast performance is not critical. For instance, Sqlite.Version.* are
// sparse because for any given database, there's going to be exactly one
// version logged.
//
// For important details on performance, data size, and usage, see the
// documentation on the regular function equivalents (histogram_functions.h).
#define DAWN_HISTOGRAM_SPARSE(platformObj, name, sparse_sample) \
    platformObj->HistogramSparse(name, sparse_sample)

namespace dawn::detail {
enum class ScopedHistogramTiming { kMicrosecondTimes, kMediumTimes, kLongTimes };
}

// Scoped class which logs its time on this earth as a UMA statistic. This is
// recommended for when you want a histogram which measures the time it takes
// for a method to execute. This measures up to 10 seconds.
#define SCOPED_DAWN_HISTOGRAM_TIMER(platform, name) \
    SCOPED_DAWN_HISTOGRAM_TIMER_EXPANDER(           \
        platform, name, dawn::detail::ScopedHistogramTiming::kMediumTimes, __COUNTER__)

// Similar scoped histogram timer, but this uses DAWN_HISTOGRAM_LONG_TIMES_100,
// which measures up to an hour, and uses 100 buckets. This is more expensive
// to store, so only use if this often takes >10 seconds.
#define SCOPED_DAWN_HISTOGRAM_LONG_TIMER(platform, name) \
    SCOPED_DAWN_HISTOGRAM_TIMER_EXPANDER(                \
        platform, name, dawn::detail::ScopedHistogramTiming::kLongTimes, __COUNTER__)

// Similar scoped histogram timer, but this uses
// DAWN_HISTOGRAM_CUSTOM_MICROSECOND_TIMES, measuring from 1 microseconds to 1 second,
// with 50 buckets.
#define SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(platform, name) \
    SCOPED_DAWN_HISTOGRAM_TIMER_EXPANDER(                  \
        platform, name, dawn::detail::ScopedHistogramTiming::kMicrosecondTimes, __COUNTER__)

// This nested macro is necessary to expand __COUNTER__ to an actual value.
#define SCOPED_DAWN_HISTOGRAM_TIMER_EXPANDER(platform, name, timing, key) \
    SCOPED_DAWN_HISTOGRAM_TIMER_UNIQUE(platform, name, timing, key)

// This is a helper macro used by other macros and shouldn't be used directly.
#define SCOPED_DAWN_HISTOGRAM_TIMER_UNIQUE(platform, name, timing, key)                     \
    using PlatformType##key = std::decay_t<std::remove_pointer_t<decltype(&*platform)>>;    \
    class [[nodiscard]] ScopedHistogramTimer##key {                                         \
      public:                                                                               \
        using Platform = PlatformType##key;                                                 \
        ScopedHistogramTimer##key(Platform* p)                                              \
            : platform_(p), constructed_(platform_->MonotonicallyIncreasingTime()) {}       \
        ~ScopedHistogramTimer##key() {                                                      \
            if (constructed_ == 0)                                                          \
                return;                                                                     \
            double elapsed = this->platform_->MonotonicallyIncreasingTime() - constructed_; \
            switch (timing) {                                                               \
                case dawn::detail::ScopedHistogramTiming::kMicrosecondTimes: {              \
                    int elapsedUS = static_cast<int>(elapsed * 1'000'000.0);                \
                    DAWN_HISTOGRAM_CUSTOM_MICROSECOND_TIMES(platform_, name, elapsedUS, 1,  \
                                                            1'000'000, 50);                 \
                } break;                                                                    \
                case dawn::detail::ScopedHistogramTiming::kMediumTimes: {                   \
                    int elapsedMS = static_cast<int>(elapsed * 1'000.0);                    \
                    DAWN_HISTOGRAM_TIMES(platform_, name, elapsedMS);                       \
                } break;                                                                    \
                case dawn::detail::ScopedHistogramTiming::kLongTimes: {                     \
                    int elapsedMS = static_cast<int>(elapsed * 1'000.0);                    \
                    DAWN_HISTOGRAM_LONG_TIMES_100(platform_, name, elapsedMS);              \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
                                                                                            \
      private:                                                                              \
        Platform* platform_;                                                                \
        double constructed_;                                                                \
    } scoped_histogram_timer_##key(platform)

namespace dawn::platform::metrics {

// Timer class that gives more flexibility for recording over the macros when there may be
// conditionals on the name or whether to record at all.
class DAWN_PLATFORM_EXPORT DawnHistogramTimer {
  public:
    explicit DawnHistogramTimer(dawn::platform::Platform* platform);

    // Timing recording function(s) records the time since this object was constructed until when
    // the recording function is called.
    void RecordMicroseconds(const char* name);

    // Resets the effective start timer. Useful to reuse the same timer instead of constructing a
    // new one when dealing with conditional scopes.
    void Reset();

  private:
    const raw_ptr<dawn::platform::Platform> mPlatform;
    double mConstructed;
};

}  // namespace dawn::platform::metrics

#endif  // SRC_DAWN_PLATFORM_METRICS_HISTOGRAM_MACROS_H_
