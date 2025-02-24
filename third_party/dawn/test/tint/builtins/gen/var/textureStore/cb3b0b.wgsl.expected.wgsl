@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba8sint, write>;

fn textureStore_cb3b0b() {
  var arg_1 = vec3<u32>(1u);
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_cb3b0b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_cb3b0b();
}
