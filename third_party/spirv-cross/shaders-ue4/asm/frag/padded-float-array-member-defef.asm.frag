; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 3107
; Schema: 0
               OpCapability Shader
               OpCapability Geometry
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPS "main" %in_var_TEXCOORD0 %gl_FragCoord %gl_Layer %out_var_SV_Target0
               OpExecutionMode %MainPS OriginUpperLeft
               OpSource HLSL 600
               OpName %type__Globals "type.$Globals"
               OpMemberName %type__Globals 0 "MappingPolynomial"
               OpMemberName %type__Globals 1 "InverseGamma"
               OpMemberName %type__Globals 2 "ColorMatrixR_ColorCurveCd1"
               OpMemberName %type__Globals 3 "ColorMatrixG_ColorCurveCd3Cm3"
               OpMemberName %type__Globals 4 "ColorMatrixB_ColorCurveCm2"
               OpMemberName %type__Globals 5 "ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3"
               OpMemberName %type__Globals 6 "ColorCurve_Ch1_Ch2"
               OpMemberName %type__Globals 7 "ColorShadow_Luma"
               OpMemberName %type__Globals 8 "ColorShadow_Tint1"
               OpMemberName %type__Globals 9 "ColorShadow_Tint2"
               OpMemberName %type__Globals 10 "FilmSlope"
               OpMemberName %type__Globals 11 "FilmToe"
               OpMemberName %type__Globals 12 "FilmShoulder"
               OpMemberName %type__Globals 13 "FilmBlackClip"
               OpMemberName %type__Globals 14 "FilmWhiteClip"
               OpMemberName %type__Globals 15 "LUTWeights"
               OpMemberName %type__Globals 16 "ColorScale"
               OpMemberName %type__Globals 17 "OverlayColor"
               OpMemberName %type__Globals 18 "WhiteTemp"
               OpMemberName %type__Globals 19 "WhiteTint"
               OpMemberName %type__Globals 20 "ColorSaturation"
               OpMemberName %type__Globals 21 "ColorContrast"
               OpMemberName %type__Globals 22 "ColorGamma"
               OpMemberName %type__Globals 23 "ColorGain"
               OpMemberName %type__Globals 24 "ColorOffset"
               OpMemberName %type__Globals 25 "ColorSaturationShadows"
               OpMemberName %type__Globals 26 "ColorContrastShadows"
               OpMemberName %type__Globals 27 "ColorGammaShadows"
               OpMemberName %type__Globals 28 "ColorGainShadows"
               OpMemberName %type__Globals 29 "ColorOffsetShadows"
               OpMemberName %type__Globals 30 "ColorSaturationMidtones"
               OpMemberName %type__Globals 31 "ColorContrastMidtones"
               OpMemberName %type__Globals 32 "ColorGammaMidtones"
               OpMemberName %type__Globals 33 "ColorGainMidtones"
               OpMemberName %type__Globals 34 "ColorOffsetMidtones"
               OpMemberName %type__Globals 35 "ColorSaturationHighlights"
               OpMemberName %type__Globals 36 "ColorContrastHighlights"
               OpMemberName %type__Globals 37 "ColorGammaHighlights"
               OpMemberName %type__Globals 38 "ColorGainHighlights"
               OpMemberName %type__Globals 39 "ColorOffsetHighlights"
               OpMemberName %type__Globals 40 "ColorCorrectionShadowsMax"
               OpMemberName %type__Globals 41 "ColorCorrectionHighlightsMin"
               OpMemberName %type__Globals 42 "OutputDevice"
               OpMemberName %type__Globals 43 "OutputGamut"
               OpMemberName %type__Globals 44 "BlueCorrection"
               OpMemberName %type__Globals 45 "ExpandGamut"
               OpName %_Globals "$Globals"
               OpName %type_2d_image "type.2d.image"
               OpName %Texture1 "Texture1"
               OpName %type_sampler "type.sampler"
               OpName %Texture1Sampler "Texture1Sampler"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPS "MainPS"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorateString %in_var_TEXCOORD0 UserSemantic "TEXCOORD0"
               OpDecorate %in_var_TEXCOORD0 NoPerspective
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorateString %gl_FragCoord UserSemantic "SV_POSITION"
               OpDecorate %gl_Layer BuiltIn Layer
               OpDecorateString %gl_Layer UserSemantic "SV_RenderTargetArrayIndex"
               OpDecorate %gl_Layer Flat
               OpDecorateString %out_var_SV_Target0 UserSemantic "SV_Target0"
               OpDecorate %in_var_TEXCOORD0 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %_Globals DescriptorSet 0
               OpDecorate %_Globals Binding 0
               OpDecorate %Texture1 DescriptorSet 0
               OpDecorate %Texture1 Binding 0
               OpDecorate %Texture1Sampler DescriptorSet 0
               OpDecorate %Texture1Sampler Binding 0
               OpDecorate %_arr_float_uint_5 ArrayStride 16
               OpMemberDecorate %type__Globals 0 Offset 0
               OpMemberDecorate %type__Globals 1 Offset 16
               OpMemberDecorate %type__Globals 2 Offset 32
               OpMemberDecorate %type__Globals 3 Offset 48
               OpMemberDecorate %type__Globals 4 Offset 64
               OpMemberDecorate %type__Globals 5 Offset 80
               OpMemberDecorate %type__Globals 6 Offset 96
               OpMemberDecorate %type__Globals 7 Offset 112
               OpMemberDecorate %type__Globals 8 Offset 128
               OpMemberDecorate %type__Globals 9 Offset 144
               OpMemberDecorate %type__Globals 10 Offset 160
               OpMemberDecorate %type__Globals 11 Offset 164
               OpMemberDecorate %type__Globals 12 Offset 168
               OpMemberDecorate %type__Globals 13 Offset 172
               OpMemberDecorate %type__Globals 14 Offset 176
               OpMemberDecorate %type__Globals 15 Offset 192
               OpMemberDecorate %type__Globals 16 Offset 272
               OpMemberDecorate %type__Globals 17 Offset 288
               OpMemberDecorate %type__Globals 18 Offset 304
               OpMemberDecorate %type__Globals 19 Offset 308
               OpMemberDecorate %type__Globals 20 Offset 320
               OpMemberDecorate %type__Globals 21 Offset 336
               OpMemberDecorate %type__Globals 22 Offset 352
               OpMemberDecorate %type__Globals 23 Offset 368
               OpMemberDecorate %type__Globals 24 Offset 384
               OpMemberDecorate %type__Globals 25 Offset 400
               OpMemberDecorate %type__Globals 26 Offset 416
               OpMemberDecorate %type__Globals 27 Offset 432
               OpMemberDecorate %type__Globals 28 Offset 448
               OpMemberDecorate %type__Globals 29 Offset 464
               OpMemberDecorate %type__Globals 30 Offset 480
               OpMemberDecorate %type__Globals 31 Offset 496
               OpMemberDecorate %type__Globals 32 Offset 512
               OpMemberDecorate %type__Globals 33 Offset 528
               OpMemberDecorate %type__Globals 34 Offset 544
               OpMemberDecorate %type__Globals 35 Offset 560
               OpMemberDecorate %type__Globals 36 Offset 576
               OpMemberDecorate %type__Globals 37 Offset 592
               OpMemberDecorate %type__Globals 38 Offset 608
               OpMemberDecorate %type__Globals 39 Offset 624
               OpMemberDecorate %type__Globals 40 Offset 640
               OpMemberDecorate %type__Globals 41 Offset 644
               OpMemberDecorate %type__Globals 42 Offset 648
               OpMemberDecorate %type__Globals 43 Offset 652
               OpMemberDecorate %type__Globals 44 Offset 656
               OpMemberDecorate %type__Globals 45 Offset 660
               OpDecorate %type__Globals Block
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_7 = OpConstant %uint 7
     %uint_4 = OpConstant %uint 4
%float_0_952552378 = OpConstant %float 0.952552378
    %float_0 = OpConstant %float 0

; HACK: Needed to hack this constant since MSVC and GNU libc are off by 1 ULP when converting to string (it probably still works fine though in a roundtrip ...)
%float_9_36786018en05 = OpConstant %float 9.25

%float_0_343966454 = OpConstant %float 0.343966454
%float_0_728166103 = OpConstant %float 0.728166103
%float_n0_0721325427 = OpConstant %float -0.0721325427
%float_1_00882518 = OpConstant %float 1.00882518
%float_1_04981101 = OpConstant %float 1.04981101
%float_n9_74845025en05 = OpConstant %float -9.74845025e-05
%float_n0_495903015 = OpConstant %float -0.495903015
%float_1_37331307 = OpConstant %float 1.37331307
%float_0_0982400328 = OpConstant %float 0.0982400328
%float_0_991252005 = OpConstant %float 0.991252005
%float_0_662454188 = OpConstant %float 0.662454188
%float_0_134004205 = OpConstant %float 0.134004205
%float_0_156187683 = OpConstant %float 0.156187683
%float_0_272228718 = OpConstant %float 0.272228718
%float_0_674081743 = OpConstant %float 0.674081743
%float_0_0536895171 = OpConstant %float 0.0536895171
%float_n0_00557464967 = OpConstant %float -0.00557464967
%float_0_0040607336 = OpConstant %float 0.0040607336
%float_1_01033914 = OpConstant %float 1.01033914
%float_1_6410234 = OpConstant %float 1.6410234
%float_n0_324803293 = OpConstant %float -0.324803293
%float_n0_236424699 = OpConstant %float -0.236424699
%float_n0_663662851 = OpConstant %float -0.663662851
%float_1_61533165 = OpConstant %float 1.61533165
%float_0_0167563483 = OpConstant %float 0.0167563483
%float_0_0117218941 = OpConstant %float 0.0117218941
%float_n0_00828444213 = OpConstant %float -0.00828444213
%float_0_988394856 = OpConstant %float 0.988394856
%float_1_45143926 = OpConstant %float 1.45143926
%float_n0_236510754 = OpConstant %float -0.236510754
%float_n0_214928567 = OpConstant %float -0.214928567
%float_n0_0765537769 = OpConstant %float -0.0765537769
%float_1_17622972 = OpConstant %float 1.17622972
%float_n0_0996759236 = OpConstant %float -0.0996759236
%float_0_00831614807 = OpConstant %float 0.00831614807
%float_n0_00603244966 = OpConstant %float -0.00603244966
%float_0_997716308 = OpConstant %float 0.997716308
%float_0_695452213 = OpConstant %float 0.695452213
%float_0_140678704 = OpConstant %float 0.140678704
%float_0_163869068 = OpConstant %float 0.163869068
%float_0_0447945632 = OpConstant %float 0.0447945632
%float_0_859671116 = OpConstant %float 0.859671116
%float_0_0955343172 = OpConstant %float 0.0955343172
%float_n0_00552588282 = OpConstant %float -0.00552588282
%float_0_00402521016 = OpConstant %float 0.00402521016
%float_1_00150073 = OpConstant %float 1.00150073
         %73 = OpConstantComposite %v3float %float_0_272228718 %float_0_674081743 %float_0_0536895171
%float_3_2409699 = OpConstant %float 3.2409699
%float_n1_5373832 = OpConstant %float -1.5373832
%float_n0_498610765 = OpConstant %float -0.498610765
%float_n0_969243646 = OpConstant %float -0.969243646
%float_1_8759675 = OpConstant %float 1.8759675
%float_0_0415550582 = OpConstant %float 0.0415550582
%float_0_0556300804 = OpConstant %float 0.0556300804
%float_n0_203976959 = OpConstant %float -0.203976959
%float_1_05697155 = OpConstant %float 1.05697155
%float_0_412456393 = OpConstant %float 0.412456393
%float_0_357576102 = OpConstant %float 0.357576102
%float_0_180437505 = OpConstant %float 0.180437505
%float_0_212672904 = OpConstant %float 0.212672904
%float_0_715152204 = OpConstant %float 0.715152204
%float_0_0721750036 = OpConstant %float 0.0721750036
%float_0_0193339009 = OpConstant %float 0.0193339009
%float_0_119191997 = OpConstant %float 0.119191997
%float_0_950304091 = OpConstant %float 0.950304091
%float_1_71660841 = OpConstant %float 1.71660841
%float_n0_355662107 = OpConstant %float -0.355662107
%float_n0_253360093 = OpConstant %float -0.253360093
%float_n0_666682899 = OpConstant %float -0.666682899
%float_1_61647761 = OpConstant %float 1.61647761
%float_0_0157685 = OpConstant %float 0.0157685
%float_0_0176422 = OpConstant %float 0.0176422
%float_n0_0427763015 = OpConstant %float -0.0427763015
%float_0_942228675 = OpConstant %float 0.942228675
%float_2_49339628 = OpConstant %float 2.49339628
%float_n0_93134588 = OpConstant %float -0.93134588
%float_n0_402694494 = OpConstant %float -0.402694494
%float_n0_829486787 = OpConstant %float -0.829486787
%float_1_76265967 = OpConstant %float 1.76265967
%float_0_0236246008 = OpConstant %float 0.0236246008
%float_0_0358507 = OpConstant %float 0.0358507
%float_n0_0761827007 = OpConstant %float -0.0761827007
%float_0_957014024 = OpConstant %float 0.957014024
%float_1_01303005 = OpConstant %float 1.01303005
%float_0_00610530982 = OpConstant %float 0.00610530982
%float_n0_0149710001 = OpConstant %float -0.0149710001
%float_0_00769822998 = OpConstant %float 0.00769822998
%float_0_998165011 = OpConstant %float 0.998165011
%float_n0_00503202993 = OpConstant %float -0.00503202993
%float_n0_00284131011 = OpConstant %float -0.00284131011
%float_0_00468515977 = OpConstant %float 0.00468515977
%float_0_924507022 = OpConstant %float 0.924507022
%float_0_987223983 = OpConstant %float 0.987223983
%float_n0_00611326983 = OpConstant %float -0.00611326983
%float_0_0159533005 = OpConstant %float 0.0159533005
%float_n0_00759836007 = OpConstant %float -0.00759836007
%float_1_00186002 = OpConstant %float 1.00186002
%float_0_0053300201 = OpConstant %float 0.0053300201
%float_0_00307257008 = OpConstant %float 0.00307257008
%float_n0_00509594986 = OpConstant %float -0.00509594986
%float_1_08168006 = OpConstant %float 1.08168006
  %float_0_5 = OpConstant %float 0.5
   %float_n1 = OpConstant %float -1
    %float_1 = OpConstant %float 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%float_0_015625 = OpConstant %float 0.015625
        %134 = OpConstantComposite %v2float %float_0_015625 %float_0_015625
        %135 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
     %int_43 = OpConstant %int 43
     %uint_3 = OpConstant %uint 3
        %138 = OpConstantComposite %v3float %float_0 %float_0 %float_0
      %int_9 = OpConstant %int 9
      %int_3 = OpConstant %int 3
        %141 = OpConstantComposite %v3float %float_1 %float_1 %float_1
   %float_n4 = OpConstant %float -4
     %int_45 = OpConstant %int 45
%float_0_544169128 = OpConstant %float 0.544169128
%float_0_239592597 = OpConstant %float 0.239592597
%float_0_166694298 = OpConstant %float 0.166694298
%float_0_239465594 = OpConstant %float 0.239465594
%float_0_702153027 = OpConstant %float 0.702153027
%float_0_058381401 = OpConstant %float 0.058381401
%float_n0_00234390004 = OpConstant %float -0.00234390004
%float_0_0361833982 = OpConstant %float 0.0361833982
%float_1_05521834 = OpConstant %float 1.05521834
%float_0_940437257 = OpConstant %float 0.940437257
%float_n0_0183068793 = OpConstant %float -0.0183068793
%float_0_077869609 = OpConstant %float 0.077869609
%float_0_00837869663 = OpConstant %float 0.00837869663
%float_0_828660011 = OpConstant %float 0.828660011
%float_0_162961304 = OpConstant %float 0.162961304
%float_0_00054712611 = OpConstant %float 0.00054712611
%float_n0_000883374596 = OpConstant %float -0.000883374596
%float_1_00033629 = OpConstant %float 1.00033629
%float_1_06317997 = OpConstant %float 1.06317997
%float_0_0233955998 = OpConstant %float 0.0233955998
%float_n0_0865726024 = OpConstant %float -0.0865726024
%float_n0_0106336996 = OpConstant %float -0.0106336996
%float_1_20632005 = OpConstant %float 1.20632005
%float_n0_195690006 = OpConstant %float -0.195690006
%float_n0_000590886979 = OpConstant %float -0.000590886979
%float_0_00105247996 = OpConstant %float 0.00105247996
%float_0_999538004 = OpConstant %float 0.999538004
     %int_44 = OpConstant %int 44
%float_0_9375 = OpConstant %float 0.9375
        %173 = OpConstantComposite %v3float %float_0_9375 %float_0_9375 %float_0_9375
%float_0_03125 = OpConstant %float 0.03125
        %175 = OpConstantComposite %v3float %float_0_03125 %float_0_03125 %float_0_03125
     %int_15 = OpConstant %int 15
   %float_16 = OpConstant %float 16
     %int_16 = OpConstant %int 16
     %int_17 = OpConstant %int 17
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_5 = OpConstant %uint 5
     %uint_6 = OpConstant %uint 6
      %int_2 = OpConstant %int 2
%mat3v3float = OpTypeMatrix %v3float 3
     %int_42 = OpConstant %int 42
%float_0_159301758 = OpConstant %float 0.159301758
%float_78_84375 = OpConstant %float 78.84375
%float_0_8359375 = OpConstant %float 0.8359375
%float_18_8515625 = OpConstant %float 18.8515625
%float_18_6875 = OpConstant %float 18.6875
%float_10000 = OpConstant %float 10000
%float_0_0126833133 = OpConstant %float 0.0126833133
        %194 = OpConstantComposite %v3float %float_0_0126833133 %float_0_0126833133 %float_0_0126833133
        %195 = OpConstantComposite %v3float %float_0_8359375 %float_0_8359375 %float_0_8359375
        %196 = OpConstantComposite %v3float %float_18_8515625 %float_18_8515625 %float_18_8515625
        %197 = OpConstantComposite %v3float %float_18_6875 %float_18_6875 %float_18_6875
%float_6_27739477 = OpConstant %float 6.27739477
        %199 = OpConstantComposite %v3float %float_6_27739477 %float_6_27739477 %float_6_27739477
        %200 = OpConstantComposite %v3float %float_10000 %float_10000 %float_10000
   %float_14 = OpConstant %float 14
%float_0_180000007 = OpConstant %float 0.180000007
%float_0_434017599 = OpConstant %float 0.434017599
        %204 = OpConstantComposite %v3float %float_0_434017599 %float_0_434017599 %float_0_434017599
        %205 = OpConstantComposite %v3float %float_14 %float_14 %float_14
        %206 = OpConstantComposite %v3float %float_0_180000007 %float_0_180000007 %float_0_180000007
     %int_18 = OpConstant %int 18
 %float_4000 = OpConstant %float 4000
%float_0_312700003 = OpConstant %float 0.312700003
%float_0_328999996 = OpConstant %float 0.328999996
     %int_19 = OpConstant %int 19
     %int_25 = OpConstant %int 25
     %int_20 = OpConstant %int 20
     %int_26 = OpConstant %int 26
     %int_21 = OpConstant %int 21
     %int_27 = OpConstant %int 27
     %int_22 = OpConstant %int 22
     %int_28 = OpConstant %int 28
     %int_23 = OpConstant %int 23
     %int_29 = OpConstant %int 29
     %int_24 = OpConstant %int 24
     %int_40 = OpConstant %int 40
     %int_35 = OpConstant %int 35
     %int_36 = OpConstant %int 36
     %int_37 = OpConstant %int 37
     %int_38 = OpConstant %int 38
     %int_39 = OpConstant %int 39
     %int_41 = OpConstant %int 41
     %int_30 = OpConstant %int 30
     %int_31 = OpConstant %int 31
     %int_32 = OpConstant %int 32
     %int_33 = OpConstant %int 33
     %int_34 = OpConstant %int 34
%float_0_0500000007 = OpConstant %float 0.0500000007
 %float_1_75 = OpConstant %float 1.75
%float_0_400000006 = OpConstant %float 0.400000006
%float_0_819999993 = OpConstant %float 0.819999993
%float_0_0299999993 = OpConstant %float 0.0299999993
    %float_2 = OpConstant %float 2
%float_0_959999979 = OpConstant %float 0.959999979
        %241 = OpConstantComposite %v3float %float_0_959999979 %float_0_959999979 %float_0_959999979
     %int_13 = OpConstant %int 13
     %int_11 = OpConstant %int 11
     %int_14 = OpConstant %int 14
     %int_12 = OpConstant %int 12
%float_0_800000012 = OpConstant %float 0.800000012
     %int_10 = OpConstant %int 10
   %float_10 = OpConstant %float 10
   %float_n2 = OpConstant %float -2
    %float_3 = OpConstant %float 3
        %251 = OpConstantComposite %v3float %float_3 %float_3 %float_3
        %252 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%float_0_930000007 = OpConstant %float 0.930000007
        %254 = OpConstantComposite %v3float %float_0_930000007 %float_0_930000007 %float_0_930000007
      %int_4 = OpConstant %int 4
      %int_8 = OpConstant %int 8
      %int_7 = OpConstant %int 7
      %int_5 = OpConstant %int 5
      %int_6 = OpConstant %int 6
%float_0_00200000009 = OpConstant %float 0.00200000009
        %261 = OpConstantComposite %v3float %float_0_00200000009 %float_0_00200000009 %float_0_00200000009
%float_6_10351999en05 = OpConstant %float 6.10351999e-05
        %263 = OpConstantComposite %v3float %float_6_10351999en05 %float_6_10351999en05 %float_6_10351999en05
%float_0_0404499993 = OpConstant %float 0.0404499993
        %265 = OpConstantComposite %v3float %float_0_0404499993 %float_0_0404499993 %float_0_0404499993
%float_0_947867274 = OpConstant %float 0.947867274
        %267 = OpConstantComposite %v3float %float_0_947867274 %float_0_947867274 %float_0_947867274
%float_0_0521326996 = OpConstant %float 0.0521326996
        %269 = OpConstantComposite %v3float %float_0_0521326996 %float_0_0521326996 %float_0_0521326996
%float_2_4000001 = OpConstant %float 2.4000001
        %271 = OpConstantComposite %v3float %float_2_4000001 %float_2_4000001 %float_2_4000001
%float_0_0773993805 = OpConstant %float 0.0773993805
        %273 = OpConstantComposite %v3float %float_0_0773993805 %float_0_0773993805 %float_0_0773993805
  %float_4_5 = OpConstant %float 4.5
        %275 = OpConstantComposite %v3float %float_4_5 %float_4_5 %float_4_5
%float_0_0179999992 = OpConstant %float 0.0179999992
        %277 = OpConstantComposite %v3float %float_0_0179999992 %float_0_0179999992 %float_0_0179999992
%float_0_449999988 = OpConstant %float 0.449999988
        %279 = OpConstantComposite %v3float %float_0_449999988 %float_0_449999988 %float_0_449999988
%float_1_09899998 = OpConstant %float 1.09899998
        %281 = OpConstantComposite %v3float %float_1_09899998 %float_1_09899998 %float_1_09899998
%float_0_0989999995 = OpConstant %float 0.0989999995
        %283 = OpConstantComposite %v3float %float_0_0989999995 %float_0_0989999995 %float_0_0989999995
  %float_1_5 = OpConstant %float 1.5
        %285 = OpConstantComposite %v3float %float_1_5 %float_1_5 %float_1_5
        %286 = OpConstantComposite %v3float %float_0_159301758 %float_0_159301758 %float_0_159301758
        %287 = OpConstantComposite %v3float %float_78_84375 %float_78_84375 %float_78_84375
%float_1_00055635 = OpConstant %float 1.00055635
 %float_7000 = OpConstant %float 7000
%float_0_244063005 = OpConstant %float 0.244063005
%float_99_1100006 = OpConstant %float 99.1100006
%float_2967800 = OpConstant %float 2967800
%float_0_237039998 = OpConstant %float 0.237039998
%float_247_479996 = OpConstant %float 247.479996
%float_1901800 = OpConstant %float 1901800
   %float_n3 = OpConstant %float -3
%float_2_86999989 = OpConstant %float 2.86999989
%float_0_275000006 = OpConstant %float 0.275000006
%float_0_860117733 = OpConstant %float 0.860117733
%float_0_000154118257 = OpConstant %float 0.000154118257
%float_1_28641219en07 = OpConstant %float 1.28641219e-07
%float_0_00084242021 = OpConstant %float 0.00084242021
%float_7_08145137en07 = OpConstant %float 7.08145137e-07
%float_0_317398727 = OpConstant %float 0.317398727

; HACK: Needed to hack this constant since MSVC and GNU libc are off by 1 ULP when converting to string (it probably still works fine though in a roundtrip ...)
%float_4_22806261en05 = OpConstant %float 4.25

