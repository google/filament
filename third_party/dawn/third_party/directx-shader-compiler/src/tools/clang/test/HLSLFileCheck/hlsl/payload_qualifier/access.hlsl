// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=0 | FileCheck -check-prefix=CHK0 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=1 | FileCheck -check-prefix=CHK1 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=2 | FileCheck -check-prefix=CHK2 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=3 | FileCheck -check-prefix=CHK3 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=4 | FileCheck -check-prefix=CHK4 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=5 | FileCheck -check-prefix=CHK5 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=6 | FileCheck -check-prefix=CHK6 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=7 | FileCheck -check-prefix=CHK7 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=8 | FileCheck -check-prefix=CHK8 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=9 | FileCheck -check-prefix=CHK9 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=10 | FileCheck -check-prefix=CHK10 -input-file=stderr  %s
// RUN: %dxc -T lib_6_6 main %s -enable-payload-qualifiers -D TEST_NUM=11 | FileCheck -check-prefix=CHK11 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=0 | FileCheck -check-prefix=CHK0 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=1 | FileCheck -check-prefix=CHK1 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=2 | FileCheck -check-prefix=CHK2 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=3 | FileCheck -check-prefix=CHK3 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=4 | FileCheck -check-prefix=CHK4 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=5 | FileCheck -check-prefix=CHK5 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=6 | FileCheck -check-prefix=CHK6 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=7 | FileCheck -check-prefix=CHK7 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=8 | FileCheck -check-prefix=CHK8 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=9 | FileCheck -check-prefix=CHK9 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=10 | FileCheck -check-prefix=CHK10 -input-file=stderr  %s
// RUN: %dxc -T lib_6_7 main %s -D TEST_NUM=11 | FileCheck -check-prefix=CHK11 -input-file=stderr  %s

// CHK0-NOT: -Wpayload-access-

// CHK1: warning: write will be dropped ('noWrite' is not qualified 'write' for shader stage 'closesthit')
// CHK1: warning: reading undefined value ('noRead' is not qualified 'read' for shader stage 'closesthit')

// CHK2: warning: reading undefined value ('noRead2' is not qualified 'read' for shader stage 'closesthit')
// CHK2-NOT: warning: reading undefined value ('noRead' is not qualified 'read' for shader stage 'closesthit')

// CHK3: warning: write will be dropped ('noWrite' is not qualified 'write' for shader stage 'closesthit')
// CHK3: warning: reading undefined value ('noRead' is not qualified 'read' for shader stage 'closesthit')
// CHK3: warning: reading undefined value ('noRead2' is not qualified 'read' for shader stage 'closesthit')
// CHK3: warning: write will be dropped ('noWrite3' is not qualified 'write' for shader stage 'closesthit')
// CHK3-NOT: warning: write will be dropped ('noWrite2' is not qualified 'write' for shader stage 'closesthit')
// CHK3-NOT: warning: reading undefined value ('noRead3' is not qualified 'read' for shader stage 'closesthit')
// CHK3-NOT: fooload

// CHK4: warning: write will be dropped ('noWrite' is not qualified 'write' for shader stage 'closesthit')
// CHK4: warning: reading undefined value ('noRead2' is not qualified 'read' for shader stage 'closesthit')
// CHK4: warning: reading undefined value ('noRead2' is not qualified 'read' for shader stage 'closesthit')

// CHK5: warning: field 'noWrite' is 'write' for 'caller' stage but field is never written for TraceRay call
// CHK5: warning: field 'noWrite2' is 'write' for 'caller' stage but field is never written for TraceRay call
// CHK5: warning: field 'noWrite3' is 'write' for 'caller' stage but field is never written for TraceRay call
// CHK5: warning: value will be undefined inside TraceRay ('noRead' is not qualified 'write' for 'caller')
// CHK5: warning: 'noRead' is qualified 'read' for 'caller' but the field is never read after TraceCall (possible performance issue)
// CHK5: warning: 'noRead2' is qualified 'read' for 'caller' but the field is never read after TraceCall (possible performance issue)
// CHK5: warning: 'noRead3' is qualified 'read' for 'caller' but the field is never read after TraceCall (possible performance issue)
// CHK5: warning: 'readWrite' is qualified 'read' for 'caller' but the field is never read after TraceCall (possible performance issue)
// CHK5: warning: reading undefined value ('noWrite' is returned from TraceRay but not qualified 'read' for 'caller')

