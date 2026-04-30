// Copyright 2021 The Draco Authors.
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
#ifdef DRACO_TRANSCODER_SUPPORTED

#include "draco/tools/draco_transcoder_lib.h"

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/io/file_utils.h"

// Tests encoding a .gltf file with default Draco compression.
TEST(DracoTranscoderTest, DefaultDracoCompression) {
  const std::string input_name = "sphere.gltf";
  const std::string input_filename = draco::GetTestFileFullPath(input_name);
  const std::string output_filename =
      draco::GetTestTempFileFullPath("test.gltf");

  const draco::DracoTranscodingOptions options;
  DRACO_ASSIGN_OR_ASSERT(std::unique_ptr<draco::DracoTranscoder> dt,
                         draco::DracoTranscoder::Create(options));

  draco::DracoTranscoder::FileOptions file_options;
  file_options.input_filename = input_filename;
  file_options.output_filename = output_filename;
  DRACO_ASSERT_OK(dt->Transcode(file_options));

  const std::string output_bin_filename =
      draco::GetTestTempFileFullPath("test.bin");
  const size_t output_bin_size = draco::GetFileSize(output_bin_filename);
  ASSERT_GT(output_bin_size, 0);
}

// Tests setting the output glTF .bin name.
TEST(DracoTranscoderTest, TestBinName) {
  const std::string input_name = "sphere.gltf";
  const std::string input_filename = draco::GetTestFileFullPath(input_name);
  const std::string output_filename =
      draco::GetTestTempFileFullPath("test.gltf");
  const std::string output_bin_filename =
      draco::GetTestTempFileFullPath("different_name.bin");

  const draco::DracoTranscodingOptions options;
  DRACO_ASSIGN_OR_ASSERT(std::unique_ptr<draco::DracoTranscoder> dt,
                         draco::DracoTranscoder::Create(options));

  draco::DracoTranscoder::FileOptions file_options;
  file_options.input_filename = input_filename;
  file_options.output_filename = output_filename;
  file_options.output_bin_filename = output_bin_filename;
  DRACO_ASSERT_OK(dt->Transcode(file_options));

  const size_t output_bin_size = draco::GetFileSize(output_bin_filename);
  ASSERT_GT(output_bin_size, 0);
}

// Tests setting the output glTF resource directory.
TEST(DracoTranscoderTest, TestResourceDirName) {
  const std::string input_name = "sphere.gltf";
  const std::string input_filename = draco::GetTestFileFullPath(input_name);
  const std::string output_filename =
      draco::GetTestTempFileFullPath("test.gltf");
  const std::string output_bin_filename =
      draco::GetTestTempFileFullPath("another_name.bin");
  const std::string output_resource_directory =
      draco::GetTestTempFileFullPath("res/other_files");

  const draco::DracoTranscodingOptions options;
  DRACO_ASSIGN_OR_ASSERT(std::unique_ptr<draco::DracoTranscoder> dt,
                         draco::DracoTranscoder::Create(options));

  draco::DracoTranscoder::FileOptions file_options;
  file_options.input_filename = input_filename;
  file_options.output_filename = output_filename;
  file_options.output_bin_filename = output_bin_filename;
  file_options.output_resource_directory = output_resource_directory;
  DRACO_ASSERT_OK(dt->Transcode(file_options));

  const size_t output_bin_size = draco::GetFileSize(output_bin_filename);
  ASSERT_GT(output_bin_size, 0);

  const std::string res_dir_png_filename = draco::GetTestTempFileFullPath(
      "res/other_files/sphere_Texture0_Normal.png");
  const size_t output_png_size = draco::GetFileSize(res_dir_png_filename);
  ASSERT_GT(output_png_size, 0);
}

// Tests creating one transcoder to encode multiple files.
TEST(DracoTranscoderTest, EncodeMultipleFiles) {
  const draco::DracoTranscodingOptions options;
  DRACO_ASSIGN_OR_ASSERT(std::unique_ptr<draco::DracoTranscoder> dt,
                         draco::DracoTranscoder::Create(options));

  draco::DracoTranscoder::FileOptions file_options;
  file_options.input_filename = draco::GetTestFileFullPath("sphere.gltf");
  file_options.output_filename = draco::GetTestTempFileFullPath("first.gltf");
  DRACO_ASSERT_OK(dt->Transcode(file_options));
  const size_t first_bin_size =
      draco::GetFileSize(draco::GetTestTempFileFullPath("first.bin"));
  ASSERT_GT(first_bin_size, 0);

  file_options.input_filename =
      draco::GetTestFileFullPath("CesiumMan/glTF/CesiumMan.gltf");
  file_options.output_filename = draco::GetTestTempFileFullPath("second.gltf");
  DRACO_ASSERT_OK(dt->Transcode(file_options));
  const size_t second_bin_size =
      draco::GetFileSize(draco::GetTestTempFileFullPath("second.bin"));
  ASSERT_GT(second_bin_size, 0);
}

// Tests using glTF binary as input.
TEST(DracoTranscoderTest, SimpleGlbInput) {
  const std::string input_name = "Box/glTF_Binary/Box.glb";
  const std::string input_filename = draco::GetTestFileFullPath(input_name);
  const std::string output_filename =
      draco::GetTestTempFileFullPath("test.gltf");

  const draco::DracoTranscodingOptions options;
  DRACO_ASSIGN_OR_ASSERT(std::unique_ptr<draco::DracoTranscoder> dt,
                         draco::DracoTranscoder::Create(options));

  draco::DracoTranscoder::FileOptions file_options;
  file_options.input_filename = input_filename;
  file_options.output_filename = output_filename;
  DRACO_ASSERT_OK(dt->Transcode(file_options));

  const std::string output_bin_filename =
      draco::GetTestTempFileFullPath("test.bin");
  const size_t output_bin_size = draco::GetFileSize(output_bin_filename);
  ASSERT_GT(output_bin_size, 0);
}

// Simple test to check glb input and setting smaller position quantizations
// outputs a smaller file overall.
TEST(DracoTranscoderTest, TestPositionQuantization) {
  const std::string input_name =
      "KhronosSampleModels/Duck/glTF_Binary/Duck.glb";
  const std::string input_filename = draco::GetTestFileFullPath(input_name);

  draco::DracoTranscodingOptions options;
  DRACO_ASSIGN_OR_ASSERT(std::unique_ptr<draco::DracoTranscoder> dt,
                         draco::DracoTranscoder::Create(options));

  draco::DracoTranscoder::FileOptions file_options;
  file_options.input_filename = input_filename;
  file_options.output_filename = draco::GetTestTempFileFullPath("first.glb");
  DRACO_ASSERT_OK(dt->Transcode(file_options));
  const size_t first_glb_size =
      draco::GetFileSize(draco::GetTestTempFileFullPath("first.glb"));

  options.geometry.quantization_position.SetQuantizationBits(10);
  DRACO_ASSIGN_OR_ASSERT(std::unique_ptr<draco::DracoTranscoder> dt2,
                         draco::DracoTranscoder::Create(options));
  file_options.output_filename = draco::GetTestTempFileFullPath("second.glb");
  DRACO_ASSERT_OK(dt2->Transcode(file_options));
  const size_t second_glb_size =
      draco::GetFileSize(draco::GetTestTempFileFullPath("second.glb"));
  ASSERT_GT(first_glb_size, second_glb_size);
}

#endif  // DRACO_TRANSCODER_SUPPORTED