%float_4_20481676en08 = OpConstant %float 4.20481676e-08
%float_2_8974182en05 = OpConstant %float 2.8974182e-05
%float_1_61456057en07 = OpConstant %float 1.61456057e-07
    %float_8 = OpConstant %float 8
    %float_4 = OpConstant %float 4
%float_0_895099998 = OpConstant %float 0.895099998
%float_0_266400009 = OpConstant %float 0.266400009
%float_n0_161400005 = OpConstant %float -0.161400005
%float_n0_750199974 = OpConstant %float -0.750199974
%float_1_71350002 = OpConstant %float 1.71350002
%float_0_0366999991 = OpConstant %float 0.0366999991
%float_0_0388999991 = OpConstant %float 0.0388999991
%float_n0_0684999973 = OpConstant %float -0.0684999973
%float_1_02960002 = OpConstant %float 1.02960002
%float_0_986992896 = OpConstant %float 0.986992896
%float_n0_1470543 = OpConstant %float -0.1470543
%float_0_159962699 = OpConstant %float 0.159962699
%float_0_432305306 = OpConstant %float 0.432305306
%float_0_518360317 = OpConstant %float 0.518360317
%float_0_0492912009 = OpConstant %float 0.0492912009
%float_n0_0085287001 = OpConstant %float -0.0085287001
%float_0_040042799 = OpConstant %float 0.040042799
%float_0_968486726 = OpConstant %float 0.968486726
%float_5_55555534 = OpConstant %float 5.55555534
        %330 = OpConstantComposite %v3float %float_5_55555534 %float_5_55555534 %float_5_55555534
%float_1_00000001en10 = OpConstant %float 1.00000001e-10
%float_0_00999999978 = OpConstant %float 0.00999999978
%float_0_666666687 = OpConstant %float 0.666666687
  %float_180 = OpConstant %float 180
  %float_360 = OpConstant %float 360
%float_65535 = OpConstant %float 65535
        %337 = OpConstantComposite %v3float %float_65535 %float_65535 %float_65535
%float_n4_97062206 = OpConstant %float -4.97062206
%float_n3_02937818 = OpConstant %float -3.02937818
%float_n2_12619996 = OpConstant %float -2.12619996
%float_n1_51049995 = OpConstant %float -1.51049995
%float_n1_05780005 = OpConstant %float -1.05780005
%float_n0_466800004 = OpConstant %float -0.466800004
%float_0_119379997 = OpConstant %float 0.119379997
%float_0_708813429 = OpConstant %float 0.708813429
%float_1_29118657 = OpConstant %float 1.29118657
%float_0_808913231 = OpConstant %float 0.808913231
%float_1_19108677 = OpConstant %float 1.19108677
%float_1_56830001 = OpConstant %float 1.56830001
%float_1_9483 = OpConstant %float 1.9483
%float_2_30830002 = OpConstant %float 2.30830002
%float_2_63840008 = OpConstant %float 2.63840008
%float_2_85949993 = OpConstant %float 2.85949993
%float_2_98726082 = OpConstant %float 2.98726082
%float_3_01273918 = OpConstant %float 3.01273918
%float_0_179999992 = OpConstant %float 0.179999992
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
 %float_1000 = OpConstant %float 1000
%float_0_0599999987 = OpConstant %float 0.0599999987
%float_3_50738446en05 = OpConstant %float 3.50738446e-05
        %361 = OpConstantComposite %v3float %float_3_50738446en05 %float_3_50738446en05 %float_3_50738446en05
%float_n2_30102992 = OpConstant %float -2.30102992
%float_n1_93120003 = OpConstant %float -1.93120003
%float_n1_52049994 = OpConstant %float -1.52049994
%float_0_801995218 = OpConstant %float 0.801995218
%float_1_19800484 = OpConstant %float 1.19800484
%float_1_59430003 = OpConstant %float 1.59430003
%float_1_99730003 = OpConstant %float 1.99730003
%float_2_37829995 = OpConstant %float 2.37829995
%float_2_76839995 = OpConstant %float 2.76839995
%float_3_05150008 = OpConstant %float 3.05150008
%float_3_27462935 = OpConstant %float 3.27462935
%float_3_32743073 = OpConstant %float 3.32743073
%float_0_00499999989 = OpConstant %float 0.00499999989
   %float_11 = OpConstant %float 11
 %float_2000 = OpConstant %float 2000
%float_0_119999997 = OpConstant %float 0.119999997
%float_0_00313066994 = OpConstant %float 0.00313066994
%float_12_9200001 = OpConstant %float 12.9200001
%float_0_416666657 = OpConstant %float 0.416666657
%float_1_05499995 = OpConstant %float 1.05499995
%float_0_0549999997 = OpConstant %float 0.0549999997
%float_n0_166666672 = OpConstant %float -0.166666672
 %float_n0_5 = OpConstant %float -0.5
%float_0_166666672 = OpConstant %float 0.166666672
%float_n3_15737653 = OpConstant %float -3.15737653
%float_n0_485249996 = OpConstant %float -0.485249996
%float_1_84773242 = OpConstant %float 1.84773242
%float_n0_718548238 = OpConstant %float -0.718548238
%float_2_08103061 = OpConstant %float 2.08103061
%float_3_6681242 = OpConstant %float 3.6681242
   %float_18 = OpConstant %float 18
    %float_7 = OpConstant %float 7
%_arr_float_uint_5 = OpTypeArray %float %uint_5
%type__Globals = OpTypeStruct %v4float %v3float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %float %float %float %float %float %_arr_float_uint_5 %v3float %v4float %float %float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %float %float %uint %uint %float %float
%_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
        %402 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
       %bool = OpTypeBool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
%type_sampled_image = OpTypeSampledImage %type_2d_image
    %uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%_arr_float_uint_6 = OpTypeArray %float %uint_6
   %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
   %Texture1 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%Texture1Sampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v2float Input
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
   %gl_Layer = OpVariable %_ptr_Input_uint Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function__arr_float_uint_6 = OpTypePointer Function %_arr_float_uint_6
%_ptr_Function__arr_float_uint_10 = OpTypePointer Function %_arr_float_uint_10
        %416 = OpConstantComposite %v3float %float_0_952552378 %float_0 %float_9_36786018en05
        %417 = OpConstantComposite %v3float %float_0_343966454 %float_0_728166103 %float_n0_0721325427
        %418 = OpConstantComposite %v3float %float_0 %float_0 %float_1_00882518
        %419 = OpConstantComposite %mat3v3float %416 %417 %418
        %420 = OpConstantComposite %v3float %float_1_04981101 %float_0 %float_n9_74845025en05
        %421 = OpConstantComposite %v3float %float_n0_495903015 %float_1_37331307 %float_0_0982400328
        %422 = OpConstantComposite %v3float %float_0 %float_0 %float_0_991252005
        %423 = OpConstantComposite %mat3v3float %420 %421 %422
        %424 = OpConstantComposite %v3float %float_0_662454188 %float_0_134004205 %float_0_156187683
        %425 = OpConstantComposite %v3float %float_n0_00557464967 %float_0_0040607336 %float_1_01033914
        %426 = OpConstantComposite %mat3v3float %424 %73 %425
        %427 = OpConstantComposite %v3float %float_1_6410234 %float_n0_324803293 %float_n0_236424699
        %428 = OpConstantComposite %v3float %float_n0_663662851 %float_1_61533165 %float_0_0167563483
        %429 = OpConstantComposite %v3float %float_0_0117218941 %float_n0_00828444213 %float_0_988394856
        %430 = OpConstantComposite %mat3v3float %427 %428 %429
        %431 = OpConstantComposite %v3float %float_1_45143926 %float_n0_236510754 %float_n0_214928567
        %432 = OpConstantComposite %v3float %float_n0_0765537769 %float_1_17622972 %float_n0_0996759236
        %433 = OpConstantComposite %v3float %float_0_00831614807 %float_n0_00603244966 %float_0_997716308
        %434 = OpConstantComposite %mat3v3float %431 %432 %433
        %435 = OpConstantComposite %v3float %float_0_695452213 %float_0_140678704 %float_0_163869068
        %436 = OpConstantComposite %v3float %float_0_0447945632 %float_0_859671116 %float_0_0955343172
        %437 = OpConstantComposite %v3float %float_n0_00552588282 %float_0_00402521016 %float_1_00150073
        %438 = OpConstantComposite %mat3v3float %435 %436 %437
        %439 = OpConstantComposite %v3float %float_3_2409699 %float_n1_5373832 %float_n0_498610765
        %440 = OpConstantComposite %v3float %float_n0_969243646 %float_1_8759675 %float_0_0415550582
        %441 = OpConstantComposite %v3float %float_0_0556300804 %float_n0_203976959 %float_1_05697155
        %442 = OpConstantComposite %mat3v3float %439 %440 %441
        %443 = OpConstantComposite %v3float %float_0_412456393 %float_0_357576102 %float_0_180437505
        %444 = OpConstantComposite %v3float %float_0_212672904 %float_0_715152204 %float_0_0721750036
        %445 = OpConstantComposite %v3float %float_0_0193339009 %float_0_119191997 %float_0_950304091
        %446 = OpConstantComposite %mat3v3float %443 %444 %445
        %447 = OpConstantComposite %v3float %float_1_71660841 %float_n0_355662107 %float_n0_253360093
        %448 = OpConstantComposite %v3float %float_n0_666682899 %float_1_61647761 %float_0_0157685
        %449 = OpConstantComposite %v3float %float_0_0176422 %float_n0_0427763015 %float_0_942228675
        %450 = OpConstantComposite %mat3v3float %447 %448 %449
        %451 = OpConstantComposite %v3float %float_2_49339628 %float_n0_93134588 %float_n0_402694494
        %452 = OpConstantComposite %v3float %float_n0_829486787 %float_1_76265967 %float_0_0236246008
        %453 = OpConstantComposite %v3float %float_0_0358507 %float_n0_0761827007 %float_0_957014024
        %454 = OpConstantComposite %mat3v3float %451 %452 %453
        %455 = OpConstantComposite %v3float %float_1_01303005 %float_0_00610530982 %float_n0_0149710001
        %456 = OpConstantComposite %v3float %float_0_00769822998 %float_0_998165011 %float_n0_00503202993
        %457 = OpConstantComposite %v3float %float_n0_00284131011 %float_0_00468515977 %float_0_924507022
        %458 = OpConstantComposite %mat3v3float %455 %456 %457
        %459 = OpConstantComposite %v3float %float_0_987223983 %float_n0_00611326983 %float_0_0159533005
        %460 = OpConstantComposite %v3float %float_n0_00759836007 %float_1_00186002 %float_0_0053300201
        %461 = OpConstantComposite %v3float %float_0_00307257008 %float_n0_00509594986 %float_1_08168006
        %462 = OpConstantComposite %mat3v3float %459 %460 %461
        %463 = OpConstantComposite %v3float %float_0_5 %float_n1 %float_0_5
        %464 = OpConstantComposite %v3float %float_n1 %float_1 %float_0_5
        %465 = OpConstantComposite %v3float %float_0_5 %float_0 %float_0
        %466 = OpConstantComposite %mat3v3float %463 %464 %465
        %467 = OpConstantComposite %v3float %float_1 %float_0 %float_0
        %468 = OpConstantComposite %v3float %float_0 %float_1 %float_0
        %469 = OpConstantComposite %v3float %float_0 %float_0 %float_1
        %470 = OpConstantComposite %mat3v3float %467 %468 %469
%float_n6_07624626 = OpConstant %float -6.07624626
        %472 = OpConstantComposite %v3float %float_n6_07624626 %float_n6_07624626 %float_n6_07624626
        %473 = OpConstantComposite %v3float %float_0_895099998 %float_0_266400009 %float_n0_161400005
        %474 = OpConstantComposite %v3float %float_n0_750199974 %float_1_71350002 %float_0_0366999991
        %475 = OpConstantComposite %v3float %float_0_0388999991 %float_n0_0684999973 %float_1_02960002
        %476 = OpConstantComposite %mat3v3float %473 %474 %475
        %477 = OpConstantComposite %v3float %float_0_986992896 %float_n0_1470543 %float_0_159962699
        %478 = OpConstantComposite %v3float %float_0_432305306 %float_0_518360317 %float_0_0492912009
        %479 = OpConstantComposite %v3float %float_n0_0085287001 %float_0_040042799 %float_0_968486726
        %480 = OpConstantComposite %mat3v3float %477 %478 %479
        %481 = OpConstantComposite %v3float %float_0_544169128 %float_0_239592597 %float_0_166694298
        %482 = OpConstantComposite %v3float %float_0_239465594 %float_0_702153027 %float_0_058381401
        %483 = OpConstantComposite %v3float %float_n0_00234390004 %float_0_0361833982 %float_1_05521834
        %484 = OpConstantComposite %mat3v3float %481 %482 %483
        %485 = OpConstantComposite %v3float %float_0_940437257 %float_n0_0183068793 %float_0_077869609
        %486 = OpConstantComposite %v3float %float_0_00837869663 %float_0_828660011 %float_0_162961304
        %487 = OpConstantComposite %v3float %float_0_00054712611 %float_n0_000883374596 %float_1_00033629
        %488 = OpConstantComposite %mat3v3float %485 %486 %487
        %489 = OpConstantComposite %v3float %float_1_06317997 %float_0_0233955998 %float_n0_0865726024
        %490 = OpConstantComposite %v3float %float_n0_0106336996 %float_1_20632005 %float_n0_195690006
        %491 = OpConstantComposite %v3float %float_n0_000590886979 %float_0_00105247996 %float_0_999538004
        %492 = OpConstantComposite %mat3v3float %489 %490 %491
%float_0_0533333346 = OpConstant %float 0.0533333346
%float_0_159999996 = OpConstant %float 0.159999996
%float_57_2957764 = OpConstant %float 57.2957764
%float_0_0625 = OpConstant %float 0.0625
%float_n67_5 = OpConstant %float -67.5
 %float_67_5 = OpConstant %float 67.5
        %499 = OpConstantComposite %_arr_float_uint_6 %float_n4 %float_n4 %float_n3_15737653 %float_n0_485249996 %float_1_84773242 %float_1_84773242
        %500 = OpConstantComposite %_arr_float_uint_6 %float_n0_718548238 %float_2_08103061 %float_3_6681242 %float_4 %float_4 %float_4
  %float_n15 = OpConstant %float -15
  %float_n14 = OpConstant %float -14
        %503 = OpConstantComposite %_arr_float_uint_10 %float_n4_97062206 %float_n3_02937818 %float_n2_12619996 %float_n1_51049995 %float_n1_05780005 %float_n0_466800004 %float_0_119379997 %float_0_708813429 %float_1_29118657 %float_1_29118657
        %504 = OpConstantComposite %_arr_float_uint_10 %float_0_808913231 %float_1_19108677 %float_1_56830001 %float_1_9483 %float_2_30830002 %float_2_63840008 %float_2_85949993 %float_2_98726082 %float_3_01273918 %float_3_01273918
  %float_n12 = OpConstant %float -12
        %506 = OpConstantComposite %_arr_float_uint_10 %float_n2_30102992 %float_n2_30102992 %float_n1_93120003 %float_n1_52049994 %float_n1_05780005 %float_n0_466800004 %float_0_119379997 %float_0_708813429 %float_1_29118657 %float_1_29118657
        %507 = OpConstantComposite %_arr_float_uint_10 %float_0_801995218 %float_1_19800484 %float_1_59430003 %float_1_99730003 %float_2_37829995 %float_2_76839995 %float_3_05150008 %float_3_27462935 %float_3_32743073 %float_3_32743073
%float_0_0322580636 = OpConstant %float 0.0322580636
%float_1_03225803 = OpConstant %float 1.03225803
        %510 = OpConstantComposite %v2float %float_1_03225803 %float_1_03225803
%float_4_60443853e_09 = OpConstant %float 4.60443853e+09
%float_2_00528435e_09 = OpConstant %float 2.00528435e+09
%float_0_333333343 = OpConstant %float 0.333333343
    %float_5 = OpConstant %float 5
  %float_2_5 = OpConstant %float 2.5
%float_0_0250000004 = OpConstant %float 0.0250000004
%float_0_239999995 = OpConstant %float 0.239999995
%float_0_0148148146 = OpConstant %float 0.0148148146
        %519 = OpConstantComposite %v3float %float_9_99999975en05 %float_9_99999975en05 %float_9_99999975en05
%float_0_0296296291 = OpConstant %float 0.0296296291
%float_0_952381015 = OpConstant %float 0.952381015
        %522 = OpConstantComposite %v3float %float_0_952381015 %float_0_952381015 %float_0_952381015
        %523 = OpUndef %v3float
