/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#if defined(__clang_analyzer__) && !defined(SDL_DISABLE_ANALYZE_MACROS)
#define SDL_DISABLE_ANALYZE_MACROS 1
#endif

#include "../SDL_internal.h"

/* This file contains portable stdlib functions for SDL */

#include "SDL_stdinc.h"
#include "../libm/math_libm.h"


double
SDL_atan(double x)
{
#if defined(HAVE_ATAN)
    return atan(x);
#else
    return SDL_uclibc_atan(x);
#endif
}

float
SDL_atanf(float x)
{
#if defined(HAVE_ATANF)
    return atanf(x);
#else
    return (float)SDL_atan((double)x);
#endif
}

double
SDL_atan2(double x, double y)
{
#if defined(HAVE_ATAN2)
    return atan2(x, y);
#else
    return SDL_uclibc_atan2(x, y);
#endif
}

float
SDL_atan2f(float x, float y)
{
#if defined(HAVE_ATAN2F)
    return atan2f(x, y);
#else
    return (float)SDL_atan2((double)x, (double)y);
#endif
}

double
SDL_acos(double val)
{
#if defined(HAVE_ACOS)
    return acos(val);
#else
    double result;
    if (val == -1.0) {
        result = M_PI;
    } else {
        result = SDL_atan(SDL_sqrt(1.0 - val * val) / val);
        if (result < 0.0)
        {
            result += M_PI;
        }
    }
    return result;
#endif
}

float
SDL_acosf(float val)
{
#if defined(HAVE_ACOSF)
    return acosf(val);
#else
    return (float)SDL_acos((double)val);
#endif
}

double
SDL_asin(double val)
{
#if defined(HAVE_ASIN)
    return asin(val);
#else
    double result;
    if (val == -1.0) {
        result = -(M_PI / 2.0);
    } else {
        result = (M_PI / 2.0) - SDL_acos(val);
    }
    return result;
#endif
}

float
SDL_asinf(float val)
{
#if defined(HAVE_ASINF)
    return asinf(val);
#else
    return (float)SDL_asin((double)val);
#endif
}

double
SDL_ceil(double x)
{
#if defined(HAVE_CEIL)
    return ceil(x);
#else
    double integer = SDL_floor(x);
    double fraction = x - integer;
    if (fraction > 0.0) {
        integer += 1.0;
    }
    return integer;
#endif /* HAVE_CEIL */
}

float
SDL_ceilf(float x)
{
#if defined(HAVE_CEILF)
    return ceilf(x);
#else
    return (float)SDL_ceil((double)x);
#endif
}

double
SDL_copysign(double x, double y)
{
#if defined(HAVE_COPYSIGN)
    return copysign(x, y);
#elif defined(HAVE__COPYSIGN)
    return _copysign(x, y);
#elif defined(__WATCOMC__) && defined(__386__)
    /* this is nasty as hell, but it works.. */
    unsigned int *xi = (unsigned int *) &x,
                 *yi = (unsigned int *) &y;
    xi[1] = (yi[1] & 0x80000000) | (xi[1] & 0x7fffffff);
    return x;
#else
    return SDL_uclibc_copysign(x, y);
#endif /* HAVE_COPYSIGN */
}

float
SDL_copysignf(float x, float y)
{
#if defined(HAVE_COPYSIGNF)
    return copysignf(x, y);
#else
    return (float)SDL_copysign((double)x, (double)y);
#endif
}

double
SDL_cos(double x)
{
#if defined(HAVE_COS)
    return cos(x);
#else
    return SDL_uclibc_cos(x);
#endif
}

float
SDL_cosf(float x)
{
#if defined(HAVE_COSF)
    return cosf(x);
#else
    return (float)SDL_cos((double)x);
#endif
}

double
SDL_exp(double x)
{
#if defined(HAVE_EXP)
    return exp(x);
#else
    return SDL_uclibc_exp(x);
#endif
}

float
SDL_expf(float x)
{
#if defined(HAVE_EXPF)
    return expf(x);
#else
    return (float)SDL_exp((double)x);
#endif
}

double
SDL_fabs(double x)
{
#if defined(HAVE_FABS)
    return fabs(x);
#else
    return SDL_uclibc_fabs(x);
#endif
}

