//==============================================================================================================================
// An optimized AMD FSR's EASU implementation for Mobiles
// Based on https://github.com/GPUOpen-Effects/FidelityFX-FSR/blob/master/ffx-fsr/ffx_fsr1.h
// Details can be found: https://atyuwen.github.io/posts/optimizing-fsr/
// Distributed under the MIT License.
// -- FsrEasuSampleH should be implemented by calling shader, like following:
//    half3 FsrEasuSampleH(float2 p) { return MyTex.SampleLevel(LinearSampler, p, 0).xyz; }
//==============================================================================================================================
void FsrEasuL(
        out AH3 pix,
        AF2 ip,
        AF4 con0,
        AF4 con1,
        AF4 con2,
        AF4 con3){
    //------------------------------------------------------------------------------------------------------------------------------
    // Direction is the '+' diff.
    //    A
    //  B C D
    //    E
    AF2 pp=(ip)*(con0.xy)+(con0.zw);
    AF2 tc=(pp+AF2_(0.5))*con1.xy;
    AH3 sA=FsrEasuSampleH(tc-AF2(0, con1.y));
    AH3 sB=FsrEasuSampleH(tc-AF2(con1.x, 0));
    AH3 sC=FsrEasuSampleH(tc);
    AH3 sD=FsrEasuSampleH(tc+AF2(con1.x, 0));
    AH3 sE=FsrEasuSampleH(tc+AF2(0, con1.y));
    AH1 lA=sA.r*AH1_(0.5)+sA.g;
    AH1 lB=sB.r*AH1_(0.5)+sB.g;
    AH1 lC=sC.r*AH1_(0.5)+sC.g;
    AH1 lD=sD.r*AH1_(0.5)+sD.g;
    AH1 lE=sE.r*AH1_(0.5)+sE.g;
    // Then takes magnitude from abs average of both sides of 'C'.
    // Length converts gradient reversal to 0, smoothly to non-reversal at 1, shaped, then adding horz and vert terms.
    AH1 dc=lD-lC;
    AH1 cb=lC-lB;
    AH1 lenX=max(abs(dc),abs(cb));
    lenX=ARcpH1(lenX);
    AH1 dirX=lD-lB;
    lenX=ASatH1(abs(dirX)*lenX);
    lenX*=lenX;
    // Repeat for the y axis.
    AH1 ec=lE-lC;
    AH1 ca=lC-lA;
    AH1 lenY=max(abs(ec),abs(ca));
    lenY=ARcpH1(lenY);
    AH1 dirY=lE-lA;
    lenY=ASatH1(abs(dirY)*lenY);
    AH1 len = lenY * lenY + lenX;
    AH2 dir = AH2(dirX, dirY);
    //------------------------------------------------------------------------------------------------------------------------------
    AH2 dir2=dir*dir;
    AH1 dirR=dir2.x+dir2.y;
    if (dirR<AH1_(1.0/64.0)) {
        pix = sC;
        return;
    }
    dirR=rsqrt(dirR);
    dir*=AH2_(dirR);
    len=len*AH1_(0.5);
    len*=len;
    AH1 stretch=(dir.x*dir.x+dir.y*dir.y)*rcp(max(abs(dir.x),abs(dir.y)));
    AH2 len2=AH2(AH1_(1.0)+(stretch-AH1_(1.0))*len,AH1_(1.0)+AH1_(-0.5)*len);
    AH1 lob=AH1_(0.5)+AH1_((1.0/4.0-0.04)-0.5)*len;
    AH1 clp=rcp(lob);
    //------------------------------------------------------------------------------------------------------------------------------
    AF2 fp=floor(pp);
    pp-=fp;
    AH2 ppp=AH2(pp);
    AF2 p0=fp*(con1.xy)+(con1.zw);
    AF2 p1=p0+(con2.xy);
    AF2 p2=p0+(con2.zw);
    AF2 p3=p0+(con3.xy);
    p0.y-=con1.w; p3.y+=con1.w;
    AH4 fgcbR=FsrEasuRH(p0);
    AH4 fgcbG=FsrEasuGH(p0);
    AH4 fgcbB=FsrEasuBH(p0);
    AH4 ijfeR=FsrEasuRH(p1);
    AH4 ijfeG=FsrEasuGH(p1);
    AH4 ijfeB=FsrEasuBH(p1);
    AH4 klhgR=FsrEasuRH(p2);
    AH4 klhgG=FsrEasuGH(p2);
    AH4 klhgB=FsrEasuBH(p2);
    AH4 nokjR=FsrEasuRH(p3);
    AH4 nokjG=FsrEasuGH(p3);
    AH4 nokjB=FsrEasuBH(p3);
    //------------------------------------------------------------------------------------------------------------------------------
    // This part is different for FP16, working pairs of taps at a time.
    AH2 pR=AH2_(0.0);
    AH2 pG=AH2_(0.0);
    AH2 pB=AH2_(0.0);
    AH2 pW=AH2_(0.0);
    FsrEasuTapH(pR,pG,pB,pW,AH2( 1.0, 0.0)-ppp.xx,AH2(-1.0,-1.0)-ppp.yy,dir,len2,lob,clp,fgcbR.zw,fgcbG.zw,fgcbB.zw);
    FsrEasuTapH(pR,pG,pB,pW,AH2(-1.0, 0.0)-ppp.xx,AH2( 1.0, 1.0)-ppp.yy,dir,len2,lob,clp,ijfeR.xy,ijfeG.xy,ijfeB.xy);
    FsrEasuTapH(pR,pG,pB,pW,AH2( 0.0,-1.0)-ppp.xx,AH2( 0.0, 0.0)-ppp.yy,dir,len2,lob,clp,ijfeR.zw,ijfeG.zw,ijfeB.zw);
    FsrEasuTapH(pR,pG,pB,pW,AH2( 1.0, 2.0)-ppp.xx,AH2( 1.0, 1.0)-ppp.yy,dir,len2,lob,clp,klhgR.xy,klhgG.xy,klhgB.xy);
    FsrEasuTapH(pR,pG,pB,pW,AH2( 2.0, 1.0)-ppp.xx,AH2( 0.0, 0.0)-ppp.yy,dir,len2,lob,clp,klhgR.zw,klhgG.zw,klhgB.zw);
    FsrEasuTapH(pR,pG,pB,pW,AH2( 0.0, 1.0)-ppp.xx,AH2( 2.0, 2.0)-ppp.yy,dir,len2,lob,clp,nokjR.xy,nokjG.xy,nokjB.xy);
    AH3 aC=AH3(pR.x+pR.y,pG.x+pG.y,pB.x+pB.y);
    AH1 aW=pW.x+pW.y;
    //------------------------------------------------------------------------------------------------------------------------------
    pix=aC*AH3_(ARcpH1(aW));} 