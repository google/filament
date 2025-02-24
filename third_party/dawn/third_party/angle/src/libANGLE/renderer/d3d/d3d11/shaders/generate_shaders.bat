@ECHO OFF
REM
REM Copyright 2013 The ANGLE Project Authors. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.
REM

PATH %ProgramFiles(x86)%\Windows Kits\8.1\bin\x86;%DXSDK_DIR%\Utilities\bin\x86;%PATH%

setlocal
set errorCount=0
set successCount=0
set debug=0

if "%1" == "debug" (
    set debug=1
)
if "%1" == "release" (
    set debug=0
)

:: Shaders for OpenGL ES 2.0 and OpenGL ES 3.0+
::              | Input file         | Entry point            | Type            | Output file                        | Debug |
call:BuildShader Passthrough2D11.hlsl VS_Passthrough2D         vs_4_0_level_9_3  compiled\passthrough2d11vs.h         %debug%
call:BuildShader Passthrough2D11.hlsl PS_PassthroughRGBA2D     ps_4_0_level_9_3  compiled\passthroughrgba2d11ps.h     %debug%
call:BuildShader Passthrough2D11.hlsl PS_PassthroughA2D        ps_4_0_level_9_3  compiled\passthrougha2d11ps.h        %debug%
call:BuildShader Passthrough2D11.hlsl PS_PassthroughRGBA2DMS   ps_4_1            compiled\passthroughrgba2dms11ps.h   %debug%
call:BuildShader Passthrough2D11.hlsl PS_PassthroughRGB2D      ps_4_0_level_9_3  compiled\passthroughrgb2d11ps.h      %debug%
call:BuildShader Passthrough2D11.hlsl PS_PassthroughRG2D       ps_4_0_level_9_3  compiled\passthroughrg2d11ps.h       %debug%
call:BuildShader Passthrough2D11.hlsl PS_PassthroughR2D        ps_4_0_level_9_3  compiled\passthroughr2d11ps.h        %debug%
call:BuildShader Passthrough2D11.hlsl PS_PassthroughLum2D      ps_4_0_level_9_3  compiled\passthroughlum2d11ps.h      %debug%
call:BuildShader Passthrough2D11.hlsl PS_PassthroughLumAlpha2D ps_4_0_level_9_3  compiled\passthroughlumalpha2d11ps.h %debug%

call:BuildShader Passthrough2D11.hlsl PS_PassthroughRGBA2D_4444 ps_4_0_level_9_3 compiled\passthroughrgba2d_4444_11ps.h %debug%
call:BuildShader Passthrough2D11.hlsl PS_PassthroughRGB2D_565   ps_4_0_level_9_3 compiled\passthroughrgb2d_565_11ps.h   %debug%
call:BuildShader Passthrough2D11.hlsl PS_PassthroughRGBA2D_5551 ps_4_0_level_9_3 compiled\passthroughrgba2d_5551_11ps.h %debug%

call:BuildShader ResolveColor.hlsl PS_ResolveColor2D ps_4_1 compiled\resolvecolor2dps.h %debug%

call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGBA_2D ps_4_0 compiled\multiplyalpha_ftof_pm_rgba_2d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGBA_2D ps_4_0 compiled\multiplyalpha_ftof_um_rgba_2d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGB_2D  ps_4_0 compiled\multiplyalpha_ftof_pm_rgb_2d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGB_2D  ps_4_0 compiled\multiplyalpha_ftof_um_rgb_2d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PT_RGBA_2D ps_4_0 compiled\multiplyalpha_ftou_pt_rgba_2d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PM_RGBA_2D ps_4_0 compiled\multiplyalpha_ftou_pm_rgba_2d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_UM_RGBA_2D ps_4_0 compiled\multiplyalpha_ftou_um_rgba_2d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PT_RGB_2D  ps_4_0 compiled\multiplyalpha_ftou_pt_rgb_2d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PM_RGB_2D  ps_4_0 compiled\multiplyalpha_ftou_pm_rgb_2d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_UM_RGB_2D  ps_4_0 compiled\multiplyalpha_ftou_um_rgb_2d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_LUMA_2D ps_4_0 compiled\multiplyalpha_ftof_pm_luma_2d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_LUMA_2D ps_4_0 compiled\multiplyalpha_ftof_um_luma_2d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_LUMAALPHA_2D   ps_4_0 compiled\multiplyalpha_ftof_pm_lumaalpha_2d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_LUMAALPHA_2D   ps_4_0 compiled\multiplyalpha_ftof_um_lumaalpha_2d_ps.h  %debug%

