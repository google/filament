// RUN: %dxc -E main -DMAT1x1 -Zpc -T ms_6_5 %s | FileCheck %s -check-prefix=CHK_MAT1x1
// RUN: %dxc -E main -DMAT1x2 -Zpc -T ms_6_5 %s | FileCheck %s -check-prefix=CHK_MAT1x2
// RUN: %dxc -E main -DMAT2x1 -Zpc -T ms_6_5 %s | FileCheck %s -check-prefix=CHK_MAT2x1
// RUN: %dxc -E main -DMAT2x2 -Zpc -T ms_6_5 %s | FileCheck %s -check-prefix=CHK_MAT2x2
// RUN: %dxc -E main -DMAT2x3 -Zpc -T ms_6_5 %s | FileCheck %s -check-prefix=CHK_MAT2x3
// RUN: %dxc -E main -DMAT3x2 -Zpc -T ms_6_5 %s | FileCheck %s -check-prefix=CHK_MAT3x2
// RUN: %dxc -E main -DMAT3x3 -Zpc -T ms_6_5 %s | FileCheck %s -check-prefix=CHK_MAT3x3
// RUN: %dxc -E main -DMAT3x4 -Zpc -T ms_6_5 %s | FileCheck %s -check-prefix=CHK_MAT3x4
// RUN: %dxc -E main -DMAT4x3 -Zpc -T ms_6_5 %s | FileCheck %s -check-prefix=CHK_MAT4x3
// RUN: %dxc -E main -DMAT4x4 -Zpc -T ms_6_5 %s | FileCheck %s -check-prefix=CHK_MAT4x4

// Regression test to check that store vertex output for matrix works fine

#ifdef MAT1x1
#define TY float1x1
#endif

#ifdef MAT1x2
#define TY float1x2
#endif

#ifdef MAT2x1
#define TY float2x1
#endif

#ifdef MAT2x2
#define TY float2x2
#endif

#ifdef MAT2x3
#define TY float2x3
#endif

#ifdef MAT3x2
#define TY float3x2
#endif

#ifdef MAT3x3
#define TY float3x3
#endif

#ifdef MAT3x4
#define TY float3x4
#endif

#ifdef MAT4x3
#define TY float4x3
#endif

#ifdef MAT4x4
#define TY float4x4
#endif

struct VertexOutput
{
		TY test : TEXCOORD0;
};

[NumThreads(64, 1, 1)]
[OutputTopology("triangle")]
void main(out vertices VertexOutput verts[1])
{
 
 // CHK_MAT1x1: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 0, float 1.000000e+00, i32 0)
#ifdef MAT1x1  
	verts[0].test = TY(1);
#endif  

  // CHK_MAT1x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 0, float 1.000000e+00, i32 0)
  // CHK_MAT1x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 0, float 2.000000e+00, i32 0)
#ifdef MAT1x2
	verts[0].test = TY(float2(1, 2));
#endif

  // CHK_MAT2x1: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 0, float 1.000000e+00, i32 0)
  // CHK_MAT2x1: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 1, float 2.000000e+00, i32 0)
#ifdef MAT2x1
	verts[0].test = TY(float2(1, 2)); 
#endif


  // CHK_MAT2x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 0, float 1.000000e+00, i32 0)
  // CHK_MAT2x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 1, float 3.000000e+00, i32 0)
  // CHK_MAT2x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 0, float 2.000000e+00, i32 0)
  // CHK_MAT2x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 1, float 4.000000e+00, i32 0)  
#ifdef MAT2x2
	verts[0].test = TY(float2(1, 2), 
                     float2(3, 4));
#endif

  // CHK_MAT2x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 0, float 1.000000e+00, i32 0)
  // CHK_MAT2x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 1, float 4.000000e+00, i32 0)
  // CHK_MAT2x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 0, float 2.000000e+00, i32 0)
  // CHK_MAT2x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 1, float 5.000000e+00, i32 0)
  // CHK_MAT2x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 0, float 3.000000e+00, i32 0) 
  // CHK_MAT2x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 1, float 6.000000e+00, i32 0)                     
#ifdef MAT2x3
	verts[0].test = TY(float3(1, 2, 3),
                     float3(4, 5, 6));
#endif

  // CHK_MAT3x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 0, float 1.000000e+00, i32 0)
  // CHK_MAT3x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 1, float 3.000000e+00, i32 0)
  // CHK_MAT3x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 2, float 5.000000e+00, i32 0)
  // CHK_MAT3x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 0, float 2.000000e+00, i32 0)  
  // CHK_MAT3x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 1, float 4.000000e+00, i32 0)  
  // CHK_MAT3x2: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 2, float 6.000000e+00, i32 0)                     
#ifdef MAT3x2
	verts[0].test = TY(float2(1, 2),
                     float2(3, 4),
                     float2(5, 6));
#endif

  // CHK_MAT3x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 0, float 1.000000e+00, i32 0)
  // CHK_MAT3x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 1, float 4.000000e+00, i32 0)
  // CHK_MAT3x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 2, float 7.000000e+00, i32 0)  
  // CHK_MAT3x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 0, float 2.000000e+00, i32 0)
  // CHK_MAT3x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 1, float 5.000000e+00, i32 0)
  // CHK_MAT3x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 2, float 8.000000e+00, i32 0)
  // CHK_MAT3x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 0, float 3.000000e+00, i32 0)  
  // CHK_MAT3x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 1, float 6.000000e+00, i32 0) 
  // CHK_MAT3x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 2, float 9.000000e+00, i32 0)                     
#ifdef MAT3x3
	verts[0].test = TY(float3(1, 2, 3),
                     float3(4, 5, 6),
                     float3(7, 8, 9));
#endif

  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 0, float 1.000000e+00, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 1, float 5.000000e+00, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 2, float 9.000000e+00, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 0, float 2.000000e+00, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 1, float 6.000000e+00, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 2, float 1.000000e+01, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 0, float 3.000000e+00, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 1, float 7.000000e+00, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 2, float 1.100000e+01, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 3, i8 0, float 4.000000e+00, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 3, i8 1, float 8.000000e+00, i32 0)
  // CHK_MAT3x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 3, i8 2, float 1.200000e+01, i32 0)                     
