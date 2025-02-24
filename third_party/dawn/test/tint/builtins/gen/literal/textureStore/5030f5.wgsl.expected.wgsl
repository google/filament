@group(1) @binding(0) var arg_0 : texture_storage_2d_array<bgra8unorm, read_write>;

fn textureStore_5030f5() {
  textureStore(arg_0, vec2<u32>(1u), 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_5030f5();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_5030f5();
}
