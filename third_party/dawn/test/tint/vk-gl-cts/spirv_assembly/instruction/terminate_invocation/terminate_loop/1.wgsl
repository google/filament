var<private> x_2 : vec4<f32>;

var<private> x_3 : i32;

var<private> x_4 : i32;

fn main_1() {
  var x_33_phi : i32;
  let x_18 : vec4<f32> = x_2;
  let x_28 : i32 = x_3;
  x_33_phi = 0;
  if (((((i32(x_18.x) & 1) + (i32(x_18.y) & 1)) + x_28) == i32(x_18.z))) {
    loop {
      var x_34 : i32;
      let x_33 : i32 = x_33_phi;
      if ((bitcast<u32>(x_33) < bitcast<u32>(10))) {
      } else {
        break;
      }

      continuing {
        x_34 = (x_33 + 1);
        x_33_phi = x_34;
      }
    }
  }
  x_4 = 1;
  return;
}

struct main_out {
  @location(0) @interpolate(flat)
  x_4_1 : i32,
}

@fragment
fn main(@builtin(position) x_2_param : vec4<f32>, @location(0) @interpolate(flat) x_3_param : i32) -> main_out {
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  return main_out(x_4);
}