float
SDL_fabsf(float x)
{
#if defined(HAVE_FABSF)
    return fabsf(x);
#else
    return (float)SDL_fabs((double)x);
#endif
}

double
SDL_floor(double x)
{
#if defined(HAVE_FLOOR)
    return floor(x);
#else
    return SDL_uclibc_floor(x);
#endif
}

float
SDL_floorf(float x)
{
#if defined(HAVE_FLOORF)
    return floorf(x);
#else
    return (float)SDL_floor((double)x);
#endif
}

double
SDL_trunc(double x)
{
#if defined(HAVE_TRUNC)
    return trunc(x);
#else
    if (x >= 0.0f) {
        return SDL_floor(x);
    } else {
        return SDL_ceil(x);
    }
#endif
}

float
SDL_truncf(float x)
{
#if defined(HAVE_TRUNCF)
    return truncf(x);
#else
    return (float)SDL_trunc((double)x);
#endif
}

double
SDL_fmod(double x, double y)
{
#if defined(HAVE_FMOD)
    return fmod(x, y);
#else
    return SDL_uclibc_fmod(x, y);
#endif
}

float
SDL_fmodf(float x, float y)
{
#if defined(HAVE_FMODF)
    return fmodf(x, y);
#else
    return (float)SDL_fmod((double)x, (double)y);
#endif
}

double
SDL_log(double x)
{
#if defined(HAVE_LOG)
    return log(x);
#else
    return SDL_uclibc_log(x);
#endif
}

float
SDL_logf(float x)
{
#if defined(HAVE_LOGF)
    return logf(x);
#else
    return (float)SDL_log((double)x);
#endif
}

double
SDL_log10(double x)
{
#if defined(HAVE_LOG10)
    return log10(x);
#else
    return SDL_uclibc_log10(x);
#endif
}

float
SDL_log10f(float x)
{
#if defined(HAVE_LOG10F)
    return log10f(x);
#else
    return (float)SDL_log10((double)x);
#endif
}

double
SDL_pow(double x, double y)
{
#if defined(HAVE_POW)
    return pow(x, y);
#else
    return SDL_uclibc_pow(x, y);
#endif
}

float
SDL_powf(float x, float y)
{
#if defined(HAVE_POWF)
    return powf(x, y);
#else
    return (float)SDL_pow((double)x, (double)y);
#endif
}

double
SDL_round(double arg)
{
#if defined HAVE_ROUND
    return round(arg);
#else
    if (arg >= 0.0) {
        return SDL_floor(arg + 0.5);
    } else {
        return SDL_ceil(arg - 0.5);
    }
#endif
}

float
SDL_roundf(float arg)
{
#if defined HAVE_ROUNDF
    return roundf(arg);
#else
    return (float)SDL_round((double)arg);
#endif
}

long
SDL_lround(double arg)
{
#if defined HAVE_LROUND
    return lround(arg);
#else
    return (long)SDL_round(arg);
#endif
}

long
SDL_lroundf(float arg)
{
#if defined HAVE_LROUNDF
    return lroundf(arg);
#else
    return (long)SDL_round((double)arg);
#endif
}

double
SDL_scalbn(double x, int n)
{
#if defined(HAVE_SCALBN)
    return scalbn(x, n);
#elif defined(HAVE__SCALB)
    return _scalb(x, n);
#elif defined(HAVE_LIBC) && defined(HAVE_FLOAT_H) && (FLT_RADIX == 2)
/* from scalbn(3): If FLT_RADIX equals 2 (which is
 * usual), then scalbn() is equivalent to ldexp(3). */
    return ldexp(x, n);
#else
    return SDL_uclibc_scalbn(x, n);
#endif
}

float
SDL_scalbnf(float x, int n)
{
#if defined(HAVE_SCALBNF)
    return scalbnf(x, n);
#else
    return (float)SDL_scalbn((double)x, n);
#endif
}

double
SDL_sin(double x)
{
#if defined(HAVE_SIN)
    return sin(x);
#else
    return SDL_uclibc_sin(x);
#endif
}

float 
SDL_sinf(float x)
{
#if defined(HAVE_SINF)
    return sinf(x);
#else
    return (float)SDL_sin((double)x);
#endif
}

