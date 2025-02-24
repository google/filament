struct S {
  field0 : vec4<f32>,
}

struct S_1 {
  field0 : vec4<f32>,
}

var<private> x_3 : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : S;

fn main_1() {
  let x_20 : vec4<f32> = x_5.field0;
  var x_21_1 : S_1 = S_1(vec4<f32>());
  x_21_1.field0 = x_20;
  let x_21 : S_1 = x_21_1;
  x_3 = x_21.field0;
  return;
}

struct main_out {
  @location(0)
  x_3_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_3);
}
