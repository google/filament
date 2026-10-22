#version 320 es
precision mediump float;
precision highp int;

layout(binding = 0, std140) uniform UBO
{
    float mediump_float;
    highp float highp_float;
} ubo;

layout(location = 0) out vec4 FragColor0;
layout(location = 1) out vec4 FragColor1;
layout(location = 2) out vec4 FragColor2;
layout(location = 3) out vec4 FragColor3;
layout(location = 0) in vec4 V4;

void main()
{
    highp float hp_copy_phi_mp;
    float mp_copy_phi_hp;
    vec4 V4_value0 = V4;
    highp vec4 hp_copy_V4_value0 = V4_value0;
    float V1_value0 = V4.x;
    highp float hp_copy_V1_value0 = V1_value0;
    float V1_value2 = V4_value0.z;
    highp float hp_copy_V1_value2 = V1_value2;
    float ubo_mp0 = ubo.mediump_float;
    highp float hp_copy_ubo_mp0 = ubo_mp0;
    highp float ubo_hp0 = ubo.highp_float;
    float mp_copy_ubo_hp0 = ubo_hp0;
    highp vec4 _48 = hp_copy_V4_value0 - vec4(3.0);
    vec4 mp_copy_48 = _48;
    FragColor0 = V4_value0 + vec4(3.0);
    FragColor1 = _48;
    FragColor2 = mp_copy_48 * vec4(3.0);
    float _23 = V1_value0 + 3.0;
    float float_0_weird = 3.0 - mp_copy_ubo_hp0;
    highp float hp_copy_float_0_weird = float_0_weird;
    highp float _49 = hp_copy_V1_value0 - hp_copy_float_0_weird;
    float mp_copy_49 = _49;
    FragColor3 = vec4(_23, _49, mp_copy_49 * mp_copy_ubo_hp0, 3.0);
    highp float _51 = hp_copy_V1_value2 - hp_copy_ubo_mp0;
    float mp_copy_51 = _51;
    FragColor3 = vec4(V4_value0.z + ubo_mp0, _51, mp_copy_51 * mp_copy_ubo_hp0, 3.0);
    FragColor0 = sin(hp_copy_V4_value0);
    FragColor1 = sin(V4_value0);
    float phi_mp;
    highp float phi_hp;
    phi_mp = _23;
    phi_hp = _49;
    hp_copy_phi_mp = phi_mp;
    mp_copy_phi_hp = phi_hp;
    FragColor2 = vec4(phi_mp + phi_mp, hp_copy_phi_mp + hp_copy_phi_mp, mp_copy_phi_hp + mp_copy_phi_hp, phi_hp + phi_hp);
}