double
SDL_sqrt(double x)
{
#if defined(HAVE_SQRT)
    return sqrt(x);
#else
    return SDL_uclibc_sqrt(x);
#endif
}

float
SDL_sqrtf(float x)
{
#if defined(HAVE_SQRTF)
    return sqrtf(x);
#else
    return (float)SDL_sqrt((double)x);
#endif
}

double
SDL_tan(double x)
{
#if defined(HAVE_TAN)
    return tan(x);
#else
    return SDL_uclibc_tan(x);
#endif
}

float
SDL_tanf(float x)
{
#if defined(HAVE_TANF)
    return tanf(x);
#else
    return (float)SDL_tan((double)x);
#endif
}

int SDL_abs(int x)
{
#if defined(HAVE_ABS)
    return abs(x);
#else
    return ((x) < 0 ? -(x) : (x));
#endif
}

#if defined(HAVE_CTYPE_H)
int SDL_isalpha(int x) { return isalpha(x); }
int SDL_isalnum(int x) { return isalnum(x); }
int SDL_isdigit(int x) { return isdigit(x); }
int SDL_isxdigit(int x) { return isxdigit(x); }
int SDL_ispunct(int x) { return ispunct(x); }
int SDL_isspace(int x) { return isspace(x); }
int SDL_isupper(int x) { return isupper(x); }
int SDL_islower(int x) { return islower(x); }
int SDL_isprint(int x) { return isprint(x); }
int SDL_isgraph(int x) { return isgraph(x); }
int SDL_iscntrl(int x) { return iscntrl(x); }
int SDL_toupper(int x) { return toupper(x); }
int SDL_tolower(int x) { return tolower(x); }
#else
int SDL_isalpha(int x) { return (SDL_isupper(x)) || (SDL_islower(x)); }
int SDL_isalnum(int x) { return (SDL_isalpha(x)) || (SDL_isdigit(x)); }
int SDL_isdigit(int x) { return ((x) >= '0') && ((x) <= '9'); }
int SDL_isxdigit(int x) { return (((x) >= 'A') && ((x) <= 'F')) || (((x) >= 'a') && ((x) <= 'f')) || (SDL_isdigit(x)); }
int SDL_ispunct(int x) { return (SDL_isgraph(x)) && (!SDL_isalnum(x)); }
int SDL_isspace(int x) { return ((x) == ' ') || ((x) == '\t') || ((x) == '\r') || ((x) == '\n') || ((x) == '\f') || ((x) == '\v'); }
int SDL_isupper(int x) { return ((x) >= 'A') && ((x) <= 'Z'); }
int SDL_islower(int x) { return ((x) >= 'a') && ((x) <= 'z'); }
int SDL_isprint(int x) { return ((x) >= ' ') && ((x) < '\x7f'); }
int SDL_isgraph(int x) { return (SDL_isprint(x)) && ((x) != ' '); }
int SDL_iscntrl(int x) { return (((x) >= '\0') && ((x) <= '\x1f')) || ((x) == '\x7f'); }
int SDL_toupper(int x) { return ((x) >= 'a') && ((x) <= 'z') ? ('A'+((x)-'a')) : (x); }
int SDL_tolower(int x) { return ((x) >= 'A') && ((x) <= 'Z') ? ('a'+((x)-'A')) : (x); }
#endif

#if defined(HAVE_CTYPE_H) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
int SDL_isblank(int x) { return isblank(x); }
#else
int SDL_isblank(int x) { return ((x) == ' ') || ((x) == '\t'); }
#endif

#ifndef HAVE_LIBC
/* These are some C runtime intrinsics that need to be defined */

#if defined(_MSC_VER)

#ifndef __FLTUSED__
#define __FLTUSED__
__declspec(selectany) int _fltused = 1;
#endif

/* The optimizer on Visual Studio 2005 and later generates memcpy() and memset() calls */
#if _MSC_VER >= 1400
extern void *memcpy(void* dst, const void* src, size_t len);
#pragma intrinsic(memcpy)

#pragma function(memcpy)
void *
memcpy(void *dst, const void *src, size_t len)
{
    return SDL_memcpy(dst, src, len);
}

extern void *memset(void* dst, int c, size_t len);
#pragma intrinsic(memset)

