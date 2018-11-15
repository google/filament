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

#include "ChunkInterfaceBlock.h"

#include <utils/CString.h>

using namespace filament;
using namespace utils;

namespace filaflat {

bool ChunkUniformInterfaceBlock::unflatten(Unflattener& unflattener,
        filament::UniformInterfaceBlock* uib) {

    UniformInterfaceBlock::Builder builder = UniformInterfaceBlock::Builder();

    CString name;
    if (!unflattener.read(&name)) {
        return false;
    }

    builder.name(std::move(name));

    // Read number of fields.
    uint64_t numFields = 0 ;
    if (!unflattener.read(&numFields) ) {
        return false;
    }

    for (uint64_t i = 0; i < numFields; i++) {
        CString fieldName;
        uint64_t fieldSize;
        uint8_t fieldType;
        uint8_t fieldPrecision;

        if (!unflattener.read(&fieldName)) {
            return false;
        }

        if (!unflattener.read(&fieldSize)) {
            return false;
        }

        if (!unflattener.read(&fieldType)) {
            return false;
        }

        if (!unflattener.read(&fieldPrecision)) {
           return false;
        }

        builder.add(fieldName, fieldSize, UniformInterfaceBlock::Type(fieldType),
                    UniformInterfaceBlock::Precision(fieldPrecision));
    }

    *uib = builder.build();
    return true;
}

bool ChunkSamplerInterfaceBlock::unflatten(Unflattener& unflattener,
        filament::SamplerInterfaceBlock* sib) {

    SamplerInterfaceBlock::Builder builder = SamplerInterfaceBlock::Builder();

    CString name;
    if (!unflattener.read(&name)) {
        return false;
    }
    builder.name(name);

    // Read number of fields.
    uint64_t numFields = 0 ;
    if (!unflattener.read(&numFields) ) {
        return false;
    }

    for (uint64_t i = 0; i < numFields; i++) {
        CString fieldName;
        uint8_t fieldType;
        uint8_t fieldFormat;
        uint8_t fieldPrecision;
        bool fieldMultisample;

        if (!unflattener.read(&fieldName)) {
            return false;
        }

        if (!unflattener.read(&fieldType)) {
            return false;
        }

        if (!unflattener.read(&fieldFormat))
            return false;

        if (!unflattener.read(&fieldPrecision)) {
            return false;
        }

        if (!unflattener.read(&fieldMultisample)) {
            return false;
        }

        builder.add(fieldName, SamplerInterfaceBlock::Type(fieldType),
                SamplerInterfaceBlock::Format(fieldFormat),
                SamplerInterfaceBlock::Precision(fieldPrecision),
                fieldMultisample);
    }

    *sib = builder.build();
    return true;
}

bool ChunkSamplerBindingsBlock::unflatten(Unflattener& unflattener,
        filament::SamplerBindingMap* map) {
    assert(map);

    uint64_t numFields = 0 ;
    if (!unflattener.read(&numFields) ) {
        return false;
    }

    for (uint64_t i = 0; i < numFields; i++) {
        SamplerBindingInfo info;
        if (!unflattener.read(&info.blockIndex)) {
            return false;
        }

        if (!unflattener.read(&info.localOffset)) {
            return false;
        }

        if (!unflattener.read(&info.globalOffset)) {
            return false;
        }

        if (!unflattener.read(&info.groupIndex)) {
            return false;
        }

        map->addSampler(info);
    }

    return true;
}

} // namespace filaflat