// CHK6: warning: potential loss of data for payload field 'clobbered'. Field is qualified 'write' in earlier stages and 'write' only for stage 'closesthit' but never unconditionally written.

// CHK7: warning: potential loss of data for payload field 'clobbered'. Field is qualified 'write' in earlier stages and 'write' only for stage 'anyhit' but never unconditionally written.

// CHK8: warning: write will be dropped ('noWrite' is not qualified 'write' for shader stage 'closesthit')
// CHK8: warning: reading undefined value ('noRead2' is not qualified 'read' for shader stage 'closesthit')
// CHK8-NOT: warning: reading undefined value ('noRead3' is not qualified 'read' for shader stage 'closesthit')

// CHK9-NOT: warning: field 'noWrite' is 'write' for 'caller' stage but field is never written for TraceRay call
// CHK9-NOT: warning: field 'noWrite2' is 'write' for 'caller' stage but field is never written for TraceRay call
// CHK9-NOT: warning: field 'noWrite3' is 'write' for 'caller' stage but field is never written for TraceRay call

// CHK10-NOT: warning: field 'noWrite' is 'write' for 'caller' stage but field is never written for TraceRay call
// CHK10-NOT: warning: field 'noWrite2' is 'write' for 'caller' stage but field is never written for TraceRay call
// CHK10-NOT: warning: field 'noWrite3' is 'write' for 'caller' stage but field is never written for TraceRay call

// CHK11-NOT: warning: 'noRead' is qualified 'read' for 'caller' but the field is never read after TraceCall (possible performance issue)

struct [raypayload] Payload
{
    float noRead : write(closesthit) : read(caller); 
    float noRead2 : write(closesthit) : read(caller); 
    float noRead3 : write(closesthit) : read(caller); 
    float noWrite  : read(closesthit) : write(caller); 
    float noWrite2  : read(closesthit) : write(caller); 
    float noWrite3  : read(closesthit) : write(caller); 
    float readWrite : write(closesthit,  caller) : read(caller, closesthit); 
#if TEST_NUM == 6
    float clobbered : write(caller, closesthit) : read(caller);
#endif
#if TEST_NUM == 7
    float clobbered : write(anyhit, closesthit) : read(caller);
#endif
};

struct Attribs { float2 barys; };

RaytracingAccelerationStructure scene : register(t0);

// Check if no warning is produced if no access happens.
#if TEST_NUM == 0
[shader("closesthit")]
void ClosestHit0( inout Payload payload, in Attribs attribs )
{
}
#endif

// Check if we produce a warning: noRead is undefined
//                                noWrite is lost
#if TEST_NUM == 1
[shader("closesthit")]
void ClosestHit1( inout Payload payload, in Attribs attribs )
{
    payload.noWrite = payload.noRead;
}
#endif

// Check if we do not produce warnings for fields dominated by a write.
// noRead is dominated by a write and thus has a vaild value.
// noRead2 is not written and undefied because of access qualifiers.
#if TEST_NUM == 2
[shader("closesthit")]
void ClosestHit3( inout Payload payload, in Attribs attribs )
{
    payload.noRead = 24;
    if (payload.noRead)
        int x = 2 + payload.noRead + payload.noRead2; // warning, read is not dominated by write
}
#endif

// Check if we warn in a function that gets the payload as parameter.
// This should produce warnings on the access to payload but not to fooload.
// For foo we expect a warning for the read and the write.
// For foo_in we expect only a warning for the read since the payload is not copy-out.
// For foo_out we expect only a warning for the write since the payload is not copy-in.
#if TEST_NUM == 3
void foo(inout Payload fooload, inout Payload payload)
{
    if (fooload.noRead)
        payload.noWrite = payload.noRead;
}
void foo_in(inout Payload fooload, in Payload payload)
{
    if (fooload.noRead)
        payload.noWrite2 = payload.noRead2;
}

