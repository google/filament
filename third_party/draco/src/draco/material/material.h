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
#ifndef DRACO_MATERIAL_MATERIAL_H_
#define DRACO_MATERIAL_MATERIAL_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <memory>
#include <unordered_map>

#include "draco/core/status.h"
#include "draco/core/vector_d.h"
#include "draco/texture/texture_library.h"
#include "draco/texture/texture_map.h"

namespace draco {

// Material specification for Draco geometry. Parameters are based on the
// metallic-roughness PBR model adopted by GLTF 2.0 standard:
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#materials
class Material {
 public:
  enum TransparencyMode {
    TRANSPARENCY_OPAQUE = 0,
    TRANSPARENCY_MASK,
    TRANSPARENCY_BLEND
  };

  Material();
  explicit Material(TextureLibrary *texture_library);

  // Copies all material data from the |src| material to this material.
  void Copy(const Material &src);

  // Deletes all texture maps and resets all material properties to default
  // values.
  void Clear();

  // Deletes all texture maps from the material while keeping other material
  // properties unchanged.
  void ClearTextureMaps();

  const std::string &GetName() const { return name_; }
  void SetName(const std::string &name) { name_ = name; }
  Vector4f GetColorFactor() const { return color_factor_; }
  void SetColorFactor(const Vector4f &color_factor) {
    color_factor_ = color_factor;
  }
  float GetMetallicFactor() const { return metallic_factor_; }
  void SetMetallicFactor(float metallic_factor) {
    metallic_factor_ = metallic_factor;
  }
  float GetRoughnessFactor() const { return roughness_factor_; }
  void SetRoughnessFactor(float roughness_factor) {
    roughness_factor_ = roughness_factor;
  }
  Vector3f GetEmissiveFactor() const { return emissive_factor_; }
  void SetEmissiveFactor(const Vector3f &emissive_factor) {
    emissive_factor_ = emissive_factor;
  }
  bool GetDoubleSided() const { return double_sided_; }
  void SetDoubleSided(bool double_sided) { double_sided_ = double_sided; }
  TransparencyMode GetTransparencyMode() const { return transparency_mode_; }
  void SetTransparencyMode(TransparencyMode mode) { transparency_mode_ = mode; }
  float GetAlphaCutoff() const { return alpha_cutoff_; }
  void SetAlphaCutoff(float alpha_cutoff) { alpha_cutoff_ = alpha_cutoff; }
  float GetNormalTextureScale() const { return normal_texture_scale_; }
  void SetNormalTextureScale(float scale) { normal_texture_scale_ = scale; }

  // Properties of glTF material extension KHR_materials_unlit.
  bool GetUnlit() const { return unlit_; }
  void SetUnlit(bool unlit) { unlit_ = unlit; }

  // Properties of glTF material extension KHR_materials_sheen.
  bool HasSheen() const { return has_sheen_; }
  void SetHasSheen(bool value) { has_sheen_ = value; }
  Vector3f GetSheenColorFactor() const { return sheen_color_factor_; }
  void SetSheenColorFactor(const Vector3f &value) {
    sheen_color_factor_ = value;
  }
  float GetSheenRoughnessFactor() const { return sheen_roughness_factor_; }
  void SetSheenRoughnessFactor(float value) { sheen_roughness_factor_ = value; }

  // Properties of glTF material extension KHR_materials_transmission.
  bool HasTransmission() const { return has_transmission_; }
  void SetHasTransmission(bool value) { has_transmission_ = value; }
  float GetTransmissionFactor() const { return transmission_factor_; }
  void SetTransmissionFactor(float value) { transmission_factor_ = value; }

  // Properties of glTF material extension KHR_materials_clearcoat.
  bool HasClearcoat() const { return has_clearcoat_; }
  void SetHasClearcoat(bool value) { has_clearcoat_ = value; }
  float GetClearcoatFactor() const { return clearcoat_factor_; }
  void SetClearcoatFactor(float value) { clearcoat_factor_ = value; }
  float GetClearcoatRoughnessFactor() const {
    return clearcoat_roughness_factor_;
  }
  void SetClearcoatRoughnessFactor(float value) {
    clearcoat_roughness_factor_ = value;
  }

  // Properties of glTF material extension KHR_materials_volume.
  bool HasVolume() const { return has_volume_; }
  void SetHasVolume(bool value) { has_volume_ = value; }
  float GetThicknessFactor() const { return thickness_factor_; }
  void SetThicknessFactor(float value) { thickness_factor_ = value; }
  float GetAttenuationDistance() const { return attenuation_distance_; }
  void SetAttenuationDistance(float value) { attenuation_distance_ = value; }
  Vector3f GetAttenuationColor() const { return attenuation_color_; }
  void SetAttenuationColor(const Vector3f &value) {
    attenuation_color_ = value;
  }

  // Properties of glTF material extension KHR_materials_ior.
  bool HasIor() const { return has_ior_; }
  void SetHasIor(bool value) { has_ior_ = value; }
  float GetIor() const { return ior_; }
  void SetIor(float value) { ior_ = value; }

  // Properties of glTF material extension KHR_materials_specular.
  bool HasSpecular() const { return has_specular_; }
  void SetHasSpecular(bool value) { has_specular_ = value; }
  float GetSpecularFactor() const { return specular_factor_; }
  void SetSpecularFactor(float value) { specular_factor_ = value; }
  Vector3f GetSpecularColorFactor() const { return specular_color_factor_; }
  void SetSpecularColorFactor(const Vector3f &value) {
    specular_color_factor_ = value;
  }

