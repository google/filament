// RUN: %dxc -E main -T ps_6_2 -enable-16bit-types %s

Texture2D<half> tex_f16;
Texture2D<int16_t> tex_i16;
SamplerState samp;
SamplerComparisonState sampcmp;
float cmpval;

float isHalf(float); // Overload resolution shouldn't pick this
float isHalf(half) { return 0; }
float isInt16(int); // Overload resolution shouldn't pick this
float isInt16(int16_t) { return 0; }

float main() : SV_Target
{
  return isHalf(tex_f16.Gather(samp, 0).x) + isInt16(tex_i16.Gather(samp, 0).x)
    + isHalf(tex_f16.GatherRed(samp, 0).x) + isInt16(tex_i16.GatherRed(samp, 0).x)
    + isHalf(tex_f16.GatherGreen(samp, 0).x) + isInt16(tex_i16.GatherGreen(samp, 0).x)
    + isHalf(tex_f16.GatherBlue(samp, 0).x) + isInt16(tex_i16.GatherBlue(samp, 0).x)
    + isHalf(tex_f16.GatherAlpha(samp, 0).x) + isInt16(tex_i16.GatherAlpha(samp, 0).x)
    + isHalf(tex_f16.GatherCmp(sampcmp, 0, cmpval).x) + isInt16(tex_i16.GatherCmp(sampcmp, 0, cmpval).x)
    + isHalf(tex_f16.GatherCmpRed(sampcmp, 0, cmpval).x) + isInt16(tex_i16.GatherCmpRed(sampcmp, 0, cmpval).x)
    + isHalf(tex_f16.GatherCmpGreen(sampcmp, 0, cmpval).x) + isInt16(tex_i16.GatherCmpGreen(sampcmp, 0, cmpval).x)
    + isHalf(tex_f16.GatherCmpBlue(sampcmp, 0, cmpval).x) + isInt16(tex_i16.GatherCmpBlue(sampcmp, 0, cmpval).x)
    + isHalf(tex_f16.GatherCmpAlpha(sampcmp, 0, cmpval).x) + isInt16(tex_i16.GatherCmpAlpha(sampcmp, 0, cmpval).x)
    + isHalf(tex_f16.Load(0).x) + isInt16(tex_i16.Load(0).x);
}
