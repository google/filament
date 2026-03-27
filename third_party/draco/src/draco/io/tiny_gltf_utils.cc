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
#include "draco/io/tiny_gltf_utils.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/animation/animation.h"
#include "draco/animation/node_animation_data.h"
#include "draco/core/status.h"
#include "draco/core/vector_d.h"
#include "tiny_gltf.h"

namespace draco {

int TinyGltfUtils::GetNumComponentsForType(int type) {
  switch (type) {
    case TINYGLTF_TYPE_SCALAR:
      return 1;
    case TINYGLTF_TYPE_VEC2:
      return 2;
    case TINYGLTF_TYPE_VEC3:
      return 3;
    case TINYGLTF_TYPE_VEC4:
    case TINYGLTF_TYPE_MAT2:
      return 4;
    case TINYGLTF_TYPE_MAT3:
      return 9;
    case TINYGLTF_TYPE_MAT4:
      return 16;
  }
  return 0;
}

Material::TransparencyMode TinyGltfUtils::TextToMaterialMode(
    const std::string &mode) {
  if (mode == "MASK") {
    return Material::TRANSPARENCY_MASK;
  } else if (mode == "BLEND") {
    return Material::TRANSPARENCY_BLEND;
  } else {
    return Material::TRANSPARENCY_OPAQUE;
  }
}

AnimationSampler::SamplerInterpolation
TinyGltfUtils::TextToSamplerInterpolation(const std::string &interpolation) {
  if (interpolation == "STEP") {
    return AnimationSampler::SamplerInterpolation::STEP;
  } else if (interpolation == "CUBICSPLINE") {
    return AnimationSampler::SamplerInterpolation::CUBICSPLINE;
  } else {
    return AnimationSampler::SamplerInterpolation::LINEAR;
  }
}

AnimationChannel::ChannelTransformation
TinyGltfUtils::TextToChannelTransformation(const std::string &path) {
  if (path == "rotation") {
    return AnimationChannel::ChannelTransformation::ROTATION;
  } else if (path == "scale") {
    return AnimationChannel::ChannelTransformation::SCALE;
  } else if (path == "weights") {
    return AnimationChannel::ChannelTransformation::WEIGHTS;
  } else {
    return AnimationChannel::ChannelTransformation::TRANSLATION;
  }
}

Status TinyGltfUtils::AddChannelToAnimation(
    const tinygltf::Model &model, const tinygltf::Animation &input_animation,
    const tinygltf::AnimationChannel &channel, int node_index,
    Animation *animation) {
  std::unique_ptr<AnimationChannel> new_channel(new AnimationChannel());

  const tinygltf::AnimationSampler &sampler =
      input_animation.samplers[channel.sampler];
  // Add the sampler associated with the channel.
  DRACO_RETURN_IF_ERROR(
      TinyGltfUtils::AddSamplerToAnimation(model, sampler, animation));
  new_channel->sampler_index = animation->NumSamplers() - 1;
  new_channel->target_index = node_index;
  new_channel->transformation_type =
      TinyGltfUtils::TextToChannelTransformation(channel.target_path);

  animation->AddChannel(std::move(new_channel));
  return OkStatus();
}

Status TinyGltfUtils::AddSamplerToAnimation(
    const tinygltf::Model &model, const tinygltf::AnimationSampler &sampler,
    Animation *animation) {
  std::unique_ptr<NodeAnimationData> node_animation_data(
      new NodeAnimationData());
  // TODO(fgalligan): Add support to not copy the accessor data if it is
  // referenced more than once. Currently we duplicate all animation data so
  // that it is referenced only once in the glTF file.
  const tinygltf::Accessor &input_accessor = model.accessors[sampler.input];
  DRACO_RETURN_IF_ERROR(AddAccessorToAnimationData(model, input_accessor,
                                                   node_animation_data.get()));
  animation->AddNodeAnimationData(std::move(node_animation_data));
  std::unique_ptr<AnimationSampler> new_sampler(new AnimationSampler());
  new_sampler->input_index = animation->NumNodeAnimationData() - 1;

  node_animation_data.reset(new NodeAnimationData());
  const tinygltf::Accessor &output_accessor = model.accessors[sampler.output];
  DRACO_RETURN_IF_ERROR(AddAccessorToAnimationData(model, output_accessor,
                                                   node_animation_data.get()));
  animation->AddNodeAnimationData(std::move(node_animation_data));
  new_sampler->output_index = animation->NumNodeAnimationData() - 1;

  new_sampler->interpolation_type =
      TinyGltfUtils::TextToSamplerInterpolation(sampler.interpolation);
  animation->AddSampler(std::move(new_sampler));
  return OkStatus();
}

// Specialization for returning the data from |accessor| as a vector of float.
template <>
StatusOr<std::vector<float>> TinyGltfUtils::CopyDataAsFloat(
    const tinygltf::Model &model, const tinygltf::Accessor &accessor) {
  const int num_components = GetNumComponentsForType(accessor.type);
  if (num_components != 1) {
    return Status(Status::DRACO_ERROR,
                  "Dimension does not equal num components.");
  }
  return CopyDataAsFloatImpl<float>(model, accessor);
}

// Specialization for returing the data from |accessor| as a vector of
// Matrix4x4.
template <>
StatusOr<std::vector<Eigen::Matrix4f>> TinyGltfUtils::CopyDataAsFloat(
    const tinygltf::Model &model, const tinygltf::Accessor &accessor) {
  const int num_components = GetNumComponentsForType(accessor.type);
  if (num_components != 16) {
    return Status(Status::DRACO_ERROR,
                  "Dimension does not equal num components.");
  }
  return CopyDataAsFloatImpl<Eigen::Matrix4f>(model, accessor);
}

Status TinyGltfUtils::AddAccessorToAnimationData(
    const tinygltf::Model &model, const tinygltf::Accessor &accessor,
    NodeAnimationData *node_animation_data) {
  if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
    return Status(Status::DRACO_ERROR,
                  "Unsupported ComponentType for NodeAnimationData.");
  }

