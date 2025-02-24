@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba8uint, write>;

fn textureStore_d2b565() {
  var arg_1 = vec3<u32>(1u);
  var arg_2 = vec4<u32>(1u);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_d2b565();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_d2b565();
}
