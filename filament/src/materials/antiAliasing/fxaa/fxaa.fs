// ES 3.0/3.1 gives us the ARB_gpu_shader5 bits we need
#define gpu_shader5        1

// ES 3.0 does not have gather though
#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
#define FXAA_GATHER4_ALPHA 1
#else
#define FXAA_GATHER4_ALPHA 0
#endif
#define FXAA_PC_CONSOLE    1
#define FXAA_GLSL_130      1

#define G3D_FXAA_PATCHES   1

#if POST_PROCESS_OPAQUE
#   define FXAA_GREEN_AS_LUMA 0
#else
#   define FXAA_GREEN_AS_LUMA 1
#endif

// This substitute for the built-in "mix" function exists to work around #732,
// seen with Vulkan on the Pixel 3 + Android P.
vec4 lerp(const vec4 x, const vec4 y, float a) {
    return x * (1.0 - a) + y * a;
}

/**
  G3D version of FXAA. See copyright and warranty statement below.

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2018, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

/*============================================================================


                    NVIDIA FXAA 3.11 by TIMOTHY LOTTES
               (modified for G3D with bug fixes and PC GLSL preamble)

------------------------------------------------------------------------------
COPYRIGHT (C) 2010, 2011 NVIDIA CORPORATION. ALL RIGHTS RESERVED.
------------------------------------------------------------------------------
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR
LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION,
OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE
THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES.

------------------------------------------------------------------------------
                           INTEGRATION CHECKLIST
------------------------------------------------------------------------------
(1.)
In the shader source, setup defines for the desired configuration.
When providing multiple shaders (for different presets),
simply setup the defines differently in multiple files.
Example,

  #define FXAA_PC 1
  #define FXAA_HLSL_5 1
  #define FXAA_QUALITY_PRESET 12

Or,

  #define FXAA_360 1
  
Or,

  #define FXAA_PS3 1
  
Etc.

(2.)
Then include this file,

  include "Fxaa3_11.h"

(3.)
Then call the FXAA pixel shader from within your desired shader.
Look at the FXAA Quality FxaaPixelShader() for docs on inputs.
As for FXAA 3.11 all inputs for all shaders are the same 
to enable easy porting between platforms.

  return FxaaPixelShader(...);

(4.)
Insure pass prior to FXAA outputs RGBL (see next section).
Or use,

  #define FXAA_GREEN_AS_LUMA 1

(5.)
Setup engine to provide the following constants
which are used in the FxaaPixelShader() inputs,

  FxaaFloat2 fxaaQualityRcpFrame,
  FxaaFloat4 fxaaConsoleRcpFrameOpt,
  FxaaFloat4 fxaaConsoleRcpFrameOpt2,
  FxaaFloat fxaaQualitySubpix,
  FxaaFloat fxaaQualityEdgeThreshold,
  FxaaFloat fxaaQualityEdgeThresholdMin,
  FxaaFloat fxaaConsoleEdgeSharpness,
  FxaaFloat fxaaConsoleEdgeThreshold,
  FxaaFloat fxaaConsoleEdgeThresholdMin

Look at the FXAA Quality FxaaPixelShader() for docs on inputs.

(6.)
Have FXAA vertex shader run as a full screen triangle,
and output "pos" and "fxaaConsolePosPos" 
such that inputs in the pixel shader provide,

  // {xy} = center of pixel
  FxaaFloat2 pos,

  // {xy__} = upper left of pixel
  // {__zw} = lower right of pixel
  FxaaFloat4 fxaaConsolePosPos,

(7.)
Insure the texture sampler(s) used by FXAA are set to bilinear filtering.


------------------------------------------------------------------------------
                    INTEGRATION - RGBL AND COLORSPACE
------------------------------------------------------------------------------
FXAA3 requires RGBL as input unless the following is set, 

  #define FXAA_GREEN_AS_LUMA 1

In which case the engine uses green in place of luma,
and requires RGB input is in a non-linear colorspace.

RGB should be LDR (low dynamic range).
Specifically do FXAA after tonemapping.

RGB data as returned by a texture fetch can be non-linear,
or linear when FXAA_GREEN_AS_LUMA is not set.
Note an "sRGB format" texture counts as linear,
because the result of a texture fetch is linear data.
Regular "RGBA8" textures in the sRGB colorspace are non-linear.

If FXAA_GREEN_AS_LUMA is not set,
luma must be stored in the alpha channel prior to running FXAA.
This luma should be in a perceptual space (could be gamma 2.0).
Example pass before FXAA where output is gamma 2.0 encoded,

  color.rgb = ToneMap(color.rgb); // linear color output
  color.rgb = sqrt(color.rgb);    // gamma 2.0 color output
  return color;

To use FXAA,

  color.rgb = ToneMap(color.rgb);  // linear color output
  color.rgb = sqrt(color.rgb);     // gamma 2.0 color output
  color.a = dot(color.rgb, FxaaFloat3(0.299, 0.587, 0.114)); // compute luma
  return color;

Another example where output is linear encoded,
say for instance writing to an sRGB formated render target,
where the render target does the conversion back to sRGB after blending,

  color.rgb = ToneMap(color.rgb); // linear color output
  return color;

To use FXAA,

  color.rgb = ToneMap(color.rgb); // linear color output
  color.a = sqrt(dot(color.rgb, FxaaFloat3(0.299, 0.587, 0.114))); // compute luma
  return color;

Getting luma correct is required for the algorithm to work correctly.


------------------------------------------------------------------------------
                          BEING LINEARLY CORRECT?
------------------------------------------------------------------------------
Applying FXAA to a framebuffer with linear RGB color will look worse.
This is very counter intuitive, but happends to be true in this case.
The reason is because dithering artifacts will be more visible 
in a linear colorspace.


------------------------------------------------------------------------------
                             COMPLEX INTEGRATION
------------------------------------------------------------------------------
Q. What if the engine is blending into RGB before wanting to run FXAA?

A. In the last opaque pass prior to FXAA,
   have the pass write out luma into alpha.
   Then blend into RGB only.
   FXAA should be able to run ok
   assuming the blending pass did not any add aliasing.
   This should be the common case for particles and common blending passes.

A. Or use FXAA_GREEN_AS_LUMA.

============================================================================*/

