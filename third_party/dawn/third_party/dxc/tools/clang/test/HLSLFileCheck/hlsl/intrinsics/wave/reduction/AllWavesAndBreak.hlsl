// RUN: %dxc -T ps_6_5 %s | FileCheck %s
StructuredBuffer<int> buf[]: register(t2);
StructuredBuffer<uint4> g_mask;

// CHECK: @dx.break.cond = internal constant

// Cannonical example. Expected to keep the block in loop
// Verify this function loads the global
// CHECK: load i32
// CHECK-SAME: @dx.break.cond
// CHECK: icmp eq i32

int main(int a : A, int b : B) : SV_Target
{
  int res = 0;
  int i = WaveGetLaneCount() + WaveGetLaneIndex();

  // These verify the break block keeps the conditional
  // CHECK: call i1 @dx.op.waveIsFirstLane
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {
    bool u = WaveIsFirstLane();
    if (a != u) {
      res += buf[b][(int)u];
      break;
    }
  }

  // These verify the break block keeps the conditional
  // CHECK: call i1 @dx.op.waveAnyTrue
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {
    bool u = WaveActiveAnyTrue(a);
    if (a != u) {
      res += buf[(int)u][b];
      break;
    }
  }

  // These verify the break block keeps the conditional
  // CHECK: call i1 @dx.op.waveAllTrue
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {
    bool u = WaveActiveAllTrue(a);
    if (a != u) {
      res += buf[(int)u][b];
      break;
    }
  }

  // These verify the break block keeps the conditional
  // CHECK: call i1 @dx.op.waveActiveAllEqual
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {
    bool u = WaveActiveAllEqual(a);
    if (a != u) {
      res += buf[(int)u][b];
      break;
    }
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveReadLaneFirst
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveReadLaneFirst(a);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveAllOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    uint u = WaveActiveCountBits(a == b);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveActiveOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveActiveSum(a);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveActiveOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveActiveProduct(a);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveActiveBit
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveActiveBitAnd((uint)a);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveActiveBit
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveActiveBitOr((uint)a);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveActiveBit
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveActiveBitXor((uint)a);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveActiveOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveActiveMin(a);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveActiveOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveActiveMax(a);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.wavePrefixOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    uint u = WavePrefixCountBits(a < b);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.wavePrefixOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    uint u = WavePrefixSum(a);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.wavePrefixOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    uint u = WavePrefixProduct(a);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call %dx.types.fouri32 @dx.op.waveMatch
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    uint4 u = WaveMatch(a);
    if (a != u.y) {
      res += buf[u.x][b];
      break;
    }       
  }

  uint4 mask = g_mask[0];
  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveMultiPrefixOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveMultiPrefixBitAnd((uint)a, mask);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveMultiPrefixOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveMultiPrefixBitOr((uint)a, mask);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveMultiPrefixOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    int u = WaveMultiPrefixBitXor((uint)a, mask);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveMultiPrefixBitCount
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    uint u = WaveMultiPrefixCountBits(a <= b, mask);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveMultiPrefixOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    uint u = WaveMultiPrefixProduct(a, mask);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  // These verify the break block keeps the conditional
  // CHECK: call i32 @dx.op.waveMultiPrefixOp
  // CHECK: call %dx.types.Handle @dx.op.createHandle
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
  // CHECK: add
  // CHECK: br i1
  // Loop with wave-dependent conditional break block
  for (;;) {   
    uint u = WaveMultiPrefixSum(a, mask);
    if (a != u) {
      res += buf[u][b];
      break;
    }       
  }

  return res;
}
