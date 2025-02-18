struct buf0 {
  zero : i32,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : buf0;

fn main_1() {
  var x_10 : array<i32, 1u>;
  var x_9 : array<i32, 1u>;
  var x_7 : i32;
  var x_11_phi : i32;
  let x_6 : i32 = x_5.zero;
  x_9[0] = x_6;
  let x_37 : array<i32, 1u> = x_9;
  x_10 = x_37;
  x_7 = x_9[0];
  switch(0u) {
    default: {
      x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
      let x_8 : i32 = x_10[0];
      if ((x_8 == bitcast<i32>(x_7))) {
        x_11_phi = 1;
        break;
      }
      x_11_phi = 2;
    }
  }
  let x_11 : i32 = x_11_phi;
  if ((x_11 == 1)) {
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
