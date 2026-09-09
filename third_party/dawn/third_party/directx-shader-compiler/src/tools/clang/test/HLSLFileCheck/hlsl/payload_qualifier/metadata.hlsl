// RUN: %dxc -T lib_6_6 %s -enable-payload-qualifiers | FileCheck %s

// CHECK: !dx.dxrPayloadAnnotations = !{{{![0-9]+}}}
// CHECK: {{![0-9]+}} = !{i32 0, %struct.MyPayload undef, {{![0-9]+}}, %struct.SubPayload undef, {{![0-9]+}}}
// CHECK: {{![0-9]+}} = !{i32 0, i32 0}
// CHECK: {{![0-9]+}} = !{i32 0, i32 513}
// CHECK: {{![0-9]+}} = !{i32 0, i32 33}
// CHECK: {{![0-9]+}} = !{i32 0, i32 545}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8448}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8208}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8464}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12288}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12544}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12304}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12560}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8193}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8449}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8209}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8465}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12289}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12545}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12305}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12561}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8705}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8961}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8721}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8977}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12801}
// CHECK: {{![0-9]+}} = !{i32 0, i32 13057}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12817}
// CHECK: {{![0-9]+}} = !{i32 0, i32 13073}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8225}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8481}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8241}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8497}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12321}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12577}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12337}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12593}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8737}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8993}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8753}
// CHECK: {{![0-9]+}} = !{i32 0, i32 9009}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12833}
// CHECK: {{![0-9]+}} = !{i32 0, i32 13089}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12849}
// CHECK: {{![0-9]+}} = !{i32 0, i32 13105}
// CHECK: {{![0-9]+}} = !{i32 0, i32 258}
// CHECK: {{![0-9]+}} = !{i32 0, i32 18}
// CHECK: {{![0-9]+}} = !{i32 0, i32 274}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4098}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4354}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4114}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4370}
// CHECK: {{![0-9]+}} = !{i32 0, i32 3}
// CHECK: {{![0-9]+}} = !{i32 0, i32 259}
// CHECK: {{![0-9]+}} = !{i32 0, i32 19}
// CHECK: {{![0-9]+}} = !{i32 0, i32 275}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4099}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4355}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4115}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4371}
// CHECK: {{![0-9]+}} = !{i32 0, i32 515}
// CHECK: {{![0-9]+}} = !{i32 0, i32 771}
// CHECK: {{![0-9]+}} = !{i32 0, i32 531}
// CHECK: {{![0-9]+}} = !{i32 0, i32 787}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4611}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4867}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4627}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4883}
// CHECK: {{![0-9]+}} = !{i32 0, i32 35}
// CHECK: {{![0-9]+}} = !{i32 0, i32 291}
// CHECK: {{![0-9]+}} = !{i32 0, i32 51}
// CHECK: {{![0-9]+}} = !{i32 0, i32 307}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4131}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4387}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4147}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4403}
// CHECK: {{![0-9]+}} = !{i32 0, i32 547}
// CHECK: {{![0-9]+}} = !{i32 0, i32 803}
// CHECK: {{![0-9]+}} = !{i32 0, i32 563}
// CHECK: {{![0-9]+}} = !{i32 0, i32 819}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4643}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4899}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4659}
// CHECK: {{![0-9]+}} = !{i32 0, i32 4915}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8450}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8210}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8466}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12290}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12546}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12306}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12562}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8195}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8451}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8211}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8467}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12291}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12547}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12307}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12563}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8707}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8963}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8723}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8979}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12803}
// CHECK: {{![0-9]+}} = !{i32 0, i32 13059}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12819}
// CHECK: {{![0-9]+}} = !{i32 0, i32 13075}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8227}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8483}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8243}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8499}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12323}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12579}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12339}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12595}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8739}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8995}
// CHECK: {{![0-9]+}} = !{i32 0, i32 8755}
// CHECK: {{![0-9]+}} = !{i32 0, i32 9011}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12835}
// CHECK: {{![0-9]+}} = !{i32 0, i32 13091}
// CHECK: {{![0-9]+}} = !{i32 0, i32 12851}
// CHECK: {{![0-9]+}} = !{i32 0, i32 13107}

struct [raypayload] SubPayload{
    int a1 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
    int a2 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
};

