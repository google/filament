// RUN: %dxc -T cs_6_0 %s -fcgl | FileCheck %s

// Compiling this HLSL used to trigger this assert:
//    
//    Error: assert(V[i]->getType() == Ty->getElementType() && "Wrong type in array element initializer")
//    File:
//    ..\..\third_party\dawn\third_party\dxc\lib\IR\Constants.cpp(886)
//    Func:   static llvm::ConstantArray::getImpl
//
// Reported in https://github.com/microsoft/DirectXShaderCompiler/issues/5294
//
// Bug was fixed in CGHLSLMSFinishCodeGen in BuildImmInit: when initializing an array,
// if the init value type doesn't match the array element type, bail. We check that
// a call to the global ctor function is called.

// CHECK:      define internal void @"\01??__EP@@YAXXZ"() #1 {
// CHECK:        store i32 0, i32* getelementptr inbounds ([4 x %struct.str], [4 x %struct.str]* @P, i32 0, i32 0, i32 0)
// CHECK-NEXT:   store i32 0, i32* getelementptr inbounds ([4 x %struct.str], [4 x %struct.str]* @P, i32 0, i32 1, i32 0)
// CHECK-NEXT:   store i32 0, i32* getelementptr inbounds ([4 x %struct.str], [4 x %struct.str]* @P, i32 0, i32 2, i32 0)
// CHECK-NEXT:   store i32 0, i32* getelementptr inbounds ([4 x %struct.str], [4 x %struct.str]* @P, i32 0, i32 3, i32 0)
// CHECK-NEXT:   ret void
// CHECK-NEXT: }
//
// CHECK:      define internal void @_GLOBAL__sub_I_construct_global_array_of_struct.hlsl() #1 {
// CHECK:        call void @"\01??__EP@@YAXXZ"()
// CHECK-NEXT:   ret void
// CHECK-NEXT: }

struct str {
  int i;
};

str func(inout str pointer) {
  return pointer;
}

static str P[4] = (str[4])0;

[numthreads(1, 1, 1)]
void main() {
  const str r = func(P[2]);
  return;
}
