define <16 x i8> @_Z8icmpi1EqDv16_aS_(<16 x i8> %a, <16 x i8> %b) {
entry:
  %a.trunc = trunc <16 x i8> %a to <16 x i1>
  %b.trunc = trunc <16 x i8> %b to <16 x i1>
  %cmp = icmp eq <16 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <16 x i1> %cmp to <16 x i8>
  ret <16 x i8> %cmp.sext
}

define <16 x i8> @_Z8icmpi1NeDv16_aS_(<16 x i8> %a, <16 x i8> %b) {
entry:
  %a.trunc = trunc <16 x i8> %a to <16 x i1>
  %b.trunc = trunc <16 x i8> %b to <16 x i1>
  %cmp = icmp ne <16 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <16 x i1> %cmp to <16 x i8>
  ret <16 x i8> %cmp.sext
}

define <16 x i8> @_Z9icmpi1UgtDv16_aS_(<16 x i8> %a, <16 x i8> %b) {
entry:
  %a.trunc = trunc <16 x i8> %a to <16 x i1>
  %b.trunc = trunc <16 x i8> %b to <16 x i1>
  %cmp = icmp ugt <16 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <16 x i1> %cmp to <16 x i8>
  ret <16 x i8> %cmp.sext
}

define <16 x i8> @_Z9icmpi1UgeDv16_aS_(<16 x i8> %a, <16 x i8> %b) {
entry:
  %a.trunc = trunc <16 x i8> %a to <16 x i1>
  %b.trunc = trunc <16 x i8> %b to <16 x i1>
  %cmp = icmp uge <16 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <16 x i1> %cmp to <16 x i8>
  ret <16 x i8> %cmp.sext
}

define <16 x i8> @_Z9icmpi1UltDv16_aS_(<16 x i8> %a, <16 x i8> %b) {
entry:
  %a.trunc = trunc <16 x i8> %a to <16 x i1>
  %b.trunc = trunc <16 x i8> %b to <16 x i1>
  %cmp = icmp ult <16 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <16 x i1> %cmp to <16 x i8>
  ret <16 x i8> %cmp.sext
}

define <16 x i8> @_Z9icmpi1UleDv16_aS_(<16 x i8> %a, <16 x i8> %b) {
entry:
  %a.trunc = trunc <16 x i8> %a to <16 x i1>
  %b.trunc = trunc <16 x i8> %b to <16 x i1>
  %cmp = icmp ule <16 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <16 x i1> %cmp to <16 x i8>
  ret <16 x i8> %cmp.sext
}

define <16 x i8> @_Z9icmpi1SgtDv16_aS_(<16 x i8> %a, <16 x i8> %b) {
entry:
  %a.trunc = trunc <16 x i8> %a to <16 x i1>
  %b.trunc = trunc <16 x i8> %b to <16 x i1>
  %cmp = icmp sgt <16 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <16 x i1> %cmp to <16 x i8>
  ret <16 x i8> %cmp.sext
}

define <16 x i8> @_Z9icmpi1SgeDv16_aS_(<16 x i8> %a, <16 x i8> %b) {
entry:
  %a.trunc = trunc <16 x i8> %a to <16 x i1>
  %b.trunc = trunc <16 x i8> %b to <16 x i1>
  %cmp = icmp sge <16 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <16 x i1> %cmp to <16 x i8>
  ret <16 x i8> %cmp.sext
}

define <16 x i8> @_Z9icmpi1SltDv16_aS_(<16 x i8> %a, <16 x i8> %b) {
entry:
  %a.trunc = trunc <16 x i8> %a to <16 x i1>
  %b.trunc = trunc <16 x i8> %b to <16 x i1>
  %cmp = icmp slt <16 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <16 x i1> %cmp to <16 x i8>
  ret <16 x i8> %cmp.sext
}

