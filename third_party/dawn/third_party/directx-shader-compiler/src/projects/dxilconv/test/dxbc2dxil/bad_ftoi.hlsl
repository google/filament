// FXC command line: fxc /T vs_5_0 /Od %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

int4 main() : OUTPUT
{
  return int4(340282346638528860000000000000000000000.0,
              -340282346638528860000000000000000000000.0,
              asint((uint)340282346638528860000000000000000000000.0),
              asint((uint)-340282346638528860000000000000000000000.0));
}

// fxc produces:
// -> ftou o0.z, l(340282346638528860000000000000000000000.000000)
// -> ftou o0.w, l(-340282346638528860000000000000000000000.000000)
// -> ftoi o0.xy, l(340282346638528860000000000000000000000.000000, -340282346638528860000000000000000000000.000000, 0.000000, 0.000000)

// dxbc2dxil used to produce:
// -> call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 2, i32 undef)
// -> call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 3, i32 undef)
// -> call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 undef)
// -> call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 undef)
// "i32 undef" is invalid here.  It's caused by:
//   return of opInvalidOp from APFloat::convertToSignExtendedInteger
//   which llvm::ConstantFoldCastInstruction turns into i32 undef

// fixed dxbc2dxil produces:
// -> call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 2, i32 -1)
// -> call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 3, i32 0)
// -> call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 2147483647)
// -> call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 -2147483648)
// Which is int4(max int, min int, max uint, min uint)
