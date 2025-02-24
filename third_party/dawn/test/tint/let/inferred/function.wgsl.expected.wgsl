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

fn let_decls() {
  let v1 = 1;
  let v2 = 1u;
  let v3 = 1.0;
  let v4 = vec3<i32>(1, 1, 1);
  let v5 = vec3<u32>(1u, 1u, 1u);
  let v6 = vec3<f32>(1.0, 1.0, 1.0);
  let v7 = mat3x3<f32>(v6, v6, v6);
  let v8 = MyStruct(1.0);
  let v9 = MyArray();
  let v10 = ret_i32();
  let v11 = ret_u32();
  let v12 = ret_f32();
  let v13 = ret_MyStruct();
  let v14 = ret_MyStruct();
  let v15 = ret_MyArray();
}

@fragment
fn main() -> @location(0) vec4<f32> {
  return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}
