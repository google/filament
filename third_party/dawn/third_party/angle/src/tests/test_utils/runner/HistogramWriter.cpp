//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// HistogramWriter:
//   Helper class for writing histogram-json-set-format files to JSON.

#include "HistogramWriter.h"

#include "common/debug.h"

#include <rapidjson/document.h>

#if !ANGLE_HAS_HISTOGRAMS
#    error "Must have histograms enabled"
#endif  // !ANGLE_HAS_HISTOGRAMS

ANGLE_DISABLE_EXTRA_SEMI_WARNING
ANGLE_DISABLE_EXTRA_SEMI_STMT_WARNING
ANGLE_DISABLE_DESTRUCTOR_OVERRIDE_WARNING
ANGLE_DISABLE_SUGGEST_OVERRIDE_WARNINGS
#include "tracing/tracing/value/diagnostics/reserved_infos.h"
#include "tracing/tracing/value/histogram.h"
ANGLE_REENABLE_SUGGEST_OVERRIDE_WARNINGS
ANGLE_REENABLE_DESTRUCTOR_OVERRIDE_WARNING
ANGLE_REENABLE_EXTRA_SEMI_STMT_WARNING
ANGLE_REENABLE_EXTRA_SEMI_WARNING

namespace js    = rapidjson;
namespace proto = catapult::tracing::tracing::proto;

