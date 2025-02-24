// RUN: %dxc -Tlib_6_8 -verify %s
// RUN: %dxc -Tcs_6_8 -verify %s

// Test the ordinary compute shader model case with node memory flags.

struct RECORD { uint a; };
RWBuffer<uint> buf0;
static uint i = 7;

// Node barriers not supported in non-node shaders.
[noinline] export
void NodeBarriers() {
  // expected-error@+1{{NODE_INPUT_MEMORY or NODE_OUTPUT_MEMORY may only be specified for Barrier operation in a node shader}}
  Barrier(NODE_INPUT_MEMORY, 0);
  // expected-error@+1{{NODE_INPUT_MEMORY or NODE_OUTPUT_MEMORY may only be specified for Barrier operation in a node shader}}
  Barrier(NODE_INPUT_MEMORY, DEVICE_SCOPE);

  // expected-error@+1{{NODE_INPUT_MEMORY or NODE_OUTPUT_MEMORY may only be specified for Barrier operation in a node shader}}
  Barrier(NODE_OUTPUT_MEMORY, 0);
  // expected-error@+1{{NODE_INPUT_MEMORY or NODE_OUTPUT_MEMORY may only be specified for Barrier operation in a node shader}}
  Barrier(NODE_OUTPUT_MEMORY, DEVICE_SCOPE);
}

// expected-note@+6{{entry function defined here}}
// expected-note@+5{{entry function defined here}}
// expected-note@+4{{entry function defined here}}
// expected-note@+3{{entry function defined here}}
[Shader("compute")]
[numthreads(1, 1, 1)]
void main() {
  NodeBarriers();
}