struct [raypayload] MyPayload {
int x0  : write(); 
int x1  : read();
int x2  : read() : write();
int x24 : write(miss) : read(caller);
int x40 : write(closesthit) : read(caller);
int x56 : write(miss, closesthit) : read(caller);
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
int x88 : write(miss, anyhit) : read(caller);
int x89 : write(miss, anyhit) : read(miss, caller);
int x90 : write(miss, anyhit) : read(closesthit, caller);
int x91 : write(miss, anyhit) : read(miss, closesthit, caller);
int x92 : write(miss, anyhit) : read(anyhit, caller);
int x93 : write(miss, anyhit) : read(miss, anyhit, caller);
int x94 : write(miss, anyhit) : read(closesthit, anyhit, caller);
int x95 : write(miss, anyhit) : read(miss, closesthit, anyhit, caller);
int x104 : write(closesthit, anyhit) : read(caller);
int x105 : write(closesthit, anyhit) : read(miss, caller);
int x106 : write(closesthit, anyhit) : read(closesthit, caller);
int x107 : write(closesthit, anyhit) : read(miss, closesthit, caller);
int x108 : write(closesthit, anyhit) : read(anyhit, caller);
int x109 : write(closesthit, anyhit) : read(miss, anyhit, caller);
int x110 : write(closesthit, anyhit) : read(closesthit, anyhit, caller);
int x111 : write(closesthit, anyhit) : read(miss, closesthit, anyhit, caller);
int x120 : write(miss, closesthit, anyhit) : read(caller);
int x121 : write(miss, closesthit, anyhit) : read(miss, caller);
int x122 : write(miss, closesthit, anyhit) : read(closesthit, caller);
int x123 : write(miss, closesthit, anyhit) : read(miss, closesthit, caller);
int x124 : write(miss, closesthit, anyhit) : read(anyhit, caller);
int x125 : write(miss, closesthit, anyhit) : read(miss, anyhit, caller);
int x126 : write(miss, closesthit, anyhit) : read(closesthit, anyhit, caller);
int x127 : write(miss, closesthit, anyhit) : read(miss, closesthit, anyhit, caller);
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
int x152 : write(miss, caller) : read(caller);
int x153 : write(miss, caller) : read(miss, caller);
int x154 : write(miss, caller) : read(closesthit, caller);
int x155 : write(miss, caller) : read(miss, closesthit, caller);
int x156 : write(miss, caller) : read(anyhit, caller);
int x157 : write(miss, caller) : read(miss, anyhit, caller);
int x158 : write(miss, caller) : read(closesthit, anyhit, caller);
int x159 : write(miss, caller) : read(miss, closesthit, anyhit, caller);
int x168 : write(closesthit, caller) : read(caller);
int x169 : write(closesthit, caller) : read(miss, caller);
int x170 : write(closesthit, caller) : read(closesthit, caller);
int x171 : write(closesthit, caller) : read(miss, closesthit, caller);
int x172 : write(closesthit, caller) : read(anyhit, caller);
int x173 : write(closesthit, caller) : read(miss, anyhit, caller);
int x174 : write(closesthit, caller) : read(closesthit, anyhit, caller);
int x175 : write(closesthit, caller) : read(miss, closesthit, anyhit, caller);
int x184 : write(miss, closesthit, caller) : read(caller);
int x185 : write(miss, closesthit, caller) : read(miss, caller);
int x186 : write(miss, closesthit, caller) : read(closesthit, caller);
int x187 : write(miss, closesthit, caller) : read(miss, closesthit, caller);
int x188 : write(miss, closesthit, caller) : read(anyhit, caller);
int x189 : write(miss, closesthit, caller) : read(miss, anyhit, caller);
int x190 : write(miss, closesthit, caller) : read(closesthit, anyhit, caller);
int x191 : write(miss, closesthit, caller) : read(miss, closesthit, anyhit, caller);
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
int x216 : write(miss, anyhit, caller) : read(caller);
int x217 : write(miss, anyhit, caller) : read(miss, caller);
int x218 : write(miss, anyhit, caller) : read(closesthit, caller);
int x219 : write(miss, anyhit, caller) : read(miss, closesthit, caller);
int x220 : write(miss, anyhit, caller) : read(anyhit, caller);
int x221 : write(miss, anyhit, caller) : read(miss, anyhit, caller);
int x222 : write(miss, anyhit, caller) : read(closesthit, anyhit, caller);
int x223 : write(miss, anyhit, caller) : read(miss, closesthit, anyhit, caller);
int x232 : write(closesthit, anyhit, caller) : read(caller);
int x233 : write(closesthit, anyhit, caller) : read(miss, caller);
int x234 : write(closesthit, anyhit, caller) : read(closesthit, caller);
int x235 : write(closesthit, anyhit, caller) : read(miss, closesthit, caller);
int x236 : write(closesthit, anyhit, caller) : read(anyhit, caller);
int x237 : write(closesthit, anyhit, caller) : read(miss, anyhit, caller);
int x238 : write(closesthit, anyhit, caller) : read(closesthit, anyhit, caller);
int x239 : write(closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
int x248 : write(miss, closesthit, anyhit, caller) : read(caller);
int x249 : write(miss, closesthit, anyhit, caller) : read(miss, caller);
int x250 : write(miss, closesthit, anyhit, caller) : read(closesthit, caller);
int x251 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, caller);
int x252 : write(miss, closesthit, anyhit, caller) : read(anyhit, caller);
int x253 : write(miss, closesthit, anyhit, caller) : read(miss, anyhit, caller);
int x254 : write(miss, closesthit, anyhit, caller) : read(closesthit, anyhit, caller);
int x255 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);

SubPayload p1;

struct { int x; } s1 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);

};

[shader("miss")]
void Miss( inout MyPayload payload ) {
    payload.x24 = 42;
}