// RUN: %dxc -T ps_6_7 %s -fcgl | FileCheck -check-prefixes=ENTRY,CHECK %s
// RUN: %dxc -T ps_6_7 %s | FileCheck -check-prefix=ENTRY %s
// RUN: %dxc -T ps_6_6 %s -fcgl | FileCheck -check-prefix=IGNORED %s
// RUN: %dxc -T cs_6_7 %s | FileCheck -check-prefix=IGNORED %s

// IGNORED-NOT: waveops-include-helper-lanes

#if __SHADER_TARGET_STAGE == __SHADER_STAGE_COMPUTE

[WaveOpsIncludeHelperLanes]
[numthreads(1,1,1)]
void main() {
}

#else

[WaveOpsIncludeHelperLanes]
float cast(int X) {
  return (float)X;
}

[WaveOpsIncludeHelperLanes]
float main(int N : A, int C : B) : SV_TARGET {
    return cast(N);
}

// ENTRY: define {{(float)|(void)}} @main({{.*}}) #[[MainAttr:[0-9]+]]
// CHECK: define internal float @{{.*}}cast{{.*}}(i32 %{{.*}}) #[[CastAttr:[0-9]+]]

// ENTRY: attributes #[[MainAttr]] = {{.*}}"waveops-include-helper-lanes"{{.*}}
// CHECK-NOT: attribtes #[[CastAttr]] = {{.*}}"waveops-include-helper-lanes"{{.*}}

#endif