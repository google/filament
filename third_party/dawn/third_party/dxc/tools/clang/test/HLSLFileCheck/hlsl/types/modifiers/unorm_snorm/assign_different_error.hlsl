// RUN: %dxc -T vs_6_0 -E main %s | FileCheck %s | XFail GitHub #2106

// Test that assignments between resources of different
// snorm/unorm qualifiers are reported as errors.

// CHECK-NOT: @dx.op.bufferLoad.f32

Buffer<snorm float> buf_snorm;

float main() : OUT
{
  Buffer<unorm float> buf_unorm = buf_snorm; // FXC: error X3017: cannot implicitly convert from 'const Buffer<snorm float>' to 'Buffer<unorm float>'
  return buf_unorm.Load(0);
}