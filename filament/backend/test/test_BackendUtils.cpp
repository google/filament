/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <private/backend/BackendUtils.h>

#include <backend/DriverEnums.h>
#include <backend/PixelBufferDescriptor.h>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

namespace filament::backend {

TEST(BackendUtilsTest, ReshapeRgbToRgba) {
    uint8_t const rgb[] = { 1, 2, 3, 4, 5, 6 };
    PixelBufferDescriptor const data(rgb, sizeof(rgb),
            PixelDataFormat::RGB, PixelDataType::UBYTE);
    PixelBufferDescriptor reshaped;
    ASSERT_TRUE(reshape(data, reshaped));
    ASSERT_EQ(reshaped.size, 8u);
    EXPECT_EQ(reshaped.format, PixelDataFormat::RGBA);
    uint8_t const expected[] = { 1, 2, 3, 0xFF, 4, 5, 6, 0xFF };
    uint8_t const* const out = static_cast<uint8_t const*>(reshaped.buffer);
    for (size_t i = 0; i < sizeof(expected); ++i) {
        EXPECT_EQ(out[i], expected[i]) << "byte " << i;
    }
}

} // namespace filament::backend
