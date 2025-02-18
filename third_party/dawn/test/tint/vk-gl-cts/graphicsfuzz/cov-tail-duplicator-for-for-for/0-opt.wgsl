struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

@group(0) @binding(1) var<uniform> x_11 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var color : vec4<f32>;
  var i : i32;
  var j : i32;
  var k : i32;
  color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  let x_37 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  i = x_37;
  loop {
    let x_42 : i32 = i;
    let x_44 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    if ((x_42 < x_44)) {
    } else {
      break;
    }
    let x_47 : i32 = i;
    switch(x_47) {
      case 2: {
        let x_83 : i32 = i;
        let x_85 : f32 = x_11.x_GLF_uniform_float_values[0].el;
        color[x_83] = x_85;
      }
      case 1: {
        let x_52 : i32 = x_7.x_GLF_uniform_int_values[0].el;
        j = x_52;
        loop {
          let x_57 : i32 = i;
          let x_58 : i32 = i;
          if ((x_57 > x_58)) {
          } else {
            break;
          }
          let x_62 : i32 = x_7.x_GLF_uniform_int_values[0].el;
          k = x_62;
          loop {
            let x_67 : i32 = k;
            let x_68 : i32 = i;
            if ((x_67 < x_68)) {
            } else {
              break;
            }
            let x_71 : i32 = k;
            let x_73 : f32 = x_11.x_GLF_uniform_float_values[0].el;
            color[x_71] = x_73;

            continuing {
              let x_75 : i32 = k;
              k = (x_75 + 1);
            }
          }

          continuing {
            let x_77 : i32 = j;
            j = (x_77 + 1);
          }
        }
        let x_79 : i32 = i;
        let x_81 : f32 = x_11.x_GLF_uniform_float_values[0].el;
        color[x_79] = x_81;
      }
      default: {
      }
    }

    continuing {
      let x_87 : i32 = i;
      i = (x_87 + 1);
    }
  }
  let x_89 : vec4<f32> = color;
  x_GLF_color = x_89;
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
