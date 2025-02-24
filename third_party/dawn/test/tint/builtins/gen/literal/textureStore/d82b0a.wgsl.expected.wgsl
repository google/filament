@group(1) @binding(0) var arg_0 : texture_storage_3d<rg32sint, write>;

fn textureStore_d82b0a() {
  textureStore(arg_0, vec3<u32>(1u), vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_d82b0a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_d82b0a();
}
