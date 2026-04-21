// Copyright 2019 The Draco Authors.
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
#include "draco/texture/texture_utils.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <utility>

#include "draco/core/draco_test_utils.h"
#include "draco/io/texture_io.h"
#include "draco/texture/color_utils.h"

namespace {

TEST(TextureUtilsTest, TestGetTargetNameForTextureLoadedFromFile) {
  // Tests that correct target stem and format are returned by texture utils for
  // texture loaded from image file (stem and format from source file).
  std::unique_ptr<draco::Texture> texture =
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("fast.jpg"))
          .value();
  ASSERT_NE(texture, nullptr);
  ASSERT_EQ(draco::TextureUtils::GetTargetStem(*texture), "fast");
  ASSERT_EQ(draco::TextureUtils::GetTargetExtension(*texture), "jpg");
  ASSERT_EQ(draco::TextureUtils::GetTargetFormat(*texture),
            draco::ImageFormat::JPEG);
  ASSERT_EQ(draco::TextureUtils::GetOrGenerateTargetStem(*texture, 5, "_Color"),
            "fast");
}

TEST(TextureUtilsTest, TestGetTargetNameForNewTexture) {
  // Tests that correct target stem and format are returned by texture utils for
  // a newly created texture (empty stem and PNG image type by default).
  std::unique_ptr<draco::Texture> texture(new draco::Texture());
  ASSERT_NE(texture, nullptr);
  ASSERT_EQ(draco::TextureUtils::GetTargetStem(*texture), "");
  ASSERT_EQ(draco::TextureUtils::GetOrGenerateTargetStem(*texture, 5, "_Color"),
            "Texture5_Color");
  ASSERT_EQ(draco::TextureUtils::GetTargetExtension(*texture), "png");
  ASSERT_EQ(draco::TextureUtils::GetTargetFormat(*texture),
            draco::ImageFormat::PNG);
}

TEST(TextureUtilsTest, TestGetSourceFormat) {
  // Tests that the source format is determined correctly for new textures and
  // for textures loaded from file.
  std::unique_ptr<draco::Texture> new_texture(new draco::Texture());
  DRACO_ASSIGN_OR_ASSERT(
      std::unique_ptr<draco::Texture> png_texture,
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("test.png")));
  DRACO_ASSIGN_OR_ASSERT(
      std::unique_ptr<draco::Texture> jpg_texture,
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("fast.jpg")));

  // Check source formats.
  ASSERT_EQ(draco::TextureUtils::GetSourceFormat(*new_texture),
            draco::ImageFormat::PNG);
  ASSERT_EQ(draco::TextureUtils::GetSourceFormat(*png_texture),
            draco::ImageFormat::PNG);
  ASSERT_EQ(draco::TextureUtils::GetSourceFormat(*jpg_texture),
            draco::ImageFormat::JPEG);

  // Remove the mime-type from the jpeg texture and ensure the source format is
  // still detected properly based on the filename.
  jpg_texture->source_image().set_mime_type("");
  ASSERT_EQ(draco::TextureUtils::GetSourceFormat(*jpg_texture),
            draco::ImageFormat::JPEG);
}

TEST(TextureUtilsTest, TestGetFormat) {
  typedef draco::ImageFormat ImageFormat;
  ASSERT_EQ(draco::TextureUtils::GetFormat("png"), ImageFormat::PNG);
  ASSERT_EQ(draco::TextureUtils::GetFormat("jpg"), ImageFormat::JPEG);
  ASSERT_EQ(draco::TextureUtils::GetFormat("jpeg"), ImageFormat::JPEG);
  ASSERT_EQ(draco::TextureUtils::GetFormat("basis"), ImageFormat::BASIS);
  ASSERT_EQ(draco::TextureUtils::GetFormat("ktx2"), ImageFormat::BASIS);
  ASSERT_EQ(draco::TextureUtils::GetFormat("webp"), ImageFormat::WEBP);
  ASSERT_EQ(draco::TextureUtils::GetFormat(""), ImageFormat::NONE);
  ASSERT_EQ(draco::TextureUtils::GetFormat("bmp"), ImageFormat::NONE);
}

TEST(TextureUtilsTest, TestGetTargetMimeType) {
  draco::Texture texture;
  texture.source_image().set_mime_type("image/jpeg");
  ASSERT_EQ(draco::TextureUtils::GetTargetMimeType(texture), "image/jpeg");

  // Set compression settings to a different format.
  draco::ImageCompressionOptions options;
  options.target_image_format = draco::ImageFormat::BASIS;
  texture.SetCompressionOptions(options);
  ASSERT_EQ(draco::TextureUtils::GetTargetMimeType(texture), "image/ktx2");

  // Test custom mime type in source image.
  draco::Texture unknown_format;
  unknown_format.source_image().set_mime_type("image/custom");
  ASSERT_EQ(draco::TextureUtils::GetTargetMimeType(unknown_format),
            "image/custom");

  // Test custom mime type from file name.
  draco::Texture unknown_format_file_name;
  unknown_format_file_name.source_image().set_filename("test.extension");
  ASSERT_EQ(draco::TextureUtils::GetTargetMimeType(unknown_format_file_name),
            "image/extension");
}

