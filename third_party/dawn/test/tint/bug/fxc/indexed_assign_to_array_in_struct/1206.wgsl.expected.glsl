#version 310 es


struct Particle {
  vec3 position[8];
  float lifetime;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec4 color;
  vec3 velocity;
  uint tint_pad_3;
};

struct Simulation {
  uint i;
};

layout(binding = 3, std430)
buffer Particles_1_ssbo {
  Particle p[];
} particles;
layout(binding = 4, std140)
uniform sim_block_1_ubo {
  Simulation inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = (uint(particles.p.length()) - 1u);
  uint v_2 = min(uint(0), v_1);
  Particle particle = particles.p[v_2];
  uint v_3 = min(v.inner.i, 7u);
  uint v_4 = min(v.inner.i, 7u);
  particle.position[v_3] = particle.position[v_4];
}
