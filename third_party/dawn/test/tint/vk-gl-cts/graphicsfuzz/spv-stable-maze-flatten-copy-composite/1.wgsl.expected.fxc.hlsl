SKIP: FAILED

int tint_ftoi(float v) {
  return ((v < 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
static int map[256] = (int[256])0;
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int tint_mod(int lhs, int rhs) {
  int rhs_or_one = (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs);
  if (any(((uint((lhs | rhs_or_one)) & 2147483648u) != 0u))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

void main_1() {
  float2 pos = float2(0.0f, 0.0f);
  int2 ipos = int2(0, 0);
  int i = 0;
  int2 p = int2(0, 0);
  bool canwalk = false;
  int v = 0;
  int directions = 0;
  int j = 0;
  int d = 0;
  float4 x_59 = gl_FragCoord;
  float2 x_62 = asfloat(x_7[0].xy);
  pos = (float2(x_59.x, x_59.y) / x_62);
  float x_65 = pos.x;
  float x_69 = pos.y;
  ipos = int2(tint_ftoi((x_65 * 16.0f)), tint_ftoi((x_69 * 16.0f)));
  i = 0;
  while (true) {
    int x_77 = i;
    if ((x_77 < 256)) {
    } else {
      break;
    }
    int x_80 = i;
    map[x_80] = 0;
    {
      int x_82 = i;
      i = (x_82 + 1);
    }
  }
  p = (0).xx;
  canwalk = true;
  v = 0;
  while (true) {
    bool x_104 = false;
    bool x_124 = false;
    bool x_144 = false;
    bool x_164 = false;
    bool x_105_phi = false;
    bool x_125_phi = false;
    bool x_145_phi = false;
    bool x_165_phi = false;
    int x_88 = v;
    v = (x_88 + 1);
    directions = 0;
    int x_91 = p.x;
    bool x_92 = (x_91 > 0);
    x_105_phi = x_92;
    if (x_92) {
      int x_96 = p.x;
      int x_99 = p.y;
      int x_103 = map[((x_96 - 2) + (x_99 * 16))];
      x_104 = (x_103 == 0);
      x_105_phi = x_104;
    }
    bool x_105 = x_105_phi;
    if (x_105) {
      int x_108 = directions;
      directions = (x_108 + 1);
    }
    int x_111 = p.y;
    bool x_112 = (x_111 > 0);
    x_125_phi = x_112;
    if (x_112) {
      int x_116 = p.x;
      int x_118 = p.y;
      int x_123 = map[(x_116 + ((x_118 - 2) * 16))];
      x_124 = (x_123 == 0);
      x_125_phi = x_124;
    }
    bool x_125 = x_125_phi;
    if (x_125) {
      int x_128 = directions;
      directions = (x_128 + 1);
    }
    int x_131 = p.x;
    bool x_132 = (x_131 < 14);
    x_145_phi = x_132;
    if (x_132) {
      int x_136 = p.x;
      int x_139 = p.y;
      int x_143 = map[((x_136 + 2) + (x_139 * 16))];
      x_144 = (x_143 == 0);
      x_145_phi = x_144;
    }
    bool x_145 = x_145_phi;
    if (x_145) {
      int x_148 = directions;
      directions = (x_148 + 1);
    }
    int x_151 = p.y;
    bool x_152 = (x_151 < 14);
    x_165_phi = x_152;
    if (x_152) {
      int x_156 = p.x;
      int x_158 = p.y;
      int x_163 = map[(x_156 + ((x_158 + 2) * 16))];
      x_164 = (x_163 == 0);
      x_165_phi = x_164;
    }
    bool x_165 = x_165_phi;
    if (x_165) {
      int x_168 = directions;
      directions = (x_168 + 1);
    }
    bool x_229 = false;
    bool x_242 = false;
    bool x_281 = false;
    int x_288 = 0;
    int x_289 = 0;
    int x_295 = 0;
    int x_296 = 0;
    int x_303[256] = (int[256])0;
    int x_304[256] = (int[256])0;
    int x_315 = 0;
    int x_316 = 0;
    bool x_359 = false;
    bool x_372 = false;
    bool x_411 = false;
    bool x_424 = false;
    bool x_230_phi = false;
    bool x_243_phi = false;
    bool x_282_phi = false;
    int x_290_phi = 0;
    int x_297_phi = 0;
    int x_305_phi[256] = (int[256])0;
    int x_317_phi = 0;
    bool x_360_phi = false;
    bool x_373_phi = false;
    bool x_412_phi = false;
    bool x_425_phi = false;
    int x_170 = directions;
    if ((x_170 == 0)) {
      canwalk = false;
      i = 0;
      while (true) {
        int x_179 = i;
        if ((x_179 < 8)) {
        } else {
          break;
        }
        j = 0;
        while (true) {
          int x_186 = j;
          if ((x_186 < 8)) {
          } else {
            break;
          }
          int x_189 = j;
          int x_191 = i;
          int x_196 = map[((x_189 * 2) + ((x_191 * 2) * 16))];
          if ((x_196 == 0)) {
            int x_200 = j;
            p.x = (x_200 * 2);
            int x_203 = i;
            p.y = (x_203 * 2);
            canwalk = true;
          }
          {
            int x_206 = j;
            j = (x_206 + 1);
          }
        }
        {
          int x_208 = i;
          i = (x_208 + 1);
        }
      }
      int x_211 = p.x;
      int x_213 = p.y;
      map[(x_211 + (x_213 * 16))] = 1;
    } else {
      int x_217 = v;
      int x_218 = directions;
      d = tint_mod(x_217, x_218);
      int x_220 = directions;
      int x_221 = v;
      v = (x_221 + x_220);
      int x_223 = d;
      bool x_224 = (x_223 >= 0);
      x_230_phi = x_224;
      if (x_224) {
        int x_228 = p.x;
        x_229 = (x_228 > 0);
        x_230_phi = x_229;
      }
      bool x_230 = x_230_phi;
      x_243_phi = x_230;
      if (x_230) {
        int x_234 = p.x;
        int x_237 = p.y;
        int x_241 = map[((x_234 - 2) + (x_237 * 16))];
        x_242 = (x_241 == 0);
        x_243_phi = x_242;
      }
      bool x_243 = x_243_phi;
      if (x_243) {
        int x_246 = d;
        d = (x_246 - 1);
        int x_249 = p.x;
        int x_251 = p.y;
        map[(x_249 + (x_251 * 16))] = 1;
        int x_256 = p.x;
        int x_259 = p.y;
        map[((x_256 - 1) + (x_259 * 16))] = 1;
        int x_264 = p.x;
        int x_267 = p.y;
        map[((x_264 - 2) + (x_267 * 16))] = 1;
        int x_272 = p.x;
        p.x = (x_272 - 2);
      }
      int x_275 = d;
      bool x_276 = (x_275 >= 0);
      x_282_phi = x_276;
      if (x_276) {
        int x_280 = p.y;
        x_281 = (x_280 > 0);
        x_282_phi = x_281;
      }
      bool x_282 = x_282_phi;
      if (x_282) {
        x_288 = p.x;
        x_290_phi = x_288;
      } else {
        x_289 = 0;
        x_290_phi = x_289;
      }
      int x_290 = x_290_phi;
      if (x_282) {
        x_295 = p.y;
        x_297_phi = x_295;
      } else {
        x_296 = 0;
        x_297_phi = x_296;
      }
      int x_297 = x_297_phi;
      int x_299 = ((x_297 - 2) * 16);
      if (x_282) {
        x_303 = map;
        x_305_phi = x_303;
      } else {
        int tint_symbol_3[256] = (int[256])0;
        x_304 = tint_symbol_3;
        x_305_phi = x_304;
      }
      int x_305[256] = x_305_phi;
      if (x_282) {
        int tint_symbol_4[256] = (int[256])0;
        map = tint_symbol_4;
      }
      if (x_282) {
        map = x_305;
      }
      if (x_282) {
        x_315 = map[(x_290 + x_299)];
        x_317_phi = x_315;
      } else {
        x_316 = 0;
        x_317_phi = x_316;
      }
      int x_317 = x_317_phi;
      bool x_318 = (x_317 == 0);
      if ((x_282 ? x_318 : x_282)) {
        int x_323 = d;
        d = (x_323 - 1);
        int x_326 = p.x;
        int x_328 = p.y;
        map[(x_326 + (x_328 * 16))] = 1;
        int x_333 = p.x;
        int x_335 = p.y;
        map[(x_333 + ((x_335 - 1) * 16))] = 1;
        int x_341 = p.x;
        int x_343 = p.y;
        int x_345[256] = map;
        int tint_symbol_5[256] = (int[256])0;
        map = tint_symbol_5;
        map = x_345;
        map[(x_341 + ((x_343 - 2) * 16))] = 1;
        int x_350 = p.y;
        p.y = (x_350 - 2);
      }
      int x_353 = d;
      bool x_354 = (x_353 >= 0);
      x_360_phi = x_354;
      if (x_354) {
        int x_358 = p.x;
        x_359 = (x_358 < 14);
        x_360_phi = x_359;
      }
      bool x_360 = x_360_phi;
      x_373_phi = x_360;
      if (x_360) {
        int x_364 = p.x;
        int x_367 = p.y;
        int x_371 = map[((x_364 + 2) + (x_367 * 16))];
        x_372 = (x_371 == 0);
        x_373_phi = x_372;
      }
      bool x_373 = x_373_phi;
      if (x_373) {
        int x_376 = d;
        d = (x_376 - 1);
        int x_379 = p.x;
        int x_381 = p.y;
        map[(x_379 + (x_381 * 16))] = 1;
        int x_386 = p.x;
        int x_389 = p.y;
        map[((x_386 + 1) + (x_389 * 16))] = 1;
        int x_394 = p.x;
        int x_397 = p.y;
        map[((x_394 + 2) + (x_397 * 16))] = 1;
        int x_402 = p.x;
        p.x = (x_402 + 2);
      }
      int x_405 = d;
      bool x_406 = (x_405 >= 0);
      x_412_phi = x_406;
      if (x_406) {
        int x_410 = p.y;
        x_411 = (x_410 < 14);
        x_412_phi = x_411;
      }
      bool x_412 = x_412_phi;
      x_425_phi = x_412;
      if (x_412) {
        int x_416 = p.x;
        int x_418 = p.y;
        int x_423 = map[(x_416 + ((x_418 + 2) * 16))];
        x_424 = (x_423 == 0);
        x_425_phi = x_424;
      }
      bool x_425 = x_425_phi;
      if (x_425) {
        int x_428 = d;
        d = (x_428 - 1);
        int x_431 = p.x;
        int x_433 = p.y;
        map[(x_431 + (x_433 * 16))] = 1;
        int x_438 = p.x;
        int x_440 = p.y;
        map[(x_438 + ((x_440 + 1) * 16))] = 1;
        int x_446 = p.x;
        int x_448 = p.y;
        map[(x_446 + ((x_448 + 2) * 16))] = 1;
        int x_454 = p.y;
        p.y = (x_454 + 2);
      }
    }
    int x_458 = ipos.y;
    int x_461 = ipos.x;
    int x_464 = map[((x_458 * 16) + x_461)];
    if ((x_464 == 1)) {
      x_GLF_color = (1.0f).xxxx;
      return;
    }
    {
      bool x_468 = canwalk;
      if (!(x_468)) { break; }
    }
  }
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out tint_symbol_6 = {x_GLF_color};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  main_out inner_result = main_inner(float4(tint_symbol.gl_FragCoord_param.xyz, (1.0f / tint_symbol.gl_FragCoord_param.w)));
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