#pragma function(memset)
void *
memset(void *dst, int c, size_t len)
{
    return SDL_memset(dst, c, len);
}
#endif /* _MSC_VER >= 1400 */

#ifdef _M_IX86

/* Float to long */
void
__declspec(naked)
_ftol()
{
    /* *INDENT-OFF* */
    __asm {
        push        ebp
        mov         ebp,esp
        sub         esp,20h
        and         esp,0FFFFFFF0h
        fld         st(0)
        fst         dword ptr [esp+18h]
        fistp       qword ptr [esp+10h]
        fild        qword ptr [esp+10h]
        mov         edx,dword ptr [esp+18h]
        mov         eax,dword ptr [esp+10h]
        test        eax,eax
        je          integer_QnaN_or_zero
arg_is_not_integer_QnaN:
        fsubp       st(1),st
        test        edx,edx
        jns         positive
        fstp        dword ptr [esp]
        mov         ecx,dword ptr [esp]
        xor         ecx,80000000h
        add         ecx,7FFFFFFFh
        adc         eax,0
        mov         edx,dword ptr [esp+14h]
        adc         edx,0
        jmp         localexit
positive:
        fstp        dword ptr [esp]
        mov         ecx,dword ptr [esp]
        add         ecx,7FFFFFFFh
        sbb         eax,0
        mov         edx,dword ptr [esp+14h]
        sbb         edx,0
        jmp         localexit
integer_QnaN_or_zero:
        mov         edx,dword ptr [esp+14h]
        test        edx,7FFFFFFFh
        jne         arg_is_not_integer_QnaN
        fstp        dword ptr [esp+18h]
        fstp        dword ptr [esp+18h]
localexit:
        leave
        ret
    }
    /* *INDENT-ON* */
}

void
_ftol2_sse()
{
    _ftol();
}

/* 64-bit math operators for 32-bit systems */
void
__declspec(naked)
_allmul()
{
    /* *INDENT-OFF* */
    __asm {
        mov         eax, dword ptr[esp+8]
        mov         ecx, dword ptr[esp+10h]
        or          ecx, eax
        mov         ecx, dword ptr[esp+0Ch]
        jne         hard
        mov         eax, dword ptr[esp+4]
        mul         ecx
        ret         10h
hard:
        push        ebx
        mul         ecx
        mov         ebx, eax
        mov         eax, dword ptr[esp+8]
        mul         dword ptr[esp+14h]
        add         ebx, eax
        mov         eax, dword ptr[esp+8]
        mul         ecx
        add         edx, ebx
        pop         ebx
        ret         10h
    }
    /* *INDENT-ON* */
}

void
__declspec(naked)
_alldiv()
{
    /* *INDENT-OFF* */
    __asm {
        push        edi
        push        esi
        push        ebx
        xor         edi,edi
        mov         eax,dword ptr [esp+14h]
        or          eax,eax
        jge         L1
        inc         edi
        mov         edx,dword ptr [esp+10h]
        neg         eax
        neg         edx
        sbb         eax,0
        mov         dword ptr [esp+14h],eax
        mov         dword ptr [esp+10h],edx
L1:
        mov         eax,dword ptr [esp+1Ch]
        or          eax,eax
        jge         L2
        inc         edi
        mov         edx,dword ptr [esp+18h]
        neg         eax
        neg         edx
        sbb         eax,0
        mov         dword ptr [esp+1Ch],eax
        mov         dword ptr [esp+18h],edx
L2:
        or          eax,eax
        jne         L3
        mov         ecx,dword ptr [esp+18h]
        mov         eax,dword ptr [esp+14h]
        xor         edx,edx
        div         ecx
        mov         ebx,eax
        mov         eax,dword ptr [esp+10h]
        div         ecx
        mov         edx,ebx
        jmp         L4
L3:
        mov         ebx,eax
        mov         ecx,dword ptr [esp+18h]
        mov         edx,dword ptr [esp+14h]
        mov         eax,dword ptr [esp+10h]
L5:
        shr         ebx,1
        rcr         ecx,1
        shr         edx,1
        rcr         eax,1
        or          ebx,ebx
        jne         L5
        div         ecx
        mov         esi,eax
        mul         dword ptr [esp+1Ch]
        mov         ecx,eax
        mov         eax,dword ptr [esp+18h]
        mul         esi
        add         edx,ecx
        jb          L6
        cmp         edx,dword ptr [esp+14h]
        ja          L6
        jb          L7
        cmp         eax,dword ptr [esp+10h]
        jbe         L7
L6:
        dec         esi
L7:
        xor         edx,edx
        mov         eax,esi
L4:
        dec         edi
        jne         L8
        neg         edx
        neg         eax
        sbb         edx,0
L8:
        pop         ebx
        pop         esi
        pop         edi
        ret         10h
    }
    /* *INDENT-ON* */
}