  // Methods for working with texture maps.
  size_t NumTextureMaps() const { return texture_maps_.size(); }
  const TextureMap *GetTextureMapByIndex(int index) const {
    return texture_maps_[index].get();
  }
  TextureMap *GetTextureMapByIndex(int index) {
    return texture_maps_[index].get();
  }
  const TextureMap *GetTextureMapByType(TextureMap::Type texture_type) const {
    const auto it = texture_map_type_to_index_map_.find(texture_type);
    if (it == texture_map_type_to_index_map_.end()) {
      return nullptr;
    }
    return GetTextureMapByIndex(it->second);
  }
  TextureMap *GetTextureMapByType(TextureMap::Type texture_type) {
    const auto it = texture_map_type_to_index_map_.find(texture_type);
    if (it == texture_map_type_to_index_map_.end()) {
      return nullptr;
    }
    return GetTextureMapByIndex(it->second);
  }

  // TODO(b/146061359): Refactor the set texture map code.
  // Specifies a new texture map using a texture with a given type.
  // |tex_coord_index| defines which texture coordinate attribute should be used
  // to map the texture on the underlying geometry (e.g. tex_coord_index 0 would
  // use the first texture coordinate attribute).
  void SetTextureMap(std::unique_ptr<Texture> texture,
                     TextureMap::Type texture_map_type, int tex_coord_index);
  void SetTextureMap(std::unique_ptr<Texture> texture,
                     TextureMap::Type texture_map_type,
                     TextureMap::WrappingMode wrapping_mode,
                     int tex_coord_index);

  // Sets a new texture map using a |texture| that is already owned by this
  // material (that is by one of its texture maps or by the unerlying
  // |texture_library_|). |transform| is the texture map's transform if set.
  // |min_filter| and |mag_filter| are the texture filter types. Returns error
  // status if provided |texture| is not owned by the material.
  Status SetTextureMap(Texture *texture, TextureMap::Type texture_map_type,
                       int tex_coord_index);
  Status SetTextureMap(Texture *texture, TextureMap::Type texture_map_type,
                       TextureMap::WrappingMode wrapping_mode,
                       int tex_coord_index);
  Status SetTextureMap(Texture *texture, TextureMap::Type texture_map_type,
                       TextureMap::WrappingMode wrapping_mode,
                       TextureMap::FilterType min_filter,
                       TextureMap::FilterType mag_filter, int tex_coord_index);
  Status SetTextureMap(Texture *texture, TextureMap::Type texture_map_type,
                       TextureMap::WrappingMode wrapping_mode,
                       TextureMap::FilterType min_filter,
                       TextureMap::FilterType mag_filter,
                       const TextureTransform &transform, int tex_coord_index);

  // Removes a texture map from the material based on its index or texture type.
  // The material releases the ownership of the texture map and returns it as
  // a unique_ptr to allow the caller to use the texture map for other purposes.
  std::unique_ptr<TextureMap> RemoveTextureMapByIndex(int index);
  std::unique_ptr<TextureMap> RemoveTextureMapByType(
      TextureMap::Type texture_type);

 private:
  void SetTextureMap(TextureMap &&texture_map);
  void SetTextureMap(std::unique_ptr<TextureMap> texture_map);
  Status SetTextureMap(std::unique_ptr<TextureMap> texture_map,
                       Texture *texture, TextureMap::Type texture_map_type,
                       TextureMap::WrappingMode wrapping_mode,
                       TextureMap::FilterType min_filter,
                       TextureMap::FilterType mag_filter, int tex_coord_index);

  // Returns true if the |texture| is owned by the material.
  bool IsTextureOwned(const Texture &texture);

 private:
  std::string name_;
  Vector4f color_factor_;
  float metallic_factor_;
  float roughness_factor_;
  Vector3f emissive_factor_;
  bool double_sided_;
  TransparencyMode transparency_mode_;
  float alpha_cutoff_;
  float normal_texture_scale_;

  // Properties of glTF material extension KHR_materials_unlit.
  bool unlit_;

  // Properties of glTF material extension KHR_materials_sheen.
  bool has_sheen_;
  Vector3f sheen_color_factor_;
  float sheen_roughness_factor_;

  // Properties of glTF material extension KHR_materials_transmission.
  bool has_transmission_;
  float transmission_factor_;

  // Properties of glTF material extension KHR_materials_clearcoat.
  bool has_clearcoat_;
  float clearcoat_factor_;
  float clearcoat_roughness_factor_;

  // Properties of glTF material extension KHR_materials_volume.
  bool has_volume_;
  float thickness_factor_;
  float attenuation_distance_;
  Vector3f attenuation_color_;

  // Properties of glTF material extension KHR_materials_ior.
  bool has_ior_;
  float ior_;

  // Properties of glTF material extension KHR_materials_specular.
  bool has_specular_;
  float specular_factor_;
  Vector3f specular_color_factor_;

  // Texture maps.
  std::vector<std::unique_ptr<TextureMap>> texture_maps_;

  // Map between a texture type to texture index in |texture_maps_|. Allows fast
  // retrieval of texture maps based on their type.
  std::unordered_map<int, int> texture_map_type_to_index_map_;

  // Optional pointer to a library that holds ownership of textures used for
  // this material. If set to nullptr, the texture ownership will be assigned
  // to the newly created TextureMaps directly.
  TextureLibrary *texture_library_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_MATERIAL_MATERIAL_H_
