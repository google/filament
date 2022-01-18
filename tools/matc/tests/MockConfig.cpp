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

#include "MockConfig.h"

bool NullOutput::open() noexcept {
    return true;
}
bool NullOutput::write(const uint8_t* data, size_t size) noexcept {
    return true;
}
std::ostream& NullOutput::getOutputStream() noexcept {
    return mNullStream;
}
bool NullOutput::close() noexcept {
    return true;
}

MockInput::MockInput(uint8_t* data, size_t size) : mSize(size) {
    mData = new uint8_t[size];
    memcpy(mData, data, size);
}

ssize_t MockInput::open() noexcept {
    return mSize;
}

std::unique_ptr<const char[]> MockInput::read() noexcept {
    char* rawBuffer = new char[mSize];
    memcpy(rawBuffer, mData, mSize);
    return std::unique_ptr<const char[]>(rawBuffer);
}

bool MockInput::close() noexcept {
    return false;
}

MockInput::~MockInput() {
    delete mData;
}

matc::Config::Output* MockConfig::getOutput() const noexcept {
    return nullptr;
}

matc::Config::Input* MockConfig::getInput() const noexcept {
    return nullptr;
}

std::string MockConfig::toString() const noexcept {
    return {};
}
