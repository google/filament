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

size_t MaterialCache::MaterialKey::Hash::operator()(
        filament::MaterialCache::MaterialKey const& key) const noexcept {
    uint32_t crc;
    if (key.parser->getMaterialCrc32(&crc)) {
        return size_t(crc);
    }
    return size_t(key.parser->computeCrc32());
}

bool MaterialCache::MaterialKey::operator==(MaterialKey const& rhs) const noexcept {
    return parser == rhs.parser;
}

MaterialCache::~MaterialCache() {
    assert_invariant(mDefinitions.empty());
    assert_invariant(mPrograms.empty());
    assert_invariant(mSpecializationConstantsInternPool.empty());
}

MaterialDefinition* UTILS_NULLABLE MaterialCache::acquireMaterial(FEngine& engine,
        const void* UTILS_NONNULL data, size_t size) noexcept {
    std::unique_ptr<MaterialParser> parser = MaterialDefinition::createParser(engine.getBackend(),
            engine.getShaderLanguage(), data, size);
    assert_invariant(parser);

    // The `key` must be constructed using parser.get() before parser is moved into the lambda
    // function. This prevents a potential crash or undefined behavior, as accessing a moved-from
    // object is unsafe. The validity of the generated key is guaranteed because the
    // MaterialDefinition object (which owns the same parser object) created within the lambda is
    // subsequently used as the associated value in the map.
    const MaterialKey key{ parser.get() };

    return mDefinitions.acquire(key, [&engine, parser = std::move(parser)]() mutable {
        return MaterialDefinition::create(engine, std::move(parser));
    });
}

void MaterialCache::releaseMaterial(FEngine& engine,
        MaterialDefinition const& definition) noexcept {
    mDefinitions.release(MaterialKey{ &definition.getMaterialParser() },
            [&engine](MaterialDefinition& definition) {
                definition.terminate(engine);
            });
}

} // namespace filament
