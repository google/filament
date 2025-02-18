//
// vert_main
//
#version 310 es

layout(location = 0) in vec2 vert_main_loc0_Input;
layout(location = 1) in vec2 vert_main_loc1_Input;
layout(location = 2) in vec2 vert_main_loc2_Input;
vec4 vert_main_inner(vec2 a_particlePos, vec2 a_particleVel, vec2 a_pos) {
  float angle = -(atan(a_particleVel.x, a_particleVel.y));
  vec2 pos = vec2(((a_pos.x * cos(angle)) - (a_pos.y * sin(angle))), ((a_pos.x * sin(angle)) + (a_pos.y * cos(angle))));
  return vec4((pos + a_particlePos), 0.0f, 1.0f);
}
void main() {
  vec4 v = vert_main_inner(vert_main_loc0_Input, vert_main_loc1_Input, vert_main_loc2_Input);
  gl_Position = vec4(v.x, -(v.y), ((2.0f * v.z) - v.w), v.w);
  gl_PointSize = 1.0f;
}
//
// frag_main
//
#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 frag_main_loc0_Output;
vec4 frag_main_inner() {
  return vec4(1.0f);
}
void main() {
  frag_main_loc0_Output = frag_main_inner();
}
//
// comp_main
//
#version 310 es


struct SimParams {
  float deltaT;
  float rule1Distance;
  float rule2Distance;
  float rule3Distance;
  float rule1Scale;
  float rule2Scale;
  float rule3Scale;
};

struct Particle {
  vec2 pos;
  vec2 vel;
};

struct Particles {
  Particle particles[5];
};

layout(binding = 0, std140)
uniform params_block_1_ubo {
  SimParams inner;
} v;
layout(binding = 1, std430)
buffer particlesA_block_1_ssbo {
  Particles inner;
} v_1;
layout(binding = 2, std430)
buffer particlesB_block_1_ssbo {
  Particles inner;
} v_2;
void comp_main_inner(uvec3 v_3) {
  uint index = v_3.x;
  if ((index >= 5u)) {
    return;
  }
  uint v_4 = min(index, 4u);
  vec2 vPos = v_1.inner.particles[v_4].pos;
  uint v_5 = min(index, 4u);
  vec2 vVel = v_1.inner.particles[v_5].vel;
  vec2 cMass = vec2(0.0f);
  vec2 cVel = vec2(0.0f);
  vec2 colVel = vec2(0.0f);
  int cMassCount = 0;
  int cVelCount = 0;
  vec2 pos = vec2(0.0f);
  vec2 vel = vec2(0.0f);
  {
    uint i = 0u;
    while(true) {
      if ((i < 5u)) {
      } else {
        break;
      }
      if ((i == index)) {
        {
          i = (i + 1u);
        }
        continue;
      }
      uint v_6 = min(i, 4u);
      pos = v_1.inner.particles[v_6].pos.xy;
      uint v_7 = min(i, 4u);
      vel = v_1.inner.particles[v_7].vel.xy;
      if ((distance(pos, vPos) < v.inner.rule1Distance)) {
        cMass = (cMass + pos);
        cMassCount = (cMassCount + 1);
      }
      if ((distance(pos, vPos) < v.inner.rule2Distance)) {
        colVel = (colVel - (pos - vPos));
      }
      if ((distance(pos, vPos) < v.inner.rule3Distance)) {
        cVel = (cVel + vel);
        cVelCount = (cVelCount + 1);
      }
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  if ((cMassCount > 0)) {
    vec2 v_8 = cMass;
    float v_9 = float(cMassCount);
    vec2 v_10 = (v_8 / vec2(v_9, float(cMassCount)));
    cMass = (v_10 - vPos);
  }
  if ((cVelCount > 0)) {
    vec2 v_11 = cVel;
    float v_12 = float(cVelCount);
    cVel = (v_11 / vec2(v_12, float(cVelCount)));
  }
  vVel = (((vVel + (cMass * v.inner.rule1Scale)) + (colVel * v.inner.rule2Scale)) + (cVel * v.inner.rule3Scale));
  vVel = (normalize(vVel) * clamp(length(vVel), 0.0f, 0.10000000149011611938f));
  vPos = (vPos + (vVel * v.inner.deltaT));
  if ((vPos.x < -1.0f)) {
    vPos.x = 1.0f;
  }
  if ((vPos.x > 1.0f)) {
    vPos.x = -1.0f;
  }
  if ((vPos.y < -1.0f)) {
    vPos.y = 1.0f;
  }
  if ((vPos.y > 1.0f)) {
    vPos.y = -1.0f;
  }
  uint v_13 = min(index, 4u);
  v_2.inner.particles[v_13].pos = vPos;
  uint v_14 = min(index, 4u);
  v_2.inner.particles[v_14].vel = vVel;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  comp_main_inner(gl_GlobalInvocationID);
}