TEST(TextureUtilsTest, TestGetMimeType) {
  typedef draco::ImageFormat ImageFormat;
  ASSERT_EQ(draco::TextureUtils::GetMimeType(ImageFormat::PNG), "image/png");
  ASSERT_EQ(draco::TextureUtils::GetMimeType(ImageFormat::JPEG), "image/jpeg");
  ASSERT_EQ(draco::TextureUtils::GetMimeType(ImageFormat::BASIS), "image/ktx2");
  ASSERT_EQ(draco::TextureUtils::GetMimeType(ImageFormat::WEBP), "image/webp");
  ASSERT_EQ(draco::TextureUtils::GetMimeType(ImageFormat::NONE), "");
}

TEST(TextureUtilsTest, TestGetExtension) {
  typedef draco::ImageFormat ImageFormat;
  ASSERT_EQ(draco::TextureUtils::GetExtension(ImageFormat::PNG), "png");
  ASSERT_EQ(draco::TextureUtils::GetExtension(ImageFormat::JPEG), "jpg");
  ASSERT_EQ(draco::TextureUtils::GetExtension(ImageFormat::BASIS), "ktx2");
  ASSERT_EQ(draco::TextureUtils::GetExtension(ImageFormat::WEBP), "webp");
  ASSERT_EQ(draco::TextureUtils::GetExtension(ImageFormat::NONE), "");
}

TEST(TextureUtilsTest, TestComputeRequiredNumChannels) {
  // Tests that the number of texture channels can be computed. Material library
  // under test is created programmatically.

  // Load textures.
  DRACO_ASSIGN_OR_ASSERT(
      auto texture0, draco::ReadTextureFromFile(
                         draco::GetTestFileFullPath("fully_transparent.png")));
  ASSERT_NE(texture0, nullptr);
  draco::Texture *texture0_ptr = texture0.get();
  DRACO_ASSIGN_OR_ASSERT(
      auto texture1,
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("squares.png")));
  ASSERT_NE(texture1, nullptr);
  const draco::Texture *texture1_ptr = texture1.get();
  DRACO_ASSIGN_OR_ASSERT(
      auto texture2, draco::ReadTextureFromFile(
                         draco::GetTestFileFullPath("fully_transparent.png")));
  ASSERT_NE(texture2, nullptr);
  const draco::Texture *texture2_ptr = texture2.get();

  // Compute number of channels for occlusion-only texture.
  draco::MaterialLibrary library;
  draco::Material *const material0 = library.MutableMaterial(0);
  material0->SetTextureMap(std::move(texture0),
                           draco::TextureMap::AMBIENT_OCCLUSION, 0);
  ASSERT_EQ(
      draco::TextureUtils::ComputeRequiredNumChannels(*texture0_ptr, library),
      1);

  // Compute number of channels for occlusion-only texture with MR present but
  // not using the same texture.
  draco::Material *const material1 = library.MutableMaterial(1);
  material1->SetTextureMap(std::move(texture1),
                           draco::TextureMap::METALLIC_ROUGHNESS, 0);
  ASSERT_EQ(
      draco::TextureUtils::ComputeRequiredNumChannels(*texture0_ptr, library),
      1);

  // Compute number of channels for metallic-roughness texture.
  ASSERT_EQ(
      draco::TextureUtils::ComputeRequiredNumChannels(*texture1_ptr, library),
      3);

  // Compute number of channels texture that is used for occlusin map in one
  // material and also shared with metallic-roughness map in another material.
  draco::Material *const material2 = library.MutableMaterial(2);
  DRACO_ASSERT_OK(material2->SetTextureMap(
      texture0_ptr, draco::TextureMap::METALLIC_ROUGHNESS, 0));
  ASSERT_EQ(
      draco::TextureUtils::ComputeRequiredNumChannels(*texture0_ptr, library),
      3);

  // Compute number of channels for non-opaque texture.
  material0->SetTextureMap(std::move(texture2), draco::TextureMap::COLOR, 0);
  ASSERT_EQ(
      draco::TextureUtils::ComputeRequiredNumChannels(*texture2_ptr, library),
      4);
}

}  // namespace

#endif  // DRACO_TRANSCODER_SUPPORTED
