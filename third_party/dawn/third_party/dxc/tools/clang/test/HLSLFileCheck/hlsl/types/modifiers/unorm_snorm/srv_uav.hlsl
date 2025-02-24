// RUN: %dxc -T vs_6_0 -E main %s | FileCheck %s

// Test that snorm/unorm is preserved in the DXIL metadata.

// With an SRV and textures (9 = float, 13 = snorm float, 14 = unorm float)
// CHECK-DAG: !{i32 0, %{{.*}}* undef, !{{.*}}, i32 0, i32 0, i32 1, i32 1, i32 0, ![[bf:.*]]}
// CHECK-DAG: ![[bf]] = !{i32 0, i32 9}
// CHECK-DAG: !{i32 1, %{{.*}}* undef, !{{.*}}, i32 0, i32 1, i32 1, i32 1, i32 0, ![[bs:.*]]}
// CHECK-DAG: ![[bs]] = !{i32 0, i32 13}
// CHECK-DAG: !{i32 2, %{{.*}}* undef, !{{.*}}, i32 0, i32 2, i32 1, i32 1, i32 0, ![[bu:.*]]}
// CHECK-DAG: ![[bu]] = !{i32 0, i32 14}
Texture1D<float> tex_float;
Texture1D<snorm float> tex_snorm;
Texture1D<unorm float> tex_unorm;

// With a UAV and buffers (9 = float, 13 = snorm float, 14 = unorm float)
// CHECK-DAG: !{i32 0, %{{.*}}* undef, !{{.*}}, i32 0, i32 0, i32 1, i32 10, i1 false, i1 false, i1 false, ![[rwbf:.*]]}
// CHECK-DAG: ![[rwbf]] = !{i32 0, i32 9}
// CHECK-DAG: !{i32 1, %{{.*}}* undef, !{{.*}}, i32 0, i32 1, i32 1, i32 10, i1 false, i1 false, i1 false, ![[rwbs:.*]]}
// CHECK-DAG: ![[rwbs]] = !{i32 0, i32 13}
// CHECK-DAG: !{i32 2, %{{.*}}* undef, !{{.*}}, i32 0, i32 2, i32 1, i32 10, i1 false, i1 false, i1 false, ![[rwbu:.*]]}
// CHECK-DAG: ![[rwbu]] = !{i32 0, i32 14}
RWBuffer<float> rwbuf_float;
RWBuffer<snorm float> rwbuf_snorm;
RWBuffer<unorm float> rwbuf_unorm;

float main() : OUT
{
  return tex_float.Load(0) + tex_snorm.Load(0) + tex_unorm.Load(0)
    + rwbuf_float.Load(0) + rwbuf_snorm.Load(0) + rwbuf_unorm.Load(0);
}