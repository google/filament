// RUN: %dxc -T vs_6_0 -E main %s | FileCheck %s

// Test that snorm/unorm is preserved in the DXIL metadata for
// resources declared as fields of global structures.

// With an SRV and textures (9 = float, 13 = snorm float, 14 = unorm float)
// CHECK-DAG: !{i32 0, %{{.*}}* undef, !{{.*}}, i32 0, i32 0, i32 1, i32 1, i32 0, ![[bs:.*]]}
// CHECK-DAG: ![[bs]] = !{i32 0, i32 13}
struct { Texture1D<snorm float> tex_snorm; } globals;

float main() : OUT
{
  return globals.tex_snorm.Load(0);
}