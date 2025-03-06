// RUN: %dxc -T lib_6_6 %s  | FileCheck %s
// RUN: %dxc -disable-lifetime-markers -T lib_6_6 %s  | FileCheck %s -check-prefix=NOLIFE

// NOLIFE-NOT: @llvm.lifetime

//
// Non-SSA arrays should have lifetimes within the correct scope.
//
// CHECK: define i32 @"\01?if_scoped_array{{[@$?.A-Za-z0-9_]+}}"
// CHECK: alloca
// CHECK: icmp
// CHECK: br i1
// CHECK: call void @llvm.lifetime.start
// CHECK: br label
// CHECK: load i32
// CHECK: call void @llvm.lifetime.end
// CHECK: br label
// CHECK: phi i32
// CHECK: load i32
// CHECK: store i32
// CHECK: br i1
// CHECK: phi i32
// CHECK: ret i32
export
int if_scoped_array(int n, int c)
{
  int res = c;

  if (n > 0) {
    int arr[200];

    // Fake some dynamic initialization so the array can't be optimzed away.
    for (int i = 0; i < n; ++i) {
        arr[i] = arr[c - i];
    }

    res = arr[c];
  }

  return res;
}

//
// Escaping structs should have lifetimes within the correct scope.
//
// CHECK: define void @"\01?loop_scoped_escaping_struct{{[@$?.A-Za-z0-9_]+}}"(i32 %n)
// CHECK: %[[alloca:.*]] = alloca %struct.MyStruct
// CHECK: ret
// CHECK: phi i32
// CHECK-NEXT: bitcast
// CHECK-NEXT: call void @llvm.lifetime.start
// CHECK-NEXT: call float @"\01?func{{[@$?.A-Za-z0-9_]+}}"(%struct.MyStruct* nonnull %[[alloca]])
// CHECK-NEXT: call void @llvm.lifetime.end
// CHECK: br i1
struct MyStruct {
  float x;
};

float func(MyStruct data);

export
void loop_scoped_escaping_struct(int n)
{
  for (int i = 0; i < n; ++i) {
    MyStruct data;
    func(data);
  }
}

//
// Loop-scoped structs that are passed as inout should have lifetimes
// within the correct scope and should not produce values live across multiple
// loop iterations (=no loop phi nodes).
//
// CHECK: define i32 @"\01?loop_scoped_escaping_struct_write{{[@$?.A-Za-z0-9_]+}}"(i32 %n)
// CHECK: %[[alloca:.*]] = alloca %struct.MyStruct
// CHECK: br i1
// CHECK: phi i32
// CHECK-NEXT: ret
// CHECK: phi i32
// CHECK-NEXT: phi i32
// CHECK-NOT: phi float
// CHECK-NEXT: bitcast
// CHECK-NEXT: call void @llvm.lifetime.start
// CHECK-NOT: store
// CHECK-NEXT: call void @"\01?func2{{[@$?.A-Za-z0-9_]+}}"(%struct.MyStruct* nonnull %[[alloca]])
// CHECK-NEXT: getelementptr
// CHECK-NEXT: load
// CHECK: call void @llvm.lifetime.end
// CHECK: br i1
void func2(inout MyStruct data);

export
int loop_scoped_escaping_struct_write(int n)
{
  int res = 0;
  for (int i = 0; i < n; ++i) {
    MyStruct data;
    func2(data);
    res += data.x;
  }
  return res;
}

//
// Loop-scoped structs that can be promoted to registers should not produce
// values considered live across multiple loop iterations (= no loop phi nodes).
//
// Make sure there is only one loop phi node, which is the induction var.
// CHECK: define i32 @"\01?loop_scoped_struct_conditional_init{{[@$?.A-Za-z0-9_]+}}"(i32 %n, i32 %c1)
// CHECK-NOT: alloca
// CHECK: phi i32
// CHECK-NEXT: ret i32
// CHECK-NOT: phi float
// CHECK: phi i32
// CHECK-NOT: phi float
// CHECK-NEXT: call void @"\01?expensiveComputation
// CHECK-NEXT: icmp
// CHECK-NEXT: br i1
// CHECK: dx.op.rawBufferLoad
// CHECK: phi float
RWStructuredBuffer<MyStruct> g_rwbuf : register(u0);

void expensiveComputation();

export
int loop_scoped_struct_conditional_init(int n, int c1)
{
  int res = n;

  for (int i = 0; i < n; ++i) {
    expensiveComputation(); // s mut not be considered live here.

    MyStruct s;

    // Initialize struct conditionally.
    // NOTE: If some optimization decides to flatten the if statement or if the
    //       computation could be hoisted out of the loop, the phi with undef
    //       below will be replaced by the non-undef value (which is a valid
    //       "specialization" of undef).
    if (c1 < 0)
      s.x = g_rwbuf[i - c1].x;

    res = s.x; // i or undef.
  }

  return res; // Result is n if loop wasn't executed, n-1 if it was.
}

//
// Another real-world use-case for loop-scoped structs that can be promoted
// to registers. Again, this should not produce values that are live across
// multiple loop iterations (= no loop phi nodes).
// Both the consume and produce calls must be inlined, otherwise the alloca
// can't be promoted.
//
// CHECK: define i32 @"\01?loop_scoped_struct_conditional_assign_from_func_output{{[@$?.A-Za-z0-9_]+}}"(i32 %n, i32 %c1)
// CHECK-NOT: alloca
// CHECK: phi i32
// CHECK-NOT: phi i32
// CHECK-NOT: phi float
void consume(int i, in MyStruct data)
{
  // This must be inlined, otherwise the alloca can't be promoted.
  g_rwbuf[i] = data;
}

bool produce(in int c, out MyStruct data)
{
  if (c > 0) {
    MyStruct s;
    s.x = 13;
    data = s; // <-- Conditional assignment of out-qualified parameter.
    return true;
  }
  return false; // <-- Out-qualified parameter left uninitialized.
}

export
int loop_scoped_struct_conditional_assign_from_func_output(int n, int c1)
{
  for (int i=0; i<n; ++i) {
    MyStruct data;
    bool valid = produce(c1, data); // <-- Without lifetimes, inlining this generates a loop phi using prior iteration's value.
    if (valid)
      consume(i, data);
    expensiveComputation(); // <-- Said phi is alive here, inflating register pressure.
  }
  return n;
}

//
// Global constants should have lifetimes.
// The constant array should be hoisted to a constant global.
//
// CHECK: define i32 @"\01?global_constant{{[@$?.A-Za-z0-9_]+}}"(i32 %n)
// CHECK: call void @llvm.lifetime.start
// CHECK: load i32
// CHECK: call void @llvm.lifetime.end
// CHECK: ret i32
int compute(int i)
{
  int arr[] = {0, 1, 2, 3, 4, 5, -1, 13};
  return arr[i % 8];
}

export
int global_constant(int n)
{
  return compute(n);
}

//
// Global constants should have lifetimes within the correct scope.
// The constant array should be hoisted to a constant global with lifetime
// only inside the loop.
//
// CHECK: define i32 @"\01?global_constant2{{[@$?.A-Za-z0-9_]+}}"(i32 %n)
// CHECK: phi i32
// CHECK: phi i32
// CHECK: call void @llvm.lifetime.start
// CHECK: load i32
// CHECK: call void @llvm.lifetime.end
export
int global_constant2(int n)
{
  int res = 0;
  for (int i = 0; i < n; ++i)
    res += compute(i);
  return res;
}
