// RUN: %dxc -T lib_6_3 %s  | FileCheck %s


// Make sure we're still eliminating all the allocas one way or the other
//CHECK-NOT: alloca

struct Istruct {
  uint ival;
};

cbuffer cbuf : register(b1)
{
  Istruct istructs[1];
}
//CHECK: define i32 @"\01?loop_with_break1{{[@$?.A-Za-z0-9_]+}}"
//CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
//CHECK: extractvalue %dx.types.CBufRet.i32
//CHECK: icmp eq i32
//CHECK: phi i32
//CHECK: ret i32

export
int loop_with_break1(int i)
{
  Istruct istruct;
  uint cascadeIndex = 10;

  for (; i>=0; --i) {
    istruct = istructs[i];
    // break conditional introduces additional complications
    if (istruct.ival) {
      cascadeIndex = i;
      break;
    }
  }

  return istruct.ival;
}

//CHECK: define i32 @"\01?loop_with_break2{{[@$?.A-Za-z0-9_]+}}"
//CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
//CHECK: extractvalue %dx.types.CBufRet.i32
//CHECK: icmp eq i32
//CHECK: phi i32
//CHECK: ret i32

export
int loop_with_break2(int i)
{
  Istruct istruct;

  for (; i>=0; --i) {
    istruct = istructs[i];
    // break conditional introduces additional complications
    if (istruct.ival) {
      break;
    }
  }

  return istruct.ival;
}

//CHECK: define i32 @"\01?uncond_loop_with_break{{[@$?.A-Za-z0-9_]+}}"
//CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
//CHECK: extractvalue %dx.types.CBufRet.i32
//CHECK: icmp eq i32
//CHECK: ret i32
export
int uncond_loop_with_break(uint i)
{
  Istruct istruct;

  for (; i>=0; --i) {
    istruct = istructs[i];
    // break conditional introduces additional complications
    if (istruct.ival)
      break;
  }

  return istruct.ival;
}

//CHECK define i32 @"\01?uncond_init_loop{{[@$?.A-Za-z0-9_]+}}"
//CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
//CHECK: extractvalue %dx.types.CBufRet.i32
//CHECK: phi i32
//CHECK: ret i32
export
int init_loop(int ct)
{
  Istruct istruct;

  for (int i = 0; i < ct; ++i)
    istruct = istructs[i];

  return istruct.ival;
}

//CHECK: define i32 @"\01?cond_if{{[@$?.A-Za-z0-9_]+}}"(i32 %i)
//CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
//CHECK: extractvalue %dx.types.CBufRet.i32
//CHECK: phi i32
//CHECK: ret i32
export
int cond_if(int i)
{
  Istruct istruct;

  if(i>=0)
    istruct = istructs[i];

  return istruct.ival;
}

//CHECK: define i32 @"\01?uncond_if{{[@$?.A-Za-z0-9_]+}}"(i32 %i)
//CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
//CHECK: extractvalue %dx.types.CBufRet.i32
//CHECK: ret i32

export
int uncond_if(uint i)
{
  Istruct istruct;

  if(i>=0)
    istruct = istructs[i];

  return istruct.ival;
}


//CHECK: define i32 @"\01?cond_if_else{{[@$?.A-Za-z0-9_]+}}"(i32 %i, i32 %j)
//CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
//CHECK: extractvalue %dx.types.CBufRet.i32
//CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
//CHECK: extractvalue %dx.types.CBufRet.i32
//CHECK: phi i32
//CHECK: ret i32

export
int cond_if_else(int i, int j)
{
  Istruct istruct;

  if(i>=0)
    istruct = istructs[i];
  else
    istruct = istructs[j];

  return istruct.ival;
}

//CHECK: define i32 @"\01?uncond_if_else{{[@$?.A-Za-z0-9_]+}}"(i32 %i, i32 %j)
//CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
//CHECK: extractvalue %dx.types.CBufRet.i32
//CHECK: ret i32
export
int uncond_if_else(uint i, int j)
{
  Istruct istruct;

  if(i>=0)
    istruct = istructs[i];
  else
    istruct = istructs[j];

  return istruct.ival;
}

//CHECK: define i32 @"\01?entry_memcpy{{[@$?.A-Za-z0-9_]+}}"(i32 %i, i32 %ct)
//CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
//CHECK: extractvalue %dx.types.CBufRet.i32
//CHECK: phi i32
//CHECK: ret i32
// This should allow the complete RAUW replacement
export
int entry_memcpy(int i, int ct)
{
  Istruct istruct;

  istruct = istructs[i];

  int ival = 0;

  for (; i < ct; ++i)
    ival += istruct.ival;

  return ival;
}