void
__declspec(naked)
_aulldiv()
{
    /* *INDENT-OFF* */
    __asm {
        push        ebx
        push        esi
        mov         eax,dword ptr [esp+18h]
        or          eax,eax
        jne         L1
        mov         ecx,dword ptr [esp+14h]
        mov         eax,dword ptr [esp+10h]
        xor         edx,edx
        div         ecx
        mov         ebx,eax
        mov         eax,dword ptr [esp+0Ch]
        div         ecx
        mov         edx,ebx
        jmp         L2
L1:
        mov         ecx,eax
        mov         ebx,dword ptr [esp+14h]
        mov         edx,dword ptr [esp+10h]
        mov         eax,dword ptr [esp+0Ch]
L3:
        shr         ecx,1
        rcr         ebx,1
        shr         edx,1
        rcr         eax,1
        or          ecx,ecx
        jne         L3
        div         ebx
        mov         esi,eax
        mul         dword ptr [esp+18h]
        mov         ecx,eax
        mov         eax,dword ptr [esp+14h]
        mul         esi
        add         edx,ecx
        jb          L4
        cmp         edx,dword ptr [esp+10h]
        ja          L4
        jb          L5
        cmp         eax,dword ptr [esp+0Ch]
        jbe         L5
L4:
        dec         esi
L5:
        xor         edx,edx
        mov         eax,esi
L2:
        pop         esi
        pop         ebx
        ret         10h
    }
    /* *INDENT-ON* */
}

void
__declspec(naked)
_allrem()
{
    /* *INDENT-OFF* */
    __asm {
        push        ebx
        push        edi
        xor         edi,edi
        mov         eax,dword ptr [esp+10h]
        or          eax,eax
        jge         L1
        inc         edi
        mov         edx,dword ptr [esp+0Ch]
        neg         eax
        neg         edx
        sbb         eax,0
        mov         dword ptr [esp+10h],eax
        mov         dword ptr [esp+0Ch],edx
L1:
        mov         eax,dword ptr [esp+18h]
        or          eax,eax
        jge         L2
        mov         edx,dword ptr [esp+14h]
        neg         eax
        neg         edx
        sbb         eax,0
        mov         dword ptr [esp+18h],eax
        mov         dword ptr [esp+14h],edx
L2:
        or          eax,eax
        jne         L3
        mov         ecx,dword ptr [esp+14h]
        mov         eax,dword ptr [esp+10h]
        xor         edx,edx
        div         ecx
        mov         eax,dword ptr [esp+0Ch]
        div         ecx
        mov         eax,edx
        xor         edx,edx
        dec         edi
        jns         L4
        jmp         L8
L3:
        mov         ebx,eax
        mov         ecx,dword ptr [esp+14h]
        mov         edx,dword ptr [esp+10h]
        mov         eax,dword ptr [esp+0Ch]
L5:
        shr         ebx,1
        rcr         ecx,1
        shr         edx,1
        rcr         eax,1
        or          ebx,ebx
        jne         L5
        div         ecx
        mov         ecx,eax
        mul         dword ptr [esp+18h]
        xchg        eax,ecx
        mul         dword ptr [esp+14h]
        add         edx,ecx
        jb          L6
        cmp         edx,dword ptr [esp+10h]
        ja          L6
        jb          L7
        cmp         eax,dword ptr [esp+0Ch]
        jbe         L7
L6:
        sub         eax,dword ptr [esp+14h]
        sbb         edx,dword ptr [esp+18h]
L7:
        sub         eax,dword ptr [esp+0Ch]
        sbb         edx,dword ptr [esp+10h]
        dec         edi
        jns         L8
L4:
        neg         edx
        neg         eax
        sbb         edx,0
L8:
        pop         edi
        pop         ebx
        ret         10h
    }
    /* *INDENT-ON* */
}

