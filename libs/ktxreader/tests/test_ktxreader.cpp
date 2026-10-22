/*
 * Copyright 2022 The Android Open Source Project
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

#include <ktxreader/Ktx1Reader.h>
#include <ktxreader/Ktx2Reader.h>

#include <filament/Engine.h>
#include <filament/Texture.h>

#include <gtest/gtest.h>
#include <utils/Path.h>

#include <fstream>
#include <vector>

using namespace filament;
using namespace std;

using utils::Path;

class KtxReaderTest : public testing::Test {
protected:
    void SetUp() override {
        engine = Engine::create(Engine::Backend::NOOP);
    }

    void TearDown() override {
        Engine::destroy(&engine);
    }

    Engine* engine = nullptr;
};

static vector<uint8_t> readFile(const Path& inputPath) {
    std::ifstream file(inputPath, ios::binary);
    return vector<uint8_t>((istreambuf_iterator<char>(file)), {});
}

TEST_F(KtxReaderTest, Ktx1) {
    const utils::Path parent = Path::getCurrentExecutable().getParent();
    const auto contents = readFile(parent + "lightroom_ibl.ktx");
    ASSERT_EQ(contents.size(), 129368);

    image::Ktx1Bundle bundle(contents.data(), contents.size());

    Texture* tex = ktxreader::Ktx1Reader::createTexture(engine, bundle, false,
            [](void* userdata) {  /* */ }, nullptr);

    ASSERT_NE(tex, nullptr);
    ASSERT_EQ(tex->getWidth(), 64);
    ASSERT_EQ(tex->getHeight(), 64);

    engine->destroy(tex);
}

TEST_F(KtxReaderTest, Ktx2) {
    const utils::Path parent = Path::getCurrentExecutable().getParent();
    const auto contents = readFile(parent + "color_grid_uastc_zstd.ktx2");
    ASSERT_EQ(contents.size(), 170512);

    ktxreader::Ktx2Reader reader(*engine);

    reader.requestFormat(Texture::InternalFormat::DXT5_SRGBA);
    reader.requestFormat(Texture::InternalFormat::DXT5_RGBA);

    // Uncompressed formats are lower priority, so they get added last.
    reader.requestFormat(Texture::InternalFormat::SRGB8_A8);
    reader.requestFormat(Texture::InternalFormat::RGBA8);

    Texture* tex = reader.load(contents.data(), contents.size(),
            ktxreader::Ktx2Reader::TransferFunction::sRGB);

    ASSERT_NE(tex, nullptr);
    ASSERT_EQ(tex->getFormat(), Texture::InternalFormat::DXT5_SRGBA);
    ASSERT_EQ(tex->getWidth(), 1024);

    engine->destroy(tex);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