call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGBA_4444_2D ps_4_0 compiled\multiplyalpha_ftof_pm_rgba_4444_2d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGBA_4444_2D ps_4_0 compiled\multiplyalpha_ftof_um_rgba_4444_2d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGB_565_2D   ps_4_0 compiled\multiplyalpha_ftof_pm_rgb_565_2d_ps.h   %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGB_565_2D   ps_4_0 compiled\multiplyalpha_ftof_um_rgb_565_2d_ps.h   %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGBA_5551_2D ps_4_0 compiled\multiplyalpha_ftof_pm_rgba_5551_2d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGBA_5551_2D ps_4_0 compiled\multiplyalpha_ftof_um_rgba_5551_2d_ps.h %debug%

call:BuildShader Clear11.hlsl           VS_Clear                 vs_4_0            compiled\clear11vs.h                 %debug%
call:BuildShader Clear11.hlsl           VS_Multiview_Clear                 vs_4_0            compiled\clear11multiviewvs.h                 %debug%
call:BuildShader Clear11.hlsl           GS_Multiview_Clear                 gs_4_0            compiled\clear11multiviewgs.h                 %debug%
call:BuildShader Clear11.hlsl           PS_ClearDepth            ps_4_0            compiled\cleardepth11ps.h            %debug%
call:BuildShader Clear11.hlsl           PS_ClearFloat1           ps_4_0            compiled\clearfloat11ps1.h           %debug%
call:BuildShader Clear11.hlsl           PS_ClearFloat2           ps_4_0            compiled\clearfloat11ps2.h           %debug%
call:BuildShader Clear11.hlsl           PS_ClearFloat3           ps_4_0            compiled\clearfloat11ps3.h           %debug%
call:BuildShader Clear11.hlsl           PS_ClearFloat4           ps_4_0            compiled\clearfloat11ps4.h           %debug%
call:BuildShader Clear11.hlsl           PS_ClearFloat5           ps_4_0            compiled\clearfloat11ps5.h           %debug%
call:BuildShader Clear11.hlsl           PS_ClearFloat6           ps_4_0            compiled\clearfloat11ps6.h           %debug%
call:BuildShader Clear11.hlsl           PS_ClearFloat7           ps_4_0            compiled\clearfloat11ps7.h           %debug%
call:BuildShader Clear11.hlsl           PS_ClearFloat8           ps_4_0            compiled\clearfloat11ps8.h           %debug%

:: Shaders for OpenGL ES 3.0+ only
::              | Input file               | Entry point            | Type   | Output file                        | Debug |
call:BuildShader Passthrough2D11.hlsl       PS_PassthroughDepth2D    ps_4_0   compiled\passthroughdepth2d11ps.h    %debug%
call:BuildShader Passthrough2D11.hlsl       PS_PassthroughRGBA2DUI   ps_4_0   compiled\passthroughrgba2dui11ps.h   %debug%
call:BuildShader Passthrough2D11.hlsl       PS_PassthroughRGBA2DI    ps_4_0   compiled\passthroughrgba2di11ps.h    %debug%
call:BuildShader Passthrough2D11.hlsl       PS_PassthroughRGB2DUI    ps_4_0   compiled\passthroughrgb2dui11ps.h    %debug%
call:BuildShader Passthrough2D11.hlsl       PS_PassthroughRGB2DI     ps_4_0   compiled\passthroughrgb2di11ps.h     %debug%
call:BuildShader Passthrough2D11.hlsl       PS_PassthroughRG2DUI     ps_4_0   compiled\passthroughrg2dui11ps.h     %debug%
call:BuildShader Passthrough2D11.hlsl       PS_PassthroughRG2DI      ps_4_0   compiled\passthroughrg2di11ps.h      %debug%
call:BuildShader Passthrough2D11.hlsl       PS_PassthroughR2DUI      ps_4_0   compiled\passthroughr2dui11ps.h      %debug%
call:BuildShader Passthrough2D11.hlsl       PS_PassthroughR2DI       ps_4_0   compiled\passthroughr2di11ps.h       %debug%

