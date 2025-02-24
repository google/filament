// RUN: %dxc -E main -fcgl  -T ps_6_0  %s | FileCheck %s -check-prefix=IR
// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s -check-prefix=DXIL

// The issue happens when d.cb copy in foo and copy in d.cb = cbv.
// Then, when lower memcpy, there're more than one write to d.cb.
// As a result, the memcopy will be flattened into ld/st which will generate alloca and lot of cb load.
// Enable NRVO (Named Return Value Optimization) will avoid copy in return d inside foo and resolve the issue.

// For clang codeGen IR make sure only 1 memcpy call.
// IR:call void @llvm.memcpy
// IR-NOT:call void @llvm.memcpy


// For DXIL, make sure no alloca and only 2 cb load ( 1 for arrayidx, 1 for cbv.idx[arrayidx] ).
// DXIL-NOT:alloca
// DXIL:call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy
// DXIL:call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy
// DXIL-NOT:call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy


struct CB {
  uint idx[8];
};

ConstantBuffer<CB> cbv;

struct Data {
  CB cb;
};

// Named Return Value Optimization
Data foo() {
  Data d;
  // When NRVO is disabled, there will be a copy from d to return value.
  return d;
}

uint arrayidx;

Buffer<float> buf;

float main() : SV_Target {
  Data d = foo();
  d.cb = cbv;

  return buf[d.cb.idx[arrayidx]];
}



