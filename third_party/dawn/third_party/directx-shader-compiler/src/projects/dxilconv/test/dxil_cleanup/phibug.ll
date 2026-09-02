; Bug 12870380
; Make sure dxil cleanup pass generates valid llvm ir.
; RUN: %opt-exe %s -S -dxil-cleanup -verify -o %t.ll.converted
; RUN: fc %t.ll.converted %b.ref

%dx.types.Handle = type { i8* }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%dx.types.CBufRet.f32 = type { float, float, float, float }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%dx.types.f32 = type { float }
%dx.types.i8x48 = type { [48 x i8] }
%dx.types.i8x4 = type { [4 x i8] }
%dx.types.i8x16 = type { [16 x i8] }
%dx.types.i8x32 = type { [32 x i8] }

@TGSM0 = internal addrspace(3) global [2048 x i8] undef, align 4
@llvm.used = appending global [1 x i8*] [i8* addrspacecast (i8 addrspace(3)* getelementptr inbounds ([2048 x i8], [2048 x i8] addrspace(3)* @TGSM0, i32 0, i32 0) to i8*)], section "llvm.metadata"

define void @main() {
entry:
  %0 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)
  %1 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 1, i32 1, i1 false)
  %2 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 0, i1 false)
  %3 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)
  %4 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 1, i32 1, i1 false)
  %5 = call i32 @dx.op.groupId.i32(i32 94, i32 0)
  %6 = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 0)
  %7 = call i32 @dx.op.tertiary.i32(i32 48, i32 %5, i32 64, i32 %6)
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %7)
  %8 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %9 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %1, i32 %8, i32 0)
  %10 = extractvalue %dx.types.ResRet.i32 %9, 0
  %11 = extractvalue %dx.types.ResRet.i32 %9, 1
  %12 = extractvalue %dx.types.ResRet.i32 %9, 2
  %13 = extractvalue %dx.types.ResRet.i32 %9, 3
  call void @dx.op.tempRegStore.i32(i32 1, i32 4, i32 %10)
  call void @dx.op.tempRegStore.i32(i32 1, i32 5, i32 %11)
  call void @dx.op.tempRegStore.i32(i32 1, i32 6, i32 %12)
  call void @dx.op.tempRegStore.i32(i32 1, i32 7, i32 %13)
  %14 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %15 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %1, i32 %14, i32 16)
  %16 = extractvalue %dx.types.ResRet.i32 %15, 0
  %17 = extractvalue %dx.types.ResRet.i32 %15, 1
  %18 = extractvalue %dx.types.ResRet.i32 %15, 2
  %19 = extractvalue %dx.types.ResRet.i32 %15, 3
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %16)
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %17)
  call void @dx.op.tempRegStore.i32(i32 1, i32 2, i32 %18)
  call void @dx.op.tempRegStore.i32(i32 1, i32 3, i32 %19)
  %20 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %21 = icmp slt i32 %20, 18
  %22 = sext i1 %21 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %22)
  %23 = call float @dx.op.tempRegLoad.f32(i32 0, i32 6)
  %24 = call float @dx.op.tempRegLoad.f32(i32 0, i32 7)
  %25 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %3, i32 0)
  %26 = extractvalue %dx.types.CBufRet.f32 %25, 0
  %27 = extractvalue %dx.types.CBufRet.f32 %25, 1
  %28 = fmul fast float %23, %26
  %29 = fmul fast float %24, %27
  call void @dx.op.tempRegStore.f32(i32 1, i32 8, float %28)
  call void @dx.op.tempRegStore.f32(i32 1, i32 9, float %29)
  %30 = call float @dx.op.tempRegLoad.f32(i32 0, i32 8)
  %31 = call float @dx.op.tempRegLoad.f32(i32 0, i32 9)
  %32 = call float @dx.op.tempRegLoad.f32(i32 0, i32 8)
  %33 = call float @dx.op.tempRegLoad.f32(i32 0, i32 9)
  %34 = call float @dx.op.dot2.f32(i32 54, float %30, float %31, float %32, float %33)
  call void @dx.op.tempRegStore.f32(i32 1, i32 10, float %34)
  %35 = call float @dx.op.tempRegLoad.f32(i32 0, i32 10)
  %36 = call float @dx.op.unary.f32(i32 25, float %35)
  call void @dx.op.tempRegStore.f32(i32 1, i32 10, float %36)
  %37 = call float @dx.op.tempRegLoad.f32(i32 0, i32 10)
  %38 = call float @dx.op.tempRegLoad.f32(i32 0, i32 8)
  %39 = call float @dx.op.tempRegLoad.f32(i32 0, i32 9)
  %40 = fmul fast float %37, %38
  %41 = fmul fast float %37, %39
  call void @dx.op.tempRegStore.f32(i32 1, i32 8, float %40)
  call void @dx.op.tempRegStore.f32(i32 1, i32 9, float %41)
  %42 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %43 = icmp ne i32 %42, 0
  %44 = select i1 %43, i32 128, i32 -128
  %45 = select i1 %43, i32 0, i32 1
  %46 = select i1 %43, i32 -128, i32 128
  call void @dx.op.tempRegStore.i32(i32 1, i32 12, i32 %44)
  call void @dx.op.tempRegStore.i32(i32 1, i32 13, i32 %45)
  call void @dx.op.tempRegStore.i32(i32 1, i32 14, i32 %46)
  %47 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %48 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 13)
  %49 = call i32 @dx.op.quaternary.i32(i32 53, i32 31, i32 1, i32 %47, i32 %48)
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %49)
  %50 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  %51 = icmp sge i32 0, %50
  %52 = sext i1 %51 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 3, i32 %52)
  %53 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %54 = icmp ne i32 %53, 0
  br i1 %54, label %if0.then, label %if0.end

if0.then:                                         ; preds = %entry
  %55 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %2, i32 %55, i32 0, i32 -1140821790, i32 undef, i32 undef, i32 undef, i8 1)
  %56 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 12)
  %57 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %58 = add i32 %56, %57
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %58)
  br label %if0.end

