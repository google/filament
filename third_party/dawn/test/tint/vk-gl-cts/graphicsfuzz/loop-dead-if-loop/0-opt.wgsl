struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var k : i32;
  var GLF_dead0j : i32;
  var donor_replacementGLF_dead0stack : array<i32, 10u>;
  var donor_replacementGLF_dead0top : i32;
  var x_54 : i32;
  var matrix_b : vec4<f32>;
  var b : i32;
  k = 0;
  loop {
    let x_12 : i32 = k;
    if ((x_12 < 4)) {
    } else {
      break;
    }
    let x_62 : f32 = x_6.injectionSwitch.y;
    if ((0.0 > x_62)) {
      GLF_dead0j = 1;
      loop {
        let x_13 : i32 = donor_replacementGLF_dead0stack[0];
        if ((1 <= x_13)) {
        } else {
          break;
        }
      }
      let x_14 : i32 = donor_replacementGLF_dead0top;
      let x_15 : i32 = donor_replacementGLF_dead0top;
      if (((x_14 >= 0) & (x_15 < 9))) {
        let x_16 : i32 = donor_replacementGLF_dead0top;
        let x_17 : i32 = (x_16 + 1);
        donor_replacementGLF_dead0top = x_17;
        x_54 = x_17;
      } else {
        x_54 = 0;
      }
      let x_18 : i32 = x_54;
      donor_replacementGLF_dead0stack[x_18] = 1;
    }
    matrix_b = vec4<f32>(0.0, 0.0, 0.0, 0.0);
    b = 3;
    loop {
      let x_19 : i32 = b;
      if ((x_19 >= 0)) {
      } else {
        break;
      }
      let x_20 : i32 = b;
      let x_21 : i32 = b;
      let x_87 : f32 = matrix_b[x_21];
      matrix_b[x_20] = (x_87 - 1.0);

      continuing {
        let x_22 : i32 = b;
        b = (x_22 - 1);
      }
    }

    continuing {
      let x_24 : i32 = k;
      k = (x_24 + 1);
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
