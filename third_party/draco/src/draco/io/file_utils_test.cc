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
#include "draco/io/file_utils.h"

#include <string>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace {

TEST(FileUtilsTest, SplitsPath) {
  // Tests that the function SplitPath correctly splits a set of test paths.
  std::string folder_path, file_name;
  draco::SplitPath("file.x", &folder_path, &file_name);
  ASSERT_EQ(folder_path, ".");
  ASSERT_EQ(file_name, "file.x");

  draco::SplitPath("a/b/file.y", &folder_path, &file_name);
  ASSERT_EQ(folder_path, "a/b");
  ASSERT_EQ(file_name, "file.y");

  draco::SplitPath("//a/b/c/d/file.z", &folder_path, &file_name);
  ASSERT_EQ(folder_path, "//a/b/c/d");
  ASSERT_EQ(file_name, "file.z");
}

TEST(FileUtilsTest, ReplaceExtension) {
  // Tests that the function ReplaceFileExtension correctly replaces extensions
  // of specified files.
  ASSERT_EQ(draco::ReplaceFileExtension("a.abc", "x"), "a.x");
  ASSERT_EQ(draco::ReplaceFileExtension("abc", "x"), "abc.x");  // No extension
  ASSERT_EQ(draco::ReplaceFileExtension("a/b/c.d", "xyz"), "a/b/c.xyz");
}

TEST(FileUtilsTest, LowercaseFileExtension) {
  ASSERT_EQ(draco::LowercaseFileExtension("image.jpeg"), "jpeg");
  ASSERT_EQ(draco::LowercaseFileExtension("image.JPEG"), "jpeg");
  ASSERT_EQ(draco::LowercaseFileExtension("image.png"), "png");
  ASSERT_EQ(draco::LowercaseFileExtension("image.pNg"), "png");
  ASSERT_EQ(draco::LowercaseFileExtension("FILE.glb"), "glb");
  ASSERT_EQ(draco::LowercaseFileExtension(".file.gltf"), "gltf");
  ASSERT_EQ(draco::LowercaseFileExtension("the.file.gltf"), "gltf");
  ASSERT_EQ(draco::LowercaseFileExtension("FILE_glb"), "");
  ASSERT_EQ(draco::LowercaseFileExtension(""), "");
  ASSERT_EQ(draco::LowercaseFileExtension("image."), "");
}

TEST(FileUtilsTest, GetFullPath) {
  // Tests that full path is returned when a sibling file has full path.
  ASSERT_EQ(draco::GetFullPath("xo.png", "/d/i/r/xo.gltf"), "/d/i/r/xo.png");
  ASSERT_EQ(draco::GetFullPath("buf/01.bin", "dir/xo.gltf"), "dir/buf/01.bin");
  ASSERT_EQ(draco::GetFullPath("xo.mtl", "/xo.obj"), "/xo.mtl");

  // Tests that only file name is returned when a sibling file has no full path.
  ASSERT_EQ(draco::GetFullPath("xo.mtl", "xo.obj"), "xo.mtl");
}

}  // namespace
