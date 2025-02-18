enable f16;

@compute @workgroup_size(1)
fn f() {
  var a = 1.0h;
  var b = 0.0h;
  let r : f16 = (a % (b + b));
}
