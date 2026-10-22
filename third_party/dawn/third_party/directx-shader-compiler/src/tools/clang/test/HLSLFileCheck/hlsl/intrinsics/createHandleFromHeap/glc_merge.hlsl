// RUN: %dxc -T cs_6_6 %s | %FileCheck %s

// Make sure only 1 annotate handle and mark glc.
// CHECK:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 20491, i32 0 })
// CHECK-NOT:call %dx.types.Handle @dx.op.annotateHandle(

globallycoherent RWByteAddressBuffer Buffer0;

void Foo(RWByteAddressBuffer Buffer1)
{
    Buffer1.Store(0, 0);	// ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
}

void Bar(globallycoherent RWByteAddressBuffer Buffer1)
{
    Buffer1.Store(0, 0);	// ; AnnotateHandle(res,props)  resource: globallycoherent RWByteAddressBuffer
}

[numthreads(1, 1, 1)]
void main()
{
	Buffer0.Store(0, 0);	// ; AnnotateHandle(res,props)  resource: globallycoherent RWByteAddressBuffer
    Foo(Buffer0);
    Bar(Buffer0);

    RWByteAddressBuffer Buffer1 = Buffer0;
    Buffer1.Store(0, 0);	// ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
}
