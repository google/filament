// Copyright 2018 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "draco/io/texture_io.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "draco/core/draco_test_utils.h"
#include "draco/io/file_utils.h"

namespace {

// Tests loading of textures from a buffer.
TEST(TextureIoTest, TestLoadFromBuffer) {
  const std::string file_name = draco::GetTestFileFullPath("test.png");
  std::vector<uint8_t> image_data;
  ASSERT_TRUE(draco::ReadFileToBuffer(file_name, &image_data));

  DRACO_ASSIGN_OR_ASSERT(
      std::unique_ptr<draco::Texture> texture,
      draco::ReadTextureFromBuffer(image_data.data(), image_data.size(),
                                   "image/png"));
  ASSERT_NE(texture, nullptr);

  ASSERT_EQ(texture->source_image().mime_type(), "image/png");

  // Re-encode the texture again to ensure the content hasn't changed.
  std::vector<uint8_t> encoded_buffer;
  DRACO_ASSERT_OK(draco::WriteTextureToBuffer(*texture, &encoded_buffer));

  ASSERT_EQ(image_data.size(), encoded_buffer.size());
  for (int i = 0; i < encoded_buffer.size(); ++i) {
    ASSERT_EQ(image_data[i], encoded_buffer[i]);
  }
}

// Tests that we can set mime type correctly even when the source file had
// an incorrect extension.
TEST(TextureIoTest, TestWrongExtension) {
  const std::string file_name = draco::GetTestFileFullPath("this_is_png.jpg");

  DRACO_ASSIGN_OR_ASSERT(std::unique_ptr<draco::Texture> texture,
                         draco::ReadTextureFromFile(file_name));
  ASSERT_NE(texture, nullptr);

  // Ensure the mime type was set to png even though the file extension was jpg.
  ASSERT_EQ(texture->source_image().mime_type(), "image/png");
}

// Tests that we can load jpeg files that have some trailing bytes after the
// jpeg end marker.
TEST(TextureIoTest, TestTrailingJpegBytes) {
  const std::string file_name = draco::GetTestFileFullPath("trailing_zero.jpg");

  DRACO_ASSIGN_OR_ASSERT(std::unique_ptr<draco::Texture> texture,
                         draco::ReadTextureFromFile(file_name));
  ASSERT_NE(texture, nullptr);
}

}  // namespace

#endif  // DRACO_TRANSCODER_SUPPORTED
