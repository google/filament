struct buf0 {
  matrix_a_uni : mat4x4<f32>,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x : i32;
  var matrix_u : vec4<f32>;
  var b : i32;
  var matrix_b : vec4<f32>;
  var x_42 : vec4<f32>;
  x = 4;
  loop {
    let x_10 : i32 = x;
    if ((x_10 >= 1)) {
    } else {
      break;
    }
    let x_11 : i32 = x;
    matrix_u[x_11] = 1.0;

    continuing {
      let x_12 : i32 = x;
      x = (x_12 - 1);
    }
  }
  b = 4;
  loop {
    let x_55 : f32 = x_8.matrix_a_uni[0].x;
    if ((x_55 < -1.0)) {
    } else {
      break;
    }
    let x_14 : i32 = b;
    let x_15 : i32 = b;
    if ((x_15 > 1)) {
      let x_62 : vec4<f32> = matrix_b;
      let x_63 : vec4<f32> = matrix_b;
      x_42 = min(x_62, x_63);
    } else {
      let x_65 : vec4<f32> = matrix_u;
      x_42 = x_65;
    }
    let x_67 : f32 = x_42.y;
    matrix_b[x_14] = x_67;

    continuing {
      let x_16 : i32 = b;
      b = (x_16 - 1);
    }
  }
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
