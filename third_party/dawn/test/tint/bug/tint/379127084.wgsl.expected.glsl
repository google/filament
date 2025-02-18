#version 310 es
precision highp float;
precision highp int;


struct FSUniformData {
  vec2 baseFrequency_1;
  vec2 stitchData_1;
  int noiseType_1;
  int numOctaves_1;
  int stitching_1;
  uint tint_pad_0;
  mat4 matrix_4;
  vec4 translate_4;
  int inHSL_4;
  int clampRGB_4;
  uint tint_pad_1;
  uint tint_pad_2;
};

struct FSIn {
  uvec2 ssboIndicesVar;
  vec2 localCoordsVar;
};

struct FSOut {
  vec4 sk_FragColor;
};

layout(binding = 2, std430)
buffer f_FSUniforms_ssbo {
  FSUniformData fsUniformData[];
} _storage1;
uint shadingSsboIndex = 0u;
uniform highp sampler2D permutationsSampler_1_Texture_permutationsSampler_1_Sampler;
uniform highp sampler2D noiseSampler_1_Texture_noiseSampler_1_Sampler;
layout(location = 0) flat in uvec2 tint_interstage_location0;
layout(location = 1) in vec2 tint_interstage_location1;
layout(location = 0) out vec4 main_loc0_Output;
void _skslMain(FSIn _stageIn, inout FSOut _stageOut) {
  shadingSsboIndex = _stageIn.ssboIndicesVar.y;
  uint v = shadingSsboIndex;
  uint v_1 = min(v, (uint(_storage1.fsUniformData.length()) - 1u));
  int _56_d = _storage1.fsUniformData[v_1].noiseType_1;
  uint v_2 = shadingSsboIndex;
  uint v_3 = min(v_2, (uint(_storage1.fsUniformData.length()) - 1u));
  vec2 _57_k = vec2(((_stageIn.localCoordsVar + 0.5f) * _storage1.fsUniformData[v_3].baseFrequency_1));
  vec4 _58_l = vec4(0.0f);
  uint v_4 = shadingSsboIndex;
  uint v_5 = min(v_4, (uint(_storage1.fsUniformData.length()) - 1u));
  vec2 _59_m = vec2(_storage1.fsUniformData[v_5].stitchData_1);
  float _60_n = 1.0f;
  int _61_o = 0;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      int v_6 = _61_o;
      uint v_7 = shadingSsboIndex;
      uint v_8 = min(v_7, (uint(_storage1.fsUniformData.length()) - 1u));
      if ((v_6 < _storage1.fsUniformData[v_8].numOctaves_1)) {
        vec4 _62_f = vec4(0.0f);
        vec2 _skTemp2 = floor(_57_k);
        _62_f = vec4(_skTemp2, _62_f.zw);
        _62_f = vec4(_62_f.xy, (_62_f.xy + vec2(1.0f)));
        uint v_9 = shadingSsboIndex;
        uint v_10 = min(v_9, (uint(_storage1.fsUniformData.length()) - 1u));
        if (bool(_storage1.fsUniformData[v_10].stitching_1)) {
          vec4 _skTemp3 = step(_59_m.xyxy, _62_f);
          _62_f = (_62_f - (_skTemp3 * _59_m.xyxy));
        }
        float _63_g = texture(permutationsSampler_1_Texture_permutationsSampler_1_Sampler, vec2(vec2(((_62_f.x + 0.5f) * 0.00390625f), 0.5f)), clamp(-0.47499999403953552246f, -16.0f, 15.9899997711181640625f)).x;
        float _64_h = texture(permutationsSampler_1_Texture_permutationsSampler_1_Sampler, vec2(vec2(((_62_f.z + 0.5f) * 0.00390625f), 0.5f)), clamp(-0.47499999403953552246f, -16.0f, 15.9899997711181640625f)).x;
        vec2 _65_i = vec2(_63_g, _64_h);
        if (false) {
          vec2 _skTemp4 = floor(((_65_i * vec2(255.0f)) + vec2(0.5f)));
          _65_i = (_skTemp4 * vec2(0.0039215688593685627f));
        }
        vec4 _66_j = ((256.0f * _65_i.xyxy) + _62_f.yyww);
        _66_j = (_66_j * vec4(0.00390625f));
        vec4 _67_p = _66_j;
        vec2 _skTemp5 = fract(_57_k);
        vec2 _68_d = _skTemp5;
        vec2 _skTemp6 = (clamp(((_68_d - vec2(0.0f)) / (vec2(1.0f) - vec2(0.0f))), vec2(0.0f), vec2(1.0f)) * (clamp(((_68_d - vec2(0.0f)) / (vec2(1.0f) - vec2(0.0f))), vec2(0.0f), vec2(1.0f)) * (vec2(3.0f) - (vec2(2.0f) * clamp(((_68_d - vec2(0.0f)) / (vec2(1.0f) - vec2(0.0f))), vec2(0.0f), vec2(1.0f))))));
        vec2 _69_e = _skTemp6;
        vec4 _71_g = vec4(0.0f);
        int _72_h = 0;
        {
          uvec2 tint_loop_idx_1 = uvec2(0u);
          while(true) {
            if (all(equal(tint_loop_idx_1, uvec2(4294967295u)))) {
              break;
            }
            float _73_i = ((float(_72_h) + 0.5f) * 0.25f);
            float v_11 = float(_67_p.x);
            vec4 _74_j = texture(noiseSampler_1_Texture_noiseSampler_1_Sampler, vec2(v_11, float(_73_i)), clamp(-0.47499999403953552246f, -16.0f, 15.9899997711181640625f));
            float v_12 = float(_67_p.y);
            vec4 _75_k = texture(noiseSampler_1_Texture_noiseSampler_1_Sampler, vec2(v_12, float(_73_i)), clamp(-0.47499999403953552246f, -16.0f, 15.9899997711181640625f));
            float v_13 = float(_67_p.w);
            vec4 _76_l = texture(noiseSampler_1_Texture_noiseSampler_1_Sampler, vec2(v_13, float(_73_i)), clamp(-0.47499999403953552246f, -16.0f, 15.9899997711181640625f));
            float v_14 = float(_67_p.z);
            vec4 _77_m = texture(noiseSampler_1_Texture_noiseSampler_1_Sampler, vec2(v_14, float(_73_i)), clamp(-0.47499999403953552246f, -16.0f, 15.9899997711181640625f));
            vec2 _78_n = _68_d;
            float _skTemp7 = dot((((_74_j.yw + (_74_j.xz * 0.00390625f)) * 2.0f) - 1.0f), _78_n);
            float _79_o = _skTemp7;
            _78_n.x = (_78_n.x - 1.0f);
            float _skTemp8 = dot((((_75_k.yw + (_75_k.xz * 0.00390625f)) * 2.0f) - 1.0f), _78_n);
            float _80_p = _skTemp8;
            float _skTemp9 = mix(_79_o, _80_p, _69_e.x);
            float _81_q = _skTemp9;
            _78_n.y = (_78_n.y - 1.0f);
            float _skTemp10 = dot((((_76_l.yw + (_76_l.xz * 0.00390625f)) * 2.0f) - 1.0f), _78_n);
            _80_p = _skTemp10;
            _78_n.x = (_78_n.x + 1.0f);
            float _skTemp11 = dot((((_77_m.yw + (_77_m.xz * 0.00390625f)) * 2.0f) - 1.0f), _78_n);
            _79_o = _skTemp11;
            float _skTemp12 = mix(_79_o, _80_p, _69_e.x);
            float _82_r = _skTemp12;
            float _skTemp13 = mix(_81_q, _82_r, _69_e.y);
            _71_g[min(uint(_72_h), 3u)] = _skTemp13;
            {
              uint tint_low_inc_1 = (tint_loop_idx_1.x + 1u);
              tint_loop_idx_1.x = tint_low_inc_1;
              uint tint_carry_1 = uint((tint_low_inc_1 == 0u));
              tint_loop_idx_1.y = (tint_loop_idx_1.y + tint_carry_1);
              _72_h = (_72_h + 1);
              if ((_72_h >= 4)) { break; }
            }
            continue;
          }
        }
        vec4 _83_q = _71_g;
        if ((_56_d != 0)) {
          vec4 _skTemp14 = abs(_83_q);
          _83_q = _skTemp14;
        }
        _58_l = (_58_l + (_83_q * _60_n));
        _57_k = (_57_k * vec2(2.0f));
        _60_n = (_60_n * 0.5f);
        _59_m = (_59_m * vec2(2.0f));
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        _61_o = (_61_o + 1);
      }
      continue;
    }
  }
  if ((_56_d == 0)) {
    _58_l = ((_58_l * vec4(0.5f)) + vec4(0.5f));
  }
  vec4 _skTemp15 = clamp(_58_l, vec4(0.0f), vec4(1.0f));
  _58_l = _skTemp15;
  vec3 v_15 = vec3(_58_l.xyz);
  vec3 v_16 = vec3((v_15 * float(_58_l.w)));
  float _skTemp16 = dot(vec3(0.21259999275207519531f, 0.71520000696182250977f, 0.07220000028610229492f), vec4(v_16, float(float(_58_l.w))).xyz);
  float _skTemp17 = clamp(_skTemp16, 0.0f, 1.0f);
  vec4 _84_a = vec4(0.0f, 0.0f, 0.0f, _skTemp17);
  uint v_17 = shadingSsboIndex;
  uint v_18 = min(v_17, (uint(_storage1.fsUniformData.length()) - 1u));
  int _85_d = _storage1.fsUniformData[v_18].inHSL_4;
  if (bool(_85_d)) {
    vec4 _skTemp18 = vec4(0.0f);
    if ((_84_a.y < _84_a.z)) {
      _skTemp18 = vec4(_84_a.zy, -1.0f, 0.6666666865348815918f);
    } else {
      _skTemp18 = vec4(_84_a.yz, 0.0f, -0.3333333432674407959f);
    }
    vec4 _86_e = _skTemp18;
    vec4 _skTemp19 = vec4(0.0f);
    if ((_84_a.x < _86_e.x)) {
      _skTemp19 = vec4(_86_e.x, _84_a.x, _86_e.yw);
    } else {
      _skTemp19 = vec4(_84_a.x, _86_e.x, _86_e.yz);
    }
    vec4 _87_f = _skTemp19;
    float _88_h = _87_f.x;
    float _skTemp20 = min(_87_f.y, _87_f.z);
    float _89_i = (_88_h - _skTemp20);
    float _90_j = (_88_h - (_89_i * 0.5f));
    float _skTemp21 = abs((_87_f.w + ((_87_f.y - _87_f.z) / ((_89_i * 6.0f) + 0.00009999999747378752f))));
    float _91_k = _skTemp21;
    float _skTemp22 = abs(((_90_j * 2.0f) - _84_a.w));
    float _92_l = (_89_i / ((_84_a.w + 0.00009999999747378752f) - _skTemp22));
    float _93_m = (_90_j / (_84_a.w + 0.00009999999747378752f));
    _84_a = vec4(_91_k, _92_l, _93_m, _84_a.w);
  } else {
    float _skTemp23 = max(_84_a.w, 0.00009999999747378752f);
    _84_a = vec4((_84_a.xyz / _skTemp23), _84_a.w);
  }
  uint v_19 = shadingSsboIndex;
  uint v_20 = min(v_19, (uint(_storage1.fsUniformData.length()) - 1u));
  mat4 v_21 = _storage1.fsUniformData[v_20].matrix_4;
  vec4 v_22 = (v_21 * vec4(_84_a));
  uint v_23 = shadingSsboIndex;
  uint v_24 = min(v_23, (uint(_storage1.fsUniformData.length()) - 1u));
  vec4 _94_f = vec4((v_22 + _storage1.fsUniformData[v_24].translate_4));
  if (bool(_85_d)) {
    float _skTemp24 = abs(((2.0f * _94_f.z) - 1.0f));
    float _95_b = ((1.0f - _skTemp24) * _94_f.y);
    vec3 _96_c = (_94_f.xxx + vec3(0.0f, 0.6666666865348815918f, 0.3333333432674407959f));
    vec3 _skTemp25 = fract(_96_c);
    vec3 _skTemp26 = abs(((_skTemp25 * 6.0f) - 3.0f));
    vec3 _skTemp27 = clamp((_skTemp26 - 1.0f), vec3(0.0f), vec3(1.0f));
    vec3 _97_d = _skTemp27;
    vec4 _skTemp28 = clamp(vec4(((((_97_d - 0.5f) * _95_b) + _94_f.z) * _94_f.w), _94_f.w), vec4(0.0f), vec4(1.0f));
    _94_f = _skTemp28;
  } else {
    uint v_25 = shadingSsboIndex;
    uint v_26 = min(v_25, (uint(_storage1.fsUniformData.length()) - 1u));
    if (bool(_storage1.fsUniformData[v_26].clampRGB_4)) {
      vec4 _skTemp29 = clamp(_94_f, vec4(0.0f), vec4(1.0f));
      _94_f = _skTemp29;
    } else {
      float _skTemp30 = clamp(_94_f.w, 0.0f, 1.0f);
      _94_f.w = _skTemp30;
    }
    _94_f = vec4((_94_f.xyz * _94_f.w), _94_f.w);
  }
  vec4 outColor_0 = _94_f;
  _stageOut.sk_FragColor = outColor_0;
}
FSOut main_inner(FSIn _stageIn) {
  FSOut _stageOut = FSOut(vec4(0.0f));
  _skslMain(_stageIn, _stageOut);
  return _stageOut;
}
void main() {
  main_loc0_Output = main_inner(FSIn(tint_interstage_location0, tint_interstage_location1)).sk_FragColor;
}
