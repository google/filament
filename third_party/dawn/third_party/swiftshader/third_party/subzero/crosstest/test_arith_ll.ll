
define internal i32 @_Z7testAddbb(i32 %a, i32 %b) {


  %result = add i32 %a, %b

  ret i32 %result
}

define internal i32 @_Z7testAddhh(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = add i8 %a.trunc, %b.trunc
  %result = zext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testAddtt(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = add i16 %a.trunc, %b.trunc
  %result = zext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testAddjj(i32 %a, i32 %b) {


  %result = add i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z7testAddyy(i64 %a, i64 %b) {


  %result = add i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z7testAddDv4_jS_(<4 x i32> %a, <4 x i32> %b) {


  %result = add <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z7testAddDv8_tS_(<8 x i16> %a, <8 x i16> %b) {


  %result = add <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z7testAddDv16_hS_(<16 x i8> %a, <16 x i8> %b) {


  %result = add <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z7testSubbb(i32 %a, i32 %b) {


  %result = sub i32 %a, %b

  ret i32 %result
}

define internal i32 @_Z7testSubhh(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = sub i8 %a.trunc, %b.trunc
  %result = zext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testSubtt(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = sub i16 %a.trunc, %b.trunc
  %result = zext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testSubjj(i32 %a, i32 %b) {


  %result = sub i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z7testSubyy(i64 %a, i64 %b) {


  %result = sub i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z7testSubDv4_jS_(<4 x i32> %a, <4 x i32> %b) {


  %result = sub <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z7testSubDv8_tS_(<8 x i16> %a, <8 x i16> %b) {


  %result = sub <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z7testSubDv16_hS_(<16 x i8> %a, <16 x i8> %b) {


  %result = sub <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z7testMulbb(i32 %a, i32 %b) {


  %result = mul i32 %a, %b

  ret i32 %result
}

define internal i32 @_Z7testMulhh(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = mul i8 %a.trunc, %b.trunc
  %result = zext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testMultt(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = mul i16 %a.trunc, %b.trunc
  %result = zext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testMuljj(i32 %a, i32 %b) {


  %result = mul i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z7testMulyy(i64 %a, i64 %b) {


  %result = mul i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z7testMulDv4_jS_(<4 x i32> %a, <4 x i32> %b) {


  %result = mul <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z7testMulDv8_tS_(<8 x i16> %a, <8 x i16> %b) {


  %result = mul <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z7testMulDv16_hS_(<16 x i8> %a, <16 x i8> %b) {


  %result = mul <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z8testSdivbb(i32 %a, i32 %b) {


  %result = sdiv i32 %a, %b

  ret i32 %result
}

define internal i32 @_Z8testSdivaa(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = sdiv i8 %a.trunc, %b.trunc
  %result = sext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testSdivss(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = sdiv i16 %a.trunc, %b.trunc
  %result = sext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testSdivii(i32 %a, i32 %b) {


  %result = sdiv i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z8testSdivxx(i64 %a, i64 %b) {


  %result = sdiv i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z8testSdivDv4_iS_(<4 x i32> %a, <4 x i32> %b) {


  %result = sdiv <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z8testSdivDv8_sS_(<8 x i16> %a, <8 x i16> %b) {


  %result = sdiv <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z8testSdivDv16_aS_(<16 x i8> %a, <16 x i8> %b) {


  %result = sdiv <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z8testUdivbb(i32 %a, i32 %b) {


  %result = udiv i32 %a, %b

  ret i32 %result
}

define internal i32 @_Z8testUdivhh(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = udiv i8 %a.trunc, %b.trunc
  %result = zext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testUdivtt(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = udiv i16 %a.trunc, %b.trunc
  %result = zext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testUdivjj(i32 %a, i32 %b) {


  %result = udiv i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z8testUdivyy(i64 %a, i64 %b) {


  %result = udiv i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z8testUdivDv4_jS_(<4 x i32> %a, <4 x i32> %b) {


  %result = udiv <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z8testUdivDv8_tS_(<8 x i16> %a, <8 x i16> %b) {


  %result = udiv <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z8testUdivDv16_hS_(<16 x i8> %a, <16 x i8> %b) {


  %result = udiv <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z8testSrembb(i32 %a, i32 %b) {


  %result = srem i32 %a, %b

  ret i32 %result
}

define internal i32 @_Z8testSremaa(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = srem i8 %a.trunc, %b.trunc
  %result = sext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testSremss(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = srem i16 %a.trunc, %b.trunc
  %result = sext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testSremii(i32 %a, i32 %b) {


  %result = srem i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z8testSremxx(i64 %a, i64 %b) {


  %result = srem i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z8testSremDv4_iS_(<4 x i32> %a, <4 x i32> %b) {


  %result = srem <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z8testSremDv8_sS_(<8 x i16> %a, <8 x i16> %b) {


  %result = srem <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z8testSremDv16_aS_(<16 x i8> %a, <16 x i8> %b) {


  %result = srem <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z8testUrembb(i32 %a, i32 %b) {


  %result = urem i32 %a, %b

  ret i32 %result
}

define internal i32 @_Z8testUremhh(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = urem i8 %a.trunc, %b.trunc
  %result = zext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testUremtt(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = urem i16 %a.trunc, %b.trunc
  %result = zext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testUremjj(i32 %a, i32 %b) {


  %result = urem i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z8testUremyy(i64 %a, i64 %b) {


  %result = urem i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z8testUremDv4_jS_(<4 x i32> %a, <4 x i32> %b) {


  %result = urem <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z8testUremDv8_tS_(<8 x i16> %a, <8 x i16> %b) {


  %result = urem <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z8testUremDv16_hS_(<16 x i8> %a, <16 x i8> %b) {


  %result = urem <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z7testShlbb(i32 %a, i32 %b) {


  %result = shl i32 %a, %b

  ret i32 %result
}

define internal i32 @_Z7testShlhh(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = shl i8 %a.trunc, %b.trunc
  %result = zext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testShltt(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = shl i16 %a.trunc, %b.trunc
  %result = zext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testShljj(i32 %a, i32 %b) {


  %result = shl i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z7testShlyy(i64 %a, i64 %b) {


  %result = shl i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z7testShlDv4_jS_(<4 x i32> %a, <4 x i32> %b) {


  %result = shl <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z7testShlDv8_tS_(<8 x i16> %a, <8 x i16> %b) {


  %result = shl <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z7testShlDv16_hS_(<16 x i8> %a, <16 x i8> %b) {


  %result = shl <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z8testLshrbb(i32 %a, i32 %b) {


  %result = lshr i32 %a, %b

  ret i32 %result
}

define internal i32 @_Z8testLshrhh(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = lshr i8 %a.trunc, %b.trunc
  %result = zext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testLshrtt(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = lshr i16 %a.trunc, %b.trunc
  %result = zext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testLshrjj(i32 %a, i32 %b) {


  %result = lshr i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z8testLshryy(i64 %a, i64 %b) {


  %result = lshr i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z8testLshrDv4_jS_(<4 x i32> %a, <4 x i32> %b) {


  %result = lshr <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z8testLshrDv8_tS_(<8 x i16> %a, <8 x i16> %b) {


  %result = lshr <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z8testLshrDv16_hS_(<16 x i8> %a, <16 x i8> %b) {


  %result = lshr <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z8testAshrbb(i32 %a, i32 %b) {


  %result = ashr i32 %a, %b

  ret i32 %result
}

define internal i32 @_Z8testAshraa(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = ashr i8 %a.trunc, %b.trunc
  %result = sext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testAshrss(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = ashr i16 %a.trunc, %b.trunc
  %result = sext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z8testAshrii(i32 %a, i32 %b) {


  %result = ashr i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z8testAshrxx(i64 %a, i64 %b) {


  %result = ashr i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z8testAshrDv4_iS_(<4 x i32> %a, <4 x i32> %b) {


  %result = ashr <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z8testAshrDv8_sS_(<8 x i16> %a, <8 x i16> %b) {


  %result = ashr <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z8testAshrDv16_aS_(<16 x i8> %a, <16 x i8> %b) {


  %result = ashr <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z7testAndbb(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i1
  %b.trunc = trunc i32 %b to i1
  %result.trunc = and i1 %a.trunc, %b.trunc
  %result = zext i1 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testAndhh(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = and i8 %a.trunc, %b.trunc
  %result = zext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testAndtt(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = and i16 %a.trunc, %b.trunc
  %result = zext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testAndjj(i32 %a, i32 %b) {


  %result = and i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z7testAndyy(i64 %a, i64 %b) {


  %result = and i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z7testAndDv4_jS_(<4 x i32> %a, <4 x i32> %b) {


  %result = and <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z7testAndDv8_tS_(<8 x i16> %a, <8 x i16> %b) {


  %result = and <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z7testAndDv16_hS_(<16 x i8> %a, <16 x i8> %b) {


  %result = and <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z6testOrbb(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i1
  %b.trunc = trunc i32 %b to i1
  %result.trunc = or i1 %a.trunc, %b.trunc
  %result = zext i1 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z6testOrhh(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = or i8 %a.trunc, %b.trunc
  %result = zext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z6testOrtt(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = or i16 %a.trunc, %b.trunc
  %result = zext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z6testOrjj(i32 %a, i32 %b) {


  %result = or i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z6testOryy(i64 %a, i64 %b) {


  %result = or i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z6testOrDv4_jS_(<4 x i32> %a, <4 x i32> %b) {


  %result = or <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z6testOrDv8_tS_(<8 x i16> %a, <8 x i16> %b) {


  %result = or <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z6testOrDv16_hS_(<16 x i8> %a, <16 x i8> %b) {


  %result = or <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal i32 @_Z7testXorbb(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i1
  %b.trunc = trunc i32 %b to i1
  %result.trunc = xor i1 %a.trunc, %b.trunc
  %result = zext i1 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testXorhh(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i8
  %b.trunc = trunc i32 %b to i8
  %result.trunc = xor i8 %a.trunc, %b.trunc
  %result = zext i8 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testXortt(i32 %a, i32 %b) {
  %a.trunc = trunc i32 %a to i16
  %b.trunc = trunc i32 %b to i16
  %result.trunc = xor i16 %a.trunc, %b.trunc
  %result = zext i16 %result.trunc to i32
  ret i32 %result
}

define internal i32 @_Z7testXorjj(i32 %a, i32 %b) {


  %result = xor i32 %a, %b

  ret i32 %result
}

define internal i64 @_Z7testXoryy(i64 %a, i64 %b) {


  %result = xor i64 %a, %b

  ret i64 %result
}

define internal <4 x i32> @_Z7testXorDv4_jS_(<4 x i32> %a, <4 x i32> %b) {


  %result = xor <4 x i32> %a, %b

  ret <4 x i32> %result
}

define internal <8 x i16> @_Z7testXorDv8_tS_(<8 x i16> %a, <8 x i16> %b) {


  %result = xor <8 x i16> %a, %b

  ret <8 x i16> %result
}

define internal <16 x i8> @_Z7testXorDv16_hS_(<16 x i8> %a, <16 x i8> %b) {


  %result = xor <16 x i8> %a, %b

  ret <16 x i8> %result
}

define internal float @_Z8testFaddff(float %a, float %b) {


  %result = fadd float %a, %b

  ret float %result
}

define internal double @_Z8testFadddd(double %a, double %b) {


  %result = fadd double %a, %b

  ret double %result
}

define internal <4 x float> @_Z8testFaddDv4_fS_(<4 x float> %a, <4 x float> %b) {


  %result = fadd <4 x float> %a, %b

  ret <4 x float> %result
}

define internal float @_Z8testFsubff(float %a, float %b) {


  %result = fsub float %a, %b

  ret float %result
}

define internal double @_Z8testFsubdd(double %a, double %b) {


  %result = fsub double %a, %b

  ret double %result
}

define internal <4 x float> @_Z8testFsubDv4_fS_(<4 x float> %a, <4 x float> %b) {


  %result = fsub <4 x float> %a, %b

  ret <4 x float> %result
}

define internal float @_Z8testFmulff(float %a, float %b) {


  %result = fmul float %a, %b

  ret float %result
}

define internal double @_Z8testFmuldd(double %a, double %b) {


  %result = fmul double %a, %b

  ret double %result
}

define internal <4 x float> @_Z8testFmulDv4_fS_(<4 x float> %a, <4 x float> %b) {


  %result = fmul <4 x float> %a, %b

  ret <4 x float> %result
}

define internal float @_Z8testFdivff(float %a, float %b) {


  %result = fdiv float %a, %b

  ret float %result
}

define internal double @_Z8testFdivdd(double %a, double %b) {


  %result = fdiv double %a, %b

  ret double %result
}

define internal <4 x float> @_Z8testFdivDv4_fS_(<4 x float> %a, <4 x float> %b) {


  %result = fdiv <4 x float> %a, %b

  ret <4 x float> %result
}

define internal float @_Z8testFremff(float %a, float %b) {


  %result = frem float %a, %b

  ret float %result
}

define internal double @_Z8testFremdd(double %a, double %b) {


  %result = frem double %a, %b

  ret double %result
}

define internal <4 x float> @_Z8testFremDv4_fS_(<4 x float> %a, <4 x float> %b) {


  %result = frem <4 x float> %a, %b

  ret <4 x float> %result
}
