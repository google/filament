struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 6u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_36 : i32;
  var x_74 : bool;
  var x_33_phi : vec4<f32>;
  var x_36_phi : i32;
  var x_38_phi : i32;
  var x_75_phi : bool;
  let x_29 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  let x_31 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  x_33_phi = vec4<f32>();
  x_36_phi = x_29;
  x_38_phi = x_31;
  loop {
    var x_53 : vec4<f32>;
    var x_39 : i32;
    var x_34_phi : vec4<f32>;
    var x_62_phi : i32;
    let x_33 : vec4<f32> = x_33_phi;
    x_36 = x_36_phi;
    let x_38 : i32 = x_38_phi;
    let x_41 : i32 = x_5.x_GLF_uniform_int_values[4].el;
    if ((x_38 < x_41)) {
    } else {
      break;
    }
    var x_53_phi : vec4<f32>;
    var x_56_phi : i32;
    switch(0u) {
      default: {
        let x_48 : i32 = x_5.x_GLF_uniform_int_values[3].el;
        if ((x_38 > x_48)) {
          x_34_phi = x_33;
          x_62_phi = 2;
          break;
        }
        x_53_phi = x_33;
        x_56_phi = x_29;
        loop {
          var x_54 : vec4<f32>;
          var x_57 : i32;
          x_53 = x_53_phi;
          let x_56 : i32 = x_56_phi;
          if ((x_56 < x_41)) {
          } else {
            break;
          }

          continuing {
            let x_61 : f32 = f32((x_38 + x_56));
            x_54 = vec4<f32>(x_61, x_61, x_61, x_61);
            x_57 = (x_56 + 1);
            x_53_phi = x_54;
            x_56_phi = x_57;
          }
        }
        x_GLF_color = x_53;
        x_34_phi = x_53;
        x_62_phi = x_31;
      }
    }
    let x_34 : vec4<f32> = x_34_phi;
    let x_62 : i32 = x_62_phi;

    continuing {
      x_39 = (x_38 + 1);
      x_33_phi = x_34;
      x_36_phi = bitcast<i32>((x_36 + bitcast<i32>(x_62)));
      x_38_phi = x_39;
    }
  }
  let x_63 : vec4<f32> = x_GLF_color;
  let x_65 : i32 = x_5.x_GLF_uniform_int_values[2].el;
  let x_66 : f32 = f32(x_65);
  let x_69 : bool = all((x_63 == vec4<f32>(x_66, x_66, x_66, x_66)));
  x_75_phi = x_69;
  if (x_69) {
    let x_73 : i32 = x_5.x_GLF_uniform_int_values[5].el;
    x_74 = (x_36 == bitcast<i32>(x_73));
    x_75_phi = x_74;
  }
  let x_75 : bool = x_75_phi;
  if (x_75) {
    let x_79 : f32 = f32(x_31);
    let x_80 : f32 = f32(x_29);
    x_GLF_color = vec4<f32>(x_79, x_80, x_80, x_79);
  } else {
    let x_82 : f32 = f32(x_29);
    x_GLF_color = vec4<f32>(x_82, x_82, x_82, x_82);
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