if0.end:                                          ; preds = %if0.then, %entry
  %59 = call float @dx.op.tempRegLoad.f32(i32 0, i32 4)
  %60 = call float @dx.op.tempRegLoad.f32(i32 0, i32 5)
  %61 = call float @dx.op.tertiary.f32(i32 46, float %59, float 1.000000e+00, float 0.000000e+00)
  %62 = call float @dx.op.tertiary.f32(i32 46, float %60, float -1.000000e+00, float 1.000000e+00)
  call void @dx.op.tempRegStore.f32(i32 1, i32 13, float %61)
  call void @dx.op.tempRegStore.f32(i32 1, i32 15, float %62)
  %63 = call float @dx.op.tempRegLoad.f32(i32 0, i32 13)
  %64 = call float @dx.op.tempRegLoad.f32(i32 0, i32 15)
  %65 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %3, i32 0)
  %66 = extractvalue %dx.types.CBufRet.f32 %65, 0
  %67 = extractvalue %dx.types.CBufRet.f32 %65, 1
  %68 = fmul fast float %63, %66
  %69 = fmul fast float %64, %67
  call void @dx.op.tempRegStore.f32(i32 1, i32 13, float %68)
  call void @dx.op.tempRegStore.f32(i32 1, i32 15, float %69)
  %70 = call float @dx.op.tempRegLoad.f32(i32 0, i32 13)
  %71 = call float @dx.op.tempRegLoad.f32(i32 0, i32 15)
  %72 = fptosi float %70 to i32
  %73 = fptosi float %71 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 13, i32 %72)
  call void @dx.op.tempRegStore.i32(i32 1, i32 15, i32 %73)
  %74 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %3, i32 0)
  %75 = extractvalue %dx.types.CBufRet.f32 %74, 0
  %76 = extractvalue %dx.types.CBufRet.f32 %74, 1
  %77 = fptosi float %75 to i32
  %78 = fptosi float %76 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 16, i32 %77)
  call void @dx.op.tempRegStore.i32(i32 1, i32 17, i32 %78)
  %79 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 16)
  %80 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 17)
  %81 = add i32 %79, -1
  %82 = add i32 %80, -1
  call void @dx.op.tempRegStore.i32(i32 1, i32 16, i32 %81)
  call void @dx.op.tempRegStore.i32(i32 1, i32 17, i32 %82)
  %83 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 13)
  %84 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 15)
  %85 = call i32 @dx.op.binary.i32(i32 37, i32 %83, i32 0)
  %86 = call i32 @dx.op.binary.i32(i32 37, i32 %84, i32 0)
  call void @dx.op.tempRegStore.i32(i32 1, i32 13, i32 %85)
  call void @dx.op.tempRegStore.i32(i32 1, i32 15, i32 %86)
  %87 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 16)
  %88 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 17)
  %89 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 13)
  %90 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 15)
  %91 = call i32 @dx.op.binary.i32(i32 38, i32 %87, i32 %89)
  %92 = call i32 @dx.op.binary.i32(i32 38, i32 %88, i32 %90)
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %91)
  call void @dx.op.tempRegStore.i32(i32 1, i32 21, i32 %92)
  call void @dx.op.tempRegStore.i32(i32 1, i32 22, i32 0)
  call void @dx.op.tempRegStore.i32(i32 1, i32 23, i32 0)
  %93 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %94 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 21)
  %95 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 23)
  %96 = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %0, i32 %95, i32 %93, i32 %94, i32 undef, i32 0, i32 0, i32 undef)
  %97 = extractvalue %dx.types.ResRet.f32 %96, 0
  call void @dx.op.tempRegStore.f32(i32 1, i32 15, float %97)
  %98 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %99 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 21)
  %100 = uitofp i32 %98 to float
  %101 = uitofp i32 %99 to float
  call void @dx.op.tempRegStore.f32(i32 1, i32 18, float %100)
  call void @dx.op.tempRegStore.f32(i32 1, i32 19, float %101)
  %102 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %103 = extractvalue %dx.types.CBufRet.f32 %102, 2
  %104 = call float @dx.op.tempRegLoad.f32(i32 0, i32 18)
  %105 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 1)
  %106 = extractvalue %dx.types.CBufRet.f32 %105, 0
  %107 = call float @dx.op.tertiary.f32(i32 46, float %103, float %104, float %106)
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %107)
  %108 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %109 = extractvalue %dx.types.CBufRet.f32 %108, 3
  %110 = fsub fast float -0.000000e+00, %109
  %111 = call float @dx.op.tempRegLoad.f32(i32 0, i32 19)
  %112 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 1)
  %113 = extractvalue %dx.types.CBufRet.f32 %112, 1
  %114 = fsub fast float -0.000000e+00, %113
  %115 = call float @dx.op.tertiary.f32(i32 46, float %110, float %111, float %114)
  call void @dx.op.tempRegStore.f32(i32 1, i32 21, float %115)
  %116 = call float @dx.op.tempRegLoad.f32(i32 0, i32 15)
  %117 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %118 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %119 = fmul fast float %116, %117
  %120 = fmul fast float %116, %118
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %119)
  call void @dx.op.tempRegStore.f32(i32 1, i32 21, float %120)
  %121 = call float @dx.op.tempRegLoad.f32(i32 0, i32 9)
  %122 = fsub fast float -0.000000e+00, %121
  call void @dx.op.tempRegStore.f32(i32 1, i32 10, float %122)
  %123 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %124 = fsub fast float -0.000000e+00, %123
  call void @dx.op.tempRegStore.f32(i32 1, i32 22, float %124)
  %125 = call float @dx.op.tempRegLoad.f32(i32 0, i32 8)
  %126 = call float @dx.op.tempRegLoad.f32(i32 0, i32 10)
  %127 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %128 = call float @dx.op.tempRegLoad.f32(i32 0, i32 22)
  %129 = call float @dx.op.dot2.f32(i32 54, float %125, float %126, float %127, float %128)
  call void @dx.op.tempRegStore.f32(i32 1, i32 13, float %129)
  %130 = call float @dx.op.tempRegLoad.f32(i32 0, i32 6)
  %131 = call float @dx.op.tempRegLoad.f32(i32 0, i32 7)
  %132 = call float @dx.op.tempRegLoad.f32(i32 0, i32 4)
  %133 = call float @dx.op.tempRegLoad.f32(i32 0, i32 5)
  %134 = fadd fast float %130, %132
  %135 = fadd fast float %131, %133
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %134)
  call void @dx.op.tempRegStore.f32(i32 1, i32 21, float %135)
  %136 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  %137 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %138 = add i32 %136, -1
  %139 = add i32 %137, -2
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %138)
  call void @dx.op.tempRegStore.i32(i32 1, i32 2, i32 %139)
  %140 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %141 = icmp sge i32 0, %140
  %142 = sext i1 %141 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %142)
  %143 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %144 = icmp ne i32 %143, 0
  br i1 %144, label %if1.then, label %if1.end

if1.then:                                         ; preds = %if0.end
  %145 = call float @dx.op.tempRegLoad.f32(i32 0, i32 13)
  %146 = call float @dx.op.tempRegLoad.f32(i32 0, i32 15)
  %147 = call float @dx.op.binary.f32(i32 35, float %145, float -6.550400e+04)
  %148 = call float @dx.op.binary.f32(i32 35, float %146, float -6.550400e+04)
  call void @dx.op.tempRegStore.f32(i32 1, i32 1, float %147)
  call void @dx.op.tempRegStore.f32(i32 1, i32 3, float %148)
  %149 = call float @dx.op.tempRegLoad.f32(i32 0, i32 1)
  %150 = call float @dx.op.tempRegLoad.f32(i32 0, i32 3)
  %151 = call float @dx.op.binary.f32(i32 36, float %149, float 6.550400e+04)
  %152 = call float @dx.op.binary.f32(i32 36, float %150, float 6.550400e+04)
  call void @dx.op.tempRegStore.f32(i32 1, i32 1, float %151)
  call void @dx.op.tempRegStore.f32(i32 1, i32 3, float %152)
  %153 = call float @dx.op.tempRegLoad.f32(i32 0, i32 1)
  %154 = call float @dx.op.tempRegLoad.f32(i32 0, i32 3)
  %155 = call i32 @dx.op.legacyF32ToF16(i32 130, float %153)
  %156 = call i32 @dx.op.legacyF32ToF16(i32 130, float %154)
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %155)
  call void @dx.op.tempRegStore.i32(i32 1, i32 3, i32 %156)
  %157 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %158 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %159 = call i32 @dx.op.tertiary.i32(i32 48, i32 %157, i32 65536, i32 %158)
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %159)
  %160 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %161 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %2, i32 %161, i32 0, i32 %160, i32 undef, i32 undef, i32 undef, i8 1)
  %162 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 12)
  %163 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %164 = add i32 %162, %163
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %164)
  br label %if1.end