call:BuildShader Passthrough3D11.hlsl       VS_Passthrough3D         vs_4_0   compiled\passthrough3d11vs.h         %debug%
call:BuildShader Passthrough3D11.hlsl       GS_Passthrough3D         gs_4_0   compiled\passthrough3d11gs.h         %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughRGBA3D     ps_4_0   compiled\passthroughrgba3d11ps.h     %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughRGBA3DUI   ps_4_0   compiled\passthroughrgba3dui11ps.h   %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughRGBA3DI    ps_4_0   compiled\passthroughrgba3di11ps.h    %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughRGB3D      ps_4_0   compiled\passthroughrgb3d11ps.h      %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughRGB3DUI    ps_4_0   compiled\passthroughrgb3dui11ps.h    %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughRGB3DI     ps_4_0   compiled\passthroughrgb3di11ps.h     %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughRG3D       ps_4_0   compiled\passthroughrg3d11ps.h       %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughRG3DUI     ps_4_0   compiled\passthroughrg3dui11ps.h     %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughRG3DI      ps_4_0   compiled\passthroughrg3di11ps.h      %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughR3D        ps_4_0   compiled\passthroughr3d11ps.h        %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughR3DUI      ps_4_0   compiled\passthroughr3dui11ps.h      %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughR3DI       ps_4_0   compiled\passthroughr3di11ps.h       %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughLum3D      ps_4_0   compiled\passthroughlum3d11ps.h      %debug%
call:BuildShader Passthrough3D11.hlsl       PS_PassthroughLumAlpha3D ps_4_0   compiled\passthroughlumalpha3d11ps.h %debug%

call:BuildShader Passthrough3D11.hlsl PS_PassthroughRGBA3D_4444 ps_4_0 compiled\passthroughrgba3d_4444_11ps.h %debug%
call:BuildShader Passthrough3D11.hlsl PS_PassthroughRGB3D_565   ps_4_0 compiled\passthroughrgb3d_565_11ps.h   %debug%
call:BuildShader Passthrough3D11.hlsl PS_PassthroughRGBA3D_5551 ps_4_0 compiled\passthroughrgba3d_5551_11ps.h %debug%

call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughRGBA2DArray     ps_4_0   compiled\passthroughrgba2darray11ps.h     %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughRGBA2DArrayUI   ps_4_0   compiled\passthroughrgba2darrayui11ps.h   %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughRGBA2DArrayI    ps_4_0   compiled\passthroughrgba2darrayi11ps.h    %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughRGB2DArray      ps_4_0   compiled\passthroughrgb2darray11ps.h      %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughRGB2DArrayUI    ps_4_0   compiled\passthroughrgb2darrayui11ps.h    %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughRGB2DArrayI     ps_4_0   compiled\passthroughrgb2darrayi11ps.h     %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughRG2DArray       ps_4_0   compiled\passthroughrg2darray11ps.h       %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughRG2DArrayUI     ps_4_0   compiled\passthroughrg2darrayui11ps.h     %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughRG2DArrayI      ps_4_0   compiled\passthroughrg2darrayi11ps.h      %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughR2DArray        ps_4_0   compiled\passthroughr2darray11ps.h        %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughR2DArrayUI      ps_4_0   compiled\passthroughr2darrayui11ps.h      %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughR2DArrayI       ps_4_0   compiled\passthroughr2darrayi11ps.h       %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughLum2DArray      ps_4_0   compiled\passthroughlum2darray11ps.h      %debug%
call:BuildShader Passthrough2DArray11.hlsl       PS_PassthroughLumAlpha2DArray ps_4_0   compiled\passthroughlumalpha2darray11ps.h %debug%

