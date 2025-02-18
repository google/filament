// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// vertex shader

@vertex
fn vert_main(@location(0) a_particlePos : vec2<f32>,
             @location(1) a_particleVel : vec2<f32>,
             @location(2) a_pos : vec2<f32>)
          -> @builtin(position) vec4<f32> {
  var angle : f32 = -atan2(a_particleVel.x, a_particleVel.y);
  var pos : vec2<f32> = vec2<f32>(
      (a_pos.x * cos(angle)) - (a_pos.y * sin(angle)),
      (a_pos.x * sin(angle)) + (a_pos.y * cos(angle)));
  return vec4<f32>(pos + a_particlePos, 0.0, 1.0);
}

// fragment shader

@fragment
fn frag_main() -> @location(0) vec4<f32> {
  return vec4<f32>(1.0, 1.0, 1.0, 1.0);
}

// compute shader
struct Particle {
  pos : vec2<f32>,
  vel : vec2<f32>,
};

 struct SimParams {
  deltaT : f32,
  rule1Distance : f32,
  rule2Distance : f32,
  rule3Distance : f32,
  rule1Scale : f32,
  rule2Scale : f32,
  rule3Scale : f32,
};

 struct Particles {
  particles : array<Particle, 5>,
};

@binding(0) @group(0) var<uniform> params : SimParams;
@binding(1) @group(0) var<storage, read_write> particlesA : Particles;
@binding(2) @group(0) var<storage, read_write> particlesB : Particles;

// https://github.com/austinEng/Project6-Vulkan-Flocking/blob/master/data/shaders/computeparticles/particle.comp
@compute @workgroup_size(1)
fn comp_main(
  @builtin(global_invocation_id) gl_GlobalInvocationID : vec3<u32>) {
  var index : u32 = gl_GlobalInvocationID.x;
  if (index >= 5u) {
    return;
  }

  var vPos : vec2<f32> = particlesA.particles[index].pos;
  var vVel : vec2<f32> = particlesA.particles[index].vel;

  var cMass : vec2<f32> = vec2<f32>(0.0, 0.0);
  var cVel : vec2<f32> = vec2<f32>(0.0, 0.0);
  var colVel : vec2<f32> = vec2<f32>(0.0, 0.0);
  var cMassCount : i32 = 0;
  var cVelCount : i32 = 0;

  var pos : vec2<f32>;
  var vel : vec2<f32>;
  for(var i : u32 = 0u; i < 5u; i = i + 1u) {
    if (i == index) {
      continue;
    }

    pos = particlesA.particles[i].pos.xy;
    vel = particlesA.particles[i].vel.xy;

    if (distance(pos, vPos) < params.rule1Distance) {
      cMass = cMass + pos;
      cMassCount = cMassCount + 1;
    }
    if (distance(pos, vPos) < params.rule2Distance) {
      colVel = colVel - (pos - vPos);
    }
    if (distance(pos, vPos) < params.rule3Distance) {
      cVel = cVel + vel;
      cVelCount = cVelCount + 1;
    }
  }
  if (cMassCount > 0) {
    cMass =
      (cMass / vec2<f32>(f32(cMassCount), f32(cMassCount))) - vPos;
  }
  if (cVelCount > 0) {
    cVel = cVel / vec2<f32>(f32(cVelCount), f32(cVelCount));
  }

  vVel = vVel + (cMass * params.rule1Scale) + (colVel * params.rule2Scale) +
      (cVel * params.rule3Scale);

  // clamp velocity for a more pleasing simulation
  vVel = normalize(vVel) * clamp(length(vVel), 0.0, 0.1);

  // kinematic update
  vPos = vPos + (vVel * params.deltaT);

  // Wrap around boundary
  if (vPos.x < -1.0) {
    vPos.x = 1.0;
  }
  if (vPos.x > 1.0) {
    vPos.x = -1.0;
  }
  if (vPos.y < -1.0) {
    vPos.y = 1.0;
  }
  if (vPos.y > 1.0) {
    vPos.y = -1.0;
  }

  // Write back
  particlesB.particles[index].pos = vPos;
  particlesB.particles[index].vel = vVel;
}
