/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_ASTRICT_GLSLTYPES_H
#define TNT_ASTRICT_GLSLTYPES_H

#include <astrict/CommonTypes.h>
#include <set>
#include <unordered_map>
#include <vector>

namespace astrict {

struct PackFromGlsl {
    int version;
    std::unordered_map<TypeId, Type> types;
    std::unordered_map<GlobalSymbolId, Symbol> globalSymbols;
    std::unordered_map<RValueId, RValue> rValues;
    std::unordered_map<FunctionId, std::string_view> functionNames;
    std::unordered_map<StatementBlockId, std::vector<Statement>> statementBlocks;
    std::unordered_map<FunctionId, FunctionDefinition> functionDefinitions;
    std::set<FunctionId> functionPrototypes;
    std::vector<FunctionId> functionDefinitionOrder;
};

};

#endif  // TNT_ASTRICT_GLSLTYPES_H