void
__declspec(naked)
_aullrem()
{
    /* *INDENT-OFF* */
    __asm {
        push        ebx
        mov         eax,dword ptr [esp+14h]
        or          eax,eax
        jne         L1
        mov         ecx,dword ptr [esp+10h]
        mov         eax,dword ptr [esp+0Ch]
        xor         edx,edx
        div         ecx
        mov         eax,dword ptr [esp+8]
        div         ecx
        mov         eax,edx
        xor         edx,edx
        jmp         L2
L1:
        mov         ecx,eax
        mov         ebx,dword ptr [esp+10h]
        mov         edx,dword ptr [esp+0Ch]
        mov         eax,dword ptr [esp+8]
L3:
        shr         ecx,1
        rcr         ebx,1
        shr         edx,1
        rcr         eax,1
        or          ecx,ecx
        jne         L3
        div         ebx
        mov         ecx,eax
        mul         dword ptr [esp+14h]
        xchg        eax,ecx
        mul         dword ptr [esp+10h]
        add         edx,ecx
        jb          L4
        cmp         edx,dword ptr [esp+0Ch]
        ja          L4
        jb          L5
        cmp         eax,dword ptr [esp+8]
        jbe         L5
L4:
        sub         eax,dword ptr [esp+10h]
        sbb         edx,dword ptr [esp+14h]
L5:
        sub         eax,dword ptr [esp+8]
        sbb         edx,dword ptr [esp+0Ch]
        neg         edx
        neg         eax
        sbb         edx,0
L2:
        pop         ebx
        ret         10h
    }
    /* *INDENT-ON* */
}

void
__declspec(naked)
_alldvrm()
{
    /* *INDENT-OFF* */
    __asm {
        push        edi
        push        esi
        push        ebp
        xor         edi,edi
        xor         ebp,ebp
        mov         eax,dword ptr [esp+14h]
        or          eax,eax
        jge         L1
        inc         edi
        inc         ebp
        mov         edx,dword ptr [esp+10h]
        neg         eax
        neg         edx
        sbb         eax,0
        mov         dword ptr [esp+14h],eax
        mov         dword ptr [esp+10h],edx
L1:
        mov         eax,dword ptr [esp+1Ch]
        or          eax,eax
        jge         L2
        inc         edi
        mov         edx,dword ptr [esp+18h]
        neg         eax
        neg         edx
        sbb         eax,0
        mov         dword ptr [esp+1Ch],eax
        mov         dword ptr [esp+18h],edx
L2:
        or          eax,eax
        jne         L3
        mov         ecx,dword ptr [esp+18h]
        mov         eax,dword ptr [esp+14h]
        xor         edx,edx
        div         ecx
        mov         ebx,eax
        mov         eax,dword ptr [esp+10h]
        div         ecx
        mov         esi,eax
        mov         eax,ebx
        mul         dword ptr [esp+18h]
        mov         ecx,eax
        mov         eax,esi
        mul         dword ptr [esp+18h]
        add         edx,ecx
        jmp         L4
L3:
        mov         ebx,eax
        mov         ecx,dword ptr [esp+18h]
        mov         edx,dword ptr [esp+14h]
        mov         eax,dword ptr [esp+10h]
L5:
        shr         ebx,1
        rcr         ecx,1
        shr         edx,1
        rcr         eax,1
        or          ebx,ebx
        jne         L5
        div         ecx
        mov         esi,eax
        mul         dword ptr [esp+1Ch]
        mov         ecx,eax
        mov         eax,dword ptr [esp+18h]
        mul         esi
        add         edx,ecx
        jb          L6
        cmp         edx,dword ptr [esp+14h]
        ja          L6
        jb          L7
        cmp         eax,dword ptr [esp+10h]
        jbe         L7
L6:
        dec         esi
        sub         eax,dword ptr [esp+18h]
        sbb         edx,dword ptr [esp+1Ch]
L7:
        xor         ebx,ebx
L4:
        sub         eax,dword ptr [esp+10h]
        sbb         edx,dword ptr [esp+14h]
        dec         ebp
        jns         L9
        neg         edx
        neg         eax
        sbb         edx,0
L9:
        mov         ecx,edx
        mov         edx,ebx
        mov         ebx,ecx
        mov         ecx,eax
        mov         eax,esi
        dec         edi
        jne         L8
        neg         edx
        neg         eax
        sbb         edx,0
L8:
        pop         ebp
        pop         esi
        pop         edi
        ret         10h
    }
    /* *INDENT-ON* */
}