if1.end:                                          ; preds = %if1.then, %if0.end
  %165 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 14)
  %166 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %167 = add i32 %165, %166
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %167)
  %168 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %169 = fsub fast float -0.000000e+00, %168
  %170 = fadd fast float %169, 1.000000e+00
  call void @dx.op.tempRegStore.f32(i32 1, i32 22, float %170)
  %171 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %172 = call float @dx.op.tempRegLoad.f32(i32 0, i32 22)
  %173 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %3, i32 0)
  %174 = extractvalue %dx.types.CBufRet.f32 %173, 0
  %175 = extractvalue %dx.types.CBufRet.f32 %173, 1
  %176 = fmul fast float %171, %174
  %177 = fmul fast float %172, %175
  call void @dx.op.tempRegStore.f32(i32 1, i32 1, float %176)
  call void @dx.op.tempRegStore.f32(i32 1, i32 3, float %177)
  %178 = call float @dx.op.tempRegLoad.f32(i32 0, i32 1)
  %179 = call float @dx.op.tempRegLoad.f32(i32 0, i32 3)
  %180 = fptosi float %178 to i32
  %181 = fptosi float %179 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %180)
  call void @dx.op.tempRegStore.i32(i32 1, i32 3, i32 %181)
  %182 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %183 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %184 = call i32 @dx.op.binary.i32(i32 37, i32 %182, i32 0)
  %185 = call i32 @dx.op.binary.i32(i32 37, i32 %183, i32 0)
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %184)
  call void @dx.op.tempRegStore.i32(i32 1, i32 3, i32 %185)
  %186 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 16)
  %187 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 17)
  %188 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %189 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %190 = call i32 @dx.op.binary.i32(i32 38, i32 %186, i32 %188)
  %191 = call i32 @dx.op.binary.i32(i32 38, i32 %187, i32 %189)
  call void @dx.op.tempRegStore.i32(i32 1, i32 24, i32 %190)
  call void @dx.op.tempRegStore.i32(i32 1, i32 25, i32 %191)
  call void @dx.op.tempRegStore.i32(i32 1, i32 26, i32 0)
  call void @dx.op.tempRegStore.i32(i32 1, i32 27, i32 0)
  %192 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 24)
  %193 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 25)
  %194 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 27)
  %195 = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %0, i32 %194, i32 %192, i32 %193, i32 undef, i32 0, i32 0, i32 undef)
  %196 = extractvalue %dx.types.ResRet.f32 %195, 0
  call void @dx.op.tempRegStore.f32(i32 1, i32 1, float %196)
  %197 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 24)
  %198 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 25)
  %199 = uitofp i32 %197 to float
  %200 = uitofp i32 %198 to float
  call void @dx.op.tempRegStore.f32(i32 1, i32 4, float %199)
  call void @dx.op.tempRegStore.f32(i32 1, i32 5, float %200)
  %201 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %202 = extractvalue %dx.types.CBufRet.f32 %201, 2
  %203 = call float @dx.op.tempRegLoad.f32(i32 0, i32 4)
  %204 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 1)
  %205 = extractvalue %dx.types.CBufRet.f32 %204, 0
  %206 = call float @dx.op.tertiary.f32(i32 46, float %202, float %203, float %205)
  call void @dx.op.tempRegStore.f32(i32 1, i32 24, float %206)
  %207 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %208 = extractvalue %dx.types.CBufRet.f32 %207, 3
  %209 = fsub fast float -0.000000e+00, %208
  %210 = call float @dx.op.tempRegLoad.f32(i32 0, i32 5)
  %211 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 1)
  %212 = extractvalue %dx.types.CBufRet.f32 %211, 1
  %213 = fsub fast float -0.000000e+00, %212
  %214 = call float @dx.op.tertiary.f32(i32 46, float %209, float %210, float %213)
  call void @dx.op.tempRegStore.f32(i32 1, i32 25, float %214)
  %215 = call float @dx.op.tempRegLoad.f32(i32 0, i32 1)
  %216 = call float @dx.op.tempRegLoad.f32(i32 0, i32 24)
  %217 = call float @dx.op.tempRegLoad.f32(i32 0, i32 25)
  %218 = fmul fast float %215, %216
  %219 = fmul fast float %215, %217
  call void @dx.op.tempRegStore.f32(i32 1, i32 24, float %218)
  call void @dx.op.tempRegStore.f32(i32 1, i32 25, float %219)
  %220 = call float @dx.op.tempRegLoad.f32(i32 0, i32 25)
  %221 = fsub fast float -0.000000e+00, %220
  call void @dx.op.tempRegStore.f32(i32 1, i32 26, float %221)
  %222 = call float @dx.op.tempRegLoad.f32(i32 0, i32 8)
  %223 = call float @dx.op.tempRegLoad.f32(i32 0, i32 10)
  %224 = call float @dx.op.tempRegLoad.f32(i32 0, i32 24)
  %225 = call float @dx.op.tempRegLoad.f32(i32 0, i32 26)
  %226 = call float @dx.op.dot2.f32(i32 54, float %222, float %223, float %224, float %225)
  call void @dx.op.tempRegStore.f32(i32 1, i32 3, float %226)
  %227 = call float @dx.op.tempRegLoad.f32(i32 0, i32 6)
  %228 = call float @dx.op.tempRegLoad.f32(i32 0, i32 7)
  %229 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %230 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %231 = fadd fast float %227, %229
  %232 = fadd fast float %228, %230
  call void @dx.op.tempRegStore.f32(i32 1, i32 4, float %231)
  call void @dx.op.tempRegStore.f32(i32 1, i32 5, float %232)
  %233 = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 0)
  %234 = shl i32 %233, 2
  call void @dx.op.tempRegStore.i32(i32 1, i32 9, i32 %234)
  call void @dx.op.tempRegStore.i32(i32 1, i32 22, i32 0)
  call void @dx.op.tempRegStore.i32(i32 1, i32 23, i32 0)
  call void @dx.op.tempRegStore.i32(i32 1, i32 26, i32 0)
  call void @dx.op.tempRegStore.i32(i32 1, i32 27, i32 0)
  %235 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 4)
  %236 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 5)
  call void @dx.op.tempRegStore.i32(i32 1, i32 18, i32 %235)
  call void @dx.op.tempRegStore.i32(i32 1, i32 19, i32 %236)
  %237 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %238 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  call void @dx.op.tempRegStore.i32(i32 1, i32 28, i32 %237)
  call void @dx.op.tempRegStore.i32(i32 1, i32 29, i32 %238)
  %239 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 13)
  %240 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 15)
  call void @dx.op.tempRegStore.i32(i32 1, i32 32, i32 %239)
  call void @dx.op.tempRegStore.i32(i32 1, i32 33, i32 %240)
  call void @dx.op.tempRegStore.i32(i32 1, i32 30, i32 1176256512)
  call void @dx.op.tempRegStore.i32(i32 1, i32 31, i32 -1082130432)
  %241 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  call void @dx.op.tempRegStore.i32(i32 1, i32 11, i32 %241)
  %242 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  call void @dx.op.tempRegStore.i32(i32 1, i32 14, i32 %242)
  call void @dx.op.tempRegStore.i32(i32 1, i32 34, i32 1)
  br label %loop0

loop0:                                            ; preds = %if3.end, %if1.end
  %243 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 11)
  %244 = icmp sge i32 0, %243
  %245 = sext i1 %244 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 35, i32 %245)
  %246 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 35)
  %247 = icmp ne i32 %246, 0
  br i1 %247, label %loop0.end, label %loop0.breakc0