%float_0_358299971 = OpConstant %float 0.358299971
        %525 = OpUndef %v3float
     %MainPS = OpFunction %void None %402
        %526 = OpLabel
        %527 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %528 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %529 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %530 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %531 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %532 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %533 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %534 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %535 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %536 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %537 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %538 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %539 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %540 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %541 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %542 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %543 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %544 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %545 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %546 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %547 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %548 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %549 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %550 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %551 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %552 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %553 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %554 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %555 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %556 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %557 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %558 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %559 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %560 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %561 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %562 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %563 = OpLoad %v2float %in_var_TEXCOORD0
        %564 = OpLoad %uint %gl_Layer
        %565 = OpFSub %v2float %563 %134
        %566 = OpFMul %v2float %565 %510
        %567 = OpCompositeExtract %float %566 0
        %568 = OpCompositeExtract %float %566 1
        %569 = OpConvertUToF %float %564
        %570 = OpFMul %float %569 %float_0_0322580636
        %571 = OpCompositeConstruct %v4float %567 %568 %570 %float_0
        %572 = OpMatrixTimesMatrix %mat3v3float %446 %458
        %573 = OpMatrixTimesMatrix %mat3v3float %572 %430
        %574 = OpMatrixTimesMatrix %mat3v3float %426 %462
        %575 = OpMatrixTimesMatrix %mat3v3float %574 %442
        %576 = OpMatrixTimesMatrix %mat3v3float %419 %430
        %577 = OpMatrixTimesMatrix %mat3v3float %426 %423
        %578 = OpAccessChain %_ptr_Uniform_uint %_Globals %int_43
        %579 = OpLoad %uint %578
               OpBranch %580
        %580 = OpLabel
               OpLoopMerge %581 %582 None
               OpBranch %583
        %583 = OpLabel
        %584 = OpMatrixTimesMatrix %mat3v3float %574 %454
        %585 = OpMatrixTimesMatrix %mat3v3float %574 %450
        %586 = OpIEqual %bool %579 %uint_1
               OpSelectionMerge %587 None
               OpBranchConditional %586 %588 %589
        %589 = OpLabel
        %590 = OpIEqual %bool %579 %uint_2
               OpSelectionMerge %591 None
               OpBranchConditional %590 %592 %593
        %593 = OpLabel
        %594 = OpIEqual %bool %579 %uint_3
               OpSelectionMerge %595 None
               OpBranchConditional %594 %596 %597
        %597 = OpLabel
        %598 = OpIEqual %bool %579 %uint_4
               OpSelectionMerge %599 None
               OpBranchConditional %598 %600 %601
        %601 = OpLabel
               OpBranch %581
        %600 = OpLabel
               OpBranch %581
        %599 = OpLabel
               OpUnreachable
        %596 = OpLabel
               OpBranch %581
        %595 = OpLabel
               OpUnreachable
        %592 = OpLabel
               OpBranch %581
        %591 = OpLabel
               OpUnreachable
        %588 = OpLabel
               OpBranch %581
        %587 = OpLabel
               OpUnreachable
        %582 = OpLabel
               OpBranch %580
        %581 = OpLabel
        %602 = OpPhi %mat3v3float %575 %601 %470 %600 %438 %596 %585 %592 %584 %588
        %603 = OpVectorShuffle %v3float %571 %571 0 1 2
        %604 = OpAccessChain %_ptr_Uniform_uint %_Globals %int_42
        %605 = OpLoad %uint %604
        %606 = OpUGreaterThanEqual %bool %605 %uint_3
               OpSelectionMerge %607 None
               OpBranchConditional %606 %608 %609
        %609 = OpLabel
        %610 = OpFSub %v3float %603 %204
        %611 = OpFMul %v3float %610 %205
        %612 = OpExtInst %v3float %1 Exp2 %611
        %613 = OpFMul %v3float %612 %206
        %614 = OpExtInst %v3float %1 Exp2 %472
        %615 = OpFMul %v3float %614 %206
        %616 = OpFSub %v3float %613 %615
               OpBranch %607
        %608 = OpLabel
        %617 = OpExtInst %v3float %1 Pow %603 %194
        %618 = OpFSub %v3float %617 %195
        %619 = OpExtInst %v3float %1 FMax %138 %618
        %620 = OpFMul %v3float %197 %617
        %621 = OpFSub %v3float %196 %620
        %622 = OpFDiv %v3float %619 %621
        %623 = OpExtInst %v3float %1 Pow %622 %199
        %624 = OpFMul %v3float %623 %200
               OpBranch %607
        %607 = OpLabel
        %625 = OpPhi %v3float %616 %609 %624 %608
        %626 = OpAccessChain %_ptr_Uniform_float %_Globals %int_18
        %627 = OpLoad %float %626
        %628 = OpFMul %float %627 %float_1_00055635
        %629 = OpFOrdLessThanEqual %bool %628 %float_7000
        %630 = OpFDiv %float %float_4_60443853e_09 %627
        %631 = OpFSub %float %float_2967800 %630
        %632 = OpFDiv %float %631 %628
        %633 = OpFAdd %float %float_99_1100006 %632
        %634 = OpFDiv %float %633 %628
        %635 = OpFAdd %float %float_0_244063005 %634
        %636 = OpFDiv %float %float_2_00528435e_09 %627
        %637 = OpFSub %float %float_1901800 %636
        %638 = OpFDiv %float %637 %628
        %639 = OpFAdd %float %float_247_479996 %638
        %640 = OpFDiv %float %639 %628
        %641 = OpFAdd %float %float_0_237039998 %640
        %642 = OpSelect %float %629 %635 %641
        %643 = OpFMul %float %float_n3 %642
        %644 = OpFMul %float %643 %642
        %645 = OpFMul %float %float_2_86999989 %642
        %646 = OpFAdd %float %644 %645
        %647 = OpFSub %float %646 %float_0_275000006
        %648 = OpCompositeConstruct %v2float %642 %647
        %649 = OpFMul %float %float_0_000154118257 %627
        %650 = OpFAdd %float %float_0_860117733 %649
        %651 = OpFMul %float %float_1_28641219en07 %627
        %652 = OpFMul %float %651 %627
        %653 = OpFAdd %float %650 %652
        %654 = OpFMul %float %float_0_00084242021 %627
        %655 = OpFAdd %float %float_1 %654
        %656 = OpFMul %float %float_7_08145137en07 %627
        %657 = OpFMul %float %656 %627
        %658 = OpFAdd %float %655 %657
        %659 = OpFDiv %float %653 %658
        %660 = OpFMul %float %float_4_22806261en05 %627
        %661 = OpFAdd %float %float_0_317398727 %660
        %662 = OpFMul %float %float_4_20481676en08 %627
        %663 = OpFMul %float %662 %627
        %664 = OpFAdd %float %661 %663
        %665 = OpFMul %float %float_2_8974182en05 %627
        %666 = OpFSub %float %float_1 %665
        %667 = OpFMul %float %float_1_61456057en07 %627
        %668 = OpFMul %float %667 %627
        %669 = OpFAdd %float %666 %668
        %670 = OpFDiv %float %664 %669
        %671 = OpFMul %float %float_3 %659
        %672 = OpFMul %float %float_2 %659
        %673 = OpFMul %float %float_8 %670
        %674 = OpFSub %float %672 %673
        %675 = OpFAdd %float %674 %float_4
        %676 = OpFDiv %float %671 %675
        %677 = OpFMul %float %float_2 %670
        %678 = OpFDiv %float %677 %675
        %679 = OpCompositeConstruct %v2float %676 %678
        %680 = OpFOrdLessThan %bool %627 %float_4000
        %681 = OpCompositeConstruct %v2bool %680 %680
        %682 = OpSelect %v2float %681 %679 %648
        %683 = OpAccessChain %_ptr_Uniform_float %_Globals %int_19
        %684 = OpLoad %float %683
        %685 = OpCompositeConstruct %v2float %659 %670
        %686 = OpExtInst %v2float %1 Normalize %685
        %687 = OpCompositeExtract %float %686 1
        %688 = OpFNegate %float %687
        %689 = OpFMul %float %688 %684
        %690 = OpFMul %float %689 %float_0_0500000007
        %691 = OpFAdd %float %659 %690
        %692 = OpCompositeExtract %float %686 0
        %693 = OpFMul %float %692 %684
        %694 = OpFMul %float %693 %float_0_0500000007
        %695 = OpFAdd %float %670 %694
        %696 = OpFMul %float %float_3 %691
        %697 = OpFMul %float %float_2 %691
        %698 = OpFMul %float %float_8 %695
        %699 = OpFSub %float %697 %698
        %700 = OpFAdd %float %699 %float_4
        %701 = OpFDiv %float %696 %700
        %702 = OpFMul %float %float_2 %695
        %703 = OpFDiv %float %702 %700
        %704 = OpCompositeConstruct %v2float %701 %703
        %705 = OpFSub %v2float %704 %679
        %706 = OpFAdd %v2float %682 %705
        %707 = OpCompositeExtract %float %706 0
        %708 = OpCompositeExtract %float %706 1
        %709 = OpExtInst %float %1 FMax %708 %float_1_00000001en10
        %710 = OpFDiv %float %707 %709
        %711 = OpCompositeInsert %v3float %710 %523 0
        %712 = OpCompositeInsert %v3float %float_1 %711 1
        %713 = OpFSub %float %float_1 %707
        %714 = OpFSub %float %713 %708
        %715 = OpFDiv %float %714 %709
        %716 = OpCompositeInsert %v3float %715 %712 2
        %717 = OpExtInst %float %1 FMax %float_0_328999996 %float_1_00000001en10
        %718 = OpFDiv %float %float_0_312700003 %717
        %719 = OpCompositeInsert %v3float %718 %523 0
        %720 = OpCompositeInsert %v3float %float_1 %719 1
        %721 = OpFDiv %float %float_0_358299971 %717
        %722 = OpCompositeInsert %v3float %721 %720 2
        %723 = OpVectorTimesMatrix %v3float %716 %476
        %724 = OpVectorTimesMatrix %v3float %722 %476
        %725 = OpCompositeExtract %float %724 0
        %726 = OpCompositeExtract %float %723 0
        %727 = OpFDiv %float %725 %726
        %728 = OpCompositeConstruct %v3float %727 %float_0 %float_0
        %729 = OpCompositeExtract %float %724 1
        %730 = OpCompositeExtract %float %723 1
        %731 = OpFDiv %float %729 %730
        %732 = OpCompositeConstruct %v3float %float_0 %731 %float_0
        %733 = OpCompositeExtract %float %724 2
        %734 = OpCompositeExtract %float %723 2
        %735 = OpFDiv %float %733 %734
        %736 = OpCompositeConstruct %v3float %float_0 %float_0 %735
        %737 = OpCompositeConstruct %mat3v3float %728 %732 %736
        %738 = OpMatrixTimesMatrix %mat3v3float %476 %737
        %739 = OpMatrixTimesMatrix %mat3v3float %738 %480
        %740 = OpMatrixTimesMatrix %mat3v3float %446 %739
        %741 = OpMatrixTimesMatrix %mat3v3float %740 %442
        %742 = OpVectorTimesMatrix %v3float %625 %741
        %743 = OpVectorTimesMatrix %v3float %742 %573
        %744 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_9
        %745 = OpAccessChain %_ptr_Uniform_float %_Globals %int_9 %int_3
        %746 = OpLoad %float %745
        %747 = OpFOrdNotEqual %bool %746 %float_0
               OpSelectionMerge %748 None
               OpBranchConditional %747 %749 %748
        %749 = OpLabel
        %750 = OpDot %float %743 %73
        %751 = OpCompositeConstruct %v3float %750 %750 %750
        %752 = OpFDiv %v3float %743 %751
        %753 = OpFSub %v3float %752 %141
        %754 = OpDot %float %753 %753
        %755 = OpFMul %float %float_n4 %754
        %756 = OpExtInst %float %1 Exp2 %755
        %757 = OpFSub %float %float_1 %756
        %758 = OpAccessChain %_ptr_Uniform_float %_Globals %int_45
        %759 = OpLoad %float %758
        %760 = OpFMul %float %float_n4 %759
        %761 = OpFMul %float %760 %750
        %762 = OpFMul %float %761 %750
        %763 = OpExtInst %float %1 Exp2 %762
        %764 = OpFSub %float %float_1 %763
        %765 = OpFMul %float %757 %764
        %766 = OpMatrixTimesMatrix %mat3v3float %484 %430
        %767 = OpMatrixTimesMatrix %mat3v3float %575 %766
        %768 = OpVectorTimesMatrix %v3float %743 %767
        %769 = OpCompositeConstruct %v3float %765 %765 %765
        %770 = OpExtInst %v3float %1 FMix %743 %768 %769
               OpBranch %748
        %748 = OpLabel
        %771 = OpPhi %v3float %743 %607 %770 %749
        %772 = OpDot %float %771 %73
        %773 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_25
        %774 = OpLoad %v4float %773
        %775 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_20
        %776 = OpLoad %v4float %775
        %777 = OpFMul %v4float %774 %776
        %778 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_26
        %779 = OpLoad %v4float %778
        %780 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_21
        %781 = OpLoad %v4float %780
        %782 = OpFMul %v4float %779 %781
        %783 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_27
        %784 = OpLoad %v4float %783
        %785 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_22
        %786 = OpLoad %v4float %785
        %787 = OpFMul %v4float %784 %786
        %788 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_28
        %789 = OpLoad %v4float %788
        %790 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_23
        %791 = OpLoad %v4float %790
        %792 = OpFMul %v4float %789 %791
        %793 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_29
        %794 = OpLoad %v4float %793
        %795 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_24
        %796 = OpLoad %v4float %795
        %797 = OpFAdd %v4float %794 %796
        %798 = OpCompositeConstruct %v3float %772 %772 %772
        %799 = OpVectorShuffle %v3float %777 %777 0 1 2
        %800 = OpCompositeExtract %float %777 3
        %801 = OpCompositeConstruct %v3float %800 %800 %800
        %802 = OpFMul %v3float %799 %801
        %803 = OpExtInst %v3float %1 FMix %798 %771 %802
        %804 = OpExtInst %v3float %1 FMax %138 %803
        %805 = OpFMul %v3float %804 %330
        %806 = OpVectorShuffle %v3float %782 %782 0 1 2
        %807 = OpCompositeExtract %float %782 3
        %808 = OpCompositeConstruct %v3float %807 %807 %807
        %809 = OpFMul %v3float %806 %808
        %810 = OpExtInst %v3float %1 Pow %805 %809
        %811 = OpFMul %v3float %810 %206
        %812 = OpVectorShuffle %v3float %787 %787 0 1 2
        %813 = OpCompositeExtract %float %787 3
        %814 = OpCompositeConstruct %v3float %813 %813 %813
        %815 = OpFMul %v3float %812 %814
        %816 = OpFDiv %v3float %141 %815
        %817 = OpExtInst %v3float %1 Pow %811 %816
        %818 = OpVectorShuffle %v3float %792 %792 0 1 2
        %819 = OpCompositeExtract %float %792 3
        %820 = OpCompositeConstruct %v3float %819 %819 %819
        %821 = OpFMul %v3float %818 %820
        %822 = OpFMul %v3float %817 %821
        %823 = OpVectorShuffle %v3float %797 %797 0 1 2
        %824 = OpCompositeExtract %float %797 3
        %825 = OpCompositeConstruct %v3float %824 %824 %824
        %826 = OpFAdd %v3float %823 %825
        %827 = OpFAdd %v3float %822 %826
        %828 = OpAccessChain %_ptr_Uniform_float %_Globals %int_40
        %829 = OpLoad %float %828
        %830 = OpExtInst %float %1 SmoothStep %float_0 %829 %772
        %831 = OpFSub %float %float_1 %830
        %832 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_35
        %833 = OpLoad %v4float %832
        %834 = OpFMul %v4float %833 %776
        %835 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_36
        %836 = OpLoad %v4float %835
        %837 = OpFMul %v4float %836 %781
        %838 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_37
        %839 = OpLoad %v4float %838
        %840 = OpFMul %v4float %839 %786
        %841 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_38
        %842 = OpLoad %v4float %841
        %843 = OpFMul %v4float %842 %791
        %844 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_39
        %845 = OpLoad %v4float %844
        %846 = OpFAdd %v4float %845 %796
        %847 = OpVectorShuffle %v3float %834 %834 0 1 2
        %848 = OpCompositeExtract %float %834 3
        %849 = OpCompositeConstruct %v3float %848 %848 %848
        %850 = OpFMul %v3float %847 %849
        %851 = OpExtInst %v3float %1 FMix %798 %771 %850
        %852 = OpExtInst %v3float %1 FMax %138 %851
        %853 = OpFMul %v3float %852 %330
        %854 = OpVectorShuffle %v3float %837 %837 0 1 2
        %855 = OpCompositeExtract %float %837 3
        %856 = OpCompositeConstruct %v3float %855 %855 %855
        %857 = OpFMul %v3float %854 %856
        %858 = OpExtInst %v3float %1 Pow %853 %857
        %859 = OpFMul %v3float %858 %206
        %860 = OpVectorShuffle %v3float %840 %840 0 1 2
        %861 = OpCompositeExtract %float %840 3
        %862 = OpCompositeConstruct %v3float %861 %861 %861
        %863 = OpFMul %v3float %860 %862
        %864 = OpFDiv %v3float %141 %863
        %865 = OpExtInst %v3float %1 Pow %859 %864
        %866 = OpVectorShuffle %v3float %843 %843 0 1 2
        %867 = OpCompositeExtract %float %843 3
        %868 = OpCompositeConstruct %v3float %867 %867 %867
        %869 = OpFMul %v3float %866 %868
        %870 = OpFMul %v3float %865 %869
        %871 = OpVectorShuffle %v3float %846 %846 0 1 2
        %872 = OpCompositeExtract %float %846 3
        %873 = OpCompositeConstruct %v3float %872 %872 %872
        %874 = OpFAdd %v3float %871 %873
        %875 = OpFAdd %v3float %870 %874
        %876 = OpAccessChain %_ptr_Uniform_float %_Globals %int_41
        %877 = OpLoad %float %876
        %878 = OpExtInst %float %1 SmoothStep %877 %float_1 %772
        %879 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_30
        %880 = OpLoad %v4float %879
        %881 = OpFMul %v4float %880 %776
        %882 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_31
        %883 = OpLoad %v4float %882
        %884 = OpFMul %v4float %883 %781
        %885 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_32
        %886 = OpLoad %v4float %885
        %887 = OpFMul %v4float %886 %786
        %888 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_33
        %889 = OpLoad %v4float %888
        %890 = OpFMul %v4float %889 %791
        %891 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_34
        %892 = OpLoad %v4float %891
        %893 = OpFAdd %v4float %892 %796
        %894 = OpVectorShuffle %v3float %881 %881 0 1 2
        %895 = OpCompositeExtract %float %881 3
        %896 = OpCompositeConstruct %v3float %895 %895 %895
        %897 = OpFMul %v3float %894 %896
        %898 = OpExtInst %v3float %1 FMix %798 %771 %897
        %899 = OpExtInst %v3float %1 FMax %138 %898
        %900 = OpFMul %v3float %899 %330
        %901 = OpVectorShuffle %v3float %884 %884 0 1 2
        %902 = OpCompositeExtract %float %884 3
        %903 = OpCompositeConstruct %v3float %902 %902 %902
        %904 = OpFMul %v3float %901 %903
        %905 = OpExtInst %v3float %1 Pow %900 %904
        %906 = OpFMul %v3float %905 %206
        %907 = OpVectorShuffle %v3float %887 %887 0 1 2
        %908 = OpCompositeExtract %float %887 3
        %909 = OpCompositeConstruct %v3float %908 %908 %908
        %910 = OpFMul %v3float %907 %909
        %911 = OpFDiv %v3float %141 %910
        %912 = OpExtInst %v3float %1 Pow %906 %911
        %913 = OpVectorShuffle %v3float %890 %890 0 1 2
        %914 = OpCompositeExtract %float %890 3
        %915 = OpCompositeConstruct %v3float %914 %914 %914
        %916 = OpFMul %v3float %913 %915
        %917 = OpFMul %v3float %912 %916
        %918 = OpVectorShuffle %v3float %893 %893 0 1 2
        %919 = OpCompositeExtract %float %893 3
        %920 = OpCompositeConstruct %v3float %919 %919 %919
        %921 = OpFAdd %v3float %918 %920
        %922 = OpFAdd %v3float %917 %921
        %923 = OpFSub %float %830 %878
        %924 = OpCompositeConstruct %v3float %831 %831 %831
        %925 = OpFMul %v3float %827 %924
        %926 = OpCompositeConstruct %v3float %923 %923 %923
        %927 = OpFMul %v3float %922 %926
        %928 = OpFAdd %v3float %925 %927
        %929 = OpCompositeConstruct %v3float %878 %878 %878
        %930 = OpFMul %v3float %875 %929
        %931 = OpFAdd %v3float %928 %930
        %932 = OpVectorTimesMatrix %v3float %931 %575
        %933 = OpMatrixTimesMatrix %mat3v3float %577 %488
        %934 = OpMatrixTimesMatrix %mat3v3float %933 %576
        %935 = OpMatrixTimesMatrix %mat3v3float %577 %492
        %936 = OpMatrixTimesMatrix %mat3v3float %935 %576
        %937 = OpVectorTimesMatrix %v3float %931 %934
        %938 = OpAccessChain %_ptr_Uniform_float %_Globals %int_44
        %939 = OpLoad %float %938
        %940 = OpCompositeConstruct %v3float %939 %939 %939
        %941 = OpExtInst %v3float %1 FMix %931 %937 %940
        %942 = OpVectorTimesMatrix %v3float %941 %577
        %943 = OpCompositeExtract %float %942 0
        %944 = OpCompositeExtract %float %942 1
        %945 = OpExtInst %float %1 FMin %943 %944
        %946 = OpCompositeExtract %float %942 2
        %947 = OpExtInst %float %1 FMin %945 %946
        %948 = OpExtInst %float %1 FMax %943 %944
        %949 = OpExtInst %float %1 FMax %948 %946
        %950 = OpExtInst %float %1 FMax %949 %float_1_00000001en10
        %951 = OpExtInst %float %1 FMax %947 %float_1_00000001en10
        %952 = OpFSub %float %950 %951
        %953 = OpExtInst %float %1 FMax %949 %float_0_00999999978
        %954 = OpFDiv %float %952 %953
        %955 = OpFSub %float %946 %944
        %956 = OpFMul %float %946 %955
        %957 = OpFSub %float %944 %943
        %958 = OpFMul %float %944 %957
        %959 = OpFAdd %float %956 %958
        %960 = OpFSub %float %943 %946
        %961 = OpFMul %float %943 %960
        %962 = OpFAdd %float %959 %961
        %963 = OpExtInst %float %1 Sqrt %962
        %964 = OpFAdd %float %946 %944
        %965 = OpFAdd %float %964 %943
        %966 = OpFMul %float %float_1_75 %963
        %967 = OpFAdd %float %965 %966
        %968 = OpFMul %float %967 %float_0_333333343
        %969 = OpFSub %float %954 %float_0_400000006
        %970 = OpFMul %float %969 %float_5
        %971 = OpFMul %float %969 %float_2_5
        %972 = OpExtInst %float %1 FAbs %971
        %973 = OpFSub %float %float_1 %972
        %974 = OpExtInst %float %1 FMax %973 %float_0
        %975 = OpExtInst %float %1 FSign %970
        %976 = OpConvertFToS %int %975
        %977 = OpConvertSToF %float %976
        %978 = OpFMul %float %974 %974
        %979 = OpFSub %float %float_1 %978
        %980 = OpFMul %float %977 %979
        %981 = OpFAdd %float %float_1 %980
        %982 = OpFMul %float %981 %float_0_0250000004
        %983 = OpFOrdLessThanEqual %bool %968 %float_0_0533333346
               OpSelectionMerge %984 None
               OpBranchConditional %983 %985 %986
        %986 = OpLabel
        %987 = OpFOrdGreaterThanEqual %bool %968 %float_0_159999996
               OpSelectionMerge %988 None
               OpBranchConditional %987 %989 %990
        %990 = OpLabel
        %991 = OpFDiv %float %float_0_239999995 %967
        %992 = OpFSub %float %991 %float_0_5
        %993 = OpFMul %float %982 %992
               OpBranch %988
        %989 = OpLabel
               OpBranch %988
        %988 = OpLabel
        %994 = OpPhi %float %993 %990 %float_0 %989
               OpBranch %984
        %985 = OpLabel
               OpBranch %984
        %984 = OpLabel
        %995 = OpPhi %float %994 %988 %982 %985
        %996 = OpFAdd %float %float_1 %995
        %997 = OpCompositeConstruct %v3float %996 %996 %996
        %998 = OpFMul %v3float %942 %997
        %999 = OpCompositeExtract %float %998 0
       %1000 = OpCompositeExtract %float %998 1
       %1001 = OpFOrdEqual %bool %999 %1000
       %1002 = OpCompositeExtract %float %998 2
       %1003 = OpFOrdEqual %bool %1000 %1002
       %1004 = OpLogicalAnd %bool %1001 %1003
               OpSelectionMerge %1005 None
               OpBranchConditional %1004 %1006 %1007
       %1007 = OpLabel
       %1008 = OpExtInst %float %1 Sqrt %float_3
       %1009 = OpFSub %float %1000 %1002
       %1010 = OpFMul %float %1008 %1009
       %1011 = OpFMul %float %float_2 %999
       %1012 = OpFSub %float %1011 %1000
       %1013 = OpFSub %float %1012 %1002
       %1014 = OpExtInst %float %1 Atan2 %1010 %1013
       %1015 = OpFMul %float %float_57_2957764 %1014
               OpBranch %1005
       %1006 = OpLabel
               OpBranch %1005
       %1005 = OpLabel
       %1016 = OpPhi %float %1015 %1007 %float_0 %1006
       %1017 = OpFOrdLessThan %bool %1016 %float_0
               OpSelectionMerge %1018 None
               OpBranchConditional %1017 %1019 %1018
       %1019 = OpLabel
       %1020 = OpFAdd %float %1016 %float_360
               OpBranch %1018
       %1018 = OpLabel
       %1021 = OpPhi %float %1016 %1005 %1020 %1019
       %1022 = OpExtInst %float %1 FClamp %1021 %float_0 %float_360
       %1023 = OpFOrdGreaterThan %bool %1022 %float_180
               OpSelectionMerge %1024 None
               OpBranchConditional %1023 %1025 %1024
       %1025 = OpLabel
       %1026 = OpFSub %float %1022 %float_360
               OpBranch %1024
       %1024 = OpLabel
       %1027 = OpPhi %float %1022 %1018 %1026 %1025
       %1028 = OpFMul %float %1027 %float_0_0148148146
       %1029 = OpExtInst %float %1 FAbs %1028
       %1030 = OpFSub %float %float_1 %1029
       %1031 = OpExtInst %float %1 SmoothStep %float_0 %float_1 %1030
       %1032 = OpFMul %float %1031 %1031
       %1033 = OpFMul %float %1032 %954
       %1034 = OpFSub %float %float_0_0299999993 %999
       %1035 = OpFMul %float %1033 %1034
       %1036 = OpFMul %float %1035 %float_0_180000007
       %1037 = OpFAdd %float %999 %1036
       %1038 = OpCompositeInsert %v3float %1037 %998 0
       %1039 = OpVectorTimesMatrix %v3float %1038 %434
       %1040 = OpExtInst %v3float %1 FMax %138 %1039
       %1041 = OpDot %float %1040 %73
       %1042 = OpCompositeConstruct %v3float %1041 %1041 %1041
       %1043 = OpExtInst %v3float %1 FMix %1042 %1040 %241
       %1044 = OpAccessChain %_ptr_Uniform_float %_Globals %int_13
       %1045 = OpLoad %float %1044
       %1046 = OpFAdd %float %float_1 %1045
       %1047 = OpAccessChain %_ptr_Uniform_float %_Globals %int_11
       %1048 = OpLoad %float %1047
       %1049 = OpFSub %float %1046 %1048
       %1050 = OpAccessChain %_ptr_Uniform_float %_Globals %int_14
       %1051 = OpLoad %float %1050
       %1052 = OpFAdd %float %float_1 %1051
       %1053 = OpAccessChain %_ptr_Uniform_float %_Globals %int_12
       %1054 = OpLoad %float %1053
       %1055 = OpFSub %float %1052 %1054
       %1056 = OpFOrdGreaterThan %bool %1048 %float_0_800000012
               OpSelectionMerge %1057 None
               OpBranchConditional %1056 %1058 %1059
       %1059 = OpLabel
       %1060 = OpFAdd %float %float_0_180000007 %1045
       %1061 = OpFDiv %float %1060 %1049
       %1062 = OpExtInst %float %1 Log %float_0_180000007
       %1063 = OpExtInst %float %1 Log %float_10
       %1064 = OpFDiv %float %1062 %1063
       %1065 = OpFSub %float %float_2 %1061
       %1066 = OpFDiv %float %1061 %1065
       %1067 = OpExtInst %float %1 Log %1066
       %1068 = OpFMul %float %float_0_5 %1067
       %1069 = OpAccessChain %_ptr_Uniform_float %_Globals %int_10
       %1070 = OpLoad %float %1069
       %1071 = OpFDiv %float %1049 %1070
       %1072 = OpFMul %float %1068 %1071
       %1073 = OpFSub %float %1064 %1072
               OpBranch %1057
       %1058 = OpLabel
       %1074 = OpFSub %float %float_0_819999993 %1048
       %1075 = OpAccessChain %_ptr_Uniform_float %_Globals %int_10
       %1076 = OpLoad %float %1075
       %1077 = OpFDiv %float %1074 %1076
       %1078 = OpExtInst %float %1 Log %float_0_180000007
       %1079 = OpExtInst %float %1 Log %float_10
       %1080 = OpFDiv %float %1078 %1079
       %1081 = OpFAdd %float %1077 %1080
               OpBranch %1057
       %1057 = OpLabel
       %1082 = OpPhi %float %1073 %1059 %1081 %1058
       %1083 = OpFSub %float %float_1 %1048
       %1084 = OpAccessChain %_ptr_Uniform_float %_Globals %int_10
       %1085 = OpLoad %float %1084
       %1086 = OpFDiv %float %1083 %1085
       %1087 = OpFSub %float %1086 %1082
       %1088 = OpFDiv %float %1054 %1085
       %1089 = OpFSub %float %1088 %1087
       %1090 = OpExtInst %v3float %1 Log %1043
       %1091 = OpExtInst %float %1 Log %float_10
       %1092 = OpCompositeConstruct %v3float %1091 %1091 %1091
       %1093 = OpFDiv %v3float %1090 %1092
       %1094 = OpCompositeConstruct %v3float %1085 %1085 %1085
       %1095 = OpCompositeConstruct %v3float %1087 %1087 %1087
       %1096 = OpFAdd %v3float %1093 %1095
       %1097 = OpFMul %v3float %1094 %1096
       %1098 = OpFNegate %float %1045
       %1099 = OpCompositeConstruct %v3float %1098 %1098 %1098
       %1100 = OpFMul %float %float_2 %1049
       %1101 = OpCompositeConstruct %v3float %1100 %1100 %1100
       %1102 = OpFMul %float %float_n2 %1085
       %1103 = OpFDiv %float %1102 %1049
       %1104 = OpCompositeConstruct %v3float %1103 %1103 %1103
       %1105 = OpCompositeConstruct %v3float %1082 %1082 %1082
       %1106 = OpFSub %v3float %1093 %1105
       %1107 = OpFMul %v3float %1104 %1106
       %1108 = OpExtInst %v3float %1 Exp %1107
       %1109 = OpFAdd %v3float %141 %1108
       %1110 = OpFDiv %v3float %1101 %1109
       %1111 = OpFAdd %v3float %1099 %1110
       %1112 = OpCompositeConstruct %v3float %1052 %1052 %1052
       %1113 = OpFMul %float %float_2 %1055
       %1114 = OpCompositeConstruct %v3float %1113 %1113 %1113
       %1115 = OpFMul %float %float_2 %1085
       %1116 = OpFDiv %float %1115 %1055
       %1117 = OpCompositeConstruct %v3float %1116 %1116 %1116
       %1118 = OpCompositeConstruct %v3float %1089 %1089 %1089
       %1119 = OpFSub %v3float %1093 %1118
       %1120 = OpFMul %v3float %1117 %1119
       %1121 = OpExtInst %v3float %1 Exp %1120
       %1122 = OpFAdd %v3float %141 %1121
       %1123 = OpFDiv %v3float %1114 %1122
       %1124 = OpFSub %v3float %1112 %1123
       %1125 = OpFOrdLessThan %v3bool %1093 %1105
       %1126 = OpSelect %v3float %1125 %1111 %1097
       %1127 = OpFOrdGreaterThan %v3bool %1093 %1118
       %1128 = OpSelect %v3float %1127 %1124 %1097
       %1129 = OpFSub %float %1089 %1082
       %1130 = OpCompositeConstruct %v3float %1129 %1129 %1129
       %1131 = OpFDiv %v3float %1106 %1130
       %1132 = OpExtInst %v3float %1 FClamp %1131 %138 %141
       %1133 = OpFOrdLessThan %bool %1089 %1082
       %1134 = OpFSub %v3float %141 %1132
       %1135 = OpCompositeConstruct %v3bool %1133 %1133 %1133
       %1136 = OpSelect %v3float %1135 %1134 %1132
       %1137 = OpFMul %v3float %252 %1136
       %1138 = OpFSub %v3float %251 %1137
       %1139 = OpFMul %v3float %1138 %1136
       %1140 = OpFMul %v3float %1139 %1136
       %1141 = OpExtInst %v3float %1 FMix %1126 %1128 %1140
       %1142 = OpDot %float %1141 %73
       %1143 = OpCompositeConstruct %v3float %1142 %1142 %1142
       %1144 = OpExtInst %v3float %1 FMix %1143 %1141 %254
       %1145 = OpExtInst %v3float %1 FMax %138 %1144
       %1146 = OpVectorTimesMatrix %v3float %1145 %936
       %1147 = OpExtInst %v3float %1 FMix %1145 %1146 %940
       %1148 = OpVectorTimesMatrix %v3float %1147 %575
       %1149 = OpExtInst %v3float %1 FMax %138 %1148
       %1150 = OpFOrdEqual %bool %746 %float_0
               OpSelectionMerge %1151 DontFlatten
               OpBranchConditional %1150 %1152 %1151
       %1152 = OpLabel
       %1153 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_2
       %1154 = OpLoad %v4float %1153
       %1155 = OpVectorShuffle %v3float %1154 %1154 0 1 2
       %1156 = OpDot %float %932 %1155
       %1157 = OpCompositeInsert %v3float %1156 %525 0
       %1158 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_3
       %1159 = OpLoad %v4float %1158
       %1160 = OpVectorShuffle %v3float %1159 %1159 0 1 2
       %1161 = OpDot %float %932 %1160
       %1162 = OpCompositeInsert %v3float %1161 %1157 1
       %1163 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_4
       %1164 = OpLoad %v4float %1163
       %1165 = OpVectorShuffle %v3float %1164 %1164 0 1 2
       %1166 = OpDot %float %932 %1165
       %1167 = OpCompositeInsert %v3float %1166 %1162 2
       %1168 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_8
       %1169 = OpLoad %v4float %1168
       %1170 = OpVectorShuffle %v3float %1169 %1169 0 1 2
       %1171 = OpLoad %v4float %744
       %1172 = OpVectorShuffle %v3float %1171 %1171 0 1 2
       %1173 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_7
       %1174 = OpLoad %v4float %1173
       %1175 = OpVectorShuffle %v3float %1174 %1174 0 1 2
       %1176 = OpDot %float %932 %1175
       %1177 = OpFAdd %float %1176 %float_1
       %1178 = OpFDiv %float %float_1 %1177
       %1179 = OpCompositeConstruct %v3float %1178 %1178 %1178
       %1180 = OpFMul %v3float %1172 %1179
       %1181 = OpFAdd %v3float %1170 %1180
       %1182 = OpFMul %v3float %1167 %1181
       %1183 = OpExtInst %v3float %1 FMax %138 %1182
       %1184 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_5
       %1185 = OpLoad %v4float %1184
       %1186 = OpVectorShuffle %v3float %1185 %1185 0 0 0
       %1187 = OpFSub %v3float %1186 %1183
       %1188 = OpExtInst %v3float %1 FMax %138 %1187
       %1189 = OpVectorShuffle %v3float %1185 %1185 2 2 2
       %1190 = OpExtInst %v3float %1 FMax %1183 %1189
       %1191 = OpExtInst %v3float %1 FClamp %1183 %1186 %1189
       %1192 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_6
       %1193 = OpLoad %v4float %1192
       %1194 = OpVectorShuffle %v3float %1193 %1193 0 0 0
       %1195 = OpFMul %v3float %1190 %1194
       %1196 = OpVectorShuffle %v3float %1193 %1193 1 1 1
       %1197 = OpFAdd %v3float %1195 %1196
       %1198 = OpVectorShuffle %v3float %1185 %1185 3 3 3
       %1199 = OpFAdd %v3float %1190 %1198
       %1200 = OpFDiv %v3float %141 %1199
       %1201 = OpFMul %v3float %1197 %1200
       %1202 = OpVectorShuffle %v3float %1164 %1164 3 3 3
       %1203 = OpFMul %v3float %1191 %1202
       %1204 = OpVectorShuffle %v3float %1154 %1154 3 3 3
       %1205 = OpFMul %v3float %1188 %1204
       %1206 = OpVectorShuffle %v3float %1185 %1185 1 1 1
       %1207 = OpFAdd %v3float %1188 %1206
       %1208 = OpFDiv %v3float %141 %1207
       %1209 = OpFMul %v3float %1205 %1208
       %1210 = OpVectorShuffle %v3float %1159 %1159 3 3 3
       %1211 = OpFAdd %v3float %1209 %1210
       %1212 = OpFAdd %v3float %1203 %1211
       %1213 = OpFAdd %v3float %1201 %1212
       %1214 = OpFSub %v3float %1213 %261
               OpBranch %1151
       %1151 = OpLabel
       %1215 = OpPhi %v3float %1149 %1057 %1214 %1152
       %1216 = OpExtInst %v3float %1 FClamp %1215 %138 %141
       %1217 = OpCompositeExtract %float %1216 0
               OpBranch %1218
       %1218 = OpLabel
               OpLoopMerge %1219 %1220 None
               OpBranch %1221
       %1221 = OpLabel
       %1222 = OpFOrdLessThan %bool %1217 %float_0_00313066994
               OpSelectionMerge %1223 None
               OpBranchConditional %1222 %1224 %1223
       %1224 = OpLabel
       %1225 = OpFMul %float %1217 %float_12_9200001
               OpBranch %1219
       %1223 = OpLabel
       %1226 = OpExtInst %float %1 Pow %1217 %float_0_416666657
       %1227 = OpFMul %float %1226 %float_1_05499995
       %1228 = OpFSub %float %1227 %float_0_0549999997
               OpBranch %1219
       %1220 = OpLabel
               OpBranch %1218
       %1219 = OpLabel
       %1229 = OpPhi %float %1225 %1224 %1228 %1223
       %1230 = OpCompositeExtract %float %1216 1
               OpBranch %1231
       %1231 = OpLabel
               OpLoopMerge %1232 %1233 None
               OpBranch %1234
       %1234 = OpLabel
       %1235 = OpFOrdLessThan %bool %1230 %float_0_00313066994
               OpSelectionMerge %1236 None
               OpBranchConditional %1235 %1237 %1236
       %1237 = OpLabel
       %1238 = OpFMul %float %1230 %float_12_9200001
               OpBranch %1232
       %1236 = OpLabel
       %1239 = OpExtInst %float %1 Pow %1230 %float_0_416666657
       %1240 = OpFMul %float %1239 %float_1_05499995
       %1241 = OpFSub %float %1240 %float_0_0549999997
               OpBranch %1232
       %1233 = OpLabel
               OpBranch %1231
       %1232 = OpLabel
       %1242 = OpPhi %float %1238 %1237 %1241 %1236
       %1243 = OpCompositeExtract %float %1216 2
               OpBranch %1244
       %1244 = OpLabel
               OpLoopMerge %1245 %1246 None
               OpBranch %1247
       %1247 = OpLabel
       %1248 = OpFOrdLessThan %bool %1243 %float_0_00313066994
               OpSelectionMerge %1249 None
               OpBranchConditional %1248 %1250 %1249
       %1250 = OpLabel
       %1251 = OpFMul %float %1243 %float_12_9200001
               OpBranch %1245
       %1249 = OpLabel
       %1252 = OpExtInst %float %1 Pow %1243 %float_0_416666657
       %1253 = OpFMul %float %1252 %float_1_05499995
       %1254 = OpFSub %float %1253 %float_0_0549999997
               OpBranch %1245
       %1246 = OpLabel
               OpBranch %1244
       %1245 = OpLabel
       %1255 = OpPhi %float %1251 %1250 %1254 %1249
       %1256 = OpCompositeConstruct %v3float %1229 %1242 %1255
       %1257 = OpFMul %v3float %1256 %173
       %1258 = OpFAdd %v3float %1257 %175
       %1259 = OpAccessChain %_ptr_Uniform_float %_Globals %int_15 %int_0
       %1260 = OpLoad %float %1259
       %1261 = OpCompositeConstruct %v3float %1260 %1260 %1260
       %1262 = OpFMul %v3float %1261 %1256
       %1263 = OpAccessChain %_ptr_Uniform_float %_Globals %int_15 %int_1
       %1264 = OpLoad %float %1263
       %1265 = OpCompositeConstruct %v3float %1264 %1264 %1264
       %1266 = OpLoad %type_2d_image %Texture1
       %1267 = OpLoad %type_sampler %Texture1Sampler
       %1268 = OpCompositeExtract %float %1258 2
       %1269 = OpFMul %float %1268 %float_16
       %1270 = OpFSub %float %1269 %float_0_5
       %1271 = OpExtInst %float %1 Floor %1270
       %1272 = OpFSub %float %1270 %1271
       %1273 = OpCompositeExtract %float %1258 0
       %1274 = OpFAdd %float %1273 %1271
       %1275 = OpFMul %float %1274 %float_0_0625
       %1276 = OpCompositeExtract %float %1258 1
       %1277 = OpCompositeConstruct %v2float %1275 %1276
       %1278 = OpSampledImage %type_sampled_image %1266 %1267
       %1279 = OpImageSampleImplicitLod %v4float %1278 %1277 None
       %1280 = OpFAdd %float %1275 %float_0_0625
       %1281 = OpCompositeConstruct %v2float %1280 %1276
       %1282 = OpSampledImage %type_sampled_image %1266 %1267
       %1283 = OpImageSampleImplicitLod %v4float %1282 %1281 None
       %1284 = OpCompositeConstruct %v4float %1272 %1272 %1272 %1272
       %1285 = OpExtInst %v4float %1 FMix %1279 %1283 %1284
       %1286 = OpVectorShuffle %v3float %1285 %1285 0 1 2
       %1287 = OpFMul %v3float %1265 %1286
       %1288 = OpFAdd %v3float %1262 %1287
       %1289 = OpExtInst %v3float %1 FMax %263 %1288
       %1290 = OpFOrdGreaterThan %v3bool %1289 %265
       %1291 = OpFMul %v3float %1289 %267
       %1292 = OpFAdd %v3float %1291 %269
       %1293 = OpExtInst %v3float %1 Pow %1292 %271
       %1294 = OpFMul %v3float %1289 %273
       %1295 = OpSelect %v3float %1290 %1293 %1294
       %1296 = OpAccessChain %_ptr_Uniform_float %_Globals %int_0 %int_0
       %1297 = OpLoad %float %1296
       %1298 = OpCompositeConstruct %v3float %1297 %1297 %1297
       %1299 = OpFMul %v3float %1295 %1295
       %1300 = OpFMul %v3float %1298 %1299
       %1301 = OpAccessChain %_ptr_Uniform_float %_Globals %int_0 %int_1
       %1302 = OpLoad %float %1301
       %1303 = OpCompositeConstruct %v3float %1302 %1302 %1302
       %1304 = OpFMul %v3float %1303 %1295
       %1305 = OpFAdd %v3float %1300 %1304
       %1306 = OpAccessChain %_ptr_Uniform_float %_Globals %int_0 %int_2
       %1307 = OpLoad %float %1306
       %1308 = OpCompositeConstruct %v3float %1307 %1307 %1307
       %1309 = OpFAdd %v3float %1305 %1308
       %1310 = OpAccessChain %_ptr_Uniform_v3float %_Globals %int_16
       %1311 = OpLoad %v3float %1310
       %1312 = OpFMul %v3float %1309 %1311
       %1313 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_17
       %1314 = OpLoad %v4float %1313
       %1315 = OpVectorShuffle %v3float %1314 %1314 0 1 2
       %1316 = OpAccessChain %_ptr_Uniform_float %_Globals %int_17 %int_3
       %1317 = OpLoad %float %1316
       %1318 = OpCompositeConstruct %v3float %1317 %1317 %1317
       %1319 = OpExtInst %v3float %1 FMix %1312 %1315 %1318
       %1320 = OpExtInst %v3float %1 FMax %138 %1319
       %1321 = OpAccessChain %_ptr_Uniform_float %_Globals %int_1 %int_1
       %1322 = OpLoad %float %1321
       %1323 = OpCompositeConstruct %v3float %1322 %1322 %1322
       %1324 = OpExtInst %v3float %1 Pow %1320 %1323
       %1325 = OpIEqual %bool %605 %uint_0
               OpSelectionMerge %1326 DontFlatten
               OpBranchConditional %1325 %1327 %1328
       %1328 = OpLabel
       %1329 = OpIEqual %bool %605 %uint_1
               OpSelectionMerge %1330 None
               OpBranchConditional %1329 %1331 %1332
       %1332 = OpLabel
       %1333 = OpIEqual %bool %605 %uint_3
       %1334 = OpIEqual %bool %605 %uint_5
       %1335 = OpLogicalOr %bool %1333 %1334
               OpSelectionMerge %1336 None
               OpBranchConditional %1335 %1337 %1338
       %1338 = OpLabel
       %1339 = OpIEqual %bool %605 %uint_4
       %1340 = OpIEqual %bool %605 %uint_6
       %1341 = OpLogicalOr %bool %1339 %1340
               OpSelectionMerge %1342 None
               OpBranchConditional %1341 %1343 %1344
       %1344 = OpLabel
       %1345 = OpIEqual %bool %605 %uint_7
               OpSelectionMerge %1346 None
               OpBranchConditional %1345 %1347 %1348
       %1348 = OpLabel
       %1349 = OpVectorTimesMatrix %v3float %1324 %573
       %1350 = OpVectorTimesMatrix %v3float %1349 %602
       %1351 = OpAccessChain %_ptr_Uniform_float %_Globals %int_1 %int_2
       %1352 = OpLoad %float %1351
       %1353 = OpCompositeConstruct %v3float %1352 %1352 %1352
       %1354 = OpExtInst %v3float %1 Pow %1350 %1353
               OpBranch %1346
       %1347 = OpLabel
       %1355 = OpVectorTimesMatrix %v3float %932 %573
       %1356 = OpVectorTimesMatrix %v3float %1355 %602
       %1357 = OpFMul %v3float %1356 %519
       %1358 = OpExtInst %v3float %1 Pow %1357 %286
       %1359 = OpFMul %v3float %196 %1358
       %1360 = OpFAdd %v3float %195 %1359
       %1361 = OpFMul %v3float %197 %1358
       %1362 = OpFAdd %v3float %141 %1361
       %1363 = OpFDiv %v3float %141 %1362
       %1364 = OpFMul %v3float %1360 %1363
       %1365 = OpExtInst %v3float %1 Pow %1364 %287
               OpBranch %1346
       %1346 = OpLabel
       %1366 = OpPhi %v3float %1354 %1348 %1365 %1347
               OpBranch %1342
       %1343 = OpLabel
       %1367 = OpMatrixTimesMatrix %mat3v3float %572 %423
       %1368 = OpFMul %v3float %932 %285
       %1369 = OpVectorTimesMatrix %v3float %1368 %1367
       %1370 = OpCompositeExtract %float %1369 0
       %1371 = OpCompositeExtract %float %1369 1
       %1372 = OpExtInst %float %1 FMin %1370 %1371
       %1373 = OpCompositeExtract %float %1369 2
       %1374 = OpExtInst %float %1 FMin %1372 %1373
       %1375 = OpExtInst %float %1 FMax %1370 %1371
       %1376 = OpExtInst %float %1 FMax %1375 %1373
       %1377 = OpExtInst %float %1 FMax %1376 %float_1_00000001en10
       %1378 = OpExtInst %float %1 FMax %1374 %float_1_00000001en10
       %1379 = OpFSub %float %1377 %1378
       %1380 = OpExtInst %float %1 FMax %1376 %float_0_00999999978
       %1381 = OpFDiv %float %1379 %1380
       %1382 = OpFSub %float %1373 %1371
       %1383 = OpFMul %float %1373 %1382
       %1384 = OpFSub %float %1371 %1370
       %1385 = OpFMul %float %1371 %1384
       %1386 = OpFAdd %float %1383 %1385
       %1387 = OpFSub %float %1370 %1373
       %1388 = OpFMul %float %1370 %1387
       %1389 = OpFAdd %float %1386 %1388
       %1390 = OpExtInst %float %1 Sqrt %1389
       %1391 = OpFAdd %float %1373 %1371
       %1392 = OpFAdd %float %1391 %1370
       %1393 = OpFMul %float %float_1_75 %1390
       %1394 = OpFAdd %float %1392 %1393
       %1395 = OpFMul %float %1394 %float_0_333333343
       %1396 = OpFSub %float %1381 %float_0_400000006
       %1397 = OpFMul %float %1396 %float_5
       %1398 = OpFMul %float %1396 %float_2_5
       %1399 = OpExtInst %float %1 FAbs %1398
       %1400 = OpFSub %float %float_1 %1399
       %1401 = OpExtInst %float %1 FMax %1400 %float_0
       %1402 = OpExtInst %float %1 FSign %1397
       %1403 = OpConvertFToS %int %1402
       %1404 = OpConvertSToF %float %1403
       %1405 = OpFMul %float %1401 %1401
       %1406 = OpFSub %float %float_1 %1405
       %1407 = OpFMul %float %1404 %1406
       %1408 = OpFAdd %float %float_1 %1407
       %1409 = OpFMul %float %1408 %float_0_0250000004
       %1410 = OpFOrdLessThanEqual %bool %1395 %float_0_0533333346
               OpSelectionMerge %1411 None
               OpBranchConditional %1410 %1412 %1413
       %1413 = OpLabel
       %1414 = OpFOrdGreaterThanEqual %bool %1395 %float_0_159999996
               OpSelectionMerge %1415 None
               OpBranchConditional %1414 %1416 %1417
       %1417 = OpLabel
       %1418 = OpFDiv %float %float_0_239999995 %1394
       %1419 = OpFSub %float %1418 %float_0_5
       %1420 = OpFMul %float %1409 %1419
               OpBranch %1415
       %1416 = OpLabel
               OpBranch %1415
       %1415 = OpLabel
       %1421 = OpPhi %float %1420 %1417 %float_0 %1416
               OpBranch %1411
       %1412 = OpLabel
               OpBranch %1411
       %1411 = OpLabel
       %1422 = OpPhi %float %1421 %1415 %1409 %1412
       %1423 = OpFAdd %float %float_1 %1422
       %1424 = OpCompositeConstruct %v3float %1423 %1423 %1423
       %1425 = OpFMul %v3float %1369 %1424
       %1426 = OpCompositeExtract %float %1425 0
       %1427 = OpCompositeExtract %float %1425 1
       %1428 = OpFOrdEqual %bool %1426 %1427
       %1429 = OpCompositeExtract %float %1425 2
       %1430 = OpFOrdEqual %bool %1427 %1429
       %1431 = OpLogicalAnd %bool %1428 %1430
               OpSelectionMerge %1432 None
               OpBranchConditional %1431 %1433 %1434
       %1434 = OpLabel
       %1435 = OpExtInst %float %1 Sqrt %float_3
       %1436 = OpFSub %float %1427 %1429
       %1437 = OpFMul %float %1435 %1436
       %1438 = OpFMul %float %float_2 %1426
       %1439 = OpFSub %float %1438 %1427
       %1440 = OpFSub %float %1439 %1429
       %1441 = OpExtInst %float %1 Atan2 %1437 %1440
       %1442 = OpFMul %float %float_57_2957764 %1441
               OpBranch %1432
       %1433 = OpLabel
               OpBranch %1432
       %1432 = OpLabel
       %1443 = OpPhi %float %1442 %1434 %float_0 %1433
       %1444 = OpFOrdLessThan %bool %1443 %float_0
               OpSelectionMerge %1445 None
               OpBranchConditional %1444 %1446 %1445
       %1446 = OpLabel
       %1447 = OpFAdd %float %1443 %float_360
               OpBranch %1445
       %1445 = OpLabel
       %1448 = OpPhi %float %1443 %1432 %1447 %1446
       %1449 = OpExtInst %float %1 FClamp %1448 %float_0 %float_360
       %1450 = OpFOrdGreaterThan %bool %1449 %float_180
               OpSelectionMerge %1451 None
               OpBranchConditional %1450 %1452 %1451
       %1452 = OpLabel
       %1453 = OpFSub %float %1449 %float_360
               OpBranch %1451
       %1451 = OpLabel
       %1454 = OpPhi %float %1449 %1445 %1453 %1452
       %1455 = OpFOrdGreaterThan %bool %1454 %float_n67_5
       %1456 = OpFOrdLessThan %bool %1454 %float_67_5
       %1457 = OpLogicalAnd %bool %1455 %1456
               OpSelectionMerge %1458 None
               OpBranchConditional %1457 %1459 %1458
       %1459 = OpLabel
       %1460 = OpFSub %float %1454 %float_n67_5
       %1461 = OpFMul %float %1460 %float_0_0296296291
       %1462 = OpConvertFToS %int %1461
       %1463 = OpConvertSToF %float %1462
       %1464 = OpFSub %float %1461 %1463
       %1465 = OpFMul %float %1464 %1464
       %1466 = OpFMul %float %1465 %1464
       %1467 = OpIEqual %bool %1462 %int_3
               OpSelectionMerge %1468 None
               OpBranchConditional %1467 %1469 %1470
       %1470 = OpLabel
       %1471 = OpIEqual %bool %1462 %int_2
               OpSelectionMerge %1472 None
               OpBranchConditional %1471 %1473 %1474
       %1474 = OpLabel
       %1475 = OpIEqual %bool %1462 %int_1
               OpSelectionMerge %1476 None
               OpBranchConditional %1475 %1477 %1478
       %1478 = OpLabel
       %1479 = OpIEqual %bool %1462 %int_0
               OpSelectionMerge %1480 None
               OpBranchConditional %1479 %1481 %1482
       %1482 = OpLabel
               OpBranch %1480
       %1481 = OpLabel
       %1483 = OpFMul %float %1466 %float_0_166666672
               OpBranch %1480
       %1480 = OpLabel
       %1484 = OpPhi %float %float_0 %1482 %1483 %1481
               OpBranch %1476
       %1477 = OpLabel
       %1485 = OpFMul %float %1466 %float_n0_5
       %1486 = OpFMul %float %1465 %float_0_5
       %1487 = OpFAdd %float %1485 %1486
       %1488 = OpFMul %float %1464 %float_0_5
       %1489 = OpFAdd %float %1487 %1488
       %1490 = OpFAdd %float %1489 %float_0_166666672
               OpBranch %1476
       %1476 = OpLabel
       %1491 = OpPhi %float %1484 %1480 %1490 %1477
               OpBranch %1472
       %1473 = OpLabel
       %1492 = OpFMul %float %1466 %float_0_5
       %1493 = OpFMul %float %1465 %float_n1
       %1494 = OpFAdd %float %1492 %1493
       %1495 = OpFAdd %float %1494 %float_0_666666687
               OpBranch %1472
       %1472 = OpLabel
       %1496 = OpPhi %float %1491 %1476 %1495 %1473
               OpBranch %1468
       %1469 = OpLabel
       %1497 = OpFMul %float %1466 %float_n0_166666672
       %1498 = OpFMul %float %1465 %float_0_5
       %1499 = OpFAdd %float %1497 %1498
       %1500 = OpFMul %float %1464 %float_n0_5
       %1501 = OpFAdd %float %1499 %1500
       %1502 = OpFAdd %float %1501 %float_0_166666672
               OpBranch %1468
       %1468 = OpLabel
       %1503 = OpPhi %float %1496 %1472 %1502 %1469
               OpBranch %1458
       %1458 = OpLabel
       %1504 = OpPhi %float %float_0 %1451 %1503 %1468
       %1505 = OpFMul %float %1504 %float_1_5
       %1506 = OpFMul %float %1505 %1381
       %1507 = OpFSub %float %float_0_0299999993 %1426
       %1508 = OpFMul %float %1506 %1507
       %1509 = OpFMul %float %1508 %float_0_180000007
       %1510 = OpFAdd %float %1426 %1509
       %1511 = OpCompositeInsert %v3float %1510 %1425 0
       %1512 = OpExtInst %v3float %1 FClamp %1511 %138 %337
       %1513 = OpVectorTimesMatrix %v3float %1512 %434
       %1514 = OpExtInst %v3float %1 FClamp %1513 %138 %337
       %1515 = OpDot %float %1514 %73
       %1516 = OpCompositeConstruct %v3float %1515 %1515 %1515
       %1517 = OpExtInst %v3float %1 FMix %1516 %1514 %241
       %1518 = OpCompositeExtract %float %1517 0
       %1519 = OpExtInst %float %1 Exp2 %float_n15
       %1520 = OpFMul %float %float_0_179999992 %1519
       %1521 = OpExtInst %float %1 Exp2 %float_18
       %1522 = OpFMul %float %float_0_179999992 %1521
               OpStore %528 %499
               OpStore %527 %500
       %1523 = OpFOrdLessThanEqual %bool %1518 %float_0
       %1524 = OpExtInst %float %1 Exp2 %float_n14
       %1525 = OpSelect %float %1523 %1524 %1518
       %1526 = OpExtInst %float %1 Log %1525
       %1527 = OpFDiv %float %1526 %1091
       %1528 = OpExtInst %float %1 Log %1520
       %1529 = OpFDiv %float %1528 %1091
       %1530 = OpFOrdLessThanEqual %bool %1527 %1529
               OpSelectionMerge %1531 None
               OpBranchConditional %1530 %1532 %1533
       %1533 = OpLabel
       %1534 = OpFOrdGreaterThan %bool %1527 %1529
       %1535 = OpExtInst %float %1 Log %float_0_180000007
       %1536 = OpFDiv %float %1535 %1091
       %1537 = OpFOrdLessThan %bool %1527 %1536
       %1538 = OpLogicalAnd %bool %1534 %1537
               OpSelectionMerge %1539 None
               OpBranchConditional %1538 %1540 %1541
       %1541 = OpLabel
       %1542 = OpFOrdGreaterThanEqual %bool %1527 %1536
       %1543 = OpExtInst %float %1 Log %1522
       %1544 = OpFDiv %float %1543 %1091
       %1545 = OpFOrdLessThan %bool %1527 %1544
       %1546 = OpLogicalAnd %bool %1542 %1545
               OpSelectionMerge %1547 None
               OpBranchConditional %1546 %1548 %1549
       %1549 = OpLabel
       %1550 = OpExtInst %float %1 Log %float_10000
       %1551 = OpFDiv %float %1550 %1091
               OpBranch %1547
       %1548 = OpLabel
       %1552 = OpFSub %float %1527 %1536
       %1553 = OpFMul %float %float_3 %1552
       %1554 = OpFSub %float %1544 %1536
       %1555 = OpFDiv %float %1553 %1554
       %1556 = OpConvertFToS %int %1555
       %1557 = OpConvertSToF %float %1556
       %1558 = OpFSub %float %1555 %1557
       %1559 = OpAccessChain %_ptr_Function_float %527 %1556
       %1560 = OpLoad %float %1559
       %1561 = OpIAdd %int %1556 %int_1
       %1562 = OpAccessChain %_ptr_Function_float %527 %1561
       %1563 = OpLoad %float %1562
       %1564 = OpIAdd %int %1556 %int_2
       %1565 = OpAccessChain %_ptr_Function_float %527 %1564
       %1566 = OpLoad %float %1565
       %1567 = OpCompositeConstruct %v3float %1560 %1563 %1566
       %1568 = OpFMul %float %1558 %1558
       %1569 = OpCompositeConstruct %v3float %1568 %1558 %float_1
       %1570 = OpMatrixTimesVector %v3float %466 %1567
       %1571 = OpDot %float %1569 %1570
               OpBranch %1547
       %1547 = OpLabel
       %1572 = OpPhi %float %1551 %1549 %1571 %1548
               OpBranch %1539
       %1540 = OpLabel
       %1573 = OpFSub %float %1527 %1529
       %1574 = OpFMul %float %float_3 %1573
       %1575 = OpFSub %float %1536 %1529
       %1576 = OpFDiv %float %1574 %1575
       %1577 = OpConvertFToS %int %1576
       %1578 = OpConvertSToF %float %1577
       %1579 = OpFSub %float %1576 %1578
       %1580 = OpAccessChain %_ptr_Function_float %528 %1577
       %1581 = OpLoad %float %1580
       %1582 = OpIAdd %int %1577 %int_1
       %1583 = OpAccessChain %_ptr_Function_float %528 %1582
       %1584 = OpLoad %float %1583
       %1585 = OpIAdd %int %1577 %int_2
       %1586 = OpAccessChain %_ptr_Function_float %528 %1585
       %1587 = OpLoad %float %1586
       %1588 = OpCompositeConstruct %v3float %1581 %1584 %1587
       %1589 = OpFMul %float %1579 %1579
       %1590 = OpCompositeConstruct %v3float %1589 %1579 %float_1
       %1591 = OpMatrixTimesVector %v3float %466 %1588
       %1592 = OpDot %float %1590 %1591
               OpBranch %1539
       %1539 = OpLabel
       %1593 = OpPhi %float %1572 %1547 %1592 %1540
               OpBranch %1531
       %1532 = OpLabel
       %1594 = OpExtInst %float %1 Log %float_9_99999975en05
       %1595 = OpFDiv %float %1594 %1091
               OpBranch %1531
       %1531 = OpLabel
       %1596 = OpPhi %float %1593 %1539 %1595 %1532
       %1597 = OpExtInst %float %1 Pow %float_10 %1596
       %1598 = OpCompositeInsert %v3float %1597 %523 0
       %1599 = OpCompositeExtract %float %1517 1
               OpStore %530 %499
               OpStore %529 %500
       %1600 = OpFOrdLessThanEqual %bool %1599 %float_0
       %1601 = OpSelect %float %1600 %1524 %1599
       %1602 = OpExtInst %float %1 Log %1601
       %1603 = OpFDiv %float %1602 %1091
       %1604 = OpFOrdLessThanEqual %bool %1603 %1529
               OpSelectionMerge %1605 None
               OpBranchConditional %1604 %1606 %1607
       %1607 = OpLabel
       %1608 = OpFOrdGreaterThan %bool %1603 %1529
       %1609 = OpExtInst %float %1 Log %float_0_180000007
       %1610 = OpFDiv %float %1609 %1091
       %1611 = OpFOrdLessThan %bool %1603 %1610
       %1612 = OpLogicalAnd %bool %1608 %1611
               OpSelectionMerge %1613 None
               OpBranchConditional %1612 %1614 %1615
       %1615 = OpLabel
       %1616 = OpFOrdGreaterThanEqual %bool %1603 %1610
       %1617 = OpExtInst %float %1 Log %1522
       %1618 = OpFDiv %float %1617 %1091
       %1619 = OpFOrdLessThan %bool %1603 %1618
       %1620 = OpLogicalAnd %bool %1616 %1619
               OpSelectionMerge %1621 None
               OpBranchConditional %1620 %1622 %1623
       %1623 = OpLabel
       %1624 = OpExtInst %float %1 Log %float_10000
       %1625 = OpFDiv %float %1624 %1091
               OpBranch %1621
       %1622 = OpLabel
       %1626 = OpFSub %float %1603 %1610
       %1627 = OpFMul %float %float_3 %1626
       %1628 = OpFSub %float %1618 %1610
       %1629 = OpFDiv %float %1627 %1628
       %1630 = OpConvertFToS %int %1629
       %1631 = OpConvertSToF %float %1630
       %1632 = OpFSub %float %1629 %1631
       %1633 = OpAccessChain %_ptr_Function_float %529 %1630
       %1634 = OpLoad %float %1633
       %1635 = OpIAdd %int %1630 %int_1
       %1636 = OpAccessChain %_ptr_Function_float %529 %1635
       %1637 = OpLoad %float %1636
       %1638 = OpIAdd %int %1630 %int_2
       %1639 = OpAccessChain %_ptr_Function_float %529 %1638
       %1640 = OpLoad %float %1639
       %1641 = OpCompositeConstruct %v3float %1634 %1637 %1640
       %1642 = OpFMul %float %1632 %1632
       %1643 = OpCompositeConstruct %v3float %1642 %1632 %float_1
       %1644 = OpMatrixTimesVector %v3float %466 %1641
       %1645 = OpDot %float %1643 %1644
               OpBranch %1621
       %1621 = OpLabel
       %1646 = OpPhi %float %1625 %1623 %1645 %1622
               OpBranch %1613
       %1614 = OpLabel
       %1647 = OpFSub %float %1603 %1529
       %1648 = OpFMul %float %float_3 %1647
       %1649 = OpFSub %float %1610 %1529
       %1650 = OpFDiv %float %1648 %1649
       %1651 = OpConvertFToS %int %1650
       %1652 = OpConvertSToF %float %1651
       %1653 = OpFSub %float %1650 %1652
       %1654 = OpAccessChain %_ptr_Function_float %530 %1651
       %1655 = OpLoad %float %1654
       %1656 = OpIAdd %int %1651 %int_1
       %1657 = OpAccessChain %_ptr_Function_float %530 %1656
       %1658 = OpLoad %float %1657
       %1659 = OpIAdd %int %1651 %int_2
       %1660 = OpAccessChain %_ptr_Function_float %530 %1659
       %1661 = OpLoad %float %1660
       %1662 = OpCompositeConstruct %v3float %1655 %1658 %1661
       %1663 = OpFMul %float %1653 %1653
       %1664 = OpCompositeConstruct %v3float %1663 %1653 %float_1
       %1665 = OpMatrixTimesVector %v3float %466 %1662
       %1666 = OpDot %float %1664 %1665
               OpBranch %1613
       %1613 = OpLabel
       %1667 = OpPhi %float %1646 %1621 %1666 %1614
               OpBranch %1605
       %1606 = OpLabel
       %1668 = OpExtInst %float %1 Log %float_9_99999975en05
       %1669 = OpFDiv %float %1668 %1091
               OpBranch %1605
       %1605 = OpLabel
       %1670 = OpPhi %float %1667 %1613 %1669 %1606
       %1671 = OpExtInst %float %1 Pow %float_10 %1670
       %1672 = OpCompositeInsert %v3float %1671 %1598 1
       %1673 = OpCompositeExtract %float %1517 2
               OpStore %532 %499
               OpStore %531 %500
       %1674 = OpFOrdLessThanEqual %bool %1673 %float_0
       %1675 = OpSelect %float %1674 %1524 %1673
       %1676 = OpExtInst %float %1 Log %1675
       %1677 = OpFDiv %float %1676 %1091
       %1678 = OpFOrdLessThanEqual %bool %1677 %1529
               OpSelectionMerge %1679 None
               OpBranchConditional %1678 %1680 %1681
       %1681 = OpLabel
       %1682 = OpFOrdGreaterThan %bool %1677 %1529
       %1683 = OpExtInst %float %1 Log %float_0_180000007
       %1684 = OpFDiv %float %1683 %1091
       %1685 = OpFOrdLessThan %bool %1677 %1684
       %1686 = OpLogicalAnd %bool %1682 %1685
               OpSelectionMerge %1687 None
               OpBranchConditional %1686 %1688 %1689
       %1689 = OpLabel
       %1690 = OpFOrdGreaterThanEqual %bool %1677 %1684
       %1691 = OpExtInst %float %1 Log %1522
       %1692 = OpFDiv %float %1691 %1091
       %1693 = OpFOrdLessThan %bool %1677 %1692
       %1694 = OpLogicalAnd %bool %1690 %1693
               OpSelectionMerge %1695 None
               OpBranchConditional %1694 %1696 %1697
       %1697 = OpLabel
       %1698 = OpExtInst %float %1 Log %float_10000
       %1699 = OpFDiv %float %1698 %1091
               OpBranch %1695
       %1696 = OpLabel
       %1700 = OpFSub %float %1677 %1684
       %1701 = OpFMul %float %float_3 %1700
       %1702 = OpFSub %float %1692 %1684
       %1703 = OpFDiv %float %1701 %1702
       %1704 = OpConvertFToS %int %1703
       %1705 = OpConvertSToF %float %1704
       %1706 = OpFSub %float %1703 %1705
       %1707 = OpAccessChain %_ptr_Function_float %531 %1704
       %1708 = OpLoad %float %1707
       %1709 = OpIAdd %int %1704 %int_1
       %1710 = OpAccessChain %_ptr_Function_float %531 %1709
       %1711 = OpLoad %float %1710
       %1712 = OpIAdd %int %1704 %int_2
       %1713 = OpAccessChain %_ptr_Function_float %531 %1712
       %1714 = OpLoad %float %1713
       %1715 = OpCompositeConstruct %v3float %1708 %1711 %1714
       %1716 = OpFMul %float %1706 %1706
       %1717 = OpCompositeConstruct %v3float %1716 %1706 %float_1
       %1718 = OpMatrixTimesVector %v3float %466 %1715
       %1719 = OpDot %float %1717 %1718
               OpBranch %1695
       %1695 = OpLabel
       %1720 = OpPhi %float %1699 %1697 %1719 %1696
               OpBranch %1687
       %1688 = OpLabel
       %1721 = OpFSub %float %1677 %1529
       %1722 = OpFMul %float %float_3 %1721
       %1723 = OpFSub %float %1684 %1529
       %1724 = OpFDiv %float %1722 %1723
       %1725 = OpConvertFToS %int %1724
       %1726 = OpConvertSToF %float %1725
       %1727 = OpFSub %float %1724 %1726
       %1728 = OpAccessChain %_ptr_Function_float %532 %1725
       %1729 = OpLoad %float %1728
       %1730 = OpIAdd %int %1725 %int_1
       %1731 = OpAccessChain %_ptr_Function_float %532 %1730
       %1732 = OpLoad %float %1731
       %1733 = OpIAdd %int %1725 %int_2
       %1734 = OpAccessChain %_ptr_Function_float %532 %1733
       %1735 = OpLoad %float %1734
       %1736 = OpCompositeConstruct %v3float %1729 %1732 %1735
       %1737 = OpFMul %float %1727 %1727
       %1738 = OpCompositeConstruct %v3float %1737 %1727 %float_1
       %1739 = OpMatrixTimesVector %v3float %466 %1736
       %1740 = OpDot %float %1738 %1739
               OpBranch %1687
       %1687 = OpLabel
       %1741 = OpPhi %float %1720 %1695 %1740 %1688
               OpBranch %1679
       %1680 = OpLabel
       %1742 = OpExtInst %float %1 Log %float_9_99999975en05
       %1743 = OpFDiv %float %1742 %1091
               OpBranch %1679
       %1679 = OpLabel
       %1744 = OpPhi %float %1741 %1687 %1743 %1680
       %1745 = OpExtInst %float %1 Pow %float_10 %1744
       %1746 = OpCompositeInsert %v3float %1745 %1672 2
       %1747 = OpVectorTimesMatrix %v3float %1746 %438
       %1748 = OpVectorTimesMatrix %v3float %1747 %434
       %1749 = OpExtInst %float %1 Pow %float_2 %float_n12
       %1750 = OpFMul %float %float_0_179999992 %1749
               OpStore %540 %499
               OpStore %539 %500
       %1751 = OpFOrdLessThanEqual %bool %1750 %float_0
       %1752 = OpSelect %float %1751 %1524 %1750
       %1753 = OpExtInst %float %1 Log %1752
       %1754 = OpFDiv %float %1753 %1091
       %1755 = OpFOrdLessThanEqual %bool %1754 %1529
               OpSelectionMerge %1756 None
               OpBranchConditional %1755 %1757 %1758
       %1758 = OpLabel
       %1759 = OpFOrdGreaterThan %bool %1754 %1529
       %1760 = OpExtInst %float %1 Log %float_0_180000007
       %1761 = OpFDiv %float %1760 %1091
       %1762 = OpFOrdLessThan %bool %1754 %1761
       %1763 = OpLogicalAnd %bool %1759 %1762
               OpSelectionMerge %1764 None
               OpBranchConditional %1763 %1765 %1766
       %1766 = OpLabel
       %1767 = OpFOrdGreaterThanEqual %bool %1754 %1761
       %1768 = OpExtInst %float %1 Log %1522
       %1769 = OpFDiv %float %1768 %1091
       %1770 = OpFOrdLessThan %bool %1754 %1769
       %1771 = OpLogicalAnd %bool %1767 %1770
               OpSelectionMerge %1772 None
               OpBranchConditional %1771 %1773 %1774
       %1774 = OpLabel
       %1775 = OpExtInst %float %1 Log %float_10000
       %1776 = OpFDiv %float %1775 %1091
               OpBranch %1772
       %1773 = OpLabel
       %1777 = OpFSub %float %1754 %1761
       %1778 = OpFMul %float %float_3 %1777
       %1779 = OpFSub %float %1769 %1761
       %1780 = OpFDiv %float %1778 %1779
       %1781 = OpConvertFToS %int %1780
       %1782 = OpConvertSToF %float %1781
       %1783 = OpFSub %float %1780 %1782
       %1784 = OpAccessChain %_ptr_Function_float %539 %1781
       %1785 = OpLoad %float %1784
       %1786 = OpIAdd %int %1781 %int_1
       %1787 = OpAccessChain %_ptr_Function_float %539 %1786
       %1788 = OpLoad %float %1787
       %1789 = OpIAdd %int %1781 %int_2
       %1790 = OpAccessChain %_ptr_Function_float %539 %1789
       %1791 = OpLoad %float %1790
       %1792 = OpCompositeConstruct %v3float %1785 %1788 %1791
       %1793 = OpFMul %float %1783 %1783
       %1794 = OpCompositeConstruct %v3float %1793 %1783 %float_1
       %1795 = OpMatrixTimesVector %v3float %466 %1792
       %1796 = OpDot %float %1794 %1795
               OpBranch %1772
       %1772 = OpLabel
       %1797 = OpPhi %float %1776 %1774 %1796 %1773
               OpBranch %1764
       %1765 = OpLabel
       %1798 = OpFSub %float %1754 %1529
       %1799 = OpFMul %float %float_3 %1798
       %1800 = OpFSub %float %1761 %1529
       %1801 = OpFDiv %float %1799 %1800
       %1802 = OpConvertFToS %int %1801
       %1803 = OpConvertSToF %float %1802
       %1804 = OpFSub %float %1801 %1803
       %1805 = OpAccessChain %_ptr_Function_float %540 %1802
       %1806 = OpLoad %float %1805
       %1807 = OpIAdd %int %1802 %int_1
       %1808 = OpAccessChain %_ptr_Function_float %540 %1807
       %1809 = OpLoad %float %1808
       %1810 = OpIAdd %int %1802 %int_2
       %1811 = OpAccessChain %_ptr_Function_float %540 %1810
       %1812 = OpLoad %float %1811
       %1813 = OpCompositeConstruct %v3float %1806 %1809 %1812
       %1814 = OpFMul %float %1804 %1804
       %1815 = OpCompositeConstruct %v3float %1814 %1804 %float_1
       %1816 = OpMatrixTimesVector %v3float %466 %1813
       %1817 = OpDot %float %1815 %1816
               OpBranch %1764
       %1764 = OpLabel
       %1818 = OpPhi %float %1797 %1772 %1817 %1765
               OpBranch %1756
       %1757 = OpLabel
       %1819 = OpExtInst %float %1 Log %float_9_99999975en05
       %1820 = OpFDiv %float %1819 %1091
               OpBranch %1756
       %1756 = OpLabel
       %1821 = OpPhi %float %1818 %1764 %1820 %1757
       %1822 = OpExtInst %float %1 Pow %float_10 %1821
               OpStore %542 %499
               OpStore %541 %500
       %1823 = OpExtInst %float %1 Log %float_0_180000007
       %1824 = OpFDiv %float %1823 %1091
       %1825 = OpFOrdLessThanEqual %bool %1824 %1529
               OpSelectionMerge %1826 None
               OpBranchConditional %1825 %1827 %1828
       %1828 = OpLabel
       %1829 = OpFOrdGreaterThan %bool %1824 %1529
       %1830 = OpFOrdLessThan %bool %1824 %1824
       %1831 = OpLogicalAnd %bool %1829 %1830
               OpSelectionMerge %1832 None
               OpBranchConditional %1831 %1833 %1834
       %1834 = OpLabel
       %1835 = OpFOrdGreaterThanEqual %bool %1824 %1824
       %1836 = OpExtInst %float %1 Log %1522
       %1837 = OpFDiv %float %1836 %1091
       %1838 = OpFOrdLessThan %bool %1824 %1837
       %1839 = OpLogicalAnd %bool %1835 %1838
               OpSelectionMerge %1840 None
               OpBranchConditional %1839 %1841 %1842
       %1842 = OpLabel
       %1843 = OpExtInst %float %1 Log %float_10000
       %1844 = OpFDiv %float %1843 %1091
               OpBranch %1840
       %1841 = OpLabel
       %1845 = OpFSub %float %1824 %1824
       %1846 = OpFMul %float %float_3 %1845
       %1847 = OpFSub %float %1837 %1824
       %1848 = OpFDiv %float %1846 %1847
       %1849 = OpConvertFToS %int %1848
       %1850 = OpConvertSToF %float %1849
       %1851 = OpFSub %float %1848 %1850
       %1852 = OpAccessChain %_ptr_Function_float %541 %1849
       %1853 = OpLoad %float %1852
       %1854 = OpIAdd %int %1849 %int_1
       %1855 = OpAccessChain %_ptr_Function_float %541 %1854
       %1856 = OpLoad %float %1855
       %1857 = OpIAdd %int %1849 %int_2
       %1858 = OpAccessChain %_ptr_Function_float %541 %1857
       %1859 = OpLoad %float %1858
       %1860 = OpCompositeConstruct %v3float %1853 %1856 %1859
       %1861 = OpFMul %float %1851 %1851
       %1862 = OpCompositeConstruct %v3float %1861 %1851 %float_1
       %1863 = OpMatrixTimesVector %v3float %466 %1860
       %1864 = OpDot %float %1862 %1863
               OpBranch %1840
       %1840 = OpLabel
       %1865 = OpPhi %float %1844 %1842 %1864 %1841
               OpBranch %1832
       %1833 = OpLabel
       %1866 = OpAccessChain %_ptr_Function_float %542 %int_3
       %1867 = OpLoad %float %1866
       %1868 = OpAccessChain %_ptr_Function_float %542 %int_4
       %1869 = OpLoad %float %1868
       %1870 = OpAccessChain %_ptr_Function_float %542 %int_5
       %1871 = OpLoad %float %1870
       %1872 = OpCompositeConstruct %v3float %1867 %1869 %1871
       %1873 = OpMatrixTimesVector %v3float %466 %1872
       %1874 = OpCompositeExtract %float %1873 2
               OpBranch %1832
       %1832 = OpLabel
       %1875 = OpPhi %float %1865 %1840 %1874 %1833
               OpBranch %1826
       %1827 = OpLabel
       %1876 = OpExtInst %float %1 Log %float_9_99999975en05
       %1877 = OpFDiv %float %1876 %1091
               OpBranch %1826
       %1826 = OpLabel
       %1878 = OpPhi %float %1875 %1832 %1877 %1827
       %1879 = OpExtInst %float %1 Pow %float_10 %1878
       %1880 = OpExtInst %float %1 Pow %float_2 %float_11
       %1881 = OpFMul %float %float_0_179999992 %1880
               OpStore %544 %499
               OpStore %543 %500
       %1882 = OpFOrdLessThanEqual %bool %1881 %float_0
       %1883 = OpSelect %float %1882 %1524 %1881
       %1884 = OpExtInst %float %1 Log %1883
       %1885 = OpFDiv %float %1884 %1091
       %1886 = OpFOrdLessThanEqual %bool %1885 %1529
               OpSelectionMerge %1887 None
               OpBranchConditional %1886 %1888 %1889
       %1889 = OpLabel
       %1890 = OpFOrdGreaterThan %bool %1885 %1529
       %1891 = OpFOrdLessThan %bool %1885 %1824
       %1892 = OpLogicalAnd %bool %1890 %1891
               OpSelectionMerge %1893 None
               OpBranchConditional %1892 %1894 %1895
       %1895 = OpLabel
       %1896 = OpFOrdGreaterThanEqual %bool %1885 %1824
       %1897 = OpExtInst %float %1 Log %1522
       %1898 = OpFDiv %float %1897 %1091
       %1899 = OpFOrdLessThan %bool %1885 %1898
       %1900 = OpLogicalAnd %bool %1896 %1899
               OpSelectionMerge %1901 None
               OpBranchConditional %1900 %1902 %1903
       %1903 = OpLabel
       %1904 = OpExtInst %float %1 Log %float_10000
       %1905 = OpFDiv %float %1904 %1091
               OpBranch %1901
       %1902 = OpLabel
       %1906 = OpFSub %float %1885 %1824
       %1907 = OpFMul %float %float_3 %1906
       %1908 = OpFSub %float %1898 %1824
       %1909 = OpFDiv %float %1907 %1908
       %1910 = OpConvertFToS %int %1909
       %1911 = OpConvertSToF %float %1910
       %1912 = OpFSub %float %1909 %1911
       %1913 = OpAccessChain %_ptr_Function_float %543 %1910
       %1914 = OpLoad %float %1913
       %1915 = OpIAdd %int %1910 %int_1
       %1916 = OpAccessChain %_ptr_Function_float %543 %1915
       %1917 = OpLoad %float %1916
       %1918 = OpIAdd %int %1910 %int_2
       %1919 = OpAccessChain %_ptr_Function_float %543 %1918
       %1920 = OpLoad %float %1919
       %1921 = OpCompositeConstruct %v3float %1914 %1917 %1920
       %1922 = OpFMul %float %1912 %1912
       %1923 = OpCompositeConstruct %v3float %1922 %1912 %float_1
       %1924 = OpMatrixTimesVector %v3float %466 %1921
       %1925 = OpDot %float %1923 %1924
               OpBranch %1901
       %1901 = OpLabel
       %1926 = OpPhi %float %1905 %1903 %1925 %1902
               OpBranch %1893
       %1894 = OpLabel
       %1927 = OpFSub %float %1885 %1529
       %1928 = OpFMul %float %float_3 %1927
       %1929 = OpFSub %float %1824 %1529
       %1930 = OpFDiv %float %1928 %1929
       %1931 = OpConvertFToS %int %1930
       %1932 = OpConvertSToF %float %1931
       %1933 = OpFSub %float %1930 %1932
       %1934 = OpAccessChain %_ptr_Function_float %544 %1931
       %1935 = OpLoad %float %1934
       %1936 = OpIAdd %int %1931 %int_1
       %1937 = OpAccessChain %_ptr_Function_float %544 %1936
       %1938 = OpLoad %float %1937
       %1939 = OpIAdd %int %1931 %int_2
       %1940 = OpAccessChain %_ptr_Function_float %544 %1939
       %1941 = OpLoad %float %1940
       %1942 = OpCompositeConstruct %v3float %1935 %1938 %1941
       %1943 = OpFMul %float %1933 %1933
       %1944 = OpCompositeConstruct %v3float %1943 %1933 %float_1
       %1945 = OpMatrixTimesVector %v3float %466 %1942
       %1946 = OpDot %float %1944 %1945
               OpBranch %1893
       %1893 = OpLabel
       %1947 = OpPhi %float %1926 %1901 %1946 %1894
               OpBranch %1887
       %1888 = OpLabel
       %1948 = OpExtInst %float %1 Log %float_9_99999975en05
       %1949 = OpFDiv %float %1948 %1091
               OpBranch %1887
       %1887 = OpLabel
       %1950 = OpPhi %float %1947 %1893 %1949 %1888
       %1951 = OpExtInst %float %1 Pow %float_10 %1950
       %1952 = OpCompositeExtract %float %1748 0
               OpStore %538 %506
               OpStore %537 %507
       %1953 = OpFOrdLessThanEqual %bool %1952 %float_0
       %1954 = OpSelect %float %1953 %float_9_99999975en05 %1952
       %1955 = OpExtInst %float %1 Log %1954
       %1956 = OpFDiv %float %1955 %1091
       %1957 = OpExtInst %float %1 Log %1822
       %1958 = OpFDiv %float %1957 %1091
       %1959 = OpFOrdLessThanEqual %bool %1956 %1958
               OpSelectionMerge %1960 None
               OpBranchConditional %1959 %1961 %1962
       %1962 = OpLabel
       %1963 = OpFOrdGreaterThan %bool %1956 %1958
       %1964 = OpExtInst %float %1 Log %1879
       %1965 = OpFDiv %float %1964 %1091
       %1966 = OpFOrdLessThan %bool %1956 %1965
       %1967 = OpLogicalAnd %bool %1963 %1966
               OpSelectionMerge %1968 None
               OpBranchConditional %1967 %1969 %1970
       %1970 = OpLabel
       %1971 = OpFOrdGreaterThanEqual %bool %1956 %1965
       %1972 = OpExtInst %float %1 Log %1951
       %1973 = OpFDiv %float %1972 %1091
       %1974 = OpFOrdLessThan %bool %1956 %1973
       %1975 = OpLogicalAnd %bool %1971 %1974
               OpSelectionMerge %1976 None
               OpBranchConditional %1975 %1977 %1978
       %1978 = OpLabel
       %1979 = OpFMul %float %1956 %float_0_119999997
       %1980 = OpExtInst %float %1 Log %float_2000
       %1981 = OpFDiv %float %1980 %1091
       %1982 = OpFMul %float %float_0_119999997 %1972
       %1983 = OpFDiv %float %1982 %1091
       %1984 = OpFSub %float %1981 %1983
       %1985 = OpFAdd %float %1979 %1984
               OpBranch %1976
       %1977 = OpLabel
       %1986 = OpFSub %float %1956 %1965
       %1987 = OpFMul %float %float_7 %1986
       %1988 = OpFSub %float %1973 %1965
       %1989 = OpFDiv %float %1987 %1988
       %1990 = OpConvertFToS %int %1989
       %1991 = OpConvertSToF %float %1990
       %1992 = OpFSub %float %1989 %1991
       %1993 = OpAccessChain %_ptr_Function_float %537 %1990
       %1994 = OpLoad %float %1993
       %1995 = OpIAdd %int %1990 %int_1
       %1996 = OpAccessChain %_ptr_Function_float %537 %1995
       %1997 = OpLoad %float %1996
       %1998 = OpIAdd %int %1990 %int_2
       %1999 = OpAccessChain %_ptr_Function_float %537 %1998
       %2000 = OpLoad %float %1999
       %2001 = OpCompositeConstruct %v3float %1994 %1997 %2000
       %2002 = OpFMul %float %1992 %1992
       %2003 = OpCompositeConstruct %v3float %2002 %1992 %float_1
       %2004 = OpMatrixTimesVector %v3float %466 %2001
       %2005 = OpDot %float %2003 %2004
               OpBranch %1976
       %1976 = OpLabel
       %2006 = OpPhi %float %1985 %1978 %2005 %1977
               OpBranch %1968
       %1969 = OpLabel
       %2007 = OpFSub %float %1956 %1958
       %2008 = OpFMul %float %float_7 %2007
       %2009 = OpFSub %float %1965 %1958
       %2010 = OpFDiv %float %2008 %2009
       %2011 = OpConvertFToS %int %2010
       %2012 = OpConvertSToF %float %2011
       %2013 = OpFSub %float %2010 %2012
       %2014 = OpAccessChain %_ptr_Function_float %538 %2011
       %2015 = OpLoad %float %2014
       %2016 = OpIAdd %int %2011 %int_1
       %2017 = OpAccessChain %_ptr_Function_float %538 %2016
       %2018 = OpLoad %float %2017
       %2019 = OpIAdd %int %2011 %int_2
       %2020 = OpAccessChain %_ptr_Function_float %538 %2019
       %2021 = OpLoad %float %2020
       %2022 = OpCompositeConstruct %v3float %2015 %2018 %2021
       %2023 = OpFMul %float %2013 %2013
       %2024 = OpCompositeConstruct %v3float %2023 %2013 %float_1
       %2025 = OpMatrixTimesVector %v3float %466 %2022
       %2026 = OpDot %float %2024 %2025
               OpBranch %1968
       %1968 = OpLabel
       %2027 = OpPhi %float %2006 %1976 %2026 %1969
               OpBranch %1960
       %1961 = OpLabel
       %2028 = OpExtInst %float %1 Log %float_0_00499999989
       %2029 = OpFDiv %float %2028 %1091
               OpBranch %1960
       %1960 = OpLabel
       %2030 = OpPhi %float %2027 %1968 %2029 %1961
       %2031 = OpExtInst %float %1 Pow %float_10 %2030
       %2032 = OpCompositeInsert %v3float %2031 %523 0
       %2033 = OpCompositeExtract %float %1748 1
               OpStore %536 %506
               OpStore %535 %507
       %2034 = OpFOrdLessThanEqual %bool %2033 %float_0
       %2035 = OpSelect %float %2034 %float_9_99999975en05 %2033
       %2036 = OpExtInst %float %1 Log %2035
       %2037 = OpFDiv %float %2036 %1091
       %2038 = OpFOrdLessThanEqual %bool %2037 %1958
               OpSelectionMerge %2039 None
               OpBranchConditional %2038 %2040 %2041
       %2041 = OpLabel
       %2042 = OpFOrdGreaterThan %bool %2037 %1958
       %2043 = OpExtInst %float %1 Log %1879
       %2044 = OpFDiv %float %2043 %1091
       %2045 = OpFOrdLessThan %bool %2037 %2044
       %2046 = OpLogicalAnd %bool %2042 %2045
               OpSelectionMerge %2047 None
               OpBranchConditional %2046 %2048 %2049
       %2049 = OpLabel
       %2050 = OpFOrdGreaterThanEqual %bool %2037 %2044
       %2051 = OpExtInst %float %1 Log %1951
       %2052 = OpFDiv %float %2051 %1091
       %2053 = OpFOrdLessThan %bool %2037 %2052
       %2054 = OpLogicalAnd %bool %2050 %2053
               OpSelectionMerge %2055 None
               OpBranchConditional %2054 %2056 %2057
       %2057 = OpLabel
       %2058 = OpFMul %float %2037 %float_0_119999997
       %2059 = OpExtInst %float %1 Log %float_2000
       %2060 = OpFDiv %float %2059 %1091
       %2061 = OpFMul %float %float_0_119999997 %2051
       %2062 = OpFDiv %float %2061 %1091
       %2063 = OpFSub %float %2060 %2062
       %2064 = OpFAdd %float %2058 %2063
               OpBranch %2055
       %2056 = OpLabel
       %2065 = OpFSub %float %2037 %2044
       %2066 = OpFMul %float %float_7 %2065
       %2067 = OpFSub %float %2052 %2044
       %2068 = OpFDiv %float %2066 %2067
       %2069 = OpConvertFToS %int %2068
       %2070 = OpConvertSToF %float %2069
       %2071 = OpFSub %float %2068 %2070
       %2072 = OpAccessChain %_ptr_Function_float %535 %2069
       %2073 = OpLoad %float %2072
       %2074 = OpIAdd %int %2069 %int_1
       %2075 = OpAccessChain %_ptr_Function_float %535 %2074
       %2076 = OpLoad %float %2075
       %2077 = OpIAdd %int %2069 %int_2
       %2078 = OpAccessChain %_ptr_Function_float %535 %2077
       %2079 = OpLoad %float %2078
       %2080 = OpCompositeConstruct %v3float %2073 %2076 %2079
       %2081 = OpFMul %float %2071 %2071
       %2082 = OpCompositeConstruct %v3float %2081 %2071 %float_1
       %2083 = OpMatrixTimesVector %v3float %466 %2080
       %2084 = OpDot %float %2082 %2083
               OpBranch %2055
       %2055 = OpLabel
       %2085 = OpPhi %float %2064 %2057 %2084 %2056
               OpBranch %2047
       %2048 = OpLabel
       %2086 = OpFSub %float %2037 %1958
       %2087 = OpFMul %float %float_7 %2086
       %2088 = OpFSub %float %2044 %1958
       %2089 = OpFDiv %float %2087 %2088
       %2090 = OpConvertFToS %int %2089
       %2091 = OpConvertSToF %float %2090
       %2092 = OpFSub %float %2089 %2091
       %2093 = OpAccessChain %_ptr_Function_float %536 %2090
       %2094 = OpLoad %float %2093
       %2095 = OpIAdd %int %2090 %int_1
       %2096 = OpAccessChain %_ptr_Function_float %536 %2095
       %2097 = OpLoad %float %2096
       %2098 = OpIAdd %int %2090 %int_2
       %2099 = OpAccessChain %_ptr_Function_float %536 %2098
       %2100 = OpLoad %float %2099
       %2101 = OpCompositeConstruct %v3float %2094 %2097 %2100
       %2102 = OpFMul %float %2092 %2092
       %2103 = OpCompositeConstruct %v3float %2102 %2092 %float_1
       %2104 = OpMatrixTimesVector %v3float %466 %2101
       %2105 = OpDot %float %2103 %2104
               OpBranch %2047
       %2047 = OpLabel
       %2106 = OpPhi %float %2085 %2055 %2105 %2048
               OpBranch %2039
       %2040 = OpLabel
       %2107 = OpExtInst %float %1 Log %float_0_00499999989
       %2108 = OpFDiv %float %2107 %1091
               OpBranch %2039
       %2039 = OpLabel
       %2109 = OpPhi %float %2106 %2047 %2108 %2040
       %2110 = OpExtInst %float %1 Pow %float_10 %2109
       %2111 = OpCompositeInsert %v3float %2110 %2032 1
       %2112 = OpCompositeExtract %float %1748 2
               OpStore %534 %506
               OpStore %533 %507
       %2113 = OpFOrdLessThanEqual %bool %2112 %float_0
       %2114 = OpSelect %float %2113 %float_9_99999975en05 %2112
       %2115 = OpExtInst %float %1 Log %2114
       %2116 = OpFDiv %float %2115 %1091
       %2117 = OpFOrdLessThanEqual %bool %2116 %1958
               OpSelectionMerge %2118 None
               OpBranchConditional %2117 %2119 %2120
       %2120 = OpLabel
       %2121 = OpFOrdGreaterThan %bool %2116 %1958
       %2122 = OpExtInst %float %1 Log %1879
       %2123 = OpFDiv %float %2122 %1091
       %2124 = OpFOrdLessThan %bool %2116 %2123
       %2125 = OpLogicalAnd %bool %2121 %2124
               OpSelectionMerge %2126 None
               OpBranchConditional %2125 %2127 %2128
       %2128 = OpLabel
       %2129 = OpFOrdGreaterThanEqual %bool %2116 %2123
       %2130 = OpExtInst %float %1 Log %1951
       %2131 = OpFDiv %float %2130 %1091
       %2132 = OpFOrdLessThan %bool %2116 %2131
       %2133 = OpLogicalAnd %bool %2129 %2132
               OpSelectionMerge %2134 None
               OpBranchConditional %2133 %2135 %2136
       %2136 = OpLabel
       %2137 = OpFMul %float %2116 %float_0_119999997
       %2138 = OpExtInst %float %1 Log %float_2000
       %2139 = OpFDiv %float %2138 %1091
       %2140 = OpFMul %float %float_0_119999997 %2130
       %2141 = OpFDiv %float %2140 %1091
       %2142 = OpFSub %float %2139 %2141
       %2143 = OpFAdd %float %2137 %2142
               OpBranch %2134
       %2135 = OpLabel
       %2144 = OpFSub %float %2116 %2123
       %2145 = OpFMul %float %float_7 %2144
       %2146 = OpFSub %float %2131 %2123
       %2147 = OpFDiv %float %2145 %2146
       %2148 = OpConvertFToS %int %2147
       %2149 = OpConvertSToF %float %2148
       %2150 = OpFSub %float %2147 %2149
       %2151 = OpAccessChain %_ptr_Function_float %533 %2148
       %2152 = OpLoad %float %2151
       %2153 = OpIAdd %int %2148 %int_1
       %2154 = OpAccessChain %_ptr_Function_float %533 %2153
       %2155 = OpLoad %float %2154
       %2156 = OpIAdd %int %2148 %int_2
       %2157 = OpAccessChain %_ptr_Function_float %533 %2156
       %2158 = OpLoad %float %2157
       %2159 = OpCompositeConstruct %v3float %2152 %2155 %2158
       %2160 = OpFMul %float %2150 %2150
       %2161 = OpCompositeConstruct %v3float %2160 %2150 %float_1
       %2162 = OpMatrixTimesVector %v3float %466 %2159
       %2163 = OpDot %float %2161 %2162
               OpBranch %2134
       %2134 = OpLabel
       %2164 = OpPhi %float %2143 %2136 %2163 %2135
               OpBranch %2126
       %2127 = OpLabel
       %2165 = OpFSub %float %2116 %1958
       %2166 = OpFMul %float %float_7 %2165
       %2167 = OpFSub %float %2123 %1958
       %2168 = OpFDiv %float %2166 %2167
       %2169 = OpConvertFToS %int %2168
       %2170 = OpConvertSToF %float %2169
       %2171 = OpFSub %float %2168 %2170
       %2172 = OpAccessChain %_ptr_Function_float %534 %2169
       %2173 = OpLoad %float %2172
       %2174 = OpIAdd %int %2169 %int_1
       %2175 = OpAccessChain %_ptr_Function_float %534 %2174
       %2176 = OpLoad %float %2175
       %2177 = OpIAdd %int %2169 %int_2
       %2178 = OpAccessChain %_ptr_Function_float %534 %2177
       %2179 = OpLoad %float %2178
       %2180 = OpCompositeConstruct %v3float %2173 %2176 %2179
       %2181 = OpFMul %float %2171 %2171
       %2182 = OpCompositeConstruct %v3float %2181 %2171 %float_1
       %2183 = OpMatrixTimesVector %v3float %466 %2180
       %2184 = OpDot %float %2182 %2183
               OpBranch %2126
       %2126 = OpLabel
       %2185 = OpPhi %float %2164 %2134 %2184 %2127
               OpBranch %2118
       %2119 = OpLabel
       %2186 = OpExtInst %float %1 Log %float_0_00499999989
       %2187 = OpFDiv %float %2186 %1091
               OpBranch %2118
       %2118 = OpLabel
       %2188 = OpPhi %float %2185 %2126 %2187 %2119
       %2189 = OpExtInst %float %1 Pow %float_10 %2188
       %2190 = OpCompositeInsert %v3float %2189 %2111 2
       %2191 = OpVectorTimesMatrix %v3float %2190 %602
       %2192 = OpFMul %v3float %2191 %519
       %2193 = OpExtInst %v3float %1 Pow %2192 %286
       %2194 = OpFMul %v3float %196 %2193
       %2195 = OpFAdd %v3float %195 %2194
       %2196 = OpFMul %v3float %197 %2193
       %2197 = OpFAdd %v3float %141 %2196
       %2198 = OpFDiv %v3float %141 %2197
       %2199 = OpFMul %v3float %2195 %2198
       %2200 = OpExtInst %v3float %1 Pow %2199 %287
               OpBranch %1342
       %1342 = OpLabel
       %2201 = OpPhi %v3float %1366 %1346 %2200 %2118
               OpBranch %1336
       %1337 = OpLabel
       %2202 = OpMatrixTimesMatrix %mat3v3float %572 %423
       %2203 = OpFMul %v3float %932 %285
       %2204 = OpVectorTimesMatrix %v3float %2203 %2202
       %2205 = OpCompositeExtract %float %2204 0
       %2206 = OpCompositeExtract %float %2204 1
       %2207 = OpExtInst %float %1 FMin %2205 %2206
       %2208 = OpCompositeExtract %float %2204 2
       %2209 = OpExtInst %float %1 FMin %2207 %2208
       %2210 = OpExtInst %float %1 FMax %2205 %2206
       %2211 = OpExtInst %float %1 FMax %2210 %2208
       %2212 = OpExtInst %float %1 FMax %2211 %float_1_00000001en10
       %2213 = OpExtInst %float %1 FMax %2209 %float_1_00000001en10
       %2214 = OpFSub %float %2212 %2213
       %2215 = OpExtInst %float %1 FMax %2211 %float_0_00999999978
       %2216 = OpFDiv %float %2214 %2215
       %2217 = OpFSub %float %2208 %2206
       %2218 = OpFMul %float %2208 %2217
       %2219 = OpFSub %float %2206 %2205
       %2220 = OpFMul %float %2206 %2219
       %2221 = OpFAdd %float %2218 %2220
       %2222 = OpFSub %float %2205 %2208
       %2223 = OpFMul %float %2205 %2222
       %2224 = OpFAdd %float %2221 %2223
       %2225 = OpExtInst %float %1 Sqrt %2224
       %2226 = OpFAdd %float %2208 %2206
       %2227 = OpFAdd %float %2226 %2205
       %2228 = OpFMul %float %float_1_75 %2225
       %2229 = OpFAdd %float %2227 %2228
       %2230 = OpFMul %float %2229 %float_0_333333343
       %2231 = OpFSub %float %2216 %float_0_400000006
       %2232 = OpFMul %float %2231 %float_5
       %2233 = OpFMul %float %2231 %float_2_5
       %2234 = OpExtInst %float %1 FAbs %2233
       %2235 = OpFSub %float %float_1 %2234
       %2236 = OpExtInst %float %1 FMax %2235 %float_0
       %2237 = OpExtInst %float %1 FSign %2232
       %2238 = OpConvertFToS %int %2237
       %2239 = OpConvertSToF %float %2238
       %2240 = OpFMul %float %2236 %2236
       %2241 = OpFSub %float %float_1 %2240
       %2242 = OpFMul %float %2239 %2241
       %2243 = OpFAdd %float %float_1 %2242
       %2244 = OpFMul %float %2243 %float_0_0250000004
       %2245 = OpFOrdLessThanEqual %bool %2230 %float_0_0533333346
               OpSelectionMerge %2246 None
               OpBranchConditional %2245 %2247 %2248
       %2248 = OpLabel
       %2249 = OpFOrdGreaterThanEqual %bool %2230 %float_0_159999996
               OpSelectionMerge %2250 None
               OpBranchConditional %2249 %2251 %2252
       %2252 = OpLabel
       %2253 = OpFDiv %float %float_0_239999995 %2229
       %2254 = OpFSub %float %2253 %float_0_5
       %2255 = OpFMul %float %2244 %2254
               OpBranch %2250
       %2251 = OpLabel
               OpBranch %2250
       %2250 = OpLabel
       %2256 = OpPhi %float %2255 %2252 %float_0 %2251
               OpBranch %2246
       %2247 = OpLabel
               OpBranch %2246
       %2246 = OpLabel
       %2257 = OpPhi %float %2256 %2250 %2244 %2247
       %2258 = OpFAdd %float %float_1 %2257
       %2259 = OpCompositeConstruct %v3float %2258 %2258 %2258
       %2260 = OpFMul %v3float %2204 %2259
       %2261 = OpCompositeExtract %float %2260 0
       %2262 = OpCompositeExtract %float %2260 1
       %2263 = OpFOrdEqual %bool %2261 %2262
       %2264 = OpCompositeExtract %float %2260 2
       %2265 = OpFOrdEqual %bool %2262 %2264
       %2266 = OpLogicalAnd %bool %2263 %2265
               OpSelectionMerge %2267 None
               OpBranchConditional %2266 %2268 %2269
       %2269 = OpLabel
       %2270 = OpExtInst %float %1 Sqrt %float_3
       %2271 = OpFSub %float %2262 %2264
       %2272 = OpFMul %float %2270 %2271
       %2273 = OpFMul %float %float_2 %2261
       %2274 = OpFSub %float %2273 %2262
       %2275 = OpFSub %float %2274 %2264
       %2276 = OpExtInst %float %1 Atan2 %2272 %2275
       %2277 = OpFMul %float %float_57_2957764 %2276
               OpBranch %2267
       %2268 = OpLabel
               OpBranch %2267
       %2267 = OpLabel
       %2278 = OpPhi %float %2277 %2269 %float_0 %2268
       %2279 = OpFOrdLessThan %bool %2278 %float_0
               OpSelectionMerge %2280 None
               OpBranchConditional %2279 %2281 %2280
       %2281 = OpLabel
       %2282 = OpFAdd %float %2278 %float_360
               OpBranch %2280
       %2280 = OpLabel
       %2283 = OpPhi %float %2278 %2267 %2282 %2281
       %2284 = OpExtInst %float %1 FClamp %2283 %float_0 %float_360
       %2285 = OpFOrdGreaterThan %bool %2284 %float_180
               OpSelectionMerge %2286 None
               OpBranchConditional %2285 %2287 %2286
       %2287 = OpLabel
       %2288 = OpFSub %float %2284 %float_360
               OpBranch %2286
       %2286 = OpLabel
       %2289 = OpPhi %float %2284 %2280 %2288 %2287
       %2290 = OpFOrdGreaterThan %bool %2289 %float_n67_5
       %2291 = OpFOrdLessThan %bool %2289 %float_67_5
       %2292 = OpLogicalAnd %bool %2290 %2291
               OpSelectionMerge %2293 None
               OpBranchConditional %2292 %2294 %2293
       %2294 = OpLabel
       %2295 = OpFSub %float %2289 %float_n67_5
       %2296 = OpFMul %float %2295 %float_0_0296296291
       %2297 = OpConvertFToS %int %2296
       %2298 = OpConvertSToF %float %2297
       %2299 = OpFSub %float %2296 %2298
       %2300 = OpFMul %float %2299 %2299
       %2301 = OpFMul %float %2300 %2299
       %2302 = OpIEqual %bool %2297 %int_3
               OpSelectionMerge %2303 None
               OpBranchConditional %2302 %2304 %2305
       %2305 = OpLabel
       %2306 = OpIEqual %bool %2297 %int_2
               OpSelectionMerge %2307 None
               OpBranchConditional %2306 %2308 %2309
       %2309 = OpLabel
       %2310 = OpIEqual %bool %2297 %int_1
               OpSelectionMerge %2311 None
               OpBranchConditional %2310 %2312 %2313
       %2313 = OpLabel
       %2314 = OpIEqual %bool %2297 %int_0
               OpSelectionMerge %2315 None
               OpBranchConditional %2314 %2316 %2317
       %2317 = OpLabel
               OpBranch %2315
       %2316 = OpLabel
       %2318 = OpFMul %float %2301 %float_0_166666672
               OpBranch %2315
       %2315 = OpLabel
       %2319 = OpPhi %float %float_0 %2317 %2318 %2316
               OpBranch %2311
       %2312 = OpLabel
       %2320 = OpFMul %float %2301 %float_n0_5
       %2321 = OpFMul %float %2300 %float_0_5
       %2322 = OpFAdd %float %2320 %2321
       %2323 = OpFMul %float %2299 %float_0_5
       %2324 = OpFAdd %float %2322 %2323
       %2325 = OpFAdd %float %2324 %float_0_166666672
               OpBranch %2311
       %2311 = OpLabel
       %2326 = OpPhi %float %2319 %2315 %2325 %2312
               OpBranch %2307
       %2308 = OpLabel
       %2327 = OpFMul %float %2301 %float_0_5
       %2328 = OpFMul %float %2300 %float_n1
       %2329 = OpFAdd %float %2327 %2328
       %2330 = OpFAdd %float %2329 %float_0_666666687
               OpBranch %2307
       %2307 = OpLabel
       %2331 = OpPhi %float %2326 %2311 %2330 %2308
               OpBranch %2303
       %2304 = OpLabel
       %2332 = OpFMul %float %2301 %float_n0_166666672
       %2333 = OpFMul %float %2300 %float_0_5
       %2334 = OpFAdd %float %2332 %2333
       %2335 = OpFMul %float %2299 %float_n0_5
       %2336 = OpFAdd %float %2334 %2335
       %2337 = OpFAdd %float %2336 %float_0_166666672
               OpBranch %2303
       %2303 = OpLabel
       %2338 = OpPhi %float %2331 %2307 %2337 %2304
               OpBranch %2293
       %2293 = OpLabel
       %2339 = OpPhi %float %float_0 %2286 %2338 %2303
       %2340 = OpFMul %float %2339 %float_1_5
       %2341 = OpFMul %float %2340 %2216
       %2342 = OpFSub %float %float_0_0299999993 %2261
       %2343 = OpFMul %float %2341 %2342
       %2344 = OpFMul %float %2343 %float_0_180000007
       %2345 = OpFAdd %float %2261 %2344
       %2346 = OpCompositeInsert %v3float %2345 %2260 0
       %2347 = OpExtInst %v3float %1 FClamp %2346 %138 %337
       %2348 = OpVectorTimesMatrix %v3float %2347 %434
       %2349 = OpExtInst %v3float %1 FClamp %2348 %138 %337
       %2350 = OpDot %float %2349 %73
       %2351 = OpCompositeConstruct %v3float %2350 %2350 %2350
       %2352 = OpExtInst %v3float %1 FMix %2351 %2349 %241
       %2353 = OpCompositeExtract %float %2352 0
       %2354 = OpExtInst %float %1 Exp2 %float_n15
       %2355 = OpFMul %float %float_0_179999992 %2354
       %2356 = OpExtInst %float %1 Exp2 %float_18
       %2357 = OpFMul %float %float_0_179999992 %2356
               OpStore %546 %499
               OpStore %545 %500
       %2358 = OpFOrdLessThanEqual %bool %2353 %float_0
       %2359 = OpExtInst %float %1 Exp2 %float_n14
       %2360 = OpSelect %float %2358 %2359 %2353
       %2361 = OpExtInst %float %1 Log %2360
       %2362 = OpFDiv %float %2361 %1091
       %2363 = OpExtInst %float %1 Log %2355
       %2364 = OpFDiv %float %2363 %1091
       %2365 = OpFOrdLessThanEqual %bool %2362 %2364
               OpSelectionMerge %2366 None
               OpBranchConditional %2365 %2367 %2368
       %2368 = OpLabel
       %2369 = OpFOrdGreaterThan %bool %2362 %2364
       %2370 = OpExtInst %float %1 Log %float_0_180000007
       %2371 = OpFDiv %float %2370 %1091
       %2372 = OpFOrdLessThan %bool %2362 %2371
       %2373 = OpLogicalAnd %bool %2369 %2372
               OpSelectionMerge %2374 None
               OpBranchConditional %2373 %2375 %2376
       %2376 = OpLabel
       %2377 = OpFOrdGreaterThanEqual %bool %2362 %2371
       %2378 = OpExtInst %float %1 Log %2357
       %2379 = OpFDiv %float %2378 %1091
       %2380 = OpFOrdLessThan %bool %2362 %2379
       %2381 = OpLogicalAnd %bool %2377 %2380
               OpSelectionMerge %2382 None
               OpBranchConditional %2381 %2383 %2384
       %2384 = OpLabel
       %2385 = OpExtInst %float %1 Log %float_10000
       %2386 = OpFDiv %float %2385 %1091
               OpBranch %2382
       %2383 = OpLabel
       %2387 = OpFSub %float %2362 %2371
       %2388 = OpFMul %float %float_3 %2387
       %2389 = OpFSub %float %2379 %2371
       %2390 = OpFDiv %float %2388 %2389
       %2391 = OpConvertFToS %int %2390
       %2392 = OpConvertSToF %float %2391
       %2393 = OpFSub %float %2390 %2392
       %2394 = OpAccessChain %_ptr_Function_float %545 %2391
       %2395 = OpLoad %float %2394
       %2396 = OpIAdd %int %2391 %int_1
       %2397 = OpAccessChain %_ptr_Function_float %545 %2396
       %2398 = OpLoad %float %2397
       %2399 = OpIAdd %int %2391 %int_2
       %2400 = OpAccessChain %_ptr_Function_float %545 %2399
       %2401 = OpLoad %float %2400
       %2402 = OpCompositeConstruct %v3float %2395 %2398 %2401
       %2403 = OpFMul %float %2393 %2393
       %2404 = OpCompositeConstruct %v3float %2403 %2393 %float_1
       %2405 = OpMatrixTimesVector %v3float %466 %2402
       %2406 = OpDot %float %2404 %2405
               OpBranch %2382
       %2382 = OpLabel
       %2407 = OpPhi %float %2386 %2384 %2406 %2383
               OpBranch %2374
       %2375 = OpLabel
       %2408 = OpFSub %float %2362 %2364
       %2409 = OpFMul %float %float_3 %2408
       %2410 = OpFSub %float %2371 %2364
       %2411 = OpFDiv %float %2409 %2410
       %2412 = OpConvertFToS %int %2411
       %2413 = OpConvertSToF %float %2412
       %2414 = OpFSub %float %2411 %2413
       %2415 = OpAccessChain %_ptr_Function_float %546 %2412
       %2416 = OpLoad %float %2415
       %2417 = OpIAdd %int %2412 %int_1
       %2418 = OpAccessChain %_ptr_Function_float %546 %2417
       %2419 = OpLoad %float %2418
       %2420 = OpIAdd %int %2412 %int_2
       %2421 = OpAccessChain %_ptr_Function_float %546 %2420
       %2422 = OpLoad %float %2421
       %2423 = OpCompositeConstruct %v3float %2416 %2419 %2422
       %2424 = OpFMul %float %2414 %2414
       %2425 = OpCompositeConstruct %v3float %2424 %2414 %float_1
       %2426 = OpMatrixTimesVector %v3float %466 %2423
       %2427 = OpDot %float %2425 %2426
               OpBranch %2374
       %2374 = OpLabel
       %2428 = OpPhi %float %2407 %2382 %2427 %2375
               OpBranch %2366
       %2367 = OpLabel
       %2429 = OpExtInst %float %1 Log %float_9_99999975en05
       %2430 = OpFDiv %float %2429 %1091
               OpBranch %2366
       %2366 = OpLabel
       %2431 = OpPhi %float %2428 %2374 %2430 %2367
       %2432 = OpExtInst %float %1 Pow %float_10 %2431
       %2433 = OpCompositeInsert %v3float %2432 %523 0
       %2434 = OpCompositeExtract %float %2352 1
               OpStore %548 %499
               OpStore %547 %500
       %2435 = OpFOrdLessThanEqual %bool %2434 %float_0
       %2436 = OpSelect %float %2435 %2359 %2434
       %2437 = OpExtInst %float %1 Log %2436
       %2438 = OpFDiv %float %2437 %1091
       %2439 = OpFOrdLessThanEqual %bool %2438 %2364
               OpSelectionMerge %2440 None
               OpBranchConditional %2439 %2441 %2442
       %2442 = OpLabel
       %2443 = OpFOrdGreaterThan %bool %2438 %2364
       %2444 = OpExtInst %float %1 Log %float_0_180000007
       %2445 = OpFDiv %float %2444 %1091
       %2446 = OpFOrdLessThan %bool %2438 %2445
       %2447 = OpLogicalAnd %bool %2443 %2446
               OpSelectionMerge %2448 None
               OpBranchConditional %2447 %2449 %2450
       %2450 = OpLabel
       %2451 = OpFOrdGreaterThanEqual %bool %2438 %2445
       %2452 = OpExtInst %float %1 Log %2357
       %2453 = OpFDiv %float %2452 %1091
       %2454 = OpFOrdLessThan %bool %2438 %2453
       %2455 = OpLogicalAnd %bool %2451 %2454
               OpSelectionMerge %2456 None
               OpBranchConditional %2455 %2457 %2458
       %2458 = OpLabel
       %2459 = OpExtInst %float %1 Log %float_10000
       %2460 = OpFDiv %float %2459 %1091
               OpBranch %2456
       %2457 = OpLabel
       %2461 = OpFSub %float %2438 %2445
       %2462 = OpFMul %float %float_3 %2461
       %2463 = OpFSub %float %2453 %2445
       %2464 = OpFDiv %float %2462 %2463
       %2465 = OpConvertFToS %int %2464
       %2466 = OpConvertSToF %float %2465
       %2467 = OpFSub %float %2464 %2466
       %2468 = OpAccessChain %_ptr_Function_float %547 %2465
       %2469 = OpLoad %float %2468
       %2470 = OpIAdd %int %2465 %int_1
       %2471 = OpAccessChain %_ptr_Function_float %547 %2470
       %2472 = OpLoad %float %2471
       %2473 = OpIAdd %int %2465 %int_2
       %2474 = OpAccessChain %_ptr_Function_float %547 %2473
       %2475 = OpLoad %float %2474
       %2476 = OpCompositeConstruct %v3float %2469 %2472 %2475
       %2477 = OpFMul %float %2467 %2467
       %2478 = OpCompositeConstruct %v3float %2477 %2467 %float_1
       %2479 = OpMatrixTimesVector %v3float %466 %2476
       %2480 = OpDot %float %2478 %2479
               OpBranch %2456
       %2456 = OpLabel
       %2481 = OpPhi %float %2460 %2458 %2480 %2457
               OpBranch %2448
       %2449 = OpLabel
       %2482 = OpFSub %float %2438 %2364
       %2483 = OpFMul %float %float_3 %2482
       %2484 = OpFSub %float %2445 %2364
       %2485 = OpFDiv %float %2483 %2484
       %2486 = OpConvertFToS %int %2485
       %2487 = OpConvertSToF %float %2486
       %2488 = OpFSub %float %2485 %2487
       %2489 = OpAccessChain %_ptr_Function_float %548 %2486
       %2490 = OpLoad %float %2489
       %2491 = OpIAdd %int %2486 %int_1
       %2492 = OpAccessChain %_ptr_Function_float %548 %2491
       %2493 = OpLoad %float %2492
       %2494 = OpIAdd %int %2486 %int_2
       %2495 = OpAccessChain %_ptr_Function_float %548 %2494
       %2496 = OpLoad %float %2495
       %2497 = OpCompositeConstruct %v3float %2490 %2493 %2496
       %2498 = OpFMul %float %2488 %2488
       %2499 = OpCompositeConstruct %v3float %2498 %2488 %float_1
       %2500 = OpMatrixTimesVector %v3float %466 %2497
       %2501 = OpDot %float %2499 %2500
               OpBranch %2448
       %2448 = OpLabel
       %2502 = OpPhi %float %2481 %2456 %2501 %2449
               OpBranch %2440
       %2441 = OpLabel
       %2503 = OpExtInst %float %1 Log %float_9_99999975en05
       %2504 = OpFDiv %float %2503 %1091
               OpBranch %2440
       %2440 = OpLabel
       %2505 = OpPhi %float %2502 %2448 %2504 %2441
       %2506 = OpExtInst %float %1 Pow %float_10 %2505
       %2507 = OpCompositeInsert %v3float %2506 %2433 1
       %2508 = OpCompositeExtract %float %2352 2
               OpStore %550 %499
               OpStore %549 %500
       %2509 = OpFOrdLessThanEqual %bool %2508 %float_0
       %2510 = OpSelect %float %2509 %2359 %2508
       %2511 = OpExtInst %float %1 Log %2510
       %2512 = OpFDiv %float %2511 %1091
       %2513 = OpFOrdLessThanEqual %bool %2512 %2364
               OpSelectionMerge %2514 None
               OpBranchConditional %2513 %2515 %2516
       %2516 = OpLabel
       %2517 = OpFOrdGreaterThan %bool %2512 %2364
       %2518 = OpExtInst %float %1 Log %float_0_180000007
       %2519 = OpFDiv %float %2518 %1091
       %2520 = OpFOrdLessThan %bool %2512 %2519
       %2521 = OpLogicalAnd %bool %2517 %2520
               OpSelectionMerge %2522 None
               OpBranchConditional %2521 %2523 %2524
       %2524 = OpLabel
       %2525 = OpFOrdGreaterThanEqual %bool %2512 %2519
       %2526 = OpExtInst %float %1 Log %2357
       %2527 = OpFDiv %float %2526 %1091
       %2528 = OpFOrdLessThan %bool %2512 %2527
       %2529 = OpLogicalAnd %bool %2525 %2528
               OpSelectionMerge %2530 None
               OpBranchConditional %2529 %2531 %2532
       %2532 = OpLabel
       %2533 = OpExtInst %float %1 Log %float_10000
       %2534 = OpFDiv %float %2533 %1091
               OpBranch %2530
       %2531 = OpLabel
       %2535 = OpFSub %float %2512 %2519
       %2536 = OpFMul %float %float_3 %2535
       %2537 = OpFSub %float %2527 %2519
       %2538 = OpFDiv %float %2536 %2537
       %2539 = OpConvertFToS %int %2538
       %2540 = OpConvertSToF %float %2539
       %2541 = OpFSub %float %2538 %2540
       %2542 = OpAccessChain %_ptr_Function_float %549 %2539
       %2543 = OpLoad %float %2542
       %2544 = OpIAdd %int %2539 %int_1
       %2545 = OpAccessChain %_ptr_Function_float %549 %2544
       %2546 = OpLoad %float %2545
       %2547 = OpIAdd %int %2539 %int_2
       %2548 = OpAccessChain %_ptr_Function_float %549 %2547
       %2549 = OpLoad %float %2548
       %2550 = OpCompositeConstruct %v3float %2543 %2546 %2549
       %2551 = OpFMul %float %2541 %2541
       %2552 = OpCompositeConstruct %v3float %2551 %2541 %float_1
       %2553 = OpMatrixTimesVector %v3float %466 %2550
       %2554 = OpDot %float %2552 %2553
               OpBranch %2530
       %2530 = OpLabel
       %2555 = OpPhi %float %2534 %2532 %2554 %2531
               OpBranch %2522
       %2523 = OpLabel
       %2556 = OpFSub %float %2512 %2364
       %2557 = OpFMul %float %float_3 %2556
       %2558 = OpFSub %float %2519 %2364
       %2559 = OpFDiv %float %2557 %2558
       %2560 = OpConvertFToS %int %2559
       %2561 = OpConvertSToF %float %2560
       %2562 = OpFSub %float %2559 %2561
       %2563 = OpAccessChain %_ptr_Function_float %550 %2560
       %2564 = OpLoad %float %2563
       %2565 = OpIAdd %int %2560 %int_1
       %2566 = OpAccessChain %_ptr_Function_float %550 %2565
       %2567 = OpLoad %float %2566
       %2568 = OpIAdd %int %2560 %int_2
       %2569 = OpAccessChain %_ptr_Function_float %550 %2568
       %2570 = OpLoad %float %2569
       %2571 = OpCompositeConstruct %v3float %2564 %2567 %2570
       %2572 = OpFMul %float %2562 %2562
       %2573 = OpCompositeConstruct %v3float %2572 %2562 %float_1
       %2574 = OpMatrixTimesVector %v3float %466 %2571
       %2575 = OpDot %float %2573 %2574
               OpBranch %2522
       %2522 = OpLabel
       %2576 = OpPhi %float %2555 %2530 %2575 %2523
               OpBranch %2514
       %2515 = OpLabel
       %2577 = OpExtInst %float %1 Log %float_9_99999975en05
       %2578 = OpFDiv %float %2577 %1091
               OpBranch %2514
       %2514 = OpLabel
       %2579 = OpPhi %float %2576 %2522 %2578 %2515
       %2580 = OpExtInst %float %1 Pow %float_10 %2579
       %2581 = OpCompositeInsert %v3float %2580 %2507 2
       %2582 = OpVectorTimesMatrix %v3float %2581 %438
       %2583 = OpVectorTimesMatrix %v3float %2582 %434
       %2584 = OpExtInst %float %1 Pow %float_2 %float_n12
       %2585 = OpFMul %float %float_0_179999992 %2584
               OpStore %558 %499
               OpStore %557 %500
       %2586 = OpFOrdLessThanEqual %bool %2585 %float_0
       %2587 = OpSelect %float %2586 %2359 %2585
       %2588 = OpExtInst %float %1 Log %2587
       %2589 = OpFDiv %float %2588 %1091
       %2590 = OpFOrdLessThanEqual %bool %2589 %2364
               OpSelectionMerge %2591 None
               OpBranchConditional %2590 %2592 %2593
       %2593 = OpLabel
       %2594 = OpFOrdGreaterThan %bool %2589 %2364
       %2595 = OpExtInst %float %1 Log %float_0_180000007
       %2596 = OpFDiv %float %2595 %1091
       %2597 = OpFOrdLessThan %bool %2589 %2596
       %2598 = OpLogicalAnd %bool %2594 %2597
               OpSelectionMerge %2599 None
               OpBranchConditional %2598 %2600 %2601
       %2601 = OpLabel
       %2602 = OpFOrdGreaterThanEqual %bool %2589 %2596
       %2603 = OpExtInst %float %1 Log %2357
       %2604 = OpFDiv %float %2603 %1091
       %2605 = OpFOrdLessThan %bool %2589 %2604
       %2606 = OpLogicalAnd %bool %2602 %2605
               OpSelectionMerge %2607 None
               OpBranchConditional %2606 %2608 %2609
       %2609 = OpLabel
       %2610 = OpExtInst %float %1 Log %float_10000
       %2611 = OpFDiv %float %2610 %1091
               OpBranch %2607
       %2608 = OpLabel
       %2612 = OpFSub %float %2589 %2596
       %2613 = OpFMul %float %float_3 %2612
       %2614 = OpFSub %float %2604 %2596
       %2615 = OpFDiv %float %2613 %2614
       %2616 = OpConvertFToS %int %2615
       %2617 = OpConvertSToF %float %2616
       %2618 = OpFSub %float %2615 %2617
       %2619 = OpAccessChain %_ptr_Function_float %557 %2616
       %2620 = OpLoad %float %2619
       %2621 = OpIAdd %int %2616 %int_1
       %2622 = OpAccessChain %_ptr_Function_float %557 %2621
       %2623 = OpLoad %float %2622
       %2624 = OpIAdd %int %2616 %int_2
       %2625 = OpAccessChain %_ptr_Function_float %557 %2624
       %2626 = OpLoad %float %2625
       %2627 = OpCompositeConstruct %v3float %2620 %2623 %2626
       %2628 = OpFMul %float %2618 %2618
       %2629 = OpCompositeConstruct %v3float %2628 %2618 %float_1
       %2630 = OpMatrixTimesVector %v3float %466 %2627
       %2631 = OpDot %float %2629 %2630
               OpBranch %2607
       %2607 = OpLabel
       %2632 = OpPhi %float %2611 %2609 %2631 %2608
               OpBranch %2599
       %2600 = OpLabel
       %2633 = OpFSub %float %2589 %2364
       %2634 = OpFMul %float %float_3 %2633
       %2635 = OpFSub %float %2596 %2364
       %2636 = OpFDiv %float %2634 %2635
       %2637 = OpConvertFToS %int %2636
       %2638 = OpConvertSToF %float %2637
       %2639 = OpFSub %float %2636 %2638
       %2640 = OpAccessChain %_ptr_Function_float %558 %2637
       %2641 = OpLoad %float %2640
       %2642 = OpIAdd %int %2637 %int_1
       %2643 = OpAccessChain %_ptr_Function_float %558 %2642
       %2644 = OpLoad %float %2643
       %2645 = OpIAdd %int %2637 %int_2
       %2646 = OpAccessChain %_ptr_Function_float %558 %2645
       %2647 = OpLoad %float %2646
       %2648 = OpCompositeConstruct %v3float %2641 %2644 %2647
       %2649 = OpFMul %float %2639 %2639
       %2650 = OpCompositeConstruct %v3float %2649 %2639 %float_1
       %2651 = OpMatrixTimesVector %v3float %466 %2648
       %2652 = OpDot %float %2650 %2651
               OpBranch %2599
       %2599 = OpLabel
       %2653 = OpPhi %float %2632 %2607 %2652 %2600
               OpBranch %2591
       %2592 = OpLabel
       %2654 = OpExtInst %float %1 Log %float_9_99999975en05
       %2655 = OpFDiv %float %2654 %1091
               OpBranch %2591
       %2591 = OpLabel
       %2656 = OpPhi %float %2653 %2599 %2655 %2592
       %2657 = OpExtInst %float %1 Pow %float_10 %2656
               OpStore %560 %499
               OpStore %559 %500
       %2658 = OpExtInst %float %1 Log %float_0_180000007
       %2659 = OpFDiv %float %2658 %1091
       %2660 = OpFOrdLessThanEqual %bool %2659 %2364
               OpSelectionMerge %2661 None
               OpBranchConditional %2660 %2662 %2663
       %2663 = OpLabel
       %2664 = OpFOrdGreaterThan %bool %2659 %2364
       %2665 = OpFOrdLessThan %bool %2659 %2659
       %2666 = OpLogicalAnd %bool %2664 %2665
               OpSelectionMerge %2667 None
               OpBranchConditional %2666 %2668 %2669
       %2669 = OpLabel
       %2670 = OpFOrdGreaterThanEqual %bool %2659 %2659
       %2671 = OpExtInst %float %1 Log %2357
       %2672 = OpFDiv %float %2671 %1091
       %2673 = OpFOrdLessThan %bool %2659 %2672
       %2674 = OpLogicalAnd %bool %2670 %2673
               OpSelectionMerge %2675 None
               OpBranchConditional %2674 %2676 %2677
       %2677 = OpLabel
       %2678 = OpExtInst %float %1 Log %float_10000
       %2679 = OpFDiv %float %2678 %1091
               OpBranch %2675
       %2676 = OpLabel
       %2680 = OpFSub %float %2659 %2659
       %2681 = OpFMul %float %float_3 %2680
       %2682 = OpFSub %float %2672 %2659
       %2683 = OpFDiv %float %2681 %2682
       %2684 = OpConvertFToS %int %2683
       %2685 = OpConvertSToF %float %2684
       %2686 = OpFSub %float %2683 %2685
       %2687 = OpAccessChain %_ptr_Function_float %559 %2684
       %2688 = OpLoad %float %2687
       %2689 = OpIAdd %int %2684 %int_1
       %2690 = OpAccessChain %_ptr_Function_float %559 %2689
       %2691 = OpLoad %float %2690
       %2692 = OpIAdd %int %2684 %int_2
       %2693 = OpAccessChain %_ptr_Function_float %559 %2692
       %2694 = OpLoad %float %2693
       %2695 = OpCompositeConstruct %v3float %2688 %2691 %2694
       %2696 = OpFMul %float %2686 %2686
       %2697 = OpCompositeConstruct %v3float %2696 %2686 %float_1
       %2698 = OpMatrixTimesVector %v3float %466 %2695
       %2699 = OpDot %float %2697 %2698
               OpBranch %2675
       %2675 = OpLabel
       %2700 = OpPhi %float %2679 %2677 %2699 %2676
               OpBranch %2667
       %2668 = OpLabel
       %2701 = OpAccessChain %_ptr_Function_float %560 %int_3
       %2702 = OpLoad %float %2701
       %2703 = OpAccessChain %_ptr_Function_float %560 %int_4
       %2704 = OpLoad %float %2703
       %2705 = OpAccessChain %_ptr_Function_float %560 %int_5
       %2706 = OpLoad %float %2705
       %2707 = OpCompositeConstruct %v3float %2702 %2704 %2706
       %2708 = OpMatrixTimesVector %v3float %466 %2707
       %2709 = OpCompositeExtract %float %2708 2
               OpBranch %2667
       %2667 = OpLabel
       %2710 = OpPhi %float %2700 %2675 %2709 %2668
               OpBranch %2661
       %2662 = OpLabel
       %2711 = OpExtInst %float %1 Log %float_9_99999975en05
       %2712 = OpFDiv %float %2711 %1091
               OpBranch %2661
       %2661 = OpLabel
       %2713 = OpPhi %float %2710 %2667 %2712 %2662
       %2714 = OpExtInst %float %1 Pow %float_10 %2713
       %2715 = OpExtInst %float %1 Pow %float_2 %float_10
       %2716 = OpFMul %float %float_0_179999992 %2715
               OpStore %562 %499
               OpStore %561 %500
       %2717 = OpFOrdLessThanEqual %bool %2716 %float_0
       %2718 = OpSelect %float %2717 %2359 %2716
       %2719 = OpExtInst %float %1 Log %2718
       %2720 = OpFDiv %float %2719 %1091
       %2721 = OpFOrdLessThanEqual %bool %2720 %2364
               OpSelectionMerge %2722 None
               OpBranchConditional %2721 %2723 %2724
       %2724 = OpLabel
       %2725 = OpFOrdGreaterThan %bool %2720 %2364
       %2726 = OpFOrdLessThan %bool %2720 %2659
       %2727 = OpLogicalAnd %bool %2725 %2726
               OpSelectionMerge %2728 None
               OpBranchConditional %2727 %2729 %2730
       %2730 = OpLabel
       %2731 = OpFOrdGreaterThanEqual %bool %2720 %2659
       %2732 = OpExtInst %float %1 Log %2357
       %2733 = OpFDiv %float %2732 %1091
       %2734 = OpFOrdLessThan %bool %2720 %2733
       %2735 = OpLogicalAnd %bool %2731 %2734
               OpSelectionMerge %2736 None
               OpBranchConditional %2735 %2737 %2738
       %2738 = OpLabel
       %2739 = OpExtInst %float %1 Log %float_10000
       %2740 = OpFDiv %float %2739 %1091
               OpBranch %2736
       %2737 = OpLabel
       %2741 = OpFSub %float %2720 %2659
       %2742 = OpFMul %float %float_3 %2741
       %2743 = OpFSub %float %2733 %2659
       %2744 = OpFDiv %float %2742 %2743
       %2745 = OpConvertFToS %int %2744
       %2746 = OpConvertSToF %float %2745
       %2747 = OpFSub %float %2744 %2746
       %2748 = OpAccessChain %_ptr_Function_float %561 %2745
       %2749 = OpLoad %float %2748
       %2750 = OpIAdd %int %2745 %int_1
       %2751 = OpAccessChain %_ptr_Function_float %561 %2750
       %2752 = OpLoad %float %2751
       %2753 = OpIAdd %int %2745 %int_2
       %2754 = OpAccessChain %_ptr_Function_float %561 %2753
       %2755 = OpLoad %float %2754
       %2756 = OpCompositeConstruct %v3float %2749 %2752 %2755
       %2757 = OpFMul %float %2747 %2747
       %2758 = OpCompositeConstruct %v3float %2757 %2747 %float_1
       %2759 = OpMatrixTimesVector %v3float %466 %2756
       %2760 = OpDot %float %2758 %2759
               OpBranch %2736
       %2736 = OpLabel
       %2761 = OpPhi %float %2740 %2738 %2760 %2737
               OpBranch %2728
       %2729 = OpLabel
       %2762 = OpFSub %float %2720 %2364
       %2763 = OpFMul %float %float_3 %2762
       %2764 = OpFSub %float %2659 %2364
       %2765 = OpFDiv %float %2763 %2764
       %2766 = OpConvertFToS %int %2765
       %2767 = OpConvertSToF %float %2766
       %2768 = OpFSub %float %2765 %2767
       %2769 = OpAccessChain %_ptr_Function_float %562 %2766
       %2770 = OpLoad %float %2769
       %2771 = OpIAdd %int %2766 %int_1
       %2772 = OpAccessChain %_ptr_Function_float %562 %2771
       %2773 = OpLoad %float %2772
       %2774 = OpIAdd %int %2766 %int_2
       %2775 = OpAccessChain %_ptr_Function_float %562 %2774
       %2776 = OpLoad %float %2775
       %2777 = OpCompositeConstruct %v3float %2770 %2773 %2776
       %2778 = OpFMul %float %2768 %2768
       %2779 = OpCompositeConstruct %v3float %2778 %2768 %float_1
       %2780 = OpMatrixTimesVector %v3float %466 %2777
       %2781 = OpDot %float %2779 %2780
               OpBranch %2728
       %2728 = OpLabel
       %2782 = OpPhi %float %2761 %2736 %2781 %2729
               OpBranch %2722
       %2723 = OpLabel
       %2783 = OpExtInst %float %1 Log %float_9_99999975en05
       %2784 = OpFDiv %float %2783 %1091
               OpBranch %2722
       %2722 = OpLabel
       %2785 = OpPhi %float %2782 %2728 %2784 %2723
       %2786 = OpExtInst %float %1 Pow %float_10 %2785
       %2787 = OpCompositeExtract %float %2583 0
               OpStore %556 %503
               OpStore %555 %504
       %2788 = OpFOrdLessThanEqual %bool %2787 %float_0
       %2789 = OpSelect %float %2788 %float_9_99999975en05 %2787
       %2790 = OpExtInst %float %1 Log %2789
       %2791 = OpFDiv %float %2790 %1091
       %2792 = OpExtInst %float %1 Log %2657
       %2793 = OpFDiv %float %2792 %1091
       %2794 = OpFOrdLessThanEqual %bool %2791 %2793
               OpSelectionMerge %2795 None
               OpBranchConditional %2794 %2796 %2797
       %2797 = OpLabel
       %2798 = OpFOrdGreaterThan %bool %2791 %2793
       %2799 = OpExtInst %float %1 Log %2714
       %2800 = OpFDiv %float %2799 %1091
       %2801 = OpFOrdLessThan %bool %2791 %2800
       %2802 = OpLogicalAnd %bool %2798 %2801
               OpSelectionMerge %2803 None
               OpBranchConditional %2802 %2804 %2805
       %2805 = OpLabel
       %2806 = OpFOrdGreaterThanEqual %bool %2791 %2800
       %2807 = OpExtInst %float %1 Log %2786
       %2808 = OpFDiv %float %2807 %1091
       %2809 = OpFOrdLessThan %bool %2791 %2808
       %2810 = OpLogicalAnd %bool %2806 %2809
               OpSelectionMerge %2811 None
               OpBranchConditional %2810 %2812 %2813
       %2813 = OpLabel
       %2814 = OpFMul %float %2791 %float_0_0599999987
       %2815 = OpExtInst %float %1 Log %float_1000
       %2816 = OpFDiv %float %2815 %1091
       %2817 = OpFMul %float %float_0_0599999987 %2807
       %2818 = OpFDiv %float %2817 %1091
       %2819 = OpFSub %float %2816 %2818
       %2820 = OpFAdd %float %2814 %2819
               OpBranch %2811
       %2812 = OpLabel
       %2821 = OpFSub %float %2791 %2800
       %2822 = OpFMul %float %float_7 %2821
       %2823 = OpFSub %float %2808 %2800
       %2824 = OpFDiv %float %2822 %2823
       %2825 = OpConvertFToS %int %2824
       %2826 = OpConvertSToF %float %2825
       %2827 = OpFSub %float %2824 %2826
       %2828 = OpAccessChain %_ptr_Function_float %555 %2825
       %2829 = OpLoad %float %2828
       %2830 = OpIAdd %int %2825 %int_1
       %2831 = OpAccessChain %_ptr_Function_float %555 %2830
       %2832 = OpLoad %float %2831
       %2833 = OpIAdd %int %2825 %int_2
       %2834 = OpAccessChain %_ptr_Function_float %555 %2833
       %2835 = OpLoad %float %2834
       %2836 = OpCompositeConstruct %v3float %2829 %2832 %2835
       %2837 = OpFMul %float %2827 %2827
       %2838 = OpCompositeConstruct %v3float %2837 %2827 %float_1
       %2839 = OpMatrixTimesVector %v3float %466 %2836
       %2840 = OpDot %float %2838 %2839
               OpBranch %2811
       %2811 = OpLabel
       %2841 = OpPhi %float %2820 %2813 %2840 %2812
               OpBranch %2803
       %2804 = OpLabel
       %2842 = OpFSub %float %2791 %2793
       %2843 = OpFMul %float %float_7 %2842
       %2844 = OpFSub %float %2800 %2793
       %2845 = OpFDiv %float %2843 %2844
       %2846 = OpConvertFToS %int %2845
       %2847 = OpConvertSToF %float %2846
       %2848 = OpFSub %float %2845 %2847
       %2849 = OpAccessChain %_ptr_Function_float %556 %2846
       %2850 = OpLoad %float %2849
       %2851 = OpIAdd %int %2846 %int_1
       %2852 = OpAccessChain %_ptr_Function_float %556 %2851
       %2853 = OpLoad %float %2852
       %2854 = OpIAdd %int %2846 %int_2
       %2855 = OpAccessChain %_ptr_Function_float %556 %2854
       %2856 = OpLoad %float %2855
       %2857 = OpCompositeConstruct %v3float %2850 %2853 %2856
       %2858 = OpFMul %float %2848 %2848
       %2859 = OpCompositeConstruct %v3float %2858 %2848 %float_1
       %2860 = OpMatrixTimesVector %v3float %466 %2857
       %2861 = OpDot %float %2859 %2860
               OpBranch %2803
       %2803 = OpLabel
       %2862 = OpPhi %float %2841 %2811 %2861 %2804
               OpBranch %2795
       %2796 = OpLabel
       %2863 = OpFMul %float %2791 %float_3
       %2864 = OpExtInst %float %1 Log %float_9_99999975en05
       %2865 = OpFDiv %float %2864 %1091
       %2866 = OpFMul %float %float_3 %2792
       %2867 = OpFDiv %float %2866 %1091
       %2868 = OpFSub %float %2865 %2867
       %2869 = OpFAdd %float %2863 %2868
               OpBranch %2795
       %2795 = OpLabel
       %2870 = OpPhi %float %2862 %2803 %2869 %2796
       %2871 = OpExtInst %float %1 Pow %float_10 %2870
       %2872 = OpCompositeInsert %v3float %2871 %523 0
       %2873 = OpCompositeExtract %float %2583 1
               OpStore %554 %503
               OpStore %553 %504
       %2874 = OpFOrdLessThanEqual %bool %2873 %float_0
       %2875 = OpSelect %float %2874 %float_9_99999975en05 %2873
       %2876 = OpExtInst %float %1 Log %2875
       %2877 = OpFDiv %float %2876 %1091
       %2878 = OpFOrdLessThanEqual %bool %2877 %2793
               OpSelectionMerge %2879 None
               OpBranchConditional %2878 %2880 %2881
       %2881 = OpLabel
       %2882 = OpFOrdGreaterThan %bool %2877 %2793
       %2883 = OpExtInst %float %1 Log %2714
       %2884 = OpFDiv %float %2883 %1091
       %2885 = OpFOrdLessThan %bool %2877 %2884
       %2886 = OpLogicalAnd %bool %2882 %2885
               OpSelectionMerge %2887 None
               OpBranchConditional %2886 %2888 %2889
       %2889 = OpLabel
       %2890 = OpFOrdGreaterThanEqual %bool %2877 %2884
       %2891 = OpExtInst %float %1 Log %2786
       %2892 = OpFDiv %float %2891 %1091
       %2893 = OpFOrdLessThan %bool %2877 %2892
       %2894 = OpLogicalAnd %bool %2890 %2893
               OpSelectionMerge %2895 None
               OpBranchConditional %2894 %2896 %2897
       %2897 = OpLabel
       %2898 = OpFMul %float %2877 %float_0_0599999987
       %2899 = OpExtInst %float %1 Log %float_1000
       %2900 = OpFDiv %float %2899 %1091
       %2901 = OpFMul %float %float_0_0599999987 %2891
       %2902 = OpFDiv %float %2901 %1091
       %2903 = OpFSub %float %2900 %2902
       %2904 = OpFAdd %float %2898 %2903
               OpBranch %2895
       %2896 = OpLabel
       %2905 = OpFSub %float %2877 %2884
       %2906 = OpFMul %float %float_7 %2905
       %2907 = OpFSub %float %2892 %2884
       %2908 = OpFDiv %float %2906 %2907
       %2909 = OpConvertFToS %int %2908
       %2910 = OpConvertSToF %float %2909
       %2911 = OpFSub %float %2908 %2910
       %2912 = OpAccessChain %_ptr_Function_float %553 %2909
       %2913 = OpLoad %float %2912
       %2914 = OpIAdd %int %2909 %int_1
       %2915 = OpAccessChain %_ptr_Function_float %553 %2914
       %2916 = OpLoad %float %2915
       %2917 = OpIAdd %int %2909 %int_2
       %2918 = OpAccessChain %_ptr_Function_float %553 %2917
       %2919 = OpLoad %float %2918
       %2920 = OpCompositeConstruct %v3float %2913 %2916 %2919
       %2921 = OpFMul %float %2911 %2911
       %2922 = OpCompositeConstruct %v3float %2921 %2911 %float_1
       %2923 = OpMatrixTimesVector %v3float %466 %2920
       %2924 = OpDot %float %2922 %2923
               OpBranch %2895
       %2895 = OpLabel
       %2925 = OpPhi %float %2904 %2897 %2924 %2896
               OpBranch %2887
       %2888 = OpLabel
       %2926 = OpFSub %float %2877 %2793
       %2927 = OpFMul %float %float_7 %2926
       %2928 = OpFSub %float %2884 %2793
       %2929 = OpFDiv %float %2927 %2928
       %2930 = OpConvertFToS %int %2929
       %2931 = OpConvertSToF %float %2930
       %2932 = OpFSub %float %2929 %2931
       %2933 = OpAccessChain %_ptr_Function_float %554 %2930
       %2934 = OpLoad %float %2933
       %2935 = OpIAdd %int %2930 %int_1
       %2936 = OpAccessChain %_ptr_Function_float %554 %2935
       %2937 = OpLoad %float %2936
       %2938 = OpIAdd %int %2930 %int_2
       %2939 = OpAccessChain %_ptr_Function_float %554 %2938
       %2940 = OpLoad %float %2939
       %2941 = OpCompositeConstruct %v3float %2934 %2937 %2940
       %2942 = OpFMul %float %2932 %2932
       %2943 = OpCompositeConstruct %v3float %2942 %2932 %float_1
       %2944 = OpMatrixTimesVector %v3float %466 %2941
       %2945 = OpDot %float %2943 %2944
               OpBranch %2887
       %2887 = OpLabel
       %2946 = OpPhi %float %2925 %2895 %2945 %2888
               OpBranch %2879
       %2880 = OpLabel
       %2947 = OpFMul %float %2877 %float_3
       %2948 = OpExtInst %float %1 Log %float_9_99999975en05
       %2949 = OpFDiv %float %2948 %1091
       %2950 = OpFMul %float %float_3 %2792
       %2951 = OpFDiv %float %2950 %1091
       %2952 = OpFSub %float %2949 %2951
       %2953 = OpFAdd %float %2947 %2952
               OpBranch %2879
       %2879 = OpLabel
       %2954 = OpPhi %float %2946 %2887 %2953 %2880
       %2955 = OpExtInst %float %1 Pow %float_10 %2954
       %2956 = OpCompositeInsert %v3float %2955 %2872 1
       %2957 = OpCompositeExtract %float %2583 2
               OpStore %552 %503
               OpStore %551 %504
       %2958 = OpFOrdLessThanEqual %bool %2957 %float_0
       %2959 = OpSelect %float %2958 %float_9_99999975en05 %2957
       %2960 = OpExtInst %float %1 Log %2959
       %2961 = OpFDiv %float %2960 %1091
       %2962 = OpFOrdLessThanEqual %bool %2961 %2793
               OpSelectionMerge %2963 None
               OpBranchConditional %2962 %2964 %2965
       %2965 = OpLabel
       %2966 = OpFOrdGreaterThan %bool %2961 %2793
       %2967 = OpExtInst %float %1 Log %2714
       %2968 = OpFDiv %float %2967 %1091
       %2969 = OpFOrdLessThan %bool %2961 %2968
       %2970 = OpLogicalAnd %bool %2966 %2969
               OpSelectionMerge %2971 None
               OpBranchConditional %2970 %2972 %2973
       %2973 = OpLabel
       %2974 = OpFOrdGreaterThanEqual %bool %2961 %2968
       %2975 = OpExtInst %float %1 Log %2786
       %2976 = OpFDiv %float %2975 %1091
       %2977 = OpFOrdLessThan %bool %2961 %2976
       %2978 = OpLogicalAnd %bool %2974 %2977
               OpSelectionMerge %2979 None
               OpBranchConditional %2978 %2980 %2981
       %2981 = OpLabel
       %2982 = OpFMul %float %2961 %float_0_0599999987
       %2983 = OpExtInst %float %1 Log %float_1000
       %2984 = OpFDiv %float %2983 %1091
       %2985 = OpFMul %float %float_0_0599999987 %2975
       %2986 = OpFDiv %float %2985 %1091
       %2987 = OpFSub %float %2984 %2986
       %2988 = OpFAdd %float %2982 %2987
               OpBranch %2979
       %2980 = OpLabel
       %2989 = OpFSub %float %2961 %2968
       %2990 = OpFMul %float %float_7 %2989
       %2991 = OpFSub %float %2976 %2968
       %2992 = OpFDiv %float %2990 %2991
       %2993 = OpConvertFToS %int %2992
       %2994 = OpConvertSToF %float %2993
       %2995 = OpFSub %float %2992 %2994
       %2996 = OpAccessChain %_ptr_Function_float %551 %2993
       %2997 = OpLoad %float %2996
       %2998 = OpIAdd %int %2993 %int_1
       %2999 = OpAccessChain %_ptr_Function_float %551 %2998
       %3000 = OpLoad %float %2999
       %3001 = OpIAdd %int %2993 %int_2
       %3002 = OpAccessChain %_ptr_Function_float %551 %3001
       %3003 = OpLoad %float %3002
       %3004 = OpCompositeConstruct %v3float %2997 %3000 %3003
       %3005 = OpFMul %float %2995 %2995
       %3006 = OpCompositeConstruct %v3float %3005 %2995 %float_1
       %3007 = OpMatrixTimesVector %v3float %466 %3004
       %3008 = OpDot %float %3006 %3007
               OpBranch %2979
       %2979 = OpLabel
       %3009 = OpPhi %float %2988 %2981 %3008 %2980
               OpBranch %2971
       %2972 = OpLabel
       %3010 = OpFSub %float %2961 %2793
       %3011 = OpFMul %float %float_7 %3010
       %3012 = OpFSub %float %2968 %2793
       %3013 = OpFDiv %float %3011 %3012
       %3014 = OpConvertFToS %int %3013
       %3015 = OpConvertSToF %float %3014
       %3016 = OpFSub %float %3013 %3015
       %3017 = OpAccessChain %_ptr_Function_float %552 %3014
       %3018 = OpLoad %float %3017
       %3019 = OpIAdd %int %3014 %int_1
       %3020 = OpAccessChain %_ptr_Function_float %552 %3019
       %3021 = OpLoad %float %3020
       %3022 = OpIAdd %int %3014 %int_2
       %3023 = OpAccessChain %_ptr_Function_float %552 %3022
       %3024 = OpLoad %float %3023
       %3025 = OpCompositeConstruct %v3float %3018 %3021 %3024
       %3026 = OpFMul %float %3016 %3016
       %3027 = OpCompositeConstruct %v3float %3026 %3016 %float_1
       %3028 = OpMatrixTimesVector %v3float %466 %3025
       %3029 = OpDot %float %3027 %3028
               OpBranch %2971
       %2971 = OpLabel
       %3030 = OpPhi %float %3009 %2979 %3029 %2972
               OpBranch %2963
       %2964 = OpLabel
       %3031 = OpFMul %float %2961 %float_3
       %3032 = OpExtInst %float %1 Log %float_9_99999975en05
       %3033 = OpFDiv %float %3032 %1091
       %3034 = OpFMul %float %float_3 %2792
       %3035 = OpFDiv %float %3034 %1091
       %3036 = OpFSub %float %3033 %3035
       %3037 = OpFAdd %float %3031 %3036
               OpBranch %2963
       %2963 = OpLabel
       %3038 = OpPhi %float %3030 %2971 %3037 %2964
       %3039 = OpExtInst %float %1 Pow %float_10 %3038
       %3040 = OpCompositeInsert %v3float %3039 %2956 2
       %3041 = OpFSub %v3float %3040 %361
       %3042 = OpVectorTimesMatrix %v3float %3041 %602
       %3043 = OpFMul %v3float %3042 %519
       %3044 = OpExtInst %v3float %1 Pow %3043 %286
       %3045 = OpFMul %v3float %196 %3044
       %3046 = OpFAdd %v3float %195 %3045
       %3047 = OpFMul %v3float %197 %3044
       %3048 = OpFAdd %v3float %141 %3047
       %3049 = OpFDiv %v3float %141 %3048
       %3050 = OpFMul %v3float %3046 %3049
       %3051 = OpExtInst %v3float %1 Pow %3050 %287
               OpBranch %1336
       %1336 = OpLabel
       %3052 = OpPhi %v3float %2201 %1342 %3051 %2963
               OpBranch %1330
       %1331 = OpLabel
       %3053 = OpVectorTimesMatrix %v3float %1324 %573
       %3054 = OpVectorTimesMatrix %v3float %3053 %602
       %3055 = OpExtInst %v3float %1 FMax %263 %3054
       %3056 = OpFMul %v3float %3055 %275
       %3057 = OpExtInst %v3float %1 FMax %3055 %277
       %3058 = OpExtInst %v3float %1 Pow %3057 %279
       %3059 = OpFMul %v3float %3058 %281
       %3060 = OpFSub %v3float %3059 %283
       %3061 = OpExtInst %v3float %1 FMin %3056 %3060
               OpBranch %1330
       %1330 = OpLabel
       %3062 = OpPhi %v3float %3052 %1336 %3061 %1331
               OpBranch %1326
       %1327 = OpLabel
       %3063 = OpCompositeExtract %float %1324 0
               OpBranch %3064
       %3064 = OpLabel
               OpLoopMerge %3065 %3066 None
               OpBranch %3067
       %3067 = OpLabel
       %3068 = OpFOrdLessThan %bool %3063 %float_0_00313066994
               OpSelectionMerge %3069 None
               OpBranchConditional %3068 %3070 %3069
       %3070 = OpLabel
       %3071 = OpFMul %float %3063 %float_12_9200001
               OpBranch %3065
       %3069 = OpLabel
       %3072 = OpExtInst %float %1 Pow %3063 %float_0_416666657
       %3073 = OpFMul %float %3072 %float_1_05499995
       %3074 = OpFSub %float %3073 %float_0_0549999997
               OpBranch %3065
       %3066 = OpLabel
               OpBranch %3064
       %3065 = OpLabel
       %3075 = OpPhi %float %3071 %3070 %3074 %3069
       %3076 = OpCompositeExtract %float %1324 1
               OpBranch %3077
       %3077 = OpLabel
               OpLoopMerge %3078 %3079 None
               OpBranch %3080
       %3080 = OpLabel
       %3081 = OpFOrdLessThan %bool %3076 %float_0_00313066994
               OpSelectionMerge %3082 None
               OpBranchConditional %3081 %3083 %3082
       %3083 = OpLabel
       %3084 = OpFMul %float %3076 %float_12_9200001
               OpBranch %3078
       %3082 = OpLabel
       %3085 = OpExtInst %float %1 Pow %3076 %float_0_416666657
       %3086 = OpFMul %float %3085 %float_1_05499995
       %3087 = OpFSub %float %3086 %float_0_0549999997
               OpBranch %3078
       %3079 = OpLabel
               OpBranch %3077
       %3078 = OpLabel
       %3088 = OpPhi %float %3084 %3083 %3087 %3082
       %3089 = OpCompositeExtract %float %1324 2
               OpBranch %3090
       %3090 = OpLabel
               OpLoopMerge %3091 %3092 None
               OpBranch %3093
       %3093 = OpLabel
       %3094 = OpFOrdLessThan %bool %3089 %float_0_00313066994
               OpSelectionMerge %3095 None
               OpBranchConditional %3094 %3096 %3095
       %3096 = OpLabel
       %3097 = OpFMul %float %3089 %float_12_9200001
               OpBranch %3091
       %3095 = OpLabel
       %3098 = OpExtInst %float %1 Pow %3089 %float_0_416666657
       %3099 = OpFMul %float %3098 %float_1_05499995
       %3100 = OpFSub %float %3099 %float_0_0549999997
               OpBranch %3091
       %3092 = OpLabel
               OpBranch %3090
       %3091 = OpLabel
       %3101 = OpPhi %float %3097 %3096 %3100 %3095
       %3102 = OpCompositeConstruct %v3float %3075 %3088 %3101
               OpBranch %1326
       %1326 = OpLabel
       %3103 = OpPhi %v3float %3062 %1330 %3102 %3091
       %3104 = OpFMul %v3float %3103 %522
       %3105 = OpVectorShuffle %v4float %135 %3104 4 5 6 3
       %3106 = OpCompositeInsert %v4float %float_0 %3105 3
               OpStore %out_var_SV_Target0 %3106
               OpReturn
               OpFunctionEnd
