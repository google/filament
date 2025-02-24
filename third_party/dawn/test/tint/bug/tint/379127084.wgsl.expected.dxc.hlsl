void set_vector_element(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

struct FSIn {
  uint2 ssboIndicesVar;
  float2 localCoordsVar;
};
struct FSOut {
  float4 sk_FragColor;
};

ByteAddressBuffer _storage1 : register(t2);
static uint shadingSsboIndex = 0u;
SamplerState permutationsSampler_1_Sampler : register(s0, space1);
Texture2D<float4> permutationsSampler_1_Texture : register(t1, space1);
SamplerState noiseSampler_1_Sampler : register(s2, space1);
Texture2D<float4> noiseSampler_1_Texture : register(t3, space1);

float4x4 _storage1_load_2(uint offset) {
  return float4x4(asfloat(_storage1.Load4((offset + 0u))), asfloat(_storage1.Load4((offset + 16u))), asfloat(_storage1.Load4((offset + 32u))), asfloat(_storage1.Load4((offset + 48u))));
}

void _skslMain(FSIn _stageIn, inout FSOut _stageOut) {
  {
    uint tint_symbol_6 = 0u;
    _storage1.GetDimensions(tint_symbol_6);
    uint tint_symbol_7 = ((tint_symbol_6 - 0u) / 128u);
    shadingSsboIndex = _stageIn.ssboIndicesVar.y;
    int _56_d = asint(_storage1.Load(((128u * min(shadingSsboIndex, (tint_symbol_7 - 1u))) + 16u)));
    float2 _57_k = float2(((_stageIn.localCoordsVar + 0.5f) * asfloat(_storage1.Load2((128u * min(shadingSsboIndex, (tint_symbol_7 - 1u)))))));
    float4 _58_l = (0.0f).xxxx;
    float2 _59_m = float2(asfloat(_storage1.Load2(((128u * min(shadingSsboIndex, (tint_symbol_7 - 1u))) + 8u))));
    float _60_n = 1.0f;
    {
      int _61_o = 0;
      while (true) {
        uint tint_symbol_8 = 0u;
        _storage1.GetDimensions(tint_symbol_8);
        uint tint_symbol_9 = ((tint_symbol_8 - 0u) / 128u);
        if ((_61_o < asint(_storage1.Load(((128u * min(shadingSsboIndex, (tint_symbol_9 - 1u))) + 20u))))) {
          {
            uint tint_symbol_10 = 0u;
            _storage1.GetDimensions(tint_symbol_10);
            uint tint_symbol_11 = ((tint_symbol_10 - 0u) / 128u);
            float4 _62_f = float4(0.0f, 0.0f, 0.0f, 0.0f);
            float2 _skTemp2 = floor(_57_k);
            _62_f = float4(_skTemp2, _62_f.zw);
            _62_f = float4(_62_f.xy, (_62_f.xy + (1.0f).xx));
            if (bool(asint(_storage1.Load(((128u * min(shadingSsboIndex, (tint_symbol_11 - 1u))) + 24u))))) {
              float4 _skTemp3 = step(_59_m.xyxy, _62_f);
              _62_f = (_62_f - (_skTemp3 * _59_m.xyxy));
            }
            float4 tint_symbol = permutationsSampler_1_Texture.SampleBias(permutationsSampler_1_Sampler, float2(float2(((_62_f.x + 0.5f) * 0.00390625f), 0.5f)), clamp(-0.47499999403953552246f, -16.0f, 15.99f));
            float _63_g = tint_symbol.x;
            float4 tint_symbol_1 = permutationsSampler_1_Texture.SampleBias(permutationsSampler_1_Sampler, float2(float2(((_62_f.z + 0.5f) * 0.00390625f), 0.5f)), clamp(-0.47499999403953552246f, -16.0f, 15.99f));
            float _64_h = tint_symbol_1.x;
            float2 _65_i = float2(_63_g, _64_h);
            if (false) {
              float2 _skTemp4 = floor(((_65_i * (255.0f).xx) + (0.5f).xx));
              _65_i = (_skTemp4 * (0.0039215688593685627f).xx);
            }
            float4 _66_j = ((256.0f * _65_i.xyxy) + _62_f.yyww);
            _66_j = (_66_j * (0.00390625f).xxxx);
            float4 _67_p = _66_j;
            float2 _skTemp5 = frac(_57_k);
            float2 _68_d = _skTemp5;
            float2 _skTemp6 = smoothstep((0.0f).xx, (1.0f).xx, _68_d);
            float2 _69_e = _skTemp6;
            float4 _71_g = float4(0.0f, 0.0f, 0.0f, 0.0f);
            {
              int _72_h = 0;
              while (true) {
                {
                  float _73_i = ((float(_72_h) + 0.5f) * 0.25f);
                  float4 _74_j = noiseSampler_1_Texture.SampleBias(noiseSampler_1_Sampler, float2(float(_67_p.x), float(_73_i)), clamp(-0.47499999403953552246f, -16.0f, 15.99f));
                  float4 _75_k = noiseSampler_1_Texture.SampleBias(noiseSampler_1_Sampler, float2(float(_67_p.y), float(_73_i)), clamp(-0.47499999403953552246f, -16.0f, 15.99f));
                  float4 _76_l = noiseSampler_1_Texture.SampleBias(noiseSampler_1_Sampler, float2(float(_67_p.w), float(_73_i)), clamp(-0.47499999403953552246f, -16.0f, 15.99f));
                  float4 _77_m = noiseSampler_1_Texture.SampleBias(noiseSampler_1_Sampler, float2(float(_67_p.z), float(_73_i)), clamp(-0.47499999403953552246f, -16.0f, 15.99f));
                  float2 _78_n = _68_d;
                  float _skTemp7 = dot((((_74_j.yw + (_74_j.xz * 0.00390625f)) * 2.0f) - 1.0f), _78_n);
                  float _79_o = _skTemp7;
                  _78_n.x = (_78_n.x - 1.0f);
                  float _skTemp8 = dot((((_75_k.yw + (_75_k.xz * 0.00390625f)) * 2.0f) - 1.0f), _78_n);
                  float _80_p = _skTemp8;
                  float _skTemp9 = lerp(_79_o, _80_p, _69_e.x);
                  float _81_q = _skTemp9;
                  _78_n.y = (_78_n.y - 1.0f);
                  float _skTemp10 = dot((((_76_l.yw + (_76_l.xz * 0.00390625f)) * 2.0f) - 1.0f), _78_n);
                  _80_p = _skTemp10;
                  _78_n.x = (_78_n.x + 1.0f);
                  float _skTemp11 = dot((((_77_m.yw + (_77_m.xz * 0.00390625f)) * 2.0f) - 1.0f), _78_n);
                  _79_o = _skTemp11;
                  float _skTemp12 = lerp(_79_o, _80_p, _69_e.x);
                  float _82_r = _skTemp12;
                  float _skTemp13 = lerp(_81_q, _82_r, _69_e.y);
                  set_vector_element(_71_g, min(uint(_72_h), 3u), _skTemp13);
                }
                {
                  _72_h = (_72_h + 1);
                  if ((_72_h >= 4)) { break; }
                }
              }
            }
            float4 _83_q = _71_g;
            if ((_56_d != 0)) {
              float4 _skTemp14 = abs(_83_q);
              _83_q = _skTemp14;
            }
            _58_l = (_58_l + (_83_q * _60_n));
            _57_k = (_57_k * (2.0f).xx);
            _60_n = (_60_n * 0.5f);
            _59_m = (_59_m * (2.0f).xx);
          }
        } else {
          break;
        }
        {
          _61_o = (_61_o + 1);
        }
      }
    }
    if ((_56_d == 0)) {
      _58_l = ((_58_l * (0.5f).xxxx) + (0.5f).xxxx);
    }
    float4 _skTemp15 = saturate(_58_l);
    _58_l = _skTemp15;
    float _skTemp16 = dot(float3(0.21259999275207519531f, 0.71520000696182250977f, 0.07220000028610229492f), float4(float3((float3(_58_l.xyz) * float(_58_l.w))), float(float(_58_l.w))).xyz);
    float _skTemp17 = saturate(_skTemp16);
    float4 _84_a = float4(0.0f, 0.0f, 0.0f, _skTemp17);
    int _85_d = asint(_storage1.Load(((128u * min(shadingSsboIndex, (tint_symbol_7 - 1u))) + 112u)));
    if (bool(_85_d)) {
      {
        float4 _skTemp18 = float4(0.0f, 0.0f, 0.0f, 0.0f);
        if ((_84_a.y < _84_a.z)) {
          _skTemp18 = float4(_84_a.zy, -1.0f, 0.6666666865348815918f);
        } else {
          _skTemp18 = float4(_84_a.yz, 0.0f, -0.3333333432674407959f);
        }
        float4 _86_e = _skTemp18;
        float4 _skTemp19 = float4(0.0f, 0.0f, 0.0f, 0.0f);
        if ((_84_a.x < _86_e.x)) {
          _skTemp19 = float4(_86_e.x, _84_a.x, _86_e.yw);
        } else {
          _skTemp19 = float4(_84_a.x, _86_e.x, _86_e.yz);
        }
        float4 _87_f = _skTemp19;
        float _88_h = _87_f.x;
        float _skTemp20 = min(_87_f.y, _87_f.z);
        float _89_i = (_88_h - _skTemp20);
        float _90_j = (_88_h - (_89_i * 0.5f));
        float _skTemp21 = abs((_87_f.w + ((_87_f.y - _87_f.z) / ((_89_i * 6.0f) + 0.00009999999747378752f))));
        float _91_k = _skTemp21;
        float _skTemp22 = abs(((_90_j * 2.0f) - _84_a.w));
        float _92_l = (_89_i / ((_84_a.w + 0.00009999999747378752f) - _skTemp22));
        float _93_m = (_90_j / (_84_a.w + 0.00009999999747378752f));
        _84_a = float4(_91_k, _92_l, _93_m, _84_a.w);
      }
    } else {
      {
        float _skTemp23 = max(_84_a.w, 0.00009999999747378752f);
        _84_a = float4((_84_a.xyz / _skTemp23), _84_a.w);
      }
    }
    float4 _94_f = float4((mul(float4(_84_a), _storage1_load_2(((128u * min(shadingSsboIndex, (tint_symbol_7 - 1u))) + 32u))) + asfloat(_storage1.Load4(((128u * min(shadingSsboIndex, (tint_symbol_7 - 1u))) + 96u)))));
    if (bool(_85_d)) {
      {
        float _skTemp24 = abs(((2.0f * _94_f.z) - 1.0f));
        float _95_b = ((1.0f - _skTemp24) * _94_f.y);
        float3 _96_c = (_94_f.xxx + float3(0.0f, 0.6666666865348815918f, 0.3333333432674407959f));
        float3 _skTemp25 = frac(_96_c);
        float3 _skTemp26 = abs(((_skTemp25 * 6.0f) - 3.0f));
        float3 _skTemp27 = saturate((_skTemp26 - 1.0f));
        float3 _97_d = _skTemp27;
        float4 _skTemp28 = saturate(float4(((((_97_d - 0.5f) * _95_b) + _94_f.z) * _94_f.w), _94_f.w));
        _94_f = _skTemp28;
      }
    } else {
      {
        uint tint_symbol_12 = 0u;
        _storage1.GetDimensions(tint_symbol_12);
        uint tint_symbol_13 = ((tint_symbol_12 - 0u) / 128u);
        if (bool(asint(_storage1.Load(((128u * min(shadingSsboIndex, (tint_symbol_13 - 1u))) + 116u))))) {
          float4 _skTemp29 = saturate(_94_f);
          _94_f = _skTemp29;
        } else {
          float _skTemp30 = saturate(_94_f.w);
          _94_f.w = _skTemp30;
        }
        _94_f = float4((_94_f.xyz * _94_f.w), _94_f.w);
      }
    }
    float4 outColor_0 = _94_f;
    _stageOut.sk_FragColor = outColor_0;
  }
}

struct tint_symbol_3 {
  nointerpolation uint2 ssboIndicesVar : TEXCOORD0;
  float2 localCoordsVar : TEXCOORD1;
};
struct tint_symbol_4 {
  float4 sk_FragColor : SV_Target0;
};

FSOut main_inner(FSIn _stageIn) {
  FSOut _stageOut = (FSOut)0;
  _skslMain(_stageIn, _stageOut);
  return _stageOut;
}

tint_symbol_4 main(tint_symbol_3 tint_symbol_2) {
  FSIn tint_symbol_14 = {tint_symbol_2.ssboIndicesVar, tint_symbol_2.localCoordsVar};
  FSOut inner_result = main_inner(tint_symbol_14);
  tint_symbol_4 wrapper_result = (tint_symbol_4)0;
  wrapper_result.sk_FragColor = inner_result.sk_FragColor;
  return wrapper_result;
}