void
__declspec(naked)
_aulldvrm()
{
    /* *INDENT-OFF* */
    __asm {
        push        esi
        mov         eax,dword ptr [esp+14h]
        or          eax,eax
        jne         L1
        mov         ecx,dword ptr [esp+10h]
        mov         eax,dword ptr [esp+0Ch]
        xor         edx,edx
        div         ecx
        mov         ebx,eax
        mov         eax,dword ptr [esp+8]
        div         ecx
        mov         esi,eax
        mov         eax,ebx
        mul         dword ptr [esp+10h]
        mov         ecx,eax
        mov         eax,esi
        mul         dword ptr [esp+10h]
        add         edx,ecx
        jmp         L2
L1:
        mov         ecx,eax
        mov         ebx,dword ptr [esp+10h]
        mov         edx,dword ptr [esp+0Ch]
        mov         eax,dword ptr [esp+8]
L3:
        shr         ecx,1
        rcr         ebx,1
        shr         edx,1
        rcr         eax,1
        or          ecx,ecx
        jne         L3
        div         ebx
        mov         esi,eax
        mul         dword ptr [esp+14h]
        mov         ecx,eax
        mov         eax,dword ptr [esp+10h]
        mul         esi
        add         edx,ecx
        jb          L4
        cmp         edx,dword ptr [esp+0Ch]
        ja          L4
        jb          L5
        cmp         eax,dword ptr [esp+8]
        jbe         L5
L4:
        dec         esi
        sub         eax,dword ptr [esp+10h]
        sbb         edx,dword ptr [esp+14h]
L5:
        xor         ebx,ebx
L2:
        sub         eax,dword ptr [esp+8]
        sbb         edx,dword ptr [esp+0Ch]
        neg         edx
        neg         eax
        sbb         edx,0
        mov         ecx,edx
        mov         edx,ebx
        mov         ebx,ecx
        mov         ecx,eax
        mov         eax,esi
        pop         esi
        ret         10h
    }
    /* *INDENT-ON* */
}

void
__declspec(naked)
_allshl()
{
    /* *INDENT-OFF* */
    __asm {
        cmp         cl,40h
        jae         RETZERO
        cmp         cl,20h
        jae         MORE32
        shld        edx,eax,cl
        shl         eax,cl
        ret
MORE32:
        mov         edx,eax
        xor         eax,eax
        and         cl,1Fh
        shl         edx,cl
        ret
RETZERO:
        xor         eax,eax
        xor         edx,edx
        ret
    }
    /* *INDENT-ON* */
}

void
__declspec(naked)
_allshr()
{
    /* *INDENT-OFF* */
    __asm {
        cmp         cl,3Fh
        jae         RETSIGN
        cmp         cl,20h
        jae         MORE32
        shrd        eax,edx,cl
        sar         edx,cl
        ret
MORE32:
        mov         eax,edx
        sar         edx,1Fh
        and         cl,1Fh
        sar         eax,cl
        ret
RETSIGN:
        sar         edx,1Fh
        mov         eax,edx
        ret
    }
    /* *INDENT-ON* */
}

void
__declspec(naked)
_aullshr()
{
    /* *INDENT-OFF* */
    __asm {
        cmp         cl,40h
        jae         RETZERO
        cmp         cl,20h
        jae         MORE32
        shrd        eax,edx,cl
        shr         edx,cl
        ret
MORE32:
        mov         eax,edx
        xor         edx,edx
        and         cl,1Fh
        shr         eax,cl
        ret
RETZERO:
        xor         eax,eax
        xor         edx,edx
        ret
    }
    /* *INDENT-ON* */
}

#endif /* _M_IX86 */

#endif /* MSC_VER */

#endif /* !HAVE_LIBC */

/* vi: set ts=4 sw=4 expandtab: */
