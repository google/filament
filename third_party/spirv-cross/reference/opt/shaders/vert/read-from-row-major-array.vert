#version 310 es

layout(binding = 0, std140) uniform Block
{
    layout(row_major) mat2x3 var[3][4];
} _104;

layout(location = 0) in vec4 a_position;
layout(location = 0) out mediump float v_vtxResult;

highp mat2x3 spvWorkaroundRowMajor(highp mat2x3 wrap) { return wrap; }
mediump mat2x3 spvWorkaroundRowMajorMP(mediump mat2x3 wrap) { return wrap; }

void main()
{
    gl_Position = a_position;
    float _172 = float(abs(spvWorkaroundRowMajor(_104.var[0][0])[0].x - 2.0) < 0.0500000007450580596923828125);
    mediump float mp_copy_172 = _172;
    float _180 = float(abs(spvWorkaroundRowMajor(_104.var[0][0])[0].y - 6.0) < 0.0500000007450580596923828125);
    mediump float mp_copy_180 = _180;
    float _188 = float(abs(spvWorkaroundRowMajor(_104.var[0][0])[0].z - (-6.0)) < 0.0500000007450580596923828125);
    mediump float mp_copy_188 = _188;
    float _221 = float(abs(spvWorkaroundRowMajor(_104.var[0][0])[1].x) < 0.0500000007450580596923828125);
    mediump float mp_copy_221 = _221;
    float _229 = float(abs(spvWorkaroundRowMajor(_104.var[0][0])[1].y - 5.0) < 0.0500000007450580596923828125);
    mediump float mp_copy_229 = _229;
    float _237 = float(abs(spvWorkaroundRowMajor(_104.var[0][0])[1].z - 5.0) < 0.0500000007450580596923828125);
    mediump float mp_copy_237 = _237;
    v_vtxResult = ((mp_copy_172 * mp_copy_180) * mp_copy_188) * ((mp_copy_221 * mp_copy_229) * mp_copy_237);
}

