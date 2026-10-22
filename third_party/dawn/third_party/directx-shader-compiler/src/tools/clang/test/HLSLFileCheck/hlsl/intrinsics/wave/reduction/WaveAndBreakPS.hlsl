// RUN: %dxc -T ps_6_3 %s | FileCheck %s

// Rather than trying to account for all optimizations, 
// give each function that's going to be inlined its own buffer
StructuredBuffer<int> mainBuf[]: register(t2, space0);
StructuredBuffer<int> loopBuf[]: register(t3, space1);
StructuredBuffer<int> postBuf[]: register(t4, space2);
StructuredBuffer<int> breakBuf[]: register(t5, space3);
StructuredBuffer<int> entryBuf[]: register(t6, space4);
StructuredBuffer<int> subBuf[]: register(t7, space5);
StructuredBuffer<int> otherBuf[]: register(t8, space6);

int WaveInLoop(int a : A, int b : B);
int WaveInPostLoop(int a : A, int b : B);
int WaveInBreakBlock(int a : A, int b : B);
int WaveInEntry(int a : A, int b : B);
int WaveInSubLoop(int a : A, int b : B);
int WaveInOtherLoop(int a : A, int b : B, int c : C);
int MultiWaveInMultiLoops(int a : A, int b : B, int c : C, uint d : D);

// CHECK: @dx.break.cond = internal constant

// Verify this function loads the global
// CHECK: load i32
// CHECK-SAME: @dx.break.cond
// CHECK-NEXT: icmp eq i32

// CHECK: call i32 @dx.op.waveReadLaneFirst

// These verify the first break block keeps the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %mainBuf
// CHECK: add
// CHECK: br i1
// These verify the second break block doesn't
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %mainBuf

int main(int a : A, int b : B, int c : C, int d : D) : SV_Target
{
  int res = 0;
  int i = 0;
  int u = 0;

  for (;;) {
      u += WaveReadLaneFirst(a);
      if (a == u) {
          res += mainBuf[u][b];
          break;
        }
    }
  for (;;) {
      if (a == u) {
          res += mainBuf[b][u];
          break;
        }
    }
  return res + WaveInPostLoop(a, b) + WaveInBreakBlock(a, b) + WaveInEntry(a, b) +
    WaveInSubLoop(a,b) + WaveInOtherLoop(a,b,c) + MultiWaveInMultiLoops(a,b,c,d);
}

// Wave moved to after the break block. Expected to keep the block in loop

// These verify the break block keeps the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %postBuf
// CHECK: add
// CHECK: br i1

// CHECK: call i32 @dx.op.waveReadLaneFirst

// These verify the break block keeps the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %postBuf
// CHECK: add
// CHECK: br i1

// CHECK: call i32 @dx.op.waveReadLaneFirst
int WaveInPostLoop(int a : A, int b : B)
{
  int res = 0;
  int i = 0;
  int u = 0;

  // Loop with wave-dependent conditional break block
  for (;;) {
      if (a == u) {
          res += postBuf[b][u];
          break;
        }
      u += WaveReadLaneFirst(a);
    }

  // Loop with wave-independent conditional break block
  for (;;) {
      if (b == i) {
          res += postBuf[u][b];
          break;
        }
      u += WaveReadLaneFirst(a);
      i++;
    }
  return res;
}

// Wave op inside break block. Expected to keep the block in loop

// CHECK: call i32 @dx.op.waveReadLaneFirst

// These verify the break block keeps the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %breakBuf
// CHECK: br i1

int WaveInBreakBlock(int a : A, int b : B)
{
  int res = 0;
  int i = 0;

  // Loop with wave-independent conditional break block
  for (;;) {
      if (b == i) {
          int u = WaveReadLaneFirst(a);
          res = breakBuf[b][u];
          break;
        }
      i++;
    }
  return res;
}

// Wave in entry block. Expected to allow the break block to move out of loop
// CHECK: call i32 @dx.op.waveReadLaneFirst

// These verify the break block doesn't keep the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %entryBuf

// These verify the break block doesn't keep the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %entryBuf
int WaveInEntry(int a : A, int b : B)
{
  int res = 0;
  int i = 0;

  int u = WaveReadLaneFirst(b);

  // Loop with wave-dependent conditional break block
  for (;;) {
      if (a == u) {
          res += entryBuf[b][u];
          break;
        }
    }

  // Loop with wave-independent conditional break block
  for (;;) {
      if (b == i) {
          res += entryBuf[u][b];
          break;
        }
      i++;
    }
  return res;
}

// Wave in subloop of larger loop. Expected to keep the block in loop
// CHECK: call i32 @dx.op.waveReadLaneFirst

