diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);

struct FSIn {
  @location(0) @interpolate(flat, either)
  ssboIndicesVar : vec2<u32>,
  @location(1)
  localCoordsVar : vec2<f32>,
}

struct FSOut {
  @location(0)
  sk_FragColor : vec4<f32>,
}

struct IntrinsicUniforms {
  @size(16)
  viewport : vec4<f32>,
  dstCopyBounds : vec4<f32>,
}

@group(0) @binding(0) var<uniform> _uniform0 : IntrinsicUniforms;

struct FSUniforms {
  fsUniformData : array<FSUniformData>,
}

@group(0) @binding(2) var<storage, read> _storage1 : FSUniforms;

struct FSUniformData {
  baseFrequency_1 : vec2<f32>,
  stitchData_1 : vec2<f32>,
  noiseType_1 : i32,
  numOctaves_1 : i32,
  stitching_1 : i32,
  matrix_4 : mat4x4<f32>,
  translate_4 : vec4<f32>,
  inHSL_4 : i32,
  clampRGB_4 : i32,
}

var<private> shadingSsboIndex : u32;

@group(1) @binding(0) var permutationsSampler_1_Sampler : sampler;

@group(1) @binding(1) var permutationsSampler_1_Texture : texture_2d<f32>;

@group(1) @binding(2) var noiseSampler_1_Sampler : sampler;

@group(1) @binding(3) var noiseSampler_1_Texture : texture_2d<f32>;

