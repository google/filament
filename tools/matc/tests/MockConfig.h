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

#ifndef TNT_MOCKCONFIG_H
#define TNT_MOCKCONFIG_H


#include <matc/Config.h>

class NullOutput : public matc::Config::Output {

public:
    NullOutput() : mNullStream(&mNull){

    }
    bool open() noexcept override;
    bool write(const uint8_t* data, size_t size) noexcept override;
    std::ostream& getOutputStream() noexcept override;
    bool close() noexcept  override;

private:
    class NullBuffer : public std::streambuf {
    public:
        int overflow(int c) override { return c; }
    };
    NullBuffer mNull;
    std::ostream mNullStream;
};

class MockInput : public matc::Config::Input {
public:
    MockInput(uint8_t* data, size_t size);
    ~MockInput() override;
    ssize_t open() noexcept override;
    std::unique_ptr<const char[]> read() noexcept override;
    bool close() noexcept override;
    const char* getName() const noexcept override { return "mock"; };
private:
    uint8_t* mData;
    size_t mSize;
};

class MockConfig : public matc::Config {
public:
    Output* getOutput() const noexcept override;
    Input* getInput() const noexcept override;
    std::string toString() const noexcept override;
private:
};


#endif //TNT_MOCKCONFIG_H
