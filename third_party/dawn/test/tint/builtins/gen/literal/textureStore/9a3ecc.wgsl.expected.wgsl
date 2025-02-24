@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba32sint, write>;

fn textureStore_9a3ecc() {
  textureStore(arg_0, vec3<i32>(1i), vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_9a3ecc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_9a3ecc();
}