loop0.breakc0:                                    ; preds = %loop0
  %248 = call float @dx.op.tempRegLoad.f32(i32 0, i32 18)
  %249 = call float @dx.op.tempRegLoad.f32(i32 0, i32 19)
  %250 = call float @dx.op.tertiary.f32(i32 46, float %248, float 1.000000e+00, float 0.000000e+00)
  %251 = call float @dx.op.tertiary.f32(i32 46, float %249, float -1.000000e+00, float 1.000000e+00)
  call void @dx.op.tempRegStore.f32(i32 1, i32 36, float %250)
  call void @dx.op.tempRegStore.f32(i32 1, i32 37, float %251)
  %252 = call float @dx.op.tempRegLoad.f32(i32 0, i32 36)
  %253 = call float @dx.op.tempRegLoad.f32(i32 0, i32 37)
  %254 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %3, i32 0)
  %255 = extractvalue %dx.types.CBufRet.f32 %254, 0
  %256 = extractvalue %dx.types.CBufRet.f32 %254, 1
  %257 = fmul fast float %252, %255
  %258 = fmul fast float %253, %256
  call void @dx.op.tempRegStore.f32(i32 1, i32 36, float %257)
  call void @dx.op.tempRegStore.f32(i32 1, i32 37, float %258)
  %259 = call float @dx.op.tempRegLoad.f32(i32 0, i32 36)
  %260 = call float @dx.op.tempRegLoad.f32(i32 0, i32 37)
  %261 = fptosi float %259 to i32
  %262 = fptosi float %260 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 36, i32 %261)
  call void @dx.op.tempRegStore.i32(i32 1, i32 37, i32 %262)
  %263 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 36)
  %264 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 37)
  %265 = call i32 @dx.op.binary.i32(i32 37, i32 %263, i32 0)
  %266 = call i32 @dx.op.binary.i32(i32 37, i32 %264, i32 0)
  call void @dx.op.tempRegStore.i32(i32 1, i32 36, i32 %265)
  call void @dx.op.tempRegStore.i32(i32 1, i32 37, i32 %266)
  %267 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 16)
  %268 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 17)
  %269 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 36)
  %270 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 37)
  %271 = call i32 @dx.op.binary.i32(i32 38, i32 %267, i32 %269)
  %272 = call i32 @dx.op.binary.i32(i32 38, i32 %268, i32 %270)
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %271)
  call void @dx.op.tempRegStore.i32(i32 1, i32 21, i32 %272)
  %273 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %274 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 21)
  %275 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 23)
  %276 = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %0, i32 %275, i32 %273, i32 %274, i32 undef, i32 0, i32 0, i32 undef)
  %277 = extractvalue %dx.types.ResRet.f32 %276, 0
  call void @dx.op.tempRegStore.f32(i32 1, i32 37, float %277)
  %278 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %279 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 21)
  %280 = uitofp i32 %278 to float
  %281 = uitofp i32 %279 to float
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %280)
  call void @dx.op.tempRegStore.f32(i32 1, i32 21, float %281)
  %282 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %283 = extractvalue %dx.types.CBufRet.f32 %282, 2
  %284 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %285 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 1)
  %286 = extractvalue %dx.types.CBufRet.f32 %285, 0
  %287 = call float @dx.op.tertiary.f32(i32 46, float %283, float %284, float %286)
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %287)
  %288 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %289 = extractvalue %dx.types.CBufRet.f32 %288, 3
  %290 = fsub fast float -0.000000e+00, %289
  %291 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %292 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 1)
  %293 = extractvalue %dx.types.CBufRet.f32 %292, 1
  %294 = fsub fast float -0.000000e+00, %293
  %295 = call float @dx.op.tertiary.f32(i32 46, float %290, float %291, float %294)
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %295)
  %296 = call float @dx.op.tempRegLoad.f32(i32 0, i32 37)
  %297 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %298 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %299 = fmul fast float %296, %297
  %300 = fmul fast float %296, %298
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %299)
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %300)
  %301 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %302 = fsub fast float -0.000000e+00, %301
  call void @dx.op.tempRegStore.f32(i32 1, i32 42, float %302)
  %303 = call float @dx.op.tempRegLoad.f32(i32 0, i32 8)
  %304 = call float @dx.op.tempRegLoad.f32(i32 0, i32 10)
  %305 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %306 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %307 = call float @dx.op.dot2.f32(i32 54, float %303, float %304, float %305, float %306)
  call void @dx.op.tempRegStore.f32(i32 1, i32 36, float %307)
  %308 = call float @dx.op.tempRegLoad.f32(i32 0, i32 6)
  %309 = fsub fast float -0.000000e+00, %308
  %310 = call float @dx.op.tempRegLoad.f32(i32 0, i32 7)
  %311 = fsub fast float -0.000000e+00, %310
  %312 = call float @dx.op.tempRegLoad.f32(i32 0, i32 18)
  %313 = call float @dx.op.tempRegLoad.f32(i32 0, i32 19)
  %314 = call float @dx.op.tertiary.f32(i32 46, float %309, float 5.000000e-01, float %312)
  %315 = call float @dx.op.tertiary.f32(i32 46, float %311, float 5.000000e-01, float %313)
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %314)
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %315)
  %316 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %317 = fsub fast float -0.000000e+00, %316
  %318 = fadd fast float %317, 1.000000e+00
  call void @dx.op.tempRegStore.f32(i32 1, i32 42, float %318)
  %319 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %320 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %321 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %3, i32 0)
  %322 = extractvalue %dx.types.CBufRet.f32 %321, 0
  %323 = extractvalue %dx.types.CBufRet.f32 %321, 1
  %324 = fmul fast float %319, %322
  %325 = fmul fast float %320, %323
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %324)
  call void @dx.op.tempRegStore.f32(i32 1, i32 21, float %325)
  %326 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %327 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %328 = fptosi float %326 to i32
  %329 = fptosi float %327 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %328)
  call void @dx.op.tempRegStore.i32(i32 1, i32 21, i32 %329)
  %330 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %331 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 21)
  %332 = call i32 @dx.op.binary.i32(i32 37, i32 %330, i32 0)
  %333 = call i32 @dx.op.binary.i32(i32 37, i32 %331, i32 0)
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %332)
  call void @dx.op.tempRegStore.i32(i32 1, i32 21, i32 %333)
  %334 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 16)
  %335 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 17)
  %336 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %337 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 21)
  %338 = call i32 @dx.op.binary.i32(i32 38, i32 %334, i32 %336)
  %339 = call i32 @dx.op.binary.i32(i32 38, i32 %335, i32 %337)
  call void @dx.op.tempRegStore.i32(i32 1, i32 24, i32 %338)
  call void @dx.op.tempRegStore.i32(i32 1, i32 25, i32 %339)
  %340 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 24)
  %341 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 25)
  %342 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 27)
  %343 = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle %0, i32 %342, i32 %340, i32 %341, i32 undef, i32 0, i32 0, i32 undef)
  %344 = extractvalue %dx.types.ResRet.f32 %343, 0
  call void @dx.op.tempRegStore.f32(i32 1, i32 21, float %344)
  %345 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 24)
  %346 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 25)
  %347 = uitofp i32 %345 to float
  %348 = uitofp i32 %346 to float
  call void @dx.op.tempRegStore.f32(i32 1, i32 24, float %347)
  call void @dx.op.tempRegStore.f32(i32 1, i32 25, float %348)
  %349 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %350 = extractvalue %dx.types.CBufRet.f32 %349, 2
  %351 = call float @dx.op.tempRegLoad.f32(i32 0, i32 24)
  %352 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 1)
  %353 = extractvalue %dx.types.CBufRet.f32 %352, 0
  %354 = call float @dx.op.tertiary.f32(i32 46, float %350, float %351, float %353)
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %354)
  %355 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %356 = extractvalue %dx.types.CBufRet.f32 %355, 3
  %357 = fsub fast float -0.000000e+00, %356
  %358 = call float @dx.op.tempRegLoad.f32(i32 0, i32 25)
  %359 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 1)
  %360 = extractvalue %dx.types.CBufRet.f32 %359, 1
  %361 = fsub fast float -0.000000e+00, %360
  %362 = call float @dx.op.tertiary.f32(i32 46, float %357, float %358, float %361)
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %362)
  %363 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %364 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %365 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %366 = fmul fast float %363, %364
  %367 = fmul fast float %363, %365
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %366)
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %367)
  %368 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %369 = fsub fast float -0.000000e+00, %368
  call void @dx.op.tempRegStore.f32(i32 1, i32 42, float %369)
  %370 = call float @dx.op.tempRegLoad.f32(i32 0, i32 8)
  %371 = call float @dx.op.tempRegLoad.f32(i32 0, i32 10)
  %372 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %373 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %374 = call float @dx.op.dot2.f32(i32 54, float %370, float %371, float %372, float %373)
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %374)
  %375 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %376 = call float @dx.op.tempRegLoad.f32(i32 0, i32 37)
  %377 = fcmp fast olt float %375, %376
  %378 = sext i1 %377 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 24, i32 %378)
  %379 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 24)
  %380 = icmp ne i32 %379, 0
  %381 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %382 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 21)
  %383 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 36)
  %384 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 37)
  %385 = select i1 %380, i32 %381, i32 %383
  %386 = select i1 %380, i32 %382, i32 %384
  call void @dx.op.tempRegStore.i32(i32 1, i32 37, i32 %385)
  call void @dx.op.tempRegStore.i32(i32 1, i32 38, i32 %386)
  %387 = call float @dx.op.tempRegLoad.f32(i32 0, i32 38)
  %388 = fmul fast float %387, 0x3FEFD70A40000000
  %389 = fmul fast float %387, 0x3FEFD70A40000000
  call void @dx.op.tempRegStore.f32(i32 1, i32 36, float %388)
  call void @dx.op.tempRegStore.f32(i32 1, i32 39, float %389)
  %390 = call float @dx.op.tempRegLoad.f32(i32 0, i32 37)
  %391 = fsub fast float -0.000000e+00, %390
  %392 = call float @dx.op.tempRegLoad.f32(i32 0, i32 39)
  %393 = fsub fast float -0.000000e+00, %392
  %394 = call float @dx.op.tempRegLoad.f32(i32 0, i32 37)
  %395 = fsub fast float -0.000000e+00, %394
  %396 = call float @dx.op.tempRegLoad.f32(i32 0, i32 39)
  %397 = fsub fast float -0.000000e+00, %396
  %398 = call float @dx.op.dot2.f32(i32 54, float %391, float %393, float %395, float %397)
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %398)
  %399 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %400 = call float @dx.op.unary.f32(i32 25, float %399)
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %400)
  %401 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %402 = call float @dx.op.tempRegLoad.f32(i32 0, i32 37)
  %403 = fsub fast float -0.000000e+00, %402
  %404 = call float @dx.op.tempRegLoad.f32(i32 0, i32 39)
  %405 = fsub fast float -0.000000e+00, %404
  %406 = fmul fast float %401, %403
  %407 = fmul fast float %401, %405
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %406)
  call void @dx.op.tempRegStore.f32(i32 1, i32 21, float %407)
  %408 = call float @dx.op.tempRegLoad.f32(i32 0, i32 6)
  %409 = call float @dx.op.tempRegLoad.f32(i32 0, i32 7)
  %410 = call float @dx.op.tempRegLoad.f32(i32 0, i32 18)
  %411 = call float @dx.op.tempRegLoad.f32(i32 0, i32 19)
  %412 = fadd fast float %408, %410
  %413 = fadd fast float %409, %411
  call void @dx.op.tempRegStore.f32(i32 1, i32 18, float %412)
  call void @dx.op.tempRegStore.f32(i32 1, i32 19, float %413)
  %414 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 11)
  %415 = add i32 %414, -1
  call void @dx.op.tempRegStore.i32(i32 1, i32 11, i32 %415)
  %416 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 12)
  %417 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 14)
  %418 = add i32 %416, %417
  call void @dx.op.tempRegStore.i32(i32 1, i32 14, i32 %418)
  %419 = call float @dx.op.tempRegLoad.f32(i32 0, i32 28)
  %420 = call float @dx.op.tempRegLoad.f32(i32 0, i32 29)
  %421 = call float @dx.op.tempRegLoad.f32(i32 0, i32 37)
  %422 = fsub fast float -0.000000e+00, %421
  %423 = call float @dx.op.tempRegLoad.f32(i32 0, i32 39)
  %424 = fsub fast float -0.000000e+00, %423
  %425 = fadd fast float %419, %422
  %426 = fadd fast float %420, %424
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %425)
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %426)
  %427 = call float @dx.op.tempRegLoad.f32(i32 0, i32 32)
  %428 = call float @dx.op.tempRegLoad.f32(i32 0, i32 33)
  %429 = call float @dx.op.tempRegLoad.f32(i32 0, i32 37)
  %430 = fsub fast float -0.000000e+00, %429
  %431 = call float @dx.op.tempRegLoad.f32(i32 0, i32 39)
  %432 = fsub fast float -0.000000e+00, %431
  %433 = fadd fast float %427, %430
  %434 = fadd fast float %428, %432
  call void @dx.op.tempRegStore.f32(i32 1, i32 42, float %433)
  call void @dx.op.tempRegStore.f32(i32 1, i32 43, float %434)
  %435 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %436 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %437 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %438 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %439 = call float @dx.op.dot2.f32(i32 54, float %435, float %436, float %437, float %438)
  call void @dx.op.tempRegStore.f32(i32 1, i32 25, float %439)
  %440 = call float @dx.op.tempRegLoad.f32(i32 0, i32 25)
  %441 = call float @dx.op.unary.f32(i32 25, float %440)
  call void @dx.op.tempRegStore.f32(i32 1, i32 35, float %441)
  %442 = call float @dx.op.tempRegLoad.f32(i32 0, i32 35)
  %443 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %444 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %445 = fmul fast float %442, %443
  %446 = fmul fast float %442, %444
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %445)
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %446)
  %447 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %448 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %449 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %450 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %451 = call float @dx.op.dot2.f32(i32 54, float %447, float %448, float %449, float %450)
  call void @dx.op.tempRegStore.f32(i32 1, i32 35, float %451)
  %452 = call float @dx.op.tempRegLoad.f32(i32 0, i32 35)
  %453 = fadd fast float %452, 1.000000e+00
  call void @dx.op.tempRegStore.f32(i32 1, i32 35, float %453)
  %454 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %455 = call float @dx.op.tempRegLoad.f32(i32 0, i32 43)
  %456 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %457 = call float @dx.op.tempRegLoad.f32(i32 0, i32 43)
  %458 = call float @dx.op.dot2.f32(i32 54, float %454, float %455, float %456, float %457)
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %458)
  %459 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %460 = call float @dx.op.unary.f32(i32 25, float %459)
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %460)
  %461 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %462 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %463 = call float @dx.op.tempRegLoad.f32(i32 0, i32 43)
  %464 = fmul fast float %461, %462
  %465 = fmul fast float %461, %463
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %464)
  call void @dx.op.tempRegStore.f32(i32 1, i32 42, float %465)
  %466 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %467 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %468 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %469 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %470 = call float @dx.op.dot2.f32(i32 54, float %466, float %467, float %468, float %469)
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %470)
  %471 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %472 = fadd fast float %471, 1.000000e+00
  call void @dx.op.tempRegStore.f32(i32 1, i32 41, float %472)
  %473 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %474 = extractvalue %dx.types.CBufRet.f32 %473, 1
  %475 = call float @dx.op.tempRegLoad.f32(i32 0, i32 25)
  %476 = call float @dx.op.tempRegLoad.f32(i32 0, i32 36)
  %477 = call float @dx.op.tertiary.f32(i32 46, float %474, float %475, float %476)
  call void @dx.op.tempRegStore.f32(i32 1, i32 25, float %477)
  %478 = call float @dx.op.tempRegLoad.f32(i32 0, i32 36)
  %479 = call float @dx.op.tempRegLoad.f32(i32 0, i32 25)
  %480 = fdiv fast float %478, %479
  call void @dx.op.tempRegStore.f32(i32 1, i32 25, float %480)
  %481 = call float @dx.op.tempRegLoad.f32(i32 0, i32 25)
  %482 = call float @dx.op.binary.f32(i32 36, float %481, float 0x3FECCCCCC0000000)
  call void @dx.op.tempRegStore.f32(i32 1, i32 25, float %482)
  %483 = call float @dx.op.tempRegLoad.f32(i32 0, i32 25)
  %484 = call float @dx.op.tempRegLoad.f32(i32 0, i32 35)
  %485 = fmul fast float %483, %484
  call void @dx.op.tempRegStore.f32(i32 1, i32 25, float %485)
  %486 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %487 = extractvalue %dx.types.CBufRet.f32 %486, 1
  %488 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %489 = call float @dx.op.tempRegLoad.f32(i32 0, i32 36)
  %490 = call float @dx.op.tertiary.f32(i32 46, float %487, float %488, float %489)
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %490)
  %491 = call float @dx.op.tempRegLoad.f32(i32 0, i32 36)
  %492 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %493 = fdiv fast float %491, %492
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %493)
  %494 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %495 = call float @dx.op.binary.f32(i32 36, float %494, float 0x3FECCCCCC0000000)
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %495)
  %496 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %497 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %498 = fmul fast float %496, %497
  call void @dx.op.tempRegStore.f32(i32 1, i32 40, float %498)
  %499 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 34)
  %500 = icmp ne i32 %499, 0
  %501 = sext i1 %500 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 42, i32 %501)
  %502 = call float @dx.op.tempRegLoad.f32(i32 0, i32 40)
  %503 = call float @dx.op.tempRegLoad.f32(i32 0, i32 25)
  %504 = fcmp fast oge float %502, %503
  %505 = sext i1 %504 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 25, i32 %505)
  %506 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 25)
  %507 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 42)
  %508 = and i32 %506, %507
  call void @dx.op.tempRegStore.i32(i32 1, i32 25, i32 %508)
  %509 = call float @dx.op.tempRegLoad.f32(i32 0, i32 41)
  %510 = call float @dx.op.tempRegLoad.f32(i32 0, i32 35)
  %511 = fcmp fast oge float %509, %510
  %512 = sext i1 %511 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 35, i32 %512)
  %513 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 25)
  %514 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 35)
  %515 = and i32 %513, %514
  call void @dx.op.tempRegStore.i32(i32 1, i32 25, i32 %515)
  %516 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 25)
  %517 = icmp ne i32 %516, 0
  br i1 %517, label %if2.then, label %if2.else

