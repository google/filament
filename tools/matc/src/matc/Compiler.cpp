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

#include "Compiler.h"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace filamat;

namespace matc {

bool Compiler::writeBlob(const Package &pkg, const Config& config) const noexcept {
    Config::Output* output = config.getOutput();
    if (!output->open()) {
        std::cerr << "Unable to create blob file." << std::endl;
        return false;
    }

    output->write(pkg.getData(), pkg.getSize());
    output->close();

    return true;
}

bool Compiler::writeBlobAsHeader(const Package &pkg, const Config& config) const noexcept {
    uint8_t* data = pkg.getData();

    Config::Output* output = config.getOutput();
    if (!output->open()) {
        std::cerr << "Unable to create header file." << std::endl;
        return false;
    }

    std::ostream& file = output->getOutputStream();
    if (config.isDebug()) {
        file << "// This file was generated with the following command:" << std::endl;
        file << "// " << config.toString() << " ";
        file << std::endl;

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        file << "// Created: " << std::put_time(&tm, "%Y-%m-%d at %H:%M:%S") << std::endl;
    }

    size_t i = 0;
    for ( ; i < pkg.getSize(); i++) {
        if (i > 0 && i % 20 == 0) {
            file << std::endl;
        }
        file << "0x" << std::setfill('0') << std::setw(2) << std::hex << (int) data[i] << ", ";
    }

    if (i % 20 != 0) file << std::endl;
    file << std::endl;

    output->close();

    return true;
}

} // namespace matc