/*============================================================================

                             INTEGRATION KNOBS

============================================================================*/
/*==========================================================================*/
#ifndef FXAA_PC
    //
    // FXAA Quality
    // The high quality PC algorithm.
    //
    #define FXAA_PC 0
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_PC_CONSOLE
    //
    // The console algorithm for PC is included
    // for developers targeting really low spec machines.
    // Likely better to just run FXAA_PC, and use a really low preset.
    //
    #define FXAA_PC_CONSOLE 0
#endif

#ifndef G3D_FXAA_PATCHES
#   define G3D_FXAA_PATCHES 0
#endif

/*--------------------------------------------------------------------------*/
#ifndef FXAA_GLSL_120
    #define FXAA_GLSL_120 0
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_GLSL_130
    #define FXAA_GLSL_130 0
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_HLSL_3
    #define FXAA_HLSL_3 0
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_HLSL_4
    #define FXAA_HLSL_4 0
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_HLSL_5
    #define FXAA_HLSL_5 0
#endif
/*--------------------------------------------------------------------------*/
/*==========================================================================*/
#ifndef FXAA_GREEN_AS_LUMA
    //
    // For those using non-linear color,
    // and either not able to get luma in alpha, or not wanting to,
    // this enables FXAA to run using green as a proxy for luma.
    // So with this enabled, no need to pack luma in alpha.
    //
    // This will turn off AA on anything which lacks some amount of green.
    // Pure red and blue or combination of only R and B, will get no AA.
    //
    // Might want to lower the settings for both,
    //    fxaaConsoleEdgeThresholdMin
    //    fxaaQualityEdgeThresholdMin
    // In order to insure AA does not get turned off on colors 
    // which contain a minor amount of green.
    //
    // 1 = On.
    // 0 = Off.
    //
    #define FXAA_GREEN_AS_LUMA 0
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_EARLY_EXIT
    //
    // Controls algorithm's early exit path.
    // On PS3 turning this ON adds 2 cycles to the shader.
    // On 360 turning this OFF adds 10ths of a millisecond to the shader.
    // Turning this off on console will result in a more blurry image.
    // So this defaults to on.
    //
    // 1 = On.
    // 0 = Off.
    //
    #define FXAA_EARLY_EXIT 1
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_DISCARD
    //
    // Only valid for PC OpenGL currently.
    // Probably will not work when FXAA_GREEN_AS_LUMA = 1.
    //
    // 1 = Use discard on pixels which don't need AA.
    //     For APIs which enable concurrent TEX+ROP from same surface.
    // 0 = Return unchanged color on pixels which don't need AA.
    //
    #define FXAA_DISCARD 0
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_FAST_PIXEL_OFFSET
    //
    // Used for GLSL 120 only.
    //
    // 1 = GL API supports fast pixel offsets
    // 0 = do not use fast pixel offsets
    //
    #ifdef GL_EXT_gpu_shader4
        #define FXAA_FAST_PIXEL_OFFSET 1
    #endif
    #ifdef GL_NV_gpu_shader5
        #define FXAA_FAST_PIXEL_OFFSET 1
    #endif
    #ifdef gpu_shader5
        #define FXAA_FAST_PIXEL_OFFSET 1
    #endif
    #ifndef FXAA_FAST_PIXEL_OFFSET
        #define FXAA_FAST_PIXEL_OFFSET 0
    #endif
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_GATHER4_ALPHA
    //
    // 1 = API supports gather4 on alpha channel.
    // 0 = API does not support gather4 on alpha channel.
    //
    #if (FXAA_HLSL_5 == 1)
        #define FXAA_GATHER4_ALPHA 1
    #endif
    #ifdef gpu_shader5
        #define FXAA_GATHER4_ALPHA 1
    #endif
    #ifdef GL_NV_gpu_shader5
        #define FXAA_GATHER4_ALPHA 1
    #endif
    #ifndef FXAA_GATHER4_ALPHA
        #define FXAA_GATHER4_ALPHA 0
    #endif