if2.then:                                         ; preds = %loop0.breakc0
  %518 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 34)
  %519 = add i32 %518, -1
  call void @dx.op.tempRegStore.i32(i32 1, i32 25, i32 %519)
  %520 = call float @dx.op.tempRegLoad.f32(i32 0, i32 30)
  %521 = call float @dx.op.tempRegLoad.f32(i32 0, i32 31)
  %522 = call float @dx.op.tempRegLoad.f32(i32 0, i32 37)
  %523 = fsub fast float -0.000000e+00, %522
  %524 = call float @dx.op.tempRegLoad.f32(i32 0, i32 39)
  %525 = fsub fast float -0.000000e+00, %524
  %526 = fadd fast float %520, %523
  %527 = fadd fast float %521, %525
  call void @dx.op.tempRegStore.f32(i32 1, i32 42, float %526)
  call void @dx.op.tempRegStore.f32(i32 1, i32 43, float %527)
  %528 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %529 = call float @dx.op.tempRegLoad.f32(i32 0, i32 43)
  %530 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %531 = call float @dx.op.tempRegLoad.f32(i32 0, i32 43)
  %532 = call float @dx.op.dot2.f32(i32 54, float %528, float %529, float %530, float %531)
  call void @dx.op.tempRegStore.f32(i32 1, i32 35, float %532)
  %533 = call float @dx.op.tempRegLoad.f32(i32 0, i32 35)
  %534 = call float @dx.op.unary.f32(i32 25, float %533)
  call void @dx.op.tempRegStore.f32(i32 1, i32 39, float %534)
  %535 = call float @dx.op.tempRegLoad.f32(i32 0, i32 39)
  %536 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %537 = call float @dx.op.tempRegLoad.f32(i32 0, i32 43)
  %538 = fmul fast float %535, %536
  %539 = fmul fast float %535, %537
  call void @dx.op.tempRegStore.f32(i32 1, i32 42, float %538)
  call void @dx.op.tempRegStore.f32(i32 1, i32 43, float %539)
  %540 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %541 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %542 = call float @dx.op.tempRegLoad.f32(i32 0, i32 42)
  %543 = call float @dx.op.tempRegLoad.f32(i32 0, i32 43)
  %544 = call float @dx.op.dot2.f32(i32 54, float %540, float %541, float %542, float %543)
  call void @dx.op.tempRegStore.f32(i32 1, i32 39, float %544)
  %545 = call float @dx.op.tempRegLoad.f32(i32 0, i32 39)
  %546 = fadd fast float %545, 1.000000e+00
  call void @dx.op.tempRegStore.f32(i32 1, i32 39, float %546)
  %547 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %548 = extractvalue %dx.types.CBufRet.f32 %547, 1
  %549 = call float @dx.op.tempRegLoad.f32(i32 0, i32 35)
  %550 = call float @dx.op.tempRegLoad.f32(i32 0, i32 36)
  %551 = call float @dx.op.tertiary.f32(i32 46, float %548, float %549, float %550)
  call void @dx.op.tempRegStore.f32(i32 1, i32 35, float %551)
  %552 = call float @dx.op.tempRegLoad.f32(i32 0, i32 36)
  %553 = call float @dx.op.tempRegLoad.f32(i32 0, i32 35)
  %554 = fdiv fast float %552, %553
  call void @dx.op.tempRegStore.f32(i32 1, i32 35, float %554)
  %555 = call float @dx.op.tempRegLoad.f32(i32 0, i32 35)
  %556 = call float @dx.op.binary.f32(i32 36, float %555, float 0x3FECCCCCC0000000)
  call void @dx.op.tempRegStore.f32(i32 1, i32 35, float %556)
  %557 = call float @dx.op.tempRegLoad.f32(i32 0, i32 35)
  %558 = call float @dx.op.tempRegLoad.f32(i32 0, i32 39)
  %559 = fmul fast float %557, %558
  call void @dx.op.tempRegStore.f32(i32 1, i32 35, float %559)
  %560 = call float @dx.op.tempRegLoad.f32(i32 0, i32 38)
  %561 = fmul fast float %560, 0x3FEFD70A40000000
  call void @dx.op.tempRegStore.f32(i32 1, i32 45, float %561)
  %562 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 32)
  %563 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 33)
  call void @dx.op.tempRegStore.i32(i32 1, i32 42, i32 %562)
  call void @dx.op.tempRegStore.i32(i32 1, i32 43, i32 %563)
  %564 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 30)
  %565 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 31)
  call void @dx.op.tempRegStore.i32(i32 1, i32 46, i32 %564)
  call void @dx.op.tempRegStore.i32(i32 1, i32 47, i32 %565)
  %566 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 41)
  %567 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 40)
  call void @dx.op.tempRegStore.i32(i32 1, i32 48, i32 %566)
  call void @dx.op.tempRegStore.i32(i32 1, i32 49, i32 %567)
  %568 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 39)
  call void @dx.op.tempRegStore.i32(i32 1, i32 52, i32 %568)
  %569 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 35)
  call void @dx.op.tempRegStore.i32(i32 1, i32 53, i32 %569)
  %570 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 37)
  call void @dx.op.tempRegStore.i32(i32 1, i32 44, i32 %570)
  %571 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 25)
  call void @dx.op.tempRegStore.i32(i32 1, i32 34, i32 %571)
  call void @dx.op.tempRegStore.i32(i32 1, i32 50, i32 6)
  br label %loop1

