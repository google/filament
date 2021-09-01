//==============================================================================================================================
// An optimized AMD FSR's EASU implementation for Mobiles
// Based on https://github.com/GPUOpen-Effects/FidelityFX-FSR/blob/master/ffx-fsr/ffx_fsr1.h
// Details can be found: https://atyuwen.github.io/posts/optimizing-fsr/
// Distributed under the MIT License.
// -- FsrEasuSampleH should be implemented by calling shader, like following:
//    half3 FsrEasuSampleH(float2 p) { return MyTex.SampleLevel(LinearSampler, p, 0).xyz; }
//==============================================================================================================================

#define rcp(x) (1.0/(x))
#define rsqrt inversesqrt

AF3 FsrEasuSampleF(AF2 p);

void FsrEasuL(
        out AF3 pix,
        highp AF2 ip,
        highp AF4 con0,
        highp AF4 con1,
        highp AF4 con2,
        highp AF4 con3){
    //------------------------------------------------------------------------------------------------------------------------------
    // Direction is the '+' diff.
    //    A
    //  B C D
    //    E
    AF2 pp=(ip)*(con0.xy)+(con0.zw);
    AF2 tc=(pp+AF2_(0.5))*con1.xy;
    AF3 sA=FsrEasuSampleF(tc-AF2(0, con1.y));
    AF3 sB=FsrEasuSampleF(tc-AF2(con1.x, 0));
    AF3 sC=FsrEasuSampleF(tc);
    AF3 sD=FsrEasuSampleF(tc+AF2(con1.x, 0));
    AF3 sE=FsrEasuSampleF(tc+AF2(0, con1.y));
    AF1 lA=sA.r*AF1_(0.5)+sA.g;
    AF1 lB=sB.r*AF1_(0.5)+sB.g;
    AF1 lC=sC.r*AF1_(0.5)+sC.g;
    AF1 lD=sD.r*AF1_(0.5)+sD.g;
    AF1 lE=sE.r*AF1_(0.5)+sE.g;
    // Then takes magnitude from abs average of both sides of 'C'.
    // Length converts gradient reversal to 0, smoothly to non-reversal at 1, shaped, then adding horz and vert terms.
    AF1 dc=lD-lC;
    AF1 cb=lC-lB;
    AF1 lenX=max(abs(dc),abs(cb));
    lenX=ARcpF1(lenX);
    AF1 dirX=lD-lB;
    lenX=ASatF1(abs(dirX)*lenX);
    lenX*=lenX;
    // Repeat for the y axis.
    AF1 ec=lE-lC;
    AF1 ca=lC-lA;
    AF1 lenY=max(abs(ec),abs(ca));
    lenY=ARcpF1(lenY);
    AF1 dirY=lE-lA;
    lenY=ASatF1(abs(dirY)*lenY);
    AF1 len = lenY * lenY + lenX;
    AF2 dir = AF2(dirX, dirY);
    //------------------------------------------------------------------------------------------------------------------------------ 
    AF2 dir2=dir*dir;
    AF1 dirR=dir2.x+dir2.y;
    if (dirR<AF1_(1.0/64.0)) {
        pix = sC;
        return;
    }
    dirR=rsqrt(dirR);
    dir*=AF2_(dirR);
    len=len*AF1_(0.5);
    len*=len;
    AF1 stretch=(dir.x*dir.x+dir.y*dir.y)*rcp(max(abs(dir.x),abs(dir.y)));
    AF2 len2=AF2(AF1_(1.0)+(stretch-AF1_(1.0))*len,AF1_(1.0)+AF1_(-0.5)*len);
    AF1 lob=AF1_(0.5)+AF1_((1.0/4.0-0.04)-0.5)*len;
    AF1 clp=rcp(lob);
    //------------------------------------------------------------------------------------------------------------------------------
    AF2 fp=floor(pp);
    pp-=fp;
    AF2 ppp=AF2(pp);
    AF2 p0=fp*(con1.xy)+(con1.zw);
    AF2 p1=p0+(con2.xy);
    AF2 p2=p0+(con2.zw);
    AF2 p3=p0+(con3.xy);
    p0.y-=con1.w; p3.y+=con1.w;
    AF4 bczzR=FsrEasuRF(p0);
    AF4 bczzG=FsrEasuGF(p0);
    AF4 bczzB=FsrEasuBF(p0);
    AF4 ijfeR=FsrEasuRF(p1);
    AF4 ijfeG=FsrEasuGF(p1);
    AF4 ijfeB=FsrEasuBF(p1);
    AF4 klhgR=FsrEasuRF(p2);
    AF4 klhgG=FsrEasuGF(p2);
    AF4 klhgB=FsrEasuBF(p2);
    AF4 zzonR=FsrEasuRF(p3);
    AF4 zzonG=FsrEasuGF(p3);
    AF4 zzonB=FsrEasuBF(p3);
    //------------------------------------------------------------------------------------------------------------------------------
    // This part is different for FP16, working pairs of taps at a time.

    AF3 min4=min(AMin3F3(AF3(ijfeR.z,ijfeG.z,ijfeB.z),AF3(klhgR.w,klhgG.w,klhgB.w),AF3(ijfeR.y,ijfeG.y,ijfeB.y)),
            AF3(klhgR.x,klhgG.x,klhgB.x));
    AF3 max4=max(AMax3F3(AF3(ijfeR.z,ijfeG.z,ijfeB.z),AF3(klhgR.w,klhgG.w,klhgB.w),AF3(ijfeR.y,ijfeG.y,ijfeB.y)),
            AF3(klhgR.x,klhgG.x,klhgB.x));

    AF3 aC=AF3_(0.0);
    AF1 aW=AF1_(0.0);
    FsrEasuTapF(aC,aW,AF2( 0.0,-1.0)-pp,dir,len2,lob,clp,AF3(bczzR.x,bczzG.x,bczzB.x)); // b
    FsrEasuTapF(aC,aW,AF2( 1.0,-1.0)-pp,dir,len2,lob,clp,AF3(bczzR.y,bczzG.y,bczzB.y)); // c
    FsrEasuTapF(aC,aW,AF2(-1.0, 1.0)-pp,dir,len2,lob,clp,AF3(ijfeR.x,ijfeG.x,ijfeB.x)); // i
    FsrEasuTapF(aC,aW,AF2( 0.0, 1.0)-pp,dir,len2,lob,clp,AF3(ijfeR.y,ijfeG.y,ijfeB.y)); // j
    FsrEasuTapF(aC,aW,AF2( 0.0, 0.0)-pp,dir,len2,lob,clp,AF3(ijfeR.z,ijfeG.z,ijfeB.z)); // f
    FsrEasuTapF(aC,aW,AF2(-1.0, 0.0)-pp,dir,len2,lob,clp,AF3(ijfeR.w,ijfeG.w,ijfeB.w)); // e
    FsrEasuTapF(aC,aW,AF2( 1.0, 1.0)-pp,dir,len2,lob,clp,AF3(klhgR.x,klhgG.x,klhgB.x)); // k
    FsrEasuTapF(aC,aW,AF2( 2.0, 1.0)-pp,dir,len2,lob,clp,AF3(klhgR.y,klhgG.y,klhgB.y)); // l
    FsrEasuTapF(aC,aW,AF2( 2.0, 0.0)-pp,dir,len2,lob,clp,AF3(klhgR.z,klhgG.z,klhgB.z)); // h
    FsrEasuTapF(aC,aW,AF2( 1.0, 0.0)-pp,dir,len2,lob,clp,AF3(klhgR.w,klhgG.w,klhgB.w)); // g
    FsrEasuTapF(aC,aW,AF2( 1.0, 2.0)-pp,dir,len2,lob,clp,AF3(zzonR.z,zzonG.z,zzonB.z)); // o
    FsrEasuTapF(aC,aW,AF2( 0.0, 2.0)-pp,dir,len2,lob,clp,AF3(zzonR.w,zzonG.w,zzonB.w)); // n
    //------------------------------------------------------------------------------------------------------------------------------

#if defined(FILAMENT_FSR_DERINGING)
    pix=min(max4,max(min4,aC*AF3_(ARcpF1(aW))));
#else
    pix=aC*AF3_(ARcpF1(aW));
#endif
}
