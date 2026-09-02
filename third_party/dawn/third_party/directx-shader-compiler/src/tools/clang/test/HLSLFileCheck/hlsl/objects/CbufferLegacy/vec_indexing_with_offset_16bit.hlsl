// RUN: %dxc -E main -T ps_6_6 -enable-16bit-types -HV 2018 %s | FileCheck %s

// Make sure store .1234 to half4.
// CHECK:%[[X:.*]] = extractvalue %dx.types.CBufRet.f16.8 %{{.*}}, 1
// CHECK:%[[Y:.*]] = extractvalue %dx.types.CBufRet.f16.8 %{{.*}}, 2
// CHECK:%[[Z:.*]] = extractvalue %dx.types.CBufRet.f16.8 %{{.*}}, 3
// CHECK:%[[W:.*]] = extractvalue %dx.types.CBufRet.f16.8 %{{.*}}, 4
// CHECK:%[[PX:.*]] = getelementptr inbounds [4 x half], [4 x half]* %{{.*}}, i32 0, i32 0
// CHECK:store half %[[X]], half* %[[PX]], align 2
// CHECK:%[[PY:.*]] = getelementptr inbounds [4 x half], [4 x half]* %{{.*}}, i32 0, i32 1
// CHECK:store half %[[Y]], half* %[[PY]], align 2
// CHECK:%[[PZ:.*]] = getelementptr inbounds [4 x half], [4 x half]* %{{.*}}, i32 0, i32 2
// CHECK:store half %[[Z]], half* %[[PZ]], align 2
// CHECK:%[[PW:.*]] = getelementptr inbounds [4 x half], [4 x half]* %{{.*}}, i32 0, i32 3
// CHECK:store half %[[W]], half* %[[PW]], align 2

cbuffer cb
{
    int16_t i;
    half4 v;
};

float4 main() : SV_TARGET
{
    return v[i];
}