loop1:                                            ; preds = %loop1.breakc0, %if2.then
  %572 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 50)
  %573 = icmp ne i32 %572, 0
  %574 = sext i1 %573 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 51, i32 %574)
  %575 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 34)
  %576 = icmp ne i32 %575, 0
  %577 = sext i1 %576 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 54, i32 %577)
  %578 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 51)
  %579 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 54)
  %580 = and i32 %578, %579
  call void @dx.op.tempRegStore.i32(i32 1, i32 51, i32 %580)
  %581 = call float @dx.op.tempRegLoad.f32(i32 0, i32 53)
  %582 = call float @dx.op.tempRegLoad.f32(i32 0, i32 52)
  %583 = call float @dx.op.tempRegLoad.f32(i32 0, i32 49)
  %584 = call float @dx.op.tempRegLoad.f32(i32 0, i32 48)
  %585 = fcmp fast oge float %581, %583
  %586 = fcmp fast oge float %582, %584
  %587 = sext i1 %585 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 54, i32 %587)
  %588 = sext i1 %586 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 55, i32 %588)
  %589 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 51)
  %590 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 54)
  %591 = and i32 %589, %590
  call void @dx.op.tempRegStore.i32(i32 1, i32 51, i32 %591)
  %592 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 55)
  %593 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 51)
  %594 = and i32 %592, %593
  call void @dx.op.tempRegStore.i32(i32 1, i32 51, i32 %594)
  %595 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 51)
  %596 = icmp eq i32 %595, 0
  br i1 %596, label %loop1.end, label %loop1.breakc0

