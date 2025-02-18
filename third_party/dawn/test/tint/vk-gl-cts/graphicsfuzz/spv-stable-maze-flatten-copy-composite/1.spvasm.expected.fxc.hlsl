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
  pos = (gl_FragCoord.xy / asfloat(x_7[0].xy));
  ipos = int2(tint_ftoi((pos.x * 16.0f)), tint_ftoi((pos.y * 16.0f)));
  i = 0;
  while (true) {
    if ((i < 256)) {
    } else {
      break;
    }
    int x_80 = i;
    map[x_80] = 0;
    {
      i = (i + 1);
    }
  }
  p = (0).xx;
  canwalk = true;
  v = 0;
  while (true) {
    bool x_104 = false;
    bool x_105 = false;
    bool x_124 = false;
    bool x_125 = false;
    bool x_144 = false;
    bool x_145 = false;
    bool x_164 = false;
    bool x_165 = false;
    v = (v + 1);
    directions = 0;
    bool x_92 = (p.x > 0);
    x_105 = x_92;
    if (x_92) {
      x_104 = (map[((p.x - 2) + (p.y * 16))] == 0);
      x_105 = x_104;
    }
    if (x_105) {
      directions = (directions + 1);
    }
    bool x_112 = (p.y > 0);
    x_125 = x_112;
    if (x_112) {
      x_124 = (map[(p.x + ((p.y - 2) * 16))] == 0);
      x_125 = x_124;
    }
    if (x_125) {
      directions = (directions + 1);
    }
    bool x_132 = (p.x < 14);
    x_145 = x_132;
    if (x_132) {
      x_144 = (map[((p.x + 2) + (p.y * 16))] == 0);
      x_145 = x_144;
    }
    if (x_145) {
      directions = (directions + 1);
    }
    bool x_152 = (p.y < 14);
    x_165 = x_152;
    if (x_152) {
      x_164 = (map[(p.x + ((p.y + 2) * 16))] == 0);
      x_165 = x_164;
    }
    if (x_165) {
      directions = (directions + 1);
    }
    bool x_229 = false;
    bool x_230 = false;
    bool x_242 = false;
    bool x_243 = false;
    bool x_281 = false;
    bool x_282 = false;
    int x_288 = 0;
    int x_289 = 0;
    int x_290 = 0;
    int x_295 = 0;
    int x_296 = 0;
    int x_297 = 0;
    int x_303[256] = (int[256])0;
    int x_304[256] = (int[256])0;
    int x_305[256] = (int[256])0;
    int x_315 = 0;
    int x_316 = 0;
    int x_317 = 0;
    bool x_359 = false;
    bool x_360 = false;
    bool x_372 = false;
    bool x_373 = false;
    bool x_411 = false;
    bool x_412 = false;
    bool x_424 = false;
    bool x_425 = false;
    if ((directions == 0)) {
      canwalk = false;
      i = 0;
      while (true) {
        if ((i < 8)) {
        } else {
          break;
        }
        j = 0;
        while (true) {
          if ((j < 8)) {
          } else {
            break;
          }
          if ((map[((j * 2) + ((i * 2) * 16))] == 0)) {
            p.x = (j * 2);
            p.y = (i * 2);
            canwalk = true;
          }
          {
            j = (j + 1);
          }
        }
        {
          i = (i + 1);
        }
      }
      int x_211 = p.x;
      int x_213 = p.y;
      map[(x_211 + (x_213 * 16))] = 1;
    } else {
      d = tint_mod(v, directions);
      v = (v + directions);
      bool x_224 = (d >= 0);
      x_230 = x_224;
      if (x_224) {
        x_229 = (p.x > 0);
        x_230 = x_229;
      }
      x_243 = x_230;
      if (x_230) {
        x_242 = (map[((p.x - 2) + (p.y * 16))] == 0);
        x_243 = x_242;
      }
      if (x_243) {
        d = (d - 1);
        int x_249 = p.x;
        int x_251 = p.y;
        map[(x_249 + (x_251 * 16))] = 1;
        int x_256 = p.x;
        int x_259 = p.y;
        map[((x_256 - 1) + (x_259 * 16))] = 1;
        int x_264 = p.x;
        int x_267 = p.y;
        map[((x_264 - 2) + (x_267 * 16))] = 1;
        p.x = (p.x - 2);
      }
      bool x_276 = (d >= 0);
      x_282 = x_276;
      if (x_276) {
        x_281 = (p.y > 0);
        x_282 = x_281;
      }
      if (x_282) {
        x_288 = p.x;
        x_290 = x_288;
      } else {
        x_289 = 0;
        x_290 = x_289;
      }
      if (x_282) {
        x_295 = p.y;
        x_297 = x_295;
      } else {
        x_296 = 0;
        x_297 = x_296;
      }
      int x_299 = ((x_297 - 2) * 16);
      if (x_282) {
        x_303 = map;
        x_305 = x_303;
      } else {
        int tint_symbol_3[256] = (int[256])0;
        x_304 = tint_symbol_3;
        x_305 = x_304;
      }
      if (x_282) {
        int tint_symbol_4[256] = (int[256])0;
        map = tint_symbol_4;
      }
      if (x_282) {
        map = x_305;
      }
      if (x_282) {
        x_315 = map[(x_290 + x_299)];
        x_317 = x_315;
      } else {
        x_316 = 0;
        x_317 = x_316;
      }
      if ((x_282 ? (x_317 == 0) : x_282)) {
        d = (d - 1);
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
        p.y = (p.y - 2);
      }
      bool x_354 = (d >= 0);
      x_360 = x_354;
      if (x_354) {
        x_359 = (p.x < 14);
        x_360 = x_359;
      }
      x_373 = x_360;
      if (x_360) {
        x_372 = (map[((p.x + 2) + (p.y * 16))] == 0);
        x_373 = x_372;
      }
      if (x_373) {
        d = (d - 1);
        int x_379 = p.x;
        int x_381 = p.y;
        map[(x_379 + (x_381 * 16))] = 1;
        int x_386 = p.x;
        int x_389 = p.y;
        map[((x_386 + 1) + (x_389 * 16))] = 1;
        int x_394 = p.x;
        int x_397 = p.y;
        map[((x_394 + 2) + (x_397 * 16))] = 1;
        p.x = (p.x + 2);
      }
      bool x_406 = (d >= 0);
      x_412 = x_406;
      if (x_406) {
        x_411 = (p.y < 14);
        x_412 = x_411;
      }
      x_425 = x_412;
      if (x_412) {
        x_424 = (map[(p.x + ((p.y + 2) * 16))] == 0);
        x_425 = x_424;
      }
      if (x_425) {
        d = (d - 1);
        int x_431 = p.x;
        int x_433 = p.y;
        map[(x_431 + (x_433 * 16))] = 1;
        int x_438 = p.x;
        int x_440 = p.y;
        map[(x_438 + ((x_440 + 1) * 16))] = 1;
        int x_446 = p.x;
        int x_448 = p.y;
        map[(x_446 + ((x_448 + 2) * 16))] = 1;
        p.y = (p.y + 2);
      }
    }
    if ((map[((ipos.y * 16) + ipos.x)] == 1)) {
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
