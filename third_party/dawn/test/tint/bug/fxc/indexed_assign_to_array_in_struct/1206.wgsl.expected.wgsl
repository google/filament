struct Simulation {
  i : u32,
}

struct Particle {
  position : array<vec3<f32>, 8>,
  lifetime : f32,
  color : vec4<f32>,
  velocity : vec3<f32>,
}

struct Particles {
  p : array<Particle>,
}

@group(1) @binding(3) var<storage, read> particles : Particles;

@group(1) @binding(4) var<uniform> sim : Simulation;

@compute @workgroup_size(1)
fn main() {
  var particle = particles.p[0];
  particle.position[sim.i] = particle.position[sim.i];
}
