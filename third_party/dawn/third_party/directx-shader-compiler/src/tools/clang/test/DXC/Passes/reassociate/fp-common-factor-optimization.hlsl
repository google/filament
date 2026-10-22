// RUN: %dxc -T cs_6_3 -E cs_main %s -opt-enable aggressive-reassociation | FileCheck %s -check-prefixes=CHECK,COMMON_FACTOR
// RUN: %dxc -T cs_6_3 -E cs_main %s -opt-disable aggressive-reassociation | FileCheck %s -check-prefixes=CHECK,NO_COMMON_FACTOR

// Make sure DXC recognize the common factor and generate optimized dxils.

// CHECK: [[TMP_CBUF:%.*]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 10, i1 false)

// CHECK: [[ELEM_1:%.*]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle [[TMP_CBUF]], i32 1)
// CHECK: [[FACTOR_SRC1:%.*]] = extractvalue %dx.types.CBufRet.f32 [[ELEM_1]], 2

// CHECK: [[ELEM_0:%.*]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle [[TMP_CBUF]], i32 0)
// CHECK: [[FACTOR_SRC0:%.*]] = extractvalue %dx.types.CBufRet.f32 [[ELEM_0]], 0

// COMMON_FACTOR: [[FACTOR:%.*]] = fmul fast float [[FACTOR_SRC0]], [[FACTOR_SRC1]]
// COMMON_FACTOR: fmul fast float [[FACTOR]],
// COMMON_FACTOR: fmul fast float [[FACTOR]],
// COMMON_FACTOR: fmul fast float [[FACTOR]],

// NO_COMMON_FACTOR: [[EXPRESSION_0:%.*]] = fmul fast float [[FACTOR_SRC1]],
// NO_COMMON_FACTOR:                        fmul fast float [[EXPRESSION_0]], [[FACTOR_SRC0]]
// NO_COMMON_FACTOR: [[EXPRESSION_1:%.*]] = fmul fast float [[FACTOR_SRC1]],
// NO_COMMON_FACTOR:                        fmul fast float [[EXPRESSION_1]], [[FACTOR_SRC0]] 
// NO_COMMON_FACTOR: [[EXPRESSION_2:%.*]] = fmul fast float [[FACTOR_SRC1]],
// NO_COMMON_FACTOR:                        fmul fast float [[EXPRESSION_2]], [[FACTOR_SRC0]]


cbuffer TemporalAAData : register ( b10 )
{
    float2 viewportRelativeSize ;
    float4 outputDimensions ;
}

Texture2D < float4 > HistoryColor : register ( t2 ) ;
SamplerState s_Linear : register ( s1 ) ;
RWTexture1D < float3 > outColorBuffer : register ( u0 ) ;

float4 Test ( in Texture2D < float4 > tex , in SamplerState linearSampler , in float2 uv )
{
    float2 samplePos = uv;
    float2 texPos1 = floor ( samplePos - 0.5f ) + 0.5f ;

    float2 texPos0 = texPos1 - 1 ;
    float2 texPos3 = texPos1 + 2 ;
    float2 texPos12 = texPos1 + samplePos;

    // DXC should recognize (outputDimensions . zw * viewportRelativeSize) is a common factor.
    texPos0 *= outputDimensions . zw * viewportRelativeSize ;
    texPos3 *= outputDimensions . zw * viewportRelativeSize ;
    texPos12 *= outputDimensions . zw * viewportRelativeSize ;

    float4 result = 0.0f ;
    result += tex . SampleLevel ( linearSampler , float2 ( texPos0 . x , texPos0 . y ) , 0.0f );
    result += tex . SampleLevel ( linearSampler , float2 ( texPos12 . x , texPos0 . y ) , 0.0f );
    result += tex . SampleLevel ( linearSampler , float2 ( texPos3 . x , texPos0 . y ) , 0.0f );
    result += tex . SampleLevel ( linearSampler , float2 ( texPos0 . x , texPos12 . y ) , 0.0f );
    result += tex . SampleLevel ( linearSampler , float2 ( texPos12 . x , texPos12 . y ) , 0.0f );
    result += tex . SampleLevel ( linearSampler , float2 ( texPos3 . x , texPos12 . y ) , 0.0f );
    result += tex . SampleLevel ( linearSampler , float2 ( texPos0 . x , texPos3 . y ) , 0.0f );
    result += tex . SampleLevel ( linearSampler , float2 ( texPos12 . x , texPos3 . y ) , 0.0f );
    result += tex . SampleLevel ( linearSampler , float2 ( texPos3 . x , texPos3 . y ) , 0.0f );
    return result ;
}

[ numthreads ( 8 , 8 , 1 ) ] void cs_main ( uint3 GroupID : SV_GroupID , uint GroupIndex : SV_GroupIndex , uint3 GTID : SV_GroupThreadID , uint3 DispatchThreadID : SV_DispatchThreadID )
{
    uint2 pixelCoord = GTID . xy ;

    outColorBuffer [ pixelCoord.x ] = Test ( HistoryColor , s_Linear , outputDimensions . zw ) . rgb;
}