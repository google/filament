/*
 * Copyright 2018 The Android Open Source Project
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

#include <image/LinearImage.h>

#include <utils/Path.h>

namespace image {

enum class ComparisonMode {
    SKIP,
    COMPARE,
    UPDATE,
};

// Saves an image to disk or does a load-and-compare, depending on comparison mode.
// This makes it easy for unit tests to have compare / update commands.
// The passed-in image is the "result image" and the expected image is the "golden image".
void updateOrCompare(LinearImage result, const utils::Path& golden, ComparisonMode, float epsilon);

}  // namespace image
