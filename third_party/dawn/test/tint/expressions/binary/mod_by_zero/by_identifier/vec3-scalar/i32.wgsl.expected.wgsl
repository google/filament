@compute @workgroup_size(1)
fn f() {
  var a = vec3<i32>(1, 2, 3);
  var b = 0;
  let r : vec3<i32> = (a % b);
}
