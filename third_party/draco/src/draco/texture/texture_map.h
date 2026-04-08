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
#ifndef DRACO_TEXTURE_TEXTURE_MAP_H_
#define DRACO_TEXTURE_TEXTURE_MAP_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <memory>

#include "draco/texture/texture.h"
#include "draco/texture/texture_transform.h"

namespace draco {

// Class representing mapping of one texture to a mesh. A texture map
// specifies the mesh attribute that contains texture coordinates used by the
// texture. The class also defines an intended use of the texture as a so called
// mapping type (COLOR, NORMAL_TANGENT_SPACE, etc..). Mapping types are roughly
// based on GLTF 2.0 material spec that describes a metallic-roughness PBR
// material model. More details can be found here:
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#materials
class TextureMap {
 public:
  enum Type {
    // Generic purpose texture (not GLTF compliant).
    GENERIC = 0,
    // Color data with optional alpha channel for transparency (GLTF compliant).
    COLOR = 1,
    // Dedicated texture for storing transparency (not GLTF compliant).
    OPACITY = 2,
    // Dedicated texture for storing metallic property (not GLTF compliant).
    METALLIC = 3,
    // Dedicated texture for storing roughness property (not GLTF compliant).
    ROUGHNESS = 4,
    // Combined texture for storing metallic and roughness properties.
    // B == metallic, G == roughness (GLTF compliant).
    METALLIC_ROUGHNESS = 5,
    // Normal map defined in the object space of the mesh (not GLTF compliant).
    NORMAL_OBJECT_SPACE = 6,
    // Normal map defined in the tangent space of the mesh (GLTF compliant).
    NORMAL_TANGENT_SPACE = 7,
    // Precomputed ambient occlusion on the surface (GLTF compliant).
    AMBIENT_OCCLUSION = 8,
    // Emissive color (GLTF compliant).
    EMISSIVE = 9,
    // Texture types of glTF material extension KHR_materials_sheen.
    SHEEN_COLOR = 10,
    SHEEN_ROUGHNESS = 11,
    // Texture types of glTF material extension KHR_materials_transmission.
    TRANSMISSION = 12,
    // Texture types of glTF material extension KHR_materials_clearcoat.
    CLEARCOAT = 13,
    CLEARCOAT_ROUGHNESS = 14,
    CLEARCOAT_NORMAL = 15,
    // Texture types of glTF material extension KHR_materials_volume.
    THICKNESS = 16,
    // Texture types of glTF material extension KHR_materials_specular.
    SPECULAR = 17,
    SPECULAR_COLOR = 18,
    // The number of texture types.
    TEXTURE_TYPES_COUNT
  };

  enum AxisWrappingMode {
    // Out of bounds access along a texture axis should be clamped to the
    // nearest edge (default).
    CLAMP_TO_EDGE = 0,
    // Texture is repeated along a texture axis in a mirrored pattern.
    MIRRORED_REPEAT,
    // Texture is repeated along a texture axis (tiled textures).
    REPEAT
  };

  struct WrappingMode {
    explicit WrappingMode(AxisWrappingMode mode) : WrappingMode(mode, mode) {}
    WrappingMode(AxisWrappingMode s, AxisWrappingMode t) : s(s), t(t) {}
    AxisWrappingMode s;
    AxisWrappingMode t;
  };

  // Filter types are roughly based on glTF 2.0 samplers spec.
  // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#samplers
  enum FilterType {
    UNSPECIFIED = 0,
    NEAREST,
    LINEAR,
    NEAREST_MIPMAP_NEAREST,
    LINEAR_MIPMAP_NEAREST,
    NEAREST_MIPMAP_LINEAR,
    LINEAR_MIPMAP_LINEAR
  };

  TextureMap();
  TextureMap(TextureMap &&) = default;

  // Copies texture map data from the |src| texture map to this texture map.
  void Copy(const TextureMap &src);

  // Sets the mapping information between the texture and the target mesh.
  // |tex_coord_index| is the local index of the texture coordinates that is
  // used to map the texture on the mesh.
  void SetProperties(Type type);
  void SetProperties(Type type, int tex_coord_index);
  void SetProperties(Type type, WrappingMode wrapping_mode,
                     int tex_coord_index);
  void SetProperties(Type type, WrappingMode wrapping_mode, int tex_coord_index,
                     FilterType min_filter, FilterType mag_filter);

  // Set texture and transfer its ownership to the TextureMap object.
  //
  // Note that this should not be used if this TextureMap is part of a
  // MaterialLibrary. For such cases, the TextureMap's texture should refer to
  // an entry in the MaterialLibrary's TextureLibrary.
  void SetTexture(std::unique_ptr<Texture> texture);

  // Set texture and without transferring the ownership. The caller needs to
  // ensure the texture is valid during the lifetime of the TextureMap object.
  void SetTexture(Texture *texture);

  void SetTransform(const TextureTransform &transform);
  const TextureTransform &texture_transform() const {
    return texture_transform_;
  }

  const Texture *texture() const { return texture_; }
  Texture *texture() { return texture_; }
  Type type() const { return type_; }
  WrappingMode wrapping_mode() const { return wrapping_mode_; }
  int tex_coord_index() const { return tex_coord_index_; }
  FilterType min_filter() const { return min_filter_; }
  FilterType mag_filter() const { return mag_filter_; }

  TextureMap &operator=(TextureMap &&) = default;

 private:
  Type type_;
  WrappingMode wrapping_mode_;

  // Local index of the texture coordinates that is used to map the texture on
  // the mesh. For example, |tex_coord_index_ == 0| would correspond to the
  // first TEX_COORD attribute of the mesh.
  int tex_coord_index_;

  FilterType min_filter_;
  FilterType mag_filter_;

  // Used when the texture object is owned by TextureMap, otherwise set to
  // nullptr.
  std::unique_ptr<Texture> owned_texture_;

  // Either raw pointer owned by |owned_texture_| or a pointer to a user
  // specified texture in case |owned_texture_| is nullptr.
  Texture *texture_;

  // Transformation values of the texture map.
  TextureTransform texture_transform_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_TEXTURE_TEXTURE_MAP_H_
