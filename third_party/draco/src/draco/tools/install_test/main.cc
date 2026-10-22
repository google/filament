// Copyright 2022 The Draco Authors.
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

// This program is used to test the installed version of Draco. It does just
// enough to confirm that an application using Draco can compile and link
// against an installed version of Draco without errors. It does not perform
// any sort of library tests.

#include <cstdio>
#include <vector>

#include "draco/core/decoder_buffer.h"

#if defined DRACO_TRANSCODER_SUPPORTED
#include "draco/scene/scene.h"
#include "draco/scene/scene_utils.h"
#endif

int main(int /*argc*/, char** /*argv*/) {
  std::vector<char> empty_buffer;
  draco::DecoderBuffer buffer;
  buffer.Init(empty_buffer.data(), empty_buffer.size());

#if defined DRACO_TRANSCODER_SUPPORTED
  draco::Scene empty_scene;
  const int num_meshes = empty_scene.NumMeshes();
  (void)num_meshes;
#endif

  printf("Partial sanity test passed.\n");
  return 0;
}
