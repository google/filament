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

#include <gtest/gtest.h>

#include <matc/MaterialCompiler.h>

#include "TestMaterialCompiler.h"

#include <utils/JobSystem.h>

static std::string_view jsonMaterialSourceSimple(R"(
material {
    name: test_compute,
    domain: compute,
    groupSize: [8, 8, 1],
    parameters: [
    ]
}
compute {
    void compute() {
    }
}
)");

TEST(TestComputeMaterial, JsonMaterialCompilerSimple) {
    matc::MaterialCompiler rawCompiler;
    TestMaterialCompiler compiler(rawCompiler);

    filamat::MaterialBuilder::init();
    filamat::MaterialBuilder builder;

    bool result = compiler.parseMaterial(jsonMaterialSourceSimple.data(), jsonMaterialSourceSimple.size(), builder);

    EXPECT_TRUE(result);

    utils::JobSystem js;
    js.adopt();

    auto package = builder.build(js);

    EXPECT_TRUE(package.isValid());

    js.emancipate();
    filamat::MaterialBuilder::shutdown();
}

