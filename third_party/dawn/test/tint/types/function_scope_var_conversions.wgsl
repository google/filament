fn constant_with_non_constant() {
  var a : f32 = f32();
  var b : vec2<f32> = vec2<f32>(f32(i32(1)), a);
}

@compute @workgroup_size(1)
fn main() {
  var bool_var1 : bool = bool(123u);
  var bool_var2 : bool = bool(123);
  var bool_var3 : bool = bool(123.0);

  var i32_var1 : i32 = i32(123u);
  var i32_var2 : i32 = i32(123.0);
  var i32_var3 : i32 = i32(true);

  var u32_var1 : u32 = u32(123);
  var u32_var2 : u32 = u32(123.0);
  var u32_var3 : u32 = u32(true);

  var v3bool_var1 : vec3<bool> = vec3<bool>(vec3<u32>(123u));
  var v3bool_var11 : vec3<bool> = vec3<bool>(vec3<u32>(1234u));
  var v3bool_var2 : vec3<bool> = vec3<bool>(vec3<i32>(123));
  var v3bool_var3 : vec3<bool> = vec3<bool>(vec3<f32>(123.0));

  var v3i32_var1 : vec3<i32> = vec3<i32>(vec3<u32>(123u));
  var v3i32_var2 : vec3<i32> = vec3<i32>(vec3<f32>(123.0));
  var v3i32_var3 : vec3<i32> = vec3<i32>(vec3<bool>(true));

  var v3u32_var1 : vec3<u32> = vec3<u32>(vec3<i32>(123));
  var v3u32_var2 : vec3<u32> = vec3<u32>(vec3<f32>(123.0));
  var v3u32_var3 : vec3<u32> = vec3<u32>(vec3<bool>(true));

  var v3bool_var4 : vec3<bool> = vec3<bool>(vec2<bool>(vec2<f32>(123.0)), true);
  var v4bool_var5 : vec4<bool> = vec4<bool>(vec2<bool>(vec2<f32>(123.0, 0.0)), vec2<bool>(true, bool(f32(0.0))));

  constant_with_non_constant();
}
