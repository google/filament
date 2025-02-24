// flags: --glsl-desktop

@group(0) @binding(0) var t_rgba8unorm : texture_storage_2d_array<rgba8unorm, write>;
@group(0) @binding(1) var t_rgba8snorm : texture_storage_2d_array<rgba8snorm, write>;
@group(0) @binding(2) var t_rgba8uint : texture_storage_2d_array<rgba8uint, write>;
@group(0) @binding(3) var t_rgba8sint : texture_storage_2d_array<rgba8sint, write>;
@group(0) @binding(4) var t_rgba16uint : texture_storage_2d_array<rgba16uint, write>;
@group(0) @binding(5) var t_rgba16sint : texture_storage_2d_array<rgba16sint, write>;
@group(0) @binding(6) var t_rgba16float : texture_storage_2d_array<rgba16float, write>;
@group(0) @binding(7) var t_r32uint : texture_storage_2d_array<r32uint, write>;
@group(0) @binding(8) var t_r32sint : texture_storage_2d_array<r32sint, write>;
@group(0) @binding(9) var t_r32float : texture_storage_2d_array<r32float, write>;
@group(0) @binding(10) var t_rg32uint : texture_storage_2d_array<rg32uint, write>;
@group(0) @binding(11) var t_rg32sint : texture_storage_2d_array<rg32sint, write>;
@group(0) @binding(12) var t_rg32float : texture_storage_2d_array<rg32float, write>;
@group(0) @binding(13) var t_rgba32uint : texture_storage_2d_array<rgba32uint, write>;
@group(0) @binding(14) var t_rgba32sint : texture_storage_2d_array<rgba32sint, write>;
@group(0) @binding(15) var t_rgba32float : texture_storage_2d_array<rgba32float, write>;

@compute @workgroup_size(1)
fn main() {
  var dim1  = textureDimensions(t_rgba8unorm);
  var dim2  = textureDimensions(t_rgba8snorm);
  var dim3  = textureDimensions(t_rgba8uint);
  var dim4  = textureDimensions(t_rgba8sint);
  var dim5  = textureDimensions(t_rgba16uint);
  var dim6  = textureDimensions(t_rgba16sint);
  var dim7  = textureDimensions(t_rgba16float);
  var dim8  = textureDimensions(t_r32uint);
  var dim9  = textureDimensions(t_r32sint);
  var dim10 = textureDimensions(t_r32float);
  var dim11 = textureDimensions(t_rg32uint);
  var dim12 = textureDimensions(t_rg32sint);
  var dim13 = textureDimensions(t_rg32float);
  var dim14 = textureDimensions(t_rgba32uint);
  var dim15 = textureDimensions(t_rgba32sint);
  var dim16 = textureDimensions(t_rgba32float);
}
