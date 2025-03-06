// RUN: %dxc -Tlib_6_7 -verify %s

// Test non-translatable barriers in compute shader model.

struct RECORD { uint a; };
RWBuffer<uint> buf0;
static uint i = 7;

// Barriers not requiring visible group, compatible with any shader stage.
// expected-note@+4 {{entry function defined here}}
// expected-note@+3 {{entry function defined here}}
// expected-note@+2 {{entry function defined here}}
[noinline] export
void DeviceBarriers() {
  // expected-error@+2 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  // expected-error@+1 {{intrinsic Barrier potentially used by 'DeviceBarriers' requires shader model 6.8 or greater}}
  Barrier(0, 0);
  // expected-error@+2 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  // expected-error@+1 {{intrinsic Barrier potentially used by 'DeviceBarriers' requires shader model 6.8 or greater}}
  Barrier(ALL_MEMORY, 0);
  // expected-error@+2 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  // expected-error@+1 {{intrinsic Barrier potentially used by 'DeviceBarriers' requires shader model 6.8 or greater}}
  Barrier(UAV_MEMORY, 0);
}

// Barriers requiring visible group.
// expected-note@+5 {{entry function defined here}}
// expected-note@+4 {{entry function defined here}}
// expected-note@+3 {{entry function defined here}}
// expected-note@+2 {{entry function defined here}}
[noinline] export
void GroupBarriers() {
  // No scope
  // expected-error@+2 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  // expected-error@+1 {{intrinsic Barrier potentially used by 'GroupBarriers' requires shader model 6.8 or greater}}
  Barrier(ALL_MEMORY, GROUP_SYNC);
  // expected-error@+2 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  // expected-error@+1 {{intrinsic Barrier potentially used by 'GroupBarriers' requires shader model 6.8 or greater}}
  Barrier(UAV_MEMORY, GROUP_SYNC);

  // expected-error@+2 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  // expected-error@+1 {{intrinsic Barrier potentially used by 'GroupBarriers' requires shader model 6.8 or greater}}
  Barrier(GROUP_SHARED_MEMORY, 0);
  // expected-error@+2 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  // expected-error@+1 {{intrinsic Barrier potentially used by 'GroupBarriers' requires shader model 6.8 or greater}}
  Barrier(GROUP_SHARED_MEMORY, GROUP_SYNC);
}

// expected-note@+14 {{entry function defined here}}
// expected-note@+13 {{entry function defined here}}
// expected-note@+12 {{entry function defined here}}
// expected-note@+11 {{entry function defined here}}
// expected-note@+10 {{entry function defined here}}
// expected-note@+9 {{entry function defined here}}
// expected-note@+8 {{entry function defined here}}
// expected-note@+7 {{entry function defined here}}
// expected-note@+6 {{entry function defined here}}
// expected-note@+5 {{entry function defined here}}
// expected-note@+4 {{entry function defined here}}
// expected-note@+3 {{entry function defined here}}
[Shader("compute")]
[numthreads(1, 1, 1)]
void main() {
  DeviceBarriers();
  GroupBarriers();

  // expected-error@+1 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  Barrier(buf0, 0);
  // expected-error@+1 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  Barrier(buf0, GROUP_SYNC);
  // expected-error@+1 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  Barrier(buf0, GROUP_SCOPE);
  // expected-error@+1 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  Barrier(buf0, DEVICE_SCOPE);
  // expected-error@+1 {{intrinsic Barrier potentially used by 'main' requires shader model 6.8 or greater}}
  Barrier(buf0, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);
}

// Unused compute entry should not produce any diagnostics, other than the
// numthreads warning.

// expected-warning@+1 {{attribute 'numthreads' ignored without accompanying shader attribute}}
[numthreads(1, 1, 1)]
void main_unused() {
  DeviceBarriers();
  GroupBarriers();

  Barrier(buf0, 0);
  Barrier(buf0, GROUP_SYNC);
  Barrier(buf0, GROUP_SCOPE);
  Barrier(buf0, DEVICE_SCOPE);
  Barrier(buf0, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);
}
