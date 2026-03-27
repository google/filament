// Copyright 2017 The Draco Authors.
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
#include "draco/mesh/mesh_splitter.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include "draco/attributes/geometry_attribute.h"
#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/core/draco_types.h"
#include "draco/core/vector_d.h"
#include "draco/io/mesh_io.h"
#include "draco/material/material.h"
#include "draco/mesh/mesh_misc_functions.h"
#include "draco/point_cloud/point_cloud_builder.h"

namespace {}  // namespace
#endif        // DRACO_TRANSCODER_SUPPORTED
