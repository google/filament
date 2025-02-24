// RUN: %dxc -Tlib_6_8 -verify %s
// RUN: %dxc -Tvs_6_8 -verify %s

// Test the ordinary shader model case with no visible group.
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

[Shader("vertex")]
uint main() : OUT {

  DeviceBarriers();

  Barrier(buf0, 0);
  Barrier(buf0, DEVICE_SCOPE);

  return buf0[0];
}