fn _skslMain(_stageIn : FSIn, _stageOut : ptr<function, FSOut>) {
  {
    shadingSsboIndex = _stageIn.ssboIndicesVar.y;
    let _56_d : i32 = _storage1.fsUniformData[shadingSsboIndex].noiseType_1;
    var _57_k : vec2<f32> = vec2<f32>(((_stageIn.localCoordsVar + 0.5) * _storage1.fsUniformData[shadingSsboIndex].baseFrequency_1));
    var _58_l : vec4<f32> = vec4<f32>(0.0);
    var _59_m : vec2<f32> = vec2<f32>(_storage1.fsUniformData[shadingSsboIndex].stitchData_1);
    var _60_n : f32 = 1.0;
    {
      var _61_o : i32 = 0;
      loop {
        if ((_61_o < _storage1.fsUniformData[shadingSsboIndex].numOctaves_1)) {
          {
            var _62_f : vec4<f32>;
            let _skTemp2 = floor(_57_k);
            _62_f = vec4<f32>(_skTemp2, _62_f.zw);
            _62_f = vec4<f32>(_62_f.xy, (_62_f.xy + vec2<f32>(1.0)));
            if (bool(_storage1.fsUniformData[shadingSsboIndex].stitching_1)) {
              let _skTemp3 = step(_59_m.xyxy, _62_f);
              _62_f = (_62_f - (_skTemp3 * _59_m.xyxy));
            }
            let _63_g : f32 = textureSampleBias(permutationsSampler_1_Texture, permutationsSampler_1_Sampler, vec2<f32>(vec2<f32>(((_62_f.x + 0.5) * 0.00390625), 0.5)), -(0.4749999999999999778)).x;
            let _64_h : f32 = textureSampleBias(permutationsSampler_1_Texture, permutationsSampler_1_Sampler, vec2<f32>(vec2<f32>(((_62_f.z + 0.5) * 0.00390625), 0.5)), -(0.4749999999999999778)).x;
            var _65_i : vec2<f32> = vec2<f32>(_63_g, _64_h);
            if (false) {
              let _skTemp4 = floor(((_65_i * vec2<f32>(255.0)) + vec2<f32>(0.5)));
              _65_i = (_skTemp4 * vec2<f32>(0.00392156899999999975));
            }
            var _66_j : vec4<f32> = ((256.0 * _65_i.xyxy) + _62_f.yyww);
            _66_j = (_66_j * vec4<f32>(0.00390625));
            let _67_p : vec4<f32> = _66_j;
            let _skTemp5 = fract(_57_k);
            let _68_d : vec2<f32> = _skTemp5;
            let _skTemp6 = smoothstep(vec2<f32>(0.0), vec2<f32>(1.0), _68_d);
            let _69_e : vec2<f32> = _skTemp6;
            const _70_f : f32 = 0.00390625;
            var _71_g : vec4<f32>;
            {
              var _72_h : i32 = 0;
              loop {
                {
                  let _73_i : f32 = ((f32(_72_h) + 0.5) * 0.25);
                  let _74_j : vec4<f32> = textureSampleBias(noiseSampler_1_Texture, noiseSampler_1_Sampler, vec2<f32>(f32(_67_p.x), f32(_73_i)), -(0.4749999999999999778));
                  let _75_k : vec4<f32> = textureSampleBias(noiseSampler_1_Texture, noiseSampler_1_Sampler, vec2<f32>(f32(_67_p.y), f32(_73_i)), -(0.4749999999999999778));
                  let _76_l : vec4<f32> = textureSampleBias(noiseSampler_1_Texture, noiseSampler_1_Sampler, vec2<f32>(f32(_67_p.w), f32(_73_i)), -(0.4749999999999999778));
                  let _77_m : vec4<f32> = textureSampleBias(noiseSampler_1_Texture, noiseSampler_1_Sampler, vec2<f32>(f32(_67_p.z), f32(_73_i)), -(0.4749999999999999778));
                  var _78_n : vec2<f32> = _68_d;
                  let _skTemp7 = dot((((_74_j.yw + (_74_j.xz * _70_f)) * 2.0) - 1.0), _78_n);
                  var _79_o : f32 = _skTemp7;
                  _78_n.x = (_78_n.x - 1.0);
                  let _skTemp8 = dot((((_75_k.yw + (_75_k.xz * _70_f)) * 2.0) - 1.0), _78_n);
                  var _80_p : f32 = _skTemp8;
                  let _skTemp9 = mix(_79_o, _80_p, _69_e.x);
                  let _81_q : f32 = _skTemp9;
                  _78_n.y = (_78_n.y - 1.0);
                  let _skTemp10 = dot((((_76_l.yw + (_76_l.xz * _70_f)) * 2.0) - 1.0), _78_n);
                  _80_p = _skTemp10;
                  _78_n.x = (_78_n.x + 1.0);
                  let _skTemp11 = dot((((_77_m.yw + (_77_m.xz * _70_f)) * 2.0) - 1.0), _78_n);
                  _79_o = _skTemp11;
                  let _skTemp12 = mix(_79_o, _80_p, _69_e.x);
                  let _82_r : f32 = _skTemp12;
                  let _skTemp13 = mix(_81_q, _82_r, _69_e.y);
                  _71_g[_72_h] = _skTemp13;
                }

                continuing {
                  _72_h = (_72_h + i32(1));
                  break if (_72_h >= 4);
                }
              }
            }
            var _83_q : vec4<f32> = _71_g;
            if ((_56_d != 0)) {
              let _skTemp14 = abs(_83_q);
              _83_q = _skTemp14;
            }
            _58_l = (_58_l + (_83_q * _60_n));
            _57_k = (_57_k * vec2<f32>(2.0));
            _60_n = (_60_n * 0.5);
            _59_m = (_59_m * vec2<f32>(2.0));
          }
        } else {
          break;
        }

        continuing {
          _61_o = (_61_o + i32(1));
        }
      }
    }
    if ((_56_d == 0)) {
      _58_l = ((_58_l * vec4<f32>(0.5)) + vec4<f32>(0.5));
    }
    let _skTemp15 = saturate(_58_l);
    _58_l = _skTemp15;
    let _skTemp16 = dot(vec3<f32>(0.21260000000000001119, 0.71519999999999994689, 0.07220000000000000029), vec4<f32>(vec3<f32>((vec3<f32>(_58_l.xyz) * f32(_58_l.w))), f32(f32(_58_l.w))).xyz);
    let _skTemp17 = saturate(_skTemp16);
    var _84_a : vec4<f32> = vec4<f32>(0.0, 0.0, 0.0, _skTemp17);
    let _85_d : i32 = _storage1.fsUniformData[shadingSsboIndex].inHSL_4;
    if (bool(_85_d)) {
      {
        var _skTemp18 : vec4<f32>;
        if ((_84_a.y < _84_a.z)) {
          _skTemp18 = vec4<f32>(_84_a.zy, -(1.0), 0.66666669999999994545);
        } else {
          _skTemp18 = vec4<f32>(_84_a.yz, 0.0, -(0.33333334300000000416));
        }
        let _86_e : vec4<f32> = _skTemp18;
        var _skTemp19 : vec4<f32>;
        if ((_84_a.x < _86_e.x)) {
          _skTemp19 = vec4<f32>(_86_e.x, _84_a.x, _86_e.yw);
        } else {
          _skTemp19 = vec4<f32>(_84_a.x, _86_e.x, _86_e.yz);
        }
        let _87_f : vec4<f32> = _skTemp19;
        let _88_h : f32 = _87_f.x;
        let _skTemp20 = min(_87_f.y, _87_f.z);
        let _89_i : f32 = (_88_h - _skTemp20);
        let _90_j : f32 = (_88_h - (_89_i * 0.5));
        let _skTemp21 = abs((_87_f.w + ((_87_f.y - _87_f.z) / ((_89_i * 6.0) + 0.0001))));
        let _91_k : f32 = _skTemp21;
        let _skTemp22 = abs(((_90_j * 2.0) - _84_a.w));
        let _92_l : f32 = (_89_i / ((_84_a.w + 0.0001) - _skTemp22));
        let _93_m : f32 = (_90_j / (_84_a.w + 0.0001));
        _84_a = vec4<f32>(_91_k, _92_l, _93_m, _84_a.w);
      }
    } else {
      {
        let _skTemp23 = max(_84_a.w, 0.0001);
        _84_a = vec4<f32>((_84_a.xyz / _skTemp23), _84_a.w);
      }
    }
    var _94_f : vec4<f32> = vec4<f32>(((_storage1.fsUniformData[shadingSsboIndex].matrix_4 * vec4<f32>(_84_a)) + _storage1.fsUniformData[shadingSsboIndex].translate_4));
    if (bool(_85_d)) {
      {
        let _skTemp24 = abs(((2.0 * _94_f.z) - 1.0));
        let _95_b : f32 = ((1.0 - _skTemp24) * _94_f.y);
        let _96_c : vec3<f32> = (_94_f.xxx + vec3<f32>(0.0, 0.66666669999999994545, 0.33333334300000000416));
        let _skTemp25 = fract(_96_c);
        let _skTemp26 = abs(((_skTemp25 * 6.0) - 3.0));
        let _skTemp27 = saturate((_skTemp26 - 1.0));
        let _97_d : vec3<f32> = _skTemp27;
        let _skTemp28 = saturate(vec4<f32>(((((_97_d - 0.5) * _95_b) + _94_f.z) * _94_f.w), _94_f.w));
        _94_f = _skTemp28;
      }
    } else {
      {
        if (bool(_storage1.fsUniformData[shadingSsboIndex].clampRGB_4)) {
          let _skTemp29 = saturate(_94_f);
          _94_f = _skTemp29;
        } else {
          let _skTemp30 = saturate(_94_f.w);
          _94_f.w = _skTemp30;
        }
        _94_f = vec4<f32>((_94_f.xyz * _94_f.w), _94_f.w);
      }
    }
    let outColor_0 : vec4<f32> = _94_f;
    (*(_stageOut)).sk_FragColor = outColor_0;
  }
}

@fragment
fn main(_stageIn : FSIn) -> FSOut {
  var _stageOut : FSOut;
  _skslMain(_stageIn, &(_stageOut));
  return _stageOut;
}
