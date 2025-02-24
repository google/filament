@compute @workgroup_size(1)
fn f() {
  var a = 4;
  var b = vec3<i32>(0, 2, 0);
  let r : vec3<i32> = (a % b);
}
