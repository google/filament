var<private> bool_var1 : bool = bool(1u);
var<private> bool_var2 : bool = bool(1);
var<private> bool_var3 : bool = bool(1.0);

var<private> i32_var1 : i32 = i32(1u);
var<private> i32_var2 : i32 = i32(1.0);
var<private> i32_var3 : i32 = i32(true);

var<private> u32_var1 : u32 = u32(1);
var<private> u32_var2 : u32 = u32(1.0);
var<private> u32_var3 : u32 = u32(true);

var<private> v3bool_var1 : vec3<bool> = vec3<bool>(vec3<u32>(1u));
var<private> v3bool_var2 : vec3<bool> = vec3<bool>(vec3<i32>(1));
var<private> v3bool_var3 : vec3<bool> = vec3<bool>(vec3<f32>(1.0));

var<private> v3i32_var1 : vec3<i32> = vec3<i32>(vec3<u32>(1u));
var<private> v3i32_var2 : vec3<i32> = vec3<i32>(vec3<f32>(1.0));
var<private> v3i32_var3 : vec3<i32> = vec3<i32>(vec3<bool>(true));

var<private> v3u32_var1 : vec3<u32> = vec3<u32>(vec3<i32>(1));
var<private> v3u32_var2 : vec3<u32> = vec3<u32>(vec3<f32>(1.0));
var<private> v3u32_var3 : vec3<u32> = vec3<u32>(vec3<bool>(true));

var<private> v3bool_var4 : vec3<bool> = vec3<bool>(vec2<bool>(vec2<f32>(123.0)), true);
var<private> v4bool_var5 : vec4<bool> = vec4<bool>(vec2<bool>(vec2<f32>(123.0, 0.0)), vec2<bool>(true, bool(f32(0.0))));

@compute @workgroup_size(1)
fn main() {
  // Reference the module-scope variables to stop them from being removed.
  bool_var1 = bool();
  bool_var2 = bool();
  bool_var3 = bool();
  i32_var1 = i32();
  i32_var2 = i32();
  i32_var3 = i32();
  u32_var1 = u32();
  u32_var2 = u32();
  u32_var3 = u32();
  v3bool_var1 = vec3<bool>();
  v3bool_var2 = vec3<bool>();
  v3bool_var3 = vec3<bool>();
  v3bool_var4 = vec3<bool>();
  v4bool_var5 = vec4<bool>();
  v3i32_var1 = vec3<i32>();
  v3i32_var2 = vec3<i32>();
  v3i32_var3 = vec3<i32>();
  v3u32_var1 = vec3<u32>();
  v3u32_var2 = vec3<u32>();
  v3u32_var3 = vec3<u32>();
}