// These verify the break block keeps the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %subBuf
// CHECK: add
// CHECK: br i1

// These verify the break block keeps the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %subBuf
// CHECK: add
// CHECK: br i1
int WaveInSubLoop(int a : A, int b : B)
{
  int res = 0;
  int i = 0;

  // Loop with wave-dependent conditional break block
  for (;;) {
      int u = 0;
      for (int i = 0; i < b; i ++)
        u += WaveReadLaneFirst(a);
      if (a == u) {
          res += subBuf[a][u];
          break;
        }
    }

  // Loop with wave-independent conditional break block
  for (;;) {
      int u = 0;
      for (int j = 0; j < b; j ++)
        u += WaveReadLaneFirst(a);
      if (b == i) {
          res += subBuf[b][u];
          break;
        }
      i++;
    }
  return res;
}

// Wave in a separate loop. Expected to allow the break block to move out of loop
// CHECK: call i32 @dx.op.waveReadLaneFirst

// These verify the first break block keeps the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %otherBuf
// CHECK: add
// CHECK: br i1

// These verify the second break block doesn't
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %otherBuf

// These verify the third break block doesn't
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %otherBuf
int WaveInOtherLoop(int a : A, int b : B, int c : C)
{
  int res = 0;
  int i = 0;
  int u = 0;

  for (;;) {
      u += WaveReadLaneFirst(a);
      if (a == u) {
          res += otherBuf[u][b];
          break;
        }
    }

  // Loop with wave-dependent conditional break block
  for (;;) {
      if (a == u) {
          res += otherBuf[b][u];
          break;
        }
    }

  // Loop with wave-independent conditional break block
  for (;;) {
      if (b == i) {
          res += otherBuf[c][u];
          break;
        }
      i++;
    }
  return res;
}

// Complicated case where multiple waves are in multiple loops overlapping and not

// Position all the wave ops
// CHECK: call i32 @dx.op.waveReadLaneFirst
// CHECK: call i32 @dx.op.waveActiveOp
// CHECK: call i32 @dx.op.waveActiveOp
// CHECK: call i32 @dx.op.waveActiveBit

// These verify the first four break blocks keep the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %mainBuf
// CHECK: add
// CHECK: br i1
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %mainBuf
// CHECK: add
// CHECK: br i1
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %mainBuf
// CHECK: add
// CHECK: br i1
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %mainBuf
// CHECK: add
// CHECK: br i1

// Repeat for second loop

// Position all the wave ops
// CHECK: call i32 @dx.op.waveReadLaneFirst
// CHECK: call i32 @dx.op.waveActiveOp
// CHECK: call i32 @dx.op.waveActiveOp
// CHECK: call i32 @dx.op.waveActiveBit

// These verify the last four break blocks keep the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %mainBuf
// CHECK: add
// CHECK: br i1
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %mainBuf
// CHECK: add
// CHECK: br i1
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %mainBuf
// CHECK: add
// CHECK: br i1
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK-SAME: %mainBuf
// CHECK: add
// CHECK: br i1


int MultiWaveInMultiLoops(int a : A, int b : B, int c : C, uint d : D)
{
  int res = 0;
  int u = 0;
  int v = 0;
  int w = 0;
  int x = 0;

  for (;;) {
    u += WaveReadLaneFirst(a);
    v += WaveActiveSum(b);
    for (int i = 0; i < c; i++) {
      w += WaveActiveProduct(c);
      x += WaveActiveBitAnd(d);
    }
    if (a == u + b) {
      res += mainBuf[u + c][b];
      break;
    }
    if (a == v + b) {
      res += mainBuf[v + c][b];
      break;
    }
    if (a == w + b) {
      res += mainBuf[w + c][b];
      break;
    }
    if (a == x + b) {
      res += mainBuf[x + c][b];
      break;
    }
  }

  u = 0;
  v = 0;
  w = 0;
  x = 0;

  for (;;) {
    u += WaveReadLaneFirst(a);
    v += WaveActiveSum(b);
    for (int i = 0; i < c; i++) {
      w += WaveActiveProduct(c);
      x += WaveActiveBitAnd(d);
    }
    if (a == b + u) {
      res += mainBuf[c + u][b];
      break;
    }
    if (a == b + v) {
      res += mainBuf[c + v][b];
      break;
    }
    if (a == b + w) {
      res += mainBuf[c + w][b];
      break;
    }
    if (a == b + x) {
      res += mainBuf[c + x][b];
      break;
    }
  }


  return res;
}

// Final operations
// CHECK-NOT: br i1
// CHECK: call void @dx.op.storeOutput