namespace angle
{
namespace
{
std::string UnitAndDirectionToString(proto::UnitAndDirection unit)
{
    std::stringstream strstr;

    switch (unit.unit())
    {
        case proto::MS_BEST_FIT_FORMAT:
            strstr << "msBestFitFormat";
            break;
        case proto::COUNT:
            strstr << "count";
            break;
        case proto::SIZE_IN_BYTES:
            strstr << "sizeInBytes";
            break;
        default:
            UNREACHABLE();
            strstr << "error";
            break;
    }

    switch (unit.improvement_direction())
    {
        case proto::NOT_SPECIFIED:
            break;
        case proto::SMALLER_IS_BETTER:
            strstr << "_smallerIsBetter";
            break;
        default:
            UNREACHABLE();
            break;
    }

    return strstr.str();
}

proto::UnitAndDirection StringToUnitAndDirection(const std::string &str)
{
    proto::UnitAndDirection unitAndDirection;

    if (str == "count")
    {
        unitAndDirection.set_improvement_direction(proto::NOT_SPECIFIED);
        unitAndDirection.set_unit(proto::COUNT);
    }
    else if (str == "msBestFitFormat_smallerIsBetter")
    {
        unitAndDirection.set_improvement_direction(proto::SMALLER_IS_BETTER);
        unitAndDirection.set_unit(proto::MS_BEST_FIT_FORMAT);
    }
    else if (str == "sizeInBytes_smallerIsBetter")
    {
        unitAndDirection.set_improvement_direction(proto::SMALLER_IS_BETTER);
        unitAndDirection.set_unit(proto::SIZE_IN_BYTES);
    }
    else
    {
        UNREACHABLE();
    }

    return unitAndDirection;
}
}  // namespace

HistogramWriter::HistogramWriter() = default;

HistogramWriter::~HistogramWriter() = default;

void HistogramWriter::addSample(const std::string &measurement,
                                const std::string &story,
                                double value,
                                const std::string &units)
{
    std::string measurementAndStory = measurement + story;
    if (mHistograms.count(measurementAndStory) == 0)
    {
        proto::UnitAndDirection unitAndDirection = StringToUnitAndDirection(units);

        std::unique_ptr<catapult::HistogramBuilder> builder =
            std::make_unique<catapult::HistogramBuilder>(measurement, unitAndDirection);

        // Set all summary options as false - we don't want to generate metric_std, metric_count,
        // and so on for all metrics.
        builder->SetSummaryOptions(proto::SummaryOptions());
        mHistograms[measurementAndStory] = std::move(builder);

        proto::Diagnostic stories;
        proto::GenericSet *genericSet = stories.mutable_generic_set();
        genericSet->add_values(story);
        mHistograms[measurementAndStory]->AddDiagnostic(catapult::kStoriesDiagnostic, stories);
    }

    mHistograms[measurementAndStory]->AddSample(value);
}

void HistogramWriter::getAsJSON(js::Document *doc) const
{
    proto::HistogramSet histogramSet;

    for (const auto &histogram : mHistograms)
    {
        std::unique_ptr<proto::Histogram> proto = histogram.second->toProto();
        histogramSet.mutable_histograms()->AddAllocated(proto.release());
    }

    // Custom JSON serialization for histogram-json-set.
    doc->SetArray();

    js::Document::AllocatorType &allocator = doc->GetAllocator();

    for (int histogramIndex = 0; histogramIndex < histogramSet.histograms_size(); ++histogramIndex)
    {
        const proto::Histogram &histogram = histogramSet.histograms(histogramIndex);

        js::Value obj(js::kObjectType);

        js::Value name(histogram.name(), allocator);
        obj.AddMember("name", name, allocator);

        js::Value description(histogram.description(), allocator);
        obj.AddMember("description", description, allocator);

        js::Value unitAndDirection(UnitAndDirectionToString(histogram.unit()), allocator);
        obj.AddMember("unit", unitAndDirection, allocator);

        if (histogram.has_diagnostics())
        {
            js::Value diags(js::kObjectType);

            for (const auto &mapIter : histogram.diagnostics().diagnostic_map())
            {
                js::Value key(mapIter.first, allocator);
                const proto::Diagnostic &diagnostic = mapIter.second;

                if (!diagnostic.shared_diagnostic_guid().empty())
                {
                    js::Value guid(diagnostic.shared_diagnostic_guid(), allocator);
                    diags.AddMember(key, guid, allocator);
                }
                else if (diagnostic.has_generic_set())
                {
                    const proto::GenericSet genericSet = diagnostic.generic_set();

                    js::Value setObj(js::kObjectType);
                    setObj.AddMember("type", "GenericSet", allocator);

                    js::Value values(js::kArrayType);

                    for (const std::string &value : genericSet.values())
                    {
                        js::Value valueStr(value, allocator);
                        values.PushBack(valueStr, allocator);
                    }

                    setObj.AddMember("values", values, allocator);

                    diags.AddMember(key, setObj, allocator);
                }
                else
                {
                    UNREACHABLE();
                }
            }

            obj.AddMember("diagnostics", diags, allocator);
        }

        js::Value sampleValues(js::kArrayType);

        for (int sampleIndex = 0; sampleIndex < histogram.sample_values_size(); ++sampleIndex)
        {
            js::Value sample(histogram.sample_values(sampleIndex));
            sampleValues.PushBack(sample, allocator);
        }

        obj.AddMember("sampleValues", sampleValues, allocator);

        js::Value maxNumSamplesValues(histogram.max_num_sample_values());
        obj.AddMember("maxNumSamplesValues", maxNumSamplesValues, allocator);

        if (histogram.has_bin_boundaries())
        {
            js::Value binBoundaries(js::kArrayType);

            const proto::BinBoundaries &boundaries = histogram.bin_boundaries();
            for (int binIndex = 0; binIndex < boundaries.bin_specs_size(); ++binIndex)
            {
                js::Value binSpec(boundaries.bin_specs(binIndex).bin_boundary());
                binBoundaries.PushBack(binSpec, allocator);
            }

            obj.AddMember("binBoundaries", binBoundaries, allocator);
        }

        if (histogram.has_summary_options())
        {
            const proto::SummaryOptions &options = histogram.summary_options();

            js::Value summary(js::kObjectType);

            js::Value avg(options.avg());
            js::Value count(options.count());
            js::Value max(options.max());
            js::Value min(options.min());
            js::Value std(options.std());
            js::Value sum(options.sum());

            summary.AddMember("avg", avg, allocator);
            summary.AddMember("count", count, allocator);
            summary.AddMember("max", max, allocator);
            summary.AddMember("min", min, allocator);
            summary.AddMember("std", std, allocator);
            summary.AddMember("sum", sum, allocator);

            obj.AddMember("summaryOptions", summary, allocator);
        }

        if (histogram.has_running())
        {
            const proto::RunningStatistics &running = histogram.running();

            js::Value stats(js::kArrayType);

            js::Value count(running.count());
            js::Value max(running.max());
            js::Value meanlogs(running.meanlogs());
            js::Value mean(running.mean());
            js::Value min(running.min());
            js::Value sum(running.sum());
            js::Value variance(running.variance());

            stats.PushBack(count, allocator);
            stats.PushBack(max, allocator);
            stats.PushBack(meanlogs, allocator);
            stats.PushBack(mean, allocator);
            stats.PushBack(min, allocator);
            stats.PushBack(sum, allocator);
            stats.PushBack(variance, allocator);

            obj.AddMember("running", stats, allocator);
        }

        doc->PushBack(obj, allocator);
    }

    for (const auto &diagnosticIt : histogramSet.shared_diagnostics())
    {
        const proto::Diagnostic &diagnostic = diagnosticIt.second;

        js::Value obj(js::kObjectType);

        js::Value name(diagnosticIt.first, allocator);
        obj.AddMember("name", name, allocator);

        switch (diagnostic.diagnostic_oneof_case())
        {
            case proto::Diagnostic::kGenericSet:
            {
                js::Value type("GenericSet", allocator);
                obj.AddMember("type", type, allocator);

                const proto::GenericSet &genericSet = diagnostic.generic_set();

                js::Value values(js::kArrayType);

                for (const std::string &value : genericSet.values())
                {
                    js::Value valueStr(value, allocator);
                    values.PushBack(valueStr, allocator);
                }

                obj.AddMember("values", values, allocator);
                break;
            }

            default:
                UNREACHABLE();
        }

        doc->PushBack(obj, allocator);
    }
}
}  // namespace angle
