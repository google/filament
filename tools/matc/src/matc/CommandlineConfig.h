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

#ifndef TNT_COMPILERPARAMETERS_H
#define TNT_COMPILERPARAMETERS_H

#include <filament-matp/Config.h>
#include <fstream>
#include <iostream>
#include <string>

namespace matc {

class FilesystemOutput : public matp::Config::Output {
public:
    explicit FilesystemOutput(const char* path) : mPath(path) {
    }

    ~FilesystemOutput() override = default;

    bool open() noexcept override {
        mFile.open(mPath.c_str(), std::ofstream::out | std::ofstream::binary);
        return !mFile.fail();
    }

    bool write(const uint8_t* data, size_t size) noexcept override {
        mFile.write((char*)(data), size);
        return mFile.fail();
    }

    std::ostream& getOutputStream() noexcept override {
        return mFile;
    }

    bool close() noexcept override {
        mFile.close();
        return mFile.fail();
    };
private:
    const std::string mPath;
    std::ofstream mFile;
};

class FilesystemInput : public matp::Config::Input {
public:
    explicit FilesystemInput(const char* path) : mPath(path) {
    }

    ~FilesystemInput() override = default;

    ssize_t open() noexcept override {
        mFile.open(mPath.c_str(), std::ifstream::binary | std::ios::ate);
        if (!mFile) {
            std::cerr << "Unable to open material source file '" << mPath << "'" << std::endl;
            return 0;
        }
        mFilesize = mFile.tellg();
        mFile.seekg(0, std::ios::beg);
        return mFilesize;
    }

    std::unique_ptr<const char[]> read() noexcept override {
        auto buffer = std::make_unique<char[]>(mFilesize);
        if (!mFile.read(buffer.get(), mFilesize)) {
            std::cerr << "Unable to read material source file '" << mPath << "'" << std::endl;
            return nullptr;
        }
        return std::unique_ptr<const char[]>(buffer.release());
    }

    bool close() noexcept override {
        mFile.close();
        return mFile.fail();
    }

    const char* getName() const noexcept override {
        return mPath.c_str();
    }

private:
    const std::string mPath;
    std::ifstream mFile;
    ssize_t mFilesize = 0;
};

class CommandlineConfig : public matp::Config {
public:

    CommandlineConfig(int argc, char** argv);
    ~CommandlineConfig() override {
        delete mInput;
        delete mOutput;
    };

    matp::Config::Output* getOutput()  const noexcept override  {
        return mOutput;
    }

    matp::Config::Input* getInput() const noexcept override {
        return mInput;
    }

    std::string toString() const noexcept override {
        std::string parameters;
        for (size_t i = 0 ; i < mArgc; i++) {
            parameters += mArgv[i];
            parameters += " ";
        }
        return parameters;
    }

    std::string toPIISafeString() const noexcept override;

private:
    bool parse();

    int mArgc = 0;
    char** mArgv = nullptr;

    FilesystemInput* mInput = nullptr;
    FilesystemOutput* mOutput = nullptr;
};

} // namespace matc

#endif //TNT_COMPILERPARAMETERS_H
