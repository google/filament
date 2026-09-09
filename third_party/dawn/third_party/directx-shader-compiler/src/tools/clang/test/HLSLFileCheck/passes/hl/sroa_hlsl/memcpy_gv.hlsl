// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// This test contrives a way to reach a potential memcopy replacement involving a static global
// Previously, this was believed impossible so a nullptr was provided for the domtree.
// Since it is possible, this verifies that a crash doesn't occur as the null case is now handled.

// A struct, however minimal, is needed to force a memcpy
struct struct_val_t
{
    float val;
};

// A retrieval function to prevent an additional load that would prevent the problem optimization
float GetVal(struct struct_val_t sv) {
  return sv.val;
}

cbuffer cbuf : register(b0 , space2 ) { struct_val_t sv; };

// Trivial conditional initialization of the first global
// meant to keep the memcopy src GEP out of the entry block
float CondInit(float val)
{
    if (val <= 0.0)
      return 0.0;
    return val;
}

// A global initialization that makes use of memcpy
struct_val_t MemCpyInit()
{
    struct_val_t i;
    i = sv;
    return i;
}

static const float global1 = CondInit( 0.4242424242);
static const struct_val_t global2 = MemCpyInit();

// This function is embarassingly optimizable in later passes.
// This check is mostly just to verify it doesn't crash anymore
// So checks are very simple
//CHECK: define void @main
//CHECK: ret void
float main(int a : A) : SV_Target
{
  // Implicit inititalization of global1 including conditional will go here.
  // This initialization prevents memcpy source being in the entry block which makes it dominate everything
  // global1 isn't even used, but the conditionals it introduces remain
  // at the point of memcopy lowering

  // Implicit initialization of global2 which includes the memcopy operation in question will go here.

  // This conditional prevents the source from dominating all uses of the memcopy dest
  if (a)
    // This function call keeps the memcopy being the only load use of the alloca
    // at least at this stage of the passes
    return GetVal(global2);
  return 0;
}
