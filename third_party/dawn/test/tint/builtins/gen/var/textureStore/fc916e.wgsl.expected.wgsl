@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32uint, read_write>;

fn textureStore_fc916e() {
  var arg_1 = vec2<u32>(1u);
  var arg_2 = 1i;
  var arg_3 = vec4<u32>(1u);
  textureStore(arg_0, arg_1, arg_2, arg_3);
}

@fragment
fn fragment_main() {
  textureStore_fc916e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_fc916e();
}
