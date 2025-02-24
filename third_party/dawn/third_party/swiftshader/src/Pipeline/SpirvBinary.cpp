// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "SpirvBinary.hpp"

namespace sw {

std::atomic<uint32_t> SpirvBinary::serialCounter(1);  // Start at 1, 0 is invalid shader.

SpirvBinary::SpirvBinary()
    : identifier(serialCounter++)
{}

SpirvBinary::SpirvBinary(const uint32_t *binary, uint32_t wordCount)
    : std::vector<uint32_t>(binary, binary + wordCount)
    , identifier(serialCounter++)
{
}

void SpirvBinary::mapOptimizedIdentifier(const SpirvBinary &unoptimized)
{
	// The bitwise NOT accomplishes a 1-to-1 mapping of the identifiers,
	// while avoiding clashes with previous or future serial IDs.
	identifier = ~unoptimized.identifier;
}

}  // namespace sw