call:BuildShader Passthrough2DArray11.hlsl PS_PassthroughRGBA2DArray_4444 ps_4_0 compiled\passthroughrgba2darray_4444_11ps.h %debug%
call:BuildShader Passthrough2DArray11.hlsl PS_PassthroughRGB2DArray_565   ps_4_0 compiled\passthroughrgb2darray_565_11ps.h   %debug%
call:BuildShader Passthrough2DArray11.hlsl PS_PassthroughRGBA2DArray_5551 ps_4_0 compiled\passthroughrgba2darray_5551_11ps.h %debug%

call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGBA_3D ps_4_0 compiled\multiplyalpha_ftof_pm_rgba_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGBA_3D ps_4_0 compiled\multiplyalpha_ftof_um_rgba_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGB_3D  ps_4_0 compiled\multiplyalpha_ftof_pm_rgb_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGB_3D  ps_4_0 compiled\multiplyalpha_ftof_um_rgb_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PT_RGBA_3D ps_4_0 compiled\multiplyalpha_ftou_pt_rgba_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PM_RGBA_3D ps_4_0 compiled\multiplyalpha_ftou_pm_rgba_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_UM_RGBA_3D ps_4_0 compiled\multiplyalpha_ftou_um_rgba_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PT_RGB_3D  ps_4_0 compiled\multiplyalpha_ftou_pt_rgb_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PM_RGB_3D  ps_4_0 compiled\multiplyalpha_ftou_pm_rgb_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_UM_RGB_3D  ps_4_0 compiled\multiplyalpha_ftou_um_rgb_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_PT_RGBA_3D ps_4_0 compiled\multiplyalpha_ftoi_pt_rgba_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_PM_RGBA_3D ps_4_0 compiled\multiplyalpha_ftoi_pm_rgba_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_UM_RGBA_3D ps_4_0 compiled\multiplyalpha_ftoi_um_rgba_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_PT_RGB_3D  ps_4_0 compiled\multiplyalpha_ftoi_pt_rgb_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_PM_RGB_3D  ps_4_0 compiled\multiplyalpha_ftoi_pm_rgb_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_UM_RGB_3D  ps_4_0 compiled\multiplyalpha_ftoi_um_rgb_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_LUMA_3D ps_4_0 compiled\multiplyalpha_ftof_pm_luma_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_LUMA_3D ps_4_0 compiled\multiplyalpha_ftof_um_luma_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_LUMAALPHA_3D   ps_4_0 compiled\multiplyalpha_ftof_pm_lumaalpha_3d_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_LUMAALPHA_3D   ps_4_0 compiled\multiplyalpha_ftof_um_lumaalpha_3d_ps.h  %debug%

call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGBA_4444_3D ps_4_0 compiled\multiplyalpha_ftof_pm_rgba_4444_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGBA_4444_3D ps_4_0 compiled\multiplyalpha_ftof_um_rgba_4444_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGB_565_3D   ps_4_0 compiled\multiplyalpha_ftof_pm_rgb_565_3d_ps.h   %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGB_565_3D   ps_4_0 compiled\multiplyalpha_ftof_um_rgb_565_3d_ps.h   %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGBA_5551_3D ps_4_0 compiled\multiplyalpha_ftof_pm_rgba_5551_3d_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGBA_5551_3D ps_4_0 compiled\multiplyalpha_ftof_um_rgba_5551_3d_ps.h %debug%

