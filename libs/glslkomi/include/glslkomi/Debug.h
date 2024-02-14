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

#ifndef TNT_GLSLKOMI_DEBUG_H
#define TNT_GLSLKOMI_DEBUG_H

#include <glslkomi/Komi.h>
#include <glslang/MachineIndependent/localintermediate.h>

namespace glslkomi {

const char* glslangOperatorToString(glslang::TOperator op);
std::string glslangNodeToString(const TIntermNode* node);
std::string glslangLocToString(const glslang::TSourceLoc& loc);
std::string glslangNodeToStringWithLoc(const TIntermNode* node);
const char* rValueOperatorToString(RValueOperator op);

} // namespace glslkomi

#endif  // TNT_GLSLKOMI_INCLUDE_GLSLKOMI_DEBUG_H
