fn get_i32() -> i32 {
  return 1;
}

fn f() {
  var v2 : vec2<i32> = vec2<i32>(get_i32());
  var v3 : vec3<i32> = vec3<i32>(get_i32());
  var v4 : vec4<i32> = vec4<i32>(get_i32());
}
