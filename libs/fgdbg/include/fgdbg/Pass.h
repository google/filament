/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef FGDBG_PASS_H
#define FGDBG_PASS_H

#include <string>
#include <vector>

namespace filament::fgdbg {
using ResourceId = int;

struct Pass {
    std::string name;
    int id;
    std::vector<ResourceId> reads;
    std::vector<ResourceId> writes;
};
}

#endif //FGDBG_PASS_H
