@group(1) @binding(0) var arg_0 : texture_storage_3d<r32uint, read_write>;

fn textureStore_5246b4() {
  textureStore(arg_0, vec3<u32>(1u), vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_5246b4();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_5246b4();
}