  std::vector<float> *dest_data = node_animation_data->GetMutableData();
  if (accessor.type == TINYGLTF_TYPE_SCALAR) {
    DRACO_ASSIGN_OR_RETURN(std::vector<float> data,
                           CopyDataAsFloat<float>(model, accessor));

    for (int i = 0; i < data.size(); ++i) {
      dest_data->push_back(data[i]);
    }
    node_animation_data->SetType(NodeAnimationData::Type::SCALAR);
  } else if (accessor.type == TINYGLTF_TYPE_VEC3) {
    DRACO_ASSIGN_OR_RETURN(std::vector<Vector3f> data,
                           CopyDataAsFloat<Vector3f>(model, accessor));

    for (int i = 0; i < data.size(); ++i) {
      for (int j = 0; j < 3; ++j) {
        dest_data->push_back(data[i][j]);
      }
    }
    node_animation_data->SetType(NodeAnimationData::Type::VEC3);
  } else if (accessor.type == TINYGLTF_TYPE_VEC4) {
    DRACO_ASSIGN_OR_RETURN(std::vector<Vector4f> data,
                           CopyDataAsFloat<Vector4f>(model, accessor));

    for (int i = 0; i < data.size(); ++i) {
      for (int j = 0; j < 4; ++j) {
        dest_data->push_back(data[i][j]);
      }
    }
    node_animation_data->SetType(NodeAnimationData::Type::VEC4);
  } else if (accessor.type == TINYGLTF_TYPE_MAT4) {
    DRACO_ASSIGN_OR_RETURN(std::vector<Eigen::Matrix4f> data,
                           CopyDataAsFloat<Eigen::Matrix4f>(model, accessor));

    for (int i = 0; i < data.size(); ++i) {
      for (int j = 0; j < 16; ++j) {
        dest_data->push_back(data[i](j));
      }
    }
    node_animation_data->SetType(NodeAnimationData::Type::MAT4);
  } else {
    return Status(Status::DRACO_ERROR,
                  "Unsupported Type for GltfNodeAnimationData.");
  }
  node_animation_data->SetCount(accessor.count);
  node_animation_data->SetNormalized(accessor.normalized);
  return OkStatus();
}

template <>
void TinyGltfUtils::SetDataImpl(float value, int index, float *values) {
  *values = value;
}

template <>
void TinyGltfUtils::SetDataImpl(float value, int index,
                                Eigen::Matrix4f *values) {
  (*values)(index) = value;
}

}  // namespace draco

// Actual definitions needed by the tinygltf library using our configuration.
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_ENABLE_DRACO
#define TINYGLTF_IMPLEMENTATION

#include "tiny_gltf.h"

#endif  // DRACO_TRANSCODER_SUPPORTED
