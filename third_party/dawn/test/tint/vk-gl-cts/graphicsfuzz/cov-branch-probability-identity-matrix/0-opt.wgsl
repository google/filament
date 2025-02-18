struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 4u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var sums : array<f32, 2u>;
  var a : i32;
  var b : i32;
  var c : i32;
  var d : i32;
  var indexable : mat2x2<f32>;
  var indexable_1 : mat2x2<f32>;
  var x_158 : bool;
  var x_159_phi : bool;
  let x_16 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_85 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  sums[x_16] = -(x_85);
  let x_18 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_90 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  sums[x_18] = -(x_90);
  let x_19 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_19;
  loop {
    let x_20 : i32 = a;
    let x_21 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_20 < x_21)) {
    } else {
      break;
    }
    let x_22 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    b = x_22;
    loop {
      let x_23 : i32 = b;
      let x_24 : i32 = x_6.x_GLF_uniform_int_values[3].el;
      if ((x_23 < x_24)) {
      } else {
        break;
      }
      let x_25 : i32 = x_6.x_GLF_uniform_int_values[1].el;
      c = x_25;
      loop {
        let x_26 : i32 = c;
        let x_27 : i32 = a;
        if ((x_26 <= x_27)) {
        } else {
          break;
        }
        let x_28 : i32 = x_6.x_GLF_uniform_int_values[1].el;
        d = x_28;
        loop {
          let x_29 : i32 = d;
          let x_30 : i32 = x_6.x_GLF_uniform_int_values[3].el;
          if ((x_29 < x_30)) {
          } else {
            break;
          }
          let x_31 : i32 = a;
          let x_32 : i32 = x_6.x_GLF_uniform_int_values[2].el;
          let x_125 : f32 = f32(x_32);
          let x_33 : i32 = c;
          let x_34 : i32 = x_6.x_GLF_uniform_int_values[2].el;
          indexable = mat2x2<f32>(vec2<f32>(x_125, 0.0), vec2<f32>(0.0, x_125));
          let x_131 : f32 = indexable[x_33][x_34];
          sums[x_31] = x_131;
          let x_35 : i32 = a;
          let x_36 : i32 = x_6.x_GLF_uniform_int_values[2].el;
          let x_134 : f32 = f32(x_36);
          let x_37 : i32 = c;
          let x_38 : i32 = x_6.x_GLF_uniform_int_values[2].el;
          indexable_1 = mat2x2<f32>(vec2<f32>(x_134, 0.0), vec2<f32>(0.0, x_134));
          let x_140 : f32 = indexable_1[x_37][x_38];
          let x_142 : f32 = sums[x_35];
          sums[x_35] = (x_142 + x_140);

          continuing {
            let x_39 : i32 = d;
            d = (x_39 + 1);
          }
        }

        continuing {
          let x_41 : i32 = c;
          c = (x_41 + 1);
        }
      }

      continuing {
        let x_43 : i32 = b;
        b = (x_43 + 1);
      }
    }

    continuing {
      let x_45 : i32 = a;
      a = (x_45 + 1);
    }
  }
  let x_47 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_147 : f32 = sums[x_47];
  let x_149 : f32 = x_8.x_GLF_uniform_float_values[1].el;
  let x_150 : bool = (x_147 == x_149);
  x_159_phi = x_150;
  if (x_150) {
    let x_48 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_155 : f32 = sums[x_48];
    let x_157 : f32 = x_8.x_GLF_uniform_float_values[2].el;
    x_158 = (x_155 == x_157);
    x_159_phi = x_158;
  }
  let x_159 : bool = x_159_phi;
  if (x_159) {
    let x_49 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_50 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_51 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_52 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_49), f32(x_50), f32(x_51), f32(x_52));
  } else {
    let x_53 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_173 : f32 = f32(x_53);
    x_GLF_color = vec4<f32>(x_173, x_173, x_173, x_173);
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
