@group(0) @binding(0)
var<storage, read_write> out : f32;

fn bar(p : ptr<function, f32>) {
  let a : f32 = 1.0;
  let b : f32 = 2.0;
  let cond = (a >= 0) && (b >= 0);
  *p = select(a, b, cond);
}

@compute @workgroup_size(1)
fn foo() {
  var param : f32;
  bar(&param);
  out = param;
}