call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGBA_2DArray ps_4_0 compiled\multiplyalpha_ftof_pm_rgba_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGBA_2DArray ps_4_0 compiled\multiplyalpha_ftof_um_rgba_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGB_2DArray  ps_4_0 compiled\multiplyalpha_ftof_pm_rgb_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGB_2DArray  ps_4_0 compiled\multiplyalpha_ftof_um_rgb_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PT_RGBA_2DArray ps_4_0 compiled\multiplyalpha_ftou_pt_rgba_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PM_RGBA_2DArray ps_4_0 compiled\multiplyalpha_ftou_pm_rgba_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_UM_RGBA_2DArray ps_4_0 compiled\multiplyalpha_ftou_um_rgba_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PT_RGB_2DArray  ps_4_0 compiled\multiplyalpha_ftou_pt_rgb_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_PM_RGB_2DArray  ps_4_0 compiled\multiplyalpha_ftou_pm_rgb_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoU_UM_RGB_2DArray  ps_4_0 compiled\multiplyalpha_ftou_um_rgb_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_PT_RGBA_2DArray ps_4_0 compiled\multiplyalpha_ftoi_pt_rgba_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_PM_RGBA_2DArray ps_4_0 compiled\multiplyalpha_ftoi_pm_rgba_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_UM_RGBA_2DArray ps_4_0 compiled\multiplyalpha_ftoi_um_rgba_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_PT_RGB_2DArray  ps_4_0 compiled\multiplyalpha_ftoi_pt_rgb_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_PM_RGB_2DArray  ps_4_0 compiled\multiplyalpha_ftoi_pm_rgb_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoI_UM_RGB_2DArray  ps_4_0 compiled\multiplyalpha_ftoi_um_rgb_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_LUMA_2DArray ps_4_0 compiled\multiplyalpha_ftof_pm_luma_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_LUMA_2DArray ps_4_0 compiled\multiplyalpha_ftof_um_luma_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_LUMAALPHA_2DArray   ps_4_0 compiled\multiplyalpha_ftof_pm_lumaalpha_2darray_ps.h  %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_LUMAALPHA_2DArray   ps_4_0 compiled\multiplyalpha_ftof_um_lumaalpha_2darray_ps.h  %debug%

call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGBA_4444_2DArray ps_4_0 compiled\multiplyalpha_ftof_pm_rgba_4444_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGBA_4444_2DArray ps_4_0 compiled\multiplyalpha_ftof_um_rgba_4444_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGB_565_2DArray   ps_4_0 compiled\multiplyalpha_ftof_pm_rgb_565_2darray_ps.h   %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGB_565_2DArray   ps_4_0 compiled\multiplyalpha_ftof_um_rgb_565_2darray_ps.h   %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_PM_RGBA_5551_2DArray ps_4_0 compiled\multiplyalpha_ftof_pm_rgba_5551_2darray_ps.h %debug%
call:BuildShader MultiplyAlpha.hlsl PS_FtoF_UM_RGBA_5551_2DArray ps_4_0 compiled\multiplyalpha_ftof_um_rgba_5551_2darray_ps.h %debug%

call:BuildShader Swizzle11.hlsl             PS_SwizzleF2D            ps_4_0   compiled\swizzlef2dps.h              %debug%
call:BuildShader Swizzle11.hlsl             PS_SwizzleI2D            ps_4_0   compiled\swizzlei2dps.h              %debug%
call:BuildShader Swizzle11.hlsl             PS_SwizzleUI2D           ps_4_0   compiled\swizzleui2dps.h             %debug%

call:BuildShader Swizzle11.hlsl             PS_SwizzleF3D            ps_4_0   compiled\swizzlef3dps.h              %debug%
call:BuildShader Swizzle11.hlsl             PS_SwizzleI3D            ps_4_0   compiled\swizzlei3dps.h              %debug%
call:BuildShader Swizzle11.hlsl             PS_SwizzleUI3D           ps_4_0   compiled\swizzleui3dps.h             %debug%

