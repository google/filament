struct buf0 {
  zero : i32,
}

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_i1_(x : ptr<function, i32>) -> i32 {
  let x_45 : i32 = *(x);
  if ((x_45 == 10)) {
    discard;
  }
  let x_49 : i32 = *(x);
  return x_49;
}

fn main_1() {
  var a : i32;
  var b : i32;
  var param : i32;
  var x_37 : i32;
  var x_35_phi : i32;
  a = 0;
  let x_33 : i32 = x_9.zero;
  b = x_33;
  x_35_phi = x_33;
  loop {
    let x_35 : i32 = x_35_phi;
    param = x_35;
    x_37 = func_i1_(&(param));
    a = x_37;
    let x_36 : i32 = (x_35 + 1);
    b = x_36;
    x_35_phi = x_36;
    if ((x_36 < 4)) {
    } else {
      break;
    }
  }
  if ((x_37 == bitcast<i32>(3))) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
