## Overview

This folder contains unit tests covering [HLSL intrinsics](https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-intrinsic-functions).

## Logical grouping of Intrinsics

- atomic
- barrier
- basic : (abs, fma, frac, frexp, ldexp, dot, mad, max, min)
- bitwise: (countbits, firstbithigh, firstbitlow, reversebits, ...}
- boolean: (all, any, ...?)
- cast: (asfloat, asuint, asdouble, f16tof32, f32tof16, ...)
- CheckAccessFullyMapped - there really isn't a category of intrinsics that fit with this
- compound: (normalize, step, smoothstep, lerp, saturate, lit, distance, length, cross, rcp, exp, log, log10, pow, dst, reflect, refract, modf, ...)
  Intrinsics that are expanded to native instructions or other intrinsics during lowering, but we may intend
  to map to new (or existing) native dxil intrinsics in the future (to make more efficient backend expansion possible).
  List not accurate/complete.  
  **TODO**: Double check and fill in. Some may move to basic, some to helper. 
- fpspecial: (isfinite, isinf, isnan, ...)
- helper: (D3DCOLORtoUBYTE4, Process\*, degrees, radians, faceforward)
- matrix: (determinant, transpose) - Thought of putting mul here, but...
- mixed: (dot2add, dot4add, msad4) (specialized mixed precision and/or fused operations)
- mul: this has so many overloads, it deserves its own folder.
- pixel: (clip, GetRenderTargetSampleCount, GetRenderTargetSamplePosition)
- pixel/attr: (Evaluate\*, GetAttributeAtVertex).
  There could also be more testing of barycentrics, using GetAttributeAtVertex, under shader_targets/pixel/barycentrics or something.
- pixel/deriv: (ddx\*, ddy\*)
- power: (exp2, log2, rsqrt, sqrt)
- rounding: (round, ceil, floor, trunc)
- trig: (asin, atan*, cos*, sin*, tan*, ...)
- unsupported: (noise) - unsupported intrinsics tested to make sure they result in expected error message.
- unsupported/debug: (abort, errorf, printf) - Future: not yet supported by dxc.
- unsupported/legacy_tex (tex*)
- wave: - breakdown groups according to sections in spec, which makes a lot of sense...
- wave/broadcast: (WaveReadLaneFirst, WaveReadLaneAt)
- wave/prefix: (WavePrefixCountBits, WavePrefixProduct, WavePrefixSum)
- wave/quad: (QuadReadAcrossX, QuadReadAcrossY, QuadReadAcrossDiagonal, QuadReadLaneAt)
- wave/quad/pixel: - test in pixel context
- wave/quad/compute: - test in compute context (newly enabled)
- wave/query: (WaveIsFirstLane, WaveGetLaneCount, WaveGetLaneIndex)
- wave/reduction: (WaveActiveAllEqual[Bool], WaveActiveCountBits, WaveActive[Sum|Product], WaveActiveBit*, WaveActive[Min|Max])
- wave/vote: (WaveActive[Any|All]True, WaveActiveBallot)


## Test Coverage Gap
Current we have missing/minimal test coverage for below HLSL intrinsics:

- abs
- AllMemoryBarrierWithGroupSync
- any
- asdouble
- ceil
- CheckAccessFullyMapped
- clamp
- cross
- ddx
- ddx_coarse
- ddx_fine
- ddy
- ddy_coarse
- ddy_fine
- degrees
- DeviceMemoryBarrierWithGroupSync
- distance
- dst
- EvaluateAttributeSnapped
- exp2
- f16tof32
- f32tof16
- floor
- fmod
- GetRenderTargetSampleCount
- GetRenderTargetSamplePosition
- GroupMemoryBarrier
- GroupMemoryBarrierWithGroupSync
- isinf
- isnan
- ldexp
- length
- lerp
- log10
- log2
- msad4
- Process\* intrinsics
- radians
- reflect
- refract
- sincos
- trunc

## Unsupported Intrinsics
- abort
- errorf
- printf
- noise
- tex\* intrinsics

## Additional Notes
- CheckAccessFullyMapped should be tested as part of resource tests that test Sample/Load/Gather method overloads with status output.
- tex\* intrinsics are legacy DX9 intrinsics not supported by dxc.