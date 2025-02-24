// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef sw_x86_hpp
#define sw_x86_hpp

#include "Reactor.hpp"

namespace rr {
namespace x86 {

RValue<Int> cvtss2si(RValue<Float> val);
RValue<Int4> cvtps2dq(RValue<Float4> val);

RValue<Float> rcpss(RValue<Float> val);
RValue<Float> sqrtss(RValue<Float> val);
RValue<Float> rsqrtss(RValue<Float> val);

RValue<Float4> rcpps(RValue<Float4> val);
RValue<Float4> sqrtps(RValue<Float4> val);
RValue<Float4> rsqrtps(RValue<Float4> val);
RValue<Float4> maxps(RValue<Float4> x, RValue<Float4> y);
RValue<Float4> minps(RValue<Float4> x, RValue<Float4> y);

RValue<Float> roundss(RValue<Float> val, unsigned char imm);
RValue<Float> floorss(RValue<Float> val);
RValue<Float> ceilss(RValue<Float> val);

RValue<Float4> roundps(RValue<Float4> val, unsigned char imm);
RValue<Float4> floorps(RValue<Float4> val);
RValue<Float4> ceilps(RValue<Float4> val);

RValue<Short4> paddsw(RValue<Short4> x, RValue<Short4> y);
RValue<Short4> psubsw(RValue<Short4> x, RValue<Short4> y);
RValue<UShort4> paddusw(RValue<UShort4> x, RValue<UShort4> y);
RValue<UShort4> psubusw(RValue<UShort4> x, RValue<UShort4> y);
RValue<SByte8> paddsb(RValue<SByte8> x, RValue<SByte8> y);
RValue<SByte8> psubsb(RValue<SByte8> x, RValue<SByte8> y);
RValue<Byte8> paddusb(RValue<Byte8> x, RValue<Byte8> y);
RValue<Byte8> psubusb(RValue<Byte8> x, RValue<Byte8> y);

RValue<UShort4> pavgw(RValue<UShort4> x, RValue<UShort4> y);

RValue<Short4> pmaxsw(RValue<Short4> x, RValue<Short4> y);
RValue<Short4> pminsw(RValue<Short4> x, RValue<Short4> y);

RValue<Short4> pcmpgtw(RValue<Short4> x, RValue<Short4> y);
RValue<Short4> pcmpeqw(RValue<Short4> x, RValue<Short4> y);
RValue<Byte8> pcmpgtb(RValue<SByte8> x, RValue<SByte8> y);
RValue<Byte8> pcmpeqb(RValue<Byte8> x, RValue<Byte8> y);

RValue<Short4> packssdw(RValue<Int2> x, RValue<Int2> y);
RValue<Short8> packssdw(RValue<Int4> x, RValue<Int4> y);
RValue<SByte8> packsswb(RValue<Short4> x, RValue<Short4> y);
RValue<Byte8> packuswb(RValue<Short4> x, RValue<Short4> y);

RValue<UShort8> packusdw(RValue<Int4> x, RValue<Int4> y);

RValue<UShort4> psrlw(RValue<UShort4> x, unsigned char y);
RValue<UShort8> psrlw(RValue<UShort8> x, unsigned char y);
RValue<Short4> psraw(RValue<Short4> x, unsigned char y);
RValue<Short8> psraw(RValue<Short8> x, unsigned char y);
RValue<Short4> psllw(RValue<Short4> x, unsigned char y);
RValue<Short8> psllw(RValue<Short8> x, unsigned char y);
RValue<Int2> pslld(RValue<Int2> x, unsigned char y);
RValue<Int4> pslld(RValue<Int4> x, unsigned char y);
RValue<Int2> psrad(RValue<Int2> x, unsigned char y);
RValue<Int4> psrad(RValue<Int4> x, unsigned char y);
RValue<UInt2> psrld(RValue<UInt2> x, unsigned char y);
RValue<UInt4> psrld(RValue<UInt4> x, unsigned char y);

RValue<Int4> pmaxsd(RValue<Int4> x, RValue<Int4> y);
RValue<Int4> pminsd(RValue<Int4> x, RValue<Int4> y);
RValue<UInt4> pmaxud(RValue<UInt4> x, RValue<UInt4> y);
RValue<UInt4> pminud(RValue<UInt4> x, RValue<UInt4> y);

RValue<Short4> pmulhw(RValue<Short4> x, RValue<Short4> y);
RValue<UShort4> pmulhuw(RValue<UShort4> x, RValue<UShort4> y);
RValue<Int2> pmaddwd(RValue<Short4> x, RValue<Short4> y);

RValue<Short8> pmulhw(RValue<Short8> x, RValue<Short8> y);
RValue<UShort8> pmulhuw(RValue<UShort8> x, RValue<UShort8> y);
RValue<Int4> pmaddwd(RValue<Short8> x, RValue<Short8> y);

RValue<Int> movmskps(RValue<Float4> x);
RValue<Int> pmovmskb(RValue<Byte8> x);

}  // namespace x86
}  // namespace rr

#endif  // rr_x86_hpp