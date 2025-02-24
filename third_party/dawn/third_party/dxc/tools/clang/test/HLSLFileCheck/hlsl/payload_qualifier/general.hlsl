// RUN: %dxc -T lib_6_5 -D TEST_NUM=4 %s -enable-payload-qualifiers | FileCheck -input-file=stderr -check-prefix=CHK4 %s
// RUN: %dxc -T lib_6_6 -D TEST_NUM=4 %s | FileCheck -input-file=stderr -check-prefix=CHK5 %s
// RUN: %dxc -T lib_6_6 -D TEST_NUM=4 %s -enable-payload-qualifiers | FileCheck -check-prefix=CHK6 %s
// RUN: %dxc -T lib_6_6 -D TEST_NUM=5 %s -enable-payload-qualifiers | FileCheck -check-prefix=CHK7 %s

// check if we get DXIL and the payload type is there 
// CHK4: Invalid target for payload access qualifiers. Only lib_6_6 and beyond are supported.
// CHK5: warning: payload access qualifiers ignored. These are only supported for lib_6_7+ targets and lib_6_6 with with the -enable-payload-qualifiers flag.
// CHK5: struct [raypayload] Payload {
// CHK5-NOT: struct [raypayload] OtherPayload {
// CHK6: %struct.Payload = type { i32, i32 }

// CHK7: error: type 'Payload' used as payload requires that it is annotated with the {{\[[a-z]*\]}} attribute

#if TEST_NUM <= 4
struct [raypayload] Payload {
    int a : read(closesthit) : write(caller);
    int b : write(closesthit) : read(caller);
};
struct [raypayload] OtherPayload {
    int a : read(closesthit) : write(caller);
    int b : write(closesthit) : read(caller);
};
#else 
struct Payload {
    int a;
    int b : read (caller) : write(closesthit);
};
#endif

// test if compilation fails because not all payload filds are qualified for lib_6_6
// test if compilation succeeds for lib_6_5 where payload access qualifiers are not required
#if TEST_NUM == 4
[shader("miss")]
void Miss4(inout Payload payload){
}
#endif

#if TEST_NUM == 5
[shader("miss")]
void Miss5(inout Payload payload){
    payload.b = 42;
}
#endif