loop1.breakc0:                                    ; preds = %loop1
  %597 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 34)
  %598 = add i32 %597, -1
  call void @dx.op.tempRegStore.i32(i32 1, i32 34, i32 %598)
  %599 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 34)
  %600 = and i32 %599, 7
  call void @dx.op.tempRegStore.i32(i32 1, i32 51, i32 %600)
  %601 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 51)
  %602 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 9)
  %603 = mul i32 %601, 256
  %604 = add i32 %603, %602
  %605 = getelementptr [2048 x i8], [2048 x i8] addrspace(3)* @TGSM0, i32 0, i32 %604
  %606 = bitcast i8 addrspace(3)* %605 to float addrspace(3)*
  %607 = load float, float addrspace(3)* %606, align 4
  call void @dx.op.tempRegStore.f32(i32 1, i32 51, float %607)
  %608 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 51)
  %609 = and i32 %608, 65535
  call void @dx.op.tempRegStore.i32(i32 1, i32 54, i32 %609)
  %610 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 51)
  %611 = lshr i32 %610, 16
  call void @dx.op.tempRegStore.i32(i32 1, i32 51, i32 %611)
  %612 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 54)
  %613 = call float @dx.op.legacyF16ToF32(i32 131, i32 %612)
  call void @dx.op.tempRegStore.f32(i32 1, i32 56, float %613)
  %614 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 51)
  %615 = call float @dx.op.legacyF16ToF32(i32 131, i32 %614)
  call void @dx.op.tempRegStore.f32(i32 1, i32 57, float %615)
  %616 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 50)
  %617 = add i32 %616, -1
  call void @dx.op.tempRegStore.i32(i32 1, i32 50, i32 %617)
  %618 = call float @dx.op.tempRegLoad.f32(i32 0, i32 44)
  %619 = fsub fast float -0.000000e+00, %618
  %620 = call float @dx.op.tempRegLoad.f32(i32 0, i32 45)
  %621 = fsub fast float -0.000000e+00, %620
  %622 = call float @dx.op.tempRegLoad.f32(i32 0, i32 56)
  %623 = call float @dx.op.tempRegLoad.f32(i32 0, i32 57)
  %624 = fadd fast float %619, %622
  %625 = fadd fast float %621, %623
  call void @dx.op.tempRegStore.f32(i32 1, i32 54, float %624)
  call void @dx.op.tempRegStore.f32(i32 1, i32 55, float %625)
  %626 = call float @dx.op.tempRegLoad.f32(i32 0, i32 54)
  %627 = call float @dx.op.tempRegLoad.f32(i32 0, i32 55)
  %628 = call float @dx.op.tempRegLoad.f32(i32 0, i32 54)
  %629 = call float @dx.op.tempRegLoad.f32(i32 0, i32 55)
  %630 = call float @dx.op.dot2.f32(i32 54, float %626, float %627, float %628, float %629)
  call void @dx.op.tempRegStore.f32(i32 1, i32 51, float %630)
  %631 = call float @dx.op.tempRegLoad.f32(i32 0, i32 51)
  %632 = call float @dx.op.unary.f32(i32 25, float %631)
  call void @dx.op.tempRegStore.f32(i32 1, i32 58, float %632)
  %633 = call float @dx.op.tempRegLoad.f32(i32 0, i32 54)
  %634 = call float @dx.op.tempRegLoad.f32(i32 0, i32 55)
  %635 = call float @dx.op.tempRegLoad.f32(i32 0, i32 58)
  %636 = fmul fast float %633, %635
  %637 = fmul fast float %634, %635
  call void @dx.op.tempRegStore.f32(i32 1, i32 54, float %636)
  call void @dx.op.tempRegStore.f32(i32 1, i32 55, float %637)
  %638 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %639 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %640 = call float @dx.op.tempRegLoad.f32(i32 0, i32 54)
  %641 = call float @dx.op.tempRegLoad.f32(i32 0, i32 55)
  %642 = call float @dx.op.dot2.f32(i32 54, float %638, float %639, float %640, float %641)
  call void @dx.op.tempRegStore.f32(i32 1, i32 54, float %642)
  %643 = call float @dx.op.tempRegLoad.f32(i32 0, i32 54)
  %644 = fadd fast float %643, 1.000000e+00
  call void @dx.op.tempRegStore.f32(i32 1, i32 60, float %644)
  %645 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %4, i32 0)
  %646 = extractvalue %dx.types.CBufRet.f32 %645, 1
  %647 = call float @dx.op.tempRegLoad.f32(i32 0, i32 51)
  %648 = call float @dx.op.tempRegLoad.f32(i32 0, i32 36)
  %649 = call float @dx.op.tertiary.f32(i32 46, float %646, float %647, float %648)
  call void @dx.op.tempRegStore.f32(i32 1, i32 51, float %649)
  %650 = call float @dx.op.tempRegLoad.f32(i32 0, i32 36)
  %651 = call float @dx.op.tempRegLoad.f32(i32 0, i32 51)
  %652 = fdiv fast float %650, %651
  call void @dx.op.tempRegStore.f32(i32 1, i32 51, float %652)
  %653 = call float @dx.op.tempRegLoad.f32(i32 0, i32 51)
  %654 = call float @dx.op.binary.f32(i32 36, float %653, float 0x3FECCCCCC0000000)
  call void @dx.op.tempRegStore.f32(i32 1, i32 51, float %654)
  %655 = call float @dx.op.tempRegLoad.f32(i32 0, i32 51)
  %656 = call float @dx.op.tempRegLoad.f32(i32 0, i32 60)
  %657 = fmul fast float %655, %656
  call void @dx.op.tempRegStore.f32(i32 1, i32 61, float %657)
  %658 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 46)
  %659 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 47)
  call void @dx.op.tempRegStore.i32(i32 1, i32 42, i32 %658)
  call void @dx.op.tempRegStore.i32(i32 1, i32 43, i32 %659)
  %660 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 56)
  %661 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 57)
  call void @dx.op.tempRegStore.i32(i32 1, i32 46, i32 %660)
  call void @dx.op.tempRegStore.i32(i32 1, i32 47, i32 %661)
  %662 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 52)
  %663 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 53)
  call void @dx.op.tempRegStore.i32(i32 1, i32 48, i32 %662)
  call void @dx.op.tempRegStore.i32(i32 1, i32 49, i32 %663)
  %664 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 60)
  %665 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 61)
  call void @dx.op.tempRegStore.i32(i32 1, i32 52, i32 %664)
  call void @dx.op.tempRegStore.i32(i32 1, i32 53, i32 %665)
  br label %loop1

loop1.end:                                        ; preds = %loop1
  %666 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 42)
  %667 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 43)
  call void @dx.op.tempRegStore.i32(i32 1, i32 32, i32 %666)
  call void @dx.op.tempRegStore.i32(i32 1, i32 33, i32 %667)
  %668 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 46)
  %669 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 47)
  call void @dx.op.tempRegStore.i32(i32 1, i32 30, i32 %668)
  call void @dx.op.tempRegStore.i32(i32 1, i32 31, i32 %669)
  br label %if2.end

