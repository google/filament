struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 20u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

struct buf1 {
  one : i32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_19 : buf1;

fn main_1() {
  var arr0 : array<i32, 10u>;
  var arr1 : array<i32, 10u>;
  var a : i32;
  var limiter0 : i32;
  var limiter1 : i32;
  var b : i32;
  var limiter2 : i32;
  var limiter3 : i32;
  var d : i32;
  var ref0 : array<i32, 10u>;
  var ref1 : array<i32, 10u>;
  var i : i32;
  let x_59 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_61 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_63 : i32 = x_6.x_GLF_uniform_int_values[4].el;
  let x_65 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  let x_67 : i32 = x_6.x_GLF_uniform_int_values[6].el;
  let x_69 : i32 = x_6.x_GLF_uniform_int_values[7].el;
  let x_71 : i32 = x_6.x_GLF_uniform_int_values[8].el;
  let x_73 : i32 = x_6.x_GLF_uniform_int_values[9].el;
  let x_75 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_77 : i32 = x_6.x_GLF_uniform_int_values[10].el;
  arr0 = array<i32, 10u>(x_59, x_61, x_63, x_65, x_67, x_69, x_71, x_73, x_75, x_77);
  let x_80 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_82 : i32 = x_6.x_GLF_uniform_int_values[12].el;
  let x_84 : i32 = x_6.x_GLF_uniform_int_values[15].el;
  let x_86 : i32 = x_6.x_GLF_uniform_int_values[16].el;
  let x_88 : i32 = x_6.x_GLF_uniform_int_values[17].el;
  let x_90 : i32 = x_6.x_GLF_uniform_int_values[13].el;
  let x_92 : i32 = x_6.x_GLF_uniform_int_values[14].el;
  let x_94 : i32 = x_6.x_GLF_uniform_int_values[11].el;
  let x_96 : i32 = x_6.x_GLF_uniform_int_values[18].el;
  let x_98 : i32 = x_6.x_GLF_uniform_int_values[19].el;
  arr1 = array<i32, 10u>(x_80, x_82, x_84, x_86, x_88, x_90, x_92, x_94, x_96, x_98);
  let x_101 : i32 = x_6.x_GLF_uniform_int_values[8].el;
  a = x_101;
  loop {
    let x_106 : i32 = a;
    let x_108 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_106 < x_108)) {
    } else {
      break;
    }
    let x_112 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    limiter0 = x_112;
    loop {
      let x_117 : i32 = limiter0;
      let x_119 : i32 = x_6.x_GLF_uniform_int_values[4].el;
      if ((x_117 < x_119)) {
      } else {
        break;
      }
      let x_122 : i32 = limiter0;
      limiter0 = (x_122 + 1);
      let x_125 : i32 = x_6.x_GLF_uniform_int_values[2].el;
      limiter1 = x_125;
      let x_127 : i32 = x_6.x_GLF_uniform_int_values[3].el;
      b = x_127;
      loop {
        let x_132 : i32 = b;
        let x_134 : i32 = x_6.x_GLF_uniform_int_values[1].el;
        if ((x_132 < x_134)) {
        } else {
          break;
        }
        let x_137 : i32 = limiter1;
        let x_139 : i32 = x_6.x_GLF_uniform_int_values[5].el;
        if ((x_137 > x_139)) {
          break;
        }
        let x_143 : i32 = limiter1;
        limiter1 = (x_143 + 1);
        let x_145 : i32 = b;
        let x_146 : i32 = a;
        let x_148 : i32 = arr1[x_146];
        arr0[x_145] = x_148;

        continuing {
          let x_150 : i32 = b;
          b = (x_150 + 1);
        }
      }
    }
    limiter2 = 0;
    loop {
      let x_156 : i32 = limiter2;
      if ((x_156 < 5)) {
      } else {
        break;
      }
      let x_159 : i32 = limiter2;
      limiter2 = (x_159 + 1);
      let x_162 : i32 = arr1[1];
      arr0[1] = x_162;
    }
    loop {
      limiter3 = 0;
      d = 0;
      loop {
        let x_172 : i32 = d;
        if ((x_172 < 10)) {
        } else {
          break;
        }
        let x_175 : i32 = limiter3;
        if ((x_175 > 4)) {
          break;
        }
        let x_179 : i32 = limiter3;
        limiter3 = (x_179 + 1);
        let x_181 : i32 = d;
        let x_182 : i32 = d;
        let x_184 : i32 = arr0[x_182];
        arr1[x_181] = x_184;

        continuing {
          let x_186 : i32 = d;
          d = (x_186 + 1);
        }
      }

      continuing {
        let x_189 : i32 = x_6.x_GLF_uniform_int_values[2].el;
        let x_191 : i32 = x_6.x_GLF_uniform_int_values[3].el;
        break if !(x_189 == x_191);
      }
    }