#endif

/*============================================================================
                        FXAA QUALITY - TUNING KNOBS
------------------------------------------------------------------------------
NOTE the other tuning knobs are now in the shader function inputs!
============================================================================*/
#ifndef FXAA_QUALITY_PRESET
    //
    // Choose the quality preset.
    // This needs to be compiled into the shader as it affects code.
    // Best option to include multiple presets is to 
    // in each shader define the preset, then include this file.
    // 
    // OPTIONS
    // -----------------------------------------------------------------------
    // 10 to 15 - default medium dither (10=fastest, 15=highest quality)
    // 20 to 29 - less dither, more expensive (20=fastest, 29=highest quality)
    // 39       - no dither, very expensive 
    //
    // NOTES
    // -----------------------------------------------------------------------
    // 12 = slightly faster then FXAA 3.9 and higher edge quality (default)
    // 13 = about same speed as FXAA 3.9 and better than 12
    // 23 = closest to FXAA 3.9 visually and performance wise
    //  _ = the lowest digit is directly related to performance
    // _  = the highest digit is directly related to style
    // 
    #define FXAA_QUALITY_PRESET 12
#endif

/*============================================================================

                                API PORTING

============================================================================*/
#if (FXAA_GLSL_120 == 1) || (FXAA_GLSL_130 == 1)
    #define FxaaBool bool
    #define FxaaDiscard discard
    #define FxaaFloat float
    #define FxaaFloat2 vec2
    #define FxaaFloat3 vec3
    #define FxaaFloat4 vec4
    #define FxaaHalf float
    #define FxaaHalf2 vec2
    #define FxaaHalf3 vec3
    #define FxaaHalf4 vec4
    #define FxaaInt2 ivec2
    #define FxaaSat(x) clamp(x, 0.0, 1.0)
    #define FxaaTex sampler2D
