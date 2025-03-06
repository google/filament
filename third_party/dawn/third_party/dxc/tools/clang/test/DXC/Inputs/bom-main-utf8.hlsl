// Tests main and include files with/without BOM to ensure BOM is stripped.

#include "bom-inc-ascii.hlsli"
#include "bom-inc-utf8.hlsli"
#include "bom-inc-utf16le.hlsli"

// TODO: Add support for Big Endian and UTF-32
// #include "bom-inc-utf16be.hlsli"
// #include "bom-inc-utf32le.hlsli"
// #include "bom-inc-utf32be.hlsli"

float4 main() : SV_Target {
  return f_ascii
        + f_utf8
        + f_utf16le
        // + f_utf16be
        // + f_utf32le
        // + f_utf32be
        ;
}