void foo_out(inout Payload fooload, out Payload payload)
{
    if (fooload.noRead)
        payload.noWrite3 = payload.noRead3;
}

[shader("closesthit")]
void ClosestHit4( inout Payload payload, in Attribs attribs )
{
    Payload fooload;
    if (payload.readWrite)
        foo(fooload, payload);
}

[shader("closesthit")]
void ClosestHit5( inout Payload payload, in Attribs attribs )
{
    Payload fooload;
    if (payload.readWrite)
        foo_in(fooload, payload);
}

[shader("closesthit")]
void ClosestHit6( inout Payload payload, in Attribs attribs )
{
    Payload fooload;
    if (payload.readWrite)
        foo_out(fooload, payload);
}
#endif

// Check if we don't crash if we have to handle loops and recursion.
#if TEST_NUM == 4
void bar(inout Payload payload)
{
    payload.noWrite = payload.noRead;
    bar(payload);
    payload.noWrite = payload.noRead2;
}


[shader("closesthit")]
void ClosestHit8( inout Payload payload, in Attribs attribs )
{
    payload.noRead = 1;
    bar(payload);
    for (int i = 0; i < payload.noRead2; ++i)
    payload.noWrite = 2;
}
#endif

// Test if we produce warnings for TraceRay calls. 
// In the following the noWrite fields which are write for 'caller' are not written (warn).
// The noRead field is written to in the caller but is not qualified 'write' warn about lost input.
// Several fields are marked as 'read' for 'caller' but never read. Warn about potential perf issue.
// The noWrite field is used after the trace call but the field is not qualified 'read' for 'caller', 
// the value will be dropped and the read value is undefined (warn).
#if TEST_NUM == 5
[shader("raygeneration")]
void RayGen1()
{
    Payload payload;
    payload.readWrite = 0;
    payload.noRead = 0;
    RayDesc ray;
    TraceRay( scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload );

    if (payload.noWrite)
    {
        payload.noWrite = 23;
    }
    payload.noWrite = 32;
}
#endif

// Check if we produce a warning for a shader that has write access but does not write and 
// clobbers a field written by an earlier shader stage.
#if TEST_NUM == 6
[shader("closesthit")]
void ClosestHit0( inout Payload payload, in Attribs attribs )
{
}
#endif

// Check if we produce a warning for a shader that has write access but does not write and 
// clobbers a field written by an earlier shader stage. Here we consider anyhit an earlier stage
// for anyhit.
#if TEST_NUM == 7
[shader("anyhit")]
void Anyhit0( inout Payload payload, in Attribs attribs  )
{
}
#endif

// Check if a write in a function slience the warning about an undef read in the caller.
#if TEST_NUM == 8
void bar(inout Payload payload)
{
    payload.noWrite = payload.noRead;
    bar(payload);
    payload.noWrite = payload.noRead2;
    payload.noRead3 = 5;
}


[shader("closesthit")]
void ClosestHit8( inout Payload payload, in Attribs attribs )
{
    payload.noRead = 1;
    bar(payload);
    for (int i = 0; i < payload.noRead3; ++i)
    payload.noWrite = 2;
}
#endif

// Check if initializer lists are handle correctly.
#if TEST_NUM == 9
[shader("raygeneration")]
void RayGen2()
{
    Payload payload = {0,0,0,0,0,0,0};
    RayDesc ray;
    TraceRay( scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload );
}
#endif

// Check if copy initialization is handles correctly.
#if TEST_NUM == 10
[shader("raygeneration")]
void RayGen3()
{
    Payload fooload = {0,0,0,0,0,0,0};
    Payload payload = fooload;
    RayDesc ray;
    TraceRay( scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload );
}
#endif

// Check if  CompoundAssignOperator Stmts are handled correctly while
// searching for reads. 
#if TEST_NUM == 11
[shader("raygeneration")]
void RayGen4())
{
    Payload payload;
    TraceRay( scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload );

    int a = 0; 
    a += payload.noRead;
}
#endif