define <16 x i8> @_Z9icmpi1SleDv16_aS_(<16 x i8> %a, <16 x i8> %b) {
entry:
  %a.trunc = trunc <16 x i8> %a to <16 x i1>
  %b.trunc = trunc <16 x i8> %b to <16 x i1>
  %cmp = icmp sle <16 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <16 x i1> %cmp to <16 x i8>
  ret <16 x i8> %cmp.sext
}

define <8 x i16> @_Z8icmpi1EqDv8_sS_(<8 x i16> %a, <8 x i16> %b) {
entry:
  %a.trunc = trunc <8 x i16> %a to <8 x i1>
  %b.trunc = trunc <8 x i16> %b to <8 x i1>
  %cmp = icmp eq <8 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <8 x i1> %cmp to <8 x i16>
  ret <8 x i16> %cmp.sext
}

define <8 x i16> @_Z8icmpi1NeDv8_sS_(<8 x i16> %a, <8 x i16> %b) {
entry:
  %a.trunc = trunc <8 x i16> %a to <8 x i1>
  %b.trunc = trunc <8 x i16> %b to <8 x i1>
  %cmp = icmp ne <8 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <8 x i1> %cmp to <8 x i16>
  ret <8 x i16> %cmp.sext
}

define <8 x i16> @_Z9icmpi1UgtDv8_sS_(<8 x i16> %a, <8 x i16> %b) {
entry:
  %a.trunc = trunc <8 x i16> %a to <8 x i1>
  %b.trunc = trunc <8 x i16> %b to <8 x i1>
  %cmp = icmp ugt <8 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <8 x i1> %cmp to <8 x i16>
  ret <8 x i16> %cmp.sext
}

define <8 x i16> @_Z9icmpi1UgeDv8_sS_(<8 x i16> %a, <8 x i16> %b) {
entry:
  %a.trunc = trunc <8 x i16> %a to <8 x i1>
  %b.trunc = trunc <8 x i16> %b to <8 x i1>
  %cmp = icmp uge <8 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <8 x i1> %cmp to <8 x i16>
  ret <8 x i16> %cmp.sext
}

define <8 x i16> @_Z9icmpi1UltDv8_sS_(<8 x i16> %a, <8 x i16> %b) {
entry:
  %a.trunc = trunc <8 x i16> %a to <8 x i1>
  %b.trunc = trunc <8 x i16> %b to <8 x i1>
  %cmp = icmp ult <8 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <8 x i1> %cmp to <8 x i16>
  ret <8 x i16> %cmp.sext
}

define <8 x i16> @_Z9icmpi1UleDv8_sS_(<8 x i16> %a, <8 x i16> %b) {
entry:
  %a.trunc = trunc <8 x i16> %a to <8 x i1>
  %b.trunc = trunc <8 x i16> %b to <8 x i1>
  %cmp = icmp ule <8 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <8 x i1> %cmp to <8 x i16>
  ret <8 x i16> %cmp.sext
}

define <8 x i16> @_Z9icmpi1SgtDv8_sS_(<8 x i16> %a, <8 x i16> %b) {
entry:
  %a.trunc = trunc <8 x i16> %a to <8 x i1>
  %b.trunc = trunc <8 x i16> %b to <8 x i1>
  %cmp = icmp sgt <8 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <8 x i1> %cmp to <8 x i16>
  ret <8 x i16> %cmp.sext
}

define <8 x i16> @_Z9icmpi1SgeDv8_sS_(<8 x i16> %a, <8 x i16> %b) {
entry:
  %a.trunc = trunc <8 x i16> %a to <8 x i1>
  %b.trunc = trunc <8 x i16> %b to <8 x i1>
  %cmp = icmp sge <8 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <8 x i1> %cmp to <8 x i16>
  ret <8 x i16> %cmp.sext
}

define <8 x i16> @_Z9icmpi1SltDv8_sS_(<8 x i16> %a, <8 x i16> %b) {
entry:
  %a.trunc = trunc <8 x i16> %a to <8 x i1>
  %b.trunc = trunc <8 x i16> %b to <8 x i1>
  %cmp = icmp slt <8 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <8 x i1> %cmp to <8 x i16>
  ret <8 x i16> %cmp.sext
}

