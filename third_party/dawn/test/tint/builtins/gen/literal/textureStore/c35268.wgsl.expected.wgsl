@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r32uint, read_write>;

fn textureStore_c35268() {
  textureStore(arg_0, vec2<u32>(1u), 1i, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_c35268();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_c35268();
}