#else
    #define FxaaBool bool
    #define FxaaDiscard clip(-1)
    #define FxaaFloat float
    #define FxaaFloat2 float2
    #define FxaaFloat3 float3
    #define FxaaFloat4 float4
    #define FxaaHalf half
    #define FxaaHalf2 half2
    #define FxaaHalf3 half3
    #define FxaaHalf4 half4
    #define FxaaSat(x) saturate(x)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_GLSL_120 == 1)
    // Requires,
    //  #version 120
    // And at least,
    //  #extension GL_EXT_gpu_shader4 : enable
    //  (or set FXAA_FAST_PIXEL_OFFSET 1 to work like DX9)
    #define FxaaTexTop(t, p) texture2DLod(t, p, 0.0)
    #if (FXAA_FAST_PIXEL_OFFSET == 1)
        #define FxaaTexOff(t, p, o, r) texture2DLodOffset(t, p, 0.0, o)
    #else
        #define FxaaTexOff(t, p, o, r) texture2DLod(t, p + (o * r), 0.0)
    #endif
    #if (FXAA_GATHER4_ALPHA == 1)
        // use #extension gpu_shader5 : enable
        #define FxaaTexAlpha4(t, p) textureGather(t, p, 3)
        #define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)
        #define FxaaTexGreen4(t, p) textureGather(t, p, 1)
        #define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)
    #endif
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_GLSL_130 == 1)
    // Requires "#version 130" or better
    #define FxaaTexTop(t, p) textureLod(t, p, 0.0)
    #define FxaaTexOff(t, p, o, r) textureLodOffset(t, p, 0.0, o)
    #if (FXAA_GATHER4_ALPHA == 1)
        // use #extension gpu_shader5 : enable
        #define FxaaTexAlpha4(t, p) textureGather(t, p, 3)
        #define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)
        #define FxaaTexGreen4(t, p) textureGather(t, p, 1)
        #define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)
    #endif
#endif
/*--------------------------------------------------------------------------*/


/*============================================================================
                   GREEN AS LUMA OPTION SUPPORT FUNCTION
============================================================================*/
// NOTE: Using macros instead of functions here prevents the SPIR-V optimizer
//       from unnecessarily using high precision local variables
#if (FXAA_GREEN_AS_LUMA == 0)
//    FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.w; }
    #define FxaaLuma(rgba) (rgba).w
#else
//    FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.y; }
    #define FxaaLuma(rgba) (rgba).y
#endif    

