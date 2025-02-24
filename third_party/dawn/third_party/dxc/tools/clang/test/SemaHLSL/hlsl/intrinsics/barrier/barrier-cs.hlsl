// RUN: %dxc -Tlib_6_8 -verify %s
// RUN: %dxc -Tcs_6_8 -verify %s

// Test the compute shader model case with visible group.
// expected-no-diagnostics

struct RECORD { uint a; };
RWBuffer<uint> buf0;
static uint i = 7;

// Barriers not requiring visible group, compatible with any shader stage.
[noinline] export
void DeviceBarriers() {
  Barrier(0, 0);

  Barrier(ALL_MEMORY, 0);
  Barrier(ALL_MEMORY, DEVICE_SCOPE);
  Barrier(ALL_MEMORY, 2 + 2);

  Barrier(UAV_MEMORY, 0);
  Barrier(UAV_MEMORY, DEVICE_SCOPE);

  // No diagnostic for non-constant expressions.
  Barrier(ALL_MEMORY, i + i);
  Barrier(i + i, 0);
}

// Barriers requiring visible group.
[noinline] export
void GroupBarriers() {
  Barrier(0, GROUP_SYNC);

  Barrier(ALL_MEMORY, GROUP_SYNC);
  Barrier(ALL_MEMORY, GROUP_SCOPE);
  Barrier(ALL_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  Barrier(ALL_MEMORY, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);
  Barrier(ALL_MEMORY, 1 + 2 + 4);

  Barrier(UAV_MEMORY, GROUP_SYNC);
  Barrier(UAV_MEMORY, GROUP_SCOPE);
  Barrier(UAV_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  Barrier(UAV_MEMORY, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);

  Barrier(GROUP_SHARED_MEMORY, 0);
  Barrier(GROUP_SHARED_MEMORY, GROUP_SYNC);
  Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE);
  Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE | GROUP_SYNC);
}

[Shader("compute")]
[numthreads(1, 1, 1)]
void main() {
  DeviceBarriers();
  GroupBarriers();

  Barrier(buf0, 0);
  Barrier(buf0, GROUP_SYNC);
  Barrier(buf0, GROUP_SCOPE);
  Barrier(buf0, DEVICE_SCOPE);
  Barrier(buf0, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);
}
