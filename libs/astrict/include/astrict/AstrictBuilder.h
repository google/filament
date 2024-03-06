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

#ifndef TNT_ASTRICT_ASTRICT_BUILDER_H
#define TNT_ASTRICT_ASTRICT_BUILDER_H

#include <set>
#include <unordered_map>

#include <astrict/CommonTypes.h>

namespace astrict {

using StringId = Newtype<int, struct StringIdTag>;
using ExpressionId = Newtype<int, struct ExpressionIdTag>;
using GlobalVariableId = Newtype<int, struct GlobalVariableIdTag>;
using StructId = Newtype<int, struct StructIdTag>;
using FunctionId = Newtype<int, struct FunctionIdTag>;

using LocalVariableId = Newtype<int, struct LocalVariableIdTag>;

template<typename Id, typename Value>
class IdStore {
public:
    // Inserts if non-existent.
    Id insert(Value value, bool* outWasExtant = nullptr) {
        auto it = mMap.find(value);
        if (it == mMap.end()) {
            Id id(mMap.size() + 1);
            mMap[value] = id;
            if (outWasExtant) {
                *outWasExtant = false;
            }
            return id;
        }
        if (outWasExtant) {
            *outWasExtant = true;
        }
        return it->second;
    }

    std::vector<Value> getFinal() {
        std::vector<Value> r(mMap.size());
        for (const auto& pair : mMap) {
            r[pair.second] = pair.first;
        }
        return r;
    }

private:
    std::unordered_map<Value, Id> mMap;
};

using ByteVector = std::vector<uint8_t>;

class AstrictBuilder {
public:
    std::vector<ByteVector> build();

private:
    IdStore<StringId, std::string> mStrings;
    IdStore<ExpressionId, ByteVector> mExpressions;
    IdStore<StructId, ByteVector> mStructs;
    IdStore<GlobalVariableId, ByteVector> mGlobalSymbols;
    IdStore<FunctionId, ByteVector> mFunctions;
    IdStore<FunctionId, ByteVector> mFunctionBodies;
    std::unordered_map<StructId, std::string> mStructNames;
    std::unordered_map<GlobalVariableId, std::string> mGlobalVariableNames;
};

}  // namespace astrict

#endif  // TNT_ASTRICT_ASTRICT_BUILDER_H
