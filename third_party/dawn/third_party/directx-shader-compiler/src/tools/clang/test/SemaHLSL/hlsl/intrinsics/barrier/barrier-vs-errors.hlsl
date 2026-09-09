// RUN: %dxc -Tlib_6_8 -verify %s
// RUN: %dxc -Tvs_6_8 -verify %s

// Test the ordinary shader model case with no visible group.

struct RECORD { uint a; };
RWBuffer<uint> buf0;
static uint i = 7;

// Errors expected when called without visible group.
[noinline] export
void GroupBarriers() {
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(0, GROUP_SYNC);

  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(ALL_MEMORY, GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(ALL_MEMORY, GROUP_SCOPE);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(ALL_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(ALL_MEMORY, DEVICE_SCOPE | GROUP_SCOPE);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(ALL_MEMORY, 1 + 2 + 4);

  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(UAV_MEMORY, GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(UAV_MEMORY, GROUP_SCOPE);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(UAV_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(UAV_MEMORY, DEVICE_SCOPE | GROUP_SCOPE);

  // expected-error@+1{{GROUP_SHARED_MEMORY specified for Barrier operation when context has no visible group}}
  Barrier(GROUP_SHARED_MEMORY, 0);
  // expected-error@+2{{GROUP_SHARED_MEMORY specified for Barrier operation when context has no visible group}}
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(GROUP_SHARED_MEMORY, GROUP_SYNC);
  // expected-error@+2{{GROUP_SHARED_MEMORY specified for Barrier operation when context has no visible group}}
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE);
  // expected-error@+2{{GROUP_SHARED_MEMORY specified for Barrier operation when context has no visible group}}
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE);
}

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

// expected-note@+24{{entry function defined here}}
// expected-note@+23{{entry function defined here}}
// expected-note@+22{{entry function defined here}}
// expected-note@+21{{entry function defined here}}
// expected-note@+20{{entry function defined here}}
// expected-note@+19{{entry function defined here}}
// expected-note@+18{{entry function defined here}}
// expected-note@+17{{entry function defined here}}
// expected-note@+16{{entry function defined here}}
// expected-note@+15{{entry function defined here}}
// expected-note@+14{{entry function defined here}}
// expected-note@+13{{entry function defined here}}
// expected-note@+12{{entry function defined here}}
// expected-note@+11{{entry function defined here}}
// expected-note@+10{{entry function defined here}}
// expected-note@+9{{entry function defined here}}
// expected-note@+8{{entry function defined here}}
// expected-note@+7{{entry function defined here}}
// expected-note@+6{{entry function defined here}}
// expected-note@+5{{entry function defined here}}
// expected-note@+4{{entry function defined here}}
// expected-note@+3{{entry function defined here}}
// expected-note@+2{{entry function defined here}}
[Shader("vertex")]
uint main() : OUT {

  GroupBarriers();
  NodeBarriers();

  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(buf0, GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(buf0, GROUP_SCOPE);

  return buf0[0];
}
