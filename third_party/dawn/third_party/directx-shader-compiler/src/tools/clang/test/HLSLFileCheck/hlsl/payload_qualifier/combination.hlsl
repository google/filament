// RUN: %dxc -T lib_6_6 %s -enable-payload-qualifiers | FileCheck %s
// RUN: %dxc -T lib_6_6 %s -enable-payload-qualifiers -HV 2021 -DTEMPLATES | FileCheck %s

// CHECK: error: field 'x1' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x2' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x3' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x3' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x4' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x5' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x5' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x6' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x6' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x7' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x7' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x7' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x8' is qualified 'read' for shader stage 'caller' but has no valid producer
// CHECK: error: field 'x9' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x9' is qualified 'read' for shader stage 'caller' but has no valid producer
// CHECK: error: field 'x10' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x10' is qualified 'read' for shader stage 'caller' but has no valid producer
// CHECK: error: field 'x11' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x11' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x11' is qualified 'read' for shader stage 'caller' but has no valid producer
// CHECK: error: field 'x12' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x12' is qualified 'read' for shader stage 'caller' but has no valid producer
// CHECK: error: field 'x13' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x13' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x13' is qualified 'read' for shader stage 'caller' but has no valid producer
// CHECK: error: field 'x14' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x14' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x14' is qualified 'read' for shader stage 'caller' but has no valid producer
// CHECK: error: field 'x15' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x15' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x15' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x15' is qualified 'read' for shader stage 'caller' but has no valid producer
// CHECK: error: field 'x16' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x17' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x17' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x18' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x18' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x19' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x19' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x19' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x20' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x20' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x21' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x21' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x21' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x22' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x22' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x22' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x23' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x23' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x23' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x23' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x25' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x26' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x27' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x27' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x28' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x29' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x29' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x30' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x30' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x31' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x31' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x31' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x32' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x33' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x33' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x34' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x34' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x35' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x35' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x35' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x36' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x36' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x37' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x37' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x37' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x38' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x38' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x38' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x39' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x39' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x39' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x39' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x41' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x42' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x43' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x43' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x44' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x45' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x45' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x46' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x46' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x47' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x47' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x47' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x48' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x48' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x49' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x49' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x49' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x50' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x50' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x50' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x51' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x51' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x51' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x51' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x52' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x52' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x52' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x53' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x53' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x53' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x53' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x54' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x54' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x54' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x54' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x55' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x55' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x55' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x55' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x55' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x57' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x58' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x59' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x59' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x60' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x61' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x61' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x62' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x62' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x63' is qualified 'read' for shader stage 'miss' but has no valid producer
// CHECK: error: field 'x63' is qualified 'read' for shader stage 'closesthit' but has no valid producer
// CHECK: error: field 'x63' is qualified 'read' for shader stage 'anyhit' but has no valid producer
// CHECK: error: field 'x64' is qualified 'write' for shader stage 'anyhit' but has no valid consumer
// CHECK: error: field 'x80' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x80' is qualified 'write' for shader stage 'anyhit' but has no valid consumer
// CHECK: error: field 'x81' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x82' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x83' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x84' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x85' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x86' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x87' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x96' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x96' is qualified 'write' for shader stage 'anyhit' but has no valid consumer
// CHECK: error: field 'x97' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x98' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x99' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x100' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x101' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x102' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x103' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x112' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x112' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x112' is qualified 'write' for shader stage 'anyhit' but has no valid consumer
// CHECK: error: field 'x113' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x113' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x114' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x114' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x115' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x115' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x116' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x116' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x117' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x117' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x118' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x118' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x119' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x119' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x128' is qualified 'write' for shader stage 'caller' but has no valid consumer
// CHECK: error: field 'x144' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x144' is qualified 'write' for shader stage 'caller' but has no valid consumer
// CHECK: error: field 'x145' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x146' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x147' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x148' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x149' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x150' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x151' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x160' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x160' is qualified 'write' for shader stage 'caller' but has no valid consumer
// CHECK: error: field 'x161' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x162' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x163' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x164' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x165' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x166' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x167' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x176' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x176' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x176' is qualified 'write' for shader stage 'caller' but has no valid consumer
// CHECK: error: field 'x177' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x177' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x178' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x178' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x179' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x179' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x180' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x180' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x181' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x181' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x182' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x182' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x183' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x183' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x192' is qualified 'write' for shader stage 'anyhit' but has no valid consumer
// CHECK: error: field 'x192' is qualified 'write' for shader stage 'caller' but has no valid consumer
// CHECK: error: field 'x208' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x208' is qualified 'write' for shader stage 'anyhit' but has no valid consumer
// CHECK: error: field 'x208' is qualified 'write' for shader stage 'caller' but has no valid consumer
// CHECK: error: field 'x209' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x210' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x211' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x212' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x213' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x214' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x215' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x224' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x224' is qualified 'write' for shader stage 'anyhit' but has no valid consumer
// CHECK: error: field 'x224' is qualified 'write' for shader stage 'caller' but has no valid consumer
// CHECK: error: field 'x225' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x226' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x227' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x228' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x229' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x230' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x231' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x240' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x240' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x240' is qualified 'write' for shader stage 'anyhit' but has no valid consumer
// CHECK: error: field 'x240' is qualified 'write' for shader stage 'caller' but has no valid consumer
// CHECK: error: field 'x241' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x241' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x242' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x242' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x243' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x243' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x244' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x244' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x245' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x245' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x246' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x246' is qualified 'write' for shader stage 'closesthit' but has no valid consumer
// CHECK: error: field 'x247' is qualified 'write' for shader stage 'miss' but has no valid consumer
// CHECK: error: field 'x247' is qualified 'write' for shader stage 'closesthit'
// but has no valid consumer
#ifdef TEMPLATES
template<typename I>
#else
#define I int
#endif
struct [payload] MyPayload {
I x0 : read();
int x1 : read(miss);
int x2 : read(closesthit);
int x3 : read(miss, closesthit);
int x4 : read(anyhit);
int x5 : read(miss, anyhit);
int x6 : read(closesthit, anyhit);
int x7 : read(miss, closesthit, anyhit);
int x8 : read(caller);
int x9 : read(miss, caller);
int x10 : read(closesthit, caller);
int x11 : read(miss, closesthit, caller);
int x12 : read(anyhit, caller);
int x13 : read(miss, anyhit, caller);
int x14 : read(closesthit, anyhit, caller);
int x15 : read(miss, closesthit, anyhit, caller);
int x16 : write(miss);
int x17 : write(miss) : read(miss);
int x18 : write(miss) : read(closesthit);
int x19 : write(miss) : read(miss, closesthit);
int x20 : write(miss) : read(anyhit);
int x21 : write(miss) : read(miss, anyhit);
int x22 : write(miss) : read(closesthit, anyhit);
int x23 : write(miss) : read(miss, closesthit, anyhit);
int x24 : write(miss) : read(caller);
int x25 : write(miss) : read(miss, caller);
int x26 : write(miss) : read(closesthit, caller);
int x27 : write(miss) : read(miss, closesthit, caller);
int x28 : write(miss) : read(anyhit, caller);
int x29 : write(miss) : read(miss, anyhit, caller);
int x30 : write(miss) : read(closesthit, anyhit, caller);
int x31 : write(miss) : read(miss, closesthit, anyhit, caller);
int x32 : write(closesthit);
int x33 : write(closesthit) : read(miss);
int x34 : write(closesthit) : read(closesthit);
int x35 : write(closesthit) : read(miss, closesthit);
int x36 : write(closesthit) : read(anyhit);
int x37 : write(closesthit) : read(miss, anyhit);
int x38 : write(closesthit) : read(closesthit, anyhit);
int x39 : write(closesthit) : read(miss, closesthit, anyhit);
int x40 : write(closesthit) : read(caller);
int x41 : write(closesthit) : read(miss, caller);
int x42 : write(closesthit) : read(closesthit, caller);
int x43 : write(closesthit) : read(miss, closesthit, caller);
int x44 : write(closesthit) : read(anyhit, caller);
int x45 : write(closesthit) : read(miss, anyhit, caller);
int x46 : write(closesthit) : read(closesthit, anyhit, caller);
int x47 : write(closesthit) : read(miss, closesthit, anyhit, caller);
int x48 : write(miss, closesthit);
int x49 : write(miss, closesthit) : read(miss);
int x50 : write(miss, closesthit) : read(closesthit);
int x51 : write(miss, closesthit) : read(miss, closesthit);
int x52 : write(miss, closesthit) : read(anyhit);
int x53 : write(miss, closesthit) : read(miss, anyhit);
int x54 : write(miss, closesthit) : read(closesthit, anyhit);
int x55 : write(miss, closesthit) : read(miss, closesthit, anyhit);
int x56 : write(miss, closesthit) : read(caller);
int x57 : write(miss, closesthit) : read(miss, caller);
int x58 : write(miss, closesthit) : read(closesthit, caller);
int x59 : write(miss, closesthit) : read(miss, closesthit, caller);
int x60 : write(miss, closesthit) : read(anyhit, caller);
int x61 : write(miss, closesthit) : read(miss, anyhit, caller);
int x62 : write(miss, closesthit) : read(closesthit, anyhit, caller);
int x63 : write(miss, closesthit) : read(miss, closesthit, anyhit, caller);
int x64 : write(anyhit);
int x65 : write(anyhit) : read(miss);
int x66 : write(anyhit) : read(closesthit);
int x67 : write(anyhit) : read(miss, closesthit);
int x68 : write(anyhit) : read(anyhit);
int x69 : write(anyhit) : read(miss, anyhit);
int x70 : write(anyhit) : read(closesthit, anyhit);
int x71 : write(anyhit) : read(miss, closesthit, anyhit);
int x72 : write(anyhit) : read(caller);
int x73 : write(anyhit) : read(miss, caller);
int x74 : write(anyhit) : read(closesthit, caller);
int x75 : write(anyhit) : read(miss, closesthit, caller);
int x76 : write(anyhit) : read(anyhit, caller);
int x77 : write(anyhit) : read(miss, anyhit, caller);
int x78 : write(anyhit) : read(closesthit, anyhit, caller);
int x79 : write(anyhit) : read(miss, closesthit, anyhit, caller);
int x80 : write(miss, anyhit);
int x81 : write(miss, anyhit) : read(miss);
int x82 : write(miss, anyhit) : read(closesthit);
int x83 : write(miss, anyhit) : read(miss, closesthit);
int x84 : write(miss, anyhit) : read(anyhit);
int x85 : write(miss, anyhit) : read(miss, anyhit);
int x86 : write(miss, anyhit) : read(closesthit, anyhit);
int x87 : write(miss, anyhit) : read(miss, closesthit, anyhit);
int x88 : write(miss, anyhit) : read(caller);
int x89 : write(miss, anyhit) : read(miss, caller);
int x90 : write(miss, anyhit) : read(closesthit, caller);
int x91 : write(miss, anyhit) : read(miss, closesthit, caller);
int x92 : write(miss, anyhit) : read(anyhit, caller);
int x93 : write(miss, anyhit) : read(miss, anyhit, caller);
int x94 : write(miss, anyhit) : read(closesthit, anyhit, caller);
int x95 : write(miss, anyhit) : read(miss, closesthit, anyhit, caller);
int x96 : write(closesthit, anyhit);
int x97 : write(closesthit, anyhit) : read(miss);
int x98 : write(closesthit, anyhit) : read(closesthit);
int x99 : write(closesthit, anyhit) : read(miss, closesthit);
int x100 : write(closesthit, anyhit) : read(anyhit);
int x101 : write(closesthit, anyhit) : read(miss, anyhit);
int x102 : write(closesthit, anyhit) : read(closesthit, anyhit);
int x103 : write(closesthit, anyhit) : read(miss, closesthit, anyhit);
int x104 : write(closesthit, anyhit) : read(caller);
int x105 : write(closesthit, anyhit) : read(miss, caller);
int x106 : write(closesthit, anyhit) : read(closesthit, caller);
int x107 : write(closesthit, anyhit) : read(miss, closesthit, caller);
int x108 : write(closesthit, anyhit) : read(anyhit, caller);
int x109 : write(closesthit, anyhit) : read(miss, anyhit, caller);
int x110 : write(closesthit, anyhit) : read(closesthit, anyhit, caller);
int x111 : write(closesthit, anyhit) : read(miss, closesthit, anyhit, caller);
int x112 : write(miss, closesthit, anyhit);
int x113 : write(miss, closesthit, anyhit) : read(miss);
int x114 : write(miss, closesthit, anyhit) : read(closesthit);
int x115 : write(miss, closesthit, anyhit) : read(miss, closesthit);
int x116 : write(miss, closesthit, anyhit) : read(anyhit);
int x117 : write(miss, closesthit, anyhit) : read(miss, anyhit);
int x118 : write(miss, closesthit, anyhit) : read(closesthit, anyhit);
int x119 : write(miss, closesthit, anyhit) : read(miss, closesthit, anyhit);
int x120 : write(miss, closesthit, anyhit) : read(caller);
int x121 : write(miss, closesthit, anyhit) : read(miss, caller);
int x122 : write(miss, closesthit, anyhit) : read(closesthit, caller);
int x123 : write(miss, closesthit, anyhit) : read(miss, closesthit, caller);
int x124 : write(miss, closesthit, anyhit) : read(anyhit, caller);
int x125 : write(miss, closesthit, anyhit) : read(miss, anyhit, caller);
int x126 : write(miss, closesthit, anyhit) : read(closesthit, anyhit, caller);
int x127 : write(miss, closesthit, anyhit) : read(miss, closesthit, anyhit, caller);
int x128 : write(caller);
int x129 : write(caller) : read(miss);
int x130 : write(caller) : read(closesthit);
int x131 : write(caller) : read(miss, closesthit);
int x132 : write(caller) : read(anyhit);
int x133 : write(caller) : read(miss, anyhit);
int x134 : write(caller) : read(closesthit, anyhit);
int x135 : write(caller) : read(miss, closesthit, anyhit);
int x136 : write(caller) : read(caller);
int x137 : write(caller) : read(miss, caller);
int x138 : write(caller) : read(closesthit, caller);
int x139 : write(caller) : read(miss, closesthit, caller);
int x140 : write(caller) : read(anyhit, caller);
int x141 : write(caller) : read(miss, anyhit, caller);
int x142 : write(caller) : read(closesthit, anyhit, caller);
int x143 : write(caller) : read(miss, closesthit, anyhit, caller);
int x144 : write(miss, caller);
int x145 : write(miss, caller) : read(miss);
int x146 : write(miss, caller) : read(closesthit);
int x147 : write(miss, caller) : read(miss, closesthit);
int x148 : write(miss, caller) : read(anyhit);
int x149 : write(miss, caller) : read(miss, anyhit);
int x150 : write(miss, caller) : read(closesthit, anyhit);
int x151 : write(miss, caller) : read(miss, closesthit, anyhit);
int x152 : write(miss, caller) : read(caller);
int x153 : write(miss, caller) : read(miss, caller);
int x154 : write(miss, caller) : read(closesthit, caller);
int x155 : write(miss, caller) : read(miss, closesthit, caller);
int x156 : write(miss, caller) : read(anyhit, caller);
int x157 : write(miss, caller) : read(miss, anyhit, caller);
int x158 : write(miss, caller) : read(closesthit, anyhit, caller);
int x159 : write(miss, caller) : read(miss, closesthit, anyhit, caller);
int x160 : write(closesthit, caller);
int x161 : write(closesthit, caller) : read(miss);
int x162 : write(closesthit, caller) : read(closesthit);
int x163 : write(closesthit, caller) : read(miss, closesthit);
int x164 : write(closesthit, caller) : read(anyhit);
int x165 : write(closesthit, caller) : read(miss, anyhit);
int x166 : write(closesthit, caller) : read(closesthit, anyhit);
int x167 : write(closesthit, caller) : read(miss, closesthit, anyhit);
int x168 : write(closesthit, caller) : read(caller);
int x169 : write(closesthit, caller) : read(miss, caller);
int x170 : write(closesthit, caller) : read(closesthit, caller);
int x171 : write(closesthit, caller) : read(miss, closesthit, caller);
int x172 : write(closesthit, caller) : read(anyhit, caller);
int x173 : write(closesthit, caller) : read(miss, anyhit, caller);
int x174 : write(closesthit, caller) : read(closesthit, anyhit, caller);
int x175 : write(closesthit, caller) : read(miss, closesthit, anyhit, caller);
int x176 : write(miss, closesthit, caller);
int x177 : write(miss, closesthit, caller) : read(miss);
int x178 : write(miss, closesthit, caller) : read(closesthit);
int x179 : write(miss, closesthit, caller) : read(miss, closesthit);
int x180 : write(miss, closesthit, caller) : read(anyhit);
int x181 : write(miss, closesthit, caller) : read(miss, anyhit);
int x182 : write(miss, closesthit, caller) : read(closesthit, anyhit);
int x183 : write(miss, closesthit, caller) : read(miss, closesthit, anyhit);
int x184 : write(miss, closesthit, caller) : read(caller);
int x185 : write(miss, closesthit, caller) : read(miss, caller);
int x186 : write(miss, closesthit, caller) : read(closesthit, caller);
int x187 : write(miss, closesthit, caller) : read(miss, closesthit, caller);
int x188 : write(miss, closesthit, caller) : read(anyhit, caller);
int x189 : write(miss, closesthit, caller) : read(miss, anyhit, caller);
int x190 : write(miss, closesthit, caller) : read(closesthit, anyhit, caller);
int x191 : write(miss, closesthit, caller) : read(miss, closesthit, anyhit, caller);
int x192 : write(anyhit, caller);
int x193 : write(anyhit, caller) : read(miss);
int x194 : write(anyhit, caller) : read(closesthit);
int x195 : write(anyhit, caller) : read(miss, closesthit);
int x196 : write(anyhit, caller) : read(anyhit);
int x197 : write(anyhit, caller) : read(miss, anyhit);
int x198 : write(anyhit, caller) : read(closesthit, anyhit);
int x199 : write(anyhit, caller) : read(miss, closesthit, anyhit);
int x200 : write(anyhit, caller) : read(caller);
int x201 : write(anyhit, caller) : read(miss, caller);
int x202 : write(anyhit, caller) : read(closesthit, caller);
int x203 : write(anyhit, caller) : read(miss, closesthit, caller);
int x204 : write(anyhit, caller) : read(anyhit, caller);
int x205 : write(anyhit, caller) : read(miss, anyhit, caller);
int x206 : write(anyhit, caller) : read(closesthit, anyhit, caller);
int x207 : write(anyhit, caller) : read(miss, closesthit, anyhit, caller);
int x208 : write(miss, anyhit, caller);
int x209 : write(miss, anyhit, caller) : read(miss);
int x210 : write(miss, anyhit, caller) : read(closesthit);
int x211 : write(miss, anyhit, caller) : read(miss, closesthit);
int x212 : write(miss, anyhit, caller) : read(anyhit);
int x213 : write(miss, anyhit, caller) : read(miss, anyhit);
int x214 : write(miss, anyhit, caller) : read(closesthit, anyhit);
int x215 : write(miss, anyhit, caller) : read(miss, closesthit, anyhit);
int x216 : write(miss, anyhit, caller) : read(caller);
int x217 : write(miss, anyhit, caller) : read(miss, caller);
int x218 : write(miss, anyhit, caller) : read(closesthit, caller);
int x219 : write(miss, anyhit, caller) : read(miss, closesthit, caller);
int x220 : write(miss, anyhit, caller) : read(anyhit, caller);
int x221 : write(miss, anyhit, caller) : read(miss, anyhit, caller);
int x222 : write(miss, anyhit, caller) : read(closesthit, anyhit, caller);
int x223 : write(miss, anyhit, caller) : read(miss, closesthit, anyhit, caller);
int x224 : write(closesthit, anyhit, caller);
int x225 : write(closesthit, anyhit, caller) : read(miss);
int x226 : write(closesthit, anyhit, caller) : read(closesthit);
int x227 : write(closesthit, anyhit, caller) : read(miss, closesthit);
int x228 : write(closesthit, anyhit, caller) : read(anyhit);
int x229 : write(closesthit, anyhit, caller) : read(miss, anyhit);
int x230 : write(closesthit, anyhit, caller) : read(closesthit, anyhit);
int x231 : write(closesthit, anyhit, caller) : read(miss, closesthit, anyhit);
int x232 : write(closesthit, anyhit, caller) : read(caller);
int x233 : write(closesthit, anyhit, caller) : read(miss, caller);
int x234 : write(closesthit, anyhit, caller) : read(closesthit, caller);
int x235 : write(closesthit, anyhit, caller) : read(miss, closesthit, caller);
int x236 : write(closesthit, anyhit, caller) : read(anyhit, caller);
int x237 : write(closesthit, anyhit, caller) : read(miss, anyhit, caller);
int x238 : write(closesthit, anyhit, caller) : read(closesthit, anyhit, caller);
int x239 : write(closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
int x240 : write(miss, closesthit, anyhit, caller);
int x241 : write(miss, closesthit, anyhit, caller) : read(miss);
int x242 : write(miss, closesthit, anyhit, caller) : read(closesthit);
int x243 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit);
int x244 : write(miss, closesthit, anyhit, caller) : read(anyhit);
int x245 : write(miss, closesthit, anyhit, caller) : read(miss, anyhit);
int x246 : write(miss, closesthit, anyhit, caller) : read(closesthit, anyhit);
int x247 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit);
int x248 : write(miss, closesthit, anyhit, caller) : read(caller);
int x249 : write(miss, closesthit, anyhit, caller) : read(miss, caller);
int x250 : write(miss, closesthit, anyhit, caller) : read(closesthit, caller);
int x251 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, caller);
int x252 : write(miss, closesthit, anyhit, caller) : read(anyhit, caller);
int x253 : write(miss, closesthit, anyhit, caller) : read(miss, anyhit, caller);
int x254 : write(miss, closesthit, anyhit, caller) : read(closesthit, anyhit, caller);
int x255 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
};