if2.else:                                         ; preds = %loop0.breakc0
  %670 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 32)
  %671 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 33)
  call void @dx.op.tempRegStore.i32(i32 1, i32 30, i32 %670)
  call void @dx.op.tempRegStore.i32(i32 1, i32 31, i32 %671)
  call void @dx.op.tempRegStore.i32(i32 1, i32 50, i32 7)
  %672 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 28)
  %673 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 29)
  call void @dx.op.tempRegStore.i32(i32 1, i32 32, i32 %672)
  call void @dx.op.tempRegStore.i32(i32 1, i32 33, i32 %673)
  br label %if2.end

if2.end:                                          ; preds = %if2.else, %loop1.end
  %674 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 50)
  %675 = icmp eq i32 %674, 7
  %676 = sext i1 %675 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %676)
  %677 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %678 = icmp ne i32 %677, 0
  br i1 %678, label %if3.then, label %if3.end

if3.then:                                         ; preds = %if2.end
  %679 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 34)
  %680 = and i32 %679, 7
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %680)
  %681 = call float @dx.op.tempRegLoad.f32(i32 0, i32 30)
  %682 = call float @dx.op.tempRegLoad.f32(i32 0, i32 31)
  %683 = call i32 @dx.op.legacyF32ToF16(i32 130, float %681)
  %684 = call i32 @dx.op.legacyF32ToF16(i32 130, float %682)
  call void @dx.op.tempRegStore.i32(i32 1, i32 36, i32 %683)
  call void @dx.op.tempRegStore.i32(i32 1, i32 39, i32 %684)
  %685 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 39)
  %686 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 36)
  %687 = call i32 @dx.op.tertiary.i32(i32 48, i32 %685, i32 65536, i32 %686)
  call void @dx.op.tempRegStore.i32(i32 1, i32 21, i32 %687)
  %688 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %689 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 9)
  %690 = mul i32 %688, 256
  %691 = add i32 %690, %689
  %692 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %693 = getelementptr [2048 x i8], [2048 x i8] addrspace(3)* @TGSM0, i32 0, i32 %691
  %694 = bitcast i8 addrspace(3)* %693 to float addrspace(3)*
  store float %692, float addrspace(3)* %694, align 4
  br label %if3.end

if3.end:                                          ; preds = %if3.then, %if2.end
  %695 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 34)
  %696 = add i32 %695, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 34, i32 %696)
  %697 = call float @dx.op.tempRegLoad.f32(i32 0, i32 38)
  %698 = call float @dx.op.tempRegLoad.f32(i32 0, i32 33)
  %699 = fcmp fast olt float %697, %698
  %700 = sext i1 %699 to i32
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %700)
  %701 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %702 = icmp ne i32 %701, 0
  %703 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 37)
  %704 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 38)
  %705 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 32)
  %706 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 33)
  %707 = select i1 %702, i32 %703, i32 %705
  %708 = select i1 %702, i32 %704, i32 %706
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %707)
  call void @dx.op.tempRegStore.i32(i32 1, i32 21, i32 %708)
  %709 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 24)
  %710 = icmp ne i32 %709, 0
  %711 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %712 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 21)
  %713 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 32)
  %714 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 33)
  %715 = select i1 %710, i32 %711, i32 %713
  %716 = select i1 %710, i32 %712, i32 %714
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %715)
  call void @dx.op.tempRegStore.i32(i32 1, i32 21, i32 %716)
  %717 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %718 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %719 = call float @dx.op.binary.f32(i32 35, float %717, float -6.550400e+04)
  %720 = call float @dx.op.binary.f32(i32 35, float %718, float -6.550400e+04)
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %719)
  call void @dx.op.tempRegStore.f32(i32 1, i32 21, float %720)
  %721 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %722 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %723 = call float @dx.op.binary.f32(i32 36, float %721, float 6.550400e+04)
  %724 = call float @dx.op.binary.f32(i32 36, float %722, float 6.550400e+04)
  call void @dx.op.tempRegStore.f32(i32 1, i32 20, float %723)
  call void @dx.op.tempRegStore.f32(i32 1, i32 21, float %724)
  %725 = call float @dx.op.tempRegLoad.f32(i32 0, i32 20)
  %726 = call float @dx.op.tempRegLoad.f32(i32 0, i32 21)
  %727 = call i32 @dx.op.legacyF32ToF16(i32 130, float %725)
  %728 = call i32 @dx.op.legacyF32ToF16(i32 130, float %726)
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %727)
  call void @dx.op.tempRegStore.i32(i32 1, i32 21, i32 %728)
  %729 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 21)
  %730 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %731 = call i32 @dx.op.tertiary.i32(i32 48, i32 %729, i32 65536, i32 %730)
  call void @dx.op.tempRegStore.i32(i32 1, i32 20, i32 %731)
  %732 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 20)
  %733 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 14)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %2, i32 %733, i32 0, i32 %732, i32 undef, i32 undef, i32 undef, i8 1)
  %734 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 37)
  %735 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 38)
  call void @dx.op.tempRegStore.i32(i32 1, i32 28, i32 %734)
  call void @dx.op.tempRegStore.i32(i32 1, i32 29, i32 %735)
  br label %loop0

loop0.end:                                        ; preds = %loop0
  ret void
}

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.tertiary.i32(i32, i32, i32, i32) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.groupId.i32(i32, i32) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.threadIdInGroup.i32(i32, i32) #1

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.i32(i32, i32, i32) #2

; Function Attrs: nounwind readonly
declare i32 @dx.op.tempRegLoad.i32(i32, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32) #0

; Function Attrs: nounwind readonly
declare float @dx.op.tempRegLoad.f32(i32, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.f32(i32, i32, float) #2

; Function Attrs: nounwind readnone
declare float @dx.op.dot2.f32(i32, float, float, float, float) #1

; Function Attrs: nounwind readnone
declare float @dx.op.unary.f32(i32, float) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.quaternary.i32(i32, i32, i32, i32, i32) #1

; Function Attrs: nounwind
declare void @dx.op.bufferStore.i32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i8) #2

; Function Attrs: nounwind readnone
declare float @dx.op.tertiary.f32(i32, float, float, float) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.binary.i32(i32, i32, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i32) #0

; Function Attrs: nounwind readnone
declare float @dx.op.binary.f32(i32, float, float) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.legacyF32ToF16(i32, float) #1

; Function Attrs: nounwind readnone
declare float @dx.op.legacyF16ToF32(i32, i32) #1

attributes #0 = { nounwind readonly }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.resources = !{!2}
!dx.entryPoints = !{!14}
!llvm.ident = !{!17}

!0 = !{i32 1, i32 0}
!1 = !{!"cs", i32 6, i32 0}
!2 = !{!3, !8, !11, null}
!3 = !{!4, !6}
!4 = !{i32 0, %dx.types.f32 addrspace(1)* undef, !"T0", i32 0, i32 0, i32 1, i32 2, i32 0, !5}
!5 = !{i32 0, i32 9}
!6 = !{i32 1, %dx.types.i8x48 addrspace(1)* undef, !"T1", i32 0, i32 1, i32 1, i32 12, i32 0, !7}
!7 = !{i32 1, i32 48}
!8 = !{!9}
!9 = !{i32 0, %dx.types.i8x4 addrspace(1)* undef, !"U0", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !10}
!10 = !{i32 1, i32 4}
!11 = !{!12, !13}
!12 = !{i32 0, %dx.types.i8x16 addrspace(2)* undef, !"CB0", i32 0, i32 0, i32 1, i32 16, null}
!13 = !{i32 1, %dx.types.i8x32 addrspace(2)* undef, !"CB1", i32 0, i32 1, i32 1, i32 32, null}
!14 = !{void ()* @main, !"main", null, !2, !15}
!15 = !{i32 4, !16}
!16 = !{i32 64, i32 1, i32 1}
!17 = !{!"dxbc2dxil 1.0"}
