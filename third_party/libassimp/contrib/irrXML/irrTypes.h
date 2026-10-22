// Copyright (C) 2002-2005 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_TYPES_H_INCLUDED__
#define __IRR_TYPES_H_INCLUDED__

namespace irr
{

//! 8 bit unsigned variable.
/** This is a typedef for unsigned char, it ensures portability of the engine. */
typedef unsigned char		u8; 

//! 8 bit signed variable.
/** This is a typedef for signed char, it ensures portability of the engine. */
typedef signed char			s8; 

//! 8 bit character variable.
/** This is a typedef for char, it ensures portability of the engine. */
typedef char				c8; 



//! 16 bit unsigned variable.
/** This is a typedef for unsigned short, it ensures portability of the engine. */
typedef unsigned short		u16;

//! 16 bit signed variable.
/** This is a typedef for signed short, it ensures portability of the engine. */
typedef signed short		s16; 



//! 32 bit unsigned variable.
/** This is a typedef for unsigned int, it ensures portability of the engine. */
typedef unsigned int		u32;

//! 32 bit signed variable.
/** This is a typedef for signed int, it ensures portability of the engine. */
typedef signed int			s32; 



// 64 bit signed variable.
// This is a typedef for __int64, it ensures portability of the engine. 
// This type is currently not used by the engine and not supported by compilers
// other than Microsoft Compilers, so it is outcommented.
//typedef __int64				s64; 



//! 32 bit floating point variable.
/** This is a typedef for float, it ensures portability of the engine. */
typedef float				f32; 

//! 64 bit floating point variable.
/** This is a typedef for double, it ensures portability of the engine. */
typedef double				f64; 


} // end namespace


// define the wchar_t type if not already built in.
#ifdef _MSC_VER 
#ifndef _WCHAR_T_DEFINED
//! A 16 bit wide character type.
/**
	Defines the wchar_t-type.
	In VS6, its not possible to tell
	the standard compiler to treat wchar_t as a built-in type, and 
	sometimes we just don't want to include the huge stdlib.h or wchar.h,
	so we'll use this.
*/
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif // wchar is not defined
#endif // microsoft compiler

//! define a break macro for debugging only in Win32 mode.
// WORKAROUND (assimp): remove __asm
#if defined(WIN32) && defined(_MSC_VER) && defined(_DEBUG)
#if defined(_M_IX86)
#define _IRR_DEBUG_BREAK_IF( _CONDITION_ ) /*if (_CONDITION_) {_asm int 3}*/
#else
#define _IRR_DEBUG_BREAK_IF( _CONDITION_ )
#endif
#else 
#define _IRR_DEBUG_BREAK_IF( _CONDITION_ )
#endif

//! Defines a small statement to work around a microsoft compiler bug.
/** The microsft compiler 7.0 - 7.1 has a bug:
When you call unmanaged code that returns a bool type value of false from managed code, 
the return value may appear as true. See 
http://support.microsoft.com/default.aspx?kbid=823071 for details. 
Compiler version defines: VC6.0 : 1200, VC7.0 : 1300, VC7.1 : 1310, VC8.0 : 1400*/

// WORKAROUND (assimp): remove __asm 
#if defined(WIN32) && defined(_MSC_VER) && (_MSC_VER > 1299) && (_MSC_VER < 1400)
#define _IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX /*__asm mov eax,100*/
#else
#define _IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX
#endif // _IRR_MANAGED_MARSHALLING_BUGFIX

#endif // __IRR_TYPES_H_INCLUDED__

