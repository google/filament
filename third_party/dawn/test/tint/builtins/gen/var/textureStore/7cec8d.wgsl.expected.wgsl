@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32sint, write>;

fn textureStore_7cec8d() {
  var arg_1 = vec2<i32>(1i);
  var arg_2 = 1i;
  var arg_3 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2, arg_3);
}

@fragment
fn fragment_main() {
  textureStore_7cec8d();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_7cec8d();
}
