; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 3005
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
               OpMemberName %type__Globals 15 "ColorScale"
               OpMemberName %type__Globals 16 "OverlayColor"
               OpMemberName %type__Globals 17 "WhiteTemp"
               OpMemberName %type__Globals 18 "WhiteTint"
               OpMemberName %type__Globals 19 "ColorSaturation"
               OpMemberName %type__Globals 20 "ColorContrast"
               OpMemberName %type__Globals 21 "ColorGamma"
               OpMemberName %type__Globals 22 "ColorGain"
               OpMemberName %type__Globals 23 "ColorOffset"
               OpMemberName %type__Globals 24 "ColorSaturationShadows"
               OpMemberName %type__Globals 25 "ColorContrastShadows"
               OpMemberName %type__Globals 26 "ColorGammaShadows"
               OpMemberName %type__Globals 27 "ColorGainShadows"
               OpMemberName %type__Globals 28 "ColorOffsetShadows"
               OpMemberName %type__Globals 29 "ColorSaturationMidtones"
               OpMemberName %type__Globals 30 "ColorContrastMidtones"
               OpMemberName %type__Globals 31 "ColorGammaMidtones"
               OpMemberName %type__Globals 32 "ColorGainMidtones"
               OpMemberName %type__Globals 33 "ColorOffsetMidtones"
               OpMemberName %type__Globals 34 "ColorSaturationHighlights"
               OpMemberName %type__Globals 35 "ColorContrastHighlights"
               OpMemberName %type__Globals 36 "ColorGammaHighlights"
               OpMemberName %type__Globals 37 "ColorGainHighlights"
               OpMemberName %type__Globals 38 "ColorOffsetHighlights"
               OpMemberName %type__Globals 39 "ColorCorrectionShadowsMax"
               OpMemberName %type__Globals 40 "ColorCorrectionHighlightsMin"
               OpMemberName %type__Globals 41 "OutputDevice"
               OpMemberName %type__Globals 42 "OutputGamut"
               OpMemberName %type__Globals 43 "BlueCorrection"
               OpMemberName %type__Globals 44 "ExpandGamut"
               OpName %_Globals "$Globals"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPS "MainPS"
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
               OpMemberDecorate %type__Globals 15 Offset 180
               OpMemberDecorate %type__Globals 16 Offset 192
               OpMemberDecorate %type__Globals 17 Offset 208
               OpMemberDecorate %type__Globals 18 Offset 212
               OpMemberDecorate %type__Globals 19 Offset 224
               OpMemberDecorate %type__Globals 20 Offset 240
               OpMemberDecorate %type__Globals 21 Offset 256
               OpMemberDecorate %type__Globals 22 Offset 272
               OpMemberDecorate %type__Globals 23 Offset 288
               OpMemberDecorate %type__Globals 24 Offset 304
               OpMemberDecorate %type__Globals 25 Offset 320
               OpMemberDecorate %type__Globals 26 Offset 336
               OpMemberDecorate %type__Globals 27 Offset 352
               OpMemberDecorate %type__Globals 28 Offset 368
               OpMemberDecorate %type__Globals 29 Offset 384
               OpMemberDecorate %type__Globals 30 Offset 400
               OpMemberDecorate %type__Globals 31 Offset 416
               OpMemberDecorate %type__Globals 32 Offset 432
               OpMemberDecorate %type__Globals 33 Offset 448
               OpMemberDecorate %type__Globals 34 Offset 464
               OpMemberDecorate %type__Globals 35 Offset 480
               OpMemberDecorate %type__Globals 36 Offset 496
               OpMemberDecorate %type__Globals 37 Offset 512
               OpMemberDecorate %type__Globals 38 Offset 528
               OpMemberDecorate %type__Globals 39 Offset 544
               OpMemberDecorate %type__Globals 40 Offset 548
               OpMemberDecorate %type__Globals 41 Offset 552
               OpMemberDecorate %type__Globals 42 Offset 556
               OpMemberDecorate %type__Globals 43 Offset 560
               OpMemberDecorate %type__Globals 44 Offset 564
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
         %67 = OpConstantComposite %v3float %float_0_272228718 %float_0_674081743 %float_0_0536895171
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
        %128 = OpConstantComposite %v2float %float_0_015625 %float_0_015625
        %129 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
     %int_42 = OpConstant %int 42
     %uint_3 = OpConstant %uint 3
        %132 = OpConstantComposite %v3float %float_0 %float_0 %float_0
      %int_9 = OpConstant %int 9
      %int_3 = OpConstant %int 3
        %135 = OpConstantComposite %v3float %float_1 %float_1 %float_1
   %float_n4 = OpConstant %float -4
     %int_44 = OpConstant %int 44
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
     %int_43 = OpConstant %int 43
     %int_15 = OpConstant %int 15
     %int_16 = OpConstant %int 16
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_5 = OpConstant %uint 5
     %uint_6 = OpConstant %uint 6
      %int_2 = OpConstant %int 2
%mat3v3float = OpTypeMatrix %v3float 3
     %int_41 = OpConstant %int 41
%float_0_159301758 = OpConstant %float 0.159301758
%float_78_84375 = OpConstant %float 78.84375
%float_0_8359375 = OpConstant %float 0.8359375
%float_18_8515625 = OpConstant %float 18.8515625
%float_18_6875 = OpConstant %float 18.6875
%float_10000 = OpConstant %float 10000
%float_0_0126833133 = OpConstant %float 0.0126833133
        %182 = OpConstantComposite %v3float %float_0_0126833133 %float_0_0126833133 %float_0_0126833133
        %183 = OpConstantComposite %v3float %float_0_8359375 %float_0_8359375 %float_0_8359375
        %184 = OpConstantComposite %v3float %float_18_8515625 %float_18_8515625 %float_18_8515625
        %185 = OpConstantComposite %v3float %float_18_6875 %float_18_6875 %float_18_6875
%float_6_27739477 = OpConstant %float 6.27739477
        %187 = OpConstantComposite %v3float %float_6_27739477 %float_6_27739477 %float_6_27739477
        %188 = OpConstantComposite %v3float %float_10000 %float_10000 %float_10000
   %float_14 = OpConstant %float 14
%float_0_180000007 = OpConstant %float 0.180000007
%float_0_434017599 = OpConstant %float 0.434017599
        %192 = OpConstantComposite %v3float %float_0_434017599 %float_0_434017599 %float_0_434017599
        %193 = OpConstantComposite %v3float %float_14 %float_14 %float_14
        %194 = OpConstantComposite %v3float %float_0_180000007 %float_0_180000007 %float_0_180000007
     %int_17 = OpConstant %int 17
 %float_4000 = OpConstant %float 4000
%float_0_312700003 = OpConstant %float 0.312700003
%float_0_328999996 = OpConstant %float 0.328999996
     %int_18 = OpConstant %int 18
     %int_24 = OpConstant %int 24
     %int_19 = OpConstant %int 19
     %int_25 = OpConstant %int 25
     %int_20 = OpConstant %int 20
     %int_26 = OpConstant %int 26
     %int_21 = OpConstant %int 21
     %int_27 = OpConstant %int 27
     %int_22 = OpConstant %int 22
     %int_28 = OpConstant %int 28
     %int_23 = OpConstant %int 23
     %int_39 = OpConstant %int 39
     %int_34 = OpConstant %int 34
     %int_35 = OpConstant %int 35
     %int_36 = OpConstant %int 36
     %int_37 = OpConstant %int 37
     %int_38 = OpConstant %int 38
     %int_40 = OpConstant %int 40
     %int_29 = OpConstant %int 29
     %int_30 = OpConstant %int 30
     %int_31 = OpConstant %int 31
     %int_32 = OpConstant %int 32
     %int_33 = OpConstant %int 33
%float_0_0500000007 = OpConstant %float 0.0500000007
 %float_1_75 = OpConstant %float 1.75
%float_0_400000006 = OpConstant %float 0.400000006
%float_0_0299999993 = OpConstant %float 0.0299999993
    %float_2 = OpConstant %float 2
%float_0_959999979 = OpConstant %float 0.959999979
        %228 = OpConstantComposite %v3float %float_0_959999979 %float_0_959999979 %float_0_959999979
     %int_13 = OpConstant %int 13
     %int_11 = OpConstant %int 11
     %int_14 = OpConstant %int 14
     %int_12 = OpConstant %int 12
%float_0_800000012 = OpConstant %float 0.800000012
     %int_10 = OpConstant %int 10
   %float_10 = OpConstant %float 10
   %float_n2 = OpConstant %float -2
    %float_3 = OpConstant %float 3
        %238 = OpConstantComposite %v3float %float_3 %float_3 %float_3
        %239 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%float_0_930000007 = OpConstant %float 0.930000007
        %241 = OpConstantComposite %v3float %float_0_930000007 %float_0_930000007 %float_0_930000007
      %int_4 = OpConstant %int 4
      %int_8 = OpConstant %int 8
      %int_7 = OpConstant %int 7
      %int_5 = OpConstant %int 5
      %int_6 = OpConstant %int 6
%float_0_00200000009 = OpConstant %float 0.00200000009
        %248 = OpConstantComposite %v3float %float_0_00200000009 %float_0_00200000009 %float_0_00200000009
%float_6_10351999en05 = OpConstant %float 6.10351999e-05
        %250 = OpConstantComposite %v3float %float_6_10351999en05 %float_6_10351999en05 %float_6_10351999en05
  %float_4_5 = OpConstant %float 4.5
        %252 = OpConstantComposite %v3float %float_4_5 %float_4_5 %float_4_5
%float_0_0179999992 = OpConstant %float 0.0179999992
        %254 = OpConstantComposite %v3float %float_0_0179999992 %float_0_0179999992 %float_0_0179999992
%float_0_449999988 = OpConstant %float 0.449999988
        %256 = OpConstantComposite %v3float %float_0_449999988 %float_0_449999988 %float_0_449999988
%float_1_09899998 = OpConstant %float 1.09899998
        %258 = OpConstantComposite %v3float %float_1_09899998 %float_1_09899998 %float_1_09899998
%float_0_0989999995 = OpConstant %float 0.0989999995
        %260 = OpConstantComposite %v3float %float_0_0989999995 %float_0_0989999995 %float_0_0989999995
  %float_1_5 = OpConstant %float 1.5
        %262 = OpConstantComposite %v3float %float_1_5 %float_1_5 %float_1_5
        %263 = OpConstantComposite %v3float %float_0_159301758 %float_0_159301758 %float_0_159301758
        %264 = OpConstantComposite %v3float %float_78_84375 %float_78_84375 %float_78_84375
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
        %307 = OpConstantComposite %v3float %float_5_55555534 %float_5_55555534 %float_5_55555534
%float_1_00000001en10 = OpConstant %float 1.00000001e-10
%float_0_00999999978 = OpConstant %float 0.00999999978
%float_0_666666687 = OpConstant %float 0.666666687
  %float_180 = OpConstant %float 180
  %float_360 = OpConstant %float 360
%float_65535 = OpConstant %float 65535
        %314 = OpConstantComposite %v3float %float_65535 %float_65535 %float_65535
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
        %338 = OpConstantComposite %v3float %float_3_50738446en05 %float_3_50738446en05 %float_3_50738446en05
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
%type__Globals = OpTypeStruct %v4float %v3float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %float %float %float %float %float %v3float %v4float %float %float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %float %float %uint %uint %float %float
%_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
        %377 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
       %bool = OpTypeBool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
    %uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%_arr_float_uint_6 = OpTypeArray %float %uint_6
   %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v2float Input
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
   %gl_Layer = OpVariable %_ptr_Input_uint Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function__arr_float_uint_6 = OpTypePointer Function %_arr_float_uint_6
%_ptr_Function__arr_float_uint_10 = OpTypePointer Function %_arr_float_uint_10
        %391 = OpUndef %v3float
        %392 = OpConstantComposite %v3float %float_0_952552378 %float_0 %float_9_36786018en05
        %393 = OpConstantComposite %v3float %float_0_343966454 %float_0_728166103 %float_n0_0721325427
        %394 = OpConstantComposite %v3float %float_0 %float_0 %float_1_00882518
        %395 = OpConstantComposite %mat3v3float %392 %393 %394
        %396 = OpConstantComposite %v3float %float_1_04981101 %float_0 %float_n9_74845025en05
        %397 = OpConstantComposite %v3float %float_n0_495903015 %float_1_37331307 %float_0_0982400328
        %398 = OpConstantComposite %v3float %float_0 %float_0 %float_0_991252005
        %399 = OpConstantComposite %mat3v3float %396 %397 %398
        %400 = OpConstantComposite %v3float %float_0_662454188 %float_0_134004205 %float_0_156187683
        %401 = OpConstantComposite %v3float %float_n0_00557464967 %float_0_0040607336 %float_1_01033914
        %402 = OpConstantComposite %mat3v3float %400 %67 %401
        %403 = OpConstantComposite %v3float %float_1_6410234 %float_n0_324803293 %float_n0_236424699
        %404 = OpConstantComposite %v3float %float_n0_663662851 %float_1_61533165 %float_0_0167563483
        %405 = OpConstantComposite %v3float %float_0_0117218941 %float_n0_00828444213 %float_0_988394856
        %406 = OpConstantComposite %mat3v3float %403 %404 %405
        %407 = OpConstantComposite %v3float %float_1_45143926 %float_n0_236510754 %float_n0_214928567
        %408 = OpConstantComposite %v3float %float_n0_0765537769 %float_1_17622972 %float_n0_0996759236
        %409 = OpConstantComposite %v3float %float_0_00831614807 %float_n0_00603244966 %float_0_997716308
        %410 = OpConstantComposite %mat3v3float %407 %408 %409
        %411 = OpConstantComposite %v3float %float_0_695452213 %float_0_140678704 %float_0_163869068
        %412 = OpConstantComposite %v3float %float_0_0447945632 %float_0_859671116 %float_0_0955343172
        %413 = OpConstantComposite %v3float %float_n0_00552588282 %float_0_00402521016 %float_1_00150073
        %414 = OpConstantComposite %mat3v3float %411 %412 %413
        %415 = OpConstantComposite %v3float %float_3_2409699 %float_n1_5373832 %float_n0_498610765
        %416 = OpConstantComposite %v3float %float_n0_969243646 %float_1_8759675 %float_0_0415550582
        %417 = OpConstantComposite %v3float %float_0_0556300804 %float_n0_203976959 %float_1_05697155
        %418 = OpConstantComposite %mat3v3float %415 %416 %417
        %419 = OpConstantComposite %v3float %float_0_412456393 %float_0_357576102 %float_0_180437505
        %420 = OpConstantComposite %v3float %float_0_212672904 %float_0_715152204 %float_0_0721750036
        %421 = OpConstantComposite %v3float %float_0_0193339009 %float_0_119191997 %float_0_950304091
        %422 = OpConstantComposite %mat3v3float %419 %420 %421
        %423 = OpConstantComposite %v3float %float_1_71660841 %float_n0_355662107 %float_n0_253360093
        %424 = OpConstantComposite %v3float %float_n0_666682899 %float_1_61647761 %float_0_0157685
        %425 = OpConstantComposite %v3float %float_0_0176422 %float_n0_0427763015 %float_0_942228675
        %426 = OpConstantComposite %mat3v3float %423 %424 %425
        %427 = OpConstantComposite %v3float %float_2_49339628 %float_n0_93134588 %float_n0_402694494
        %428 = OpConstantComposite %v3float %float_n0_829486787 %float_1_76265967 %float_0_0236246008
        %429 = OpConstantComposite %v3float %float_0_0358507 %float_n0_0761827007 %float_0_957014024
        %430 = OpConstantComposite %mat3v3float %427 %428 %429
        %431 = OpConstantComposite %v3float %float_1_01303005 %float_0_00610530982 %float_n0_0149710001
        %432 = OpConstantComposite %v3float %float_0_00769822998 %float_0_998165011 %float_n0_00503202993
        %433 = OpConstantComposite %v3float %float_n0_00284131011 %float_0_00468515977 %float_0_924507022
        %434 = OpConstantComposite %mat3v3float %431 %432 %433
        %435 = OpConstantComposite %v3float %float_0_987223983 %float_n0_00611326983 %float_0_0159533005
        %436 = OpConstantComposite %v3float %float_n0_00759836007 %float_1_00186002 %float_0_0053300201
        %437 = OpConstantComposite %v3float %float_0_00307257008 %float_n0_00509594986 %float_1_08168006
        %438 = OpConstantComposite %mat3v3float %435 %436 %437
        %439 = OpConstantComposite %v3float %float_0_5 %float_n1 %float_0_5
        %440 = OpConstantComposite %v3float %float_n1 %float_1 %float_0_5
        %441 = OpConstantComposite %v3float %float_0_5 %float_0 %float_0
        %442 = OpConstantComposite %mat3v3float %439 %440 %441
        %443 = OpConstantComposite %v3float %float_1 %float_0 %float_0
        %444 = OpConstantComposite %v3float %float_0 %float_1 %float_0
        %445 = OpConstantComposite %v3float %float_0 %float_0 %float_1
        %446 = OpConstantComposite %mat3v3float %443 %444 %445
%float_n6_07624626 = OpConstant %float -6.07624626
        %448 = OpConstantComposite %v3float %float_n6_07624626 %float_n6_07624626 %float_n6_07624626
        %449 = OpConstantComposite %v3float %float_0_895099998 %float_0_266400009 %float_n0_161400005
        %450 = OpConstantComposite %v3float %float_n0_750199974 %float_1_71350002 %float_0_0366999991
        %451 = OpConstantComposite %v3float %float_0_0388999991 %float_n0_0684999973 %float_1_02960002
        %452 = OpConstantComposite %mat3v3float %449 %450 %451
        %453 = OpConstantComposite %v3float %float_0_986992896 %float_n0_1470543 %float_0_159962699
        %454 = OpConstantComposite %v3float %float_0_432305306 %float_0_518360317 %float_0_0492912009
        %455 = OpConstantComposite %v3float %float_n0_0085287001 %float_0_040042799 %float_0_968486726
        %456 = OpConstantComposite %mat3v3float %453 %454 %455
%float_0_358299971 = OpConstant %float 0.358299971
        %458 = OpConstantComposite %v3float %float_0_544169128 %float_0_239592597 %float_0_166694298
        %459 = OpConstantComposite %v3float %float_0_239465594 %float_0_702153027 %float_0_058381401
        %460 = OpConstantComposite %v3float %float_n0_00234390004 %float_0_0361833982 %float_1_05521834
        %461 = OpConstantComposite %mat3v3float %458 %459 %460
        %462 = OpConstantComposite %v3float %float_0_940437257 %float_n0_0183068793 %float_0_077869609
        %463 = OpConstantComposite %v3float %float_0_00837869663 %float_0_828660011 %float_0_162961304
        %464 = OpConstantComposite %v3float %float_0_00054712611 %float_n0_000883374596 %float_1_00033629
        %465 = OpConstantComposite %mat3v3float %462 %463 %464
        %466 = OpConstantComposite %v3float %float_1_06317997 %float_0_0233955998 %float_n0_0865726024
        %467 = OpConstantComposite %v3float %float_n0_0106336996 %float_1_20632005 %float_n0_195690006
        %468 = OpConstantComposite %v3float %float_n0_000590886979 %float_0_00105247996 %float_0_999538004
        %469 = OpConstantComposite %mat3v3float %466 %467 %468
%float_0_0533333346 = OpConstant %float 0.0533333346
%float_0_159999996 = OpConstant %float 0.159999996
%float_57_2957764 = OpConstant %float 57.2957764
%float_n67_5 = OpConstant %float -67.5
 %float_67_5 = OpConstant %float 67.5
        %475 = OpConstantComposite %_arr_float_uint_6 %float_n4 %float_n4 %float_n3_15737653 %float_n0_485249996 %float_1_84773242 %float_1_84773242
        %476 = OpConstantComposite %_arr_float_uint_6 %float_n0_718548238 %float_2_08103061 %float_3_6681242 %float_4 %float_4 %float_4
  %float_n15 = OpConstant %float -15
  %float_n14 = OpConstant %float -14
        %479 = OpConstantComposite %_arr_float_uint_10 %float_n4_97062206 %float_n3_02937818 %float_n2_12619996 %float_n1_51049995 %float_n1_05780005 %float_n0_466800004 %float_0_119379997 %float_0_708813429 %float_1_29118657 %float_1_29118657
        %480 = OpConstantComposite %_arr_float_uint_10 %float_0_808913231 %float_1_19108677 %float_1_56830001 %float_1_9483 %float_2_30830002 %float_2_63840008 %float_2_85949993 %float_2_98726082 %float_3_01273918 %float_3_01273918
  %float_n12 = OpConstant %float -12
        %482 = OpConstantComposite %_arr_float_uint_10 %float_n2_30102992 %float_n2_30102992 %float_n1_93120003 %float_n1_52049994 %float_n1_05780005 %float_n0_466800004 %float_0_119379997 %float_0_708813429 %float_1_29118657 %float_1_29118657
        %483 = OpConstantComposite %_arr_float_uint_10 %float_0_801995218 %float_1_19800484 %float_1_59430003 %float_1_99730003 %float_2_37829995 %float_2_76839995 %float_3_05150008 %float_3_27462935 %float_3_32743073 %float_3_32743073
%float_0_0322580636 = OpConstant %float 0.0322580636
%float_1_03225803 = OpConstant %float 1.03225803
        %486 = OpConstantComposite %v2float %float_1_03225803 %float_1_03225803
%float_4_60443853e_09 = OpConstant %float 4.60443853e+09
%float_2_00528435e_09 = OpConstant %float 2.00528435e+09
%float_0_333333343 = OpConstant %float 0.333333343
    %float_5 = OpConstant %float 5
  %float_2_5 = OpConstant %float 2.5
%float_0_0250000004 = OpConstant %float 0.0250000004
%float_0_239999995 = OpConstant %float 0.239999995
%float_0_0148148146 = OpConstant %float 0.0148148146
%float_0_819999993 = OpConstant %float 0.819999993
        %496 = OpConstantComposite %v3float %float_9_99999975en05 %float_9_99999975en05 %float_9_99999975en05