    continuing {
      let x_193 : i32 = a;
      a = (x_193 + 1);
    }
  }
  let x_196 : i32 = x_6.x_GLF_uniform_int_values[11].el;
  let x_198 : i32 = x_6.x_GLF_uniform_int_values[12].el;
  let x_200 : i32 = x_6.x_GLF_uniform_int_values[11].el;
  let x_202 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  let x_204 : i32 = x_6.x_GLF_uniform_int_values[6].el;
  let x_206 : i32 = x_6.x_GLF_uniform_int_values[7].el;
  let x_208 : i32 = x_6.x_GLF_uniform_int_values[8].el;
  let x_210 : i32 = x_6.x_GLF_uniform_int_values[9].el;
  let x_212 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_214 : i32 = x_6.x_GLF_uniform_int_values[10].el;
  ref0 = array<i32, 10u>(x_196, x_198, x_200, x_202, x_204, x_206, x_208, x_210, x_212, x_214);
  let x_217 : i32 = x_6.x_GLF_uniform_int_values[11].el;
  let x_219 : i32 = x_6.x_GLF_uniform_int_values[12].el;
  let x_221 : i32 = x_6.x_GLF_uniform_int_values[11].el;
  let x_223 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  let x_225 : i32 = x_6.x_GLF_uniform_int_values[6].el;
  let x_227 : i32 = x_6.x_GLF_uniform_int_values[13].el;
  let x_229 : i32 = x_6.x_GLF_uniform_int_values[14].el;
  let x_231 : i32 = x_6.x_GLF_uniform_int_values[11].el;
  let x_233 : i32 = x_6.x_GLF_uniform_int_values[18].el;
  let x_235 : i32 = x_6.x_GLF_uniform_int_values[19].el;
  ref1 = array<i32, 10u>(x_217, x_219, x_221, x_223, x_225, x_227, x_229, x_231, x_233, x_235);
  let x_238 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_241 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_244 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_247 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  x_GLF_color = vec4<f32>(f32(x_238), f32(x_241), f32(x_244), f32(x_247));
  let x_251 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  i = x_251;
  loop {
    var x_277 : bool;
    var x_278_phi : bool;
    let x_256 : i32 = i;
    let x_258 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    if ((x_256 < x_258)) {
    } else {
      break;
    }
    let x_261 : i32 = i;
    let x_263 : i32 = arr0[x_261];
    let x_264 : i32 = i;
    let x_266 : i32 = ref0[x_264];
    let x_267 : bool = (x_263 != x_266);
    x_278_phi = x_267;
    if (!(x_267)) {
      let x_271 : i32 = i;
      let x_273 : i32 = arr1[x_271];
      let x_274 : i32 = i;
      let x_276 : i32 = ref1[x_274];
      x_277 = (x_273 != x_276);
      x_278_phi = x_277;
    }
    let x_278 : bool = x_278_phi;
    if (x_278) {
      let x_282 : i32 = x_6.x_GLF_uniform_int_values[3].el;
      let x_283 : f32 = f32(x_282);
      x_GLF_color = vec4<f32>(x_283, x_283, x_283, x_283);
    }

    continuing {
      let x_285 : i32 = i;
      i = (x_285 + 1);
    }
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
