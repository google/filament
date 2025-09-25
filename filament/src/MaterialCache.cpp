/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "MaterialCache.h"
#include "MaterialParser.h"

#include <backend/DriverEnums.h>

#include <details/Engine.h>
#include <details/Material.h>

#include <utils/Logger.h>

namespace filament {

size_t MaterialCache::Key::Hash::operator()(
        filament::MaterialCache::Key const& key) const noexcept {
    uint32_t crc;
    if (key.parser->getMaterialCrc32(&crc)) {
        return size_t(crc);
    }
    return size_t(key.parser->computeCrc32());
}

bool MaterialCache::Key::operator==(Key const& rhs) const noexcept {
    return parser == rhs.parser;
}

MaterialCache::~MaterialCache() {
    if (!mDefinitions.empty()) {
        LOG(WARNING) << "MaterialCache was destroyed but definitions cache wasn't empty";
    }
    if (!mPrograms.empty()) {
        LOG(WARNING) << "MaterialCache was destroyed but program cache wasn't empty";
    }
    if (!mSpecializationConstantsInternPool.empty()) {
        LOG(WARNING) << "MaterialCache was destroyed but specialization constants intern pool "
                        "wasn't empty";
    }
}

MaterialDefinition* UTILS_NULLABLE MaterialCache::acquire(FEngine& engine,
        const void* UTILS_NONNULL data, size_t size) noexcept {
    std::unique_ptr<MaterialParser> parser = MaterialDefinition::createParser(engine.getBackend(),
            engine.getShaderLanguage(), data, size);
    assert_invariant(parser);

    return mDefinitions.acquire(Key{parser.get()}, [&engine, parser = std::move(parser)]() mutable {
        return MaterialDefinition::create(engine, std::move(parser));
    });
}

void MaterialCache::release(FEngine& engine, MaterialDefinition const& definition) noexcept {
    mDefinitions.release(Key{ &definition.getMaterialParser() },
            [&engine](MaterialDefinition& definition) {
                definition.terminate(engine);
            });
}

backend::Handle<backend::HwProgram> MaterialCache::acquireProgram(FEngine& engine,
        MaterialDefinition const& material, ProgramSpecialization const& specialization,
        backend::CompilerPriorityQueue const priorityQueue) {
    backend::Handle<backend::HwProgram>* program = mPrograms.acquire(specialization,
            [&engine, &material, &specialization, priorityQueue]() {
                return material.compileProgram(engine, specialization, priorityQueue);
            });
    assert_invariant(program);
    return *program;
}

backend::Handle<backend::HwProgram> MaterialCache::getProgram(
        ProgramSpecialization const& specialization) {
    return mPrograms.get(specialization);
}

void MaterialCache::releaseProgram(FEngine& engine, ProgramSpecialization const& specialization) {
    // TODO: properly release program
    return mPrograms.release(specialization);
}


} // namespace filament
