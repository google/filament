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
#ifndef DRACO_IO_SCENE_IO_H_
#define DRACO_IO_SCENE_IO_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <string>
#include <vector>

#include "draco/core/options.h"
#include "draco/core/status_or.h"
#include "draco/scene/scene.h"

namespace draco {

// Reads a scene from a file. Currently only GLTF 2.0 scene files are supported.
// The second form returns the files associated with the scene via the
// |scene_files| argument.
StatusOr<std::unique_ptr<Scene>> ReadSceneFromFile(
    const std::string &file_name);
StatusOr<std::unique_ptr<Scene>> ReadSceneFromFile(
    const std::string &file_name, std::vector<std::string> *scene_files);

// Writes a scene into a file.
Status WriteSceneToFile(const std::string &file_name, const Scene &scene);

// Writes a scene into a file, configurable with |options|.
//
// Supported options:
//
//   force_usd_vertex_interpolation=<bool> - forces implicit vertex
//                             interpolation while exporting to USD
//                             (default = false)
//
Status WriteSceneToFile(const std::string &file_name, const Scene &scene,
                        const Options &options);

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_IO_SCENE_IO_H_
