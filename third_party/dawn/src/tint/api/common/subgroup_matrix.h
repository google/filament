// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_API_COMMON_SUBGROUP_MATRIX_H_
#define SRC_TINT_API_COMMON_SUBGROUP_MATRIX_H_

#include <unordered_set>

#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/reflection.h"

namespace tint {

enum class SubgroupMatrixType : uint8_t {
    kF16 = 0,
    kF32,
    kU8,
    kI8,
    kU32,
    kI32,
};
TINT_REFLECT_ENUM_RANGE(tint::SubgroupMatrixType, kF16, kI32);

struct SubgroupMatrixMultiply {
    uint32_t M;
    uint32_t N;
    uint32_t K;

    SubgroupMatrixType input_type;
    SubgroupMatrixType output_type;

    TINT_REFLECT(SubgroupMatrixMultiply, M, N, K, input_type, output_type);
    TINT_REFLECT_HASH_CODE(SubgroupMatrixMultiply);

    bool operator==(const SubgroupMatrixMultiply&) const = default;
};

enum class SubgroupMatrixDirection : uint8_t {
    kLeft,
    kRight,
    kResult,
};
TINT_REFLECT_ENUM_RANGE(tint::SubgroupMatrixDirection, kLeft, kResult);

struct SubgroupMatrixConfig {
    uint32_t M;
    uint32_t N;
    uint32_t K;

    SubgroupMatrixType type;
    SubgroupMatrixDirection direction;

    TINT_REFLECT(SubgroupMatrixConfig, M, N, K, type, direction);
    TINT_REFLECT_HASH_CODE(SubgroupMatrixConfig);

    bool operator==(const SubgroupMatrixConfig&) const = default;
};

}  // namespace tint

template <>
class std::hash<tint::SubgroupMatrixMultiply> {
  public:
    inline std::size_t operator()(const tint::SubgroupMatrixMultiply& sm) const {
        return tint::Hash(sm.M, sm.N, sm.K, sm.input_type, sm.output_type);
    }
};

template <>
class std::hash<tint::SubgroupMatrixConfig> {
  public:
    inline std::size_t operator()(const tint::SubgroupMatrixConfig sm) const {
        return tint::Hash(sm.M, sm.N, sm.K, sm.type, sm.direction);
    }
};

namespace tint {

struct SubgroupMatrixInfo {
    std::unordered_set<SubgroupMatrixMultiply> multiplies;
    std::unordered_set<SubgroupMatrixConfig> configs;

    TINT_REFLECT(SubgroupMatrixInfo, multiplies, configs);
    TINT_REFLECT_HASH_CODE(SubgroupMatrixInfo);

    bool operator==(const SubgroupMatrixInfo&) const = default;
};

}  // namespace tint

#endif  // SRC_TINT_API_COMMON_SUBGROUP_MATRIX_H_
