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

MaterialCache::~MaterialCache() {
    if (!mInner.empty()) {
        LOG(WARNING) << "MaterialCache was destroyed but wasn't empty";
    }
}

MaterialDefinition* UTILS_NULLABLE MaterialCache::acquire(FEngine& engine,
        const void* UTILS_NONNULL data, size_t size) noexcept {
    std::unique_ptr<MaterialParser> parser = MaterialDefinition::createParser(
            engine.getBackend(), engine.getShaderLanguage(), data, size);
    if (!parser) {
        return nullptr;
    }

    uint64_t uuid;
    if (UTILS_UNLIKELY(!parser->getCacheId(&uuid))) {
        return nullptr;
    }

    return mInner.acquire(uuid, [&engine, parser = std::move(parser)]() mutable {
        return MaterialDefinition::create(engine, std::move(parser));
    });
}

void MaterialCache::release(FEngine& engine, uint64_t uuid) noexcept {
    mInner.release(uuid, [&engine](MaterialDefinition& definition) {
        definition.terminate(engine);
    });
}

} // namespace filament
