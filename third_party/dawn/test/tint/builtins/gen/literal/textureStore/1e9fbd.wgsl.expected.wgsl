@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r32sint, read_write>;

fn textureStore_1e9fbd() {
  textureStore(arg_0, vec2<u32>(1u), 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_1e9fbd();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_1e9fbd();
}
