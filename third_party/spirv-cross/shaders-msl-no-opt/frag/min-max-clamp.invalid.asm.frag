; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 205
; Schema: 0
               OpCapability Shader
               OpCapability Float16
               OpExtension "SPV_AMD_gpu_shader_half_float"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %v1 %v2 %v3 %v4 %h1 %h2 %h3 %h4
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_AMD_gpu_shader_half_float"
               OpName %main "main"
               OpName %res "res"
               OpName %res2 "res2"
               OpName %res3 "res3"
               OpName %res4 "res4"
               OpName %hres "hres"
               OpName %hres2 "hres2"
               OpName %hres3 "hres3"
               OpName %hres4 "hres4"
               OpName %v1 "v1"
               OpName %v2 "v2"
               OpName %v3 "v3"
               OpName %v4 "v4"
               OpName %h1 "h1"
               OpName %h2 "h2"
               OpName %h3 "h3"
               OpName %h4 "h4"
               OpDecorate %v1 Location 0
               OpDecorate %v2 Location 1
               OpDecorate %v3 Location 2
               OpDecorate %v4 Location 3
               OpDecorate %h1 Location 4
               OpDecorate %h2 Location 5
               OpDecorate %h3 Location 6
               OpDecorate %h4 Location 7
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4
       %half = OpTypeFloat 16
     %v2half = OpTypeVector %half 2
     %v3half = OpTypeVector %half 3
     %v4half = OpTypeVector %half 4
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Input_v3float = OpTypePointer Input %v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Function_half = OpTypePointer Function %half
%_ptr_Input_half = OpTypePointer Input %half
%_ptr_Function_v2half = OpTypePointer Function %v2half
%_ptr_Input_v2half = OpTypePointer Input %v2half
%_ptr_Function_v3half = OpTypePointer Function %v3half
%_ptr_Input_v3half = OpTypePointer Input %v3half
%_ptr_Function_v4half = OpTypePointer Function %v4half
%_ptr_Input_v4half = OpTypePointer Input %v4half
         %v1 = OpVariable %_ptr_Input_float Input
         %v2 = OpVariable %_ptr_Input_v2float Input
         %v3 = OpVariable %_ptr_Input_v3float Input
         %v4 = OpVariable %_ptr_Input_v4float Input
         %h1 = OpVariable %_ptr_Input_half Input
         %h2 = OpVariable %_ptr_Input_v2half Input
         %h3 = OpVariable %_ptr_Input_v3half Input
         %h4 = OpVariable %_ptr_Input_v4half Input
       %main = OpFunction %void None %3
          %5 = OpLabel
        %res = OpVariable %_ptr_Function_float Function
         %46 = OpLoad %float %v1
         %47 = OpLoad %float %v1
         %48 = OpExtInst %float %1 FMin %46 %47
               OpStore %res %48
         %49 = OpLoad %float %v1
         %50 = OpLoad %float %v1
         %51 = OpExtInst %float %1 FMax %49 %50
               OpStore %res %51
         %52 = OpLoad %float %v1
         %53 = OpLoad %float %v1
         %54 = OpLoad %float %v1
         %55 = OpExtInst %float %1 FClamp %52 %53 %54
               OpStore %res %55
         %56 = OpLoad %float %v1
         %57 = OpLoad %float %v1
         %58 = OpExtInst %float %1 NMin %56 %57
               OpStore %res %58
         %59 = OpLoad %float %v1
         %60 = OpLoad %float %v1
         %61 = OpExtInst %float %1 NMax %59 %60
               OpStore %res %61
         %62 = OpLoad %float %v1
         %63 = OpLoad %float %v1
         %64 = OpLoad %float %v1
         %65 = OpExtInst %float %1 NClamp %62 %63 %64
               OpStore %res %65
       %res2 = OpVariable %_ptr_Function_v2float Function
         %66 = OpLoad %v2float %v2
         %67 = OpLoad %v2float %v2
         %68 = OpExtInst %v2float %1 FMin %66 %67
               OpStore %res2 %68
         %69 = OpLoad %v2float %v2
         %70 = OpLoad %v2float %v2
         %71 = OpExtInst %v2float %1 FMax %69 %70
               OpStore %res2 %71
         %72 = OpLoad %v2float %v2
         %73 = OpLoad %v2float %v2
         %74 = OpLoad %v2float %v2
         %75 = OpExtInst %v2float %1 FClamp %72 %73 %74
               OpStore %res2 %75
         %76 = OpLoad %v2float %v2
         %77 = OpLoad %v2float %v2
         %78 = OpExtInst %v2float %1 NMin %76 %77
               OpStore %res2 %78
         %79 = OpLoad %v2float %v2
         %80 = OpLoad %v2float %v2
         %81 = OpExtInst %v2float %1 NMax %79 %80
               OpStore %res2 %81
         %82 = OpLoad %v2float %v2
         %83 = OpLoad %v2float %v2
         %84 = OpLoad %v2float %v2
         %85 = OpExtInst %v2float %1 NClamp %82 %83 %84
               OpStore %res2 %85
       %res3 = OpVariable %_ptr_Function_v3float Function
         %86 = OpLoad %v3float %v3
         %87 = OpLoad %v3float %v3
         %88 = OpExtInst %v3float %1 FMin %86 %87
               OpStore %res3 %88
         %89 = OpLoad %v3float %v3
         %90 = OpLoad %v3float %v3
         %91 = OpExtInst %v3float %1 FMax %89 %90
               OpStore %res3 %91
         %92 = OpLoad %v3float %v3
         %93 = OpLoad %v3float %v3
         %94 = OpLoad %v3float %v3
         %95 = OpExtInst %v3float %1 FClamp %92 %93 %94
               OpStore %res3 %95
         %96 = OpLoad %v3float %v3
         %97 = OpLoad %v3float %v3
         %98 = OpExtInst %v3float %1 NMin %96 %97
               OpStore %res3 %98
         %99 = OpLoad %v3float %v3
        %100 = OpLoad %v3float %v3
        %101 = OpExtInst %v3float %1 NMax %99 %100
               OpStore %res3 %101
        %102 = OpLoad %v3float %v3
        %103 = OpLoad %v3float %v3
        %104 = OpLoad %v3float %v3
        %105 = OpExtInst %v3float %1 NClamp %102 %103 %104
               OpStore %res3 %105
       %res4 = OpVariable %_ptr_Function_v4float Function
        %106 = OpLoad %v4float %v4
        %107 = OpLoad %v4float %v4
        %108 = OpExtInst %v4float %1 FMin %106 %107
               OpStore %res4 %108
        %109 = OpLoad %v4float %v4
        %110 = OpLoad %v4float %v4
        %111 = OpExtInst %v4float %1 FMax %109 %110
               OpStore %res4 %111
        %112 = OpLoad %v4float %v4
        %113 = OpLoad %v4float %v4
        %114 = OpLoad %v4float %v4
        %115 = OpExtInst %v4float %1 FClamp %112 %113 %114
               OpStore %res4 %115
        %116 = OpLoad %v4float %v4
        %117 = OpLoad %v4float %v4
        %118 = OpExtInst %v4float %1 NMin %116 %117
               OpStore %res4 %118
        %119 = OpLoad %v4float %v4
        %120 = OpLoad %v4float %v4
        %121 = OpExtInst %v4float %1 NMax %119 %120
               OpStore %res4 %121
        %122 = OpLoad %v4float %v4
        %123 = OpLoad %v4float %v4
        %124 = OpLoad %v4float %v4
        %125 = OpExtInst %v4float %1 NClamp %122 %123 %124
               OpStore %res4 %125
       %hres = OpVariable %_ptr_Function_half Function
        %126 = OpLoad %half %h1
        %127 = OpLoad %half %h1
        %128 = OpExtInst %half %1 FMin %126 %127
               OpStore %hres %128
        %129 = OpLoad %half %h1
        %130 = OpLoad %half %h1
        %131 = OpExtInst %half %1 FMax %129 %130
               OpStore %hres %131
        %132 = OpLoad %half %h1
        %133 = OpLoad %half %h1
        %134 = OpLoad %half %h1
        %135 = OpExtInst %half %1 FClamp %132 %133 %134
               OpStore %hres %135
        %136 = OpLoad %half %h1
        %137 = OpLoad %half %h1
        %138 = OpExtInst %half %1 NMin %136 %137
               OpStore %hres %138
        %139 = OpLoad %half %h1
        %140 = OpLoad %half %h1
        %141 = OpExtInst %half %1 NMax %139 %140
               OpStore %hres %141
        %142 = OpLoad %half %h1
        %143 = OpLoad %half %h1
        %144 = OpLoad %half %h1
        %145 = OpExtInst %half %1 NClamp %142 %143 %144
               OpStore %hres %145
      %hres2 = OpVariable %_ptr_Function_v2half Function
        %146 = OpLoad %v2half %h2
        %147 = OpLoad %v2half %h2
        %148 = OpExtInst %v2half %1 FMin %146 %147
               OpStore %hres2 %148
        %149 = OpLoad %v2half %h2
        %150 = OpLoad %v2half %h2
        %151 = OpExtInst %v2half %1 FMax %149 %150
               OpStore %hres2 %151
        %152 = OpLoad %v2half %h2
        %153 = OpLoad %v2half %h2
        %154 = OpLoad %v2half %h2
        %155 = OpExtInst %v2half %1 FClamp %152 %153 %154
               OpStore %hres2 %155
        %156 = OpLoad %v2half %h2
        %157 = OpLoad %v2half %h2
        %158 = OpExtInst %v2half %1 NMin %156 %157
               OpStore %hres2 %158
        %159 = OpLoad %v2half %h2
        %160 = OpLoad %v2half %h2
        %161 = OpExtInst %v2half %1 NMax %159 %160
               OpStore %hres2 %161
        %162 = OpLoad %v2half %h2
        %163 = OpLoad %v2half %h2
        %164 = OpLoad %v2half %h2
        %165 = OpExtInst %v2half %1 NClamp %162 %163 %164
               OpStore %hres2 %165
      %hres3 = OpVariable %_ptr_Function_v3half Function
        %166 = OpLoad %v3half %h3
        %167 = OpLoad %v3half %h3
        %168 = OpExtInst %v3half %1 FMin %166 %167
               OpStore %hres3 %168
        %169 = OpLoad %v3half %h3
        %170 = OpLoad %v3half %h3
        %171 = OpExtInst %v3half %1 FMax %169 %170
               OpStore %hres3 %171
        %172 = OpLoad %v3half %h3
        %173 = OpLoad %v3half %h3
        %174 = OpLoad %v3half %h3
        %175 = OpExtInst %v3half %1 FClamp %172 %173 %174
               OpStore %hres3 %175
        %176 = OpLoad %v3half %h3
        %177 = OpLoad %v3half %h3
        %178 = OpExtInst %v3half %1 NMin %176 %177
               OpStore %hres3 %178
        %179 = OpLoad %v3half %h3
        %180 = OpLoad %v3half %h3
        %181 = OpExtInst %v3half %1 NMax %179 %180
               OpStore %hres3 %181
        %182 = OpLoad %v3half %h3
        %183 = OpLoad %v3half %h3
        %184 = OpLoad %v3half %h3
        %185 = OpExtInst %v3half %1 NClamp %182 %183 %184
               OpStore %hres3 %185
      %hres4 = OpVariable %_ptr_Function_v4half Function
        %186 = OpLoad %v4half %h4
        %187 = OpLoad %v4half %h4
        %188 = OpExtInst %v4half %1 FMin %186 %187
               OpStore %hres4 %188
        %189 = OpLoad %v4half %h4
        %190 = OpLoad %v4half %h4
        %191 = OpExtInst %v4half %1 FMax %189 %190
               OpStore %hres4 %191
        %192 = OpLoad %v4half %h4
        %193 = OpLoad %v4half %h4
        %194 = OpLoad %v4half %h4
        %195 = OpExtInst %v4half %1 FClamp %192 %193 %194
               OpStore %hres4 %195
        %196 = OpLoad %v4half %h4
        %197 = OpLoad %v4half %h4
        %198 = OpExtInst %v4half %1 NMin %196 %197
               OpStore %hres4 %198
        %199 = OpLoad %v4half %h4
        %200 = OpLoad %v4half %h4
        %201 = OpExtInst %v4half %1 NMax %199 %200
               OpStore %hres4 %201
        %202 = OpLoad %v4half %h4
        %203 = OpLoad %v4half %h4
        %204 = OpLoad %v4half %h4
        %205 = OpExtInst %v4half %1 NClamp %202 %203 %204
               OpStore %hres4 %205
               OpReturn
               OpFunctionEnd
