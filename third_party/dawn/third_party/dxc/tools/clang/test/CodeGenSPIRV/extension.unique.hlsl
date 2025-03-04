// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// Make sure the same decoration is not applied twice.
//
// CHECK:     OpExtension "SPV_KHR_16bit_storage"
// CHECK-NOT: OpExtension "SPV_KHR_16bit_storage"

float4 main(int16_t pix_pos : INPUT_1,
            int16_t pix_pos2: INPUT_2): SV_Target {
  return 0;
}
