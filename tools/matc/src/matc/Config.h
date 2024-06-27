/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_CONFIG_H
#define TNT_CONFIG_H

#include <filament/MaterialEnums.h>
#include <backend/DriverEnums.h>

#include <filamat/MaterialBuilder.h>

#include <map>
#include <memory>
#include <ostream>

#include <utils/compiler.h>

namespace matc {

class Config {
public:
    enum class OutputFormat {
        BLOB,
        C_HEADER,
    };

    using Platform = filamat::MaterialBuilder::Platform;
    using TargetApi = filamat::MaterialBuilder::TargetApi;
    using Optimization = filamat::MaterialBuilder::Optimization;

    // For defines, template, and material parameters, we use an ordered map with a transparent comparator.
    // Even though the key is stored using std::string, this allows you to make lookups using
    // std::string_view. There is no need to construct a std::string object just to make a lookup.
    using StringReplacementMap = std::map<std::string, std::string, std::less<>>;

    enum class Metadata {
        NONE,
        PARAMETERS
    };

    virtual ~Config() = default;

    class Output {
    public:
        virtual ~Output() = default;
        virtual bool open() noexcept = 0;
        virtual bool write(const uint8_t* data, size_t size) noexcept = 0;
        virtual std::ostream& getOutputStream() noexcept = 0;
        virtual bool close() noexcept = 0;
    };
    virtual Output* getOutput()  const noexcept = 0;

    class Input {
    public:
        virtual ~Input() = default;
        virtual ssize_t open() noexcept = 0;
        virtual std::unique_ptr<const char[]> read() noexcept = 0;
        virtual bool close() noexcept = 0;
        virtual const char* getName() const noexcept = 0;
    };
    virtual Input* getInput() const noexcept = 0;

    virtual std::string toString() const noexcept = 0;

    bool isDebug() const noexcept {
        return mDebug;
    }

    Platform getPlatform() const noexcept {
        return mPlatform;
    }

    OutputFormat getOutputFormat() const noexcept {
        return mOutputFormat;
    }

    bool isValid() const noexcept {
        return mIsValid;
    }

    Optimization getOptimizationLevel() const noexcept {
        return mOptimizationLevel;
    }

    void setOptimizationLevel(Optimization level) noexcept {
        mOptimizationLevel = level;
    }

    Metadata getReflectionTarget() const noexcept {
        return mReflectionTarget;
    }

    TargetApi getTargetApi() const noexcept {
        return mTargetApi;
    }

    bool printShaders() const noexcept {
        return mPrintShaders;
    }

    bool saveRawVariants() const noexcept {
        return mSaveRawVariants;
    }

    bool rawShaderMode() const noexcept {
        return mRawShaderMode;
    }

    bool noSamplerValidation() const noexcept {
        return mNoSamplerValidation;
    }

    bool includeEssl1() const noexcept {
        return mIncludeEssl1;
    }

    filament::UserVariantFilterMask getVariantFilter() const noexcept {
        return mVariantFilter;
    }

    const StringReplacementMap& getDefines() const noexcept {
        return mDefines;
    }

    const StringReplacementMap& getTemplateMap() const noexcept {
        return mTemplateMap;
    }

    const StringReplacementMap& getMaterialParameters() const noexcept {
        return mMaterialParameters;
    }

    filament::backend::FeatureLevel getFeatureLevel() const noexcept {
        return mFeatureLevel;
    }

protected:
    bool mDebug = false;
    bool mIsValid = true;
    bool mPrintShaders = false;
    bool mRawShaderMode = false;
    bool mNoSamplerValidation = false;
    bool mSaveRawVariants = false;
    Optimization mOptimizationLevel = Optimization::PERFORMANCE;
    Metadata mReflectionTarget = Metadata::NONE;
    Platform mPlatform = Platform::ALL;
    OutputFormat mOutputFormat = OutputFormat::BLOB;
    TargetApi mTargetApi = (TargetApi) 0;
    filament::backend::FeatureLevel mFeatureLevel = filament::backend::FeatureLevel::FEATURE_LEVEL_3;
    StringReplacementMap mDefines;
    StringReplacementMap mTemplateMap;
    StringReplacementMap mMaterialParameters;
    filament::UserVariantFilterMask mVariantFilter = 0;
    bool mIncludeEssl1 = true;
};

}

#endif //TNT_CONFIG_H