/*============================================================================

                         FXAA3 CONSOLE - PC VERSION
                         
------------------------------------------------------------------------------
Instead of using this on PC, I'd suggest just using FXAA Quality with
    #define FXAA_QUALITY_PRESET 10
Or 
    #define FXAA_QUALITY_PRESET 20
Either are higher quality and almost as fast as this on modern PC GPUs.
============================================================================*/
#if (FXAA_PC_CONSOLE == 1)
/*--------------------------------------------------------------------------*/
FxaaFloat4 fxaa(
    //
    // Use noperspective interpolation here (turn off perspective interpolation).
    // {xy} = center of pixel
    highp FxaaFloat2 pos,
    //
    // Used only for FXAA Console, and not used on the 360 version.
    // Use noperspective interpolation here (turn off perspective interpolation).
    // {xy__} = upper left of pixel
    // {__zw} = lower right of pixel
    highp FxaaFloat4 fxaaConsolePosPos,
    //
    // Input color texture.
    // {rgb_} = color in linear or perceptual color space
    // if (FXAA_GREEN_AS_LUMA == 0)
    //     {___a} = luma in perceptual color space (not linear)
    FxaaTex tex,
    //
    // Only used on FXAA Console.
    // This must be from a constant/uniform.
    // This effects sub-pixel AA quality and inversely sharpness.
    //   Where N ranges between,
    //     N = 0.50 (default)
    //     N = 0.33 (sharper)
    // {__z_} =  N/screenWidthInPixels
    // {___w} =  N/screenHeightInPixels
    highp FxaaFloat2 fxaaConsoleRcpFrameOpt,
    //
    // Only used on FXAA Console.
    // Not used on 360, but used on PS3 and PC.
    // This must be from a constant/uniform.
    // {__z_} =  2.0/screenWidthInPixels
    // {___w} =  2.0/screenHeightInPixels
    highp FxaaFloat2 fxaaConsoleRcpFrameOpt2,
    //
    // Only used on FXAA Console.
    // This used to be the FXAA_CONSOLE__EDGE_SHARPNESS define.
    // It is here now to allow easier tuning.
    // This does not effect PS3, as this needs to be compiled in.
    //   Use FXAA_CONSOLE__PS3_EDGE_SHARPNESS for PS3.
    //   Due to the PS3 being ALU bound,
    //   there are only three safe values here: 2 and 4 and 8.
    //   These options use the shaders ability to a free *|/ by 2|4|8.
    // For all other platforms can be a non-power of two.
    //   8.0 is sharper (default!!!)
    //   4.0 is softer
    //   2.0 is really soft (good only for vector graphics inputs)
    FxaaFloat fxaaConsoleEdgeSharpness,
    //
    // Only used on FXAA Console.
    // This used to be the FXAA_CONSOLE__EDGE_THRESHOLD define.
    // It is here now to allow easier tuning.
    // This does not effect PS3, as this needs to be compiled in.
    //   Use FXAA_CONSOLE__PS3_EDGE_THRESHOLD for PS3.
    //   Due to the PS3 being ALU bound,
    //   there are only two safe values here: 1/4 and 1/8.
    //   These options use the shaders ability to a free *|/ by 2|4|8.
    // The console setting has a different mapping than the quality setting.
    // Other platforms can use other values.
    //   0.125 leaves less aliasing, but is softer (default!!!)
    //   0.25 leaves more aliasing, and is sharper
    FxaaFloat fxaaConsoleEdgeThreshold,
    //
    // Only used on FXAA Console.
    // This used to be the FXAA_CONSOLE__EDGE_THRESHOLD_MIN define.
    // It is here now to allow easier tuning.
    // Trims the algorithm from processing darks.
    // The console setting has a different mapping than the quality setting.
    // This only applies when FXAA_EARLY_EXIT is 1.
    // This does not apply to PS3,
    // PS3 was simplified to avoid more shader instructions.
    //   0.06 - faster but more aliasing in darks
    //   0.05 - default
    //   0.04 - slower and less aliasing in darks
    // Special notes when using FXAA_GREEN_AS_LUMA,
    //   Likely want to set this to zero.
    //   As colors that are mostly not-green
    //   will appear very dark in the green channel!
    //   Tune by looking at mostly non-green content,
    //   then start at zero and increase until aliasing is a problem.
    FxaaFloat fxaaConsoleEdgeThresholdMin
) {
    // Corners: average luma of four corners
    FxaaFloat lumaNw = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.xy));
    FxaaFloat lumaSw = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.xw));
    FxaaFloat lumaNe = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.zy));
    FxaaFloat lumaSe = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.zw));

    // Read the middle RGB and luma
    FxaaFloat4 rgbyM = FxaaTexTop(tex, pos.xy);
    FxaaFloat lumaM = FxaaLuma(rgbyM);

    // Find the largest and smallest luminance value of the four corners.
    FxaaFloat lumaMaxNwSw = max(lumaNw, lumaSw);
    // Lottes' original bias to avoid later divide by zero.  We bias
    // the actual term required below.  In each case, the bias should be
    // less than one intensity gradiation at 8-bit
#   if G3D_FXAA_PATCHES == 0
        lumaNe += 1.0 / 384.0;
