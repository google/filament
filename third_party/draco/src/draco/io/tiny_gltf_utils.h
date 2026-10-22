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
#ifndef DRACO_IO_TINY_GLTF_UTILS_H_
#define DRACO_IO_TINY_GLTF_UTILS_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "Eigen/Geometry"
#include "draco/animation/animation.h"
#include "draco/core/status.h"
#include "draco/core/status_or.h"
#include "draco/material/material.h"

#define TINYGLTF_ENCLOSING_NAMESPACE draco
#include "tiny_gltf.h"

namespace draco {

class TinyGltfUtils {
 public:
  TinyGltfUtils() {}

  // Returns the number of components for the attribute type.
  static int GetNumComponentsForType(int type);

  // Returns the material transparency mode in |mode|.
  static Material::TransparencyMode TextToMaterialMode(const std::string &mode);

  // Returns the animation sampler interpolation in |interpolation|.
  static AnimationSampler::SamplerInterpolation TextToSamplerInterpolation(
      const std::string &interpolation);

  // Returns the animation channel transformation in |path|.
  static AnimationChannel::ChannelTransformation TextToChannelTransformation(
      const std::string &path);

  // Adds all of the animation data associated with a channel.
  // The channel references a sampler, whose data will be added to the
  // |animation|. The sampler references input and output accessors,
  // whose data will be added to the |animation|.
  static Status AddChannelToAnimation(
      const tinygltf::Model &model, const tinygltf::Animation &input_animation,
      const tinygltf::AnimationChannel &channel, int node_index,
      Animation *animation);

  // Adds all of the sampler data. The sampler references
  // input and output accessors, whose data will be added to the |animation|.
  static Status AddSamplerToAnimation(const tinygltf::Model &model,
                                      const tinygltf::AnimationSampler &sampler,
                                      Animation *animation);

  // Converts the gltf2 animation accessor and adds it to
  // |node_animation_data|.
  static Status AddAccessorToAnimationData(
      const tinygltf::Model &model, const tinygltf::Accessor &accessor,
      NodeAnimationData *node_animation_data);

  // Returns the data from |accessor| as a vector of |T|.
  template <typename T>
  static StatusOr<std::vector<T>> CopyDataAsFloat(
      const tinygltf::Model &model, const tinygltf::Accessor &accessor) {
    const int num_components = GetNumComponentsForType(accessor.type);
    if (num_components != T::dimension) {
      return Status(Status::DRACO_ERROR,
                    "Dimension does not equal num components.");
    }
    return CopyDataAsFloatImpl<T>(model, accessor);
  }

 private:
  template <typename T>
  static StatusOr<std::vector<T>> CopyDataAsFloatImpl(
      const tinygltf::Model &model, const tinygltf::Accessor &accessor) {
    if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
      return Status(Status::DRACO_ERROR,
                    "Non-float data is not supported by CopyDataAsFloat().");
    }
    if (accessor.bufferView < 0) {
      return Status(Status::DRACO_ERROR,
                    "Error CopyDataAsFloat() bufferView < 0.");
    }

    const tinygltf::BufferView &buffer_view =
        model.bufferViews[accessor.bufferView];
    if (buffer_view.buffer < 0) {
      return Status(Status::DRACO_ERROR, "Error CopyDataAsFloat() buffer < 0.");
    }

    const tinygltf::Buffer &buffer = model.buffers[buffer_view.buffer];

    const unsigned char *const data_start =
        buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset;
    const int byte_stride = accessor.ByteStride(buffer_view);
    const int component_size =
        tinygltf::GetComponentSizeInBytes(accessor.componentType);

    std::vector<T> output;
    output.resize(accessor.count);

    const int num_components = GetNumComponentsForType(accessor.type);
    const unsigned char *data = data_start;
    for (int i = 0; i < accessor.count; ++i) {
      T values;

      for (int c = 0; c < num_components; ++c) {
        float value = 0.0f;
        memcpy(&value, data + (c * component_size), component_size);
        SetDataImpl<T>(value, c, &values);
      }

      output[i] = values;
      data += byte_stride;
    }

    return output;
  }

  template <typename T>
  static void SetDataImpl(float value, int index, T *values) {
    (*values)[index] = value;
  }
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_IO_TINY_GLTF_UTILS_H_