define <8 x i16> @_Z9icmpi1SleDv8_sS_(<8 x i16> %a, <8 x i16> %b) {
entry:
  %a.trunc = trunc <8 x i16> %a to <8 x i1>
  %b.trunc = trunc <8 x i16> %b to <8 x i1>
  %cmp = icmp sle <8 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <8 x i1> %cmp to <8 x i16>
  ret <8 x i16> %cmp.sext
}

define <4 x i32> @_Z8icmpi1EqDv4_iS_(<4 x i32> %a, <4 x i32> %b) {
entry:
  %a.trunc = trunc <4 x i32> %a to <4 x i1>
  %b.trunc = trunc <4 x i32> %b to <4 x i1>
  %cmp = icmp eq <4 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %cmp.sext
}

define <4 x i32> @_Z8icmpi1NeDv4_iS_(<4 x i32> %a, <4 x i32> %b) {
entry:
  %a.trunc = trunc <4 x i32> %a to <4 x i1>
  %b.trunc = trunc <4 x i32> %b to <4 x i1>
  %cmp = icmp ne <4 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %cmp.sext
}

define <4 x i32> @_Z9icmpi1UgtDv4_iS_(<4 x i32> %a, <4 x i32> %b) {
entry:
  %a.trunc = trunc <4 x i32> %a to <4 x i1>
  %b.trunc = trunc <4 x i32> %b to <4 x i1>
  %cmp = icmp ugt <4 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %cmp.sext
}

define <4 x i32> @_Z9icmpi1UgeDv4_iS_(<4 x i32> %a, <4 x i32> %b) {
entry:
  %a.trunc = trunc <4 x i32> %a to <4 x i1>
  %b.trunc = trunc <4 x i32> %b to <4 x i1>
  %cmp = icmp uge <4 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %cmp.sext
}

define <4 x i32> @_Z9icmpi1UltDv4_iS_(<4 x i32> %a, <4 x i32> %b) {
entry:
  %a.trunc = trunc <4 x i32> %a to <4 x i1>
  %b.trunc = trunc <4 x i32> %b to <4 x i1>
  %cmp = icmp ult <4 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %cmp.sext
}

define <4 x i32> @_Z9icmpi1UleDv4_iS_(<4 x i32> %a, <4 x i32> %b) {
entry:
  %a.trunc = trunc <4 x i32> %a to <4 x i1>
  %b.trunc = trunc <4 x i32> %b to <4 x i1>
  %cmp = icmp ule <4 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %cmp.sext
}

define <4 x i32> @_Z9icmpi1SgtDv4_iS_(<4 x i32> %a, <4 x i32> %b) {
entry:
  %a.trunc = trunc <4 x i32> %a to <4 x i1>
  %b.trunc = trunc <4 x i32> %b to <4 x i1>
  %cmp = icmp sgt <4 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %cmp.sext
}

define <4 x i32> @_Z9icmpi1SgeDv4_iS_(<4 x i32> %a, <4 x i32> %b) {
entry:
  %a.trunc = trunc <4 x i32> %a to <4 x i1>
  %b.trunc = trunc <4 x i32> %b to <4 x i1>
  %cmp = icmp sge <4 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %cmp.sext
}

define <4 x i32> @_Z9icmpi1SltDv4_iS_(<4 x i32> %a, <4 x i32> %b) {
entry:
  %a.trunc = trunc <4 x i32> %a to <4 x i1>
  %b.trunc = trunc <4 x i32> %b to <4 x i1>
  %cmp = icmp slt <4 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %cmp.sext
}

define <4 x i32> @_Z9icmpi1SleDv4_iS_(<4 x i32> %a, <4 x i32> %b) {
entry:
  %a.trunc = trunc <4 x i32> %a to <4 x i1>
  %b.trunc = trunc <4 x i32> %b to <4 x i1>
  %cmp = icmp sle <4 x i1> %a.trunc, %b.trunc
  %cmp.sext = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %cmp.sext
}
