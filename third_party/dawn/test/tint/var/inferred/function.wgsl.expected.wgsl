struct MyStruct {
  f1 : f32,
}

alias MyArray = array<f32, 10>;

fn ret_i32() -> i32 {
  return 1;
}

fn ret_u32() -> u32 {
  return 1u;
}

fn ret_f32() -> f32 {
  return 1.0;
}

fn ret_MyStruct() -> MyStruct {
  return MyStruct();
}

fn ret_MyArray() -> MyArray {
  return MyArray();
}

fn var_decls() {
  var v1 = 1;
  var v2 = 1u;
  var v3 = 1.0;
  var v4 = vec3<i32>(1, 1, 1);
  var v5 = vec3<u32>(1u, 1u, 1u);
  var v6 = vec3<f32>(1.0, 1.0, 1.0);
  var v7 = mat3x3<f32>(v6, v6, v6);
  var v8 = MyStruct(1.0);
  var v9 = MyArray();
  var v10 = ret_i32();
  var v11 = ret_u32();
  var v12 = ret_f32();
  var v13 = ret_MyStruct();
  var v14 = ret_MyStruct();
  var v15 = ret_MyArray();
}

@fragment
fn main() -> @location(0) vec4<f32> {
  return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}
