// RUN: %dxc -T ps_6_8 -E main -Qkeep_reflect_in_dxil %s | FileCheck -check-prefix=CHECK68 %s
// RUN: %dxc -T ps_6_7 -E main -Qkeep_reflect_in_dxil %s | FileCheck -check-prefix=CHECK67 %s

// Make sure the vector is annotated with vector size (DXIL 1.8 and higher),
// matrix is annotated with matrix size and orientation, and scalar does not
// have any extra annotations.

//
// Shader model 6.8
//
// CHECK68: !dx.typeAnnotations
// CHECK68: !{i32 0, %hostlayout.struct.MyStruct undef, [[STRUCT_ANN:![0-9]+]], %"hostlayout.class.StructuredBuffer<MyStruct>" undef, !{{[0-9]+}}}
// CHECK68: [[STRUCT_ANN]] = !{i32 84, [[VECTOR_ANN:![0-9]+]], [[MATRIX_ANN:![0-9]+]], [[SCALAR_ANN:![0-9]+]]}

// FieldAnnotationFieldNameTag(6) = "vec" / "mat" / "scalar"
// FieldAnnotationCBufferOffsetTag(3)
// FieldAnnotationCompTypeTag(7)= 9 (float)
// FieldAnnotationVectorSize(13) = 4
// FieldAnnotationMatrixTag(2)

// CHECK68: [[VECTOR_ANN]] = !{i32 6, !"vec", i32 3, i32 0, i32 7, i32 9, i32 13, i32 4}
// CHECK68: [[MATRIX_ANN]] = !{i32 6, !"mat", i32 2, [[MATRIX__SIZE_ANN:![0-9]+]], i32 3, i32 16, i32 7, i32 9}
// matrix rows, cols, orientation
// CHECK68: [[MATRIX__SIZE_ANN]] = !{i32 4, i32 4, i32 2}
// CHECK68: [[SCALAR_ANN]] = !{i32 6, !"a", i32 3, i32 80, i32 7, i32 9}

//
// Shader model 6.7
//
// CHECK67: !dx.typeAnnotations
// CHECK67: !{i32 0, %hostlayout.struct.MyStruct undef, [[STRUCT_ANN:![0-9]+]], %"hostlayout.class.StructuredBuffer<MyStruct>" undef, !{{[0-9]+}}}
// CHECK67: [[STRUCT_ANN]] = !{i32 84, [[VECTOR_ANN:![0-9]+]], [[MATRIX_ANN:![0-9]+]], [[SCALAR_ANN:![0-9]+]]}
// CHECK67: [[VECTOR_ANN]] = !{i32 6, !"vec", i32 3, i32 0, i32 7, i32 9}
// CHECK67: [[MATRIX_ANN]] = !{i32 6, !"mat", i32 2, [[MATRIX__SIZE_ANN:![0-9]+]], i32 3, i32 16, i32 7, i32 9}
// matrix rows, cols, orientation
// CHECK67: [[MATRIX__SIZE_ANN]] = !{i32 4, i32 4, i32 2}
// CHECK67: [[SCALAR_ANN]] = !{i32 6, !"a", i32 3, i32 80, i32 7, i32 9}

struct MyStruct {
    float4 vec;
    float4x4 mat;
    float a;
};

StructuredBuffer<MyStruct> g_myStruct;

float main() : SV_Target 
{ 
    return g_myStruct[0].vec.x + g_myStruct[0].vec.y; 
}