%float_0_0296296291 = OpConstant %float 0.0296296291
%float_0_952381015 = OpConstant %float 0.952381015
        %499 = OpConstantComposite %v3float %float_0_952381015 %float_0_952381015 %float_0_952381015
     %MainPS = OpFunction %void None %377
        %500 = OpLabel
        %501 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %502 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %503 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %504 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %505 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %506 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %507 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %508 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %509 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %510 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %511 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %512 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %513 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %514 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %515 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %516 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %517 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %518 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %519 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %520 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %521 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %522 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %523 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %524 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %525 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %526 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %527 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %528 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %529 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %530 = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %531 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %532 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %533 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %534 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %535 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %536 = OpVariable %_ptr_Function__arr_float_uint_6 Function
        %537 = OpLoad %v2float %in_var_TEXCOORD0
        %538 = OpLoad %uint %gl_Layer
        %539 = OpFSub %v2float %537 %128
        %540 = OpFMul %v2float %539 %486
        %541 = OpCompositeExtract %float %540 0
        %542 = OpCompositeExtract %float %540 1
        %543 = OpConvertUToF %float %538
        %544 = OpFMul %float %543 %float_0_0322580636
        %545 = OpCompositeConstruct %v4float %541 %542 %544 %float_0
        %546 = OpMatrixTimesMatrix %mat3v3float %422 %434
        %547 = OpMatrixTimesMatrix %mat3v3float %546 %406
        %548 = OpMatrixTimesMatrix %mat3v3float %402 %438
        %549 = OpMatrixTimesMatrix %mat3v3float %548 %418
        %550 = OpMatrixTimesMatrix %mat3v3float %395 %406
        %551 = OpMatrixTimesMatrix %mat3v3float %402 %399
        %552 = OpAccessChain %_ptr_Uniform_uint %_Globals %int_42
        %553 = OpLoad %uint %552
               OpBranch %554
        %554 = OpLabel
               OpLoopMerge %555 %556 None
               OpBranch %557
        %557 = OpLabel
        %558 = OpMatrixTimesMatrix %mat3v3float %548 %430
        %559 = OpMatrixTimesMatrix %mat3v3float %548 %426
        %560 = OpIEqual %bool %553 %uint_1
               OpSelectionMerge %561 None
               OpBranchConditional %560 %562 %563
        %563 = OpLabel
        %564 = OpIEqual %bool %553 %uint_2
               OpSelectionMerge %565 None
               OpBranchConditional %564 %566 %567
        %567 = OpLabel
        %568 = OpIEqual %bool %553 %uint_3
               OpSelectionMerge %569 None
               OpBranchConditional %568 %570 %571
        %571 = OpLabel
        %572 = OpIEqual %bool %553 %uint_4
               OpSelectionMerge %573 None
               OpBranchConditional %572 %574 %575
        %575 = OpLabel
               OpBranch %555
        %574 = OpLabel
               OpBranch %555
        %573 = OpLabel
               OpUnreachable
        %570 = OpLabel
               OpBranch %555
        %569 = OpLabel
               OpUnreachable
        %566 = OpLabel
               OpBranch %555
        %565 = OpLabel
               OpUnreachable
        %562 = OpLabel
               OpBranch %555
        %561 = OpLabel
               OpUnreachable
        %556 = OpLabel
               OpBranch %554
        %555 = OpLabel
        %576 = OpPhi %mat3v3float %549 %575 %446 %574 %414 %570 %559 %566 %558 %562
        %577 = OpVectorShuffle %v3float %545 %545 0 1 2
        %578 = OpAccessChain %_ptr_Uniform_uint %_Globals %int_41
        %579 = OpLoad %uint %578
        %580 = OpUGreaterThanEqual %bool %579 %uint_3
               OpSelectionMerge %581 None
               OpBranchConditional %580 %582 %583
        %583 = OpLabel
        %584 = OpFSub %v3float %577 %192
        %585 = OpFMul %v3float %584 %193
        %586 = OpExtInst %v3float %1 Exp2 %585
        %587 = OpFMul %v3float %586 %194
        %588 = OpExtInst %v3float %1 Exp2 %448
        %589 = OpFMul %v3float %588 %194
        %590 = OpFSub %v3float %587 %589
               OpBranch %581
        %582 = OpLabel
        %591 = OpExtInst %v3float %1 Pow %577 %182
        %592 = OpFSub %v3float %591 %183
        %593 = OpExtInst %v3float %1 FMax %132 %592
        %594 = OpFMul %v3float %185 %591
        %595 = OpFSub %v3float %184 %594
        %596 = OpFDiv %v3float %593 %595
        %597 = OpExtInst %v3float %1 Pow %596 %187
        %598 = OpFMul %v3float %597 %188
               OpBranch %581
        %581 = OpLabel
        %599 = OpPhi %v3float %590 %583 %598 %582
        %600 = OpAccessChain %_ptr_Uniform_float %_Globals %int_17
        %601 = OpLoad %float %600
        %602 = OpFMul %float %601 %float_1_00055635
        %603 = OpFOrdLessThanEqual %bool %602 %float_7000
        %604 = OpFDiv %float %float_4_60443853e_09 %601
        %605 = OpFSub %float %float_2967800 %604
        %606 = OpFDiv %float %605 %602
        %607 = OpFAdd %float %float_99_1100006 %606
        %608 = OpFDiv %float %607 %602
        %609 = OpFAdd %float %float_0_244063005 %608
        %610 = OpFDiv %float %float_2_00528435e_09 %601
        %611 = OpFSub %float %float_1901800 %610
        %612 = OpFDiv %float %611 %602
        %613 = OpFAdd %float %float_247_479996 %612
        %614 = OpFDiv %float %613 %602
        %615 = OpFAdd %float %float_0_237039998 %614
        %616 = OpSelect %float %603 %609 %615
        %617 = OpFMul %float %float_n3 %616
        %618 = OpFMul %float %617 %616
        %619 = OpFMul %float %float_2_86999989 %616
        %620 = OpFAdd %float %618 %619
        %621 = OpFSub %float %620 %float_0_275000006
        %622 = OpCompositeConstruct %v2float %616 %621
        %623 = OpFMul %float %float_0_000154118257 %601
        %624 = OpFAdd %float %float_0_860117733 %623
        %625 = OpFMul %float %float_1_28641219en07 %601
        %626 = OpFMul %float %625 %601
        %627 = OpFAdd %float %624 %626
        %628 = OpFMul %float %float_0_00084242021 %601
        %629 = OpFAdd %float %float_1 %628
        %630 = OpFMul %float %float_7_08145137en07 %601
        %631 = OpFMul %float %630 %601
        %632 = OpFAdd %float %629 %631
        %633 = OpFDiv %float %627 %632
        %634 = OpFMul %float %float_4_22806261en05 %601
        %635 = OpFAdd %float %float_0_317398727 %634
        %636 = OpFMul %float %float_4_20481676en08 %601
        %637 = OpFMul %float %636 %601
        %638 = OpFAdd %float %635 %637
        %639 = OpFMul %float %float_2_8974182en05 %601
        %640 = OpFSub %float %float_1 %639
        %641 = OpFMul %float %float_1_61456057en07 %601
        %642 = OpFMul %float %641 %601
        %643 = OpFAdd %float %640 %642
        %644 = OpFDiv %float %638 %643
        %645 = OpFMul %float %float_3 %633
        %646 = OpFMul %float %float_2 %633
        %647 = OpFMul %float %float_8 %644
        %648 = OpFSub %float %646 %647
        %649 = OpFAdd %float %648 %float_4
        %650 = OpFDiv %float %645 %649
        %651 = OpFMul %float %float_2 %644
        %652 = OpFDiv %float %651 %649
        %653 = OpCompositeConstruct %v2float %650 %652
        %654 = OpFOrdLessThan %bool %601 %float_4000
        %655 = OpCompositeConstruct %v2bool %654 %654
        %656 = OpSelect %v2float %655 %653 %622
        %657 = OpAccessChain %_ptr_Uniform_float %_Globals %int_18
        %658 = OpLoad %float %657
        %659 = OpCompositeConstruct %v2float %633 %644
        %660 = OpExtInst %v2float %1 Normalize %659
        %661 = OpCompositeExtract %float %660 1
        %662 = OpFNegate %float %661
        %663 = OpFMul %float %662 %658
        %664 = OpFMul %float %663 %float_0_0500000007
        %665 = OpFAdd %float %633 %664
        %666 = OpCompositeExtract %float %660 0
        %667 = OpFMul %float %666 %658
        %668 = OpFMul %float %667 %float_0_0500000007
        %669 = OpFAdd %float %644 %668
        %670 = OpFMul %float %float_3 %665
        %671 = OpFMul %float %float_2 %665
        %672 = OpFMul %float %float_8 %669
        %673 = OpFSub %float %671 %672
        %674 = OpFAdd %float %673 %float_4
        %675 = OpFDiv %float %670 %674
        %676 = OpFMul %float %float_2 %669
        %677 = OpFDiv %float %676 %674
        %678 = OpCompositeConstruct %v2float %675 %677
        %679 = OpFSub %v2float %678 %653
        %680 = OpFAdd %v2float %656 %679
        %681 = OpCompositeExtract %float %680 0
        %682 = OpCompositeExtract %float %680 1
        %683 = OpExtInst %float %1 FMax %682 %float_1_00000001en10
        %684 = OpFDiv %float %681 %683
        %685 = OpCompositeInsert %v3float %684 %391 0
        %686 = OpCompositeInsert %v3float %float_1 %685 1
        %687 = OpFSub %float %float_1 %681
        %688 = OpFSub %float %687 %682
        %689 = OpFDiv %float %688 %683
        %690 = OpCompositeInsert %v3float %689 %686 2
        %691 = OpExtInst %float %1 FMax %float_0_328999996 %float_1_00000001en10
        %692 = OpFDiv %float %float_0_312700003 %691
        %693 = OpCompositeInsert %v3float %692 %391 0
        %694 = OpCompositeInsert %v3float %float_1 %693 1
        %695 = OpFDiv %float %float_0_358299971 %691
        %696 = OpCompositeInsert %v3float %695 %694 2
        %697 = OpVectorTimesMatrix %v3float %690 %452
        %698 = OpVectorTimesMatrix %v3float %696 %452
        %699 = OpCompositeExtract %float %698 0
        %700 = OpCompositeExtract %float %697 0
        %701 = OpFDiv %float %699 %700
        %702 = OpCompositeConstruct %v3float %701 %float_0 %float_0
        %703 = OpCompositeExtract %float %698 1
        %704 = OpCompositeExtract %float %697 1
        %705 = OpFDiv %float %703 %704
        %706 = OpCompositeConstruct %v3float %float_0 %705 %float_0
        %707 = OpCompositeExtract %float %698 2
        %708 = OpCompositeExtract %float %697 2
        %709 = OpFDiv %float %707 %708
        %710 = OpCompositeConstruct %v3float %float_0 %float_0 %709
        %711 = OpCompositeConstruct %mat3v3float %702 %706 %710
        %712 = OpMatrixTimesMatrix %mat3v3float %452 %711
        %713 = OpMatrixTimesMatrix %mat3v3float %712 %456
        %714 = OpMatrixTimesMatrix %mat3v3float %422 %713
        %715 = OpMatrixTimesMatrix %mat3v3float %714 %418
        %716 = OpVectorTimesMatrix %v3float %599 %715
        %717 = OpVectorTimesMatrix %v3float %716 %547
        %718 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_9
        %719 = OpAccessChain %_ptr_Uniform_float %_Globals %int_9 %int_3
        %720 = OpLoad %float %719
        %721 = OpFOrdNotEqual %bool %720 %float_0
               OpSelectionMerge %722 None
               OpBranchConditional %721 %723 %722
        %723 = OpLabel
        %724 = OpDot %float %717 %67
        %725 = OpCompositeConstruct %v3float %724 %724 %724
        %726 = OpFDiv %v3float %717 %725
        %727 = OpFSub %v3float %726 %135
        %728 = OpDot %float %727 %727
        %729 = OpFMul %float %float_n4 %728
        %730 = OpExtInst %float %1 Exp2 %729
        %731 = OpFSub %float %float_1 %730
        %732 = OpAccessChain %_ptr_Uniform_float %_Globals %int_44
        %733 = OpLoad %float %732
        %734 = OpFMul %float %float_n4 %733
        %735 = OpFMul %float %734 %724
        %736 = OpFMul %float %735 %724
        %737 = OpExtInst %float %1 Exp2 %736
        %738 = OpFSub %float %float_1 %737
        %739 = OpFMul %float %731 %738
        %740 = OpMatrixTimesMatrix %mat3v3float %461 %406
        %741 = OpMatrixTimesMatrix %mat3v3float %549 %740
        %742 = OpVectorTimesMatrix %v3float %717 %741
        %743 = OpCompositeConstruct %v3float %739 %739 %739
        %744 = OpExtInst %v3float %1 FMix %717 %742 %743
               OpBranch %722
        %722 = OpLabel
        %745 = OpPhi %v3float %717 %581 %744 %723
        %746 = OpDot %float %745 %67
        %747 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_24
        %748 = OpLoad %v4float %747
        %749 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_19
        %750 = OpLoad %v4float %749
        %751 = OpFMul %v4float %748 %750
        %752 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_25
        %753 = OpLoad %v4float %752
        %754 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_20
        %755 = OpLoad %v4float %754
        %756 = OpFMul %v4float %753 %755
        %757 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_26
        %758 = OpLoad %v4float %757
        %759 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_21
        %760 = OpLoad %v4float %759
        %761 = OpFMul %v4float %758 %760
        %762 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_27
        %763 = OpLoad %v4float %762
        %764 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_22
        %765 = OpLoad %v4float %764
        %766 = OpFMul %v4float %763 %765
        %767 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_28
        %768 = OpLoad %v4float %767
        %769 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_23
        %770 = OpLoad %v4float %769
        %771 = OpFAdd %v4float %768 %770
        %772 = OpCompositeConstruct %v3float %746 %746 %746
        %773 = OpVectorShuffle %v3float %751 %751 0 1 2
        %774 = OpCompositeExtract %float %751 3
        %775 = OpCompositeConstruct %v3float %774 %774 %774
        %776 = OpFMul %v3float %773 %775
        %777 = OpExtInst %v3float %1 FMix %772 %745 %776
        %778 = OpExtInst %v3float %1 FMax %132 %777
        %779 = OpFMul %v3float %778 %307
        %780 = OpVectorShuffle %v3float %756 %756 0 1 2
        %781 = OpCompositeExtract %float %756 3
        %782 = OpCompositeConstruct %v3float %781 %781 %781
        %783 = OpFMul %v3float %780 %782
        %784 = OpExtInst %v3float %1 Pow %779 %783
        %785 = OpFMul %v3float %784 %194
        %786 = OpVectorShuffle %v3float %761 %761 0 1 2
        %787 = OpCompositeExtract %float %761 3
        %788 = OpCompositeConstruct %v3float %787 %787 %787
        %789 = OpFMul %v3float %786 %788
        %790 = OpFDiv %v3float %135 %789
        %791 = OpExtInst %v3float %1 Pow %785 %790
        %792 = OpVectorShuffle %v3float %766 %766 0 1 2
        %793 = OpCompositeExtract %float %766 3
        %794 = OpCompositeConstruct %v3float %793 %793 %793
        %795 = OpFMul %v3float %792 %794
        %796 = OpFMul %v3float %791 %795
        %797 = OpVectorShuffle %v3float %771 %771 0 1 2
        %798 = OpCompositeExtract %float %771 3
        %799 = OpCompositeConstruct %v3float %798 %798 %798
        %800 = OpFAdd %v3float %797 %799
        %801 = OpFAdd %v3float %796 %800
        %802 = OpAccessChain %_ptr_Uniform_float %_Globals %int_39
        %803 = OpLoad %float %802
        %804 = OpExtInst %float %1 SmoothStep %float_0 %803 %746
        %805 = OpFSub %float %float_1 %804
        %806 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_34
        %807 = OpLoad %v4float %806
        %808 = OpFMul %v4float %807 %750
        %809 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_35
        %810 = OpLoad %v4float %809
        %811 = OpFMul %v4float %810 %755
        %812 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_36
        %813 = OpLoad %v4float %812
        %814 = OpFMul %v4float %813 %760
        %815 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_37
        %816 = OpLoad %v4float %815
        %817 = OpFMul %v4float %816 %765
        %818 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_38
        %819 = OpLoad %v4float %818
        %820 = OpFAdd %v4float %819 %770
        %821 = OpVectorShuffle %v3float %808 %808 0 1 2
        %822 = OpCompositeExtract %float %808 3
        %823 = OpCompositeConstruct %v3float %822 %822 %822
        %824 = OpFMul %v3float %821 %823
        %825 = OpExtInst %v3float %1 FMix %772 %745 %824
        %826 = OpExtInst %v3float %1 FMax %132 %825
        %827 = OpFMul %v3float %826 %307
        %828 = OpVectorShuffle %v3float %811 %811 0 1 2
        %829 = OpCompositeExtract %float %811 3
        %830 = OpCompositeConstruct %v3float %829 %829 %829
        %831 = OpFMul %v3float %828 %830
        %832 = OpExtInst %v3float %1 Pow %827 %831
        %833 = OpFMul %v3float %832 %194
        %834 = OpVectorShuffle %v3float %814 %814 0 1 2
        %835 = OpCompositeExtract %float %814 3
        %836 = OpCompositeConstruct %v3float %835 %835 %835
        %837 = OpFMul %v3float %834 %836
        %838 = OpFDiv %v3float %135 %837
        %839 = OpExtInst %v3float %1 Pow %833 %838
        %840 = OpVectorShuffle %v3float %817 %817 0 1 2
        %841 = OpCompositeExtract %float %817 3
        %842 = OpCompositeConstruct %v3float %841 %841 %841
        %843 = OpFMul %v3float %840 %842
        %844 = OpFMul %v3float %839 %843
        %845 = OpVectorShuffle %v3float %820 %820 0 1 2
        %846 = OpCompositeExtract %float %820 3
        %847 = OpCompositeConstruct %v3float %846 %846 %846
        %848 = OpFAdd %v3float %845 %847
        %849 = OpFAdd %v3float %844 %848
        %850 = OpAccessChain %_ptr_Uniform_float %_Globals %int_40
        %851 = OpLoad %float %850
        %852 = OpExtInst %float %1 SmoothStep %851 %float_1 %746
        %853 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_29
        %854 = OpLoad %v4float %853
        %855 = OpFMul %v4float %854 %750
        %856 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_30
        %857 = OpLoad %v4float %856
        %858 = OpFMul %v4float %857 %755
        %859 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_31
        %860 = OpLoad %v4float %859
        %861 = OpFMul %v4float %860 %760
        %862 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_32
        %863 = OpLoad %v4float %862
        %864 = OpFMul %v4float %863 %765
        %865 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_33
        %866 = OpLoad %v4float %865
        %867 = OpFAdd %v4float %866 %770
        %868 = OpVectorShuffle %v3float %855 %855 0 1 2
        %869 = OpCompositeExtract %float %855 3
        %870 = OpCompositeConstruct %v3float %869 %869 %869
        %871 = OpFMul %v3float %868 %870
        %872 = OpExtInst %v3float %1 FMix %772 %745 %871
        %873 = OpExtInst %v3float %1 FMax %132 %872
        %874 = OpFMul %v3float %873 %307
        %875 = OpVectorShuffle %v3float %858 %858 0 1 2
        %876 = OpCompositeExtract %float %858 3
        %877 = OpCompositeConstruct %v3float %876 %876 %876
        %878 = OpFMul %v3float %875 %877
        %879 = OpExtInst %v3float %1 Pow %874 %878
        %880 = OpFMul %v3float %879 %194
        %881 = OpVectorShuffle %v3float %861 %861 0 1 2
        %882 = OpCompositeExtract %float %861 3
        %883 = OpCompositeConstruct %v3float %882 %882 %882
        %884 = OpFMul %v3float %881 %883
        %885 = OpFDiv %v3float %135 %884
        %886 = OpExtInst %v3float %1 Pow %880 %885
        %887 = OpVectorShuffle %v3float %864 %864 0 1 2
        %888 = OpCompositeExtract %float %864 3
        %889 = OpCompositeConstruct %v3float %888 %888 %888
        %890 = OpFMul %v3float %887 %889
        %891 = OpFMul %v3float %886 %890
        %892 = OpVectorShuffle %v3float %867 %867 0 1 2
        %893 = OpCompositeExtract %float %867 3
        %894 = OpCompositeConstruct %v3float %893 %893 %893
        %895 = OpFAdd %v3float %892 %894
        %896 = OpFAdd %v3float %891 %895
        %897 = OpFSub %float %804 %852
        %898 = OpCompositeConstruct %v3float %805 %805 %805
        %899 = OpFMul %v3float %801 %898
        %900 = OpCompositeConstruct %v3float %897 %897 %897
        %901 = OpFMul %v3float %896 %900
        %902 = OpFAdd %v3float %899 %901
        %903 = OpCompositeConstruct %v3float %852 %852 %852
        %904 = OpFMul %v3float %849 %903
        %905 = OpFAdd %v3float %902 %904
        %906 = OpVectorTimesMatrix %v3float %905 %549
        %907 = OpMatrixTimesMatrix %mat3v3float %551 %465
        %908 = OpMatrixTimesMatrix %mat3v3float %907 %550
        %909 = OpMatrixTimesMatrix %mat3v3float %551 %469
        %910 = OpMatrixTimesMatrix %mat3v3float %909 %550
        %911 = OpVectorTimesMatrix %v3float %905 %908
        %912 = OpAccessChain %_ptr_Uniform_float %_Globals %int_43
        %913 = OpLoad %float %912
        %914 = OpCompositeConstruct %v3float %913 %913 %913
        %915 = OpExtInst %v3float %1 FMix %905 %911 %914
        %916 = OpVectorTimesMatrix %v3float %915 %551
        %917 = OpCompositeExtract %float %916 0
        %918 = OpCompositeExtract %float %916 1
        %919 = OpExtInst %float %1 FMin %917 %918
        %920 = OpCompositeExtract %float %916 2
        %921 = OpExtInst %float %1 FMin %919 %920
        %922 = OpExtInst %float %1 FMax %917 %918
        %923 = OpExtInst %float %1 FMax %922 %920
        %924 = OpExtInst %float %1 FMax %923 %float_1_00000001en10
        %925 = OpExtInst %float %1 FMax %921 %float_1_00000001en10
        %926 = OpFSub %float %924 %925
        %927 = OpExtInst %float %1 FMax %923 %float_0_00999999978
        %928 = OpFDiv %float %926 %927
        %929 = OpFSub %float %920 %918
        %930 = OpFMul %float %920 %929
        %931 = OpFSub %float %918 %917
        %932 = OpFMul %float %918 %931
        %933 = OpFAdd %float %930 %932
        %934 = OpFSub %float %917 %920
        %935 = OpFMul %float %917 %934
        %936 = OpFAdd %float %933 %935
        %937 = OpExtInst %float %1 Sqrt %936
        %938 = OpFAdd %float %920 %918
        %939 = OpFAdd %float %938 %917
        %940 = OpFMul %float %float_1_75 %937
        %941 = OpFAdd %float %939 %940
        %942 = OpFMul %float %941 %float_0_333333343
        %943 = OpFSub %float %928 %float_0_400000006
        %944 = OpFMul %float %943 %float_5
        %945 = OpFMul %float %943 %float_2_5
        %946 = OpExtInst %float %1 FAbs %945
        %947 = OpFSub %float %float_1 %946
        %948 = OpExtInst %float %1 FMax %947 %float_0
        %949 = OpExtInst %float %1 FSign %944
        %950 = OpConvertFToS %int %949
        %951 = OpConvertSToF %float %950
        %952 = OpFMul %float %948 %948
        %953 = OpFSub %float %float_1 %952
        %954 = OpFMul %float %951 %953
        %955 = OpFAdd %float %float_1 %954
        %956 = OpFMul %float %955 %float_0_0250000004
        %957 = OpFOrdLessThanEqual %bool %942 %float_0_0533333346
               OpSelectionMerge %958 None
               OpBranchConditional %957 %959 %960
        %960 = OpLabel
        %961 = OpFOrdGreaterThanEqual %bool %942 %float_0_159999996
               OpSelectionMerge %962 None
               OpBranchConditional %961 %963 %964
        %964 = OpLabel
        %965 = OpFDiv %float %float_0_239999995 %941
        %966 = OpFSub %float %965 %float_0_5
        %967 = OpFMul %float %956 %966
               OpBranch %962
        %963 = OpLabel
               OpBranch %962
        %962 = OpLabel
        %968 = OpPhi %float %967 %964 %float_0 %963
               OpBranch %958
        %959 = OpLabel
               OpBranch %958
        %958 = OpLabel
        %969 = OpPhi %float %968 %962 %956 %959
        %970 = OpFAdd %float %float_1 %969
        %971 = OpCompositeConstruct %v3float %970 %970 %970
        %972 = OpFMul %v3float %916 %971
        %973 = OpCompositeExtract %float %972 0
        %974 = OpCompositeExtract %float %972 1
        %975 = OpFOrdEqual %bool %973 %974
        %976 = OpCompositeExtract %float %972 2
        %977 = OpFOrdEqual %bool %974 %976
        %978 = OpLogicalAnd %bool %975 %977
               OpSelectionMerge %979 None
               OpBranchConditional %978 %980 %981
        %981 = OpLabel
        %982 = OpExtInst %float %1 Sqrt %float_3
        %983 = OpFSub %float %974 %976
        %984 = OpFMul %float %982 %983
        %985 = OpFMul %float %float_2 %973
        %986 = OpFSub %float %985 %974
        %987 = OpFSub %float %986 %976
        %988 = OpExtInst %float %1 Atan2 %984 %987
        %989 = OpFMul %float %float_57_2957764 %988
               OpBranch %979
        %980 = OpLabel
               OpBranch %979
        %979 = OpLabel
        %990 = OpPhi %float %989 %981 %float_0 %980
        %991 = OpFOrdLessThan %bool %990 %float_0
               OpSelectionMerge %992 None
               OpBranchConditional %991 %993 %992
        %993 = OpLabel
        %994 = OpFAdd %float %990 %float_360
               OpBranch %992
        %992 = OpLabel
        %995 = OpPhi %float %990 %979 %994 %993
        %996 = OpExtInst %float %1 FClamp %995 %float_0 %float_360
        %997 = OpFOrdGreaterThan %bool %996 %float_180
               OpSelectionMerge %998 None
               OpBranchConditional %997 %999 %998
        %999 = OpLabel
       %1000 = OpFSub %float %996 %float_360
               OpBranch %998
        %998 = OpLabel
       %1001 = OpPhi %float %996 %992 %1000 %999
       %1002 = OpFMul %float %1001 %float_0_0148148146
       %1003 = OpExtInst %float %1 FAbs %1002
       %1004 = OpFSub %float %float_1 %1003
       %1005 = OpExtInst %float %1 SmoothStep %float_0 %float_1 %1004
       %1006 = OpFMul %float %1005 %1005
       %1007 = OpFMul %float %1006 %928
       %1008 = OpFSub %float %float_0_0299999993 %973
       %1009 = OpFMul %float %1007 %1008
       %1010 = OpFMul %float %1009 %float_0_180000007
       %1011 = OpFAdd %float %973 %1010
       %1012 = OpCompositeInsert %v3float %1011 %972 0
       %1013 = OpVectorTimesMatrix %v3float %1012 %410
       %1014 = OpExtInst %v3float %1 FMax %132 %1013
       %1015 = OpDot %float %1014 %67
       %1016 = OpCompositeConstruct %v3float %1015 %1015 %1015
       %1017 = OpExtInst %v3float %1 FMix %1016 %1014 %228
       %1018 = OpAccessChain %_ptr_Uniform_float %_Globals %int_13
       %1019 = OpLoad %float %1018
       %1020 = OpFAdd %float %float_1 %1019
       %1021 = OpAccessChain %_ptr_Uniform_float %_Globals %int_11
       %1022 = OpLoad %float %1021
       %1023 = OpFSub %float %1020 %1022
       %1024 = OpAccessChain %_ptr_Uniform_float %_Globals %int_14
       %1025 = OpLoad %float %1024
       %1026 = OpFAdd %float %float_1 %1025
       %1027 = OpAccessChain %_ptr_Uniform_float %_Globals %int_12
       %1028 = OpLoad %float %1027
       %1029 = OpFSub %float %1026 %1028
       %1030 = OpFOrdGreaterThan %bool %1022 %float_0_800000012
               OpSelectionMerge %1031 None
               OpBranchConditional %1030 %1032 %1033
       %1033 = OpLabel
       %1034 = OpFAdd %float %float_0_180000007 %1019
       %1035 = OpFDiv %float %1034 %1023
       %1036 = OpExtInst %float %1 Log %float_0_180000007
       %1037 = OpExtInst %float %1 Log %float_10
       %1038 = OpFDiv %float %1036 %1037
       %1039 = OpFSub %float %float_2 %1035
       %1040 = OpFDiv %float %1035 %1039
       %1041 = OpExtInst %float %1 Log %1040
       %1042 = OpFMul %float %float_0_5 %1041
       %1043 = OpAccessChain %_ptr_Uniform_float %_Globals %int_10
       %1044 = OpLoad %float %1043
       %1045 = OpFDiv %float %1023 %1044
       %1046 = OpFMul %float %1042 %1045
       %1047 = OpFSub %float %1038 %1046
               OpBranch %1031
       %1032 = OpLabel
       %1048 = OpFSub %float %float_0_819999993 %1022
       %1049 = OpAccessChain %_ptr_Uniform_float %_Globals %int_10
       %1050 = OpLoad %float %1049
       %1051 = OpFDiv %float %1048 %1050
       %1052 = OpExtInst %float %1 Log %float_0_180000007
       %1053 = OpExtInst %float %1 Log %float_10
       %1054 = OpFDiv %float %1052 %1053
       %1055 = OpFAdd %float %1051 %1054
               OpBranch %1031
       %1031 = OpLabel
       %1056 = OpPhi %float %1047 %1033 %1055 %1032
       %1057 = OpFSub %float %float_1 %1022
       %1058 = OpAccessChain %_ptr_Uniform_float %_Globals %int_10
       %1059 = OpLoad %float %1058
       %1060 = OpFDiv %float %1057 %1059
       %1061 = OpFSub %float %1060 %1056
       %1062 = OpFDiv %float %1028 %1059
       %1063 = OpFSub %float %1062 %1061
       %1064 = OpExtInst %v3float %1 Log %1017
       %1065 = OpExtInst %float %1 Log %float_10
       %1066 = OpCompositeConstruct %v3float %1065 %1065 %1065
       %1067 = OpFDiv %v3float %1064 %1066
       %1068 = OpCompositeConstruct %v3float %1059 %1059 %1059
       %1069 = OpCompositeConstruct %v3float %1061 %1061 %1061
       %1070 = OpFAdd %v3float %1067 %1069
       %1071 = OpFMul %v3float %1068 %1070
       %1072 = OpFNegate %float %1019
       %1073 = OpCompositeConstruct %v3float %1072 %1072 %1072
       %1074 = OpFMul %float %float_2 %1023
       %1075 = OpCompositeConstruct %v3float %1074 %1074 %1074
       %1076 = OpFMul %float %float_n2 %1059
       %1077 = OpFDiv %float %1076 %1023
       %1078 = OpCompositeConstruct %v3float %1077 %1077 %1077
       %1079 = OpCompositeConstruct %v3float %1056 %1056 %1056
       %1080 = OpFSub %v3float %1067 %1079
       %1081 = OpFMul %v3float %1078 %1080
       %1082 = OpExtInst %v3float %1 Exp %1081
       %1083 = OpFAdd %v3float %135 %1082
       %1084 = OpFDiv %v3float %1075 %1083
       %1085 = OpFAdd %v3float %1073 %1084
       %1086 = OpCompositeConstruct %v3float %1026 %1026 %1026
       %1087 = OpFMul %float %float_2 %1029
       %1088 = OpCompositeConstruct %v3float %1087 %1087 %1087
       %1089 = OpFMul %float %float_2 %1059
       %1090 = OpFDiv %float %1089 %1029
       %1091 = OpCompositeConstruct %v3float %1090 %1090 %1090
       %1092 = OpCompositeConstruct %v3float %1063 %1063 %1063
       %1093 = OpFSub %v3float %1067 %1092
       %1094 = OpFMul %v3float %1091 %1093
       %1095 = OpExtInst %v3float %1 Exp %1094
       %1096 = OpFAdd %v3float %135 %1095
       %1097 = OpFDiv %v3float %1088 %1096
       %1098 = OpFSub %v3float %1086 %1097
       %1099 = OpFOrdLessThan %v3bool %1067 %1079
       %1100 = OpSelect %v3float %1099 %1085 %1071
       %1101 = OpFOrdGreaterThan %v3bool %1067 %1092
       %1102 = OpSelect %v3float %1101 %1098 %1071
       %1103 = OpFSub %float %1063 %1056
       %1104 = OpCompositeConstruct %v3float %1103 %1103 %1103
       %1105 = OpFDiv %v3float %1080 %1104
       %1106 = OpExtInst %v3float %1 FClamp %1105 %132 %135
       %1107 = OpFOrdLessThan %bool %1063 %1056
       %1108 = OpFSub %v3float %135 %1106
       %1109 = OpCompositeConstruct %v3bool %1107 %1107 %1107
       %1110 = OpSelect %v3float %1109 %1108 %1106
       %1111 = OpFMul %v3float %239 %1110
       %1112 = OpFSub %v3float %238 %1111
       %1113 = OpFMul %v3float %1112 %1110
       %1114 = OpFMul %v3float %1113 %1110
       %1115 = OpExtInst %v3float %1 FMix %1100 %1102 %1114
       %1116 = OpDot %float %1115 %67
       %1117 = OpCompositeConstruct %v3float %1116 %1116 %1116
       %1118 = OpExtInst %v3float %1 FMix %1117 %1115 %241
       %1119 = OpExtInst %v3float %1 FMax %132 %1118
       %1120 = OpVectorTimesMatrix %v3float %1119 %910
       %1121 = OpExtInst %v3float %1 FMix %1119 %1120 %914
       %1122 = OpVectorTimesMatrix %v3float %1121 %549
       %1123 = OpExtInst %v3float %1 FMax %132 %1122
       %1124 = OpFOrdEqual %bool %720 %float_0
               OpSelectionMerge %1125 DontFlatten
               OpBranchConditional %1124 %1126 %1125
       %1126 = OpLabel
       %1127 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_2
       %1128 = OpLoad %v4float %1127
       %1129 = OpVectorShuffle %v3float %1128 %1128 0 1 2
       %1130 = OpDot %float %906 %1129
       %1131 = OpCompositeInsert %v3float %1130 %391 0
       %1132 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_3
       %1133 = OpLoad %v4float %1132
       %1134 = OpVectorShuffle %v3float %1133 %1133 0 1 2
       %1135 = OpDot %float %906 %1134
       %1136 = OpCompositeInsert %v3float %1135 %1131 1
       %1137 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_4
       %1138 = OpLoad %v4float %1137
       %1139 = OpVectorShuffle %v3float %1138 %1138 0 1 2
       %1140 = OpDot %float %906 %1139
       %1141 = OpCompositeInsert %v3float %1140 %1136 2
       %1142 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_8
       %1143 = OpLoad %v4float %1142
       %1144 = OpVectorShuffle %v3float %1143 %1143 0 1 2
       %1145 = OpLoad %v4float %718
       %1146 = OpVectorShuffle %v3float %1145 %1145 0 1 2
       %1147 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_7
       %1148 = OpLoad %v4float %1147
       %1149 = OpVectorShuffle %v3float %1148 %1148 0 1 2
       %1150 = OpDot %float %906 %1149
       %1151 = OpFAdd %float %1150 %float_1
       %1152 = OpFDiv %float %float_1 %1151
       %1153 = OpCompositeConstruct %v3float %1152 %1152 %1152
       %1154 = OpFMul %v3float %1146 %1153
       %1155 = OpFAdd %v3float %1144 %1154
       %1156 = OpFMul %v3float %1141 %1155
       %1157 = OpExtInst %v3float %1 FMax %132 %1156
       %1158 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_5
       %1159 = OpLoad %v4float %1158
       %1160 = OpVectorShuffle %v3float %1159 %1159 0 0 0
       %1161 = OpFSub %v3float %1160 %1157
       %1162 = OpExtInst %v3float %1 FMax %132 %1161
       %1163 = OpVectorShuffle %v3float %1159 %1159 2 2 2
       %1164 = OpExtInst %v3float %1 FMax %1157 %1163
       %1165 = OpExtInst %v3float %1 FClamp %1157 %1160 %1163
       %1166 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_6
       %1167 = OpLoad %v4float %1166
       %1168 = OpVectorShuffle %v3float %1167 %1167 0 0 0
       %1169 = OpFMul %v3float %1164 %1168
       %1170 = OpVectorShuffle %v3float %1167 %1167 1 1 1
       %1171 = OpFAdd %v3float %1169 %1170
       %1172 = OpVectorShuffle %v3float %1159 %1159 3 3 3
       %1173 = OpFAdd %v3float %1164 %1172
       %1174 = OpFDiv %v3float %135 %1173
       %1175 = OpFMul %v3float %1171 %1174
       %1176 = OpVectorShuffle %v3float %1138 %1138 3 3 3
       %1177 = OpFMul %v3float %1165 %1176
       %1178 = OpVectorShuffle %v3float %1128 %1128 3 3 3
       %1179 = OpFMul %v3float %1162 %1178
       %1180 = OpVectorShuffle %v3float %1159 %1159 1 1 1
       %1181 = OpFAdd %v3float %1162 %1180
       %1182 = OpFDiv %v3float %135 %1181
       %1183 = OpFMul %v3float %1179 %1182
       %1184 = OpVectorShuffle %v3float %1133 %1133 3 3 3
       %1185 = OpFAdd %v3float %1183 %1184
       %1186 = OpFAdd %v3float %1177 %1185
       %1187 = OpFAdd %v3float %1175 %1186
       %1188 = OpFSub %v3float %1187 %248
               OpBranch %1125
       %1125 = OpLabel
       %1189 = OpPhi %v3float %1123 %1031 %1188 %1126
       %1190 = OpAccessChain %_ptr_Uniform_float %_Globals %int_0 %int_0
       %1191 = OpLoad %float %1190
       %1192 = OpCompositeConstruct %v3float %1191 %1191 %1191
       %1193 = OpFMul %v3float %1189 %1189
       %1194 = OpFMul %v3float %1192 %1193
       %1195 = OpAccessChain %_ptr_Uniform_float %_Globals %int_0 %int_1
       %1196 = OpLoad %float %1195
       %1197 = OpCompositeConstruct %v3float %1196 %1196 %1196
       %1198 = OpFMul %v3float %1197 %1189
       %1199 = OpFAdd %v3float %1194 %1198
       %1200 = OpAccessChain %_ptr_Uniform_float %_Globals %int_0 %int_2
       %1201 = OpLoad %float %1200
       %1202 = OpCompositeConstruct %v3float %1201 %1201 %1201
       %1203 = OpFAdd %v3float %1199 %1202
       %1204 = OpAccessChain %_ptr_Uniform_v3float %_Globals %int_15
       %1205 = OpLoad %v3float %1204
       %1206 = OpFMul %v3float %1203 %1205
       %1207 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_16
       %1208 = OpLoad %v4float %1207
       %1209 = OpVectorShuffle %v3float %1208 %1208 0 1 2
       %1210 = OpAccessChain %_ptr_Uniform_float %_Globals %int_16 %int_3
       %1211 = OpLoad %float %1210
       %1212 = OpCompositeConstruct %v3float %1211 %1211 %1211
       %1213 = OpExtInst %v3float %1 FMix %1206 %1209 %1212
       %1214 = OpExtInst %v3float %1 FMax %132 %1213
       %1215 = OpAccessChain %_ptr_Uniform_float %_Globals %int_1 %int_1
       %1216 = OpLoad %float %1215
       %1217 = OpCompositeConstruct %v3float %1216 %1216 %1216
       %1218 = OpExtInst %v3float %1 Pow %1214 %1217
       %1219 = OpIEqual %bool %579 %uint_0
               OpSelectionMerge %1220 DontFlatten
               OpBranchConditional %1219 %1221 %1222
       %1222 = OpLabel
       %1223 = OpIEqual %bool %579 %uint_1
               OpSelectionMerge %1224 None
               OpBranchConditional %1223 %1225 %1226
       %1226 = OpLabel
       %1227 = OpIEqual %bool %579 %uint_3
       %1228 = OpIEqual %bool %579 %uint_5
       %1229 = OpLogicalOr %bool %1227 %1228
               OpSelectionMerge %1230 None
               OpBranchConditional %1229 %1231 %1232
       %1232 = OpLabel
       %1233 = OpIEqual %bool %579 %uint_4
       %1234 = OpIEqual %bool %579 %uint_6
       %1235 = OpLogicalOr %bool %1233 %1234
               OpSelectionMerge %1236 None
               OpBranchConditional %1235 %1237 %1238
       %1238 = OpLabel
       %1239 = OpIEqual %bool %579 %uint_7
               OpSelectionMerge %1240 None
               OpBranchConditional %1239 %1241 %1242
       %1242 = OpLabel
       %1243 = OpVectorTimesMatrix %v3float %1218 %547
       %1244 = OpVectorTimesMatrix %v3float %1243 %576
       %1245 = OpAccessChain %_ptr_Uniform_float %_Globals %int_1 %int_2
       %1246 = OpLoad %float %1245
       %1247 = OpCompositeConstruct %v3float %1246 %1246 %1246
       %1248 = OpExtInst %v3float %1 Pow %1244 %1247
               OpBranch %1240
       %1241 = OpLabel
       %1249 = OpVectorTimesMatrix %v3float %906 %547
       %1250 = OpVectorTimesMatrix %v3float %1249 %576
       %1251 = OpFMul %v3float %1250 %496
       %1252 = OpExtInst %v3float %1 Pow %1251 %263
       %1253 = OpFMul %v3float %184 %1252
       %1254 = OpFAdd %v3float %183 %1253
       %1255 = OpFMul %v3float %185 %1252
       %1256 = OpFAdd %v3float %135 %1255
       %1257 = OpFDiv %v3float %135 %1256
       %1258 = OpFMul %v3float %1254 %1257
       %1259 = OpExtInst %v3float %1 Pow %1258 %264
               OpBranch %1240
       %1240 = OpLabel
       %1260 = OpPhi %v3float %1248 %1242 %1259 %1241
               OpBranch %1236
       %1237 = OpLabel
       %1261 = OpMatrixTimesMatrix %mat3v3float %546 %399
       %1262 = OpFMul %v3float %906 %262
       %1263 = OpVectorTimesMatrix %v3float %1262 %1261
       %1264 = OpCompositeExtract %float %1263 0
       %1265 = OpCompositeExtract %float %1263 1
       %1266 = OpExtInst %float %1 FMin %1264 %1265
       %1267 = OpCompositeExtract %float %1263 2
       %1268 = OpExtInst %float %1 FMin %1266 %1267
       %1269 = OpExtInst %float %1 FMax %1264 %1265
       %1270 = OpExtInst %float %1 FMax %1269 %1267
       %1271 = OpExtInst %float %1 FMax %1270 %float_1_00000001en10
       %1272 = OpExtInst %float %1 FMax %1268 %float_1_00000001en10
       %1273 = OpFSub %float %1271 %1272
       %1274 = OpExtInst %float %1 FMax %1270 %float_0_00999999978
       %1275 = OpFDiv %float %1273 %1274
       %1276 = OpFSub %float %1267 %1265
       %1277 = OpFMul %float %1267 %1276
       %1278 = OpFSub %float %1265 %1264
       %1279 = OpFMul %float %1265 %1278
       %1280 = OpFAdd %float %1277 %1279
       %1281 = OpFSub %float %1264 %1267
       %1282 = OpFMul %float %1264 %1281
       %1283 = OpFAdd %float %1280 %1282
       %1284 = OpExtInst %float %1 Sqrt %1283
       %1285 = OpFAdd %float %1267 %1265
       %1286 = OpFAdd %float %1285 %1264
       %1287 = OpFMul %float %float_1_75 %1284
       %1288 = OpFAdd %float %1286 %1287
       %1289 = OpFMul %float %1288 %float_0_333333343
       %1290 = OpFSub %float %1275 %float_0_400000006
       %1291 = OpFMul %float %1290 %float_5
       %1292 = OpFMul %float %1290 %float_2_5
       %1293 = OpExtInst %float %1 FAbs %1292
       %1294 = OpFSub %float %float_1 %1293
       %1295 = OpExtInst %float %1 FMax %1294 %float_0
       %1296 = OpExtInst %float %1 FSign %1291
       %1297 = OpConvertFToS %int %1296
       %1298 = OpConvertSToF %float %1297
       %1299 = OpFMul %float %1295 %1295
       %1300 = OpFSub %float %float_1 %1299
       %1301 = OpFMul %float %1298 %1300
       %1302 = OpFAdd %float %float_1 %1301
       %1303 = OpFMul %float %1302 %float_0_0250000004
       %1304 = OpFOrdLessThanEqual %bool %1289 %float_0_0533333346
               OpSelectionMerge %1305 None
               OpBranchConditional %1304 %1306 %1307
       %1307 = OpLabel
       %1308 = OpFOrdGreaterThanEqual %bool %1289 %float_0_159999996
               OpSelectionMerge %1309 None
               OpBranchConditional %1308 %1310 %1311
       %1311 = OpLabel
       %1312 = OpFDiv %float %float_0_239999995 %1288
       %1313 = OpFSub %float %1312 %float_0_5
       %1314 = OpFMul %float %1303 %1313
               OpBranch %1309
       %1310 = OpLabel
               OpBranch %1309
       %1309 = OpLabel
       %1315 = OpPhi %float %1314 %1311 %float_0 %1310
               OpBranch %1305
       %1306 = OpLabel
               OpBranch %1305
       %1305 = OpLabel
       %1316 = OpPhi %float %1315 %1309 %1303 %1306
       %1317 = OpFAdd %float %float_1 %1316
       %1318 = OpCompositeConstruct %v3float %1317 %1317 %1317
       %1319 = OpFMul %v3float %1263 %1318
       %1320 = OpCompositeExtract %float %1319 0
       %1321 = OpCompositeExtract %float %1319 1
       %1322 = OpFOrdEqual %bool %1320 %1321
       %1323 = OpCompositeExtract %float %1319 2
       %1324 = OpFOrdEqual %bool %1321 %1323
       %1325 = OpLogicalAnd %bool %1322 %1324
               OpSelectionMerge %1326 None
               OpBranchConditional %1325 %1327 %1328
       %1328 = OpLabel
       %1329 = OpExtInst %float %1 Sqrt %float_3
       %1330 = OpFSub %float %1321 %1323
       %1331 = OpFMul %float %1329 %1330
       %1332 = OpFMul %float %float_2 %1320
       %1333 = OpFSub %float %1332 %1321
       %1334 = OpFSub %float %1333 %1323
       %1335 = OpExtInst %float %1 Atan2 %1331 %1334
       %1336 = OpFMul %float %float_57_2957764 %1335
               OpBranch %1326
       %1327 = OpLabel
               OpBranch %1326
       %1326 = OpLabel
       %1337 = OpPhi %float %1336 %1328 %float_0 %1327
       %1338 = OpFOrdLessThan %bool %1337 %float_0
               OpSelectionMerge %1339 None
               OpBranchConditional %1338 %1340 %1339
       %1340 = OpLabel
       %1341 = OpFAdd %float %1337 %float_360
               OpBranch %1339
       %1339 = OpLabel
       %1342 = OpPhi %float %1337 %1326 %1341 %1340
       %1343 = OpExtInst %float %1 FClamp %1342 %float_0 %float_360
       %1344 = OpFOrdGreaterThan %bool %1343 %float_180
               OpSelectionMerge %1345 None
               OpBranchConditional %1344 %1346 %1345
       %1346 = OpLabel
       %1347 = OpFSub %float %1343 %float_360
               OpBranch %1345
       %1345 = OpLabel
       %1348 = OpPhi %float %1343 %1339 %1347 %1346
       %1349 = OpFOrdGreaterThan %bool %1348 %float_n67_5
       %1350 = OpFOrdLessThan %bool %1348 %float_67_5
       %1351 = OpLogicalAnd %bool %1349 %1350
               OpSelectionMerge %1352 None
               OpBranchConditional %1351 %1353 %1352
       %1353 = OpLabel
       %1354 = OpFSub %float %1348 %float_n67_5
       %1355 = OpFMul %float %1354 %float_0_0296296291
       %1356 = OpConvertFToS %int %1355
       %1357 = OpConvertSToF %float %1356
       %1358 = OpFSub %float %1355 %1357
       %1359 = OpFMul %float %1358 %1358
       %1360 = OpFMul %float %1359 %1358
       %1361 = OpIEqual %bool %1356 %int_3
               OpSelectionMerge %1362 None
               OpBranchConditional %1361 %1363 %1364
       %1364 = OpLabel
       %1365 = OpIEqual %bool %1356 %int_2
               OpSelectionMerge %1366 None
               OpBranchConditional %1365 %1367 %1368
       %1368 = OpLabel
       %1369 = OpIEqual %bool %1356 %int_1
               OpSelectionMerge %1370 None
               OpBranchConditional %1369 %1371 %1372
       %1372 = OpLabel
       %1373 = OpIEqual %bool %1356 %int_0
               OpSelectionMerge %1374 None
               OpBranchConditional %1373 %1375 %1376
       %1376 = OpLabel
               OpBranch %1374
       %1375 = OpLabel
       %1377 = OpFMul %float %1360 %float_0_166666672
               OpBranch %1374
       %1374 = OpLabel
       %1378 = OpPhi %float %float_0 %1376 %1377 %1375
               OpBranch %1370
       %1371 = OpLabel
       %1379 = OpFMul %float %1360 %float_n0_5
       %1380 = OpFMul %float %1359 %float_0_5
       %1381 = OpFAdd %float %1379 %1380
       %1382 = OpFMul %float %1358 %float_0_5
       %1383 = OpFAdd %float %1381 %1382
       %1384 = OpFAdd %float %1383 %float_0_166666672
               OpBranch %1370
       %1370 = OpLabel
       %1385 = OpPhi %float %1378 %1374 %1384 %1371
               OpBranch %1366
       %1367 = OpLabel
       %1386 = OpFMul %float %1360 %float_0_5
       %1387 = OpFMul %float %1359 %float_n1
       %1388 = OpFAdd %float %1386 %1387
       %1389 = OpFAdd %float %1388 %float_0_666666687
               OpBranch %1366
       %1366 = OpLabel
       %1390 = OpPhi %float %1385 %1370 %1389 %1367
               OpBranch %1362
       %1363 = OpLabel
       %1391 = OpFMul %float %1360 %float_n0_166666672
       %1392 = OpFMul %float %1359 %float_0_5
       %1393 = OpFAdd %float %1391 %1392
       %1394 = OpFMul %float %1358 %float_n0_5
       %1395 = OpFAdd %float %1393 %1394
       %1396 = OpFAdd %float %1395 %float_0_166666672
               OpBranch %1362
       %1362 = OpLabel
       %1397 = OpPhi %float %1390 %1366 %1396 %1363
               OpBranch %1352
       %1352 = OpLabel
       %1398 = OpPhi %float %float_0 %1345 %1397 %1362
       %1399 = OpFMul %float %1398 %float_1_5
       %1400 = OpFMul %float %1399 %1275
       %1401 = OpFSub %float %float_0_0299999993 %1320
       %1402 = OpFMul %float %1400 %1401
       %1403 = OpFMul %float %1402 %float_0_180000007
       %1404 = OpFAdd %float %1320 %1403
       %1405 = OpCompositeInsert %v3float %1404 %1319 0
       %1406 = OpExtInst %v3float %1 FClamp %1405 %132 %314
       %1407 = OpVectorTimesMatrix %v3float %1406 %410
       %1408 = OpExtInst %v3float %1 FClamp %1407 %132 %314
       %1409 = OpDot %float %1408 %67
       %1410 = OpCompositeConstruct %v3float %1409 %1409 %1409
       %1411 = OpExtInst %v3float %1 FMix %1410 %1408 %228
       %1412 = OpCompositeExtract %float %1411 0
       %1413 = OpExtInst %float %1 Exp2 %float_n15
       %1414 = OpFMul %float %float_0_179999992 %1413
       %1415 = OpExtInst %float %1 Exp2 %float_18
       %1416 = OpFMul %float %float_0_179999992 %1415
               OpStore %502 %475
               OpStore %501 %476
       %1417 = OpFOrdLessThanEqual %bool %1412 %float_0
       %1418 = OpExtInst %float %1 Exp2 %float_n14
       %1419 = OpSelect %float %1417 %1418 %1412
       %1420 = OpExtInst %float %1 Log %1419
       %1421 = OpFDiv %float %1420 %1065
       %1422 = OpExtInst %float %1 Log %1414
       %1423 = OpFDiv %float %1422 %1065
       %1424 = OpFOrdLessThanEqual %bool %1421 %1423
               OpSelectionMerge %1425 None
               OpBranchConditional %1424 %1426 %1427
       %1427 = OpLabel
       %1428 = OpFOrdGreaterThan %bool %1421 %1423
       %1429 = OpExtInst %float %1 Log %float_0_180000007
       %1430 = OpFDiv %float %1429 %1065
       %1431 = OpFOrdLessThan %bool %1421 %1430
       %1432 = OpLogicalAnd %bool %1428 %1431
               OpSelectionMerge %1433 None
               OpBranchConditional %1432 %1434 %1435
       %1435 = OpLabel
       %1436 = OpFOrdGreaterThanEqual %bool %1421 %1430
       %1437 = OpExtInst %float %1 Log %1416
       %1438 = OpFDiv %float %1437 %1065
       %1439 = OpFOrdLessThan %bool %1421 %1438
       %1440 = OpLogicalAnd %bool %1436 %1439
               OpSelectionMerge %1441 None
               OpBranchConditional %1440 %1442 %1443
       %1443 = OpLabel
       %1444 = OpExtInst %float %1 Log %float_10000
       %1445 = OpFDiv %float %1444 %1065
               OpBranch %1441
       %1442 = OpLabel
       %1446 = OpFSub %float %1421 %1430
       %1447 = OpFMul %float %float_3 %1446
       %1448 = OpFSub %float %1438 %1430
       %1449 = OpFDiv %float %1447 %1448
       %1450 = OpConvertFToS %int %1449
       %1451 = OpConvertSToF %float %1450
       %1452 = OpFSub %float %1449 %1451
       %1453 = OpAccessChain %_ptr_Function_float %501 %1450
       %1454 = OpLoad %float %1453
       %1455 = OpIAdd %int %1450 %int_1
       %1456 = OpAccessChain %_ptr_Function_float %501 %1455
       %1457 = OpLoad %float %1456
       %1458 = OpIAdd %int %1450 %int_2
       %1459 = OpAccessChain %_ptr_Function_float %501 %1458
       %1460 = OpLoad %float %1459
       %1461 = OpCompositeConstruct %v3float %1454 %1457 %1460
       %1462 = OpFMul %float %1452 %1452
       %1463 = OpCompositeConstruct %v3float %1462 %1452 %float_1
       %1464 = OpMatrixTimesVector %v3float %442 %1461
       %1465 = OpDot %float %1463 %1464
               OpBranch %1441
       %1441 = OpLabel
       %1466 = OpPhi %float %1445 %1443 %1465 %1442
               OpBranch %1433
       %1434 = OpLabel
       %1467 = OpFSub %float %1421 %1423
       %1468 = OpFMul %float %float_3 %1467
       %1469 = OpFSub %float %1430 %1423
       %1470 = OpFDiv %float %1468 %1469
       %1471 = OpConvertFToS %int %1470
       %1472 = OpConvertSToF %float %1471
       %1473 = OpFSub %float %1470 %1472
       %1474 = OpAccessChain %_ptr_Function_float %502 %1471
       %1475 = OpLoad %float %1474
       %1476 = OpIAdd %int %1471 %int_1
       %1477 = OpAccessChain %_ptr_Function_float %502 %1476
       %1478 = OpLoad %float %1477
       %1479 = OpIAdd %int %1471 %int_2
       %1480 = OpAccessChain %_ptr_Function_float %502 %1479
       %1481 = OpLoad %float %1480
       %1482 = OpCompositeConstruct %v3float %1475 %1478 %1481
       %1483 = OpFMul %float %1473 %1473
       %1484 = OpCompositeConstruct %v3float %1483 %1473 %float_1
       %1485 = OpMatrixTimesVector %v3float %442 %1482
       %1486 = OpDot %float %1484 %1485
               OpBranch %1433
       %1433 = OpLabel
       %1487 = OpPhi %float %1466 %1441 %1486 %1434
               OpBranch %1425
       %1426 = OpLabel
       %1488 = OpExtInst %float %1 Log %float_9_99999975en05
       %1489 = OpFDiv %float %1488 %1065
               OpBranch %1425
       %1425 = OpLabel
       %1490 = OpPhi %float %1487 %1433 %1489 %1426
       %1491 = OpExtInst %float %1 Pow %float_10 %1490
       %1492 = OpCompositeInsert %v3float %1491 %391 0
       %1493 = OpCompositeExtract %float %1411 1
               OpStore %504 %475
               OpStore %503 %476
       %1494 = OpFOrdLessThanEqual %bool %1493 %float_0
       %1495 = OpSelect %float %1494 %1418 %1493
       %1496 = OpExtInst %float %1 Log %1495
       %1497 = OpFDiv %float %1496 %1065
       %1498 = OpFOrdLessThanEqual %bool %1497 %1423
               OpSelectionMerge %1499 None
               OpBranchConditional %1498 %1500 %1501
       %1501 = OpLabel
       %1502 = OpFOrdGreaterThan %bool %1497 %1423
       %1503 = OpExtInst %float %1 Log %float_0_180000007
       %1504 = OpFDiv %float %1503 %1065
       %1505 = OpFOrdLessThan %bool %1497 %1504
       %1506 = OpLogicalAnd %bool %1502 %1505
               OpSelectionMerge %1507 None
               OpBranchConditional %1506 %1508 %1509
       %1509 = OpLabel
       %1510 = OpFOrdGreaterThanEqual %bool %1497 %1504
       %1511 = OpExtInst %float %1 Log %1416
       %1512 = OpFDiv %float %1511 %1065
       %1513 = OpFOrdLessThan %bool %1497 %1512
       %1514 = OpLogicalAnd %bool %1510 %1513
               OpSelectionMerge %1515 None
               OpBranchConditional %1514 %1516 %1517
       %1517 = OpLabel
       %1518 = OpExtInst %float %1 Log %float_10000
       %1519 = OpFDiv %float %1518 %1065
               OpBranch %1515
       %1516 = OpLabel
       %1520 = OpFSub %float %1497 %1504
       %1521 = OpFMul %float %float_3 %1520
       %1522 = OpFSub %float %1512 %1504
       %1523 = OpFDiv %float %1521 %1522
       %1524 = OpConvertFToS %int %1523
       %1525 = OpConvertSToF %float %1524
       %1526 = OpFSub %float %1523 %1525
       %1527 = OpAccessChain %_ptr_Function_float %503 %1524
       %1528 = OpLoad %float %1527
       %1529 = OpIAdd %int %1524 %int_1
       %1530 = OpAccessChain %_ptr_Function_float %503 %1529
       %1531 = OpLoad %float %1530
       %1532 = OpIAdd %int %1524 %int_2
       %1533 = OpAccessChain %_ptr_Function_float %503 %1532
       %1534 = OpLoad %float %1533
       %1535 = OpCompositeConstruct %v3float %1528 %1531 %1534
       %1536 = OpFMul %float %1526 %1526
       %1537 = OpCompositeConstruct %v3float %1536 %1526 %float_1
       %1538 = OpMatrixTimesVector %v3float %442 %1535
       %1539 = OpDot %float %1537 %1538
               OpBranch %1515
       %1515 = OpLabel
       %1540 = OpPhi %float %1519 %1517 %1539 %1516
               OpBranch %1507
       %1508 = OpLabel
       %1541 = OpFSub %float %1497 %1423
       %1542 = OpFMul %float %float_3 %1541
       %1543 = OpFSub %float %1504 %1423
       %1544 = OpFDiv %float %1542 %1543
       %1545 = OpConvertFToS %int %1544
       %1546 = OpConvertSToF %float %1545
       %1547 = OpFSub %float %1544 %1546
       %1548 = OpAccessChain %_ptr_Function_float %504 %1545
       %1549 = OpLoad %float %1548
       %1550 = OpIAdd %int %1545 %int_1
       %1551 = OpAccessChain %_ptr_Function_float %504 %1550
       %1552 = OpLoad %float %1551
       %1553 = OpIAdd %int %1545 %int_2
       %1554 = OpAccessChain %_ptr_Function_float %504 %1553
       %1555 = OpLoad %float %1554
       %1556 = OpCompositeConstruct %v3float %1549 %1552 %1555
       %1557 = OpFMul %float %1547 %1547
       %1558 = OpCompositeConstruct %v3float %1557 %1547 %float_1
       %1559 = OpMatrixTimesVector %v3float %442 %1556
       %1560 = OpDot %float %1558 %1559
               OpBranch %1507
       %1507 = OpLabel
       %1561 = OpPhi %float %1540 %1515 %1560 %1508
               OpBranch %1499
       %1500 = OpLabel
       %1562 = OpExtInst %float %1 Log %float_9_99999975en05
       %1563 = OpFDiv %float %1562 %1065
               OpBranch %1499
       %1499 = OpLabel
       %1564 = OpPhi %float %1561 %1507 %1563 %1500
       %1565 = OpExtInst %float %1 Pow %float_10 %1564
       %1566 = OpCompositeInsert %v3float %1565 %1492 1
       %1567 = OpCompositeExtract %float %1411 2
               OpStore %506 %475
               OpStore %505 %476
       %1568 = OpFOrdLessThanEqual %bool %1567 %float_0
       %1569 = OpSelect %float %1568 %1418 %1567
       %1570 = OpExtInst %float %1 Log %1569
       %1571 = OpFDiv %float %1570 %1065
       %1572 = OpFOrdLessThanEqual %bool %1571 %1423
               OpSelectionMerge %1573 None
               OpBranchConditional %1572 %1574 %1575
       %1575 = OpLabel
       %1576 = OpFOrdGreaterThan %bool %1571 %1423
       %1577 = OpExtInst %float %1 Log %float_0_180000007
       %1578 = OpFDiv %float %1577 %1065
       %1579 = OpFOrdLessThan %bool %1571 %1578
       %1580 = OpLogicalAnd %bool %1576 %1579
               OpSelectionMerge %1581 None
               OpBranchConditional %1580 %1582 %1583
       %1583 = OpLabel
       %1584 = OpFOrdGreaterThanEqual %bool %1571 %1578
       %1585 = OpExtInst %float %1 Log %1416
       %1586 = OpFDiv %float %1585 %1065
       %1587 = OpFOrdLessThan %bool %1571 %1586
       %1588 = OpLogicalAnd %bool %1584 %1587
               OpSelectionMerge %1589 None
               OpBranchConditional %1588 %1590 %1591
       %1591 = OpLabel
       %1592 = OpExtInst %float %1 Log %float_10000
       %1593 = OpFDiv %float %1592 %1065
               OpBranch %1589
       %1590 = OpLabel
       %1594 = OpFSub %float %1571 %1578
       %1595 = OpFMul %float %float_3 %1594
       %1596 = OpFSub %float %1586 %1578
       %1597 = OpFDiv %float %1595 %1596
       %1598 = OpConvertFToS %int %1597
       %1599 = OpConvertSToF %float %1598
       %1600 = OpFSub %float %1597 %1599
       %1601 = OpAccessChain %_ptr_Function_float %505 %1598
       %1602 = OpLoad %float %1601
       %1603 = OpIAdd %int %1598 %int_1
       %1604 = OpAccessChain %_ptr_Function_float %505 %1603
       %1605 = OpLoad %float %1604
       %1606 = OpIAdd %int %1598 %int_2
       %1607 = OpAccessChain %_ptr_Function_float %505 %1606
       %1608 = OpLoad %float %1607
       %1609 = OpCompositeConstruct %v3float %1602 %1605 %1608
       %1610 = OpFMul %float %1600 %1600
       %1611 = OpCompositeConstruct %v3float %1610 %1600 %float_1
       %1612 = OpMatrixTimesVector %v3float %442 %1609
       %1613 = OpDot %float %1611 %1612
               OpBranch %1589
       %1589 = OpLabel
       %1614 = OpPhi %float %1593 %1591 %1613 %1590
               OpBranch %1581
       %1582 = OpLabel
       %1615 = OpFSub %float %1571 %1423
       %1616 = OpFMul %float %float_3 %1615
       %1617 = OpFSub %float %1578 %1423
       %1618 = OpFDiv %float %1616 %1617
       %1619 = OpConvertFToS %int %1618
       %1620 = OpConvertSToF %float %1619
       %1621 = OpFSub %float %1618 %1620
       %1622 = OpAccessChain %_ptr_Function_float %506 %1619
       %1623 = OpLoad %float %1622
       %1624 = OpIAdd %int %1619 %int_1
       %1625 = OpAccessChain %_ptr_Function_float %506 %1624
       %1626 = OpLoad %float %1625
       %1627 = OpIAdd %int %1619 %int_2
       %1628 = OpAccessChain %_ptr_Function_float %506 %1627
       %1629 = OpLoad %float %1628
       %1630 = OpCompositeConstruct %v3float %1623 %1626 %1629
       %1631 = OpFMul %float %1621 %1621
       %1632 = OpCompositeConstruct %v3float %1631 %1621 %float_1
       %1633 = OpMatrixTimesVector %v3float %442 %1630
       %1634 = OpDot %float %1632 %1633
               OpBranch %1581
       %1581 = OpLabel
       %1635 = OpPhi %float %1614 %1589 %1634 %1582
               OpBranch %1573
       %1574 = OpLabel
       %1636 = OpExtInst %float %1 Log %float_9_99999975en05
       %1637 = OpFDiv %float %1636 %1065
               OpBranch %1573
       %1573 = OpLabel
       %1638 = OpPhi %float %1635 %1581 %1637 %1574
       %1639 = OpExtInst %float %1 Pow %float_10 %1638
       %1640 = OpCompositeInsert %v3float %1639 %1566 2
       %1641 = OpVectorTimesMatrix %v3float %1640 %414
       %1642 = OpVectorTimesMatrix %v3float %1641 %410
       %1643 = OpExtInst %float %1 Pow %float_2 %float_n12
       %1644 = OpFMul %float %float_0_179999992 %1643
               OpStore %514 %475
               OpStore %513 %476
       %1645 = OpFOrdLessThanEqual %bool %1644 %float_0
       %1646 = OpSelect %float %1645 %1418 %1644
       %1647 = OpExtInst %float %1 Log %1646
       %1648 = OpFDiv %float %1647 %1065
       %1649 = OpFOrdLessThanEqual %bool %1648 %1423
               OpSelectionMerge %1650 None
               OpBranchConditional %1649 %1651 %1652
       %1652 = OpLabel
       %1653 = OpFOrdGreaterThan %bool %1648 %1423
       %1654 = OpExtInst %float %1 Log %float_0_180000007
       %1655 = OpFDiv %float %1654 %1065
       %1656 = OpFOrdLessThan %bool %1648 %1655
       %1657 = OpLogicalAnd %bool %1653 %1656
               OpSelectionMerge %1658 None
               OpBranchConditional %1657 %1659 %1660
       %1660 = OpLabel
       %1661 = OpFOrdGreaterThanEqual %bool %1648 %1655
       %1662 = OpExtInst %float %1 Log %1416
       %1663 = OpFDiv %float %1662 %1065
       %1664 = OpFOrdLessThan %bool %1648 %1663
       %1665 = OpLogicalAnd %bool %1661 %1664
               OpSelectionMerge %1666 None
               OpBranchConditional %1665 %1667 %1668
       %1668 = OpLabel
       %1669 = OpExtInst %float %1 Log %float_10000
       %1670 = OpFDiv %float %1669 %1065
               OpBranch %1666
       %1667 = OpLabel
       %1671 = OpFSub %float %1648 %1655
       %1672 = OpFMul %float %float_3 %1671
       %1673 = OpFSub %float %1663 %1655
       %1674 = OpFDiv %float %1672 %1673
       %1675 = OpConvertFToS %int %1674
       %1676 = OpConvertSToF %float %1675
       %1677 = OpFSub %float %1674 %1676
       %1678 = OpAccessChain %_ptr_Function_float %513 %1675
       %1679 = OpLoad %float %1678
       %1680 = OpIAdd %int %1675 %int_1
       %1681 = OpAccessChain %_ptr_Function_float %513 %1680
       %1682 = OpLoad %float %1681
       %1683 = OpIAdd %int %1675 %int_2
       %1684 = OpAccessChain %_ptr_Function_float %513 %1683
       %1685 = OpLoad %float %1684
       %1686 = OpCompositeConstruct %v3float %1679 %1682 %1685
       %1687 = OpFMul %float %1677 %1677
       %1688 = OpCompositeConstruct %v3float %1687 %1677 %float_1
       %1689 = OpMatrixTimesVector %v3float %442 %1686
       %1690 = OpDot %float %1688 %1689
               OpBranch %1666
       %1666 = OpLabel
       %1691 = OpPhi %float %1670 %1668 %1690 %1667
               OpBranch %1658
       %1659 = OpLabel
       %1692 = OpFSub %float %1648 %1423
       %1693 = OpFMul %float %float_3 %1692
       %1694 = OpFSub %float %1655 %1423
       %1695 = OpFDiv %float %1693 %1694
       %1696 = OpConvertFToS %int %1695
       %1697 = OpConvertSToF %float %1696
       %1698 = OpFSub %float %1695 %1697
       %1699 = OpAccessChain %_ptr_Function_float %514 %1696
       %1700 = OpLoad %float %1699
       %1701 = OpIAdd %int %1696 %int_1
       %1702 = OpAccessChain %_ptr_Function_float %514 %1701
       %1703 = OpLoad %float %1702
       %1704 = OpIAdd %int %1696 %int_2
       %1705 = OpAccessChain %_ptr_Function_float %514 %1704
       %1706 = OpLoad %float %1705
       %1707 = OpCompositeConstruct %v3float %1700 %1703 %1706
       %1708 = OpFMul %float %1698 %1698
       %1709 = OpCompositeConstruct %v3float %1708 %1698 %float_1
       %1710 = OpMatrixTimesVector %v3float %442 %1707
       %1711 = OpDot %float %1709 %1710
               OpBranch %1658
       %1658 = OpLabel
       %1712 = OpPhi %float %1691 %1666 %1711 %1659
               OpBranch %1650
       %1651 = OpLabel
       %1713 = OpExtInst %float %1 Log %float_9_99999975en05
       %1714 = OpFDiv %float %1713 %1065
               OpBranch %1650
       %1650 = OpLabel
       %1715 = OpPhi %float %1712 %1658 %1714 %1651
       %1716 = OpExtInst %float %1 Pow %float_10 %1715
               OpStore %516 %475
               OpStore %515 %476
       %1717 = OpExtInst %float %1 Log %float_0_180000007
       %1718 = OpFDiv %float %1717 %1065
       %1719 = OpFOrdLessThanEqual %bool %1718 %1423
               OpSelectionMerge %1720 None
               OpBranchConditional %1719 %1721 %1722
       %1722 = OpLabel
       %1723 = OpFOrdGreaterThan %bool %1718 %1423
       %1724 = OpFOrdLessThan %bool %1718 %1718
       %1725 = OpLogicalAnd %bool %1723 %1724
               OpSelectionMerge %1726 None
               OpBranchConditional %1725 %1727 %1728
       %1728 = OpLabel
       %1729 = OpFOrdGreaterThanEqual %bool %1718 %1718
       %1730 = OpExtInst %float %1 Log %1416
       %1731 = OpFDiv %float %1730 %1065
       %1732 = OpFOrdLessThan %bool %1718 %1731
       %1733 = OpLogicalAnd %bool %1729 %1732
               OpSelectionMerge %1734 None
               OpBranchConditional %1733 %1735 %1736
       %1736 = OpLabel
       %1737 = OpExtInst %float %1 Log %float_10000
       %1738 = OpFDiv %float %1737 %1065
               OpBranch %1734
       %1735 = OpLabel
       %1739 = OpFSub %float %1718 %1718
       %1740 = OpFMul %float %float_3 %1739
       %1741 = OpFSub %float %1731 %1718
       %1742 = OpFDiv %float %1740 %1741
       %1743 = OpConvertFToS %int %1742
       %1744 = OpConvertSToF %float %1743
       %1745 = OpFSub %float %1742 %1744
       %1746 = OpAccessChain %_ptr_Function_float %515 %1743
       %1747 = OpLoad %float %1746
       %1748 = OpIAdd %int %1743 %int_1
       %1749 = OpAccessChain %_ptr_Function_float %515 %1748
       %1750 = OpLoad %float %1749
       %1751 = OpIAdd %int %1743 %int_2
       %1752 = OpAccessChain %_ptr_Function_float %515 %1751
       %1753 = OpLoad %float %1752
       %1754 = OpCompositeConstruct %v3float %1747 %1750 %1753
       %1755 = OpFMul %float %1745 %1745
       %1756 = OpCompositeConstruct %v3float %1755 %1745 %float_1
       %1757 = OpMatrixTimesVector %v3float %442 %1754
       %1758 = OpDot %float %1756 %1757
               OpBranch %1734
       %1734 = OpLabel
       %1759 = OpPhi %float %1738 %1736 %1758 %1735
               OpBranch %1726
       %1727 = OpLabel
       %1760 = OpFSub %float %1718 %1423
       %1761 = OpFMul %float %float_3 %1760
       %1762 = OpAccessChain %_ptr_Function_float %516 %int_3
       %1763 = OpLoad %float %1762
       %1764 = OpAccessChain %_ptr_Function_float %516 %int_4
       %1765 = OpLoad %float %1764
       %1766 = OpAccessChain %_ptr_Function_float %516 %int_5
       %1767 = OpLoad %float %1766
       %1768 = OpCompositeConstruct %v3float %1763 %1765 %1767
       %1769 = OpMatrixTimesVector %v3float %442 %1768
       %1770 = OpCompositeExtract %float %1769 2
               OpBranch %1726
       %1726 = OpLabel
       %1771 = OpPhi %float %1759 %1734 %1770 %1727
               OpBranch %1720
       %1721 = OpLabel
       %1772 = OpExtInst %float %1 Log %float_9_99999975en05
       %1773 = OpFDiv %float %1772 %1065
               OpBranch %1720
       %1720 = OpLabel
       %1774 = OpPhi %float %1771 %1726 %1773 %1721
       %1775 = OpExtInst %float %1 Pow %float_10 %1774
       %1776 = OpExtInst %float %1 Pow %float_2 %float_11
       %1777 = OpFMul %float %float_0_179999992 %1776
               OpStore %518 %475
               OpStore %517 %476
       %1778 = OpFOrdLessThanEqual %bool %1777 %float_0
       %1779 = OpSelect %float %1778 %1418 %1777
       %1780 = OpExtInst %float %1 Log %1779
       %1781 = OpFDiv %float %1780 %1065
       %1782 = OpFOrdLessThanEqual %bool %1781 %1423
               OpSelectionMerge %1783 None
               OpBranchConditional %1782 %1784 %1785
       %1785 = OpLabel
       %1786 = OpFOrdGreaterThan %bool %1781 %1423
       %1787 = OpFOrdLessThan %bool %1781 %1718
       %1788 = OpLogicalAnd %bool %1786 %1787
               OpSelectionMerge %1789 None
               OpBranchConditional %1788 %1790 %1791
       %1791 = OpLabel
       %1792 = OpFOrdGreaterThanEqual %bool %1781 %1718
       %1793 = OpExtInst %float %1 Log %1416
       %1794 = OpFDiv %float %1793 %1065
       %1795 = OpFOrdLessThan %bool %1781 %1794
       %1796 = OpLogicalAnd %bool %1792 %1795
               OpSelectionMerge %1797 None
               OpBranchConditional %1796 %1798 %1799
       %1799 = OpLabel
       %1800 = OpExtInst %float %1 Log %float_10000
       %1801 = OpFDiv %float %1800 %1065
               OpBranch %1797
       %1798 = OpLabel
       %1802 = OpFSub %float %1781 %1718
       %1803 = OpFMul %float %float_3 %1802
       %1804 = OpFSub %float %1794 %1718
       %1805 = OpFDiv %float %1803 %1804
       %1806 = OpConvertFToS %int %1805
       %1807 = OpConvertSToF %float %1806
       %1808 = OpFSub %float %1805 %1807
       %1809 = OpAccessChain %_ptr_Function_float %517 %1806
       %1810 = OpLoad %float %1809
       %1811 = OpIAdd %int %1806 %int_1
       %1812 = OpAccessChain %_ptr_Function_float %517 %1811
       %1813 = OpLoad %float %1812
       %1814 = OpIAdd %int %1806 %int_2
       %1815 = OpAccessChain %_ptr_Function_float %517 %1814
       %1816 = OpLoad %float %1815
       %1817 = OpCompositeConstruct %v3float %1810 %1813 %1816
       %1818 = OpFMul %float %1808 %1808
       %1819 = OpCompositeConstruct %v3float %1818 %1808 %float_1
       %1820 = OpMatrixTimesVector %v3float %442 %1817
       %1821 = OpDot %float %1819 %1820
               OpBranch %1797
       %1797 = OpLabel
       %1822 = OpPhi %float %1801 %1799 %1821 %1798
               OpBranch %1789
       %1790 = OpLabel
       %1823 = OpFSub %float %1781 %1423
       %1824 = OpFMul %float %float_3 %1823
       %1825 = OpFSub %float %1718 %1423
       %1826 = OpFDiv %float %1824 %1825
       %1827 = OpConvertFToS %int %1826
       %1828 = OpConvertSToF %float %1827
       %1829 = OpFSub %float %1826 %1828
       %1830 = OpAccessChain %_ptr_Function_float %518 %1827
       %1831 = OpLoad %float %1830
       %1832 = OpIAdd %int %1827 %int_1
       %1833 = OpAccessChain %_ptr_Function_float %518 %1832
       %1834 = OpLoad %float %1833
       %1835 = OpIAdd %int %1827 %int_2
       %1836 = OpAccessChain %_ptr_Function_float %518 %1835
       %1837 = OpLoad %float %1836
       %1838 = OpCompositeConstruct %v3float %1831 %1834 %1837
       %1839 = OpFMul %float %1829 %1829
       %1840 = OpCompositeConstruct %v3float %1839 %1829 %float_1
       %1841 = OpMatrixTimesVector %v3float %442 %1838
       %1842 = OpDot %float %1840 %1841
               OpBranch %1789
       %1789 = OpLabel
       %1843 = OpPhi %float %1822 %1797 %1842 %1790
               OpBranch %1783
       %1784 = OpLabel
       %1844 = OpExtInst %float %1 Log %float_9_99999975en05
       %1845 = OpFDiv %float %1844 %1065
               OpBranch %1783
       %1783 = OpLabel
       %1846 = OpPhi %float %1843 %1789 %1845 %1784
       %1847 = OpExtInst %float %1 Pow %float_10 %1846
       %1848 = OpCompositeExtract %float %1642 0
               OpStore %512 %482
               OpStore %511 %483
       %1849 = OpFOrdLessThanEqual %bool %1848 %float_0
       %1850 = OpSelect %float %1849 %float_9_99999975en05 %1848
       %1851 = OpExtInst %float %1 Log %1850
       %1852 = OpFDiv %float %1851 %1065
       %1853 = OpExtInst %float %1 Log %1716
       %1854 = OpFDiv %float %1853 %1065
       %1855 = OpFOrdLessThanEqual %bool %1852 %1854
               OpSelectionMerge %1856 None
               OpBranchConditional %1855 %1857 %1858
       %1858 = OpLabel
       %1859 = OpFOrdGreaterThan %bool %1852 %1854
       %1860 = OpExtInst %float %1 Log %1775
       %1861 = OpFDiv %float %1860 %1065
       %1862 = OpFOrdLessThan %bool %1852 %1861
       %1863 = OpLogicalAnd %bool %1859 %1862
               OpSelectionMerge %1864 None
               OpBranchConditional %1863 %1865 %1866
       %1866 = OpLabel
       %1867 = OpFOrdGreaterThanEqual %bool %1852 %1861
       %1868 = OpExtInst %float %1 Log %1847
       %1869 = OpFDiv %float %1868 %1065
       %1870 = OpFOrdLessThan %bool %1852 %1869
       %1871 = OpLogicalAnd %bool %1867 %1870
               OpSelectionMerge %1872 None
               OpBranchConditional %1871 %1873 %1874
       %1874 = OpLabel
       %1875 = OpFMul %float %1852 %float_0_119999997
       %1876 = OpExtInst %float %1 Log %float_2000
       %1877 = OpFDiv %float %1876 %1065
       %1878 = OpFMul %float %float_0_119999997 %1868
       %1879 = OpFDiv %float %1878 %1065
       %1880 = OpFSub %float %1877 %1879
       %1881 = OpFAdd %float %1875 %1880
               OpBranch %1872
       %1873 = OpLabel
       %1882 = OpFSub %float %1852 %1861
       %1883 = OpFMul %float %float_7 %1882
       %1884 = OpFSub %float %1869 %1861
       %1885 = OpFDiv %float %1883 %1884
       %1886 = OpConvertFToS %int %1885
       %1887 = OpConvertSToF %float %1886
       %1888 = OpFSub %float %1885 %1887
       %1889 = OpAccessChain %_ptr_Function_float %511 %1886
       %1890 = OpLoad %float %1889
       %1891 = OpIAdd %int %1886 %int_1
       %1892 = OpAccessChain %_ptr_Function_float %511 %1891
       %1893 = OpLoad %float %1892
       %1894 = OpIAdd %int %1886 %int_2
       %1895 = OpAccessChain %_ptr_Function_float %511 %1894
       %1896 = OpLoad %float %1895
       %1897 = OpCompositeConstruct %v3float %1890 %1893 %1896
       %1898 = OpFMul %float %1888 %1888
       %1899 = OpCompositeConstruct %v3float %1898 %1888 %float_1
       %1900 = OpMatrixTimesVector %v3float %442 %1897
       %1901 = OpDot %float %1899 %1900
               OpBranch %1872
       %1872 = OpLabel
       %1902 = OpPhi %float %1881 %1874 %1901 %1873
               OpBranch %1864
       %1865 = OpLabel
       %1903 = OpFSub %float %1852 %1854
       %1904 = OpFMul %float %float_7 %1903
       %1905 = OpFSub %float %1861 %1854
       %1906 = OpFDiv %float %1904 %1905
       %1907 = OpConvertFToS %int %1906
       %1908 = OpConvertSToF %float %1907
       %1909 = OpFSub %float %1906 %1908
       %1910 = OpAccessChain %_ptr_Function_float %512 %1907
       %1911 = OpLoad %float %1910
       %1912 = OpIAdd %int %1907 %int_1
       %1913 = OpAccessChain %_ptr_Function_float %512 %1912
       %1914 = OpLoad %float %1913
       %1915 = OpIAdd %int %1907 %int_2
       %1916 = OpAccessChain %_ptr_Function_float %512 %1915
       %1917 = OpLoad %float %1916
       %1918 = OpCompositeConstruct %v3float %1911 %1914 %1917
       %1919 = OpFMul %float %1909 %1909
       %1920 = OpCompositeConstruct %v3float %1919 %1909 %float_1
       %1921 = OpMatrixTimesVector %v3float %442 %1918
       %1922 = OpDot %float %1920 %1921
               OpBranch %1864
       %1864 = OpLabel
       %1923 = OpPhi %float %1902 %1872 %1922 %1865
               OpBranch %1856
       %1857 = OpLabel
       %1924 = OpExtInst %float %1 Log %float_0_00499999989
       %1925 = OpFDiv %float %1924 %1065
               OpBranch %1856
       %1856 = OpLabel
       %1926 = OpPhi %float %1923 %1864 %1925 %1857
       %1927 = OpExtInst %float %1 Pow %float_10 %1926
       %1928 = OpCompositeInsert %v3float %1927 %391 0
       %1929 = OpCompositeExtract %float %1642 1
               OpStore %510 %482
               OpStore %509 %483
       %1930 = OpFOrdLessThanEqual %bool %1929 %float_0
       %1931 = OpSelect %float %1930 %float_9_99999975en05 %1929
       %1932 = OpExtInst %float %1 Log %1931
       %1933 = OpFDiv %float %1932 %1065
       %1934 = OpFOrdLessThanEqual %bool %1933 %1854
               OpSelectionMerge %1935 None
               OpBranchConditional %1934 %1936 %1937
       %1937 = OpLabel
       %1938 = OpFOrdGreaterThan %bool %1933 %1854
       %1939 = OpExtInst %float %1 Log %1775
       %1940 = OpFDiv %float %1939 %1065
       %1941 = OpFOrdLessThan %bool %1933 %1940
       %1942 = OpLogicalAnd %bool %1938 %1941
               OpSelectionMerge %1943 None
               OpBranchConditional %1942 %1944 %1945
       %1945 = OpLabel
       %1946 = OpFOrdGreaterThanEqual %bool %1933 %1940
       %1947 = OpExtInst %float %1 Log %1847
       %1948 = OpFDiv %float %1947 %1065
       %1949 = OpFOrdLessThan %bool %1933 %1948
       %1950 = OpLogicalAnd %bool %1946 %1949
               OpSelectionMerge %1951 None
               OpBranchConditional %1950 %1952 %1953
       %1953 = OpLabel
       %1954 = OpFMul %float %1933 %float_0_119999997
       %1955 = OpExtInst %float %1 Log %float_2000
       %1956 = OpFDiv %float %1955 %1065
       %1957 = OpFMul %float %float_0_119999997 %1947
       %1958 = OpFDiv %float %1957 %1065
       %1959 = OpFSub %float %1956 %1958
       %1960 = OpFAdd %float %1954 %1959
               OpBranch %1951
       %1952 = OpLabel
       %1961 = OpFSub %float %1933 %1940
       %1962 = OpFMul %float %float_7 %1961
       %1963 = OpFSub %float %1948 %1940
       %1964 = OpFDiv %float %1962 %1963
       %1965 = OpConvertFToS %int %1964
       %1966 = OpConvertSToF %float %1965
       %1967 = OpFSub %float %1964 %1966
       %1968 = OpAccessChain %_ptr_Function_float %509 %1965
       %1969 = OpLoad %float %1968
       %1970 = OpIAdd %int %1965 %int_1
       %1971 = OpAccessChain %_ptr_Function_float %509 %1970
       %1972 = OpLoad %float %1971
       %1973 = OpIAdd %int %1965 %int_2
       %1974 = OpAccessChain %_ptr_Function_float %509 %1973
       %1975 = OpLoad %float %1974
       %1976 = OpCompositeConstruct %v3float %1969 %1972 %1975
       %1977 = OpFMul %float %1967 %1967
       %1978 = OpCompositeConstruct %v3float %1977 %1967 %float_1
       %1979 = OpMatrixTimesVector %v3float %442 %1976
       %1980 = OpDot %float %1978 %1979
               OpBranch %1951
       %1951 = OpLabel
       %1981 = OpPhi %float %1960 %1953 %1980 %1952
               OpBranch %1943
       %1944 = OpLabel
       %1982 = OpFSub %float %1933 %1854
       %1983 = OpFMul %float %float_7 %1982
       %1984 = OpFSub %float %1940 %1854
       %1985 = OpFDiv %float %1983 %1984
       %1986 = OpConvertFToS %int %1985
       %1987 = OpConvertSToF %float %1986
       %1988 = OpFSub %float %1985 %1987
       %1989 = OpAccessChain %_ptr_Function_float %510 %1986
       %1990 = OpLoad %float %1989
       %1991 = OpIAdd %int %1986 %int_1
       %1992 = OpAccessChain %_ptr_Function_float %510 %1991
       %1993 = OpLoad %float %1992
       %1994 = OpIAdd %int %1986 %int_2
       %1995 = OpAccessChain %_ptr_Function_float %510 %1994
       %1996 = OpLoad %float %1995
       %1997 = OpCompositeConstruct %v3float %1990 %1993 %1996
       %1998 = OpFMul %float %1988 %1988
       %1999 = OpCompositeConstruct %v3float %1998 %1988 %float_1
       %2000 = OpMatrixTimesVector %v3float %442 %1997
       %2001 = OpDot %float %1999 %2000
               OpBranch %1943
       %1943 = OpLabel
       %2002 = OpPhi %float %1981 %1951 %2001 %1944
               OpBranch %1935
       %1936 = OpLabel
       %2003 = OpExtInst %float %1 Log %float_0_00499999989
       %2004 = OpFDiv %float %2003 %1065
               OpBranch %1935
       %1935 = OpLabel
       %2005 = OpPhi %float %2002 %1943 %2004 %1936
       %2006 = OpExtInst %float %1 Pow %float_10 %2005
       %2007 = OpCompositeInsert %v3float %2006 %1928 1
       %2008 = OpCompositeExtract %float %1642 2
               OpStore %508 %482
               OpStore %507 %483
       %2009 = OpFOrdLessThanEqual %bool %2008 %float_0
       %2010 = OpSelect %float %2009 %float_9_99999975en05 %2008
       %2011 = OpExtInst %float %1 Log %2010
       %2012 = OpFDiv %float %2011 %1065
       %2013 = OpFOrdLessThanEqual %bool %2012 %1854
               OpSelectionMerge %2014 None
               OpBranchConditional %2013 %2015 %2016
       %2016 = OpLabel
       %2017 = OpFOrdGreaterThan %bool %2012 %1854
       %2018 = OpExtInst %float %1 Log %1775
       %2019 = OpFDiv %float %2018 %1065
       %2020 = OpFOrdLessThan %bool %2012 %2019
       %2021 = OpLogicalAnd %bool %2017 %2020
               OpSelectionMerge %2022 None
               OpBranchConditional %2021 %2023 %2024
       %2024 = OpLabel
       %2025 = OpFOrdGreaterThanEqual %bool %2012 %2019
       %2026 = OpExtInst %float %1 Log %1847
       %2027 = OpFDiv %float %2026 %1065
       %2028 = OpFOrdLessThan %bool %2012 %2027
       %2029 = OpLogicalAnd %bool %2025 %2028
               OpSelectionMerge %2030 None
               OpBranchConditional %2029 %2031 %2032
       %2032 = OpLabel
       %2033 = OpFMul %float %2012 %float_0_119999997
       %2034 = OpExtInst %float %1 Log %float_2000
       %2035 = OpFDiv %float %2034 %1065
       %2036 = OpFMul %float %float_0_119999997 %2026
       %2037 = OpFDiv %float %2036 %1065
       %2038 = OpFSub %float %2035 %2037
       %2039 = OpFAdd %float %2033 %2038
               OpBranch %2030
       %2031 = OpLabel
       %2040 = OpFSub %float %2012 %2019
       %2041 = OpFMul %float %float_7 %2040
       %2042 = OpFSub %float %2027 %2019
       %2043 = OpFDiv %float %2041 %2042
       %2044 = OpConvertFToS %int %2043
       %2045 = OpConvertSToF %float %2044
       %2046 = OpFSub %float %2043 %2045
       %2047 = OpAccessChain %_ptr_Function_float %507 %2044
       %2048 = OpLoad %float %2047
       %2049 = OpIAdd %int %2044 %int_1
       %2050 = OpAccessChain %_ptr_Function_float %507 %2049
       %2051 = OpLoad %float %2050
       %2052 = OpIAdd %int %2044 %int_2
       %2053 = OpAccessChain %_ptr_Function_float %507 %2052
       %2054 = OpLoad %float %2053
       %2055 = OpCompositeConstruct %v3float %2048 %2051 %2054
       %2056 = OpFMul %float %2046 %2046
       %2057 = OpCompositeConstruct %v3float %2056 %2046 %float_1
       %2058 = OpMatrixTimesVector %v3float %442 %2055
       %2059 = OpDot %float %2057 %2058
               OpBranch %2030
       %2030 = OpLabel
       %2060 = OpPhi %float %2039 %2032 %2059 %2031
               OpBranch %2022
       %2023 = OpLabel
       %2061 = OpFSub %float %2012 %1854
       %2062 = OpFMul %float %float_7 %2061
       %2063 = OpFSub %float %2019 %1854
       %2064 = OpFDiv %float %2062 %2063
       %2065 = OpConvertFToS %int %2064
       %2066 = OpConvertSToF %float %2065
       %2067 = OpFSub %float %2064 %2066
       %2068 = OpAccessChain %_ptr_Function_float %508 %2065
       %2069 = OpLoad %float %2068
       %2070 = OpIAdd %int %2065 %int_1
       %2071 = OpAccessChain %_ptr_Function_float %508 %2070
       %2072 = OpLoad %float %2071
       %2073 = OpIAdd %int %2065 %int_2
       %2074 = OpAccessChain %_ptr_Function_float %508 %2073
       %2075 = OpLoad %float %2074
       %2076 = OpCompositeConstruct %v3float %2069 %2072 %2075
       %2077 = OpFMul %float %2067 %2067
       %2078 = OpCompositeConstruct %v3float %2077 %2067 %float_1
       %2079 = OpMatrixTimesVector %v3float %442 %2076
       %2080 = OpDot %float %2078 %2079
               OpBranch %2022
       %2022 = OpLabel
       %2081 = OpPhi %float %2060 %2030 %2080 %2023
               OpBranch %2014
       %2015 = OpLabel
       %2082 = OpExtInst %float %1 Log %float_0_00499999989
       %2083 = OpFDiv %float %2082 %1065
               OpBranch %2014
       %2014 = OpLabel
       %2084 = OpPhi %float %2081 %2022 %2083 %2015
       %2085 = OpExtInst %float %1 Pow %float_10 %2084
       %2086 = OpCompositeInsert %v3float %2085 %2007 2
       %2087 = OpVectorTimesMatrix %v3float %2086 %576
       %2088 = OpFMul %v3float %2087 %496
       %2089 = OpExtInst %v3float %1 Pow %2088 %263
       %2090 = OpFMul %v3float %184 %2089
       %2091 = OpFAdd %v3float %183 %2090
       %2092 = OpFMul %v3float %185 %2089
       %2093 = OpFAdd %v3float %135 %2092
       %2094 = OpFDiv %v3float %135 %2093
       %2095 = OpFMul %v3float %2091 %2094
       %2096 = OpExtInst %v3float %1 Pow %2095 %264
               OpBranch %1236
       %1236 = OpLabel
       %2097 = OpPhi %v3float %1260 %1240 %2096 %2014
               OpBranch %1230
       %1231 = OpLabel
       %2098 = OpMatrixTimesMatrix %mat3v3float %546 %399
       %2099 = OpFMul %v3float %906 %262
       %2100 = OpVectorTimesMatrix %v3float %2099 %2098
       %2101 = OpCompositeExtract %float %2100 0
       %2102 = OpCompositeExtract %float %2100 1
       %2103 = OpExtInst %float %1 FMin %2101 %2102
       %2104 = OpCompositeExtract %float %2100 2
       %2105 = OpExtInst %float %1 FMin %2103 %2104
       %2106 = OpExtInst %float %1 FMax %2101 %2102
       %2107 = OpExtInst %float %1 FMax %2106 %2104
       %2108 = OpExtInst %float %1 FMax %2107 %float_1_00000001en10
       %2109 = OpExtInst %float %1 FMax %2105 %float_1_00000001en10
       %2110 = OpFSub %float %2108 %2109
       %2111 = OpExtInst %float %1 FMax %2107 %float_0_00999999978
       %2112 = OpFDiv %float %2110 %2111
       %2113 = OpFSub %float %2104 %2102
       %2114 = OpFMul %float %2104 %2113
       %2115 = OpFSub %float %2102 %2101
       %2116 = OpFMul %float %2102 %2115
       %2117 = OpFAdd %float %2114 %2116
       %2118 = OpFSub %float %2101 %2104
       %2119 = OpFMul %float %2101 %2118
       %2120 = OpFAdd %float %2117 %2119
       %2121 = OpExtInst %float %1 Sqrt %2120
       %2122 = OpFAdd %float %2104 %2102
       %2123 = OpFAdd %float %2122 %2101
       %2124 = OpFMul %float %float_1_75 %2121
       %2125 = OpFAdd %float %2123 %2124
       %2126 = OpFMul %float %2125 %float_0_333333343
       %2127 = OpFSub %float %2112 %float_0_400000006
       %2128 = OpFMul %float %2127 %float_5
       %2129 = OpFMul %float %2127 %float_2_5
       %2130 = OpExtInst %float %1 FAbs %2129
       %2131 = OpFSub %float %float_1 %2130
       %2132 = OpExtInst %float %1 FMax %2131 %float_0
       %2133 = OpExtInst %float %1 FSign %2128
       %2134 = OpConvertFToS %int %2133
       %2135 = OpConvertSToF %float %2134
       %2136 = OpFMul %float %2132 %2132
       %2137 = OpFSub %float %float_1 %2136
       %2138 = OpFMul %float %2135 %2137
       %2139 = OpFAdd %float %float_1 %2138
       %2140 = OpFMul %float %2139 %float_0_0250000004
       %2141 = OpFOrdLessThanEqual %bool %2126 %float_0_0533333346
               OpSelectionMerge %2142 None
               OpBranchConditional %2141 %2143 %2144
       %2144 = OpLabel
       %2145 = OpFOrdGreaterThanEqual %bool %2126 %float_0_159999996
               OpSelectionMerge %2146 None
               OpBranchConditional %2145 %2147 %2148
       %2148 = OpLabel
       %2149 = OpFDiv %float %float_0_239999995 %2125
       %2150 = OpFSub %float %2149 %float_0_5
       %2151 = OpFMul %float %2140 %2150
               OpBranch %2146
       %2147 = OpLabel
               OpBranch %2146
       %2146 = OpLabel
       %2152 = OpPhi %float %2151 %2148 %float_0 %2147
               OpBranch %2142
       %2143 = OpLabel
               OpBranch %2142
       %2142 = OpLabel
       %2153 = OpPhi %float %2152 %2146 %2140 %2143
       %2154 = OpFAdd %float %float_1 %2153
       %2155 = OpCompositeConstruct %v3float %2154 %2154 %2154
       %2156 = OpFMul %v3float %2100 %2155
       %2157 = OpCompositeExtract %float %2156 0
       %2158 = OpCompositeExtract %float %2156 1
       %2159 = OpFOrdEqual %bool %2157 %2158
       %2160 = OpCompositeExtract %float %2156 2
       %2161 = OpFOrdEqual %bool %2158 %2160
       %2162 = OpLogicalAnd %bool %2159 %2161
               OpSelectionMerge %2163 None
               OpBranchConditional %2162 %2164 %2165
       %2165 = OpLabel
       %2166 = OpExtInst %float %1 Sqrt %float_3
       %2167 = OpFSub %float %2158 %2160
       %2168 = OpFMul %float %2166 %2167
       %2169 = OpFMul %float %float_2 %2157
       %2170 = OpFSub %float %2169 %2158
       %2171 = OpFSub %float %2170 %2160
       %2172 = OpExtInst %float %1 Atan2 %2168 %2171
       %2173 = OpFMul %float %float_57_2957764 %2172
               OpBranch %2163
       %2164 = OpLabel
               OpBranch %2163
       %2163 = OpLabel
       %2174 = OpPhi %float %2173 %2165 %float_0 %2164
       %2175 = OpFOrdLessThan %bool %2174 %float_0
               OpSelectionMerge %2176 None
               OpBranchConditional %2175 %2177 %2176
       %2177 = OpLabel
       %2178 = OpFAdd %float %2174 %float_360
               OpBranch %2176
       %2176 = OpLabel
       %2179 = OpPhi %float %2174 %2163 %2178 %2177
       %2180 = OpExtInst %float %1 FClamp %2179 %float_0 %float_360
       %2181 = OpFOrdGreaterThan %bool %2180 %float_180
               OpSelectionMerge %2182 None
               OpBranchConditional %2181 %2183 %2182
       %2183 = OpLabel
       %2184 = OpFSub %float %2180 %float_360
               OpBranch %2182
       %2182 = OpLabel
       %2185 = OpPhi %float %2180 %2176 %2184 %2183
       %2186 = OpFOrdGreaterThan %bool %2185 %float_n67_5
       %2187 = OpFOrdLessThan %bool %2185 %float_67_5
       %2188 = OpLogicalAnd %bool %2186 %2187
               OpSelectionMerge %2189 None
               OpBranchConditional %2188 %2190 %2189
       %2190 = OpLabel
       %2191 = OpFSub %float %2185 %float_n67_5
       %2192 = OpFMul %float %2191 %float_0_0296296291
       %2193 = OpConvertFToS %int %2192
       %2194 = OpConvertSToF %float %2193
       %2195 = OpFSub %float %2192 %2194
       %2196 = OpFMul %float %2195 %2195
       %2197 = OpFMul %float %2196 %2195
       %2198 = OpIEqual %bool %2193 %int_3
               OpSelectionMerge %2199 None
               OpBranchConditional %2198 %2200 %2201
       %2201 = OpLabel
       %2202 = OpIEqual %bool %2193 %int_2
               OpSelectionMerge %2203 None
               OpBranchConditional %2202 %2204 %2205
       %2205 = OpLabel
       %2206 = OpIEqual %bool %2193 %int_1
               OpSelectionMerge %2207 None
               OpBranchConditional %2206 %2208 %2209
       %2209 = OpLabel
       %2210 = OpIEqual %bool %2193 %int_0
               OpSelectionMerge %2211 None
               OpBranchConditional %2210 %2212 %2213
       %2213 = OpLabel
               OpBranch %2211
       %2212 = OpLabel
       %2214 = OpFMul %float %2197 %float_0_166666672
               OpBranch %2211
       %2211 = OpLabel
       %2215 = OpPhi %float %float_0 %2213 %2214 %2212
               OpBranch %2207
       %2208 = OpLabel
       %2216 = OpFMul %float %2197 %float_n0_5
       %2217 = OpFMul %float %2196 %float_0_5
       %2218 = OpFAdd %float %2216 %2217
       %2219 = OpFMul %float %2195 %float_0_5
       %2220 = OpFAdd %float %2218 %2219
       %2221 = OpFAdd %float %2220 %float_0_166666672
               OpBranch %2207
       %2207 = OpLabel
       %2222 = OpPhi %float %2215 %2211 %2221 %2208
               OpBranch %2203
       %2204 = OpLabel
       %2223 = OpFMul %float %2197 %float_0_5
       %2224 = OpFMul %float %2196 %float_n1
       %2225 = OpFAdd %float %2223 %2224
       %2226 = OpFAdd %float %2225 %float_0_666666687
               OpBranch %2203
       %2203 = OpLabel
       %2227 = OpPhi %float %2222 %2207 %2226 %2204
               OpBranch %2199
       %2200 = OpLabel
       %2228 = OpFMul %float %2197 %float_n0_166666672
       %2229 = OpFMul %float %2196 %float_0_5
       %2230 = OpFAdd %float %2228 %2229
       %2231 = OpFMul %float %2195 %float_n0_5
       %2232 = OpFAdd %float %2230 %2231
       %2233 = OpFAdd %float %2232 %float_0_166666672
               OpBranch %2199
       %2199 = OpLabel
       %2234 = OpPhi %float %2227 %2203 %2233 %2200
               OpBranch %2189
       %2189 = OpLabel
       %2235 = OpPhi %float %float_0 %2182 %2234 %2199
       %2236 = OpFMul %float %2235 %float_1_5
       %2237 = OpFMul %float %2236 %2112
       %2238 = OpFSub %float %float_0_0299999993 %2157
       %2239 = OpFMul %float %2237 %2238
       %2240 = OpFMul %float %2239 %float_0_180000007
       %2241 = OpFAdd %float %2157 %2240
       %2242 = OpCompositeInsert %v3float %2241 %2156 0
       %2243 = OpExtInst %v3float %1 FClamp %2242 %132 %314
       %2244 = OpVectorTimesMatrix %v3float %2243 %410
       %2245 = OpExtInst %v3float %1 FClamp %2244 %132 %314
       %2246 = OpDot %float %2245 %67
       %2247 = OpCompositeConstruct %v3float %2246 %2246 %2246
       %2248 = OpExtInst %v3float %1 FMix %2247 %2245 %228
       %2249 = OpCompositeExtract %float %2248 0
       %2250 = OpExtInst %float %1 Exp2 %float_n15
       %2251 = OpFMul %float %float_0_179999992 %2250
       %2252 = OpExtInst %float %1 Exp2 %float_18
       %2253 = OpFMul %float %float_0_179999992 %2252
               OpStore %520 %475
               OpStore %519 %476
       %2254 = OpFOrdLessThanEqual %bool %2249 %float_0
       %2255 = OpExtInst %float %1 Exp2 %float_n14
       %2256 = OpSelect %float %2254 %2255 %2249
       %2257 = OpExtInst %float %1 Log %2256
       %2258 = OpFDiv %float %2257 %1065
       %2259 = OpExtInst %float %1 Log %2251
       %2260 = OpFDiv %float %2259 %1065
       %2261 = OpFOrdLessThanEqual %bool %2258 %2260
               OpSelectionMerge %2262 None
               OpBranchConditional %2261 %2263 %2264
       %2264 = OpLabel
       %2265 = OpFOrdGreaterThan %bool %2258 %2260
       %2266 = OpExtInst %float %1 Log %float_0_180000007
       %2267 = OpFDiv %float %2266 %1065
       %2268 = OpFOrdLessThan %bool %2258 %2267
       %2269 = OpLogicalAnd %bool %2265 %2268
               OpSelectionMerge %2270 None
               OpBranchConditional %2269 %2271 %2272
       %2272 = OpLabel
       %2273 = OpFOrdGreaterThanEqual %bool %2258 %2267
       %2274 = OpExtInst %float %1 Log %2253
       %2275 = OpFDiv %float %2274 %1065
       %2276 = OpFOrdLessThan %bool %2258 %2275
       %2277 = OpLogicalAnd %bool %2273 %2276
               OpSelectionMerge %2278 None
               OpBranchConditional %2277 %2279 %2280
       %2280 = OpLabel
       %2281 = OpExtInst %float %1 Log %float_10000
       %2282 = OpFDiv %float %2281 %1065
               OpBranch %2278
       %2279 = OpLabel
       %2283 = OpFSub %float %2258 %2267
       %2284 = OpFMul %float %float_3 %2283
       %2285 = OpFSub %float %2275 %2267
       %2286 = OpFDiv %float %2284 %2285
       %2287 = OpConvertFToS %int %2286
       %2288 = OpConvertSToF %float %2287
       %2289 = OpFSub %float %2286 %2288
       %2290 = OpAccessChain %_ptr_Function_float %519 %2287
       %2291 = OpLoad %float %2290
       %2292 = OpIAdd %int %2287 %int_1
       %2293 = OpAccessChain %_ptr_Function_float %519 %2292
       %2294 = OpLoad %float %2293
       %2295 = OpIAdd %int %2287 %int_2
       %2296 = OpAccessChain %_ptr_Function_float %519 %2295
       %2297 = OpLoad %float %2296
       %2298 = OpCompositeConstruct %v3float %2291 %2294 %2297
       %2299 = OpFMul %float %2289 %2289
       %2300 = OpCompositeConstruct %v3float %2299 %2289 %float_1
       %2301 = OpMatrixTimesVector %v3float %442 %2298
       %2302 = OpDot %float %2300 %2301
               OpBranch %2278
       %2278 = OpLabel
       %2303 = OpPhi %float %2282 %2280 %2302 %2279
               OpBranch %2270
       %2271 = OpLabel
       %2304 = OpFSub %float %2258 %2260
       %2305 = OpFMul %float %float_3 %2304
       %2306 = OpFSub %float %2267 %2260
       %2307 = OpFDiv %float %2305 %2306
       %2308 = OpConvertFToS %int %2307
       %2309 = OpConvertSToF %float %2308
       %2310 = OpFSub %float %2307 %2309
       %2311 = OpAccessChain %_ptr_Function_float %520 %2308
       %2312 = OpLoad %float %2311
       %2313 = OpIAdd %int %2308 %int_1
       %2314 = OpAccessChain %_ptr_Function_float %520 %2313
       %2315 = OpLoad %float %2314
       %2316 = OpIAdd %int %2308 %int_2
       %2317 = OpAccessChain %_ptr_Function_float %520 %2316
       %2318 = OpLoad %float %2317
       %2319 = OpCompositeConstruct %v3float %2312 %2315 %2318
       %2320 = OpFMul %float %2310 %2310
       %2321 = OpCompositeConstruct %v3float %2320 %2310 %float_1
       %2322 = OpMatrixTimesVector %v3float %442 %2319
       %2323 = OpDot %float %2321 %2322
               OpBranch %2270
       %2270 = OpLabel
       %2324 = OpPhi %float %2303 %2278 %2323 %2271
               OpBranch %2262
       %2263 = OpLabel
       %2325 = OpExtInst %float %1 Log %float_9_99999975en05
       %2326 = OpFDiv %float %2325 %1065
               OpBranch %2262
       %2262 = OpLabel
       %2327 = OpPhi %float %2324 %2270 %2326 %2263
       %2328 = OpExtInst %float %1 Pow %float_10 %2327
       %2329 = OpCompositeInsert %v3float %2328 %391 0
       %2330 = OpCompositeExtract %float %2248 1
               OpStore %522 %475
               OpStore %521 %476
       %2331 = OpFOrdLessThanEqual %bool %2330 %float_0
       %2332 = OpSelect %float %2331 %2255 %2330
       %2333 = OpExtInst %float %1 Log %2332
       %2334 = OpFDiv %float %2333 %1065
       %2335 = OpFOrdLessThanEqual %bool %2334 %2260
               OpSelectionMerge %2336 None
               OpBranchConditional %2335 %2337 %2338
       %2338 = OpLabel
       %2339 = OpFOrdGreaterThan %bool %2334 %2260
       %2340 = OpExtInst %float %1 Log %float_0_180000007
       %2341 = OpFDiv %float %2340 %1065
       %2342 = OpFOrdLessThan %bool %2334 %2341
       %2343 = OpLogicalAnd %bool %2339 %2342
               OpSelectionMerge %2344 None
               OpBranchConditional %2343 %2345 %2346
       %2346 = OpLabel
       %2347 = OpFOrdGreaterThanEqual %bool %2334 %2341
       %2348 = OpExtInst %float %1 Log %2253
       %2349 = OpFDiv %float %2348 %1065
       %2350 = OpFOrdLessThan %bool %2334 %2349
       %2351 = OpLogicalAnd %bool %2347 %2350
               OpSelectionMerge %2352 None
               OpBranchConditional %2351 %2353 %2354
       %2354 = OpLabel
       %2355 = OpExtInst %float %1 Log %float_10000
       %2356 = OpFDiv %float %2355 %1065
               OpBranch %2352
       %2353 = OpLabel
       %2357 = OpFSub %float %2334 %2341
       %2358 = OpFMul %float %float_3 %2357
       %2359 = OpFSub %float %2349 %2341
       %2360 = OpFDiv %float %2358 %2359
       %2361 = OpConvertFToS %int %2360
       %2362 = OpConvertSToF %float %2361
       %2363 = OpFSub %float %2360 %2362
       %2364 = OpAccessChain %_ptr_Function_float %521 %2361
       %2365 = OpLoad %float %2364
       %2366 = OpIAdd %int %2361 %int_1
       %2367 = OpAccessChain %_ptr_Function_float %521 %2366
       %2368 = OpLoad %float %2367
       %2369 = OpIAdd %int %2361 %int_2
       %2370 = OpAccessChain %_ptr_Function_float %521 %2369
       %2371 = OpLoad %float %2370
       %2372 = OpCompositeConstruct %v3float %2365 %2368 %2371
       %2373 = OpFMul %float %2363 %2363
       %2374 = OpCompositeConstruct %v3float %2373 %2363 %float_1
       %2375 = OpMatrixTimesVector %v3float %442 %2372
       %2376 = OpDot %float %2374 %2375
               OpBranch %2352
       %2352 = OpLabel
       %2377 = OpPhi %float %2356 %2354 %2376 %2353
               OpBranch %2344
       %2345 = OpLabel
       %2378 = OpFSub %float %2334 %2260
       %2379 = OpFMul %float %float_3 %2378
       %2380 = OpFSub %float %2341 %2260
       %2381 = OpFDiv %float %2379 %2380
       %2382 = OpConvertFToS %int %2381
       %2383 = OpConvertSToF %float %2382
       %2384 = OpFSub %float %2381 %2383
       %2385 = OpAccessChain %_ptr_Function_float %522 %2382
       %2386 = OpLoad %float %2385
       %2387 = OpIAdd %int %2382 %int_1
       %2388 = OpAccessChain %_ptr_Function_float %522 %2387
       %2389 = OpLoad %float %2388
       %2390 = OpIAdd %int %2382 %int_2
       %2391 = OpAccessChain %_ptr_Function_float %522 %2390
       %2392 = OpLoad %float %2391
       %2393 = OpCompositeConstruct %v3float %2386 %2389 %2392
       %2394 = OpFMul %float %2384 %2384
       %2395 = OpCompositeConstruct %v3float %2394 %2384 %float_1
       %2396 = OpMatrixTimesVector %v3float %442 %2393
       %2397 = OpDot %float %2395 %2396
               OpBranch %2344
       %2344 = OpLabel
       %2398 = OpPhi %float %2377 %2352 %2397 %2345
               OpBranch %2336
       %2337 = OpLabel
       %2399 = OpExtInst %float %1 Log %float_9_99999975en05
       %2400 = OpFDiv %float %2399 %1065
               OpBranch %2336
       %2336 = OpLabel
       %2401 = OpPhi %float %2398 %2344 %2400 %2337
       %2402 = OpExtInst %float %1 Pow %float_10 %2401
       %2403 = OpCompositeInsert %v3float %2402 %2329 1
       %2404 = OpCompositeExtract %float %2248 2
               OpStore %524 %475
               OpStore %523 %476
       %2405 = OpFOrdLessThanEqual %bool %2404 %float_0
       %2406 = OpSelect %float %2405 %2255 %2404
       %2407 = OpExtInst %float %1 Log %2406
       %2408 = OpFDiv %float %2407 %1065
       %2409 = OpFOrdLessThanEqual %bool %2408 %2260
               OpSelectionMerge %2410 None
               OpBranchConditional %2409 %2411 %2412
       %2412 = OpLabel
       %2413 = OpFOrdGreaterThan %bool %2408 %2260
       %2414 = OpExtInst %float %1 Log %float_0_180000007
       %2415 = OpFDiv %float %2414 %1065
       %2416 = OpFOrdLessThan %bool %2408 %2415
       %2417 = OpLogicalAnd %bool %2413 %2416
               OpSelectionMerge %2418 None
               OpBranchConditional %2417 %2419 %2420
       %2420 = OpLabel
       %2421 = OpFOrdGreaterThanEqual %bool %2408 %2415
       %2422 = OpExtInst %float %1 Log %2253
       %2423 = OpFDiv %float %2422 %1065
       %2424 = OpFOrdLessThan %bool %2408 %2423
       %2425 = OpLogicalAnd %bool %2421 %2424
               OpSelectionMerge %2426 None
               OpBranchConditional %2425 %2427 %2428
       %2428 = OpLabel
       %2429 = OpExtInst %float %1 Log %float_10000
       %2430 = OpFDiv %float %2429 %1065
               OpBranch %2426
       %2427 = OpLabel
       %2431 = OpFSub %float %2408 %2415
       %2432 = OpFMul %float %float_3 %2431
       %2433 = OpFSub %float %2423 %2415
       %2434 = OpFDiv %float %2432 %2433
       %2435 = OpConvertFToS %int %2434
       %2436 = OpConvertSToF %float %2435
       %2437 = OpFSub %float %2434 %2436
       %2438 = OpAccessChain %_ptr_Function_float %523 %2435
       %2439 = OpLoad %float %2438
       %2440 = OpIAdd %int %2435 %int_1
       %2441 = OpAccessChain %_ptr_Function_float %523 %2440
       %2442 = OpLoad %float %2441
       %2443 = OpIAdd %int %2435 %int_2
       %2444 = OpAccessChain %_ptr_Function_float %523 %2443
       %2445 = OpLoad %float %2444
       %2446 = OpCompositeConstruct %v3float %2439 %2442 %2445
       %2447 = OpFMul %float %2437 %2437
       %2448 = OpCompositeConstruct %v3float %2447 %2437 %float_1
       %2449 = OpMatrixTimesVector %v3float %442 %2446
       %2450 = OpDot %float %2448 %2449
               OpBranch %2426
       %2426 = OpLabel
       %2451 = OpPhi %float %2430 %2428 %2450 %2427
               OpBranch %2418
       %2419 = OpLabel
       %2452 = OpFSub %float %2408 %2260
       %2453 = OpFMul %float %float_3 %2452
       %2454 = OpFSub %float %2415 %2260
       %2455 = OpFDiv %float %2453 %2454
       %2456 = OpConvertFToS %int %2455
       %2457 = OpConvertSToF %float %2456
       %2458 = OpFSub %float %2455 %2457
       %2459 = OpAccessChain %_ptr_Function_float %524 %2456
       %2460 = OpLoad %float %2459
       %2461 = OpIAdd %int %2456 %int_1
       %2462 = OpAccessChain %_ptr_Function_float %524 %2461
       %2463 = OpLoad %float %2462
       %2464 = OpIAdd %int %2456 %int_2
       %2465 = OpAccessChain %_ptr_Function_float %524 %2464
       %2466 = OpLoad %float %2465
       %2467 = OpCompositeConstruct %v3float %2460 %2463 %2466
       %2468 = OpFMul %float %2458 %2458
       %2469 = OpCompositeConstruct %v3float %2468 %2458 %float_1
       %2470 = OpMatrixTimesVector %v3float %442 %2467
       %2471 = OpDot %float %2469 %2470
               OpBranch %2418
       %2418 = OpLabel
       %2472 = OpPhi %float %2451 %2426 %2471 %2419
               OpBranch %2410
       %2411 = OpLabel
       %2473 = OpExtInst %float %1 Log %float_9_99999975en05
       %2474 = OpFDiv %float %2473 %1065
               OpBranch %2410
       %2410 = OpLabel
       %2475 = OpPhi %float %2472 %2418 %2474 %2411
       %2476 = OpExtInst %float %1 Pow %float_10 %2475
       %2477 = OpCompositeInsert %v3float %2476 %2403 2
       %2478 = OpVectorTimesMatrix %v3float %2477 %414
       %2479 = OpVectorTimesMatrix %v3float %2478 %410
       %2480 = OpExtInst %float %1 Pow %float_2 %float_n12
       %2481 = OpFMul %float %float_0_179999992 %2480
               OpStore %532 %475
               OpStore %531 %476
       %2482 = OpFOrdLessThanEqual %bool %2481 %float_0
       %2483 = OpSelect %float %2482 %2255 %2481
       %2484 = OpExtInst %float %1 Log %2483
       %2485 = OpFDiv %float %2484 %1065
       %2486 = OpFOrdLessThanEqual %bool %2485 %2260
               OpSelectionMerge %2487 None
               OpBranchConditional %2486 %2488 %2489
       %2489 = OpLabel
       %2490 = OpFOrdGreaterThan %bool %2485 %2260
       %2491 = OpExtInst %float %1 Log %float_0_180000007
       %2492 = OpFDiv %float %2491 %1065
       %2493 = OpFOrdLessThan %bool %2485 %2492
       %2494 = OpLogicalAnd %bool %2490 %2493
               OpSelectionMerge %2495 None
               OpBranchConditional %2494 %2496 %2497
       %2497 = OpLabel
       %2498 = OpFOrdGreaterThanEqual %bool %2485 %2492
       %2499 = OpExtInst %float %1 Log %2253
       %2500 = OpFDiv %float %2499 %1065
       %2501 = OpFOrdLessThan %bool %2485 %2500
       %2502 = OpLogicalAnd %bool %2498 %2501
               OpSelectionMerge %2503 None
               OpBranchConditional %2502 %2504 %2505
       %2505 = OpLabel
       %2506 = OpExtInst %float %1 Log %float_10000
       %2507 = OpFDiv %float %2506 %1065
               OpBranch %2503
       %2504 = OpLabel
       %2508 = OpFSub %float %2485 %2492
       %2509 = OpFMul %float %float_3 %2508
       %2510 = OpFSub %float %2500 %2492
       %2511 = OpFDiv %float %2509 %2510
       %2512 = OpConvertFToS %int %2511
       %2513 = OpConvertSToF %float %2512
       %2514 = OpFSub %float %2511 %2513
       %2515 = OpAccessChain %_ptr_Function_float %531 %2512
       %2516 = OpLoad %float %2515
       %2517 = OpIAdd %int %2512 %int_1
       %2518 = OpAccessChain %_ptr_Function_float %531 %2517
       %2519 = OpLoad %float %2518
       %2520 = OpIAdd %int %2512 %int_2
       %2521 = OpAccessChain %_ptr_Function_float %531 %2520
       %2522 = OpLoad %float %2521
       %2523 = OpCompositeConstruct %v3float %2516 %2519 %2522
       %2524 = OpFMul %float %2514 %2514
       %2525 = OpCompositeConstruct %v3float %2524 %2514 %float_1
       %2526 = OpMatrixTimesVector %v3float %442 %2523
       %2527 = OpDot %float %2525 %2526
               OpBranch %2503
       %2503 = OpLabel
       %2528 = OpPhi %float %2507 %2505 %2527 %2504
               OpBranch %2495
       %2496 = OpLabel
       %2529 = OpFSub %float %2485 %2260
       %2530 = OpFMul %float %float_3 %2529
       %2531 = OpFSub %float %2492 %2260
       %2532 = OpFDiv %float %2530 %2531
       %2533 = OpConvertFToS %int %2532
       %2534 = OpConvertSToF %float %2533
       %2535 = OpFSub %float %2532 %2534
       %2536 = OpAccessChain %_ptr_Function_float %532 %2533
       %2537 = OpLoad %float %2536
       %2538 = OpIAdd %int %2533 %int_1
       %2539 = OpAccessChain %_ptr_Function_float %532 %2538
       %2540 = OpLoad %float %2539
       %2541 = OpIAdd %int %2533 %int_2
       %2542 = OpAccessChain %_ptr_Function_float %532 %2541
       %2543 = OpLoad %float %2542
       %2544 = OpCompositeConstruct %v3float %2537 %2540 %2543
       %2545 = OpFMul %float %2535 %2535
       %2546 = OpCompositeConstruct %v3float %2545 %2535 %float_1
       %2547 = OpMatrixTimesVector %v3float %442 %2544
       %2548 = OpDot %float %2546 %2547
               OpBranch %2495
       %2495 = OpLabel
       %2549 = OpPhi %float %2528 %2503 %2548 %2496
               OpBranch %2487
       %2488 = OpLabel
       %2550 = OpExtInst %float %1 Log %float_9_99999975en05
       %2551 = OpFDiv %float %2550 %1065
               OpBranch %2487
       %2487 = OpLabel
       %2552 = OpPhi %float %2549 %2495 %2551 %2488
       %2553 = OpExtInst %float %1 Pow %float_10 %2552
               OpStore %534 %475
               OpStore %533 %476
       %2554 = OpExtInst %float %1 Log %float_0_180000007
       %2555 = OpFDiv %float %2554 %1065
       %2556 = OpFOrdLessThanEqual %bool %2555 %2260
               OpSelectionMerge %2557 None
               OpBranchConditional %2556 %2558 %2559
       %2559 = OpLabel
       %2560 = OpFOrdGreaterThan %bool %2555 %2260
       %2561 = OpFOrdLessThan %bool %2555 %2555
       %2562 = OpLogicalAnd %bool %2560 %2561
               OpSelectionMerge %2563 None
               OpBranchConditional %2562 %2564 %2565
       %2565 = OpLabel
       %2566 = OpFOrdGreaterThanEqual %bool %2555 %2555
       %2567 = OpExtInst %float %1 Log %2253
       %2568 = OpFDiv %float %2567 %1065
       %2569 = OpFOrdLessThan %bool %2555 %2568
       %2570 = OpLogicalAnd %bool %2566 %2569
               OpSelectionMerge %2571 None
               OpBranchConditional %2570 %2572 %2573
       %2573 = OpLabel
       %2574 = OpExtInst %float %1 Log %float_10000
       %2575 = OpFDiv %float %2574 %1065
               OpBranch %2571
       %2572 = OpLabel
       %2576 = OpFSub %float %2555 %2555
       %2577 = OpFMul %float %float_3 %2576
       %2578 = OpFSub %float %2568 %2555
       %2579 = OpFDiv %float %2577 %2578
       %2580 = OpConvertFToS %int %2579
       %2581 = OpConvertSToF %float %2580
       %2582 = OpFSub %float %2579 %2581
       %2583 = OpAccessChain %_ptr_Function_float %533 %2580
       %2584 = OpLoad %float %2583
       %2585 = OpIAdd %int %2580 %int_1
       %2586 = OpAccessChain %_ptr_Function_float %533 %2585
       %2587 = OpLoad %float %2586
       %2588 = OpIAdd %int %2580 %int_2
       %2589 = OpAccessChain %_ptr_Function_float %533 %2588
       %2590 = OpLoad %float %2589
       %2591 = OpCompositeConstruct %v3float %2584 %2587 %2590
       %2592 = OpFMul %float %2582 %2582
       %2593 = OpCompositeConstruct %v3float %2592 %2582 %float_1
       %2594 = OpMatrixTimesVector %v3float %442 %2591
       %2595 = OpDot %float %2593 %2594
               OpBranch %2571
       %2571 = OpLabel
       %2596 = OpPhi %float %2575 %2573 %2595 %2572
               OpBranch %2563
       %2564 = OpLabel
       %2597 = OpFSub %float %2555 %2260
       %2598 = OpFMul %float %float_3 %2597
       %2599 = OpAccessChain %_ptr_Function_float %534 %int_3
       %2600 = OpLoad %float %2599
       %2601 = OpAccessChain %_ptr_Function_float %534 %int_4
       %2602 = OpLoad %float %2601
       %2603 = OpAccessChain %_ptr_Function_float %534 %int_5
       %2604 = OpLoad %float %2603
       %2605 = OpCompositeConstruct %v3float %2600 %2602 %2604
       %2606 = OpMatrixTimesVector %v3float %442 %2605
       %2607 = OpCompositeExtract %float %2606 2
               OpBranch %2563
       %2563 = OpLabel
       %2608 = OpPhi %float %2596 %2571 %2607 %2564
               OpBranch %2557
       %2558 = OpLabel
       %2609 = OpExtInst %float %1 Log %float_9_99999975en05
       %2610 = OpFDiv %float %2609 %1065
               OpBranch %2557
       %2557 = OpLabel
       %2611 = OpPhi %float %2608 %2563 %2610 %2558
       %2612 = OpExtInst %float %1 Pow %float_10 %2611
       %2613 = OpExtInst %float %1 Pow %float_2 %float_10
       %2614 = OpFMul %float %float_0_179999992 %2613
               OpStore %536 %475
               OpStore %535 %476
       %2615 = OpFOrdLessThanEqual %bool %2614 %float_0
       %2616 = OpSelect %float %2615 %2255 %2614
       %2617 = OpExtInst %float %1 Log %2616
       %2618 = OpFDiv %float %2617 %1065
       %2619 = OpFOrdLessThanEqual %bool %2618 %2260
               OpSelectionMerge %2620 None
               OpBranchConditional %2619 %2621 %2622
       %2622 = OpLabel
       %2623 = OpFOrdGreaterThan %bool %2618 %2260
       %2624 = OpFOrdLessThan %bool %2618 %2555
       %2625 = OpLogicalAnd %bool %2623 %2624
               OpSelectionMerge %2626 None
               OpBranchConditional %2625 %2627 %2628
       %2628 = OpLabel
       %2629 = OpFOrdGreaterThanEqual %bool %2618 %2555
       %2630 = OpExtInst %float %1 Log %2253
       %2631 = OpFDiv %float %2630 %1065
       %2632 = OpFOrdLessThan %bool %2618 %2631
       %2633 = OpLogicalAnd %bool %2629 %2632
               OpSelectionMerge %2634 None
               OpBranchConditional %2633 %2635 %2636
       %2636 = OpLabel
       %2637 = OpExtInst %float %1 Log %float_10000
       %2638 = OpFDiv %float %2637 %1065
               OpBranch %2634
       %2635 = OpLabel
       %2639 = OpFSub %float %2618 %2555
       %2640 = OpFMul %float %float_3 %2639
       %2641 = OpFSub %float %2631 %2555
       %2642 = OpFDiv %float %2640 %2641
       %2643 = OpConvertFToS %int %2642
       %2644 = OpConvertSToF %float %2643
       %2645 = OpFSub %float %2642 %2644
       %2646 = OpAccessChain %_ptr_Function_float %535 %2643
       %2647 = OpLoad %float %2646
       %2648 = OpIAdd %int %2643 %int_1
       %2649 = OpAccessChain %_ptr_Function_float %535 %2648
       %2650 = OpLoad %float %2649
       %2651 = OpIAdd %int %2643 %int_2
       %2652 = OpAccessChain %_ptr_Function_float %535 %2651
       %2653 = OpLoad %float %2652
       %2654 = OpCompositeConstruct %v3float %2647 %2650 %2653
       %2655 = OpFMul %float %2645 %2645
       %2656 = OpCompositeConstruct %v3float %2655 %2645 %float_1
       %2657 = OpMatrixTimesVector %v3float %442 %2654
       %2658 = OpDot %float %2656 %2657
               OpBranch %2634
       %2634 = OpLabel
       %2659 = OpPhi %float %2638 %2636 %2658 %2635
               OpBranch %2626
       %2627 = OpLabel
       %2660 = OpFSub %float %2618 %2260
       %2661 = OpFMul %float %float_3 %2660
       %2662 = OpFSub %float %2555 %2260
       %2663 = OpFDiv %float %2661 %2662
       %2664 = OpConvertFToS %int %2663
       %2665 = OpConvertSToF %float %2664
       %2666 = OpFSub %float %2663 %2665
       %2667 = OpAccessChain %_ptr_Function_float %536 %2664
       %2668 = OpLoad %float %2667
       %2669 = OpIAdd %int %2664 %int_1
       %2670 = OpAccessChain %_ptr_Function_float %536 %2669
       %2671 = OpLoad %float %2670
       %2672 = OpIAdd %int %2664 %int_2
       %2673 = OpAccessChain %_ptr_Function_float %536 %2672
       %2674 = OpLoad %float %2673
       %2675 = OpCompositeConstruct %v3float %2668 %2671 %2674
       %2676 = OpFMul %float %2666 %2666
       %2677 = OpCompositeConstruct %v3float %2676 %2666 %float_1
       %2678 = OpMatrixTimesVector %v3float %442 %2675
       %2679 = OpDot %float %2677 %2678
               OpBranch %2626
       %2626 = OpLabel
       %2680 = OpPhi %float %2659 %2634 %2679 %2627
               OpBranch %2620
       %2621 = OpLabel
       %2681 = OpExtInst %float %1 Log %float_9_99999975en05
       %2682 = OpFDiv %float %2681 %1065
               OpBranch %2620
       %2620 = OpLabel
       %2683 = OpPhi %float %2680 %2626 %2682 %2621
       %2684 = OpExtInst %float %1 Pow %float_10 %2683
       %2685 = OpCompositeExtract %float %2479 0
               OpStore %530 %479
               OpStore %529 %480
       %2686 = OpFOrdLessThanEqual %bool %2685 %float_0
       %2687 = OpSelect %float %2686 %float_9_99999975en05 %2685
       %2688 = OpExtInst %float %1 Log %2687
       %2689 = OpFDiv %float %2688 %1065
       %2690 = OpExtInst %float %1 Log %2553
       %2691 = OpFDiv %float %2690 %1065
       %2692 = OpFOrdLessThanEqual %bool %2689 %2691
               OpSelectionMerge %2693 None
               OpBranchConditional %2692 %2694 %2695
       %2695 = OpLabel
       %2696 = OpFOrdGreaterThan %bool %2689 %2691
       %2697 = OpExtInst %float %1 Log %2612
       %2698 = OpFDiv %float %2697 %1065
       %2699 = OpFOrdLessThan %bool %2689 %2698
       %2700 = OpLogicalAnd %bool %2696 %2699
               OpSelectionMerge %2701 None
               OpBranchConditional %2700 %2702 %2703
       %2703 = OpLabel
       %2704 = OpFOrdGreaterThanEqual %bool %2689 %2698
       %2705 = OpExtInst %float %1 Log %2684
       %2706 = OpFDiv %float %2705 %1065
       %2707 = OpFOrdLessThan %bool %2689 %2706
       %2708 = OpLogicalAnd %bool %2704 %2707
               OpSelectionMerge %2709 None
               OpBranchConditional %2708 %2710 %2711
       %2711 = OpLabel
       %2712 = OpFMul %float %2689 %float_0_0599999987
       %2713 = OpExtInst %float %1 Log %float_1000
       %2714 = OpFDiv %float %2713 %1065
       %2715 = OpFMul %float %float_0_0599999987 %2705
       %2716 = OpFDiv %float %2715 %1065
       %2717 = OpFSub %float %2714 %2716
       %2718 = OpFAdd %float %2712 %2717
               OpBranch %2709
       %2710 = OpLabel
       %2719 = OpFSub %float %2689 %2698
       %2720 = OpFMul %float %float_7 %2719
       %2721 = OpFSub %float %2706 %2698
       %2722 = OpFDiv %float %2720 %2721
       %2723 = OpConvertFToS %int %2722
       %2724 = OpConvertSToF %float %2723
       %2725 = OpFSub %float %2722 %2724
       %2726 = OpAccessChain %_ptr_Function_float %529 %2723
       %2727 = OpLoad %float %2726
       %2728 = OpIAdd %int %2723 %int_1
       %2729 = OpAccessChain %_ptr_Function_float %529 %2728
       %2730 = OpLoad %float %2729
       %2731 = OpIAdd %int %2723 %int_2
       %2732 = OpAccessChain %_ptr_Function_float %529 %2731
       %2733 = OpLoad %float %2732
       %2734 = OpCompositeConstruct %v3float %2727 %2730 %2733
       %2735 = OpFMul %float %2725 %2725
       %2736 = OpCompositeConstruct %v3float %2735 %2725 %float_1
       %2737 = OpMatrixTimesVector %v3float %442 %2734
       %2738 = OpDot %float %2736 %2737
               OpBranch %2709
       %2709 = OpLabel
       %2739 = OpPhi %float %2718 %2711 %2738 %2710
               OpBranch %2701
       %2702 = OpLabel
       %2740 = OpFSub %float %2689 %2691
       %2741 = OpFMul %float %float_7 %2740
       %2742 = OpFSub %float %2698 %2691
       %2743 = OpFDiv %float %2741 %2742
       %2744 = OpConvertFToS %int %2743
       %2745 = OpConvertSToF %float %2744
       %2746 = OpFSub %float %2743 %2745
       %2747 = OpAccessChain %_ptr_Function_float %530 %2744
       %2748 = OpLoad %float %2747
       %2749 = OpIAdd %int %2744 %int_1
       %2750 = OpAccessChain %_ptr_Function_float %530 %2749
       %2751 = OpLoad %float %2750
       %2752 = OpIAdd %int %2744 %int_2
       %2753 = OpAccessChain %_ptr_Function_float %530 %2752
       %2754 = OpLoad %float %2753
       %2755 = OpCompositeConstruct %v3float %2748 %2751 %2754
       %2756 = OpFMul %float %2746 %2746
       %2757 = OpCompositeConstruct %v3float %2756 %2746 %float_1
       %2758 = OpMatrixTimesVector %v3float %442 %2755
       %2759 = OpDot %float %2757 %2758
               OpBranch %2701
       %2701 = OpLabel
       %2760 = OpPhi %float %2739 %2709 %2759 %2702
               OpBranch %2693
       %2694 = OpLabel
       %2761 = OpFMul %float %2689 %float_3
       %2762 = OpExtInst %float %1 Log %float_9_99999975en05
       %2763 = OpFDiv %float %2762 %1065
       %2764 = OpFMul %float %float_3 %2690
       %2765 = OpFDiv %float %2764 %1065
       %2766 = OpFSub %float %2763 %2765
       %2767 = OpFAdd %float %2761 %2766
               OpBranch %2693
       %2693 = OpLabel
       %2768 = OpPhi %float %2760 %2701 %2767 %2694
       %2769 = OpExtInst %float %1 Pow %float_10 %2768
       %2770 = OpCompositeInsert %v3float %2769 %391 0
       %2771 = OpCompositeExtract %float %2479 1
               OpStore %528 %479
               OpStore %527 %480
       %2772 = OpFOrdLessThanEqual %bool %2771 %float_0
       %2773 = OpSelect %float %2772 %float_9_99999975en05 %2771
       %2774 = OpExtInst %float %1 Log %2773
       %2775 = OpFDiv %float %2774 %1065
       %2776 = OpFOrdLessThanEqual %bool %2775 %2691
               OpSelectionMerge %2777 None
               OpBranchConditional %2776 %2778 %2779
       %2779 = OpLabel
       %2780 = OpFOrdGreaterThan %bool %2775 %2691
       %2781 = OpExtInst %float %1 Log %2612
       %2782 = OpFDiv %float %2781 %1065
       %2783 = OpFOrdLessThan %bool %2775 %2782
       %2784 = OpLogicalAnd %bool %2780 %2783
               OpSelectionMerge %2785 None
               OpBranchConditional %2784 %2786 %2787
       %2787 = OpLabel
       %2788 = OpFOrdGreaterThanEqual %bool %2775 %2782
       %2789 = OpExtInst %float %1 Log %2684
       %2790 = OpFDiv %float %2789 %1065
       %2791 = OpFOrdLessThan %bool %2775 %2790
       %2792 = OpLogicalAnd %bool %2788 %2791
               OpSelectionMerge %2793 None
               OpBranchConditional %2792 %2794 %2795
       %2795 = OpLabel
       %2796 = OpFMul %float %2775 %float_0_0599999987
       %2797 = OpExtInst %float %1 Log %float_1000
       %2798 = OpFDiv %float %2797 %1065
       %2799 = OpFMul %float %float_0_0599999987 %2789
       %2800 = OpFDiv %float %2799 %1065
       %2801 = OpFSub %float %2798 %2800
       %2802 = OpFAdd %float %2796 %2801
               OpBranch %2793
       %2794 = OpLabel
       %2803 = OpFSub %float %2775 %2782
       %2804 = OpFMul %float %float_7 %2803
       %2805 = OpFSub %float %2790 %2782
       %2806 = OpFDiv %float %2804 %2805
       %2807 = OpConvertFToS %int %2806
       %2808 = OpConvertSToF %float %2807
       %2809 = OpFSub %float %2806 %2808
       %2810 = OpAccessChain %_ptr_Function_float %527 %2807
       %2811 = OpLoad %float %2810
       %2812 = OpIAdd %int %2807 %int_1
       %2813 = OpAccessChain %_ptr_Function_float %527 %2812
       %2814 = OpLoad %float %2813
       %2815 = OpIAdd %int %2807 %int_2
       %2816 = OpAccessChain %_ptr_Function_float %527 %2815
       %2817 = OpLoad %float %2816
       %2818 = OpCompositeConstruct %v3float %2811 %2814 %2817
       %2819 = OpFMul %float %2809 %2809
       %2820 = OpCompositeConstruct %v3float %2819 %2809 %float_1
       %2821 = OpMatrixTimesVector %v3float %442 %2818
       %2822 = OpDot %float %2820 %2821
               OpBranch %2793
       %2793 = OpLabel
       %2823 = OpPhi %float %2802 %2795 %2822 %2794
               OpBranch %2785
       %2786 = OpLabel
       %2824 = OpFSub %float %2775 %2691
       %2825 = OpFMul %float %float_7 %2824
       %2826 = OpFSub %float %2782 %2691
       %2827 = OpFDiv %float %2825 %2826
       %2828 = OpConvertFToS %int %2827
       %2829 = OpConvertSToF %float %2828
       %2830 = OpFSub %float %2827 %2829
       %2831 = OpAccessChain %_ptr_Function_float %528 %2828
       %2832 = OpLoad %float %2831
       %2833 = OpIAdd %int %2828 %int_1
       %2834 = OpAccessChain %_ptr_Function_float %528 %2833
       %2835 = OpLoad %float %2834
       %2836 = OpIAdd %int %2828 %int_2
       %2837 = OpAccessChain %_ptr_Function_float %528 %2836
       %2838 = OpLoad %float %2837
       %2839 = OpCompositeConstruct %v3float %2832 %2835 %2838
       %2840 = OpFMul %float %2830 %2830
       %2841 = OpCompositeConstruct %v3float %2840 %2830 %float_1
       %2842 = OpMatrixTimesVector %v3float %442 %2839
       %2843 = OpDot %float %2841 %2842
               OpBranch %2785
       %2785 = OpLabel
       %2844 = OpPhi %float %2823 %2793 %2843 %2786
               OpBranch %2777
       %2778 = OpLabel
       %2845 = OpFMul %float %2775 %float_3
       %2846 = OpExtInst %float %1 Log %float_9_99999975en05
       %2847 = OpFDiv %float %2846 %1065
       %2848 = OpFMul %float %float_3 %2690
       %2849 = OpFDiv %float %2848 %1065
       %2850 = OpFSub %float %2847 %2849
       %2851 = OpFAdd %float %2845 %2850
               OpBranch %2777
       %2777 = OpLabel
       %2852 = OpPhi %float %2844 %2785 %2851 %2778
       %2853 = OpExtInst %float %1 Pow %float_10 %2852
       %2854 = OpCompositeInsert %v3float %2853 %2770 1
       %2855 = OpCompositeExtract %float %2479 2
               OpStore %526 %479
               OpStore %525 %480
       %2856 = OpFOrdLessThanEqual %bool %2855 %float_0
       %2857 = OpSelect %float %2856 %float_9_99999975en05 %2855
       %2858 = OpExtInst %float %1 Log %2857
       %2859 = OpFDiv %float %2858 %1065
       %2860 = OpFOrdLessThanEqual %bool %2859 %2691
               OpSelectionMerge %2861 None
               OpBranchConditional %2860 %2862 %2863
       %2863 = OpLabel
       %2864 = OpFOrdGreaterThan %bool %2859 %2691
       %2865 = OpExtInst %float %1 Log %2612
       %2866 = OpFDiv %float %2865 %1065
       %2867 = OpFOrdLessThan %bool %2859 %2866
       %2868 = OpLogicalAnd %bool %2864 %2867
               OpSelectionMerge %2869 None
               OpBranchConditional %2868 %2870 %2871
       %2871 = OpLabel
       %2872 = OpFOrdGreaterThanEqual %bool %2859 %2866
       %2873 = OpExtInst %float %1 Log %2684
       %2874 = OpFDiv %float %2873 %1065
       %2875 = OpFOrdLessThan %bool %2859 %2874
       %2876 = OpLogicalAnd %bool %2872 %2875
               OpSelectionMerge %2877 None
               OpBranchConditional %2876 %2878 %2879
       %2879 = OpLabel
       %2880 = OpFMul %float %2859 %float_0_0599999987
       %2881 = OpExtInst %float %1 Log %float_1000
       %2882 = OpFDiv %float %2881 %1065
       %2883 = OpFMul %float %float_0_0599999987 %2873
       %2884 = OpFDiv %float %2883 %1065
       %2885 = OpFSub %float %2882 %2884
       %2886 = OpFAdd %float %2880 %2885
               OpBranch %2877
       %2878 = OpLabel
       %2887 = OpFSub %float %2859 %2866
       %2888 = OpFMul %float %float_7 %2887
       %2889 = OpFSub %float %2874 %2866
       %2890 = OpFDiv %float %2888 %2889
       %2891 = OpConvertFToS %int %2890
       %2892 = OpConvertSToF %float %2891
       %2893 = OpFSub %float %2890 %2892
       %2894 = OpAccessChain %_ptr_Function_float %525 %2891
       %2895 = OpLoad %float %2894
       %2896 = OpIAdd %int %2891 %int_1
       %2897 = OpAccessChain %_ptr_Function_float %525 %2896
       %2898 = OpLoad %float %2897
       %2899 = OpIAdd %int %2891 %int_2
       %2900 = OpAccessChain %_ptr_Function_float %525 %2899
       %2901 = OpLoad %float %2900
       %2902 = OpCompositeConstruct %v3float %2895 %2898 %2901
       %2903 = OpFMul %float %2893 %2893
       %2904 = OpCompositeConstruct %v3float %2903 %2893 %float_1
       %2905 = OpMatrixTimesVector %v3float %442 %2902
       %2906 = OpDot %float %2904 %2905
               OpBranch %2877
       %2877 = OpLabel
       %2907 = OpPhi %float %2886 %2879 %2906 %2878
               OpBranch %2869
       %2870 = OpLabel
       %2908 = OpFSub %float %2859 %2691
       %2909 = OpFMul %float %float_7 %2908
       %2910 = OpFSub %float %2866 %2691
       %2911 = OpFDiv %float %2909 %2910
       %2912 = OpConvertFToS %int %2911
       %2913 = OpConvertSToF %float %2912
       %2914 = OpFSub %float %2911 %2913
       %2915 = OpAccessChain %_ptr_Function_float %526 %2912
       %2916 = OpLoad %float %2915
       %2917 = OpIAdd %int %2912 %int_1
       %2918 = OpAccessChain %_ptr_Function_float %526 %2917
       %2919 = OpLoad %float %2918
       %2920 = OpIAdd %int %2912 %int_2
       %2921 = OpAccessChain %_ptr_Function_float %526 %2920
       %2922 = OpLoad %float %2921
       %2923 = OpCompositeConstruct %v3float %2916 %2919 %2922
       %2924 = OpFMul %float %2914 %2914
       %2925 = OpCompositeConstruct %v3float %2924 %2914 %float_1
       %2926 = OpMatrixTimesVector %v3float %442 %2923
       %2927 = OpDot %float %2925 %2926
               OpBranch %2869
       %2869 = OpLabel
       %2928 = OpPhi %float %2907 %2877 %2927 %2870
               OpBranch %2861
       %2862 = OpLabel
       %2929 = OpFMul %float %2859 %float_3
       %2930 = OpExtInst %float %1 Log %float_9_99999975en05
       %2931 = OpFDiv %float %2930 %1065
       %2932 = OpFMul %float %float_3 %2690
       %2933 = OpFDiv %float %2932 %1065
       %2934 = OpFSub %float %2931 %2933
       %2935 = OpFAdd %float %2929 %2934
               OpBranch %2861
       %2861 = OpLabel
       %2936 = OpPhi %float %2928 %2869 %2935 %2862
       %2937 = OpExtInst %float %1 Pow %float_10 %2936
       %2938 = OpCompositeInsert %v3float %2937 %2854 2
       %2939 = OpFSub %v3float %2938 %338
       %2940 = OpVectorTimesMatrix %v3float %2939 %576
       %2941 = OpFMul %v3float %2940 %496
       %2942 = OpExtInst %v3float %1 Pow %2941 %263
       %2943 = OpFMul %v3float %184 %2942
       %2944 = OpFAdd %v3float %183 %2943
       %2945 = OpFMul %v3float %185 %2942
       %2946 = OpFAdd %v3float %135 %2945
       %2947 = OpFDiv %v3float %135 %2946
       %2948 = OpFMul %v3float %2944 %2947
       %2949 = OpExtInst %v3float %1 Pow %2948 %264
               OpBranch %1230
       %1230 = OpLabel
       %2950 = OpPhi %v3float %2097 %1236 %2949 %2861
               OpBranch %1224
       %1225 = OpLabel
       %2951 = OpVectorTimesMatrix %v3float %1218 %547
       %2952 = OpVectorTimesMatrix %v3float %2951 %576
       %2953 = OpExtInst %v3float %1 FMax %250 %2952
       %2954 = OpFMul %v3float %2953 %252
       %2955 = OpExtInst %v3float %1 FMax %2953 %254
       %2956 = OpExtInst %v3float %1 Pow %2955 %256
       %2957 = OpFMul %v3float %2956 %258
       %2958 = OpFSub %v3float %2957 %260
       %2959 = OpExtInst %v3float %1 FMin %2954 %2958
               OpBranch %1224
       %1224 = OpLabel
       %2960 = OpPhi %v3float %2950 %1230 %2959 %1225
               OpBranch %1220
       %1221 = OpLabel
       %2961 = OpCompositeExtract %float %1218 0
               OpBranch %2962
       %2962 = OpLabel
               OpLoopMerge %2963 %2964 None
               OpBranch %2965
       %2965 = OpLabel
       %2966 = OpFOrdLessThan %bool %2961 %float_0_00313066994
               OpSelectionMerge %2967 None
               OpBranchConditional %2966 %2968 %2967
       %2968 = OpLabel
       %2969 = OpFMul %float %2961 %float_12_9200001
               OpBranch %2963
       %2967 = OpLabel
       %2970 = OpExtInst %float %1 Pow %2961 %float_0_416666657
       %2971 = OpFMul %float %2970 %float_1_05499995
       %2972 = OpFSub %float %2971 %float_0_0549999997
               OpBranch %2963
       %2964 = OpLabel
               OpBranch %2962
       %2963 = OpLabel
       %2973 = OpPhi %float %2969 %2968 %2972 %2967
       %2974 = OpCompositeExtract %float %1218 1
               OpBranch %2975
       %2975 = OpLabel
               OpLoopMerge %2976 %2977 None
               OpBranch %2978
       %2978 = OpLabel
       %2979 = OpFOrdLessThan %bool %2974 %float_0_00313066994
               OpSelectionMerge %2980 None
               OpBranchConditional %2979 %2981 %2980
       %2981 = OpLabel
       %2982 = OpFMul %float %2974 %float_12_9200001
               OpBranch %2976
       %2980 = OpLabel
       %2983 = OpExtInst %float %1 Pow %2974 %float_0_416666657
       %2984 = OpFMul %float %2983 %float_1_05499995
       %2985 = OpFSub %float %2984 %float_0_0549999997
               OpBranch %2976
       %2977 = OpLabel
               OpBranch %2975
       %2976 = OpLabel
       %2986 = OpPhi %float %2982 %2981 %2985 %2980
       %2987 = OpCompositeExtract %float %1218 2
               OpBranch %2988
       %2988 = OpLabel
               OpLoopMerge %2989 %2990 None
               OpBranch %2991
       %2991 = OpLabel
       %2992 = OpFOrdLessThan %bool %2987 %float_0_00313066994
               OpSelectionMerge %2993 None
               OpBranchConditional %2992 %2994 %2993
       %2994 = OpLabel
       %2995 = OpFMul %float %2987 %float_12_9200001
               OpBranch %2989
       %2993 = OpLabel
       %2996 = OpExtInst %float %1 Pow %2987 %float_0_416666657
       %2997 = OpFMul %float %2996 %float_1_05499995
       %2998 = OpFSub %float %2997 %float_0_0549999997
               OpBranch %2989
       %2990 = OpLabel
               OpBranch %2988
       %2989 = OpLabel
       %2999 = OpPhi %float %2995 %2994 %2998 %2993
       %3000 = OpCompositeConstruct %v3float %2973 %2986 %2999
               OpBranch %1220
       %1220 = OpLabel
       %3001 = OpPhi %v3float %2960 %1224 %3000 %2989
       %3002 = OpFMul %v3float %3001 %499
       %3003 = OpVectorShuffle %v4float %129 %3002 4 5 6 3
       %3004 = OpCompositeInsert %v4float %float_0 %3003 3
               OpStore %out_var_SV_Target0 %3004
               OpReturn
               OpFunctionEnd
