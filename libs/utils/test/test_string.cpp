/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <gtest/gtest.h>

#include <utils/string.h>

using namespace utils;

TEST(strtof_c, EmptyString) {
    char* end = nullptr;
    const char* start = "";
    float r = strtof_c(start, &end);
    EXPECT_EQ(start, end);
    EXPECT_EQ(0.0f, r);
}

TEST(strtof_c, ValidString) {
    char* end = nullptr;
    const char* start = "42.24, independent of the locale";
    float r = strtof_c(start, &end);
    EXPECT_TRUE(end > start);
    EXPECT_FLOAT_EQ(42.24f, r);
}