#   endif
    FxaaFloat lumaMinNwSw = min(lumaNw, lumaSw);
    FxaaFloat lumaMaxNeSe = max(lumaNe, lumaSe);
    FxaaFloat lumaMinNeSe = min(lumaNe, lumaSe);
    FxaaFloat lumaMax = max(lumaMaxNeSe, lumaMaxNwSw);
    FxaaFloat lumaMin = min(lumaMinNeSe, lumaMinNwSw);

    // The threshold for a luminance edge
    FxaaFloat lumaMaxScaled = lumaMax * fxaaConsoleEdgeThreshold;

    // Min, including the center
    FxaaFloat lumaMinM = min(lumaMin, lumaM);
    FxaaFloat lumaMaxScaledClamped = max(fxaaConsoleEdgeThresholdMin, lumaMaxScaled);

    // Max, including the center
    FxaaFloat lumaMaxM = max(lumaMax, lumaM);

    // Total range within a 3x3 neighborhood, for thresholding
    // whether it is worth appling AA
    FxaaFloat lumaMaxSubMinM = lumaMaxM - lumaMinM;

    // If the entire range is less than the edge threshold, apply no AA
    if (lumaMaxSubMinM < lumaMaxScaledClamped) { return rgbyM; }

    // Diagonal gradient (biased to avoid a later divide by zero)
    FxaaFloat dirSwMinusNe = lumaSw - lumaNe;

    // Other diagonal gradient
    FxaaFloat dirSeMinusNw = lumaSe - lumaNw;

    // Tangent to the edge
    FxaaFloat2 dir = FxaaFloat2(
        dirSwMinusNe + dirSeMinusNw, // == (SW + SE) - (NE + NW) ~= S - N
        dirSwMinusNe - dirSeMinusNw  // == (SW + NW) - (SE + NE) ~= W - E
    );

    // Avoid a division by 0 when normalizing dir
    // Should we instead do the following?
    //     FxaaFloat2 dir1 = dir.xy / (length(dir) + MEDIUMP_FLT_MIN);
    // Or even the following?
    //     FxaaFloat2 dir1 = dirLength < MEDIUMP_FLT_MIN ? FxaaFloat2(0.0) : dir.xy / dirLength
    FxaaFloat dirLength = length(dir.xy);
    if (dirLength < MEDIUMP_FLT_MIN) { return rgbyM; }

    FxaaFloat2 dir1 = dir.xy / dirLength;

    // Step one pixel-width along the tangent in both the positive and negative half
    // spaces and sample there
    FxaaFloat4 rgbyN1 = FxaaTexTop(tex, pos.xy - dir1 * fxaaConsoleRcpFrameOpt.xy);
    FxaaFloat4 rgbyP1 = FxaaTexTop(tex, pos.xy + dir1 * fxaaConsoleRcpFrameOpt.xy);

#   if G3D_FXAA_PATCHES
        // A second sample up to two pixels away along the tangent.  This is a "min" in the original.
        //  We increase the distance (i.e., amount of blur) as the luma contrast increases.
        FxaaFloat dirAbsMinTimesC = max(abs(dir1.x), abs(dir1.y)) * fxaaConsoleEdgeSharpness * 0.015;
        FxaaFloat2 dir2 = dir1.xy * min(lumaMaxSubMinM / dirAbsMinTimesC, 3.0);
#   else
        FxaaFloat dirAbsMinTimesC = min(abs(dir1.x), abs(dir1.y)) * fxaaConsoleEdgeSharpness;
        FxaaFloat2 dir2 = clamp(dir1.xy / dirAbsMinTimesC, vec2(-2.0), vec2(2.0));
#   endif
    FxaaFloat4 rgbyN2 = FxaaTexTop(tex, pos.xy - dir2 * fxaaConsoleRcpFrameOpt2.xy);
    FxaaFloat4 rgbyP2 = FxaaTexTop(tex, pos.xy + dir2 * fxaaConsoleRcpFrameOpt2.xy);

    // (twice the) average of adjacent pixels on the tangent directions
    FxaaFloat4 rgbyA = rgbyN1 + rgbyP1;

    // Average of four adjacent pixels on the tangent direction
    FxaaFloat4 rgbyB = ((rgbyN2 + rgbyP2) * 0.25) + (rgbyA * 0.25);

    // If the 4-tap average along the tangent directions is much different from
    // the range of the original, go back to the 2-tap average. 
    // Otherwise, use the 4-tap average.  Considering this threshold avoids some
    // ringing and blurring at object edges
    FxaaBool twoTap = (FxaaLuma(rgbyB) < lumaMin) || (FxaaLuma(rgbyB) > lumaMax);
    if (twoTap) { rgbyB.xyz = rgbyA.xyz * 0.5; }

#   if G3D_FXAA_PATCHES
        // Keep some of the original contribution to avoid thin lines degrading completely
        // and overblurring. This is an addition to the original Lottes algorithm
        // rgbyB = sqrt(lerp(rgbyB * rgbyB, rgbyM * rgbyM, 0.25));  // Luminance preserving
        rgbyB = lerp(rgbyB, rgbyM, 0.25); // Faster
#   endif
    return rgbyB; 
}
/*==========================================================================*/
#endif