call:BuildShader Swizzle11.hlsl             PS_SwizzleF2DArray       ps_4_0   compiled\swizzlef2darrayps.h         %debug%
call:BuildShader Swizzle11.hlsl             PS_SwizzleI2DArray       ps_4_0   compiled\swizzlei2darrayps.h         %debug%
call:BuildShader Swizzle11.hlsl             PS_SwizzleUI2DArray      ps_4_0   compiled\swizzleui2darrayps.h        %debug%

call:BuildShader Clear11.hlsl               PS_ClearUint1            ps_4_0   compiled\clearuint11ps1.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearUint2            ps_4_0   compiled\clearuint11ps2.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearUint3            ps_4_0   compiled\clearuint11ps3.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearUint4            ps_4_0   compiled\clearuint11ps4.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearUint5            ps_4_0   compiled\clearuint11ps5.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearUint6            ps_4_0   compiled\clearuint11ps6.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearUint7            ps_4_0   compiled\clearuint11ps7.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearUint8            ps_4_0   compiled\clearuint11ps8.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearSint1            ps_4_0   compiled\clearsint11ps1.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearSint2            ps_4_0   compiled\clearsint11ps2.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearSint3            ps_4_0   compiled\clearsint11ps3.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearSint4            ps_4_0   compiled\clearsint11ps4.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearSint5            ps_4_0   compiled\clearsint11ps5.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearSint6            ps_4_0   compiled\clearsint11ps6.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearSint7            ps_4_0   compiled\clearsint11ps7.h            %debug%
call:BuildShader Clear11.hlsl               PS_ClearSint8            ps_4_0   compiled\clearsint11ps8.h            %debug%

call:BuildShader BufferToTexture11.hlsl     VS_BufferToTexture       vs_4_0   compiled/buffertotexture11_vs.h      %debug%
call:BuildShader BufferToTexture11.hlsl     GS_BufferToTexture       gs_4_0   compiled/buffertotexture11_gs.h      %debug%
call:BuildShader BufferToTexture11.hlsl     PS_BufferToTexture_4F    ps_4_0   compiled/buffertotexture11_ps_4f.h   %debug%
call:BuildShader BufferToTexture11.hlsl     PS_BufferToTexture_4I    ps_4_0   compiled/buffertotexture11_ps_4i.h   %debug%
call:BuildShader BufferToTexture11.hlsl     PS_BufferToTexture_4UI   ps_4_0   compiled/buffertotexture11_ps_4ui.h  %debug%

call:BuildShader ResolveDepthStencil.hlsl   VS_ResolveDepthStencil   vs_4_1   compiled/resolvedepthstencil11_vs.h  %debug%
call:BuildShader ResolveDepthStencil.hlsl   PS_ResolveDepth          ps_4_1   compiled/resolvedepth11_ps.h         %debug%
call:BuildShader ResolveDepthStencil.hlsl   PS_ResolveDepthStencil   ps_4_1   compiled/resolvedepthstencil11_ps.h  %debug%
call:BuildShader ResolveDepthStencil.hlsl   PS_ResolveStencil        ps_4_1   compiled/resolvestencil11_ps.h       %debug%

echo.

if %successCount% GTR 0 (
   echo %successCount% shaders compiled successfully.
)
if %errorCount% GTR 0 (
   echo There were %errorCount% shader compilation errors.
)

endlocal
exit /b

:BuildShader
set input=%~1
set entry=%~2
set type=%~3
set output=%~4
set debug=%~5

if %debug% == 0 (
    set "buildCMD=fxc /nologo /E %entry% /T %type% /Fh %output% %input%"
) else (
    set "buildCMD=fxc /nologo /Zi /Od /E %entry% /T %type% /Fh %output% %input%"
)

set error=0
%buildCMD% || set error=1

if %error% == 0 (
    set /a successCount=%successCount%+1
) else (
    set /a errorCount=%errorCount%+1
)

exit /b
