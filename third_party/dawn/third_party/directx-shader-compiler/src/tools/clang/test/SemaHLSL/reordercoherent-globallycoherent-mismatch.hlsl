// RUN: %dxc -Tlib_6_9 -verify %s

RWByteAddressBuffer NonCBuf;
globallycoherent RWByteAddressBuffer GCBuf;
reordercoherent RWByteAddressBuffer RCBuf;
// expected-warning@+2{{attribute 'globallycoherent' implies 'reordercoherent'}}
// expected-warning@+1{{attribute 'reordercoherent' implied by 'globallycoherent' in 'RCGCBuf'. 'reordercoherent' ignored.}}
reordercoherent globallycoherent RWByteAddressBuffer RCGCBuf;

globallycoherent RWByteAddressBuffer getPromoteRC() {
  return RCBuf; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' promotes reordercoherent to globallycoherent annotation}}
}

reordercoherent RWByteAddressBuffer getDemoteGC() {
  return GCBuf; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' demotes globallycoherent to reordercoherent annotation}}
}

globallycoherent RWByteAddressBuffer GCBufArr[2];
reordercoherent RWByteAddressBuffer RCBufArr[2];

reordercoherent RWByteAddressBuffer RCBufMultiArr[2][2];
globallycoherent RWByteAddressBuffer GCBufMultiArr[2][2];

globallycoherent RWByteAddressBuffer getPromoteRCArr() {
  return RCBufArr[0]; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' promotes reordercoherent to globallycoherent annotation}}
}

reordercoherent RWByteAddressBuffer getDemoteGCArr() {
  return GCBufArr[0]; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' demotes globallycoherent to reordercoherent annotation}}
}

globallycoherent RWByteAddressBuffer getPromoteRCMultiArr() {
  return RCBufMultiArr[0][0]; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' promotes reordercoherent to globallycoherent annotation}}
}

reordercoherent RWByteAddressBuffer getDemoteGCMultiArr() {
  return GCBufMultiArr[0][0]; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' demotes globallycoherent to reordercoherent annotation}}
}

void NonGCStore(RWByteAddressBuffer Buf) {
  Buf.Store(0, 0);
}

void RCStore(reordercoherent RWByteAddressBuffer Buf) {
  Buf.Store(0, 0);
}

void GCStore(globallycoherent RWByteAddressBuffer Buf) {
  Buf.Store(0, 0);
}

void getPromoteToGCParam(inout globallycoherent RWByteAddressBuffer PGCBuf) {
  PGCBuf = RCBuf; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer __restrict' promotes reordercoherent to globallycoherent annotation}}
}
void getDemoteToRCParam(inout reordercoherent RWByteAddressBuffer PRCBuf) {
  PRCBuf = GCBuf; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer __restrict' demotes globallycoherent to reordercoherent annotation}}
}

static reordercoherent RWByteAddressBuffer SRCDemoteBufArr[2] = GCBufArr; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer [2]' to 'reordercoherent RWByteAddressBuffer [2]' demotes globallycoherent to reordercoherent annotation}}
static reordercoherent RWByteAddressBuffer SRCDemoteBufMultiArr0[2] = GCBufMultiArr[0]; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer [2]' to 'reordercoherent RWByteAddressBuffer [2]' demotes globallycoherent to reordercoherent annotation}}
static reordercoherent RWByteAddressBuffer SRCDemoteBufMultiArr1[2][2] = GCBufMultiArr; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer [2][2]' to 'reordercoherent RWByteAddressBuffer [2][2]' demotes globallycoherent to reordercoherent annotation}}

static globallycoherent RWByteAddressBuffer SRCPromoteBufArr[2] = RCBufArr; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer [2]' to 'globallycoherent RWByteAddressBuffer [2]' promotes reordercoherent to globallycoherent annotation}}
static globallycoherent RWByteAddressBuffer SRCPromoteBufMultiArr0[2] = RCBufMultiArr[0]; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer [2]' to 'globallycoherent RWByteAddressBuffer [2]' promotes reordercoherent to globallycoherent annotation}}
static globallycoherent RWByteAddressBuffer SRCPromoteBufMultiArr1[2][2] = RCBufMultiArr; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer [2][2]' to 'globallycoherent RWByteAddressBuffer [2][2]' promotes reordercoherent to globallycoherent annotation}}

void getPromoteToGCParamArr(inout globallycoherent RWByteAddressBuffer PGCBufArr[2]) {
  PGCBufArr = RCBufArr; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer [2]' to 'globallycoherent RWByteAddressBuffer __restrict[2]' promotes reordercoherent to globallycoherent annotation}}
}
void getDemoteToRCParamArr(inout reordercoherent RWByteAddressBuffer PRCBufArr[2]) {
  PRCBufArr = GCBufArr; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer [2]' to 'reordercoherent RWByteAddressBuffer __restrict[2]' demotes globallycoherent to reordercoherent annotation}}
}

globallycoherent RWByteAddressBuffer getGCBuf() {
  return GCBuf;
}

reordercoherent RWByteAddressBuffer getRCBuf() {
  return RCBuf;
}

[shader("raygeneration")]
void main()
{
  GCStore(RCBuf); // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' promotes reordercoherent to globallycoherent annotation}}
  RCStore(GCBuf); // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' demotes globallycoherent to reordercoherent annotation}}

  reordercoherent RWByteAddressBuffer RCCopyGC = GCBuf; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' demotes globallycoherent to reordercoherent annotation}}
  globallycoherent RWByteAddressBuffer GCCopyRC = RCBuf; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' promotes reordercoherent to globallycoherent annotation}}

  reordercoherent RWByteAddressBuffer RCCopyGCReturn = getGCBuf(); // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' demotes globallycoherent to reordercoherent annotation}}
  globallycoherent RWByteAddressBuffer GCCopyRCReturn = getRCBuf(); // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' promotes reordercoherent to globallycoherent annotation}}

  reordercoherent RWByteAddressBuffer RCCopyGC0 = GCBufArr[0]; // expected-warning{{implicit conversion from 'globallycoherent RWByteAddressBuffer' to 'reordercoherent RWByteAddressBuffer' demotes globallycoherent to reordercoherent annotation}}
  globallycoherent RWByteAddressBuffer GCCopyRC0 = RCBufArr[0]; // expected-warning{{implicit conversion from 'reordercoherent RWByteAddressBuffer' to 'globallycoherent RWByteAddressBuffer' promotes reordercoherent to globallycoherent annotation}}
}
