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
#include <filament/driver/DriverEnums.h>

#include <filamat/MaterialBuilder.h>

#include <memory>
#include <ostream>

#include <utils/compiler.h>

namespace matc {

class Config {
public:
    enum class Mode {
        MATERIAL,
        DEPTH,
        POSTPROCESS,
    };

    enum class OutputFormat {
        BLOB,
        C_HEADER,
    };

    using Platform = filamat::MaterialBuilder::Platform;
    using TargetApi = filamat::MaterialBuilder::TargetApi;
    using Optimization = filamat::MaterialBuilder::Optimization;

    enum class Metadata {
        NONE,
        PARAMETERS
    };

    virtual ~Config() {}

    class Output {
    public:
        virtual ~Output() {}
        virtual bool open() noexcept = 0;
        virtual bool write(const uint8_t* data, size_t size) noexcept = 0;
        virtual std::ostream& getOutputStream() noexcept = 0;
        virtual bool close() noexcept = 0;
    };
    virtual Output* getOutput()  const noexcept = 0;
    virtual Output* getAsmOutput()  const noexcept { return nullptr; }

    class Input {
    public:
        virtual ~Input() {}
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

    Mode getMode() const noexcept {
        return mMode;
    }

    Platform getPlatform() const noexcept {
        return mPlatform;
    }

    OutputFormat getOutputFormat() const noexcept {
        return mOutputFormat;
    }

    bool isAsmBlobEnabled() const noexcept {
        return mIsAsmBlobEnabled;
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

    /**
     * Returns the target API suitable for the current optimization level. It might be
     * different than the target API returned by getTargetApi().
     */
    TargetApi getCodeGenTargetApi() const noexcept {
        // When optimizing OpenGL we use SPIRV as an intermediate representation so we must force
        // the target API to be Vulkan for the generated shaders to compile
        return mOptimizationLevel > Optimization::PREPROCESSOR && mTargetApi != TargetApi::VULKAN ?
                TargetApi::VULKAN : mTargetApi;
    }

    bool printShaders() const noexcept {
        return mPrintShaders;
    }

    uint8_t getVariantFilter() const noexcept {
        return mVariantFilter;
    }

protected:
    bool mDebug = false;
    bool mIsValid = true;
    bool mPrintShaders = false;
    bool mIsAsmBlobEnabled = true;
    Optimization mOptimizationLevel = Optimization::NONE;
    Metadata mReflectionTarget = Metadata::NONE;
    Mode mMode = Mode::MATERIAL;
    Platform mPlatform = Platform::ALL;
    OutputFormat mOutputFormat = OutputFormat::BLOB;
    TargetApi mTargetApi = TargetApi::OPENGL;
    uint8_t mVariantFilter = 0;
};

}

#endif //TNT_CONFIG_H
