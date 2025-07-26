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

#ifndef TNT_TESTMATERIALPARSER_H
#define TNT_TESTMATERIALPARSER_H

#include "filamat/MaterialBuilder.h"
 #include <filament-matp/MaterialParser.h>

class TestMaterialParser {
public:
    explicit TestMaterialParser(const matp::MaterialParser& materialParser) :
            mMaterialParser(materialParser) {}

        bool parseMaterial(const char* buffer, size_t size, filamat::MaterialBuilder& builder)
        noexcept{
            return mMaterialParser.parseMaterial(buffer, size, builder);
        }

        bool parseMaterialAsJSON(const char* buffer, size_t size, filamat::MaterialBuilder& builder)
        noexcept{
            return mMaterialParser.parseMaterialAsJSON(buffer, size, builder);
        }

private:
    const matp::MaterialParser mMaterialParser;
};

#endif // TNT_TESTMATERIALPARSER_H