#ifdef MAT3x4
	verts[0].test = TY(float4(1, 2,  3,  4),
                     float4(5, 6,  7,  8),
                     float4(9, 10, 11, 12));
#endif

  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 0, float 1.000000e+00, i32 0)
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 1, float 4.000000e+00, i32 0)
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 2, float 7.000000e+00, i32 0)
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 3, float 1.000000e+01, i32 0)  
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 0, float 2.000000e+00, i32 0)
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 1, float 5.000000e+00, i32 0)
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 2, float 8.000000e+00, i32 0)
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 3, float 1.100000e+01, i32 0)
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 0, float 3.000000e+00, i32 0)  
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 1, float 6.000000e+00, i32 0)  
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 2, float 9.000000e+00, i32 0)  
  // CHK_MAT4x3: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 3, float 1.200000e+01, i32 0)                     
#ifdef MAT4x3
	verts[0].test = TY(float3(1,  2,  3),
                     float3(4,  5,  6),
                     float3(7,  8,  9),
                     float3(10, 11, 12));
#endif
 
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 0, float 1.000000e+00, i32 0)
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 1, float 5.000000e+00, i32 0)
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 2, float 9.000000e+00, i32 0)
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 0, i8 3, float 1.300000e+01, i32 0)  
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 0, float 2.000000e+00, i32 0)
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 1, float 6.000000e+00, i32 0)
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 2, float 1.000000e+01, i32 0)
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 1, i8 3, float 1.400000e+01, i32 0)  
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 0, float 3.000000e+00, i32 0)
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 1, float 7.000000e+00, i32 0)
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 2, float 1.100000e+01, i32 0)
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 2, i8 3, float 1.500000e+01, i32 0)
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 3, i8 0, float 4.000000e+00, i32 0)  
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 3, i8 1, float 8.000000e+00, i32 0) 
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 3, i8 2, float 1.200000e+01, i32 0) 
  // CHK_MAT4x4: call void @dx.op.storeVertexOutput.f32(i32 171, i32 0, i32 3, i8 3, float 1.600000e+01, i32 0) 
#ifdef MAT4x4
 	verts[0].test = TY(float4(1,  2,  3,  4), 
                     float4(5,  6,  7,  8), 
                     float4(9,  10, 11, 12),
                     float4(13, 14, 15, 16));
#endif
  SetMeshOutputCounts(0, 0);
}