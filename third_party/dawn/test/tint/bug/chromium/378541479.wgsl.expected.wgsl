@group(0) @binding(0) var<uniform> level : u32;

@group(0) @binding(1) var<uniform> coords : vec2<u32>;

@group(0) @binding(2) var tex : texture_depth_2d;

@compute @workgroup_size(1)
fn compute_main() {
  var res : f32 = textureLoad(tex, coords, level);
}
