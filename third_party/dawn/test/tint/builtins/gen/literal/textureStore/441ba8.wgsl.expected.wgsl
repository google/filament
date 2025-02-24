@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba32uint, write>;

fn textureStore_441ba8() {
  textureStore(arg_0, vec3<i32>(1i), vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_441ba8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_441ba8();
}
