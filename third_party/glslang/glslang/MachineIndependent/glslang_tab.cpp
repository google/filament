/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 44 "MachineIndependent/glslang.y"


/* Based on:
ANSI C Yacc grammar

In 1985, Jeff Lee published his Yacc grammar (which is accompanied by a
matching Lex specification) for the April 30, 1985 draft version of the
ANSI C standard.  Tom Stockfisch reposted it to net.sources in 1987; that
original, as mentioned in the answer to question 17.25 of the comp.lang.c
FAQ, can be ftp'ed from ftp.uu.net, file usenet/net.sources/ansi.c.grammar.Z.

I intend to keep this version as close to the current C Standard grammar as
possible; please let me know if you discover discrepancies.

Jutta Degener, 1995
*/

#include "SymbolTable.h"
#include "ParseHelper.h"
#include "../Public/ShaderLang.h"
#include "attribute.h"

using namespace glslang;


#line 97 "MachineIndependent/glslang_tab.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "glslang_tab.cpp.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_CONST = 3,                      /* CONST  */
  YYSYMBOL_BOOL = 4,                       /* BOOL  */
  YYSYMBOL_INT = 5,                        /* INT  */
  YYSYMBOL_UINT = 6,                       /* UINT  */
  YYSYMBOL_FLOAT = 7,                      /* FLOAT  */
  YYSYMBOL_BVEC2 = 8,                      /* BVEC2  */
  YYSYMBOL_BVEC3 = 9,                      /* BVEC3  */
  YYSYMBOL_BVEC4 = 10,                     /* BVEC4  */
  YYSYMBOL_IVEC2 = 11,                     /* IVEC2  */
  YYSYMBOL_IVEC3 = 12,                     /* IVEC3  */
  YYSYMBOL_IVEC4 = 13,                     /* IVEC4  */
  YYSYMBOL_UVEC2 = 14,                     /* UVEC2  */
  YYSYMBOL_UVEC3 = 15,                     /* UVEC3  */
  YYSYMBOL_UVEC4 = 16,                     /* UVEC4  */
  YYSYMBOL_VEC2 = 17,                      /* VEC2  */
  YYSYMBOL_VEC3 = 18,                      /* VEC3  */
  YYSYMBOL_VEC4 = 19,                      /* VEC4  */
  YYSYMBOL_MAT2 = 20,                      /* MAT2  */
  YYSYMBOL_MAT3 = 21,                      /* MAT3  */
  YYSYMBOL_MAT4 = 22,                      /* MAT4  */
  YYSYMBOL_MAT2X2 = 23,                    /* MAT2X2  */
  YYSYMBOL_MAT2X3 = 24,                    /* MAT2X3  */
  YYSYMBOL_MAT2X4 = 25,                    /* MAT2X4  */
  YYSYMBOL_MAT3X2 = 26,                    /* MAT3X2  */
  YYSYMBOL_MAT3X3 = 27,                    /* MAT3X3  */
  YYSYMBOL_MAT3X4 = 28,                    /* MAT3X4  */
  YYSYMBOL_MAT4X2 = 29,                    /* MAT4X2  */
  YYSYMBOL_MAT4X3 = 30,                    /* MAT4X3  */
  YYSYMBOL_MAT4X4 = 31,                    /* MAT4X4  */
  YYSYMBOL_SAMPLER2D = 32,                 /* SAMPLER2D  */
  YYSYMBOL_SAMPLER3D = 33,                 /* SAMPLER3D  */
  YYSYMBOL_SAMPLERCUBE = 34,               /* SAMPLERCUBE  */
  YYSYMBOL_SAMPLER2DSHADOW = 35,           /* SAMPLER2DSHADOW  */
  YYSYMBOL_SAMPLERCUBESHADOW = 36,         /* SAMPLERCUBESHADOW  */
  YYSYMBOL_SAMPLER2DARRAY = 37,            /* SAMPLER2DARRAY  */
  YYSYMBOL_SAMPLER2DARRAYSHADOW = 38,      /* SAMPLER2DARRAYSHADOW  */
  YYSYMBOL_ISAMPLER2D = 39,                /* ISAMPLER2D  */
  YYSYMBOL_ISAMPLER3D = 40,                /* ISAMPLER3D  */
  YYSYMBOL_ISAMPLERCUBE = 41,              /* ISAMPLERCUBE  */
  YYSYMBOL_ISAMPLER2DARRAY = 42,           /* ISAMPLER2DARRAY  */
  YYSYMBOL_USAMPLER2D = 43,                /* USAMPLER2D  */
  YYSYMBOL_USAMPLER3D = 44,                /* USAMPLER3D  */
  YYSYMBOL_USAMPLERCUBE = 45,              /* USAMPLERCUBE  */
  YYSYMBOL_USAMPLER2DARRAY = 46,           /* USAMPLER2DARRAY  */
  YYSYMBOL_SAMPLER = 47,                   /* SAMPLER  */
  YYSYMBOL_SAMPLERSHADOW = 48,             /* SAMPLERSHADOW  */
  YYSYMBOL_TEXTURE2D = 49,                 /* TEXTURE2D  */
  YYSYMBOL_TEXTURE3D = 50,                 /* TEXTURE3D  */
  YYSYMBOL_TEXTURECUBE = 51,               /* TEXTURECUBE  */
  YYSYMBOL_TEXTURE2DARRAY = 52,            /* TEXTURE2DARRAY  */
  YYSYMBOL_ITEXTURE2D = 53,                /* ITEXTURE2D  */
  YYSYMBOL_ITEXTURE3D = 54,                /* ITEXTURE3D  */
  YYSYMBOL_ITEXTURECUBE = 55,              /* ITEXTURECUBE  */
  YYSYMBOL_ITEXTURE2DARRAY = 56,           /* ITEXTURE2DARRAY  */
  YYSYMBOL_UTEXTURE2D = 57,                /* UTEXTURE2D  */
  YYSYMBOL_UTEXTURE3D = 58,                /* UTEXTURE3D  */
  YYSYMBOL_UTEXTURECUBE = 59,              /* UTEXTURECUBE  */
  YYSYMBOL_UTEXTURE2DARRAY = 60,           /* UTEXTURE2DARRAY  */
  YYSYMBOL_ATTRIBUTE = 61,                 /* ATTRIBUTE  */
  YYSYMBOL_VARYING = 62,                   /* VARYING  */
  YYSYMBOL_FLOATE5M2_T = 63,               /* FLOATE5M2_T  */
  YYSYMBOL_FLOATE4M3_T = 64,               /* FLOATE4M3_T  */
  YYSYMBOL_BFLOAT16_T = 65,                /* BFLOAT16_T  */
  YYSYMBOL_FLOAT16_T = 66,                 /* FLOAT16_T  */
  YYSYMBOL_FLOAT32_T = 67,                 /* FLOAT32_T  */
  YYSYMBOL_DOUBLE = 68,                    /* DOUBLE  */
  YYSYMBOL_FLOAT64_T = 69,                 /* FLOAT64_T  */
  YYSYMBOL_INT64_T = 70,                   /* INT64_T  */
  YYSYMBOL_UINT64_T = 71,                  /* UINT64_T  */
  YYSYMBOL_INT32_T = 72,                   /* INT32_T  */
  YYSYMBOL_UINT32_T = 73,                  /* UINT32_T  */
  YYSYMBOL_INT16_T = 74,                   /* INT16_T  */
  YYSYMBOL_UINT16_T = 75,                  /* UINT16_T  */
  YYSYMBOL_INT8_T = 76,                    /* INT8_T  */
  YYSYMBOL_UINT8_T = 77,                   /* UINT8_T  */
  YYSYMBOL_I64VEC2 = 78,                   /* I64VEC2  */
  YYSYMBOL_I64VEC3 = 79,                   /* I64VEC3  */
  YYSYMBOL_I64VEC4 = 80,                   /* I64VEC4  */
  YYSYMBOL_U64VEC2 = 81,                   /* U64VEC2  */
  YYSYMBOL_U64VEC3 = 82,                   /* U64VEC3  */
  YYSYMBOL_U64VEC4 = 83,                   /* U64VEC4  */
  YYSYMBOL_I32VEC2 = 84,                   /* I32VEC2  */
  YYSYMBOL_I32VEC3 = 85,                   /* I32VEC3  */
  YYSYMBOL_I32VEC4 = 86,                   /* I32VEC4  */
  YYSYMBOL_U32VEC2 = 87,                   /* U32VEC2  */
  YYSYMBOL_U32VEC3 = 88,                   /* U32VEC3  */
  YYSYMBOL_U32VEC4 = 89,                   /* U32VEC4  */
  YYSYMBOL_I16VEC2 = 90,                   /* I16VEC2  */
  YYSYMBOL_I16VEC3 = 91,                   /* I16VEC3  */
  YYSYMBOL_I16VEC4 = 92,                   /* I16VEC4  */
  YYSYMBOL_U16VEC2 = 93,                   /* U16VEC2  */
  YYSYMBOL_U16VEC3 = 94,                   /* U16VEC3  */
  YYSYMBOL_U16VEC4 = 95,                   /* U16VEC4  */
  YYSYMBOL_I8VEC2 = 96,                    /* I8VEC2  */
  YYSYMBOL_I8VEC3 = 97,                    /* I8VEC3  */
  YYSYMBOL_I8VEC4 = 98,                    /* I8VEC4  */
  YYSYMBOL_U8VEC2 = 99,                    /* U8VEC2  */
  YYSYMBOL_U8VEC3 = 100,                   /* U8VEC3  */
  YYSYMBOL_U8VEC4 = 101,                   /* U8VEC4  */
  YYSYMBOL_DVEC2 = 102,                    /* DVEC2  */
  YYSYMBOL_DVEC3 = 103,                    /* DVEC3  */
  YYSYMBOL_DVEC4 = 104,                    /* DVEC4  */
  YYSYMBOL_DMAT2 = 105,                    /* DMAT2  */
  YYSYMBOL_DMAT3 = 106,                    /* DMAT3  */
  YYSYMBOL_DMAT4 = 107,                    /* DMAT4  */
  YYSYMBOL_BF16VEC2 = 108,                 /* BF16VEC2  */
  YYSYMBOL_BF16VEC3 = 109,                 /* BF16VEC3  */
  YYSYMBOL_BF16VEC4 = 110,                 /* BF16VEC4  */
  YYSYMBOL_FE5M2VEC2 = 111,                /* FE5M2VEC2  */
  YYSYMBOL_FE5M2VEC3 = 112,                /* FE5M2VEC3  */
  YYSYMBOL_FE5M2VEC4 = 113,                /* FE5M2VEC4  */
  YYSYMBOL_FE4M3VEC2 = 114,                /* FE4M3VEC2  */
  YYSYMBOL_FE4M3VEC3 = 115,                /* FE4M3VEC3  */
  YYSYMBOL_FE4M3VEC4 = 116,                /* FE4M3VEC4  */
  YYSYMBOL_F16VEC2 = 117,                  /* F16VEC2  */
  YYSYMBOL_F16VEC3 = 118,                  /* F16VEC3  */
  YYSYMBOL_F16VEC4 = 119,                  /* F16VEC4  */
  YYSYMBOL_F16MAT2 = 120,                  /* F16MAT2  */
  YYSYMBOL_F16MAT3 = 121,                  /* F16MAT3  */
  YYSYMBOL_F16MAT4 = 122,                  /* F16MAT4  */
  YYSYMBOL_F32VEC2 = 123,                  /* F32VEC2  */
  YYSYMBOL_F32VEC3 = 124,                  /* F32VEC3  */
  YYSYMBOL_F32VEC4 = 125,                  /* F32VEC4  */
  YYSYMBOL_F32MAT2 = 126,                  /* F32MAT2  */
  YYSYMBOL_F32MAT3 = 127,                  /* F32MAT3  */
  YYSYMBOL_F32MAT4 = 128,                  /* F32MAT4  */
  YYSYMBOL_F64VEC2 = 129,                  /* F64VEC2  */
  YYSYMBOL_F64VEC3 = 130,                  /* F64VEC3  */
  YYSYMBOL_F64VEC4 = 131,                  /* F64VEC4  */
  YYSYMBOL_F64MAT2 = 132,                  /* F64MAT2  */
  YYSYMBOL_F64MAT3 = 133,                  /* F64MAT3  */
  YYSYMBOL_F64MAT4 = 134,                  /* F64MAT4  */
  YYSYMBOL_DMAT2X2 = 135,                  /* DMAT2X2  */
  YYSYMBOL_DMAT2X3 = 136,                  /* DMAT2X3  */
  YYSYMBOL_DMAT2X4 = 137,                  /* DMAT2X4  */
  YYSYMBOL_DMAT3X2 = 138,                  /* DMAT3X2  */
  YYSYMBOL_DMAT3X3 = 139,                  /* DMAT3X3  */
  YYSYMBOL_DMAT3X4 = 140,                  /* DMAT3X4  */
  YYSYMBOL_DMAT4X2 = 141,                  /* DMAT4X2  */
  YYSYMBOL_DMAT4X3 = 142,                  /* DMAT4X3  */
  YYSYMBOL_DMAT4X4 = 143,                  /* DMAT4X4  */
  YYSYMBOL_F16MAT2X2 = 144,                /* F16MAT2X2  */
  YYSYMBOL_F16MAT2X3 = 145,                /* F16MAT2X3  */
  YYSYMBOL_F16MAT2X4 = 146,                /* F16MAT2X4  */
  YYSYMBOL_F16MAT3X2 = 147,                /* F16MAT3X2  */
  YYSYMBOL_F16MAT3X3 = 148,                /* F16MAT3X3  */
  YYSYMBOL_F16MAT3X4 = 149,                /* F16MAT3X4  */
  YYSYMBOL_F16MAT4X2 = 150,                /* F16MAT4X2  */
  YYSYMBOL_F16MAT4X3 = 151,                /* F16MAT4X3  */
  YYSYMBOL_F16MAT4X4 = 152,                /* F16MAT4X4  */
  YYSYMBOL_F32MAT2X2 = 153,                /* F32MAT2X2  */
  YYSYMBOL_F32MAT2X3 = 154,                /* F32MAT2X3  */
  YYSYMBOL_F32MAT2X4 = 155,                /* F32MAT2X4  */
  YYSYMBOL_F32MAT3X2 = 156,                /* F32MAT3X2  */
  YYSYMBOL_F32MAT3X3 = 157,                /* F32MAT3X3  */
  YYSYMBOL_F32MAT3X4 = 158,                /* F32MAT3X4  */
  YYSYMBOL_F32MAT4X2 = 159,                /* F32MAT4X2  */
  YYSYMBOL_F32MAT4X3 = 160,                /* F32MAT4X3  */
  YYSYMBOL_F32MAT4X4 = 161,                /* F32MAT4X4  */
  YYSYMBOL_F64MAT2X2 = 162,                /* F64MAT2X2  */
  YYSYMBOL_F64MAT2X3 = 163,                /* F64MAT2X3  */
  YYSYMBOL_F64MAT2X4 = 164,                /* F64MAT2X4  */
  YYSYMBOL_F64MAT3X2 = 165,                /* F64MAT3X2  */
  YYSYMBOL_F64MAT3X3 = 166,                /* F64MAT3X3  */
  YYSYMBOL_F64MAT3X4 = 167,                /* F64MAT3X4  */
  YYSYMBOL_F64MAT4X2 = 168,                /* F64MAT4X2  */
  YYSYMBOL_F64MAT4X3 = 169,                /* F64MAT4X3  */
  YYSYMBOL_F64MAT4X4 = 170,                /* F64MAT4X4  */
  YYSYMBOL_ATOMIC_UINT = 171,              /* ATOMIC_UINT  */
  YYSYMBOL_ACCSTRUCTNV = 172,              /* ACCSTRUCTNV  */
  YYSYMBOL_ACCSTRUCTEXT = 173,             /* ACCSTRUCTEXT  */
  YYSYMBOL_RAYQUERYEXT = 174,              /* RAYQUERYEXT  */
  YYSYMBOL_FCOOPMATNV = 175,               /* FCOOPMATNV  */
  YYSYMBOL_ICOOPMATNV = 176,               /* ICOOPMATNV  */
  YYSYMBOL_UCOOPMATNV = 177,               /* UCOOPMATNV  */
  YYSYMBOL_COOPMAT = 178,                  /* COOPMAT  */
  YYSYMBOL_COOPVECNV = 179,                /* COOPVECNV  */
  YYSYMBOL_VECTOR = 180,                   /* VECTOR  */
  YYSYMBOL_HITOBJECTNV = 181,              /* HITOBJECTNV  */
  YYSYMBOL_HITOBJECTATTRNV = 182,          /* HITOBJECTATTRNV  */
  YYSYMBOL_HITOBJECTEXT = 183,             /* HITOBJECTEXT  */
  YYSYMBOL_HITOBJECTATTREXT = 184,         /* HITOBJECTATTREXT  */
  YYSYMBOL_TENSORLAYOUTNV = 185,           /* TENSORLAYOUTNV  */
  YYSYMBOL_TENSORVIEWNV = 186,             /* TENSORVIEWNV  */
  YYSYMBOL_TENSORARM = 187,                /* TENSORARM  */
  YYSYMBOL_SAMPLERCUBEARRAY = 188,         /* SAMPLERCUBEARRAY  */
  YYSYMBOL_SAMPLERCUBEARRAYSHADOW = 189,   /* SAMPLERCUBEARRAYSHADOW  */
  YYSYMBOL_ISAMPLERCUBEARRAY = 190,        /* ISAMPLERCUBEARRAY  */
  YYSYMBOL_USAMPLERCUBEARRAY = 191,        /* USAMPLERCUBEARRAY  */
  YYSYMBOL_SAMPLER1D = 192,                /* SAMPLER1D  */
  YYSYMBOL_SAMPLER1DARRAY = 193,           /* SAMPLER1DARRAY  */
  YYSYMBOL_SAMPLER1DARRAYSHADOW = 194,     /* SAMPLER1DARRAYSHADOW  */
  YYSYMBOL_ISAMPLER1D = 195,               /* ISAMPLER1D  */
  YYSYMBOL_SAMPLER1DSHADOW = 196,          /* SAMPLER1DSHADOW  */
  YYSYMBOL_SAMPLER2DRECT = 197,            /* SAMPLER2DRECT  */
  YYSYMBOL_SAMPLER2DRECTSHADOW = 198,      /* SAMPLER2DRECTSHADOW  */
  YYSYMBOL_ISAMPLER2DRECT = 199,           /* ISAMPLER2DRECT  */
  YYSYMBOL_USAMPLER2DRECT = 200,           /* USAMPLER2DRECT  */
  YYSYMBOL_SAMPLERBUFFER = 201,            /* SAMPLERBUFFER  */
  YYSYMBOL_ISAMPLERBUFFER = 202,           /* ISAMPLERBUFFER  */
  YYSYMBOL_USAMPLERBUFFER = 203,           /* USAMPLERBUFFER  */
  YYSYMBOL_SAMPLER2DMS = 204,              /* SAMPLER2DMS  */
  YYSYMBOL_ISAMPLER2DMS = 205,             /* ISAMPLER2DMS  */
  YYSYMBOL_USAMPLER2DMS = 206,             /* USAMPLER2DMS  */
  YYSYMBOL_SAMPLER2DMSARRAY = 207,         /* SAMPLER2DMSARRAY  */
  YYSYMBOL_ISAMPLER2DMSARRAY = 208,        /* ISAMPLER2DMSARRAY  */
  YYSYMBOL_USAMPLER2DMSARRAY = 209,        /* USAMPLER2DMSARRAY  */
  YYSYMBOL_SAMPLEREXTERNALOES = 210,       /* SAMPLEREXTERNALOES  */
  YYSYMBOL_SAMPLEREXTERNAL2DY2YEXT = 211,  /* SAMPLEREXTERNAL2DY2YEXT  */
  YYSYMBOL_ISAMPLER1DARRAY = 212,          /* ISAMPLER1DARRAY  */
  YYSYMBOL_USAMPLER1D = 213,               /* USAMPLER1D  */
  YYSYMBOL_USAMPLER1DARRAY = 214,          /* USAMPLER1DARRAY  */
  YYSYMBOL_F16SAMPLER1D = 215,             /* F16SAMPLER1D  */
  YYSYMBOL_F16SAMPLER2D = 216,             /* F16SAMPLER2D  */
  YYSYMBOL_F16SAMPLER3D = 217,             /* F16SAMPLER3D  */
  YYSYMBOL_F16SAMPLER2DRECT = 218,         /* F16SAMPLER2DRECT  */
  YYSYMBOL_F16SAMPLERCUBE = 219,           /* F16SAMPLERCUBE  */
  YYSYMBOL_F16SAMPLER1DARRAY = 220,        /* F16SAMPLER1DARRAY  */
  YYSYMBOL_F16SAMPLER2DARRAY = 221,        /* F16SAMPLER2DARRAY  */
  YYSYMBOL_F16SAMPLERCUBEARRAY = 222,      /* F16SAMPLERCUBEARRAY  */
  YYSYMBOL_F16SAMPLERBUFFER = 223,         /* F16SAMPLERBUFFER  */
  YYSYMBOL_F16SAMPLER2DMS = 224,           /* F16SAMPLER2DMS  */
  YYSYMBOL_F16SAMPLER2DMSARRAY = 225,      /* F16SAMPLER2DMSARRAY  */
  YYSYMBOL_F16SAMPLER1DSHADOW = 226,       /* F16SAMPLER1DSHADOW  */
  YYSYMBOL_F16SAMPLER2DSHADOW = 227,       /* F16SAMPLER2DSHADOW  */
  YYSYMBOL_F16SAMPLER1DARRAYSHADOW = 228,  /* F16SAMPLER1DARRAYSHADOW  */
  YYSYMBOL_F16SAMPLER2DARRAYSHADOW = 229,  /* F16SAMPLER2DARRAYSHADOW  */
  YYSYMBOL_F16SAMPLER2DRECTSHADOW = 230,   /* F16SAMPLER2DRECTSHADOW  */
  YYSYMBOL_F16SAMPLERCUBESHADOW = 231,     /* F16SAMPLERCUBESHADOW  */
  YYSYMBOL_F16SAMPLERCUBEARRAYSHADOW = 232, /* F16SAMPLERCUBEARRAYSHADOW  */
  YYSYMBOL_IMAGE1D = 233,                  /* IMAGE1D  */
  YYSYMBOL_IIMAGE1D = 234,                 /* IIMAGE1D  */
  YYSYMBOL_UIMAGE1D = 235,                 /* UIMAGE1D  */
  YYSYMBOL_IMAGE2D = 236,                  /* IMAGE2D  */
  YYSYMBOL_IIMAGE2D = 237,                 /* IIMAGE2D  */
  YYSYMBOL_UIMAGE2D = 238,                 /* UIMAGE2D  */
  YYSYMBOL_IMAGE3D = 239,                  /* IMAGE3D  */
  YYSYMBOL_IIMAGE3D = 240,                 /* IIMAGE3D  */
  YYSYMBOL_UIMAGE3D = 241,                 /* UIMAGE3D  */
  YYSYMBOL_IMAGE2DRECT = 242,              /* IMAGE2DRECT  */
  YYSYMBOL_IIMAGE2DRECT = 243,             /* IIMAGE2DRECT  */
  YYSYMBOL_UIMAGE2DRECT = 244,             /* UIMAGE2DRECT  */
  YYSYMBOL_IMAGECUBE = 245,                /* IMAGECUBE  */
  YYSYMBOL_IIMAGECUBE = 246,               /* IIMAGECUBE  */
  YYSYMBOL_UIMAGECUBE = 247,               /* UIMAGECUBE  */
  YYSYMBOL_IMAGEBUFFER = 248,              /* IMAGEBUFFER  */
  YYSYMBOL_IIMAGEBUFFER = 249,             /* IIMAGEBUFFER  */
  YYSYMBOL_UIMAGEBUFFER = 250,             /* UIMAGEBUFFER  */
  YYSYMBOL_IMAGE1DARRAY = 251,             /* IMAGE1DARRAY  */
  YYSYMBOL_IIMAGE1DARRAY = 252,            /* IIMAGE1DARRAY  */
  YYSYMBOL_UIMAGE1DARRAY = 253,            /* UIMAGE1DARRAY  */
  YYSYMBOL_IMAGE2DARRAY = 254,             /* IMAGE2DARRAY  */
  YYSYMBOL_IIMAGE2DARRAY = 255,            /* IIMAGE2DARRAY  */
  YYSYMBOL_UIMAGE2DARRAY = 256,            /* UIMAGE2DARRAY  */
  YYSYMBOL_IMAGECUBEARRAY = 257,           /* IMAGECUBEARRAY  */
  YYSYMBOL_IIMAGECUBEARRAY = 258,          /* IIMAGECUBEARRAY  */
  YYSYMBOL_UIMAGECUBEARRAY = 259,          /* UIMAGECUBEARRAY  */
  YYSYMBOL_IMAGE2DMS = 260,                /* IMAGE2DMS  */
  YYSYMBOL_IIMAGE2DMS = 261,               /* IIMAGE2DMS  */
  YYSYMBOL_UIMAGE2DMS = 262,               /* UIMAGE2DMS  */
  YYSYMBOL_IMAGE2DMSARRAY = 263,           /* IMAGE2DMSARRAY  */
  YYSYMBOL_IIMAGE2DMSARRAY = 264,          /* IIMAGE2DMSARRAY  */
  YYSYMBOL_UIMAGE2DMSARRAY = 265,          /* UIMAGE2DMSARRAY  */
  YYSYMBOL_F16IMAGE1D = 266,               /* F16IMAGE1D  */
  YYSYMBOL_F16IMAGE2D = 267,               /* F16IMAGE2D  */
  YYSYMBOL_F16IMAGE3D = 268,               /* F16IMAGE3D  */
  YYSYMBOL_F16IMAGE2DRECT = 269,           /* F16IMAGE2DRECT  */
  YYSYMBOL_F16IMAGECUBE = 270,             /* F16IMAGECUBE  */
  YYSYMBOL_F16IMAGE1DARRAY = 271,          /* F16IMAGE1DARRAY  */
  YYSYMBOL_F16IMAGE2DARRAY = 272,          /* F16IMAGE2DARRAY  */
  YYSYMBOL_F16IMAGECUBEARRAY = 273,        /* F16IMAGECUBEARRAY  */
  YYSYMBOL_F16IMAGEBUFFER = 274,           /* F16IMAGEBUFFER  */
  YYSYMBOL_F16IMAGE2DMS = 275,             /* F16IMAGE2DMS  */
  YYSYMBOL_F16IMAGE2DMSARRAY = 276,        /* F16IMAGE2DMSARRAY  */
  YYSYMBOL_I64IMAGE1D = 277,               /* I64IMAGE1D  */
  YYSYMBOL_U64IMAGE1D = 278,               /* U64IMAGE1D  */
  YYSYMBOL_I64IMAGE2D = 279,               /* I64IMAGE2D  */
  YYSYMBOL_U64IMAGE2D = 280,               /* U64IMAGE2D  */
  YYSYMBOL_I64IMAGE3D = 281,               /* I64IMAGE3D  */
  YYSYMBOL_U64IMAGE3D = 282,               /* U64IMAGE3D  */
  YYSYMBOL_I64IMAGE2DRECT = 283,           /* I64IMAGE2DRECT  */
  YYSYMBOL_U64IMAGE2DRECT = 284,           /* U64IMAGE2DRECT  */
  YYSYMBOL_I64IMAGECUBE = 285,             /* I64IMAGECUBE  */
  YYSYMBOL_U64IMAGECUBE = 286,             /* U64IMAGECUBE  */
  YYSYMBOL_I64IMAGEBUFFER = 287,           /* I64IMAGEBUFFER  */
  YYSYMBOL_U64IMAGEBUFFER = 288,           /* U64IMAGEBUFFER  */
  YYSYMBOL_I64IMAGE1DARRAY = 289,          /* I64IMAGE1DARRAY  */
  YYSYMBOL_U64IMAGE1DARRAY = 290,          /* U64IMAGE1DARRAY  */
  YYSYMBOL_I64IMAGE2DARRAY = 291,          /* I64IMAGE2DARRAY  */
  YYSYMBOL_U64IMAGE2DARRAY = 292,          /* U64IMAGE2DARRAY  */
  YYSYMBOL_I64IMAGECUBEARRAY = 293,        /* I64IMAGECUBEARRAY  */
  YYSYMBOL_U64IMAGECUBEARRAY = 294,        /* U64IMAGECUBEARRAY  */
  YYSYMBOL_I64IMAGE2DMS = 295,             /* I64IMAGE2DMS  */
  YYSYMBOL_U64IMAGE2DMS = 296,             /* U64IMAGE2DMS  */
  YYSYMBOL_I64IMAGE2DMSARRAY = 297,        /* I64IMAGE2DMSARRAY  */
  YYSYMBOL_U64IMAGE2DMSARRAY = 298,        /* U64IMAGE2DMSARRAY  */
  YYSYMBOL_TEXTURECUBEARRAY = 299,         /* TEXTURECUBEARRAY  */
  YYSYMBOL_ITEXTURECUBEARRAY = 300,        /* ITEXTURECUBEARRAY  */
  YYSYMBOL_UTEXTURECUBEARRAY = 301,        /* UTEXTURECUBEARRAY  */
  YYSYMBOL_TEXTURE1D = 302,                /* TEXTURE1D  */
  YYSYMBOL_ITEXTURE1D = 303,               /* ITEXTURE1D  */
  YYSYMBOL_UTEXTURE1D = 304,               /* UTEXTURE1D  */
  YYSYMBOL_TEXTURE1DARRAY = 305,           /* TEXTURE1DARRAY  */
  YYSYMBOL_ITEXTURE1DARRAY = 306,          /* ITEXTURE1DARRAY  */
  YYSYMBOL_UTEXTURE1DARRAY = 307,          /* UTEXTURE1DARRAY  */
  YYSYMBOL_TEXTURE2DRECT = 308,            /* TEXTURE2DRECT  */
  YYSYMBOL_ITEXTURE2DRECT = 309,           /* ITEXTURE2DRECT  */
  YYSYMBOL_UTEXTURE2DRECT = 310,           /* UTEXTURE2DRECT  */
  YYSYMBOL_TEXTUREBUFFER = 311,            /* TEXTUREBUFFER  */
  YYSYMBOL_ITEXTUREBUFFER = 312,           /* ITEXTUREBUFFER  */
  YYSYMBOL_UTEXTUREBUFFER = 313,           /* UTEXTUREBUFFER  */
  YYSYMBOL_TEXTURE2DMS = 314,              /* TEXTURE2DMS  */
  YYSYMBOL_ITEXTURE2DMS = 315,             /* ITEXTURE2DMS  */
  YYSYMBOL_UTEXTURE2DMS = 316,             /* UTEXTURE2DMS  */
  YYSYMBOL_TEXTURE2DMSARRAY = 317,         /* TEXTURE2DMSARRAY  */
  YYSYMBOL_ITEXTURE2DMSARRAY = 318,        /* ITEXTURE2DMSARRAY  */
  YYSYMBOL_UTEXTURE2DMSARRAY = 319,        /* UTEXTURE2DMSARRAY  */
  YYSYMBOL_F16TEXTURE1D = 320,             /* F16TEXTURE1D  */
  YYSYMBOL_F16TEXTURE2D = 321,             /* F16TEXTURE2D  */
  YYSYMBOL_F16TEXTURE3D = 322,             /* F16TEXTURE3D  */
  YYSYMBOL_F16TEXTURE2DRECT = 323,         /* F16TEXTURE2DRECT  */
  YYSYMBOL_F16TEXTURECUBE = 324,           /* F16TEXTURECUBE  */
  YYSYMBOL_F16TEXTURE1DARRAY = 325,        /* F16TEXTURE1DARRAY  */
  YYSYMBOL_F16TEXTURE2DARRAY = 326,        /* F16TEXTURE2DARRAY  */
  YYSYMBOL_F16TEXTURECUBEARRAY = 327,      /* F16TEXTURECUBEARRAY  */
  YYSYMBOL_F16TEXTUREBUFFER = 328,         /* F16TEXTUREBUFFER  */
  YYSYMBOL_F16TEXTURE2DMS = 329,           /* F16TEXTURE2DMS  */
  YYSYMBOL_F16TEXTURE2DMSARRAY = 330,      /* F16TEXTURE2DMSARRAY  */
  YYSYMBOL_SUBPASSINPUT = 331,             /* SUBPASSINPUT  */
  YYSYMBOL_SUBPASSINPUTMS = 332,           /* SUBPASSINPUTMS  */
  YYSYMBOL_ISUBPASSINPUT = 333,            /* ISUBPASSINPUT  */
  YYSYMBOL_ISUBPASSINPUTMS = 334,          /* ISUBPASSINPUTMS  */
  YYSYMBOL_USUBPASSINPUT = 335,            /* USUBPASSINPUT  */
  YYSYMBOL_USUBPASSINPUTMS = 336,          /* USUBPASSINPUTMS  */
  YYSYMBOL_F16SUBPASSINPUT = 337,          /* F16SUBPASSINPUT  */
  YYSYMBOL_F16SUBPASSINPUTMS = 338,        /* F16SUBPASSINPUTMS  */
  YYSYMBOL_SPIRV_INSTRUCTION = 339,        /* SPIRV_INSTRUCTION  */
  YYSYMBOL_SPIRV_EXECUTION_MODE = 340,     /* SPIRV_EXECUTION_MODE  */
  YYSYMBOL_SPIRV_EXECUTION_MODE_ID = 341,  /* SPIRV_EXECUTION_MODE_ID  */
  YYSYMBOL_SPIRV_DECORATE = 342,           /* SPIRV_DECORATE  */
  YYSYMBOL_SPIRV_DECORATE_ID = 343,        /* SPIRV_DECORATE_ID  */
  YYSYMBOL_SPIRV_DECORATE_STRING = 344,    /* SPIRV_DECORATE_STRING  */
  YYSYMBOL_SPIRV_TYPE = 345,               /* SPIRV_TYPE  */
  YYSYMBOL_SPIRV_STORAGE_CLASS = 346,      /* SPIRV_STORAGE_CLASS  */
  YYSYMBOL_SPIRV_BY_REFERENCE = 347,       /* SPIRV_BY_REFERENCE  */
  YYSYMBOL_SPIRV_LITERAL = 348,            /* SPIRV_LITERAL  */
  YYSYMBOL_ATTACHMENTEXT = 349,            /* ATTACHMENTEXT  */
  YYSYMBOL_IATTACHMENTEXT = 350,           /* IATTACHMENTEXT  */
  YYSYMBOL_UATTACHMENTEXT = 351,           /* UATTACHMENTEXT  */
  YYSYMBOL_LEFT_OP = 352,                  /* LEFT_OP  */
  YYSYMBOL_RIGHT_OP = 353,                 /* RIGHT_OP  */
  YYSYMBOL_INC_OP = 354,                   /* INC_OP  */
  YYSYMBOL_DEC_OP = 355,                   /* DEC_OP  */
  YYSYMBOL_LE_OP = 356,                    /* LE_OP  */
  YYSYMBOL_GE_OP = 357,                    /* GE_OP  */
  YYSYMBOL_EQ_OP = 358,                    /* EQ_OP  */
  YYSYMBOL_NE_OP = 359,                    /* NE_OP  */
  YYSYMBOL_AND_OP = 360,                   /* AND_OP  */
  YYSYMBOL_OR_OP = 361,                    /* OR_OP  */
  YYSYMBOL_XOR_OP = 362,                   /* XOR_OP  */
  YYSYMBOL_MUL_ASSIGN = 363,               /* MUL_ASSIGN  */
  YYSYMBOL_DIV_ASSIGN = 364,               /* DIV_ASSIGN  */
  YYSYMBOL_ADD_ASSIGN = 365,               /* ADD_ASSIGN  */
  YYSYMBOL_MOD_ASSIGN = 366,               /* MOD_ASSIGN  */
  YYSYMBOL_LEFT_ASSIGN = 367,              /* LEFT_ASSIGN  */
  YYSYMBOL_RIGHT_ASSIGN = 368,             /* RIGHT_ASSIGN  */
  YYSYMBOL_AND_ASSIGN = 369,               /* AND_ASSIGN  */
  YYSYMBOL_XOR_ASSIGN = 370,               /* XOR_ASSIGN  */
  YYSYMBOL_OR_ASSIGN = 371,                /* OR_ASSIGN  */
  YYSYMBOL_SUB_ASSIGN = 372,               /* SUB_ASSIGN  */
  YYSYMBOL_STRING_LITERAL = 373,           /* STRING_LITERAL  */
  YYSYMBOL_LEFT_PAREN = 374,               /* LEFT_PAREN  */
  YYSYMBOL_RIGHT_PAREN = 375,              /* RIGHT_PAREN  */
  YYSYMBOL_LEFT_BRACKET = 376,             /* LEFT_BRACKET  */
  YYSYMBOL_RIGHT_BRACKET = 377,            /* RIGHT_BRACKET  */
  YYSYMBOL_LEFT_BRACE = 378,               /* LEFT_BRACE  */
  YYSYMBOL_RIGHT_BRACE = 379,              /* RIGHT_BRACE  */
  YYSYMBOL_DOT = 380,                      /* DOT  */
  YYSYMBOL_COMMA = 381,                    /* COMMA  */
  YYSYMBOL_COLON = 382,                    /* COLON  */
  YYSYMBOL_EQUAL = 383,                    /* EQUAL  */
  YYSYMBOL_SEMICOLON = 384,                /* SEMICOLON  */
  YYSYMBOL_BANG = 385,                     /* BANG  */
  YYSYMBOL_DASH = 386,                     /* DASH  */
  YYSYMBOL_TILDE = 387,                    /* TILDE  */
  YYSYMBOL_PLUS = 388,                     /* PLUS  */
  YYSYMBOL_STAR = 389,                     /* STAR  */
  YYSYMBOL_SLASH = 390,                    /* SLASH  */
  YYSYMBOL_PERCENT = 391,                  /* PERCENT  */
  YYSYMBOL_LEFT_ANGLE = 392,               /* LEFT_ANGLE  */
  YYSYMBOL_RIGHT_ANGLE = 393,              /* RIGHT_ANGLE  */
  YYSYMBOL_VERTICAL_BAR = 394,             /* VERTICAL_BAR  */
  YYSYMBOL_CARET = 395,                    /* CARET  */
  YYSYMBOL_AMPERSAND = 396,                /* AMPERSAND  */
  YYSYMBOL_QUESTION = 397,                 /* QUESTION  */
  YYSYMBOL_INVARIANT = 398,                /* INVARIANT  */
  YYSYMBOL_HIGH_PRECISION = 399,           /* HIGH_PRECISION  */
  YYSYMBOL_MEDIUM_PRECISION = 400,         /* MEDIUM_PRECISION  */
  YYSYMBOL_LOW_PRECISION = 401,            /* LOW_PRECISION  */
  YYSYMBOL_PRECISION = 402,                /* PRECISION  */
  YYSYMBOL_PACKED = 403,                   /* PACKED  */
  YYSYMBOL_RESOURCE = 404,                 /* RESOURCE  */
  YYSYMBOL_SUPERP = 405,                   /* SUPERP  */
  YYSYMBOL_FLOATCONSTANT = 406,            /* FLOATCONSTANT  */
  YYSYMBOL_INTCONSTANT = 407,              /* INTCONSTANT  */
  YYSYMBOL_UINTCONSTANT = 408,             /* UINTCONSTANT  */
  YYSYMBOL_BOOLCONSTANT = 409,             /* BOOLCONSTANT  */
  YYSYMBOL_IDENTIFIER = 410,               /* IDENTIFIER  */
  YYSYMBOL_TYPE_NAME = 411,                /* TYPE_NAME  */
  YYSYMBOL_CENTROID = 412,                 /* CENTROID  */
  YYSYMBOL_IN = 413,                       /* IN  */
  YYSYMBOL_OUT = 414,                      /* OUT  */
  YYSYMBOL_INOUT = 415,                    /* INOUT  */
  YYSYMBOL_STRUCT = 416,                   /* STRUCT  */
  YYSYMBOL_VOID = 417,                     /* VOID  */
  YYSYMBOL_WHILE = 418,                    /* WHILE  */
  YYSYMBOL_BREAK = 419,                    /* BREAK  */
  YYSYMBOL_CONTINUE = 420,                 /* CONTINUE  */
  YYSYMBOL_DO = 421,                       /* DO  */
  YYSYMBOL_ELSE = 422,                     /* ELSE  */
  YYSYMBOL_FOR = 423,                      /* FOR  */
  YYSYMBOL_IF = 424,                       /* IF  */
  YYSYMBOL_DISCARD = 425,                  /* DISCARD  */
  YYSYMBOL_RETURN = 426,                   /* RETURN  */
  YYSYMBOL_SWITCH = 427,                   /* SWITCH  */
  YYSYMBOL_CASE = 428,                     /* CASE  */
  YYSYMBOL_DEFAULT = 429,                  /* DEFAULT  */
  YYSYMBOL_TERMINATE_INVOCATION = 430,     /* TERMINATE_INVOCATION  */
  YYSYMBOL_TERMINATE_RAY = 431,            /* TERMINATE_RAY  */
  YYSYMBOL_IGNORE_INTERSECTION = 432,      /* IGNORE_INTERSECTION  */
  YYSYMBOL_UNIFORM = 433,                  /* UNIFORM  */
  YYSYMBOL_SHARED = 434,                   /* SHARED  */
  YYSYMBOL_BUFFER = 435,                   /* BUFFER  */
  YYSYMBOL_TILEIMAGEEXT = 436,             /* TILEIMAGEEXT  */
  YYSYMBOL_FLAT = 437,                     /* FLAT  */
  YYSYMBOL_SMOOTH = 438,                   /* SMOOTH  */
  YYSYMBOL_LAYOUT = 439,                   /* LAYOUT  */
  YYSYMBOL_DOUBLECONSTANT = 440,           /* DOUBLECONSTANT  */
  YYSYMBOL_INT16CONSTANT = 441,            /* INT16CONSTANT  */
  YYSYMBOL_UINT16CONSTANT = 442,           /* UINT16CONSTANT  */
  YYSYMBOL_FLOAT16CONSTANT = 443,          /* FLOAT16CONSTANT  */
  YYSYMBOL_INT32CONSTANT = 444,            /* INT32CONSTANT  */
  YYSYMBOL_UINT32CONSTANT = 445,           /* UINT32CONSTANT  */
  YYSYMBOL_INT64CONSTANT = 446,            /* INT64CONSTANT  */
  YYSYMBOL_UINT64CONSTANT = 447,           /* UINT64CONSTANT  */
  YYSYMBOL_SUBROUTINE = 448,               /* SUBROUTINE  */
  YYSYMBOL_DEMOTE = 449,                   /* DEMOTE  */
  YYSYMBOL_FUNCTION = 450,                 /* FUNCTION  */
  YYSYMBOL_PAYLOADNV = 451,                /* PAYLOADNV  */
  YYSYMBOL_PAYLOADINNV = 452,              /* PAYLOADINNV  */
  YYSYMBOL_HITATTRNV = 453,                /* HITATTRNV  */
  YYSYMBOL_CALLDATANV = 454,               /* CALLDATANV  */
  YYSYMBOL_CALLDATAINNV = 455,             /* CALLDATAINNV  */
  YYSYMBOL_PAYLOADEXT = 456,               /* PAYLOADEXT  */
  YYSYMBOL_PAYLOADINEXT = 457,             /* PAYLOADINEXT  */
  YYSYMBOL_HITATTREXT = 458,               /* HITATTREXT  */
  YYSYMBOL_CALLDATAEXT = 459,              /* CALLDATAEXT  */
  YYSYMBOL_CALLDATAINEXT = 460,            /* CALLDATAINEXT  */
  YYSYMBOL_PATCH = 461,                    /* PATCH  */
  YYSYMBOL_SAMPLE = 462,                   /* SAMPLE  */
  YYSYMBOL_NONUNIFORM = 463,               /* NONUNIFORM  */
  YYSYMBOL_RESOURCEHEAP = 464,             /* RESOURCEHEAP  */
  YYSYMBOL_SAMPLERHEAP = 465,              /* SAMPLERHEAP  */
  YYSYMBOL_COHERENT = 466,                 /* COHERENT  */
  YYSYMBOL_VOLATILE = 467,                 /* VOLATILE  */
  YYSYMBOL_RESTRICT = 468,                 /* RESTRICT  */
  YYSYMBOL_READONLY = 469,                 /* READONLY  */
  YYSYMBOL_WRITEONLY = 470,                /* WRITEONLY  */
  YYSYMBOL_NONTEMPORAL = 471,              /* NONTEMPORAL  */
  YYSYMBOL_DEVICECOHERENT = 472,           /* DEVICECOHERENT  */
  YYSYMBOL_QUEUEFAMILYCOHERENT = 473,      /* QUEUEFAMILYCOHERENT  */
  YYSYMBOL_WORKGROUPCOHERENT = 474,        /* WORKGROUPCOHERENT  */
  YYSYMBOL_SUBGROUPCOHERENT = 475,         /* SUBGROUPCOHERENT  */
  YYSYMBOL_NONPRIVATE = 476,               /* NONPRIVATE  */
  YYSYMBOL_SHADERCALLCOHERENT = 477,       /* SHADERCALLCOHERENT  */
  YYSYMBOL_NOPERSPECTIVE = 478,            /* NOPERSPECTIVE  */
  YYSYMBOL_EXPLICITINTERPAMD = 479,        /* EXPLICITINTERPAMD  */
  YYSYMBOL_PERVERTEXEXT = 480,             /* PERVERTEXEXT  */
  YYSYMBOL_PERVERTEXNV = 481,              /* PERVERTEXNV  */
  YYSYMBOL_PERPRIMITIVENV = 482,           /* PERPRIMITIVENV  */
  YYSYMBOL_PERVIEWNV = 483,                /* PERVIEWNV  */
  YYSYMBOL_PERTASKNV = 484,                /* PERTASKNV  */
  YYSYMBOL_PERPRIMITIVEEXT = 485,          /* PERPRIMITIVEEXT  */
  YYSYMBOL_TASKPAYLOADWORKGROUPEXT = 486,  /* TASKPAYLOADWORKGROUPEXT  */
  YYSYMBOL_PRECISE = 487,                  /* PRECISE  */
  YYSYMBOL_YYACCEPT = 488,                 /* $accept  */
  YYSYMBOL_variable_identifier = 489,      /* variable_identifier  */
  YYSYMBOL_primary_expression = 490,       /* primary_expression  */
  YYSYMBOL_postfix_expression = 491,       /* postfix_expression  */
  YYSYMBOL_integer_expression = 492,       /* integer_expression  */
  YYSYMBOL_function_call = 493,            /* function_call  */
  YYSYMBOL_function_call_or_method = 494,  /* function_call_or_method  */
  YYSYMBOL_function_call_generic = 495,    /* function_call_generic  */
  YYSYMBOL_function_call_header_no_parameters = 496, /* function_call_header_no_parameters  */
  YYSYMBOL_function_call_header_with_parameters = 497, /* function_call_header_with_parameters  */
  YYSYMBOL_function_call_header = 498,     /* function_call_header  */
  YYSYMBOL_function_identifier = 499,      /* function_identifier  */
  YYSYMBOL_unary_expression = 500,         /* unary_expression  */
  YYSYMBOL_unary_operator = 501,           /* unary_operator  */
  YYSYMBOL_multiplicative_expression = 502, /* multiplicative_expression  */
  YYSYMBOL_additive_expression = 503,      /* additive_expression  */
  YYSYMBOL_shift_expression = 504,         /* shift_expression  */
  YYSYMBOL_relational_expression = 505,    /* relational_expression  */
  YYSYMBOL_equality_expression = 506,      /* equality_expression  */
  YYSYMBOL_and_expression = 507,           /* and_expression  */
  YYSYMBOL_exclusive_or_expression = 508,  /* exclusive_or_expression  */
  YYSYMBOL_inclusive_or_expression = 509,  /* inclusive_or_expression  */
  YYSYMBOL_logical_and_expression = 510,   /* logical_and_expression  */
  YYSYMBOL_logical_xor_expression = 511,   /* logical_xor_expression  */
  YYSYMBOL_logical_or_expression = 512,    /* logical_or_expression  */
  YYSYMBOL_conditional_expression = 513,   /* conditional_expression  */
  YYSYMBOL_514_1 = 514,                    /* $@1  */
  YYSYMBOL_assignment_expression = 515,    /* assignment_expression  */
  YYSYMBOL_assignment_operator = 516,      /* assignment_operator  */
  YYSYMBOL_expression = 517,               /* expression  */
  YYSYMBOL_constant_expression = 518,      /* constant_expression  */
  YYSYMBOL_declaration = 519,              /* declaration  */
  YYSYMBOL_block_structure = 520,          /* block_structure  */
  YYSYMBOL_521_2 = 521,                    /* $@2  */
  YYSYMBOL_identifier_list = 522,          /* identifier_list  */
  YYSYMBOL_function_prototype = 523,       /* function_prototype  */
  YYSYMBOL_function_declarator = 524,      /* function_declarator  */
  YYSYMBOL_function_header_with_parameters = 525, /* function_header_with_parameters  */
  YYSYMBOL_function_header = 526,          /* function_header  */
  YYSYMBOL_parameter_declarator = 527,     /* parameter_declarator  */
  YYSYMBOL_parameter_declaration = 528,    /* parameter_declaration  */
  YYSYMBOL_parameter_type_specifier = 529, /* parameter_type_specifier  */
  YYSYMBOL_init_declarator_list = 530,     /* init_declarator_list  */
  YYSYMBOL_single_declaration = 531,       /* single_declaration  */
  YYSYMBOL_fully_specified_type = 532,     /* fully_specified_type  */
  YYSYMBOL_invariant_qualifier = 533,      /* invariant_qualifier  */
  YYSYMBOL_interpolation_qualifier = 534,  /* interpolation_qualifier  */
  YYSYMBOL_layout_qualifier = 535,         /* layout_qualifier  */
  YYSYMBOL_layout_qualifier_id_list = 536, /* layout_qualifier_id_list  */
  YYSYMBOL_layout_qualifier_id = 537,      /* layout_qualifier_id  */
  YYSYMBOL_precise_qualifier = 538,        /* precise_qualifier  */
  YYSYMBOL_type_qualifier = 539,           /* type_qualifier  */
  YYSYMBOL_single_type_qualifier = 540,    /* single_type_qualifier  */
  YYSYMBOL_storage_qualifier = 541,        /* storage_qualifier  */
  YYSYMBOL_non_uniform_qualifier = 542,    /* non_uniform_qualifier  */
  YYSYMBOL_type_name_list = 543,           /* type_name_list  */
  YYSYMBOL_type_specifier = 544,           /* type_specifier  */
  YYSYMBOL_array_specifier = 545,          /* array_specifier  */
  YYSYMBOL_type_parameter_specifier_opt = 546, /* type_parameter_specifier_opt  */
  YYSYMBOL_type_parameter_specifier = 547, /* type_parameter_specifier  */
  YYSYMBOL_type_parameter_specifier_list = 548, /* type_parameter_specifier_list  */
  YYSYMBOL_type_specifier_nonarray = 549,  /* type_specifier_nonarray  */
  YYSYMBOL_precision_qualifier = 550,      /* precision_qualifier  */
  YYSYMBOL_struct_specifier = 551,         /* struct_specifier  */
  YYSYMBOL_552_3 = 552,                    /* $@3  */
  YYSYMBOL_553_4 = 553,                    /* $@4  */
  YYSYMBOL_struct_declaration_list = 554,  /* struct_declaration_list  */
  YYSYMBOL_struct_declaration_with_heap = 555, /* struct_declaration_with_heap  */
  YYSYMBOL_block_heap_inner_structure = 556, /* block_heap_inner_structure  */
  YYSYMBOL_557_5 = 557,                    /* $@5  */
  YYSYMBOL_struct_declaration_without_heap = 558, /* struct_declaration_without_heap  */
  YYSYMBOL_struct_declaration = 559,       /* struct_declaration  */
  YYSYMBOL_struct_declarator_list = 560,   /* struct_declarator_list  */
  YYSYMBOL_struct_declarator = 561,        /* struct_declarator  */
  YYSYMBOL_initializer = 562,              /* initializer  */
  YYSYMBOL_initializer_list = 563,         /* initializer_list  */
  YYSYMBOL_declaration_statement = 564,    /* declaration_statement  */
  YYSYMBOL_statement = 565,                /* statement  */
  YYSYMBOL_simple_statement = 566,         /* simple_statement  */
  YYSYMBOL_demote_statement = 567,         /* demote_statement  */
  YYSYMBOL_compound_statement = 568,       /* compound_statement  */
  YYSYMBOL_569_6 = 569,                    /* $@6  */
  YYSYMBOL_570_7 = 570,                    /* $@7  */
  YYSYMBOL_statement_no_new_scope = 571,   /* statement_no_new_scope  */
  YYSYMBOL_statement_scoped = 572,         /* statement_scoped  */
  YYSYMBOL_573_8 = 573,                    /* $@8  */
  YYSYMBOL_574_9 = 574,                    /* $@9  */
  YYSYMBOL_compound_statement_no_new_scope = 575, /* compound_statement_no_new_scope  */
  YYSYMBOL_statement_list = 576,           /* statement_list  */
  YYSYMBOL_expression_statement = 577,     /* expression_statement  */
  YYSYMBOL_selection_statement = 578,      /* selection_statement  */
  YYSYMBOL_selection_statement_nonattributed = 579, /* selection_statement_nonattributed  */
  YYSYMBOL_selection_rest_statement = 580, /* selection_rest_statement  */
  YYSYMBOL_condition = 581,                /* condition  */
  YYSYMBOL_switch_statement = 582,         /* switch_statement  */
  YYSYMBOL_switch_statement_nonattributed = 583, /* switch_statement_nonattributed  */
  YYSYMBOL_584_10 = 584,                   /* $@10  */
  YYSYMBOL_switch_statement_list = 585,    /* switch_statement_list  */
  YYSYMBOL_case_label = 586,               /* case_label  */
  YYSYMBOL_iteration_statement = 587,      /* iteration_statement  */
  YYSYMBOL_iteration_statement_nonattributed = 588, /* iteration_statement_nonattributed  */
  YYSYMBOL_589_11 = 589,                   /* $@11  */
  YYSYMBOL_590_12 = 590,                   /* $@12  */
  YYSYMBOL_591_13 = 591,                   /* $@13  */
  YYSYMBOL_for_init_statement = 592,       /* for_init_statement  */
  YYSYMBOL_conditionopt = 593,             /* conditionopt  */
  YYSYMBOL_for_rest_statement = 594,       /* for_rest_statement  */
  YYSYMBOL_jump_statement = 595,           /* jump_statement  */
  YYSYMBOL_translation_unit = 596,         /* translation_unit  */
  YYSYMBOL_external_declaration = 597,     /* external_declaration  */
  YYSYMBOL_function_definition = 598,      /* function_definition  */
  YYSYMBOL_599_14 = 599,                   /* $@14  */
  YYSYMBOL_attribute = 600,                /* attribute  */
  YYSYMBOL_attribute_list = 601,           /* attribute_list  */
  YYSYMBOL_single_attribute = 602,         /* single_attribute  */
  YYSYMBOL_spirv_requirements_list = 603,  /* spirv_requirements_list  */
  YYSYMBOL_spirv_requirements_parameter = 604, /* spirv_requirements_parameter  */
  YYSYMBOL_spirv_extension_list = 605,     /* spirv_extension_list  */
  YYSYMBOL_spirv_capability_list = 606,    /* spirv_capability_list  */
  YYSYMBOL_spirv_execution_mode_qualifier = 607, /* spirv_execution_mode_qualifier  */
  YYSYMBOL_spirv_execution_mode_parameter_list = 608, /* spirv_execution_mode_parameter_list  */
  YYSYMBOL_spirv_execution_mode_parameter = 609, /* spirv_execution_mode_parameter  */
  YYSYMBOL_spirv_execution_mode_id_parameter_list = 610, /* spirv_execution_mode_id_parameter_list  */
  YYSYMBOL_spirv_storage_class_qualifier = 611, /* spirv_storage_class_qualifier  */
  YYSYMBOL_spirv_decorate_qualifier = 612, /* spirv_decorate_qualifier  */
  YYSYMBOL_spirv_decorate_parameter_list = 613, /* spirv_decorate_parameter_list  */
  YYSYMBOL_spirv_decorate_parameter = 614, /* spirv_decorate_parameter  */
  YYSYMBOL_spirv_decorate_id_parameter_list = 615, /* spirv_decorate_id_parameter_list  */
  YYSYMBOL_spirv_decorate_id_parameter = 616, /* spirv_decorate_id_parameter  */
  YYSYMBOL_spirv_decorate_string_parameter_list = 617, /* spirv_decorate_string_parameter_list  */
  YYSYMBOL_spirv_type_specifier = 618,     /* spirv_type_specifier  */
  YYSYMBOL_spirv_type_parameter_list = 619, /* spirv_type_parameter_list  */
  YYSYMBOL_spirv_type_parameter = 620,     /* spirv_type_parameter  */
  YYSYMBOL_spirv_instruction_qualifier = 621, /* spirv_instruction_qualifier  */
  YYSYMBOL_spirv_instruction_qualifier_list = 622, /* spirv_instruction_qualifier_list  */
  YYSYMBOL_spirv_instruction_qualifier_id = 623 /* spirv_instruction_qualifier_id  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;


/* Second part of user prologue.  */
#line 111 "MachineIndependent/glslang.y"


#define parseContext (*pParseContext)
#define yyerror(context, msg) context->parserError(msg)

extern int yylex(YYSTYPE*, TParseContext&);


#line 763 "MachineIndependent/glslang_tab.cpp"


#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  476
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   13820

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  488
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  136
/* YYNRULES -- Number of rules.  */
#define YYNRULES  731
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  983

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   742


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   365,   365,   371,   374,   379,   382,   385,   389,   392,
     395,   399,   403,   407,   411,   415,   419,   425,   432,   435,
     438,   441,   444,   449,   457,   464,   471,   477,   481,   488,
     491,   497,   515,   540,   548,   553,   580,   588,   594,   598,
     602,   622,   623,   624,   625,   631,   632,   637,   642,   651,
     652,   657,   665,   666,   672,   681,   682,   687,   692,   697,
     705,   706,   715,   727,   728,   737,   738,   747,   748,   757,
     758,   766,   767,   775,   776,   784,   785,   785,   803,   804,
     820,   824,   828,   832,   837,   841,   845,   849,   853,   857,
     861,   868,   871,   882,   889,   894,   901,   906,   911,   918,
     921,   924,   927,   932,   940,   940,   952,   956,   963,   968,
     976,   984,   996,   999,  1006,  1019,  1039,  1046,  1069,  1084,
    1103,  1114,  1125,  1135,  1145,  1155,  1164,  1167,  1172,  1177,
    1182,  1190,  1195,  1201,  1206,  1211,  1220,  1230,  1257,  1266,
    1273,  1280,  1287,  1294,  1302,  1310,  1320,  1330,  1337,  1347,
    1353,  1356,  1363,  1367,  1371,  1379,  1388,  1391,  1402,  1405,
    1408,  1412,  1416,  1420,  1424,  1427,  1432,  1436,  1441,  1449,
    1453,  1458,  1464,  1470,  1477,  1482,  1487,  1495,  1500,  1512,
    1526,  1532,  1537,  1542,  1547,  1555,  1563,  1571,  1579,  1587,
    1595,  1603,  1611,  1619,  1627,  1634,  1641,  1645,  1650,  1655,
    1660,  1665,  1670,  1675,  1679,  1683,  1687,  1691,  1695,  1701,
    1707,  1717,  1724,  1727,  1735,  1742,  1753,  1758,  1766,  1770,
    1780,  1783,  1789,  1795,  1801,  1809,  1819,  1823,  1827,  1831,
    1836,  1840,  1845,  1850,  1855,  1860,  1865,  1870,  1875,  1880,
    1885,  1891,  1897,  1903,  1908,  1913,  1918,  1923,  1928,  1933,
    1938,  1943,  1948,  1953,  1958,  1963,  1970,  1975,  1980,  1985,
    1990,  1995,  2000,  2005,  2010,  2015,  2020,  2025,  2030,  2035,
    2040,  2048,  2056,  2064,  2070,  2076,  2082,  2088,  2094,  2100,
    2106,  2112,  2118,  2124,  2130,  2136,  2142,  2148,  2154,  2160,
    2166,  2172,  2178,  2184,  2190,  2196,  2202,  2208,  2214,  2220,
    2226,  2232,  2238,  2244,  2250,  2256,  2262,  2268,  2274,  2280,
    2286,  2292,  2298,  2304,  2310,  2316,  2324,  2332,  2340,  2348,
    2356,  2364,  2372,  2380,  2388,  2396,  2404,  2412,  2418,  2424,
    2430,  2436,  2442,  2448,  2454,  2460,  2466,  2472,  2478,  2484,
    2490,  2496,  2502,  2508,  2514,  2520,  2526,  2532,  2538,  2544,
    2550,  2556,  2562,  2568,  2574,  2580,  2586,  2592,  2598,  2604,
    2610,  2616,  2622,  2628,  2632,  2636,  2640,  2645,  2650,  2655,
    2660,  2665,  2670,  2675,  2680,  2685,  2690,  2695,  2700,  2705,
    2710,  2716,  2722,  2728,  2734,  2740,  2746,  2752,  2758,  2764,
    2770,  2776,  2782,  2788,  2793,  2798,  2803,  2808,  2813,  2818,
    2823,  2828,  2833,  2838,  2843,  2848,  2853,  2858,  2863,  2868,
    2873,  2878,  2883,  2888,  2893,  2898,  2903,  2908,  2913,  2918,
    2923,  2928,  2933,  2938,  2943,  2948,  2953,  2959,  2965,  2970,
    2975,  2980,  2986,  2991,  2996,  3001,  3007,  3012,  3017,  3022,
    3028,  3033,  3038,  3043,  3049,  3055,  3061,  3067,  3072,  3078,
    3084,  3090,  3095,  3100,  3105,  3110,  3115,  3121,  3126,  3131,
    3136,  3142,  3147,  3152,  3157,  3163,  3168,  3173,  3178,  3184,
    3189,  3194,  3199,  3205,  3210,  3215,  3220,  3226,  3231,  3236,
    3241,  3247,  3252,  3257,  3262,  3268,  3273,  3278,  3283,  3289,
    3294,  3299,  3304,  3310,  3315,  3320,  3325,  3331,  3336,  3341,
    3346,  3352,  3357,  3362,  3367,  3373,  3378,  3383,  3388,  3394,
    3399,  3404,  3409,  3415,  3420,  3425,  3430,  3435,  3440,  3445,
    3450,  3455,  3460,  3465,  3470,  3475,  3480,  3485,  3490,  3495,
    3500,  3505,  3510,  3515,  3520,  3525,  3530,  3535,  3541,  3547,
    3553,  3559,  3565,  3571,  3577,  3584,  3591,  3597,  3603,  3609,
    3615,  3622,  3629,  3636,  3643,  3648,  3653,  3657,  3663,  3669,
    3675,  3679,  3683,  3687,  3692,  3708,  3713,  3718,  3726,  3726,
    3742,  3742,  3752,  3755,  3758,  3768,  3781,  3798,  3798,  3813,
    3816,  3829,  3851,  3878,  3882,  3888,  3893,  3904,  3907,  3913,
    3919,  3928,  3931,  3937,  3941,  3942,  3948,  3949,  3950,  3951,
    3952,  3953,  3954,  3955,  3959,  3967,  3968,  3972,  3968,  3986,
    3987,  3991,  3991,  3998,  3998,  4012,  4015,  4025,  4033,  4044,
    4045,  4049,  4052,  4059,  4066,  4070,  4078,  4082,  4091,  4094,
    4101,  4101,  4121,  4124,  4130,  4142,  4154,  4157,  4165,  4165,
    4184,  4184,  4206,  4206,  4227,  4230,  4236,  4239,  4245,  4249,
    4256,  4261,  4266,  4273,  4276,  4280,  4284,  4288,  4297,  4301,
    4310,  4313,  4316,  4324,  4324,  4366,  4371,  4374,  4379,  4382,
    4387,  4390,  4395,  4398,  4403,  4406,  4411,  4414,  4419,  4423,
    4428,  4432,  4437,  4441,  4448,  4451,  4456,  4459,  4462,  4465,
    4468,  4473,  4482,  4493,  4498,  4506,  4510,  4515,  4519,  4524,
    4528,  4533,  4537,  4544,  4547,  4552,  4555,  4558,  4561,  4566,
    4569,  4574,  4580,  4583,  4586,  4589,  4594,  4598,  4603,  4607,
    4612,  4616,  4623,  4626,  4631,  4634,  4639,  4642,  4648,  4651,
    4656,  4659
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "CONST", "BOOL", "INT",
  "UINT", "FLOAT", "BVEC2", "BVEC3", "BVEC4", "IVEC2", "IVEC3", "IVEC4",
  "UVEC2", "UVEC3", "UVEC4", "VEC2", "VEC3", "VEC4", "MAT2", "MAT3",
  "MAT4", "MAT2X2", "MAT2X3", "MAT2X4", "MAT3X2", "MAT3X3", "MAT3X4",
  "MAT4X2", "MAT4X3", "MAT4X4", "SAMPLER2D", "SAMPLER3D", "SAMPLERCUBE",
  "SAMPLER2DSHADOW", "SAMPLERCUBESHADOW", "SAMPLER2DARRAY",
  "SAMPLER2DARRAYSHADOW", "ISAMPLER2D", "ISAMPLER3D", "ISAMPLERCUBE",
  "ISAMPLER2DARRAY", "USAMPLER2D", "USAMPLER3D", "USAMPLERCUBE",
  "USAMPLER2DARRAY", "SAMPLER", "SAMPLERSHADOW", "TEXTURE2D", "TEXTURE3D",
  "TEXTURECUBE", "TEXTURE2DARRAY", "ITEXTURE2D", "ITEXTURE3D",
  "ITEXTURECUBE", "ITEXTURE2DARRAY", "UTEXTURE2D", "UTEXTURE3D",
  "UTEXTURECUBE", "UTEXTURE2DARRAY", "ATTRIBUTE", "VARYING", "FLOATE5M2_T",
  "FLOATE4M3_T", "BFLOAT16_T", "FLOAT16_T", "FLOAT32_T", "DOUBLE",
  "FLOAT64_T", "INT64_T", "UINT64_T", "INT32_T", "UINT32_T", "INT16_T",
  "UINT16_T", "INT8_T", "UINT8_T", "I64VEC2", "I64VEC3", "I64VEC4",
  "U64VEC2", "U64VEC3", "U64VEC4", "I32VEC2", "I32VEC3", "I32VEC4",
  "U32VEC2", "U32VEC3", "U32VEC4", "I16VEC2", "I16VEC3", "I16VEC4",
  "U16VEC2", "U16VEC3", "U16VEC4", "I8VEC2", "I8VEC3", "I8VEC4", "U8VEC2",
  "U8VEC3", "U8VEC4", "DVEC2", "DVEC3", "DVEC4", "DMAT2", "DMAT3", "DMAT4",
  "BF16VEC2", "BF16VEC3", "BF16VEC4", "FE5M2VEC2", "FE5M2VEC3",
  "FE5M2VEC4", "FE4M3VEC2", "FE4M3VEC3", "FE4M3VEC4", "F16VEC2", "F16VEC3",
  "F16VEC4", "F16MAT2", "F16MAT3", "F16MAT4", "F32VEC2", "F32VEC3",
  "F32VEC4", "F32MAT2", "F32MAT3", "F32MAT4", "F64VEC2", "F64VEC3",
  "F64VEC4", "F64MAT2", "F64MAT3", "F64MAT4", "DMAT2X2", "DMAT2X3",
  "DMAT2X4", "DMAT3X2", "DMAT3X3", "DMAT3X4", "DMAT4X2", "DMAT4X3",
  "DMAT4X4", "F16MAT2X2", "F16MAT2X3", "F16MAT2X4", "F16MAT3X2",
  "F16MAT3X3", "F16MAT3X4", "F16MAT4X2", "F16MAT4X3", "F16MAT4X4",
  "F32MAT2X2", "F32MAT2X3", "F32MAT2X4", "F32MAT3X2", "F32MAT3X3",
  "F32MAT3X4", "F32MAT4X2", "F32MAT4X3", "F32MAT4X4", "F64MAT2X2",
  "F64MAT2X3", "F64MAT2X4", "F64MAT3X2", "F64MAT3X3", "F64MAT3X4",
  "F64MAT4X2", "F64MAT4X3", "F64MAT4X4", "ATOMIC_UINT", "ACCSTRUCTNV",
  "ACCSTRUCTEXT", "RAYQUERYEXT", "FCOOPMATNV", "ICOOPMATNV", "UCOOPMATNV",
  "COOPMAT", "COOPVECNV", "VECTOR", "HITOBJECTNV", "HITOBJECTATTRNV",
  "HITOBJECTEXT", "HITOBJECTATTREXT", "TENSORLAYOUTNV", "TENSORVIEWNV",
  "TENSORARM", "SAMPLERCUBEARRAY", "SAMPLERCUBEARRAYSHADOW",
  "ISAMPLERCUBEARRAY", "USAMPLERCUBEARRAY", "SAMPLER1D", "SAMPLER1DARRAY",
  "SAMPLER1DARRAYSHADOW", "ISAMPLER1D", "SAMPLER1DSHADOW", "SAMPLER2DRECT",
  "SAMPLER2DRECTSHADOW", "ISAMPLER2DRECT", "USAMPLER2DRECT",
  "SAMPLERBUFFER", "ISAMPLERBUFFER", "USAMPLERBUFFER", "SAMPLER2DMS",
  "ISAMPLER2DMS", "USAMPLER2DMS", "SAMPLER2DMSARRAY", "ISAMPLER2DMSARRAY",
  "USAMPLER2DMSARRAY", "SAMPLEREXTERNALOES", "SAMPLEREXTERNAL2DY2YEXT",
  "ISAMPLER1DARRAY", "USAMPLER1D", "USAMPLER1DARRAY", "F16SAMPLER1D",
  "F16SAMPLER2D", "F16SAMPLER3D", "F16SAMPLER2DRECT", "F16SAMPLERCUBE",
  "F16SAMPLER1DARRAY", "F16SAMPLER2DARRAY", "F16SAMPLERCUBEARRAY",
  "F16SAMPLERBUFFER", "F16SAMPLER2DMS", "F16SAMPLER2DMSARRAY",
  "F16SAMPLER1DSHADOW", "F16SAMPLER2DSHADOW", "F16SAMPLER1DARRAYSHADOW",
  "F16SAMPLER2DARRAYSHADOW", "F16SAMPLER2DRECTSHADOW",
  "F16SAMPLERCUBESHADOW", "F16SAMPLERCUBEARRAYSHADOW", "IMAGE1D",
  "IIMAGE1D", "UIMAGE1D", "IMAGE2D", "IIMAGE2D", "UIMAGE2D", "IMAGE3D",
  "IIMAGE3D", "UIMAGE3D", "IMAGE2DRECT", "IIMAGE2DRECT", "UIMAGE2DRECT",
  "IMAGECUBE", "IIMAGECUBE", "UIMAGECUBE", "IMAGEBUFFER", "IIMAGEBUFFER",
  "UIMAGEBUFFER", "IMAGE1DARRAY", "IIMAGE1DARRAY", "UIMAGE1DARRAY",
  "IMAGE2DARRAY", "IIMAGE2DARRAY", "UIMAGE2DARRAY", "IMAGECUBEARRAY",
  "IIMAGECUBEARRAY", "UIMAGECUBEARRAY", "IMAGE2DMS", "IIMAGE2DMS",
  "UIMAGE2DMS", "IMAGE2DMSARRAY", "IIMAGE2DMSARRAY", "UIMAGE2DMSARRAY",
  "F16IMAGE1D", "F16IMAGE2D", "F16IMAGE3D", "F16IMAGE2DRECT",
  "F16IMAGECUBE", "F16IMAGE1DARRAY", "F16IMAGE2DARRAY",
  "F16IMAGECUBEARRAY", "F16IMAGEBUFFER", "F16IMAGE2DMS",
  "F16IMAGE2DMSARRAY", "I64IMAGE1D", "U64IMAGE1D", "I64IMAGE2D",
  "U64IMAGE2D", "I64IMAGE3D", "U64IMAGE3D", "I64IMAGE2DRECT",
  "U64IMAGE2DRECT", "I64IMAGECUBE", "U64IMAGECUBE", "I64IMAGEBUFFER",
  "U64IMAGEBUFFER", "I64IMAGE1DARRAY", "U64IMAGE1DARRAY",
  "I64IMAGE2DARRAY", "U64IMAGE2DARRAY", "I64IMAGECUBEARRAY",
  "U64IMAGECUBEARRAY", "I64IMAGE2DMS", "U64IMAGE2DMS", "I64IMAGE2DMSARRAY",
  "U64IMAGE2DMSARRAY", "TEXTURECUBEARRAY", "ITEXTURECUBEARRAY",
  "UTEXTURECUBEARRAY", "TEXTURE1D", "ITEXTURE1D", "UTEXTURE1D",
  "TEXTURE1DARRAY", "ITEXTURE1DARRAY", "UTEXTURE1DARRAY", "TEXTURE2DRECT",
  "ITEXTURE2DRECT", "UTEXTURE2DRECT", "TEXTUREBUFFER", "ITEXTUREBUFFER",
  "UTEXTUREBUFFER", "TEXTURE2DMS", "ITEXTURE2DMS", "UTEXTURE2DMS",
  "TEXTURE2DMSARRAY", "ITEXTURE2DMSARRAY", "UTEXTURE2DMSARRAY",
  "F16TEXTURE1D", "F16TEXTURE2D", "F16TEXTURE3D", "F16TEXTURE2DRECT",
  "F16TEXTURECUBE", "F16TEXTURE1DARRAY", "F16TEXTURE2DARRAY",
  "F16TEXTURECUBEARRAY", "F16TEXTUREBUFFER", "F16TEXTURE2DMS",
  "F16TEXTURE2DMSARRAY", "SUBPASSINPUT", "SUBPASSINPUTMS", "ISUBPASSINPUT",
  "ISUBPASSINPUTMS", "USUBPASSINPUT", "USUBPASSINPUTMS", "F16SUBPASSINPUT",
  "F16SUBPASSINPUTMS", "SPIRV_INSTRUCTION", "SPIRV_EXECUTION_MODE",
  "SPIRV_EXECUTION_MODE_ID", "SPIRV_DECORATE", "SPIRV_DECORATE_ID",
  "SPIRV_DECORATE_STRING", "SPIRV_TYPE", "SPIRV_STORAGE_CLASS",
  "SPIRV_BY_REFERENCE", "SPIRV_LITERAL", "ATTACHMENTEXT", "IATTACHMENTEXT",
  "UATTACHMENTEXT", "LEFT_OP", "RIGHT_OP", "INC_OP", "DEC_OP", "LE_OP",
  "GE_OP", "EQ_OP", "NE_OP", "AND_OP", "OR_OP", "XOR_OP", "MUL_ASSIGN",
  "DIV_ASSIGN", "ADD_ASSIGN", "MOD_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN",
  "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN", "SUB_ASSIGN", "STRING_LITERAL",
  "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACKET", "RIGHT_BRACKET",
  "LEFT_BRACE", "RIGHT_BRACE", "DOT", "COMMA", "COLON", "EQUAL",
  "SEMICOLON", "BANG", "DASH", "TILDE", "PLUS", "STAR", "SLASH", "PERCENT",
  "LEFT_ANGLE", "RIGHT_ANGLE", "VERTICAL_BAR", "CARET", "AMPERSAND",
  "QUESTION", "INVARIANT", "HIGH_PRECISION", "MEDIUM_PRECISION",
  "LOW_PRECISION", "PRECISION", "PACKED", "RESOURCE", "SUPERP",
  "FLOATCONSTANT", "INTCONSTANT", "UINTCONSTANT", "BOOLCONSTANT",
  "IDENTIFIER", "TYPE_NAME", "CENTROID", "IN", "OUT", "INOUT", "STRUCT",
  "VOID", "WHILE", "BREAK", "CONTINUE", "DO", "ELSE", "FOR", "IF",
  "DISCARD", "RETURN", "SWITCH", "CASE", "DEFAULT", "TERMINATE_INVOCATION",
  "TERMINATE_RAY", "IGNORE_INTERSECTION", "UNIFORM", "SHARED", "BUFFER",
  "TILEIMAGEEXT", "FLAT", "SMOOTH", "LAYOUT", "DOUBLECONSTANT",
  "INT16CONSTANT", "UINT16CONSTANT", "FLOAT16CONSTANT", "INT32CONSTANT",
  "UINT32CONSTANT", "INT64CONSTANT", "UINT64CONSTANT", "SUBROUTINE",
  "DEMOTE", "FUNCTION", "PAYLOADNV", "PAYLOADINNV", "HITATTRNV",
  "CALLDATANV", "CALLDATAINNV", "PAYLOADEXT", "PAYLOADINEXT", "HITATTREXT",
  "CALLDATAEXT", "CALLDATAINEXT", "PATCH", "SAMPLE", "NONUNIFORM",
  "RESOURCEHEAP", "SAMPLERHEAP", "COHERENT", "VOLATILE", "RESTRICT",
  "READONLY", "WRITEONLY", "NONTEMPORAL", "DEVICECOHERENT",
  "QUEUEFAMILYCOHERENT", "WORKGROUPCOHERENT", "SUBGROUPCOHERENT",
  "NONPRIVATE", "SHADERCALLCOHERENT", "NOPERSPECTIVE", "EXPLICITINTERPAMD",
  "PERVERTEXEXT", "PERVERTEXNV", "PERPRIMITIVENV", "PERVIEWNV",
  "PERTASKNV", "PERPRIMITIVEEXT", "TASKPAYLOADWORKGROUPEXT", "PRECISE",
  "$accept", "variable_identifier", "primary_expression",
  "postfix_expression", "integer_expression", "function_call",
  "function_call_or_method", "function_call_generic",
  "function_call_header_no_parameters",
  "function_call_header_with_parameters", "function_call_header",
  "function_identifier", "unary_expression", "unary_operator",
  "multiplicative_expression", "additive_expression", "shift_expression",
  "relational_expression", "equality_expression", "and_expression",
  "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_xor_expression",
  "logical_or_expression", "conditional_expression", "$@1",
  "assignment_expression", "assignment_operator", "expression",
  "constant_expression", "declaration", "block_structure", "$@2",
  "identifier_list", "function_prototype", "function_declarator",
  "function_header_with_parameters", "function_header",
  "parameter_declarator", "parameter_declaration",
  "parameter_type_specifier", "init_declarator_list", "single_declaration",
  "fully_specified_type", "invariant_qualifier", "interpolation_qualifier",
  "layout_qualifier", "layout_qualifier_id_list", "layout_qualifier_id",
  "precise_qualifier", "type_qualifier", "single_type_qualifier",
  "storage_qualifier", "non_uniform_qualifier", "type_name_list",
  "type_specifier", "array_specifier", "type_parameter_specifier_opt",
  "type_parameter_specifier", "type_parameter_specifier_list",
  "type_specifier_nonarray", "precision_qualifier", "struct_specifier",
  "$@3", "$@4", "struct_declaration_list", "struct_declaration_with_heap",
  "block_heap_inner_structure", "$@5", "struct_declaration_without_heap",
  "struct_declaration", "struct_declarator_list", "struct_declarator",
  "initializer", "initializer_list", "declaration_statement", "statement",
  "simple_statement", "demote_statement", "compound_statement", "$@6",
  "$@7", "statement_no_new_scope", "statement_scoped", "$@8", "$@9",
  "compound_statement_no_new_scope", "statement_list",
  "expression_statement", "selection_statement",
  "selection_statement_nonattributed", "selection_rest_statement",
  "condition", "switch_statement", "switch_statement_nonattributed",
  "$@10", "switch_statement_list", "case_label", "iteration_statement",
  "iteration_statement_nonattributed", "$@11", "$@12", "$@13",
  "for_init_statement", "conditionopt", "for_rest_statement",
  "jump_statement", "translation_unit", "external_declaration",
  "function_definition", "$@14", "attribute", "attribute_list",
  "single_attribute", "spirv_requirements_list",
  "spirv_requirements_parameter", "spirv_extension_list",
  "spirv_capability_list", "spirv_execution_mode_qualifier",
  "spirv_execution_mode_parameter_list", "spirv_execution_mode_parameter",
  "spirv_execution_mode_id_parameter_list",
  "spirv_storage_class_qualifier", "spirv_decorate_qualifier",
  "spirv_decorate_parameter_list", "spirv_decorate_parameter",
  "spirv_decorate_id_parameter_list", "spirv_decorate_id_parameter",
  "spirv_decorate_string_parameter_list", "spirv_type_specifier",
  "spirv_type_parameter_list", "spirv_type_parameter",
  "spirv_instruction_qualifier", "spirv_instruction_qualifier_list",
  "spirv_instruction_qualifier_id", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-832)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-726)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    4866,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -320,  -282,  -220,
    -210,  -141,  -139,  -102,   -92,  -832,  -832,  -832,  -832,  -832,
    -244,  -832,  -832,  -832,  -832,  -832,   -54,  -832,  -832,  -832,
    -832,  -832,  -339,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
     -90,   -88,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -352,  -250,  -197,  -195,  8746,  -130,  -832,  -211,  -832,
    -832,  -832,  -832,  5836,  -832,  -832,  -832,  -832,  -168,  -832,
    -832,   986,  -832,  -832,  8746,   -93,  -832,  -832,  -832,  6321,
     -76,  -185,  -151,  -150,  -146,  -145,   -76,  -144,   -74, 13370,
    -832,   -34,  -375,   -50,  -832,  -302,  -832,   -22,   -15,  6806,
    -832,  -832,  -832,  8746,   -48,   -47,  -832,  -247,  -832,   -14,
    -114,  -832,  -832, 11990,   -11,  -832,  -832,  -832,    -9,   -43,
    8746,  -832,   -16,   -13,   -12,  -832,  -251,  -832,  -242,   -10,
      -7,    -5,    -4,  -240,    -2,    -1,     1,     6,     7,     8,
    -225,    -3,    10,    11,  -215,  -832,    12,  8746,  -832,     3,
    -832,  -216,  -832,  -832,  -214, 10150,  -832,  -296,  1471,  -832,
    -832,    14,  -832,  -832,  -832,  -290,  -279,  -832, 10610,  -276,
    -832,   -39,  -832, 11990, 11990,  -832, 11990,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -299,  -832,  -832,  -832,
      15,  -212, 12450,    21,  -832, 11990,  -832,    23,  -336,    22,
     -15,    25,  -832,  -323,   -76,  -832,   -29,  -832,  -340,    24,
    -136, 11990,  -134,  -832,  -159,  -129,  -178,  -110,    28,  -109,
     -76,  -832, 12910,  -832,  -108, 11990,    26,   -74,  -832,  7291,
     -27,    27,  8746,   -27,  8746,  -832,  8746, 11990,  -832,  -375,
    -832,    -8,  -832,  -832,   -41,  -344,  -113,  -331,  -253,    13,
      16,    18,    44,    46,  -327,    33,  -832, 11070,  -832,    34,
    -832,  -832,    40,    32,    36,  -832,    47,    48,    41, 11530,
      52, 11990,    45,    54,    61,    62,    63,  -252,  -832,  -832,
    -104,  -832,  -250,    55,    23,  -832,  -832,  -832,  -832,  -832,
    1956,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    5351,    38, 10610,    22, 10610,  -275,  9230,  -832,  -832, 10610,
    8746,  -832,  -832,  -832,  -201,  -832,  -832, 11990,    20,  -832,
    -832, 11990,    75,  -832,  -832,  -832, 11990,  -832,  -832,  -832,
    -343,  -832,  -832,  -200,    68,  -832,  -832,  -832,  -832,  -832,
    -832,  -199,  -832,  -198,  -832,  -832,  -196,    73,  -832,  -832,
    -832,  -832,  -188,  -832,  -186,  -832,  -832,  -832,  -832,  -832,
    -184,  -832,    74,  -832,  -179,    76,  -177,    68,  -832,  -329,
    -175,  -832,    81,    83,  -832,  -832,  -832,   -27,   -11,   -56,
    -832,  -832,  8746,  8746,   -55,  -832,  -832,    85,  -832,  -832,
    -832, 11990, 11990, 11990, 11990, 11990, 11990, 11990, 11990, 11990,
   11990, 11990, 11990, 11990, 11990, 11990, 11990, 11990, 11990, 11990,
    -832,  -832,  -832,    88,  -832,  2441,  -832,  -832,  -832,  2441,
    -832, 11990,  -832,  -832,   -51, 11990,   -77,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832, 11990, 11990,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -832,  -832, 10610,  -832,  -832,  -126,  -832,  7776,
    -832,    89,    86,  -832,  -832,  -832,  -832,  -832,  -154,  -143,
    -832,  -322,  -832,  -340,  -832,  -340,  -832, 11990, 11990,  -832,
    -159,  -832,  -159,  -832,  -178,  -178,  -832,    96,    28,  -832,
   12910,  -832, 11990,  -832,  -832,  8746,   -46,    22,   -27,  -832,
    -832,  -832,  -832,  -832,  -832,   -41,   -41,  -344,  -344,  -113,
    -113,  -113,  -113,  -331,  -331,  -253,    13,    16,    18,    44,
      46, 11990,  -832,  2441,  4381,    53,  3896,  -174,  -832,  -172,
    -832,  -832,  -832,  -832,  -832,  9690,  -832,  -832,  -832,    97,
    -832,    67,  -832,  -167,  -832,  -165,  -832,  -164,  -832,  -163,
    -832,  -162,  -160,  -832,  8261,  -832,  -832,   -42,    98,    86,
      65,    87,   102,  -832,  -832,  4381,   100,  -832,  -832,  -832,
    -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832, 11990,
    -832,   101,  2926, 11990,  -832,    95,   106,    64,   105,  3411,
    -832,   107,  -832, 10610,  -832,  -832,  -832,  -155, 11990,  2926,
     100,  -832,  -832,  2441,  -832,   103,    86,  -832,  -832,  2441,
     109,  -832,  -832
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,   169,   230,   228,   229,   227,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   231,   232,   233,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     368,   369,   370,   371,   372,   373,   374,   394,   395,   396,
     397,   398,   399,   400,   409,   422,   423,   410,   411,   413,
     412,   414,   415,   416,   417,   418,   419,   420,   421,   178,
     179,   257,   258,   256,   259,   260,   255,   261,   268,   269,
     266,   267,   264,   265,   262,   263,   300,   301,   302,   312,
     313,   314,   297,   298,   299,   309,   310,   311,   294,   295,
     296,   306,   307,   308,   291,   292,   293,   303,   304,   305,
     270,   271,   272,   315,   316,   317,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   327,   328,
     329,   285,   286,   287,   339,   340,   341,   288,   289,   290,
     351,   352,   353,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   366,
     363,   364,   365,   550,   551,   552,   553,   557,   559,   561,
     185,   562,   186,   554,   555,   558,   378,   379,   402,   405,
     367,   376,   377,   393,   375,   424,   425,   428,   429,   430,
     432,   433,   434,   436,   437,   438,   440,   441,   537,   538,
     401,   403,   404,   380,   381,   382,   426,   383,   387,   388,
     391,   431,   435,   439,   384,   385,   389,   390,   427,   386,
     392,   471,   473,   474,   475,   477,   478,   479,   481,   482,
     483,   485,   486,   487,   489,   490,   491,   493,   494,   495,
     497,   498,   499,   501,   502,   503,   505,   506,   507,   509,
     510,   511,   513,   514,   472,   476,   480,   484,   488,   496,
     500,   504,   492,   508,   512,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
     530,   531,   532,   533,   534,   535,   536,   406,   407,   408,
     442,   451,   453,   447,   452,   454,   455,   457,   458,   459,
     461,   462,   463,   465,   466,   467,   469,   470,   443,   444,
     445,   456,   446,   448,   449,   450,   460,   464,   468,   542,
     543,   546,   547,   548,   549,   544,   545,     0,     0,     0,
       0,     0,     0,     0,     0,   167,   168,   539,   540,   541,
       0,   662,   138,   565,   566,   567,     0,   564,   173,   171,
     172,   170,     0,   226,   174,   176,   177,   175,   140,   139,
       0,   208,   556,   188,   190,   184,   192,   194,   189,   191,
     187,   193,   195,   180,   181,   211,   182,   183,   196,   203,
     204,   205,   206,   207,   197,   198,   199,   200,   201,   202,
     141,   142,   144,   143,   145,   147,   148,   146,   210,   155,
     661,     0,   663,     0,   113,   112,     0,   126,   131,   162,
     161,   159,   163,     0,   156,   158,   164,   136,   221,   160,
     563,     0,   658,   660,     0,     0,   165,   166,   560,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     570,     0,     0,     0,    99,     0,    94,     0,   108,     0,
     122,   114,   124,     0,   125,     0,    97,   132,   102,   106,
       0,   157,   137,     0,   214,   220,     1,   659,     0,     0,
       0,    96,     0,     0,     0,   670,     0,   728,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   668,     0,   666,     0,     0,   568,   152,
     154,     0,   150,   212,     0,     0,   100,     0,     0,   664,
     109,     0,   115,   121,   123,   118,   127,   117,     0,   133,
     104,     0,   103,     0,     0,     9,     0,    43,    42,    44,
      41,     5,     6,     7,     8,     2,    16,    14,    15,    17,
      10,    11,    12,    13,     3,    18,    37,    20,    25,    26,
       0,     0,    30,     0,   224,     0,    36,   223,     0,   215,
     110,     0,    95,     0,     0,   726,     0,   678,     0,     0,
       0,     0,     0,   695,     0,     0,     0,     0,     0,     0,
       0,   720,     0,   693,     0,     0,     0,     0,    98,     0,
       0,     0,   573,     0,   572,   579,     0,     0,   149,     0,
     209,     0,   216,    45,    49,    52,    55,    60,    63,    65,
      67,    69,    71,    73,    75,     0,    34,     0,   101,   606,
     615,   619,     0,     0,     0,   640,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    45,    78,    91,
       0,   593,     0,   164,   136,   596,   617,   595,   603,   594,
       0,   597,   598,   621,   599,   628,   600,   601,   636,   602,
       0,     0,     0,   119,     0,   128,     0,   587,   135,     0,
       0,   107,    38,    39,     0,    22,    23,     0,     0,    28,
      27,     0,   226,    31,    33,    40,     0,   222,   111,   730,
       0,   731,   671,     0,     0,   729,   690,   686,   687,   688,
     689,     0,   684,     0,    93,   691,     0,     0,   705,   706,
     707,   708,     0,   703,     0,   712,   713,   714,   715,   711,
       0,   709,     0,   716,     0,     0,     0,     2,   724,   221,
       0,   722,     0,     0,   665,   667,   577,     0,   585,     0,
     583,   571,     0,   574,     0,   575,   580,     0,   153,   151,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      76,   217,   218,     0,   605,     0,   638,   651,   650,     0,
     642,     0,   654,   652,     0,     0,     0,   635,   655,   656,
     657,   604,    81,    82,    84,    83,    86,    87,    88,    89,
      90,    85,    80,     0,     0,   620,   616,   618,   622,   629,
     637,   116,   120,   130,     0,   590,   591,     0,   134,     0,
       4,     0,    24,    21,    32,   225,   674,   676,     0,     0,
     727,     0,   680,     0,   679,     0,   682,     0,     0,   697,
       0,   696,     0,   699,     0,     0,   701,     0,     0,   721,
       0,   718,     0,   694,   669,     0,     0,   586,     0,   581,
     576,   569,    46,    47,    48,    51,    50,    53,    54,    58,
      59,    56,    57,    61,    62,    64,    66,    68,    70,    72,
      74,     0,   219,   607,     0,     0,     0,     0,   653,     0,
     634,    79,    92,   129,   588,     0,   105,    19,   672,     0,
     673,     0,   685,     0,   692,     0,   704,     0,   710,     0,
     717,     0,     0,   723,     0,   582,   584,     0,     0,   626,
       0,     0,     0,   645,   644,   647,   613,   630,   589,   592,
     675,   677,   681,   683,   698,   700,   702,   719,   578,     0,
     608,     0,     0,     0,   646,     0,     0,   625,     0,     0,
     623,     0,    77,     0,   610,   639,   609,     0,   648,     0,
     613,   612,   614,   632,   627,     0,   649,   643,   624,   633,
       0,   641,   631
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -832,  -567,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,
    -832,  -832,  -455,  -832,  -413,  -408,  -526,  -419,  -284,  -283,
    -285,  -281,  -280,  -286,  -832,  -505,  -832,  -521,  -832,  -516,
    -558,     4,  -832,  -832,  -832,     5,  -418,  -832,  -832,    31,
      42,    37,  -832,  -832,  -433,  -832,  -832,  -832,  -832,  -112,
    -832,  -412,  -415,  -832,     9,  -832,     0,  -453,  -832,  -832,
    -832,  -577,   139,  -832,  -832,  -832,  -107,  -101,  -832,  -832,
    -571,  -593,  -579,  -364,  -636,  -832,  -391,  -647,  -831,  -832,
    -452,  -832,  -832,  -461,  -460,  -832,  -832,    56,  -768,  -387,
    -832,  -156,  -832,  -424,  -832,  -153,  -832,  -832,  -832,  -832,
    -149,  -832,  -832,  -832,  -832,  -832,  -832,  -832,  -832,    84,
    -832,  -832,     2,  -832,   -78,  -298,  -491,  -832,  -832,  -832,
    -325,  -321,  -324,  -832,  -832,  -326,  -319,  -330,  -318,  -328,
    -832,  -332,  -333,  -832,  -417,  -564
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   554,   555,   556,   831,   557,   558,   559,   560,   561,
     562,   563,   647,   565,   614,   615,   616,   617,   618,   619,
     620,   621,   622,   623,   624,   648,   891,   649,   813,   650,
     738,   651,   411,   680,   470,   652,   413,   414,   415,   460,
     461,   462,   416,   417,   418,   419,   420,   421,   511,   512,
     422,   423,   424,   425,   566,   514,   626,   517,   474,   475,
     568,   428,   429,   430,   606,   507,   601,   602,   603,   865,
     604,   605,   749,   750,   678,   827,   655,   656,   657,   658,
     659,   785,   928,   965,   957,   958,   959,   966,   660,   661,
     662,   663,   960,   931,   664,   665,   961,   980,   666,   667,
     668,   894,   789,   896,   935,   955,   956,   669,   431,   432,
     433,   457,   670,   504,   505,   484,   485,   838,   839,   435,
     711,   712,   716,   436,   437,   722,   723,   730,   731,   734,
     438,   740,   741,   439,   486,   487
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     427,   479,   434,   463,   410,   412,   479,   677,   471,   426,
     625,   756,   705,   817,   529,   739,   478,   893,   564,   729,
     684,   569,   480,   715,   754,   768,   769,   480,   705,   500,
     836,   753,   454,   706,   779,   509,   822,   743,   823,   450,
     826,   693,   764,   828,   765,   696,  -725,   463,   471,   758,
     699,   699,  -725,   700,   440,   685,   686,   697,   455,   510,
     613,   770,   771,   473,   837,   471,   707,   708,   709,   710,
     780,   451,   673,   675,   515,   -35,   714,   687,   682,   683,
     627,   688,   516,   702,   701,   701,   515,   714,   628,   702,
     714,   702,   441,   672,   702,   599,   702,   515,   702,   702,
     627,   627,   714,   702,   674,   772,   773,   679,   824,   829,
     695,   802,   803,   804,   805,   806,   807,   808,   809,   810,
     811,   964,   783,   794,   575,   796,   613,   527,   972,   515,
     576,   812,   448,   577,   456,   583,   528,   613,   964,   578,
     613,   584,   895,   490,   492,   494,   496,   498,   499,   502,
     591,   677,   613,   677,   442,   677,   592,   703,   677,   608,
     756,   610,   596,   690,   443,   609,   597,   611,   866,   691,
     834,   832,   613,   736,   830,   840,   842,   844,   458,   846,
     814,   576,   843,   845,   471,   847,   459,   849,   903,   851,
     752,   853,   599,   850,   599,   852,   856,   854,   859,   467,
     861,   936,   857,   937,   860,   979,   862,   814,   942,   814,
     943,   944,   945,   946,   843,   947,   847,   850,   854,   857,
     975,   862,   488,   908,   473,   489,   814,   909,   725,   726,
     727,   728,   545,   444,   910,   445,   756,   479,   911,   766,
     767,   835,   879,   880,   881,   882,   817,   718,   719,   720,
     721,   465,   478,   904,   466,   905,   491,   493,   480,   489,
     489,   495,   497,   501,   489,   489,   489,   531,   752,   939,
     532,   713,   446,   717,   489,   897,   489,   814,   724,   899,
     815,   489,   447,   739,   452,   739,   453,   729,   729,   914,
     715,   481,   901,   902,   924,   867,   705,   732,   735,   742,
     489,   489,   489,   677,   814,   900,   872,   873,   874,   613,
     613,   613,   613,   613,   613,   613,   613,   613,   613,   613,
     613,   613,   613,   613,   613,   868,   868,   974,   869,   870,
     814,   756,   817,   898,   483,   868,   503,   471,   925,   814,
     949,   752,   714,   714,   508,   353,   354,   355,   761,   762,
     763,   875,   876,   883,   884,   714,   518,   714,   877,   878,
     513,   350,   525,   526,   530,   515,   570,   571,   572,   574,
     573,   681,   593,   579,   580,   927,   581,   582,   929,   585,
     586,   704,   587,   748,   677,   595,   607,   588,   589,   590,
     689,   594,   613,   613,   671,   694,   598,   -34,   627,   527,
     700,   733,   760,   744,   777,   613,   751,   613,   778,   774,
     781,   775,   776,   784,   786,   464,   787,   752,   821,   929,
     788,   790,   791,   472,   426,   792,   795,   797,   962,   -36,
     833,   427,   426,   434,   427,   410,   412,   967,   798,   427,
     426,   434,   677,   426,   482,   799,   800,   801,   426,   506,
     -29,   841,   976,   752,   848,   855,   863,   858,   864,   464,
     520,   930,   952,   464,   871,   892,   907,   814,   426,   920,
     940,   932,   426,   567,   941,   951,   953,   950,  -611,   968,
     472,   969,   480,   629,   963,   973,   970,   981,   982,   426,
     885,   887,   886,   890,   523,   449,   888,   759,   889,   757,
     524,   522,   930,   755,   926,   933,   971,   600,   977,   934,
     978,   954,   752,   519,   818,   477,   426,   819,   654,   745,
     913,   820,   912,   480,   915,   919,   917,   653,   922,   923,
     921,   916,     0,     0,     0,     0,   918,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   698,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   747,
       0,     0,   600,     0,   600,     0,   600,     0,   426,     0,
       0,   426,     0,   426,     0,   426,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     654,     0,     0,     0,     0,     0,     0,     0,     0,   653,
     427,     0,     0,     0,     0,     0,     0,     0,     0,   426,
     600,     0,     0,     0,     0,     0,     0,     0,     0,   426,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   747,   600,     0,     0,     0,     0,     0,     0,
       0,   426,   426,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   654,     0,     0,     0,   654,
       0,     0,     0,     0,   653,     0,     0,     0,   653,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   600,
       0,     0,     0,     0,     0,     0,     0,     0,   426,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   600,     0,     0,     0,     0,
       0,     0,     0,     0,   426,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   654,   654,     0,   654,     0,   434,     0,
       0,     0,   653,   653,     0,   653,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   600,     0,     0,     0,     0,     0,
       0,     0,     0,   426,     0,   654,     0,     0,     0,     0,
       0,     0,     0,     0,   653,     0,     0,     0,     0,     0,
       0,     0,   654,     0,     0,     0,     0,     0,     0,   654,
       0,   653,     0,     0,     0,     0,     0,     0,   653,   654,
       0,     0,     0,   654,     0,     0,     0,     0,   653,   654,
       0,     0,   653,     0,     0,     0,   476,     0,   653,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   350,     0,     0,     0,     0,     0,     0,     0,
     351,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   352,   353,   354,   355,   356,     0,
       0,     0,     0,     0,     0,     0,     0,   357,   358,   359,
     360,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   364,
     365,   366,   367,   368,   369,   370,     0,     0,     0,     0,
       0,     0,     0,     0,   371,     0,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,     0,     0,   533,   534,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   535,   536,     0,   350,     0,   629,
     630,     0,     0,     0,     0,   631,   537,   538,   539,   540,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   352,
     353,   354,   355,   356,     0,     0,     0,   541,   542,   543,
     544,   545,   357,   358,   359,   360,   361,   362,   363,   632,
     633,   634,   635,     0,   636,   637,   638,   639,   640,   641,
     642,   643,   644,   645,   364,   365,   366,   367,   368,   369,
     370,   546,   547,   548,   549,   550,   551,   552,   553,   371,
     646,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,     0,     0,
     533,   534,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   535,
     536,     0,   350,     0,   629,   816,     0,     0,     0,     0,
     631,   537,   538,   539,   540,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   352,   353,   354,   355,   356,     0,
       0,     0,   541,   542,   543,   544,   545,   357,   358,   359,
     360,   361,   362,   363,   632,   633,   634,   635,     0,   636,
     637,   638,   639,   640,   641,   642,   643,   644,   645,   364,
     365,   366,   367,   368,   369,   370,   546,   547,   548,   549,
     550,   551,   552,   553,   371,   646,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,     0,     0,   533,   534,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   535,   536,     0,   350,     0,   629,
       0,     0,     0,     0,     0,   631,   537,   538,   539,   540,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   352,
     353,   354,   355,   356,     0,     0,     0,   541,   542,   543,
     544,   545,   357,   358,   359,   360,   361,   362,   363,   632,
     633,   634,   635,     0,   636,   637,   638,   639,   640,   641,
     642,   643,   644,   645,   364,   365,   366,   367,   368,   369,
     370,   546,   547,   548,   549,   550,   551,   552,   553,   371,
     646,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,     0,     0,
     533,   534,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   535,
     536,     0,   350,     0,   518,     0,     0,     0,     0,     0,
     631,   537,   538,   539,   540,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   352,   353,   354,   355,   356,     0,
       0,     0,   541,   542,   543,   544,   545,   357,   358,   359,
     360,   361,   362,   363,   632,   633,   634,   635,     0,   636,
     637,   638,   639,   640,   641,   642,   643,   644,   645,   364,
     365,   366,   367,   368,   369,   370,   546,   547,   548,   549,
     550,   551,   552,   553,   371,   646,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,     0,     0,   533,   534,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   535,   536,     0,   350,     0,     0,
       0,     0,     0,     0,     0,   631,   537,   538,   539,   540,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   352,
     353,   354,   355,   356,     0,     0,     0,   541,   542,   543,
     544,   545,   357,   358,   359,   360,   361,   362,   363,   632,
     633,   634,   635,     0,   636,   637,   638,   639,   640,   641,
     642,   643,   644,   645,   364,   365,   366,   367,   368,   369,
     370,   546,   547,   548,   549,   550,   551,   552,   553,   371,
     646,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,     0,     0,
     533,   534,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   535,
     536,     0,   350,     0,     0,     0,     0,     0,     0,     0,
     631,   537,   538,   539,   540,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   352,   353,   354,   355,   356,     0,
       0,     0,   541,   542,   543,   544,   545,   357,   358,   359,
     360,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   364,
     365,   366,   367,   368,   369,   370,   546,   547,   548,   549,
     550,   551,   552,   553,   371,     0,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,     0,     0,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,     0,     0,   533,   534,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   535,   536,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   537,   538,   539,   540,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   352,
     353,   354,   355,     0,     0,     0,     0,   541,   542,   543,
     544,   545,   357,   358,   359,   360,   361,   362,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   364,   365,   366,   367,   368,   369,
     370,   546,   547,   548,   549,   550,   551,   552,   553,   371,
       0,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   350,     0,     0,     0,     0,     0,     0,     0,
     351,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   352,   353,   354,   355,   356,     0,
       0,     0,     0,     0,     0,     0,     0,   357,   358,   359,
     360,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   364,
     365,   366,   367,   368,   369,   370,     0,     0,     0,     0,
       0,     0,     0,     0,   371,     0,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,     0,     0,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   352,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   357,   358,   359,   360,   361,   362,   363,   632,
       0,     0,   635,     0,   636,   637,     0,     0,   640,     0,
       0,     0,     0,     0,   364,   365,   366,   367,   368,   369,
     370,     0,     0,     0,     0,     0,     0,     0,     0,   371,
       0,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,     0,     0,     0,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     468,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   352,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,   469,   357,   358,   359,
     360,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   364,
     365,   366,   367,   368,   369,   370,     0,     0,     0,     0,
       0,     0,     0,     0,   371,     0,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,     0,     0,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   350,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   352,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   357,   358,   359,   360,   361,   362,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   364,   365,   366,   367,   368,   369,
     370,     0,     0,     0,     0,     0,     0,     0,     0,   371,
       0,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,     0,     0,     0,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   521,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   352,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   357,   358,   359,
     360,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   364,
     365,   366,   367,   368,   369,   370,     0,     0,     0,     0,
       0,     0,     0,     0,   371,     0,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,     0,     0,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   746,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   352,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   357,   358,   359,   360,   361,   362,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   364,   365,   366,   367,   368,   369,
     370,     0,     0,     0,     0,     0,     0,     0,     0,   371,
       0,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,     0,     0,     0,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   906,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   352,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   357,   358,   359,
     360,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   364,
     365,   366,   367,   368,   369,   370,     0,     0,     0,     0,
       0,     0,     0,     0,   371,     0,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,     0,     0,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     948,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   352,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   357,   358,   359,   360,   361,   362,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   364,   365,   366,   367,   368,   369,
     370,     0,     0,     0,     0,     0,     0,     0,     0,   371,
       0,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,     0,     0,     0,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   352,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   357,   358,   359,
     360,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   364,
     365,   366,   367,   368,   369,   370,     0,     0,     0,     0,
       0,     0,     0,     0,   371,     0,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     0,   181,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,     0,     0,   343,     0,     0,     0,   347,
     348,   349,     0,     0,   533,   534,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   535,   536,     0,     0,     0,   676,   825,
       0,     0,     0,     0,     0,   537,   538,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,   542,   543,   544,
     545,   357,     0,     0,     0,     0,   362,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     546,   547,   548,   549,   550,   551,   552,   553,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     0,   181,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,     0,     0,   343,     0,     0,     0,   347,
     348,   349,     0,     0,   533,   534,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   535,   536,     0,     0,     0,   676,   938,
       0,     0,     0,     0,     0,   537,   538,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,   542,   543,   544,
     545,   357,     0,     0,     0,     0,   362,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     546,   547,   548,   549,   550,   551,   552,   553,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     0,   181,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,     0,     0,   343,     0,     0,     0,   347,
     348,   349,     0,     0,   533,   534,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   535,   536,     0,     0,   612,     0,     0,
       0,     0,     0,     0,     0,   537,   538,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,   542,   543,   544,
     545,   357,     0,     0,     0,     0,   362,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     546,   547,   548,   549,   550,   551,   552,   553,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     0,   181,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,     0,     0,   343,     0,     0,     0,   347,
     348,   349,     0,     0,   533,   534,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   535,   536,     0,     0,     0,   676,     0,
       0,     0,     0,     0,     0,   537,   538,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,   542,   543,   544,
     545,   357,     0,     0,     0,     0,   362,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     546,   547,   548,   549,   550,   551,   552,   553,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     0,   181,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,     0,     0,   343,     0,     0,     0,   347,
     348,   349,     0,     0,   533,   534,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   535,   536,     0,     0,   782,     0,     0,
       0,     0,     0,     0,     0,   537,   538,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,   542,   543,   544,
     545,   357,     0,     0,     0,     0,   362,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     546,   547,   548,   549,   550,   551,   552,   553,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     0,   181,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,     0,     0,   343,     0,     0,     0,   347,
     348,   349,     0,     0,   533,   534,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   535,   536,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   793,   537,   538,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,   542,   543,   544,
     545,   357,     0,     0,     0,     0,   362,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     546,   547,   548,   549,   550,   551,   552,   553,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     0,   181,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,     0,     0,   343,     0,     0,     0,   347,
     348,   349,     0,     0,   533,   534,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   535,   536,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   537,   538,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,   542,   543,   544,
     545,   357,     0,     0,     0,     0,   362,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     546,   547,   548,   549,   550,   551,   552,   553,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     0,   181,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,     0,     0,   343,     0,     0,     0,   347,
     348,   349,     0,     0,   533,   534,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   535,   536,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   537,   538,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,   542,   543,   544,
     545,   357,     0,     0,     0,     0,   362,   692,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     546,   547,   548,   549,   550,   551,   552,   553,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     0,   181,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,     0,     0,   343,     0,     0,     0,   347,
     348,   349,     0,     0,   533,   534,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   535,   536,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   537,   538,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,   542,   543,   544,
     737,   357,     0,     0,     0,     0,   362,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     546,   547,   548,   549,   550,   551,   552,   553,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   385,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,     0,   181,     0,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,     0,     0,   343,     0,     0,     0,   347,
     348,   349,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   357,     0,     0,     0,     0,   362,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     372
};

static const yytype_int16 yycheck[] =
{
       0,   434,     0,   415,     0,     0,   439,   528,   423,     0,
     515,   604,   576,   660,   467,   592,   434,   785,   473,   586,
     536,   474,   434,   581,   603,   356,   357,   439,   592,   446,
     373,   602,   384,   373,   361,   410,   672,   595,   674,   378,
     676,   562,   386,   679,   388,   381,   375,   459,   463,   607,
     373,   373,   381,   376,   374,   354,   355,   393,   410,   434,
     515,   392,   393,   392,   407,   480,   406,   407,   408,   409,
     397,   410,   525,   526,   376,   374,   581,   376,   533,   534,
     376,   380,   384,   574,   407,   407,   376,   592,   384,   580,
     595,   582,   374,   383,   585,   507,   587,   376,   589,   590,
     376,   376,   607,   594,   383,   358,   359,   383,   383,   680,
     565,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   952,   627,   639,   375,   641,   581,   374,   959,   376,
     381,   383,   376,   375,   384,   375,   383,   592,   969,   381,
     595,   381,   789,   441,   442,   443,   444,   445,   446,   447,
     375,   672,   607,   674,   374,   676,   381,   574,   679,   375,
     753,   375,   377,   375,   374,   381,   381,   381,   747,   381,
     691,   687,   627,   590,   375,   375,   375,   375,   375,   375,
     381,   381,   381,   381,   599,   381,   381,   375,   824,   375,
     602,   375,   604,   381,   606,   381,   375,   381,   375,   410,
     375,   375,   381,   375,   381,   973,   381,   381,   375,   381,
     375,   375,   375,   375,   381,   375,   381,   381,   381,   381,
     375,   381,   407,   377,   392,   410,   381,   381,   406,   407,
     408,   409,   410,   374,   377,   374,   829,   670,   381,   352,
     353,   696,   768,   769,   770,   771,   893,   406,   407,   408,
     409,   381,   670,   379,   384,   381,   407,   407,   670,   410,
     410,   407,   407,   407,   410,   410,   410,   381,   680,   905,
     384,   407,   374,   407,   410,   791,   410,   381,   407,   795,
     384,   410,   374,   860,   374,   862,   374,   854,   855,   847,
     848,   384,   813,   814,   865,   748,   860,   407,   407,   407,
     410,   410,   410,   824,   381,   382,   761,   762,   763,   764,
     765,   766,   767,   768,   769,   770,   771,   772,   773,   774,
     775,   776,   777,   778,   779,   381,   381,   963,   384,   384,
     381,   924,   979,   384,   410,   381,   410,   752,   384,   381,
     382,   753,   847,   848,   378,   399,   400,   401,   389,   390,
     391,   764,   765,   772,   773,   860,   378,   862,   766,   767,
     410,   376,   410,   410,   378,   376,   375,   410,   384,   381,
     383,   410,   375,   383,   381,   891,   381,   381,   894,   381,
     381,   410,   381,   410,   905,   374,   383,   381,   381,   381,
     375,   381,   847,   848,   380,   374,   384,   374,   376,   374,
     376,   373,   410,   377,   360,   860,   379,   862,   362,   396,
     377,   395,   394,   379,   374,   415,   384,   829,   380,   935,
     384,   374,   374,   423,   415,   384,   374,   382,   949,   374,
     410,   431,   423,   431,   434,   431,   431,   953,   384,   439,
     431,   439,   963,   434,   439,   384,   384,   384,   439,   449,
     375,   383,   968,   865,   381,   381,   375,   381,   375,   459,
     458,   894,   375,   463,   379,   377,   377,   381,   459,   373,
     373,   418,   463,   473,   407,   410,   374,   379,   378,   384,
     480,   375,   894,   378,   383,   378,   422,   384,   379,   480,
     774,   776,   775,   779,   463,   356,   777,   609,   778,   606,
     463,   459,   935,   604,   868,   896,   958,   507,   969,   896,
     970,   935,   924,   457,   670,   431,   507,   670,   518,   597,
     845,   670,   843,   935,   848,   855,   852,   518,   860,   862,
     858,   850,    -1,    -1,    -1,    -1,   854,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   570,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   599,
      -1,    -1,   602,    -1,   604,    -1,   606,    -1,   599,    -1,
      -1,   602,    -1,   604,    -1,   606,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     660,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   660,
     670,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   670,
     680,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   680,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   752,   753,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   752,   753,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   785,    -1,    -1,    -1,   789,
      -1,    -1,    -1,    -1,   785,    -1,    -1,    -1,   789,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   829,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   829,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   865,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   865,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   893,   894,    -1,   896,    -1,   896,    -1,
      -1,    -1,   893,   894,    -1,   896,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   924,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   924,    -1,   935,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   935,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   952,    -1,    -1,    -1,    -1,    -1,    -1,   959,
      -1,   952,    -1,    -1,    -1,    -1,    -1,    -1,   959,   969,
      -1,    -1,    -1,   973,    -1,    -1,    -1,    -1,   969,   979,
      -1,    -1,   973,    -1,    -1,    -1,     0,    -1,   979,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   376,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     384,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   398,   399,   400,   401,   402,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   411,   412,   413,
     414,   415,   416,   417,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   433,
     434,   435,   436,   437,   438,   439,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   448,    -1,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   373,   374,    -1,   376,    -1,   378,
     379,    -1,    -1,    -1,    -1,   384,   385,   386,   387,   388,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   398,
     399,   400,   401,   402,    -1,    -1,    -1,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   417,   418,
     419,   420,   421,    -1,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,    -1,    -1,
     354,   355,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   373,
     374,    -1,   376,    -1,   378,   379,    -1,    -1,    -1,    -1,
     384,   385,   386,   387,   388,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   398,   399,   400,   401,   402,    -1,
      -1,    -1,   406,   407,   408,   409,   410,   411,   412,   413,
     414,   415,   416,   417,   418,   419,   420,   421,    -1,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   373,   374,    -1,   376,    -1,   378,
      -1,    -1,    -1,    -1,    -1,   384,   385,   386,   387,   388,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   398,
     399,   400,   401,   402,    -1,    -1,    -1,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   417,   418,
     419,   420,   421,    -1,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,    -1,    -1,
     354,   355,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   373,
     374,    -1,   376,    -1,   378,    -1,    -1,    -1,    -1,    -1,
     384,   385,   386,   387,   388,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   398,   399,   400,   401,   402,    -1,
      -1,    -1,   406,   407,   408,   409,   410,   411,   412,   413,
     414,   415,   416,   417,   418,   419,   420,   421,    -1,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   373,   374,    -1,   376,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   384,   385,   386,   387,   388,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   398,
     399,   400,   401,   402,    -1,    -1,    -1,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   417,   418,
     419,   420,   421,    -1,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,    -1,    -1,
     354,   355,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   373,
     374,    -1,   376,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     384,   385,   386,   387,   388,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   398,   399,   400,   401,   402,    -1,
      -1,    -1,   406,   407,   408,   409,   410,   411,   412,   413,
     414,   415,   416,   417,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,    -1,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
      -1,    -1,    -1,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   373,   374,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   385,   386,   387,   388,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   398,
     399,   400,   401,    -1,    -1,    -1,    -1,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   417,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
      -1,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   376,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     384,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   398,   399,   400,   401,   402,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   411,   412,   413,
     414,   415,   416,   417,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   433,
     434,   435,   436,   437,   438,   439,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   448,    -1,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
      -1,    -1,    -1,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   398,
     399,   400,   401,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   411,   412,   413,   414,   415,   416,   417,   418,
      -1,    -1,   421,    -1,   423,   424,    -1,    -1,   427,    -1,
      -1,    -1,    -1,    -1,   433,   434,   435,   436,   437,   438,
     439,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   448,
      -1,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,    -1,    -1,    -1,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     384,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   398,   399,   400,   401,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   410,   411,   412,   413,
     414,   415,   416,   417,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   433,
     434,   435,   436,   437,   438,   439,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   448,    -1,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
      -1,    -1,    -1,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   376,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   398,
     399,   400,   401,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   411,   412,   413,   414,   415,   416,   417,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   433,   434,   435,   436,   437,   438,
     439,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   448,
      -1,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,    -1,    -1,    -1,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   380,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   398,   399,   400,   401,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   411,   412,   413,
     414,   415,   416,   417,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   433,
     434,   435,   436,   437,   438,   439,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   448,    -1,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
      -1,    -1,    -1,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   378,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   398,
     399,   400,   401,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   411,   412,   413,   414,   415,   416,   417,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   433,   434,   435,   436,   437,   438,
     439,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   448,
      -1,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,    -1,    -1,    -1,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   379,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   398,   399,   400,   401,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   411,   412,   413,
     414,   415,   416,   417,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   433,
     434,   435,   436,   437,   438,   439,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   448,    -1,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
      -1,    -1,    -1,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     379,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   398,
     399,   400,   401,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   411,   412,   413,   414,   415,   416,   417,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   433,   434,   435,   436,   437,   438,
     439,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   448,
      -1,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,    -1,    -1,    -1,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   398,   399,   400,   401,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   411,   412,   413,
     414,   415,   416,   417,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   433,
     434,   435,   436,   437,   438,   439,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   448,    -1,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,    -1,
      -1,    -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,   349,
     350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,   374,    -1,    -1,    -1,   378,   379,
      -1,    -1,    -1,    -1,    -1,   385,   386,   387,   388,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,   407,   408,   409,
     410,   411,    -1,    -1,    -1,    -1,   416,   417,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,   442,   443,   444,   445,   446,   447,    -1,    -1,
     450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   463,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,    -1,
      -1,    -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,   349,
     350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,   374,    -1,    -1,    -1,   378,   379,
      -1,    -1,    -1,    -1,    -1,   385,   386,   387,   388,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,   407,   408,   409,
     410,   411,    -1,    -1,    -1,    -1,   416,   417,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,   442,   443,   444,   445,   446,   447,    -1,    -1,
     450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   463,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,    -1,
      -1,    -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,   349,
     350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,   374,    -1,    -1,   377,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   385,   386,   387,   388,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,   407,   408,   409,
     410,   411,    -1,    -1,    -1,    -1,   416,   417,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,   442,   443,   444,   445,   446,   447,    -1,    -1,
     450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   463,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,    -1,
      -1,    -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,   349,
     350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,   374,    -1,    -1,    -1,   378,    -1,
      -1,    -1,    -1,    -1,    -1,   385,   386,   387,   388,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,   407,   408,   409,
     410,   411,    -1,    -1,    -1,    -1,   416,   417,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,   442,   443,   444,   445,   446,   447,    -1,    -1,
     450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   463,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,    -1,
      -1,    -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,   349,
     350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,   374,    -1,    -1,   377,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   385,   386,   387,   388,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,   407,   408,   409,
     410,   411,    -1,    -1,    -1,    -1,   416,   417,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,   442,   443,   444,   445,   446,   447,    -1,    -1,
     450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   463,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,    -1,
      -1,    -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,   349,
     350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,   374,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   384,   385,   386,   387,   388,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,   407,   408,   409,
     410,   411,    -1,    -1,    -1,    -1,   416,   417,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,   442,   443,   444,   445,   446,   447,    -1,    -1,
     450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   463,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,    -1,
      -1,    -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,   349,
     350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,   374,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   385,   386,   387,   388,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,   407,   408,   409,
     410,   411,    -1,    -1,    -1,    -1,   416,   417,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,   442,   443,   444,   445,   446,   447,    -1,    -1,
     450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   463,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,    -1,
      -1,    -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,   349,
     350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,   374,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   385,   386,   387,   388,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,   407,   408,   409,
     410,   411,    -1,    -1,    -1,    -1,   416,   417,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,   442,   443,   444,   445,   446,   447,    -1,    -1,
     450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   463,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,    -1,
      -1,    -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,   349,
     350,   351,    -1,    -1,   354,   355,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,   374,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   385,   386,   387,   388,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   406,   407,   408,   409,
     410,   411,    -1,    -1,    -1,    -1,   416,   417,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,   442,   443,   444,   445,   446,   447,    -1,    -1,
     450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   463,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    -1,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,    -1,
      -1,    -1,    -1,    -1,    -1,   345,    -1,    -1,    -1,   349,
     350,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   411,    -1,    -1,    -1,    -1,   416,   417,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     450
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     376,   384,   398,   399,   400,   401,   402,   411,   412,   413,
     414,   415,   416,   417,   433,   434,   435,   436,   437,   438,
     439,   448,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     519,   520,   523,   524,   525,   526,   530,   531,   532,   533,
     534,   535,   538,   539,   540,   541,   542,   544,   549,   550,
     551,   596,   597,   598,   600,   607,   611,   612,   618,   621,
     374,   374,   374,   374,   374,   374,   374,   374,   376,   550,
     378,   410,   374,   374,   384,   410,   384,   599,   375,   381,
     527,   528,   529,   539,   544,   381,   384,   410,   384,   410,
     522,   540,   544,   392,   546,   547,     0,   597,   524,   532,
     539,   384,   523,   410,   603,   604,   622,   623,   407,   410,
     603,   407,   603,   407,   603,   407,   603,   407,   603,   603,
     622,   407,   603,   410,   601,   602,   544,   553,   378,   410,
     434,   536,   537,   410,   543,   376,   384,   545,   378,   575,
     600,   380,   528,   527,   529,   410,   410,   374,   383,   545,
     378,   381,   384,   354,   355,   373,   374,   385,   386,   387,
     388,   406,   407,   408,   409,   410,   440,   441,   442,   443,
     444,   445,   446,   447,   489,   490,   491,   493,   494,   495,
     496,   497,   498,   499,   500,   501,   542,   544,   548,   545,
     375,   410,   384,   383,   381,   375,   381,   375,   381,   383,
     381,   381,   381,   375,   381,   381,   381,   381,   381,   381,
     381,   375,   381,   375,   381,   374,   377,   381,   384,   539,
     544,   554,   555,   556,   558,   559,   552,   383,   375,   381,
     375,   381,   377,   500,   502,   503,   504,   505,   506,   507,
     508,   509,   510,   511,   512,   513,   544,   376,   384,   378,
     379,   384,   418,   419,   420,   421,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   449,   500,   513,   515,
     517,   519,   523,   542,   544,   564,   565,   566,   567,   568,
     576,   577,   578,   579,   582,   583,   586,   587,   588,   595,
     600,   380,   383,   545,   383,   545,   378,   515,   562,   383,
     521,   410,   500,   500,   517,   354,   355,   376,   380,   375,
     375,   381,   417,   515,   374,   500,   381,   393,   600,   373,
     376,   407,   604,   622,   410,   623,   373,   406,   407,   408,
     409,   608,   609,   407,   513,   518,   610,   407,   406,   407,
     408,   409,   613,   614,   407,   406,   407,   408,   409,   489,
     615,   616,   407,   373,   617,   407,   622,   410,   518,   549,
     619,   620,   407,   518,   377,   602,   378,   544,   410,   560,
     561,   379,   539,   558,   560,   555,   559,   554,   518,   537,
     410,   389,   390,   391,   386,   388,   352,   353,   356,   357,
     392,   393,   358,   359,   396,   395,   394,   360,   362,   361,
     397,   377,   377,   513,   379,   569,   374,   384,   384,   590,
     374,   374,   384,   384,   517,   374,   517,   382,   384,   384,
     384,   384,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   383,   516,   381,   384,   379,   565,   579,   583,
     588,   380,   562,   562,   383,   379,   562,   563,   562,   558,
     375,   492,   517,   410,   515,   500,   373,   407,   605,   606,
     375,   383,   375,   381,   375,   381,   375,   381,   381,   375,
     381,   375,   381,   375,   381,   381,   375,   381,   381,   375,
     381,   375,   381,   375,   375,   557,   560,   545,   381,   384,
     384,   379,   500,   500,   500,   502,   502,   503,   503,   504,
     504,   504,   504,   505,   505,   506,   507,   508,   509,   510,
     511,   514,   377,   576,   589,   565,   591,   517,   384,   517,
     382,   515,   515,   562,   379,   381,   379,   377,   377,   381,
     377,   381,   609,   608,   518,   610,   614,   613,   616,   615,
     373,   617,   619,   620,   558,   384,   561,   517,   570,   517,
     532,   581,   418,   564,   577,   592,   375,   375,   379,   562,
     373,   407,   375,   375,   375,   375,   375,   375,   379,   382,
     379,   410,   375,   374,   581,   593,   594,   572,   573,   574,
     580,   584,   515,   383,   566,   571,   575,   517,   384,   375,
     422,   568,   566,   378,   562,   375,   517,   571,   572,   576,
     585,   384,   379
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   488,   489,   490,   490,   490,   490,   490,   490,   490,
     490,   490,   490,   490,   490,   490,   490,   490,   491,   491,
     491,   491,   491,   491,   492,   493,   494,   495,   495,   496,
     496,   497,   497,   498,   499,   499,   499,   500,   500,   500,
     500,   501,   501,   501,   501,   502,   502,   502,   502,   503,
     503,   503,   504,   504,   504,   505,   505,   505,   505,   505,
     506,   506,   506,   507,   507,   508,   508,   509,   509,   510,
     510,   511,   511,   512,   512,   513,   514,   513,   515,   515,
     516,   516,   516,   516,   516,   516,   516,   516,   516,   516,
     516,   517,   517,   518,   519,   519,   519,   519,   519,   519,
     519,   519,   519,   519,   521,   520,   522,   522,   523,   523,
     523,   523,   524,   524,   525,   525,   525,   526,   527,   527,
     527,   528,   528,   528,   528,   529,   530,   530,   530,   530,
     530,   531,   531,   531,   531,   531,   532,   532,   533,   534,
     534,   534,   534,   534,   534,   534,   534,   534,   534,   535,
     536,   536,   537,   537,   537,   538,   539,   539,   540,   540,
     540,   540,   540,   540,   540,   540,   540,   540,   540,   541,
     541,   541,   541,   541,   541,   541,   541,   541,   541,   541,
     541,   541,   541,   541,   541,   541,   541,   541,   541,   541,
     541,   541,   541,   541,   541,   541,   541,   541,   541,   541,
     541,   541,   541,   541,   541,   541,   541,   541,   541,   541,
     541,   542,   543,   543,   544,   544,   545,   545,   545,   545,
     546,   546,   547,   548,   548,   548,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   549,   549,   549,   549,   549,
     549,   549,   549,   549,   549,   550,   550,   550,   552,   551,
     553,   551,   554,   554,   554,   554,   555,   557,   556,   558,
     558,   559,   559,   560,   560,   561,   561,   562,   562,   562,
     562,   563,   563,   564,   565,   565,   566,   566,   566,   566,
     566,   566,   566,   566,   567,   568,   569,   570,   568,   571,
     571,   573,   572,   574,   572,   575,   575,   576,   576,   577,
     577,   578,   578,   579,   580,   580,   581,   581,   582,   582,
     584,   583,   585,   585,   586,   586,   587,   587,   589,   588,
     590,   588,   591,   588,   592,   592,   593,   593,   594,   594,
     595,   595,   595,   595,   595,   595,   595,   595,   596,   596,
     597,   597,   597,   599,   598,   600,   601,   601,   602,   602,
     603,   603,   604,   604,   605,   605,   606,   606,   607,   607,
     607,   607,   607,   607,   608,   608,   609,   609,   609,   609,
     609,   610,   610,   611,   611,   612,   612,   612,   612,   612,
     612,   612,   612,   613,   613,   614,   614,   614,   614,   615,
     615,   616,   616,   616,   616,   616,   617,   617,   618,   618,
     618,   618,   619,   619,   620,   620,   621,   621,   622,   622,
     623,   623
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     3,     2,     2,     1,     1,     1,     2,     2,     2,
       1,     2,     3,     2,     1,     1,     1,     1,     2,     2,
       2,     1,     1,     1,     1,     1,     3,     3,     3,     1,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     3,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     0,     6,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     2,     3,     2,     2,     4,     2,
       3,     4,     2,     3,     0,     6,     1,     3,     2,     3,
       3,     4,     1,     1,     2,     3,     5,     3,     2,     3,
       4,     2,     1,     2,     1,     1,     1,     3,     4,     6,
       5,     1,     2,     3,     5,     4,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     3,     1,     3,     1,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     1,     1,     3,     2,     3,     2,     3,     3,     4,
       1,     0,     3,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     6,
       0,     5,     1,     1,     2,     2,     3,     0,     5,     1,
       2,     3,     4,     1,     3,     1,     2,     1,     3,     4,
       2,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     0,     0,     5,     1,
       1,     0,     2,     0,     2,     2,     3,     1,     2,     1,
       2,     1,     2,     5,     3,     1,     1,     4,     1,     2,
       0,     8,     0,     1,     3,     2,     1,     2,     0,     6,
       0,     8,     0,     7,     1,     1,     1,     0,     2,     3,
       2,     2,     2,     3,     2,     2,     2,     2,     1,     2,
       1,     1,     1,     0,     3,     5,     1,     3,     1,     4,
       1,     3,     5,     5,     1,     3,     1,     3,     4,     6,
       6,     8,     6,     8,     1,     3,     1,     1,     1,     1,
       1,     1,     3,     4,     6,     4,     6,     6,     8,     6,
       8,     6,     8,     1,     3,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     3,     6,     8,
       4,     6,     1,     3,     1,     1,     4,     6,     1,     3,
       3,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (pParseContext, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, pParseContext); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, glslang::TParseContext* pParseContext)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (pParseContext);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, glslang::TParseContext* pParseContext)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, pParseContext);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, glslang::TParseContext* pParseContext)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], pParseContext);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, pParseContext); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, glslang::TParseContext* pParseContext)
{
  YY_USE (yyvaluep);
  YY_USE (pParseContext);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (glslang::TParseContext* pParseContext)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, parseContext);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* variable_identifier: IDENTIFIER  */
#line 365 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.intermTypedNode) = parseContext.handleVariable((yyvsp[0].lex).loc, (yyvsp[0].lex).symbol, (yyvsp[0].lex).string);
    }
#line 5491 "MachineIndependent/glslang_tab.cpp"
    break;

  case 3: /* primary_expression: variable_identifier  */
#line 371 "MachineIndependent/glslang.y"
                          {
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 5499 "MachineIndependent/glslang_tab.cpp"
    break;

  case 4: /* primary_expression: LEFT_PAREN expression RIGHT_PAREN  */
#line 374 "MachineIndependent/glslang.y"
                                        {
        (yyval.interm.intermTypedNode) = (yyvsp[-1].interm.intermTypedNode);
        if ((yyval.interm.intermTypedNode)->getAsConstantUnion())
            (yyval.interm.intermTypedNode)->getAsConstantUnion()->setExpression();
    }
#line 5509 "MachineIndependent/glslang_tab.cpp"
    break;

  case 5: /* primary_expression: FLOATCONSTANT  */
#line 379 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).d, EbtFloat, (yyvsp[0].lex).loc, true);
    }
#line 5517 "MachineIndependent/glslang_tab.cpp"
    break;

  case 6: /* primary_expression: INTCONSTANT  */
#line 382 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).i, (yyvsp[0].lex).loc, true);
    }
#line 5525 "MachineIndependent/glslang_tab.cpp"
    break;

  case 7: /* primary_expression: UINTCONSTANT  */
#line 385 "MachineIndependent/glslang.y"
                   {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "unsigned literal");
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).u, (yyvsp[0].lex).loc, true);
    }
#line 5534 "MachineIndependent/glslang_tab.cpp"
    break;

  case 8: /* primary_expression: BOOLCONSTANT  */
#line 389 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).b, (yyvsp[0].lex).loc, true);
    }
#line 5542 "MachineIndependent/glslang_tab.cpp"
    break;

  case 9: /* primary_expression: STRING_LITERAL  */
#line 392 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).string, (yyvsp[0].lex).loc, true);
    }
#line 5550 "MachineIndependent/glslang_tab.cpp"
    break;

  case 10: /* primary_expression: INT32CONSTANT  */
#line 395 "MachineIndependent/glslang.y"
                    {
        parseContext.explicitInt32Check((yyvsp[0].lex).loc, "32-bit signed literal");
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).i, (yyvsp[0].lex).loc, true);
    }
#line 5559 "MachineIndependent/glslang_tab.cpp"
    break;

  case 11: /* primary_expression: UINT32CONSTANT  */
#line 399 "MachineIndependent/glslang.y"
                     {
        parseContext.explicitInt32Check((yyvsp[0].lex).loc, "32-bit signed literal");
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).u, (yyvsp[0].lex).loc, true);
    }
#line 5568 "MachineIndependent/glslang_tab.cpp"
    break;

  case 12: /* primary_expression: INT64CONSTANT  */
#line 403 "MachineIndependent/glslang.y"
                    {
        parseContext.int64Check((yyvsp[0].lex).loc, "64-bit integer literal");
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).i64, (yyvsp[0].lex).loc, true);
    }
#line 5577 "MachineIndependent/glslang_tab.cpp"
    break;

  case 13: /* primary_expression: UINT64CONSTANT  */
#line 407 "MachineIndependent/glslang.y"
                     {
        parseContext.int64Check((yyvsp[0].lex).loc, "64-bit unsigned integer literal");
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).u64, (yyvsp[0].lex).loc, true);
    }
#line 5586 "MachineIndependent/glslang_tab.cpp"
    break;

  case 14: /* primary_expression: INT16CONSTANT  */
#line 411 "MachineIndependent/glslang.y"
                    {
        parseContext.explicitInt16Check((yyvsp[0].lex).loc, "16-bit integer literal");
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((short)(yyvsp[0].lex).i, (yyvsp[0].lex).loc, true);
    }
#line 5595 "MachineIndependent/glslang_tab.cpp"
    break;

  case 15: /* primary_expression: UINT16CONSTANT  */
#line 415 "MachineIndependent/glslang.y"
                     {
        parseContext.explicitInt16Check((yyvsp[0].lex).loc, "16-bit unsigned integer literal");
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((unsigned short)(yyvsp[0].lex).u, (yyvsp[0].lex).loc, true);
    }
#line 5604 "MachineIndependent/glslang_tab.cpp"
    break;

  case 16: /* primary_expression: DOUBLECONSTANT  */
#line 419 "MachineIndependent/glslang.y"
                     {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double literal");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double literal");
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).d, EbtDouble, (yyvsp[0].lex).loc, true);
    }
#line 5615 "MachineIndependent/glslang_tab.cpp"
    break;

  case 17: /* primary_expression: FLOAT16CONSTANT  */
#line 425 "MachineIndependent/glslang.y"
                      {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float literal");
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).d, EbtFloat16, (yyvsp[0].lex).loc, true);
    }
#line 5624 "MachineIndependent/glslang_tab.cpp"
    break;

  case 18: /* postfix_expression: primary_expression  */
#line 432 "MachineIndependent/glslang.y"
                         {
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 5632 "MachineIndependent/glslang_tab.cpp"
    break;

  case 19: /* postfix_expression: postfix_expression LEFT_BRACKET integer_expression RIGHT_BRACKET  */
#line 435 "MachineIndependent/glslang.y"
                                                                       {
        (yyval.interm.intermTypedNode) = parseContext.handleBracketDereference((yyvsp[-2].lex).loc, (yyvsp[-3].interm.intermTypedNode), (yyvsp[-1].interm.intermTypedNode));
    }
#line 5640 "MachineIndependent/glslang_tab.cpp"
    break;

  case 20: /* postfix_expression: function_call  */
#line 438 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 5648 "MachineIndependent/glslang_tab.cpp"
    break;

  case 21: /* postfix_expression: postfix_expression DOT IDENTIFIER  */
#line 441 "MachineIndependent/glslang.y"
                                        {
        (yyval.interm.intermTypedNode) = parseContext.handleDotDereference((yyvsp[0].lex).loc, (yyvsp[-2].interm.intermTypedNode), *(yyvsp[0].lex).string);
    }
#line 5656 "MachineIndependent/glslang_tab.cpp"
    break;

  case 22: /* postfix_expression: postfix_expression INC_OP  */
#line 444 "MachineIndependent/glslang.y"
                                {
        parseContext.variableCheck((yyvsp[-1].interm.intermTypedNode));
        parseContext.lValueErrorCheck((yyvsp[0].lex).loc, "++", (yyvsp[-1].interm.intermTypedNode));
        (yyval.interm.intermTypedNode) = parseContext.handleUnaryMath((yyvsp[0].lex).loc, "++", EOpPostIncrement, (yyvsp[-1].interm.intermTypedNode));
    }
#line 5666 "MachineIndependent/glslang_tab.cpp"
    break;

  case 23: /* postfix_expression: postfix_expression DEC_OP  */
#line 449 "MachineIndependent/glslang.y"
                                {
        parseContext.variableCheck((yyvsp[-1].interm.intermTypedNode));
        parseContext.lValueErrorCheck((yyvsp[0].lex).loc, "--", (yyvsp[-1].interm.intermTypedNode));
        (yyval.interm.intermTypedNode) = parseContext.handleUnaryMath((yyvsp[0].lex).loc, "--", EOpPostDecrement, (yyvsp[-1].interm.intermTypedNode));
    }
#line 5676 "MachineIndependent/glslang_tab.cpp"
    break;

  case 24: /* integer_expression: expression  */
#line 457 "MachineIndependent/glslang.y"
                 {
        parseContext.arrayIndexCheck((yyvsp[0].interm.intermTypedNode), "[]");
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 5685 "MachineIndependent/glslang_tab.cpp"
    break;

  case 25: /* function_call: function_call_or_method  */
#line 464 "MachineIndependent/glslang.y"
                              {
        (yyval.interm.intermTypedNode) = parseContext.handleFunctionCall((yyvsp[0].interm).loc, (yyvsp[0].interm).function, (yyvsp[0].interm).intermNode);
        delete (yyvsp[0].interm).function;
    }
#line 5694 "MachineIndependent/glslang_tab.cpp"
    break;

  case 26: /* function_call_or_method: function_call_generic  */
#line 471 "MachineIndependent/glslang.y"
                            {
        (yyval.interm) = (yyvsp[0].interm);
    }
#line 5702 "MachineIndependent/glslang_tab.cpp"
    break;

  case 27: /* function_call_generic: function_call_header_with_parameters RIGHT_PAREN  */
#line 477 "MachineIndependent/glslang.y"
                                                       {
        (yyval.interm) = (yyvsp[-1].interm);
        (yyval.interm).loc = (yyvsp[0].lex).loc;
    }
#line 5711 "MachineIndependent/glslang_tab.cpp"
    break;

  case 28: /* function_call_generic: function_call_header_no_parameters RIGHT_PAREN  */
#line 481 "MachineIndependent/glslang.y"
                                                     {
        (yyval.interm) = (yyvsp[-1].interm);
        (yyval.interm).loc = (yyvsp[0].lex).loc;
    }
#line 5720 "MachineIndependent/glslang_tab.cpp"
    break;

  case 29: /* function_call_header_no_parameters: function_call_header VOID  */
#line 488 "MachineIndependent/glslang.y"
                                {
        (yyval.interm) = (yyvsp[-1].interm);
    }
#line 5728 "MachineIndependent/glslang_tab.cpp"
    break;

  case 30: /* function_call_header_no_parameters: function_call_header  */
#line 491 "MachineIndependent/glslang.y"
                           {
        (yyval.interm) = (yyvsp[0].interm);
    }
#line 5736 "MachineIndependent/glslang_tab.cpp"
    break;

  case 31: /* function_call_header_with_parameters: function_call_header assignment_expression  */
#line 497 "MachineIndependent/glslang.y"
                                                 {
        if (parseContext.spvVersion.vulkan > 0
            && parseContext.spvVersion.vulkanRelaxed
            && (yyvsp[0].interm.intermTypedNode)->getType().containsOpaque())
        {
            (yyval.interm).intermNode = parseContext.vkRelaxedRemapFunctionArgument((yyval.interm).loc, (yyvsp[-1].interm).function, (yyvsp[0].interm.intermTypedNode));
            (yyval.interm).function = (yyvsp[-1].interm).function;
        }
        else
        {
            TParameter param = { 0, new TType, {} };
            param.type->shallowCopy((yyvsp[0].interm.intermTypedNode)->getType());

            (yyvsp[-1].interm).function->addParameter(param);
            (yyval.interm).function = (yyvsp[-1].interm).function;
            (yyval.interm).intermNode = (yyvsp[0].interm.intermTypedNode);
        }
    }
#line 5759 "MachineIndependent/glslang_tab.cpp"
    break;

  case 32: /* function_call_header_with_parameters: function_call_header_with_parameters COMMA assignment_expression  */
#line 515 "MachineIndependent/glslang.y"
                                                                       {
        if (parseContext.spvVersion.vulkan > 0
            && parseContext.spvVersion.vulkanRelaxed
            && (yyvsp[0].interm.intermTypedNode)->getType().containsOpaque())
        {
            TIntermNode* remappedNode = parseContext.vkRelaxedRemapFunctionArgument((yyvsp[-1].lex).loc, (yyvsp[-2].interm).function, (yyvsp[0].interm.intermTypedNode));
            if (remappedNode == (yyvsp[0].interm.intermTypedNode))
                (yyval.interm).intermNode = parseContext.intermediate.growAggregate((yyvsp[-2].interm).intermNode, (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).loc);
            else
                (yyval.interm).intermNode = parseContext.intermediate.mergeAggregate((yyvsp[-2].interm).intermNode, remappedNode, (yyvsp[-1].lex).loc);
            (yyval.interm).function = (yyvsp[-2].interm).function;
        }
        else
        {
            TParameter param = { 0, new TType, {} };
            param.type->shallowCopy((yyvsp[0].interm.intermTypedNode)->getType());

            (yyvsp[-2].interm).function->addParameter(param);
            (yyval.interm).function = (yyvsp[-2].interm).function;
            (yyval.interm).intermNode = parseContext.intermediate.growAggregate((yyvsp[-2].interm).intermNode, (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).loc);
        }
    }
#line 5786 "MachineIndependent/glslang_tab.cpp"
    break;

  case 33: /* function_call_header: function_identifier LEFT_PAREN  */
#line 540 "MachineIndependent/glslang.y"
                                     {
        (yyval.interm) = (yyvsp[-1].interm);
    }
#line 5794 "MachineIndependent/glslang_tab.cpp"
    break;

  case 34: /* function_identifier: type_specifier  */
#line 548 "MachineIndependent/glslang.y"
                     {
        // Constructor
        (yyval.interm).intermNode = 0;
        (yyval.interm).function = parseContext.handleConstructorCall((yyvsp[0].interm.type).loc, (yyvsp[0].interm.type));
    }
#line 5804 "MachineIndependent/glslang_tab.cpp"
    break;

  case 35: /* function_identifier: postfix_expression  */
#line 553 "MachineIndependent/glslang.y"
                         {
        //
        // Should be a method or subroutine call, but we haven't recognized the arguments yet.
        //
        (yyval.interm).function = 0;
        (yyval.interm).intermNode = 0;

        TIntermMethod* method = (yyvsp[0].interm.intermTypedNode)->getAsMethodNode();
        if (method) {
            (yyval.interm).function = new TFunction(&method->getMethodName(), method->getType(), EOpArrayLength);
            (yyval.interm).intermNode = method->getObject();
        } else {
            TIntermSymbol* symbol = (yyvsp[0].interm.intermTypedNode)->getAsSymbolNode();
            if (symbol) {
                parseContext.reservedErrorCheck(symbol->getLoc(), symbol->getName());
                TFunction *function = new TFunction(&symbol->getName(), TType(EbtVoid));
                (yyval.interm).function = function;
            } else
                parseContext.error((yyvsp[0].interm.intermTypedNode)->getLoc(), "function call, method, or subroutine call expected", "", "");
        }

        if ((yyval.interm).function == 0) {
            // error recover
            TString* empty = NewPoolTString("");
            (yyval.interm).function = new TFunction(empty, TType(EbtVoid), EOpNull);
        }
    }
#line 5836 "MachineIndependent/glslang_tab.cpp"
    break;

  case 36: /* function_identifier: non_uniform_qualifier  */
#line 580 "MachineIndependent/glslang.y"
                            {
        // Constructor
        (yyval.interm).intermNode = 0;
        (yyval.interm).function = parseContext.handleConstructorCall((yyvsp[0].interm.type).loc, (yyvsp[0].interm.type));
    }
#line 5846 "MachineIndependent/glslang_tab.cpp"
    break;

  case 37: /* unary_expression: postfix_expression  */
#line 588 "MachineIndependent/glslang.y"
                         {
        parseContext.variableCheck((yyvsp[0].interm.intermTypedNode));
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
        if (TIntermMethod* method = (yyvsp[0].interm.intermTypedNode)->getAsMethodNode())
            parseContext.error((yyvsp[0].interm.intermTypedNode)->getLoc(), "incomplete method syntax", method->getMethodName().c_str(), "");
    }
#line 5857 "MachineIndependent/glslang_tab.cpp"
    break;

  case 38: /* unary_expression: INC_OP unary_expression  */
#line 594 "MachineIndependent/glslang.y"
                              {
        parseContext.lValueErrorCheck((yyvsp[-1].lex).loc, "++", (yyvsp[0].interm.intermTypedNode));
        (yyval.interm.intermTypedNode) = parseContext.handleUnaryMath((yyvsp[-1].lex).loc, "++", EOpPreIncrement, (yyvsp[0].interm.intermTypedNode));
    }
#line 5866 "MachineIndependent/glslang_tab.cpp"
    break;

  case 39: /* unary_expression: DEC_OP unary_expression  */
#line 598 "MachineIndependent/glslang.y"
                              {
        parseContext.lValueErrorCheck((yyvsp[-1].lex).loc, "--", (yyvsp[0].interm.intermTypedNode));
        (yyval.interm.intermTypedNode) = parseContext.handleUnaryMath((yyvsp[-1].lex).loc, "--", EOpPreDecrement, (yyvsp[0].interm.intermTypedNode));
    }
#line 5875 "MachineIndependent/glslang_tab.cpp"
    break;

  case 40: /* unary_expression: unary_operator unary_expression  */
#line 602 "MachineIndependent/glslang.y"
                                      {
        if ((yyvsp[-1].interm).op != EOpNull) {
            char errorOp[2] = {0, 0};
            switch((yyvsp[-1].interm).op) {
            case EOpNegative:   errorOp[0] = '-'; break;
            case EOpLogicalNot: errorOp[0] = '!'; break;
            case EOpBitwiseNot: errorOp[0] = '~'; break;
            default: break; // some compilers want this
            }
            (yyval.interm.intermTypedNode) = parseContext.handleUnaryMath((yyvsp[-1].interm).loc, errorOp, (yyvsp[-1].interm).op, (yyvsp[0].interm.intermTypedNode));
        } else {
            (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
            if ((yyval.interm.intermTypedNode)->getAsConstantUnion())
                (yyval.interm.intermTypedNode)->getAsConstantUnion()->setExpression();
        }
    }
#line 5896 "MachineIndependent/glslang_tab.cpp"
    break;

  case 41: /* unary_operator: PLUS  */
#line 622 "MachineIndependent/glslang.y"
            { (yyval.interm).loc = (yyvsp[0].lex).loc; (yyval.interm).op = EOpNull; }
#line 5902 "MachineIndependent/glslang_tab.cpp"
    break;

  case 42: /* unary_operator: DASH  */
#line 623 "MachineIndependent/glslang.y"
            { (yyval.interm).loc = (yyvsp[0].lex).loc; (yyval.interm).op = EOpNegative; }
#line 5908 "MachineIndependent/glslang_tab.cpp"
    break;

  case 43: /* unary_operator: BANG  */
#line 624 "MachineIndependent/glslang.y"
            { (yyval.interm).loc = (yyvsp[0].lex).loc; (yyval.interm).op = EOpLogicalNot; }
#line 5914 "MachineIndependent/glslang_tab.cpp"
    break;

  case 44: /* unary_operator: TILDE  */
#line 625 "MachineIndependent/glslang.y"
            { (yyval.interm).loc = (yyvsp[0].lex).loc; (yyval.interm).op = EOpBitwiseNot;
              parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "bitwise not"); }
#line 5921 "MachineIndependent/glslang_tab.cpp"
    break;

  case 45: /* multiplicative_expression: unary_expression  */
#line 631 "MachineIndependent/glslang.y"
                       { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 5927 "MachineIndependent/glslang_tab.cpp"
    break;

  case 46: /* multiplicative_expression: multiplicative_expression STAR unary_expression  */
#line 632 "MachineIndependent/glslang.y"
                                                      {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "*", EOpMul, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 5937 "MachineIndependent/glslang_tab.cpp"
    break;

  case 47: /* multiplicative_expression: multiplicative_expression SLASH unary_expression  */
#line 637 "MachineIndependent/glslang.y"
                                                       {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "/", EOpDiv, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 5947 "MachineIndependent/glslang_tab.cpp"
    break;

  case 48: /* multiplicative_expression: multiplicative_expression PERCENT unary_expression  */
#line 642 "MachineIndependent/glslang.y"
                                                         {
        parseContext.fullIntegerCheck((yyvsp[-1].lex).loc, "%");
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "%", EOpMod, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 5958 "MachineIndependent/glslang_tab.cpp"
    break;

  case 49: /* additive_expression: multiplicative_expression  */
#line 651 "MachineIndependent/glslang.y"
                                { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 5964 "MachineIndependent/glslang_tab.cpp"
    break;

  case 50: /* additive_expression: additive_expression PLUS multiplicative_expression  */
#line 652 "MachineIndependent/glslang.y"
                                                         {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "+", EOpAdd, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 5974 "MachineIndependent/glslang_tab.cpp"
    break;

  case 51: /* additive_expression: additive_expression DASH multiplicative_expression  */
#line 657 "MachineIndependent/glslang.y"
                                                         {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "-", EOpSub, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 5984 "MachineIndependent/glslang_tab.cpp"
    break;

  case 52: /* shift_expression: additive_expression  */
#line 665 "MachineIndependent/glslang.y"
                          { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 5990 "MachineIndependent/glslang_tab.cpp"
    break;

  case 53: /* shift_expression: shift_expression LEFT_OP additive_expression  */
#line 666 "MachineIndependent/glslang.y"
                                                   {
        parseContext.fullIntegerCheck((yyvsp[-1].lex).loc, "bit shift left");
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "<<", EOpLeftShift, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 6001 "MachineIndependent/glslang_tab.cpp"
    break;

  case 54: /* shift_expression: shift_expression RIGHT_OP additive_expression  */
#line 672 "MachineIndependent/glslang.y"
                                                    {
        parseContext.fullIntegerCheck((yyvsp[-1].lex).loc, "bit shift right");
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, ">>", EOpRightShift, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 6012 "MachineIndependent/glslang_tab.cpp"
    break;

  case 55: /* relational_expression: shift_expression  */
#line 681 "MachineIndependent/glslang.y"
                       { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 6018 "MachineIndependent/glslang_tab.cpp"
    break;

  case 56: /* relational_expression: relational_expression LEFT_ANGLE shift_expression  */
#line 682 "MachineIndependent/glslang.y"
                                                        {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "<", EOpLessThan, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion(false, (yyvsp[-1].lex).loc);
    }
#line 6028 "MachineIndependent/glslang_tab.cpp"
    break;

  case 57: /* relational_expression: relational_expression RIGHT_ANGLE shift_expression  */
#line 687 "MachineIndependent/glslang.y"
                                                          {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, ">", EOpGreaterThan, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion(false, (yyvsp[-1].lex).loc);
    }
#line 6038 "MachineIndependent/glslang_tab.cpp"
    break;

  case 58: /* relational_expression: relational_expression LE_OP shift_expression  */
#line 692 "MachineIndependent/glslang.y"
                                                    {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "<=", EOpLessThanEqual, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion(false, (yyvsp[-1].lex).loc);
    }
#line 6048 "MachineIndependent/glslang_tab.cpp"
    break;

  case 59: /* relational_expression: relational_expression GE_OP shift_expression  */
#line 697 "MachineIndependent/glslang.y"
                                                    {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, ">=", EOpGreaterThanEqual, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion(false, (yyvsp[-1].lex).loc);
    }
#line 6058 "MachineIndependent/glslang_tab.cpp"
    break;

  case 60: /* equality_expression: relational_expression  */
#line 705 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 6064 "MachineIndependent/glslang_tab.cpp"
    break;

  case 61: /* equality_expression: equality_expression EQ_OP relational_expression  */
#line 706 "MachineIndependent/glslang.y"
                                                       {
        parseContext.arrayObjectCheck((yyvsp[-1].lex).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "array comparison");
        parseContext.opaqueCheck((yyvsp[-1].lex).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "==");
        parseContext.specializationCheck((yyvsp[-1].lex).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "==");
        parseContext.referenceCheck((yyvsp[-1].lex).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "==");
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "==", EOpEqual, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion(false, (yyvsp[-1].lex).loc);
    }
#line 6078 "MachineIndependent/glslang_tab.cpp"
    break;

  case 62: /* equality_expression: equality_expression NE_OP relational_expression  */
#line 715 "MachineIndependent/glslang.y"
                                                      {
        parseContext.arrayObjectCheck((yyvsp[-1].lex).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "array comparison");
        parseContext.opaqueCheck((yyvsp[-1].lex).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "!=");
        parseContext.specializationCheck((yyvsp[-1].lex).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "!=");
        parseContext.referenceCheck((yyvsp[-1].lex).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "!=");
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "!=", EOpNotEqual, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion(false, (yyvsp[-1].lex).loc);
    }
#line 6092 "MachineIndependent/glslang_tab.cpp"
    break;

  case 63: /* and_expression: equality_expression  */
#line 727 "MachineIndependent/glslang.y"
                          { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 6098 "MachineIndependent/glslang_tab.cpp"
    break;

  case 64: /* and_expression: and_expression AMPERSAND equality_expression  */
#line 728 "MachineIndependent/glslang.y"
                                                   {
        parseContext.fullIntegerCheck((yyvsp[-1].lex).loc, "bitwise and");
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "&", EOpAnd, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 6109 "MachineIndependent/glslang_tab.cpp"
    break;

  case 65: /* exclusive_or_expression: and_expression  */
#line 737 "MachineIndependent/glslang.y"
                     { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 6115 "MachineIndependent/glslang_tab.cpp"
    break;

  case 66: /* exclusive_or_expression: exclusive_or_expression CARET and_expression  */
#line 738 "MachineIndependent/glslang.y"
                                                   {
        parseContext.fullIntegerCheck((yyvsp[-1].lex).loc, "bitwise exclusive or");
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "^", EOpExclusiveOr, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 6126 "MachineIndependent/glslang_tab.cpp"
    break;

  case 67: /* inclusive_or_expression: exclusive_or_expression  */
#line 747 "MachineIndependent/glslang.y"
                              { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 6132 "MachineIndependent/glslang_tab.cpp"
    break;

  case 68: /* inclusive_or_expression: inclusive_or_expression VERTICAL_BAR exclusive_or_expression  */
#line 748 "MachineIndependent/glslang.y"
                                                                   {
        parseContext.fullIntegerCheck((yyvsp[-1].lex).loc, "bitwise inclusive or");
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "|", EOpInclusiveOr, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 6143 "MachineIndependent/glslang_tab.cpp"
    break;

  case 69: /* logical_and_expression: inclusive_or_expression  */
#line 757 "MachineIndependent/glslang.y"
                              { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 6149 "MachineIndependent/glslang_tab.cpp"
    break;

  case 70: /* logical_and_expression: logical_and_expression AND_OP inclusive_or_expression  */
#line 758 "MachineIndependent/glslang.y"
                                                            {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "&&", EOpLogicalAnd, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion(false, (yyvsp[-1].lex).loc);
    }
#line 6159 "MachineIndependent/glslang_tab.cpp"
    break;

  case 71: /* logical_xor_expression: logical_and_expression  */
#line 766 "MachineIndependent/glslang.y"
                             { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 6165 "MachineIndependent/glslang_tab.cpp"
    break;

  case 72: /* logical_xor_expression: logical_xor_expression XOR_OP logical_and_expression  */
#line 767 "MachineIndependent/glslang.y"
                                                            {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "^^", EOpLogicalXor, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion(false, (yyvsp[-1].lex).loc);
    }
#line 6175 "MachineIndependent/glslang_tab.cpp"
    break;

  case 73: /* logical_or_expression: logical_xor_expression  */
#line 775 "MachineIndependent/glslang.y"
                             { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 6181 "MachineIndependent/glslang_tab.cpp"
    break;

  case 74: /* logical_or_expression: logical_or_expression OR_OP logical_xor_expression  */
#line 776 "MachineIndependent/glslang.y"
                                                          {
        (yyval.interm.intermTypedNode) = parseContext.handleBinaryMath((yyvsp[-1].lex).loc, "||", EOpLogicalOr, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0)
            (yyval.interm.intermTypedNode) = parseContext.intermediate.addConstantUnion(false, (yyvsp[-1].lex).loc);
    }
#line 6191 "MachineIndependent/glslang_tab.cpp"
    break;

  case 75: /* conditional_expression: logical_or_expression  */
#line 784 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 6197 "MachineIndependent/glslang_tab.cpp"
    break;

  case 76: /* $@1: %empty  */
#line 785 "MachineIndependent/glslang.y"
                                     {
        ++parseContext.controlFlowNestingLevel;
    }
#line 6205 "MachineIndependent/glslang_tab.cpp"
    break;

  case 77: /* conditional_expression: logical_or_expression QUESTION $@1 expression COLON assignment_expression  */
#line 788 "MachineIndependent/glslang.y"
                                             {
        --parseContext.controlFlowNestingLevel;
        parseContext.boolCheck((yyvsp[-4].lex).loc, (yyvsp[-5].interm.intermTypedNode));
        parseContext.rValueErrorCheck((yyvsp[-4].lex).loc, "?", (yyvsp[-5].interm.intermTypedNode));
        parseContext.rValueErrorCheck((yyvsp[-1].lex).loc, ":", (yyvsp[-2].interm.intermTypedNode));
        parseContext.rValueErrorCheck((yyvsp[-1].lex).loc, ":", (yyvsp[0].interm.intermTypedNode));
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addSelection((yyvsp[-5].interm.intermTypedNode), (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-4].lex).loc);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.binaryOpError((yyvsp[-4].lex).loc, ":", (yyvsp[-2].interm.intermTypedNode)->getCompleteString(parseContext.intermediate.getEnhancedMsgs()), (yyvsp[0].interm.intermTypedNode)->getCompleteString(parseContext.intermediate.getEnhancedMsgs()));
            (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
        }
    }
#line 6222 "MachineIndependent/glslang_tab.cpp"
    break;

  case 78: /* assignment_expression: conditional_expression  */
#line 803 "MachineIndependent/glslang.y"
                             { (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode); }
#line 6228 "MachineIndependent/glslang_tab.cpp"
    break;

  case 79: /* assignment_expression: unary_expression assignment_operator assignment_expression  */
#line 804 "MachineIndependent/glslang.y"
                                                                 {
        parseContext.arrayObjectCheck((yyvsp[-1].interm).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "array assignment");
        parseContext.opaqueCheck((yyvsp[-1].interm).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "=");
        parseContext.storage16BitAssignmentCheck((yyvsp[-1].interm).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "=");
        parseContext.specializationCheck((yyvsp[-1].interm).loc, (yyvsp[-2].interm.intermTypedNode)->getType(), "=");
        parseContext.lValueErrorCheck((yyvsp[-1].interm).loc, "assign", (yyvsp[-2].interm.intermTypedNode));
        parseContext.rValueErrorCheck((yyvsp[-1].interm).loc, "assign", (yyvsp[0].interm.intermTypedNode));
        (yyval.interm.intermTypedNode) = parseContext.addAssign((yyvsp[-1].interm).loc, (yyvsp[-1].interm).op, (yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.assignError((yyvsp[-1].interm).loc, "assign", (yyvsp[-2].interm.intermTypedNode)->getCompleteString(parseContext.intermediate.getEnhancedMsgs()), (yyvsp[0].interm.intermTypedNode)->getCompleteString(parseContext.intermediate.getEnhancedMsgs()));
            (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
        }
    }
#line 6246 "MachineIndependent/glslang_tab.cpp"
    break;

  case 80: /* assignment_operator: EQUAL  */
#line 820 "MachineIndependent/glslang.y"
            {
        (yyval.interm).loc = (yyvsp[0].lex).loc;
        (yyval.interm).op = EOpAssign;
    }
#line 6255 "MachineIndependent/glslang_tab.cpp"
    break;

  case 81: /* assignment_operator: MUL_ASSIGN  */
#line 824 "MachineIndependent/glslang.y"
                 {
        (yyval.interm).loc = (yyvsp[0].lex).loc;
        (yyval.interm).op = EOpMulAssign;
    }
#line 6264 "MachineIndependent/glslang_tab.cpp"
    break;

  case 82: /* assignment_operator: DIV_ASSIGN  */
#line 828 "MachineIndependent/glslang.y"
                 {
        (yyval.interm).loc = (yyvsp[0].lex).loc;
        (yyval.interm).op = EOpDivAssign;
    }
#line 6273 "MachineIndependent/glslang_tab.cpp"
    break;

  case 83: /* assignment_operator: MOD_ASSIGN  */
#line 832 "MachineIndependent/glslang.y"
                 {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "%=");
        (yyval.interm).loc = (yyvsp[0].lex).loc;
        (yyval.interm).op = EOpModAssign;
    }
#line 6283 "MachineIndependent/glslang_tab.cpp"
    break;

  case 84: /* assignment_operator: ADD_ASSIGN  */
#line 837 "MachineIndependent/glslang.y"
                 {
        (yyval.interm).loc = (yyvsp[0].lex).loc;
        (yyval.interm).op = EOpAddAssign;
    }
#line 6292 "MachineIndependent/glslang_tab.cpp"
    break;

  case 85: /* assignment_operator: SUB_ASSIGN  */
#line 841 "MachineIndependent/glslang.y"
                 {
        (yyval.interm).loc = (yyvsp[0].lex).loc;
        (yyval.interm).op = EOpSubAssign;
    }
#line 6301 "MachineIndependent/glslang_tab.cpp"
    break;

  case 86: /* assignment_operator: LEFT_ASSIGN  */
#line 845 "MachineIndependent/glslang.y"
                  {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "bit-shift left assign");
        (yyval.interm).loc = (yyvsp[0].lex).loc; (yyval.interm).op = EOpLeftShiftAssign;
    }
#line 6310 "MachineIndependent/glslang_tab.cpp"
    break;

  case 87: /* assignment_operator: RIGHT_ASSIGN  */
#line 849 "MachineIndependent/glslang.y"
                   {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "bit-shift right assign");
        (yyval.interm).loc = (yyvsp[0].lex).loc; (yyval.interm).op = EOpRightShiftAssign;
    }
#line 6319 "MachineIndependent/glslang_tab.cpp"
    break;

  case 88: /* assignment_operator: AND_ASSIGN  */
#line 853 "MachineIndependent/glslang.y"
                 {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "bitwise-and assign");
        (yyval.interm).loc = (yyvsp[0].lex).loc; (yyval.interm).op = EOpAndAssign;
    }
#line 6328 "MachineIndependent/glslang_tab.cpp"
    break;

  case 89: /* assignment_operator: XOR_ASSIGN  */
#line 857 "MachineIndependent/glslang.y"
                 {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "bitwise-xor assign");
        (yyval.interm).loc = (yyvsp[0].lex).loc; (yyval.interm).op = EOpExclusiveOrAssign;
    }
#line 6337 "MachineIndependent/glslang_tab.cpp"
    break;

  case 90: /* assignment_operator: OR_ASSIGN  */
#line 861 "MachineIndependent/glslang.y"
                {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "bitwise-or assign");
        (yyval.interm).loc = (yyvsp[0].lex).loc; (yyval.interm).op = EOpInclusiveOrAssign;
    }
#line 6346 "MachineIndependent/glslang_tab.cpp"
    break;

  case 91: /* expression: assignment_expression  */
#line 868 "MachineIndependent/glslang.y"
                            {
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 6354 "MachineIndependent/glslang_tab.cpp"
    break;

  case 92: /* expression: expression COMMA assignment_expression  */
#line 871 "MachineIndependent/glslang.y"
                                             {
        parseContext.samplerConstructorLocationCheck((yyvsp[-1].lex).loc, ",", (yyvsp[0].interm.intermTypedNode));
        (yyval.interm.intermTypedNode) = parseContext.intermediate.addComma((yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).loc);
        if ((yyval.interm.intermTypedNode) == 0) {
            parseContext.binaryOpError((yyvsp[-1].lex).loc, ",", (yyvsp[-2].interm.intermTypedNode)->getCompleteString(parseContext.intermediate.getEnhancedMsgs()), (yyvsp[0].interm.intermTypedNode)->getCompleteString(parseContext.intermediate.getEnhancedMsgs()));
            (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
        }
    }
#line 6367 "MachineIndependent/glslang_tab.cpp"
    break;

  case 93: /* constant_expression: conditional_expression  */
#line 882 "MachineIndependent/glslang.y"
                             {
        parseContext.constantValueCheck((yyvsp[0].interm.intermTypedNode), "");
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 6376 "MachineIndependent/glslang_tab.cpp"
    break;

  case 94: /* declaration: function_prototype SEMICOLON  */
#line 889 "MachineIndependent/glslang.y"
                                   {
        parseContext.handleFunctionDeclarator((yyvsp[-1].interm).loc, *(yyvsp[-1].interm).function, true /* prototype */);
        (yyval.interm.intermNode) = 0;
        // TODO: 4.0 functionality: subroutines: make the identifier a user type for this signature
    }
#line 6386 "MachineIndependent/glslang_tab.cpp"
    break;

  case 95: /* declaration: spirv_instruction_qualifier function_prototype SEMICOLON  */
#line 894 "MachineIndependent/glslang.y"
                                                               {
        parseContext.requireExtensions((yyvsp[-1].interm).loc, 1, &E_GL_EXT_spirv_intrinsics, "SPIR-V instruction qualifier");
        (yyvsp[-1].interm).function->setSpirvInstruction(*(yyvsp[-2].interm.spirvInst)); // Attach SPIR-V intruction qualifier
        parseContext.handleFunctionDeclarator((yyvsp[-1].interm).loc, *(yyvsp[-1].interm).function, true /* prototype */);
        (yyval.interm.intermNode) = 0;
        // TODO: 4.0 functionality: subroutines: make the identifier a user type for this signature
    }
#line 6398 "MachineIndependent/glslang_tab.cpp"
    break;

  case 96: /* declaration: spirv_execution_mode_qualifier SEMICOLON  */
#line 901 "MachineIndependent/glslang.y"
                                               {
        parseContext.globalCheck((yyvsp[0].lex).loc, "SPIR-V execution mode qualifier");
        parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_EXT_spirv_intrinsics, "SPIR-V execution mode qualifier");
        (yyval.interm.intermNode) = 0;
    }
#line 6408 "MachineIndependent/glslang_tab.cpp"
    break;

  case 97: /* declaration: init_declarator_list SEMICOLON  */
#line 906 "MachineIndependent/glslang.y"
                                     {
        if ((yyvsp[-1].interm).intermNode && (yyvsp[-1].interm).intermNode->getAsAggregate())
            (yyvsp[-1].interm).intermNode->getAsAggregate()->setOperator(EOpSequence);
        (yyval.interm.intermNode) = (yyvsp[-1].interm).intermNode;
    }
#line 6418 "MachineIndependent/glslang_tab.cpp"
    break;

  case 98: /* declaration: PRECISION precision_qualifier type_specifier SEMICOLON  */
#line 911 "MachineIndependent/glslang.y"
                                                             {
        parseContext.profileRequires((yyvsp[-3].lex).loc, ENoProfile, 130, 0, "precision statement");
        // lazy setting of the previous scope's defaults, has effect only the first time it is called in a particular scope
        parseContext.symbolTable.setPreviousDefaultPrecisions(&parseContext.defaultPrecision[0]);
        parseContext.setDefaultPrecision((yyvsp[-3].lex).loc, (yyvsp[-1].interm.type), (yyvsp[-2].interm.type).qualifier.precision);
        (yyval.interm.intermNode) = 0;
    }
#line 6430 "MachineIndependent/glslang_tab.cpp"
    break;

  case 99: /* declaration: block_structure SEMICOLON  */
#line 918 "MachineIndependent/glslang.y"
                                {
        (yyval.interm.intermNode) = parseContext.declareBlock((yyvsp[-1].interm).loc, *(yyvsp[-1].interm).typeList);
    }
#line 6438 "MachineIndependent/glslang_tab.cpp"
    break;

  case 100: /* declaration: block_structure IDENTIFIER SEMICOLON  */
#line 921 "MachineIndependent/glslang.y"
                                           {
        (yyval.interm.intermNode) = parseContext.declareBlock((yyvsp[-2].interm).loc, *(yyvsp[-2].interm).typeList, (yyvsp[-1].lex).string);
    }
#line 6446 "MachineIndependent/glslang_tab.cpp"
    break;

  case 101: /* declaration: block_structure IDENTIFIER array_specifier SEMICOLON  */
#line 924 "MachineIndependent/glslang.y"
                                                           {
        (yyval.interm.intermNode) = parseContext.declareBlock((yyvsp[-3].interm).loc, *(yyvsp[-3].interm).typeList, (yyvsp[-2].lex).string, (yyvsp[-1].interm).arraySizes);
    }
#line 6454 "MachineIndependent/glslang_tab.cpp"
    break;

  case 102: /* declaration: type_qualifier SEMICOLON  */
#line 927 "MachineIndependent/glslang.y"
                               {
        parseContext.globalQualifierFixCheck((yyvsp[-1].interm.type).loc, (yyvsp[-1].interm.type).qualifier);
        parseContext.updateStandaloneQualifierDefaults((yyvsp[-1].interm.type).loc, (yyvsp[-1].interm.type));
        (yyval.interm.intermNode) = 0;
    }
#line 6464 "MachineIndependent/glslang_tab.cpp"
    break;

  case 103: /* declaration: type_qualifier identifier_list SEMICOLON  */
#line 932 "MachineIndependent/glslang.y"
                                               {
        parseContext.checkNoShaderLayouts((yyvsp[-2].interm.type).loc, (yyvsp[-2].interm.type).shaderQualifiers);
        parseContext.addQualifierToExisting((yyvsp[-2].interm.type).loc, (yyvsp[-2].interm.type).qualifier, *(yyvsp[-1].interm.identifierList));
        (yyval.interm.intermNode) = 0;
    }
#line 6474 "MachineIndependent/glslang_tab.cpp"
    break;

  case 104: /* $@2: %empty  */
#line 940 "MachineIndependent/glslang.y"
                                           { parseContext.nestedBlockCheck((yyvsp[-2].interm.type).loc); }
#line 6480 "MachineIndependent/glslang_tab.cpp"
    break;

  case 105: /* block_structure: type_qualifier IDENTIFIER LEFT_BRACE $@2 struct_declaration_without_heap RIGHT_BRACE  */
#line 940 "MachineIndependent/glslang.y"
                                                                                                                                  {
        --parseContext.blockNestingLevel;
        parseContext.blockName = (yyvsp[-4].lex).string;
        parseContext.globalQualifierFixCheck((yyvsp[-5].interm.type).loc, (yyvsp[-5].interm.type).qualifier);
        parseContext.checkNoShaderLayouts((yyvsp[-5].interm.type).loc, (yyvsp[-5].interm.type).shaderQualifiers);
        parseContext.currentBlockQualifier = (yyvsp[-5].interm.type).qualifier;
        (yyval.interm).loc = (yyvsp[-5].interm.type).loc;
        (yyval.interm).typeList = (yyvsp[-1].interm.typeList);
    }
#line 6494 "MachineIndependent/glslang_tab.cpp"
    break;

  case 106: /* identifier_list: IDENTIFIER  */
#line 952 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.identifierList) = new TIdentifierList;
        (yyval.interm.identifierList)->push_back((yyvsp[0].lex).string);
    }
#line 6503 "MachineIndependent/glslang_tab.cpp"
    break;

  case 107: /* identifier_list: identifier_list COMMA IDENTIFIER  */
#line 956 "MachineIndependent/glslang.y"
                                       {
        (yyval.interm.identifierList) = (yyvsp[-2].interm.identifierList);
        (yyval.interm.identifierList)->push_back((yyvsp[0].lex).string);
    }
#line 6512 "MachineIndependent/glslang_tab.cpp"
    break;

  case 108: /* function_prototype: function_declarator RIGHT_PAREN  */
#line 963 "MachineIndependent/glslang.y"
                                       {
        (yyval.interm).function = (yyvsp[-1].interm.function);
        if (parseContext.compileOnly) (yyval.interm).function->setExport();
        (yyval.interm).loc = (yyvsp[0].lex).loc;
    }
#line 6522 "MachineIndependent/glslang_tab.cpp"
    break;

  case 109: /* function_prototype: function_declarator RIGHT_PAREN attribute  */
#line 968 "MachineIndependent/glslang.y"
                                                {
        (yyval.interm).function = (yyvsp[-2].interm.function);
        if (parseContext.compileOnly) (yyval.interm).function->setExport();
        (yyval.interm).loc = (yyvsp[-1].lex).loc;
        const char * extensions[2] = { E_GL_EXT_subgroup_uniform_control_flow, E_GL_EXT_maximal_reconvergence };
        parseContext.requireExtensions((yyvsp[-1].lex).loc, 2, extensions, "attribute");
        parseContext.handleFunctionAttributes((yyvsp[-1].lex).loc, *(yyvsp[0].interm.attributes));
    }
#line 6535 "MachineIndependent/glslang_tab.cpp"
    break;

  case 110: /* function_prototype: attribute function_declarator RIGHT_PAREN  */
#line 976 "MachineIndependent/glslang.y"
                                                {
        (yyval.interm).function = (yyvsp[-1].interm.function);
        if (parseContext.compileOnly) (yyval.interm).function->setExport();
        (yyval.interm).loc = (yyvsp[0].lex).loc;
        const char * extensions[2] = { E_GL_EXT_subgroup_uniform_control_flow, E_GL_EXT_maximal_reconvergence };
        parseContext.requireExtensions((yyvsp[0].lex).loc, 2, extensions, "attribute");
        parseContext.handleFunctionAttributes((yyvsp[0].lex).loc, *(yyvsp[-2].interm.attributes));
    }
#line 6548 "MachineIndependent/glslang_tab.cpp"
    break;

  case 111: /* function_prototype: attribute function_declarator RIGHT_PAREN attribute  */
#line 984 "MachineIndependent/glslang.y"
                                                          {
        (yyval.interm).function = (yyvsp[-2].interm.function);
        if (parseContext.compileOnly) (yyval.interm).function->setExport();
        (yyval.interm).loc = (yyvsp[-1].lex).loc;
        const char * extensions[2] = { E_GL_EXT_subgroup_uniform_control_flow, E_GL_EXT_maximal_reconvergence };
        parseContext.requireExtensions((yyvsp[-1].lex).loc, 2, extensions, "attribute");
        parseContext.handleFunctionAttributes((yyvsp[-1].lex).loc, *(yyvsp[-3].interm.attributes));
        parseContext.handleFunctionAttributes((yyvsp[-1].lex).loc, *(yyvsp[0].interm.attributes));
    }
#line 6562 "MachineIndependent/glslang_tab.cpp"
    break;

  case 112: /* function_declarator: function_header  */
#line 996 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.function) = (yyvsp[0].interm.function);
    }
#line 6570 "MachineIndependent/glslang_tab.cpp"
    break;

  case 113: /* function_declarator: function_header_with_parameters  */
#line 999 "MachineIndependent/glslang.y"
                                      {
        (yyval.interm.function) = (yyvsp[0].interm.function);
    }
#line 6578 "MachineIndependent/glslang_tab.cpp"
    break;

  case 114: /* function_header_with_parameters: function_header parameter_declaration  */
#line 1006 "MachineIndependent/glslang.y"
                                            {
        // Add the parameter
        (yyval.interm.function) = (yyvsp[-1].interm.function);
        if ((yyvsp[0].interm).param.type->getBasicType() != EbtVoid)
        {
            if (!(parseContext.spvVersion.vulkan > 0 && parseContext.spvVersion.vulkanRelaxed))
                (yyvsp[-1].interm.function)->addParameter((yyvsp[0].interm).param);
            else
                parseContext.vkRelaxedRemapFunctionParameter((yyvsp[-1].interm.function), (yyvsp[0].interm).param);
        }
        else
            delete (yyvsp[0].interm).param.type;
    }
#line 6596 "MachineIndependent/glslang_tab.cpp"
    break;

  case 115: /* function_header_with_parameters: function_header_with_parameters COMMA parameter_declaration  */
#line 1019 "MachineIndependent/glslang.y"
                                                                  {
        //
        // Only first parameter of one-parameter functions can be void
        // The check for named parameters not being void is done in parameter_declarator
        //
        if ((yyvsp[0].interm).param.type->getBasicType() == EbtVoid) {
            //
            // This parameter > first is void
            //
            parseContext.error((yyvsp[-1].lex).loc, "cannot be an argument type except for '(void)'", "void", "");
            delete (yyvsp[0].interm).param.type;
        } else {
            // Add the parameter
            (yyval.interm.function) = (yyvsp[-2].interm.function);
            if (!(parseContext.spvVersion.vulkan > 0 && parseContext.spvVersion.vulkanRelaxed))
                (yyvsp[-2].interm.function)->addParameter((yyvsp[0].interm).param);
            else
                parseContext.vkRelaxedRemapFunctionParameter((yyvsp[-2].interm.function), (yyvsp[0].interm).param);
        }
    }
#line 6621 "MachineIndependent/glslang_tab.cpp"
    break;

  case 116: /* function_header_with_parameters: function_header_with_parameters COMMA DOT DOT DOT  */
#line 1039 "MachineIndependent/glslang.y"
                                                        {
        (yyval.interm.function) = (yyvsp[-4].interm.function);
        parseContext.makeVariadic((yyvsp[-4].interm.function), (yyvsp[-2].lex).loc);
    }
#line 6630 "MachineIndependent/glslang_tab.cpp"
    break;

  case 117: /* function_header: fully_specified_type IDENTIFIER LEFT_PAREN  */
#line 1046 "MachineIndependent/glslang.y"
                                                 {
        if ((yyvsp[-2].interm.type).qualifier.storage != EvqGlobal && (yyvsp[-2].interm.type).qualifier.storage != EvqTemporary) {
            parseContext.error((yyvsp[-1].lex).loc, "no qualifiers allowed for function return",
                               GetStorageQualifierString((yyvsp[-2].interm.type).qualifier.storage), "");
        }
        if ((yyvsp[-2].interm.type).arraySizes)
            parseContext.arraySizeRequiredCheck((yyvsp[-2].interm.type).loc, *(yyvsp[-2].interm.type).arraySizes);

        // Add the function as a prototype after parsing it (we do not support recursion)
        TFunction *function;
        TType type((yyvsp[-2].interm.type));

        // Potentially rename shader entry point function.  No-op most of the time.
        parseContext.renameShaderFunction((yyvsp[-1].lex).string);

        // Make the function
        function = new TFunction((yyvsp[-1].lex).string, type);
        (yyval.interm.function) = function;
    }
#line 6654 "MachineIndependent/glslang_tab.cpp"
    break;

  case 118: /* parameter_declarator: type_specifier IDENTIFIER  */
#line 1069 "MachineIndependent/glslang.y"
                                {
        if ((yyvsp[-1].interm.type).arraySizes) {
            parseContext.profileRequires((yyvsp[-1].interm.type).loc, ENoProfile, 120, E_GL_3DL_array_objects, "arrayed type");
            parseContext.profileRequires((yyvsp[-1].interm.type).loc, EEsProfile, 300, 0, "arrayed type");
            parseContext.arraySizeRequiredCheck((yyvsp[-1].interm.type).loc, *(yyvsp[-1].interm.type).arraySizes);
        }
        if ((yyvsp[-1].interm.type).basicType == EbtVoid) {
            parseContext.error((yyvsp[0].lex).loc, "illegal use of type 'void'", (yyvsp[0].lex).string->c_str(), "");
        }
        parseContext.reservedErrorCheck((yyvsp[0].lex).loc, *(yyvsp[0].lex).string);

        TParameter param = {(yyvsp[0].lex).string, new TType((yyvsp[-1].interm.type)), {}};
        (yyval.interm).loc = (yyvsp[0].lex).loc;
        (yyval.interm).param = param;
    }
#line 6674 "MachineIndependent/glslang_tab.cpp"
    break;

  case 119: /* parameter_declarator: type_specifier IDENTIFIER array_specifier  */
#line 1084 "MachineIndependent/glslang.y"
                                                {
        if ((yyvsp[-2].interm.type).arraySizes) {
            parseContext.profileRequires((yyvsp[-2].interm.type).loc, ENoProfile, 120, E_GL_3DL_array_objects, "arrayed type");
            parseContext.profileRequires((yyvsp[-2].interm.type).loc, EEsProfile, 300, 0, "arrayed type");
            parseContext.arraySizeRequiredCheck((yyvsp[-2].interm.type).loc, *(yyvsp[-2].interm.type).arraySizes);
        }
        TType* type = new TType((yyvsp[-2].interm.type));
        type->transferArraySizes((yyvsp[0].interm).arraySizes);
        type->copyArrayInnerSizes((yyvsp[-2].interm.type).arraySizes);

        parseContext.arrayOfArrayVersionCheck((yyvsp[-1].lex).loc, type->getArraySizes());
        parseContext.arraySizeRequiredCheck((yyvsp[0].interm).loc, *(yyvsp[0].interm).arraySizes);
        parseContext.reservedErrorCheck((yyvsp[-1].lex).loc, *(yyvsp[-1].lex).string);

        TParameter param = { (yyvsp[-1].lex).string, type, {} };

        (yyval.interm).loc = (yyvsp[-1].lex).loc;
        (yyval.interm).param = param;
    }
#line 6698 "MachineIndependent/glslang_tab.cpp"
    break;

  case 120: /* parameter_declarator: type_specifier IDENTIFIER EQUAL initializer  */
#line 1103 "MachineIndependent/glslang.y"
                                                  {
        TParameter param = parseContext.getParamWithDefault((yyvsp[-3].interm.type), (yyvsp[-2].lex).string, (yyvsp[0].interm.intermTypedNode), (yyvsp[-1].lex).loc);
        (yyval.interm).loc = (yyvsp[-2].lex).loc;
        (yyval.interm).param = param;
    }
#line 6708 "MachineIndependent/glslang_tab.cpp"
    break;

  case 121: /* parameter_declaration: type_qualifier parameter_declarator  */
#line 1114 "MachineIndependent/glslang.y"
                                          {
        (yyval.interm) = (yyvsp[0].interm);
        if ((yyvsp[-1].interm.type).qualifier.precision != EpqNone)
            (yyval.interm).param.type->getQualifier().precision = (yyvsp[-1].interm.type).qualifier.precision;
        parseContext.precisionQualifierCheck((yyval.interm).loc, (yyval.interm).param.type->getBasicType(), (yyval.interm).param.type->getQualifier(), (yyval.interm).param.type->hasTypeParameter());

        parseContext.checkNoShaderLayouts((yyvsp[-1].interm.type).loc, (yyvsp[-1].interm.type).shaderQualifiers);
        parseContext.parameterTypeCheck((yyvsp[0].interm).loc, (yyvsp[-1].interm.type).qualifier.storage, *(yyval.interm).param.type);
        parseContext.paramCheckFix((yyvsp[-1].interm.type).loc, (yyvsp[-1].interm.type).qualifier, *(yyval.interm).param.type);

    }
#line 6724 "MachineIndependent/glslang_tab.cpp"
    break;

  case 122: /* parameter_declaration: parameter_declarator  */
#line 1125 "MachineIndependent/glslang.y"
                           {
        (yyval.interm) = (yyvsp[0].interm);

        parseContext.parameterTypeCheck((yyvsp[0].interm).loc, EvqIn, *(yyvsp[0].interm).param.type);
        parseContext.paramCheckFixStorage((yyvsp[0].interm).loc, EvqTemporary, *(yyval.interm).param.type);
        parseContext.precisionQualifierCheck((yyval.interm).loc, (yyval.interm).param.type->getBasicType(), (yyval.interm).param.type->getQualifier(), (yyval.interm).param.type->hasTypeParameter());
    }
#line 6736 "MachineIndependent/glslang_tab.cpp"
    break;

  case 123: /* parameter_declaration: type_qualifier parameter_type_specifier  */
#line 1135 "MachineIndependent/glslang.y"
                                              {
        (yyval.interm) = (yyvsp[0].interm);
        if ((yyvsp[-1].interm.type).qualifier.precision != EpqNone)
            (yyval.interm).param.type->getQualifier().precision = (yyvsp[-1].interm.type).qualifier.precision;
        parseContext.precisionQualifierCheck((yyvsp[-1].interm.type).loc, (yyval.interm).param.type->getBasicType(), (yyval.interm).param.type->getQualifier(), (yyval.interm).param.type->hasTypeParameter());

        parseContext.checkNoShaderLayouts((yyvsp[-1].interm.type).loc, (yyvsp[-1].interm.type).shaderQualifiers);
        parseContext.parameterTypeCheck((yyvsp[0].interm).loc, (yyvsp[-1].interm.type).qualifier.storage, *(yyval.interm).param.type);
        parseContext.paramCheckFix((yyvsp[-1].interm.type).loc, (yyvsp[-1].interm.type).qualifier, *(yyval.interm).param.type);
    }
#line 6751 "MachineIndependent/glslang_tab.cpp"
    break;

  case 124: /* parameter_declaration: parameter_type_specifier  */
#line 1145 "MachineIndependent/glslang.y"
                               {
        (yyval.interm) = (yyvsp[0].interm);

        parseContext.parameterTypeCheck((yyvsp[0].interm).loc, EvqIn, *(yyvsp[0].interm).param.type);
        parseContext.paramCheckFixStorage((yyvsp[0].interm).loc, EvqTemporary, *(yyval.interm).param.type);
        parseContext.precisionQualifierCheck((yyval.interm).loc, (yyval.interm).param.type->getBasicType(), (yyval.interm).param.type->getQualifier(), (yyval.interm).param.type->hasTypeParameter());
    }
#line 6763 "MachineIndependent/glslang_tab.cpp"
    break;

  case 125: /* parameter_type_specifier: type_specifier  */
#line 1155 "MachineIndependent/glslang.y"
                     {
        TParameter param = { 0, new TType((yyvsp[0].interm.type)), {} };
        (yyval.interm).param = param;
        if ((yyvsp[0].interm.type).arraySizes)
            parseContext.arraySizeRequiredCheck((yyvsp[0].interm.type).loc, *(yyvsp[0].interm.type).arraySizes);
    }
#line 6774 "MachineIndependent/glslang_tab.cpp"
    break;

  case 126: /* init_declarator_list: single_declaration  */
#line 1164 "MachineIndependent/glslang.y"
                         {
        (yyval.interm) = (yyvsp[0].interm);
    }
#line 6782 "MachineIndependent/glslang_tab.cpp"
    break;

  case 127: /* init_declarator_list: init_declarator_list COMMA IDENTIFIER  */
#line 1167 "MachineIndependent/glslang.y"
                                            {
        (yyval.interm) = (yyvsp[-2].interm);
        TIntermNode* declNode = parseContext.declareVariable((yyvsp[0].lex).loc, *(yyvsp[0].lex).string, (yyvsp[-2].interm).type);
        (yyval.interm).intermNode = parseContext.intermediate.growAggregate((yyvsp[-2].interm).intermNode, declNode, (yyvsp[0].lex).loc);
    }
#line 6792 "MachineIndependent/glslang_tab.cpp"
    break;

  case 128: /* init_declarator_list: init_declarator_list COMMA IDENTIFIER array_specifier  */
#line 1172 "MachineIndependent/glslang.y"
                                                            {
        (yyval.interm) = (yyvsp[-3].interm);
        TIntermNode* declNode = parseContext.declareVariable((yyvsp[-1].lex).loc, *(yyvsp[-1].lex).string, (yyvsp[-3].interm).type, (yyvsp[0].interm).arraySizes);
        (yyval.interm).intermNode = parseContext.intermediate.growAggregate((yyvsp[-3].interm).intermNode, declNode, (yyvsp[-1].lex).loc);
    }
#line 6802 "MachineIndependent/glslang_tab.cpp"
    break;

  case 129: /* init_declarator_list: init_declarator_list COMMA IDENTIFIER array_specifier EQUAL initializer  */
#line 1177 "MachineIndependent/glslang.y"
                                                                              {
        (yyval.interm).type = (yyvsp[-5].interm).type;
        TIntermNode* declNode = parseContext.declareVariable((yyvsp[-3].lex).loc, *(yyvsp[-3].lex).string, (yyvsp[-5].interm).type, (yyvsp[-2].interm).arraySizes, (yyvsp[0].interm.intermTypedNode));
        (yyval.interm).intermNode = parseContext.intermediate.growAggregate((yyvsp[-5].interm).intermNode, declNode, (yyvsp[-1].lex).loc);
    }
#line 6812 "MachineIndependent/glslang_tab.cpp"
    break;

  case 130: /* init_declarator_list: init_declarator_list COMMA IDENTIFIER EQUAL initializer  */
#line 1182 "MachineIndependent/glslang.y"
                                                              {
        (yyval.interm).type = (yyvsp[-4].interm).type;
        TIntermNode* declNode = parseContext.declareVariable((yyvsp[-2].lex).loc, *(yyvsp[-2].lex).string, (yyvsp[-4].interm).type, 0, (yyvsp[0].interm.intermTypedNode));
        (yyval.interm).intermNode = parseContext.intermediate.growAggregate((yyvsp[-4].interm).intermNode, declNode, (yyvsp[-1].lex).loc);
    }
#line 6822 "MachineIndependent/glslang_tab.cpp"
    break;

  case 131: /* single_declaration: fully_specified_type  */
#line 1190 "MachineIndependent/glslang.y"
                           {
        (yyval.interm).type = (yyvsp[0].interm.type);
        (yyval.interm).intermNode = 0;
        parseContext.declareTypeDefaults((yyval.interm).loc, (yyval.interm).type);
    }
#line 6832 "MachineIndependent/glslang_tab.cpp"
    break;

  case 132: /* single_declaration: fully_specified_type IDENTIFIER  */
#line 1195 "MachineIndependent/glslang.y"
                                      {
        (yyval.interm).type = (yyvsp[-1].interm.type);
        TIntermNode* declNode = parseContext.declareVariable((yyvsp[0].lex).loc, *(yyvsp[0].lex).string, (yyvsp[-1].interm.type));
        (yyval.interm).intermNode = parseContext.intermediate.growAggregate(nullptr, declNode, (yyvsp[0].lex).loc);

    }
#line 6843 "MachineIndependent/glslang_tab.cpp"
    break;

  case 133: /* single_declaration: fully_specified_type IDENTIFIER array_specifier  */
#line 1201 "MachineIndependent/glslang.y"
                                                      {
        (yyval.interm).type = (yyvsp[-2].interm.type);
        TIntermNode* declNode = parseContext.declareVariable((yyvsp[-1].lex).loc, *(yyvsp[-1].lex).string, (yyvsp[-2].interm.type), (yyvsp[0].interm).arraySizes);
        (yyval.interm).intermNode = parseContext.intermediate.growAggregate(nullptr, declNode, (yyvsp[-1].lex).loc);
    }
#line 6853 "MachineIndependent/glslang_tab.cpp"
    break;

  case 134: /* single_declaration: fully_specified_type IDENTIFIER array_specifier EQUAL initializer  */
#line 1206 "MachineIndependent/glslang.y"
                                                                        {
        (yyval.interm).type = (yyvsp[-4].interm.type);
        TIntermNode* declNode = parseContext.declareVariable((yyvsp[-3].lex).loc, *(yyvsp[-3].lex).string, (yyvsp[-4].interm.type), (yyvsp[-2].interm).arraySizes, (yyvsp[0].interm.intermTypedNode));
        (yyval.interm).intermNode = parseContext.intermediate.growAggregate(nullptr, declNode, (yyvsp[-3].lex).loc);
    }
#line 6863 "MachineIndependent/glslang_tab.cpp"
    break;

  case 135: /* single_declaration: fully_specified_type IDENTIFIER EQUAL initializer  */
#line 1211 "MachineIndependent/glslang.y"
                                                        {
        (yyval.interm).type = (yyvsp[-3].interm.type);
        TIntermNode* declNode = parseContext.declareVariable((yyvsp[-2].lex).loc, *(yyvsp[-2].lex).string, (yyvsp[-3].interm.type), 0, (yyvsp[0].interm.intermTypedNode));
        (yyval.interm).intermNode = parseContext.intermediate.growAggregate(nullptr, declNode, (yyvsp[-2].lex).loc);
    }
#line 6873 "MachineIndependent/glslang_tab.cpp"
    break;

  case 136: /* fully_specified_type: type_specifier  */
#line 1220 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type) = (yyvsp[0].interm.type);

        parseContext.globalQualifierTypeCheck((yyvsp[0].interm.type).loc, (yyvsp[0].interm.type).qualifier, (yyval.interm.type));
        if ((yyvsp[0].interm.type).arraySizes) {
            parseContext.profileRequires((yyvsp[0].interm.type).loc, ENoProfile, 120, E_GL_3DL_array_objects, "arrayed type");
            parseContext.profileRequires((yyvsp[0].interm.type).loc, EEsProfile, 300, 0, "arrayed type");
        }
        parseContext.precisionQualifierCheck((yyval.interm.type).loc, (yyval.interm.type).basicType, (yyval.interm.type).qualifier, (yyval.interm.type).hasTypeParameter());
    }
#line 6888 "MachineIndependent/glslang_tab.cpp"
    break;

  case 137: /* fully_specified_type: type_qualifier type_specifier  */
#line 1230 "MachineIndependent/glslang.y"
                                     {
        parseContext.globalQualifierFixCheck((yyvsp[-1].interm.type).loc, (yyvsp[-1].interm.type).qualifier, false, &(yyvsp[0].interm.type));
        parseContext.globalQualifierTypeCheck((yyvsp[-1].interm.type).loc, (yyvsp[-1].interm.type).qualifier, (yyvsp[0].interm.type));

        if ((yyvsp[0].interm.type).arraySizes) {
            parseContext.profileRequires((yyvsp[0].interm.type).loc, ENoProfile, 120, E_GL_3DL_array_objects, "arrayed type");
            parseContext.profileRequires((yyvsp[0].interm.type).loc, EEsProfile, 300, 0, "arrayed type");
        }

        if ((yyvsp[0].interm.type).arraySizes && parseContext.arrayQualifierError((yyvsp[0].interm.type).loc, (yyvsp[-1].interm.type).qualifier))
            (yyvsp[0].interm.type).arraySizes = nullptr;

        parseContext.checkNoShaderLayouts((yyvsp[0].interm.type).loc, (yyvsp[-1].interm.type).shaderQualifiers);
        (yyvsp[0].interm.type).shaderQualifiers.merge((yyvsp[-1].interm.type).shaderQualifiers);
        parseContext.mergeQualifiers((yyvsp[0].interm.type).loc, (yyvsp[0].interm.type).qualifier, (yyvsp[-1].interm.type).qualifier, true);
        parseContext.precisionQualifierCheck((yyvsp[0].interm.type).loc, (yyvsp[0].interm.type).basicType, (yyvsp[0].interm.type).qualifier, (yyvsp[0].interm.type).hasTypeParameter());

        (yyval.interm.type) = (yyvsp[0].interm.type);

        if (! (yyval.interm.type).qualifier.isInterpolation() &&
            ((parseContext.language == EShLangVertex   && (yyval.interm.type).qualifier.storage == EvqVaryingOut) ||
             (parseContext.language == EShLangFragment && (yyval.interm.type).qualifier.storage == EvqVaryingIn)))
            (yyval.interm.type).qualifier.smooth = true;
    }
#line 6917 "MachineIndependent/glslang_tab.cpp"
    break;

  case 138: /* invariant_qualifier: INVARIANT  */
#line 1257 "MachineIndependent/glslang.y"
                {
        parseContext.globalCheck((yyvsp[0].lex).loc, "invariant");
        parseContext.profileRequires((yyval.interm.type).loc, ENoProfile, 120, 0, "invariant");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.invariant = true;
    }
#line 6928 "MachineIndependent/glslang_tab.cpp"
    break;

  case 139: /* interpolation_qualifier: SMOOTH  */
#line 1266 "MachineIndependent/glslang.y"
             {
        parseContext.globalCheck((yyvsp[0].lex).loc, "smooth");
        parseContext.profileRequires((yyvsp[0].lex).loc, ENoProfile, 130, 0, "smooth");
        parseContext.profileRequires((yyvsp[0].lex).loc, EEsProfile, 300, 0, "smooth");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.smooth = true;
    }
#line 6940 "MachineIndependent/glslang_tab.cpp"
    break;

  case 140: /* interpolation_qualifier: FLAT  */
#line 1273 "MachineIndependent/glslang.y"
           {
        parseContext.globalCheck((yyvsp[0].lex).loc, "flat");
        parseContext.profileRequires((yyvsp[0].lex).loc, ENoProfile, 130, 0, "flat");
        parseContext.profileRequires((yyvsp[0].lex).loc, EEsProfile, 300, 0, "flat");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.flat = true;
    }
#line 6952 "MachineIndependent/glslang_tab.cpp"
    break;

  case 141: /* interpolation_qualifier: NOPERSPECTIVE  */
#line 1280 "MachineIndependent/glslang.y"
                    {
        parseContext.globalCheck((yyvsp[0].lex).loc, "noperspective");
        parseContext.profileRequires((yyvsp[0].lex).loc, EEsProfile, 0, E_GL_NV_shader_noperspective_interpolation, "noperspective");
        parseContext.profileRequires((yyvsp[0].lex).loc, ENoProfile, 130, 0, "noperspective");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.nopersp = true;
    }
#line 6964 "MachineIndependent/glslang_tab.cpp"
    break;

  case 142: /* interpolation_qualifier: EXPLICITINTERPAMD  */
#line 1287 "MachineIndependent/glslang.y"
                        {
        parseContext.globalCheck((yyvsp[0].lex).loc, "__explicitInterpAMD");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 450, E_GL_AMD_shader_explicit_vertex_parameter, "explicit interpolation");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECompatibilityProfile, 450, E_GL_AMD_shader_explicit_vertex_parameter, "explicit interpolation");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.explicitInterp = true;
    }
#line 6976 "MachineIndependent/glslang_tab.cpp"
    break;

  case 143: /* interpolation_qualifier: PERVERTEXNV  */
#line 1294 "MachineIndependent/glslang.y"
                  {
        parseContext.globalCheck((yyvsp[0].lex).loc, "pervertexNV");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 0, E_GL_NV_fragment_shader_barycentric, "fragment shader barycentric");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECompatibilityProfile, 0, E_GL_NV_fragment_shader_barycentric, "fragment shader barycentric");
        parseContext.profileRequires((yyvsp[0].lex).loc, EEsProfile, 0, E_GL_NV_fragment_shader_barycentric, "fragment shader barycentric");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.pervertexNV = true;
    }
#line 6989 "MachineIndependent/glslang_tab.cpp"
    break;

  case 144: /* interpolation_qualifier: PERVERTEXEXT  */
#line 1302 "MachineIndependent/glslang.y"
                   {
        parseContext.globalCheck((yyvsp[0].lex).loc, "pervertexEXT");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 0, E_GL_EXT_fragment_shader_barycentric, "fragment shader barycentric");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECompatibilityProfile, 0, E_GL_EXT_fragment_shader_barycentric, "fragment shader barycentric");
        parseContext.profileRequires((yyvsp[0].lex).loc, EEsProfile, 0, E_GL_EXT_fragment_shader_barycentric, "fragment shader barycentric");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.pervertexEXT = true;
    }
#line 7002 "MachineIndependent/glslang_tab.cpp"
    break;

  case 145: /* interpolation_qualifier: PERPRIMITIVENV  */
#line 1310 "MachineIndependent/glslang.y"
                     {
        // No need for profile version or extension check. Shader stage already checks both.
        parseContext.globalCheck((yyvsp[0].lex).loc, "perprimitiveNV");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangFragmentMask | EShLangMeshMask), "perprimitiveNV");
        // Fragment shader stage doesn't check for extension. So we explicitly add below extension check.
        if (parseContext.language == EShLangFragment)
            parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_NV_mesh_shader, "perprimitiveNV");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.perPrimitiveNV = true;
    }
#line 7017 "MachineIndependent/glslang_tab.cpp"
    break;

  case 146: /* interpolation_qualifier: PERPRIMITIVEEXT  */
#line 1320 "MachineIndependent/glslang.y"
                      {
        // No need for profile version or extension check. Shader stage already checks both.
        parseContext.globalCheck((yyvsp[0].lex).loc, "perprimitiveEXT");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangFragmentMask | EShLangMeshMask), "perprimitiveEXT");
        // Fragment shader stage doesn't check for extension. So we explicitly add below extension check.
        if (parseContext.language == EShLangFragment)
            parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_EXT_mesh_shader, "perprimitiveEXT");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.perPrimitiveNV = true;
    }
#line 7032 "MachineIndependent/glslang_tab.cpp"
    break;

  case 147: /* interpolation_qualifier: PERVIEWNV  */
#line 1330 "MachineIndependent/glslang.y"
                {
        // No need for profile version or extension check. Shader stage already checks both.
        parseContext.globalCheck((yyvsp[0].lex).loc, "perviewNV");
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangMesh, "perviewNV");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.perViewNV = true;
    }
#line 7044 "MachineIndependent/glslang_tab.cpp"
    break;

  case 148: /* interpolation_qualifier: PERTASKNV  */
#line 1337 "MachineIndependent/glslang.y"
                {
        // No need for profile version or extension check. Shader stage already checks both.
        parseContext.globalCheck((yyvsp[0].lex).loc, "taskNV");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangTaskMask | EShLangMeshMask), "taskNV");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.perTaskNV = true;
    }
#line 7056 "MachineIndependent/glslang_tab.cpp"
    break;

  case 149: /* layout_qualifier: LAYOUT LEFT_PAREN layout_qualifier_id_list RIGHT_PAREN  */
#line 1347 "MachineIndependent/glslang.y"
                                                             {
        (yyval.interm.type) = (yyvsp[-1].interm.type);
    }
#line 7064 "MachineIndependent/glslang_tab.cpp"
    break;

  case 150: /* layout_qualifier_id_list: layout_qualifier_id  */
#line 1353 "MachineIndependent/glslang.y"
                          {
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7072 "MachineIndependent/glslang_tab.cpp"
    break;

  case 151: /* layout_qualifier_id_list: layout_qualifier_id_list COMMA layout_qualifier_id  */
#line 1356 "MachineIndependent/glslang.y"
                                                         {
        (yyval.interm.type) = (yyvsp[-2].interm.type);
        (yyval.interm.type).shaderQualifiers.merge((yyvsp[0].interm.type).shaderQualifiers);
        parseContext.mergeObjectLayoutQualifiers((yyval.interm.type).qualifier, (yyvsp[0].interm.type).qualifier, false);
    }
#line 7082 "MachineIndependent/glslang_tab.cpp"
    break;

  case 152: /* layout_qualifier_id: IDENTIFIER  */
#line 1363 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        parseContext.setLayoutQualifier((yyvsp[0].lex).loc, (yyval.interm.type), *(yyvsp[0].lex).string);
    }
#line 7091 "MachineIndependent/glslang_tab.cpp"
    break;

  case 153: /* layout_qualifier_id: IDENTIFIER EQUAL constant_expression  */
#line 1367 "MachineIndependent/glslang.y"
                                           {
        (yyval.interm.type).init((yyvsp[-2].lex).loc);
        parseContext.setLayoutQualifier((yyvsp[-2].lex).loc, (yyval.interm.type), *(yyvsp[-2].lex).string, (yyvsp[0].interm.intermTypedNode));
    }
#line 7100 "MachineIndependent/glslang_tab.cpp"
    break;

  case 154: /* layout_qualifier_id: SHARED  */
#line 1371 "MachineIndependent/glslang.y"
             { // because "shared" is both an identifier and a keyword
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        TString strShared("shared");
        parseContext.setLayoutQualifier((yyvsp[0].lex).loc, (yyval.interm.type), strShared);
    }
#line 7110 "MachineIndependent/glslang_tab.cpp"
    break;

  case 155: /* precise_qualifier: PRECISE  */
#line 1379 "MachineIndependent/glslang.y"
              {
        parseContext.profileRequires((yyval.interm.type).loc, ECoreProfile | ECompatibilityProfile, 400, E_GL_ARB_gpu_shader5, "precise");
        parseContext.profileRequires((yyvsp[0].lex).loc, EEsProfile, 320, Num_AEP_gpu_shader5, AEP_gpu_shader5, "precise");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.noContraction = true;
    }
#line 7121 "MachineIndependent/glslang_tab.cpp"
    break;

  case 156: /* type_qualifier: single_type_qualifier  */
#line 1388 "MachineIndependent/glslang.y"
                            {
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7129 "MachineIndependent/glslang_tab.cpp"
    break;

  case 157: /* type_qualifier: type_qualifier single_type_qualifier  */
#line 1391 "MachineIndependent/glslang.y"
                                           {
        (yyval.interm.type) = (yyvsp[-1].interm.type);
        if ((yyval.interm.type).basicType == EbtVoid)
            (yyval.interm.type).basicType = (yyvsp[0].interm.type).basicType;

        (yyval.interm.type).shaderQualifiers.merge((yyvsp[0].interm.type).shaderQualifiers);
        parseContext.mergeQualifiers((yyval.interm.type).loc, (yyval.interm.type).qualifier, (yyvsp[0].interm.type).qualifier, false);
    }
#line 7142 "MachineIndependent/glslang_tab.cpp"
    break;

  case 158: /* single_type_qualifier: storage_qualifier  */
#line 1402 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7150 "MachineIndependent/glslang_tab.cpp"
    break;

  case 159: /* single_type_qualifier: layout_qualifier  */
#line 1405 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7158 "MachineIndependent/glslang_tab.cpp"
    break;

  case 160: /* single_type_qualifier: precision_qualifier  */
#line 1408 "MachineIndependent/glslang.y"
                          {
        parseContext.checkPrecisionQualifier((yyvsp[0].interm.type).loc, (yyvsp[0].interm.type).qualifier.precision);
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7167 "MachineIndependent/glslang_tab.cpp"
    break;

  case 161: /* single_type_qualifier: interpolation_qualifier  */
#line 1412 "MachineIndependent/glslang.y"
                              {
        // allow inheritance of storage qualifier from block declaration
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7176 "MachineIndependent/glslang_tab.cpp"
    break;

  case 162: /* single_type_qualifier: invariant_qualifier  */
#line 1416 "MachineIndependent/glslang.y"
                          {
        // allow inheritance of storage qualifier from block declaration
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7185 "MachineIndependent/glslang_tab.cpp"
    break;

  case 163: /* single_type_qualifier: precise_qualifier  */
#line 1420 "MachineIndependent/glslang.y"
                        {
        // allow inheritance of storage qualifier from block declaration
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7194 "MachineIndependent/glslang_tab.cpp"
    break;

  case 164: /* single_type_qualifier: non_uniform_qualifier  */
#line 1424 "MachineIndependent/glslang.y"
                            {
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7202 "MachineIndependent/glslang_tab.cpp"
    break;

  case 165: /* single_type_qualifier: spirv_storage_class_qualifier  */
#line 1427 "MachineIndependent/glslang.y"
                                    {
        parseContext.globalCheck((yyvsp[0].interm.type).loc, "spirv_storage_class");
        parseContext.requireExtensions((yyvsp[0].interm.type).loc, 1, &E_GL_EXT_spirv_intrinsics, "SPIR-V storage class qualifier");
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7212 "MachineIndependent/glslang_tab.cpp"
    break;

  case 166: /* single_type_qualifier: spirv_decorate_qualifier  */
#line 1432 "MachineIndependent/glslang.y"
                               {
        parseContext.requireExtensions((yyvsp[0].interm.type).loc, 1, &E_GL_EXT_spirv_intrinsics, "SPIR-V decorate qualifier");
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 7221 "MachineIndependent/glslang_tab.cpp"
    break;

  case 167: /* single_type_qualifier: SPIRV_BY_REFERENCE  */
#line 1436 "MachineIndependent/glslang.y"
                         {
        parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_EXT_spirv_intrinsics, "spirv_by_reference");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.setSpirvByReference();
    }
#line 7231 "MachineIndependent/glslang_tab.cpp"
    break;

  case 168: /* single_type_qualifier: SPIRV_LITERAL  */
#line 1441 "MachineIndependent/glslang.y"
                    {
        parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_EXT_spirv_intrinsics, "spirv_by_literal");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.setSpirvLiteral();
    }
#line 7241 "MachineIndependent/glslang_tab.cpp"
    break;

  case 169: /* storage_qualifier: CONST  */
#line 1449 "MachineIndependent/glslang.y"
            {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqConst;  // will later turn into EvqConstReadOnly, if the initializer is not constant
    }
#line 7250 "MachineIndependent/glslang_tab.cpp"
    break;

  case 170: /* storage_qualifier: INOUT  */
#line 1453 "MachineIndependent/glslang.y"
            {
        parseContext.globalCheck((yyvsp[0].lex).loc, "inout");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqInOut;
    }
#line 7260 "MachineIndependent/glslang_tab.cpp"
    break;

  case 171: /* storage_qualifier: IN  */
#line 1458 "MachineIndependent/glslang.y"
         {
        parseContext.globalCheck((yyvsp[0].lex).loc, "in");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        // whether this is a parameter "in" or a pipeline "in" will get sorted out a bit later
        (yyval.interm.type).qualifier.storage = EvqIn;
    }
#line 7271 "MachineIndependent/glslang_tab.cpp"
    break;

  case 172: /* storage_qualifier: OUT  */
#line 1464 "MachineIndependent/glslang.y"
          {
        parseContext.globalCheck((yyvsp[0].lex).loc, "out");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        // whether this is a parameter "out" or a pipeline "out" will get sorted out a bit later
        (yyval.interm.type).qualifier.storage = EvqOut;
    }
#line 7282 "MachineIndependent/glslang_tab.cpp"
    break;

  case 173: /* storage_qualifier: CENTROID  */
#line 1470 "MachineIndependent/glslang.y"
               {
        parseContext.profileRequires((yyvsp[0].lex).loc, ENoProfile, 120, 0, "centroid");
        parseContext.profileRequires((yyvsp[0].lex).loc, EEsProfile, 300, 0, "centroid");
        parseContext.globalCheck((yyvsp[0].lex).loc, "centroid");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.centroid = true;
    }
#line 7294 "MachineIndependent/glslang_tab.cpp"
    break;

  case 174: /* storage_qualifier: UNIFORM  */
#line 1477 "MachineIndependent/glslang.y"
              {
        parseContext.globalCheck((yyvsp[0].lex).loc, "uniform");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqUniform;
    }
#line 7304 "MachineIndependent/glslang_tab.cpp"
    break;

  case 175: /* storage_qualifier: TILEIMAGEEXT  */
#line 1482 "MachineIndependent/glslang.y"
                   {
        parseContext.globalCheck((yyvsp[0].lex).loc, "tileImageEXT");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqTileImageEXT;
    }
#line 7314 "MachineIndependent/glslang_tab.cpp"
    break;

  case 176: /* storage_qualifier: SHARED  */
#line 1487 "MachineIndependent/glslang.y"
             {
        parseContext.globalCheck((yyvsp[0].lex).loc, "shared");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, 430, E_GL_ARB_compute_shader, "shared");
        parseContext.profileRequires((yyvsp[0].lex).loc, EEsProfile, 310, 0, "shared");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangComputeMask | EShLangMeshMask | EShLangTaskMask), "shared");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqShared;
    }
#line 7327 "MachineIndependent/glslang_tab.cpp"
    break;

  case 177: /* storage_qualifier: BUFFER  */
#line 1495 "MachineIndependent/glslang.y"
             {
        parseContext.globalCheck((yyvsp[0].lex).loc, "buffer");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqBuffer;
    }
#line 7337 "MachineIndependent/glslang_tab.cpp"
    break;

  case 178: /* storage_qualifier: ATTRIBUTE  */
#line 1500 "MachineIndependent/glslang.y"
                {
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangVertex, "attribute");
        parseContext.checkDeprecated((yyvsp[0].lex).loc, ECoreProfile, 130, "attribute");
        parseContext.checkDeprecated((yyvsp[0].lex).loc, ENoProfile, 130, "attribute");
        parseContext.requireNotRemoved((yyvsp[0].lex).loc, ECoreProfile, 420, "attribute");
        parseContext.requireNotRemoved((yyvsp[0].lex).loc, EEsProfile, 300, "attribute");

        parseContext.globalCheck((yyvsp[0].lex).loc, "attribute");

        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqVaryingIn;
    }
#line 7354 "MachineIndependent/glslang_tab.cpp"
    break;

  case 179: /* storage_qualifier: VARYING  */
#line 1512 "MachineIndependent/glslang.y"
              {
        parseContext.checkDeprecated((yyvsp[0].lex).loc, ENoProfile, 130, "varying");
        parseContext.checkDeprecated((yyvsp[0].lex).loc, ECoreProfile, 130, "varying");
        parseContext.requireNotRemoved((yyvsp[0].lex).loc, ECoreProfile, 420, "varying");
        parseContext.requireNotRemoved((yyvsp[0].lex).loc, EEsProfile, 300, "varying");

        parseContext.globalCheck((yyvsp[0].lex).loc, "varying");

        (yyval.interm.type).init((yyvsp[0].lex).loc);
        if (parseContext.language == EShLangVertex)
            (yyval.interm.type).qualifier.storage = EvqVaryingOut;
        else
            (yyval.interm.type).qualifier.storage = EvqVaryingIn;
    }
#line 7373 "MachineIndependent/glslang_tab.cpp"
    break;

  case 180: /* storage_qualifier: PATCH  */
#line 1526 "MachineIndependent/glslang.y"
            {
        parseContext.globalCheck((yyvsp[0].lex).loc, "patch");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangTessControlMask | EShLangTessEvaluationMask), "patch");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.patch = true;
    }
#line 7384 "MachineIndependent/glslang_tab.cpp"
    break;

  case 181: /* storage_qualifier: SAMPLE  */
#line 1532 "MachineIndependent/glslang.y"
             {
        parseContext.globalCheck((yyvsp[0].lex).loc, "sample");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.sample = true;
    }
#line 7394 "MachineIndependent/glslang_tab.cpp"
    break;

  case 182: /* storage_qualifier: RESOURCEHEAP  */
#line 1537 "MachineIndependent/glslang.y"
                   {
        parseContext.globalCheck((yyvsp[0].lex).loc, "resourceHeap");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqResourceHeap;
    }
#line 7404 "MachineIndependent/glslang_tab.cpp"
    break;

  case 183: /* storage_qualifier: SAMPLERHEAP  */
#line 1542 "MachineIndependent/glslang.y"
                  {
        parseContext.globalCheck((yyvsp[0].lex).loc, "samplerHeap");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqSamplerHeap;
    }
#line 7414 "MachineIndependent/glslang_tab.cpp"
    break;

  case 184: /* storage_qualifier: HITATTRNV  */
#line 1547 "MachineIndependent/glslang.y"
                {
        parseContext.globalCheck((yyvsp[0].lex).loc, "hitAttributeNV");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangIntersectMask | EShLangClosestHitMask
            | EShLangAnyHitMask), "hitAttributeNV");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_NV_ray_tracing, "hitAttributeNV");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqHitAttr;
    }
#line 7427 "MachineIndependent/glslang_tab.cpp"
    break;

  case 185: /* storage_qualifier: HITOBJECTATTRNV  */
#line 1555 "MachineIndependent/glslang.y"
                      {
        parseContext.globalCheck((yyvsp[0].lex).loc, "hitAttributeNV");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangRayGenMask | EShLangClosestHitMask
            | EShLangMissMask), "hitObjectAttributeNV");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_NV_shader_invocation_reorder, "hitObjectAttributeNV");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqHitObjectAttrNV;
    }
#line 7440 "MachineIndependent/glslang_tab.cpp"
    break;

  case 186: /* storage_qualifier: HITOBJECTATTREXT  */
#line 1563 "MachineIndependent/glslang.y"
                       {
        parseContext.globalCheck((yyvsp[0].lex).loc, "hitAttributeEXT");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangRayGenMask | EShLangClosestHitMask
            | EShLangMissMask), "hitObjectAttributeEXT");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_EXT_shader_invocation_reorder, "hitObjectAttributeEXT");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqHitObjectAttrEXT;
    }
#line 7453 "MachineIndependent/glslang_tab.cpp"
    break;

  case 187: /* storage_qualifier: HITATTREXT  */
#line 1571 "MachineIndependent/glslang.y"
                 {
        parseContext.globalCheck((yyvsp[0].lex).loc, "hitAttributeEXT");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangIntersectMask | EShLangClosestHitMask
            | EShLangAnyHitMask), "hitAttributeEXT");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_EXT_ray_tracing, "hitAttributeNV");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqHitAttr;
    }
#line 7466 "MachineIndependent/glslang_tab.cpp"
    break;

  case 188: /* storage_qualifier: PAYLOADNV  */
#line 1579 "MachineIndependent/glslang.y"
                {
        parseContext.globalCheck((yyvsp[0].lex).loc, "rayPayloadNV");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangRayGenMask | EShLangClosestHitMask |
            EShLangAnyHitMask | EShLangMissMask), "rayPayloadNV");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_NV_ray_tracing, "rayPayloadNV");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqPayload;
    }
#line 7479 "MachineIndependent/glslang_tab.cpp"
    break;

  case 189: /* storage_qualifier: PAYLOADEXT  */
#line 1587 "MachineIndependent/glslang.y"
                 {
        parseContext.globalCheck((yyvsp[0].lex).loc, "rayPayloadEXT");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangRayGenMask | EShLangClosestHitMask |
            EShLangAnyHitMask | EShLangMissMask), "rayPayloadEXT");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_EXT_ray_tracing, "rayPayloadEXT");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqPayload;
    }
#line 7492 "MachineIndependent/glslang_tab.cpp"
    break;

  case 190: /* storage_qualifier: PAYLOADINNV  */
#line 1595 "MachineIndependent/glslang.y"
                  {
        parseContext.globalCheck((yyvsp[0].lex).loc, "rayPayloadInNV");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangClosestHitMask |
            EShLangAnyHitMask | EShLangMissMask), "rayPayloadInNV");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_NV_ray_tracing, "rayPayloadInNV");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqPayloadIn;
    }
#line 7505 "MachineIndependent/glslang_tab.cpp"
    break;

  case 191: /* storage_qualifier: PAYLOADINEXT  */
#line 1603 "MachineIndependent/glslang.y"
                   {
        parseContext.globalCheck((yyvsp[0].lex).loc, "rayPayloadInEXT");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangClosestHitMask |
            EShLangAnyHitMask | EShLangMissMask), "rayPayloadInEXT");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_EXT_ray_tracing, "rayPayloadInEXT");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqPayloadIn;
    }
#line 7518 "MachineIndependent/glslang_tab.cpp"
    break;

  case 192: /* storage_qualifier: CALLDATANV  */
#line 1611 "MachineIndependent/glslang.y"
                 {
        parseContext.globalCheck((yyvsp[0].lex).loc, "callableDataNV");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangRayGenMask |
            EShLangClosestHitMask | EShLangMissMask | EShLangCallableMask), "callableDataNV");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_NV_ray_tracing, "callableDataNV");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqCallableData;
    }
#line 7531 "MachineIndependent/glslang_tab.cpp"
    break;

  case 193: /* storage_qualifier: CALLDATAEXT  */
#line 1619 "MachineIndependent/glslang.y"
                  {
        parseContext.globalCheck((yyvsp[0].lex).loc, "callableDataEXT");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangRayGenMask |
            EShLangClosestHitMask | EShLangMissMask | EShLangCallableMask), "callableDataEXT");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_EXT_ray_tracing, "callableDataEXT");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqCallableData;
    }
#line 7544 "MachineIndependent/glslang_tab.cpp"
    break;

  case 194: /* storage_qualifier: CALLDATAINNV  */
#line 1627 "MachineIndependent/glslang.y"
                   {
        parseContext.globalCheck((yyvsp[0].lex).loc, "callableDataInNV");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangCallableMask), "callableDataInNV");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_NV_ray_tracing, "callableDataInNV");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqCallableDataIn;
    }
#line 7556 "MachineIndependent/glslang_tab.cpp"
    break;

  case 195: /* storage_qualifier: CALLDATAINEXT  */
#line 1634 "MachineIndependent/glslang.y"
                    {
        parseContext.globalCheck((yyvsp[0].lex).loc, "callableDataInEXT");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangCallableMask), "callableDataInEXT");
        parseContext.profileRequires((yyvsp[0].lex).loc, ECoreProfile, 460, E_GL_EXT_ray_tracing, "callableDataInEXT");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqCallableDataIn;
    }
#line 7568 "MachineIndependent/glslang_tab.cpp"
    break;

  case 196: /* storage_qualifier: COHERENT  */
#line 1641 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.coherent = true;
    }
#line 7577 "MachineIndependent/glslang_tab.cpp"
    break;

  case 197: /* storage_qualifier: DEVICECOHERENT  */
#line 1645 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_KHR_memory_scope_semantics, "devicecoherent");
        (yyval.interm.type).qualifier.devicecoherent = true;
    }
#line 7587 "MachineIndependent/glslang_tab.cpp"
    break;

  case 198: /* storage_qualifier: QUEUEFAMILYCOHERENT  */
#line 1650 "MachineIndependent/glslang.y"
                          {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_KHR_memory_scope_semantics, "queuefamilycoherent");
        (yyval.interm.type).qualifier.queuefamilycoherent = true;
    }
#line 7597 "MachineIndependent/glslang_tab.cpp"
    break;

  case 199: /* storage_qualifier: WORKGROUPCOHERENT  */
#line 1655 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_KHR_memory_scope_semantics, "workgroupcoherent");
        (yyval.interm.type).qualifier.workgroupcoherent = true;
    }
#line 7607 "MachineIndependent/glslang_tab.cpp"
    break;

  case 200: /* storage_qualifier: SUBGROUPCOHERENT  */
#line 1660 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_KHR_memory_scope_semantics, "subgroupcoherent");
        (yyval.interm.type).qualifier.subgroupcoherent = true;
    }
#line 7617 "MachineIndependent/glslang_tab.cpp"
    break;

  case 201: /* storage_qualifier: NONPRIVATE  */
#line 1665 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_KHR_memory_scope_semantics, "nonprivate");
        (yyval.interm.type).qualifier.nonprivate = true;
    }
#line 7627 "MachineIndependent/glslang_tab.cpp"
    break;

  case 202: /* storage_qualifier: SHADERCALLCOHERENT  */
#line 1670 "MachineIndependent/glslang.y"
                         {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        parseContext.requireExtensions((yyvsp[0].lex).loc, 1, &E_GL_EXT_ray_tracing, "shadercallcoherent");
        (yyval.interm.type).qualifier.shadercallcoherent = true;
    }
#line 7637 "MachineIndependent/glslang_tab.cpp"
    break;

  case 203: /* storage_qualifier: VOLATILE  */
#line 1675 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.volatil = true;
    }
#line 7646 "MachineIndependent/glslang_tab.cpp"
    break;

  case 204: /* storage_qualifier: RESTRICT  */
#line 1679 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.restrict = true;
    }
#line 7655 "MachineIndependent/glslang_tab.cpp"
    break;

  case 205: /* storage_qualifier: READONLY  */
#line 1683 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.readonly = true;
    }
#line 7664 "MachineIndependent/glslang_tab.cpp"
    break;

  case 206: /* storage_qualifier: WRITEONLY  */
#line 1687 "MachineIndependent/glslang.y"
                {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.writeonly = true;
    }
#line 7673 "MachineIndependent/glslang_tab.cpp"
    break;

  case 207: /* storage_qualifier: NONTEMPORAL  */
#line 1691 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.nontemporal  = true;
    }
#line 7682 "MachineIndependent/glslang_tab.cpp"
    break;

  case 208: /* storage_qualifier: SUBROUTINE  */
#line 1695 "MachineIndependent/glslang.y"
                 {
        parseContext.spvRemoved((yyvsp[0].lex).loc, "subroutine");
        parseContext.globalCheck((yyvsp[0].lex).loc, "subroutine");
        parseContext.unimplemented((yyvsp[0].lex).loc, "subroutine");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
    }
#line 7693 "MachineIndependent/glslang_tab.cpp"
    break;

  case 209: /* storage_qualifier: SUBROUTINE LEFT_PAREN type_name_list RIGHT_PAREN  */
#line 1701 "MachineIndependent/glslang.y"
                                                       {
        parseContext.spvRemoved((yyvsp[-3].lex).loc, "subroutine");
        parseContext.globalCheck((yyvsp[-3].lex).loc, "subroutine");
        parseContext.unimplemented((yyvsp[-3].lex).loc, "subroutine");
        (yyval.interm.type).init((yyvsp[-3].lex).loc);
    }
#line 7704 "MachineIndependent/glslang_tab.cpp"
    break;

  case 210: /* storage_qualifier: TASKPAYLOADWORKGROUPEXT  */
#line 1707 "MachineIndependent/glslang.y"
                              {
        // No need for profile version or extension check. Shader stage already checks both.
        parseContext.globalCheck((yyvsp[0].lex).loc, "taskPayloadSharedEXT");
        parseContext.requireStage((yyvsp[0].lex).loc, (EShLanguageMask)(EShLangTaskMask | EShLangMeshMask), "taskPayloadSharedEXT  ");
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqtaskPayloadSharedEXT;
    }
#line 7716 "MachineIndependent/glslang_tab.cpp"
    break;

  case 211: /* non_uniform_qualifier: NONUNIFORM  */
#line 1717 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).qualifier.nonUniform = true;
    }
#line 7725 "MachineIndependent/glslang_tab.cpp"
    break;

  case 212: /* type_name_list: IDENTIFIER  */
#line 1724 "MachineIndependent/glslang.y"
                 {
        // TODO
    }
#line 7733 "MachineIndependent/glslang_tab.cpp"
    break;

  case 213: /* type_name_list: type_name_list COMMA IDENTIFIER  */
#line 1727 "MachineIndependent/glslang.y"
                                      {
        // TODO: 4.0 semantics: subroutines
        // 1) make sure each identifier is a type declared earlier with SUBROUTINE
        // 2) save all of the identifiers for future comparison with the declared function
    }
#line 7743 "MachineIndependent/glslang_tab.cpp"
    break;

  case 214: /* type_specifier: type_specifier_nonarray type_parameter_specifier_opt  */
#line 1735 "MachineIndependent/glslang.y"
                                                           {
        (yyval.interm.type) = (yyvsp[-1].interm.type);
        (yyval.interm.type).qualifier.precision = parseContext.getDefaultPrecision((yyval.interm.type));
        (yyval.interm.type).typeParameters = (yyvsp[0].interm.typeParameters);
        parseContext.typeParametersCheck((yyvsp[-1].interm.type).loc, (yyval.interm.type));

    }
#line 7755 "MachineIndependent/glslang_tab.cpp"
    break;

  case 215: /* type_specifier: type_specifier_nonarray type_parameter_specifier_opt array_specifier  */
#line 1742 "MachineIndependent/glslang.y"
                                                                           {
        parseContext.arrayOfArrayVersionCheck((yyvsp[0].interm).loc, (yyvsp[0].interm).arraySizes);
        (yyval.interm.type) = (yyvsp[-2].interm.type);
        (yyval.interm.type).qualifier.precision = parseContext.getDefaultPrecision((yyval.interm.type));
        (yyval.interm.type).typeParameters = (yyvsp[-1].interm.typeParameters);
        (yyval.interm.type).arraySizes = (yyvsp[0].interm).arraySizes;
        parseContext.typeParametersCheck((yyvsp[-2].interm.type).loc, (yyval.interm.type));
    }
#line 7768 "MachineIndependent/glslang_tab.cpp"
    break;

  case 216: /* array_specifier: LEFT_BRACKET RIGHT_BRACKET  */
#line 1753 "MachineIndependent/glslang.y"
                                 {
        (yyval.interm).loc = (yyvsp[-1].lex).loc;
        (yyval.interm).arraySizes = new TArraySizes;
        (yyval.interm).arraySizes->addInnerSize();
    }
#line 7778 "MachineIndependent/glslang_tab.cpp"
    break;

  case 217: /* array_specifier: LEFT_BRACKET conditional_expression RIGHT_BRACKET  */
#line 1758 "MachineIndependent/glslang.y"
                                                        {
        (yyval.interm).loc = (yyvsp[-2].lex).loc;
        (yyval.interm).arraySizes = new TArraySizes;

        TArraySize size;
        parseContext.arraySizeCheck((yyvsp[-1].interm.intermTypedNode)->getLoc(), (yyvsp[-1].interm.intermTypedNode), size, "array size");
        (yyval.interm).arraySizes->addInnerSize(size);
    }
#line 7791 "MachineIndependent/glslang_tab.cpp"
    break;

  case 218: /* array_specifier: array_specifier LEFT_BRACKET RIGHT_BRACKET  */
#line 1766 "MachineIndependent/glslang.y"
                                                 {
        (yyval.interm) = (yyvsp[-2].interm);
        (yyval.interm).arraySizes->addInnerSize();
    }
#line 7800 "MachineIndependent/glslang_tab.cpp"
    break;

  case 219: /* array_specifier: array_specifier LEFT_BRACKET conditional_expression RIGHT_BRACKET  */
#line 1770 "MachineIndependent/glslang.y"
                                                                        {
        (yyval.interm) = (yyvsp[-3].interm);

        TArraySize size;
        parseContext.arraySizeCheck((yyvsp[-1].interm.intermTypedNode)->getLoc(), (yyvsp[-1].interm.intermTypedNode), size, "array size");
        (yyval.interm).arraySizes->addInnerSize(size);
    }
#line 7812 "MachineIndependent/glslang_tab.cpp"
    break;

  case 220: /* type_parameter_specifier_opt: type_parameter_specifier  */
#line 1780 "MachineIndependent/glslang.y"
                               {
        (yyval.interm.typeParameters) = (yyvsp[0].interm.typeParameters);
    }
#line 7820 "MachineIndependent/glslang_tab.cpp"
    break;

  case 221: /* type_parameter_specifier_opt: %empty  */
#line 1783 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.typeParameters) = 0;
    }
#line 7828 "MachineIndependent/glslang_tab.cpp"
    break;

  case 222: /* type_parameter_specifier: LEFT_ANGLE type_parameter_specifier_list RIGHT_ANGLE  */
#line 1789 "MachineIndependent/glslang.y"
                                                           {
        (yyval.interm.typeParameters) = (yyvsp[-1].interm.typeParameters);
    }
#line 7836 "MachineIndependent/glslang_tab.cpp"
    break;

  case 223: /* type_parameter_specifier_list: type_specifier  */
#line 1795 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.typeParameters) = new TTypeParameters;
        (yyval.interm.typeParameters)->arraySizes = new TArraySizes;
        (yyval.interm.typeParameters)->spirvType = (yyvsp[0].interm.type).spirvType;
        (yyval.interm.typeParameters)->basicType = (yyvsp[0].interm.type).basicType;
    }
#line 7847 "MachineIndependent/glslang_tab.cpp"
    break;

  case 224: /* type_parameter_specifier_list: unary_expression  */
#line 1801 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.typeParameters) = new TTypeParameters;
        (yyval.interm.typeParameters)->arraySizes = new TArraySizes;

        TArraySize size;
        parseContext.arraySizeCheck((yyvsp[0].interm.intermTypedNode)->getLoc(), (yyvsp[0].interm.intermTypedNode), size, "type parameter", true);
        (yyval.interm.typeParameters)->arraySizes->addInnerSize(size);
    }
#line 7860 "MachineIndependent/glslang_tab.cpp"
    break;

  case 225: /* type_parameter_specifier_list: type_parameter_specifier_list COMMA unary_expression  */
#line 1809 "MachineIndependent/glslang.y"
                                                           {
        (yyval.interm.typeParameters) = (yyvsp[-2].interm.typeParameters);

        TArraySize size;
        parseContext.arraySizeCheck((yyvsp[0].interm.intermTypedNode)->getLoc(), (yyvsp[0].interm.intermTypedNode), size, "type parameter", true);
        (yyval.interm.typeParameters)->arraySizes->addInnerSize(size);
    }
#line 7872 "MachineIndependent/glslang_tab.cpp"
    break;

  case 226: /* type_specifier_nonarray: VOID  */
#line 1819 "MachineIndependent/glslang.y"
           {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtVoid;
    }
#line 7881 "MachineIndependent/glslang_tab.cpp"
    break;

  case 227: /* type_specifier_nonarray: FLOAT  */
#line 1823 "MachineIndependent/glslang.y"
            {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
    }
#line 7890 "MachineIndependent/glslang_tab.cpp"
    break;

  case 228: /* type_specifier_nonarray: INT  */
#line 1827 "MachineIndependent/glslang.y"
          {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt;
    }
#line 7899 "MachineIndependent/glslang_tab.cpp"
    break;

  case 229: /* type_specifier_nonarray: UINT  */
#line 1831 "MachineIndependent/glslang.y"
           {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "unsigned integer");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint;
    }
#line 7909 "MachineIndependent/glslang_tab.cpp"
    break;

  case 230: /* type_specifier_nonarray: BOOL  */
#line 1836 "MachineIndependent/glslang.y"
           {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtBool;
    }
#line 7918 "MachineIndependent/glslang_tab.cpp"
    break;

  case 231: /* type_specifier_nonarray: VEC2  */
#line 1840 "MachineIndependent/glslang.y"
           {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setVector(2);
    }
#line 7928 "MachineIndependent/glslang_tab.cpp"
    break;

  case 232: /* type_specifier_nonarray: VEC3  */
#line 1845 "MachineIndependent/glslang.y"
           {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setVector(3);
    }
#line 7938 "MachineIndependent/glslang_tab.cpp"
    break;

  case 233: /* type_specifier_nonarray: VEC4  */
#line 1850 "MachineIndependent/glslang.y"
           {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setVector(4);
    }
#line 7948 "MachineIndependent/glslang_tab.cpp"
    break;

  case 234: /* type_specifier_nonarray: BVEC2  */
#line 1855 "MachineIndependent/glslang.y"
            {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtBool;
        (yyval.interm.type).setVector(2);
    }
#line 7958 "MachineIndependent/glslang_tab.cpp"
    break;

  case 235: /* type_specifier_nonarray: BVEC3  */
#line 1860 "MachineIndependent/glslang.y"
            {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtBool;
        (yyval.interm.type).setVector(3);
    }
#line 7968 "MachineIndependent/glslang_tab.cpp"
    break;

  case 236: /* type_specifier_nonarray: BVEC4  */
#line 1865 "MachineIndependent/glslang.y"
            {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtBool;
        (yyval.interm.type).setVector(4);
    }
#line 7978 "MachineIndependent/glslang_tab.cpp"
    break;

  case 237: /* type_specifier_nonarray: IVEC2  */
#line 1870 "MachineIndependent/glslang.y"
            {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt;
        (yyval.interm.type).setVector(2);
    }
#line 7988 "MachineIndependent/glslang_tab.cpp"
    break;

  case 238: /* type_specifier_nonarray: IVEC3  */
#line 1875 "MachineIndependent/glslang.y"
            {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt;
        (yyval.interm.type).setVector(3);
    }
#line 7998 "MachineIndependent/glslang_tab.cpp"
    break;

  case 239: /* type_specifier_nonarray: IVEC4  */
#line 1880 "MachineIndependent/glslang.y"
            {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt;
        (yyval.interm.type).setVector(4);
    }
#line 8008 "MachineIndependent/glslang_tab.cpp"
    break;

  case 240: /* type_specifier_nonarray: UVEC2  */
#line 1885 "MachineIndependent/glslang.y"
            {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "unsigned integer vector");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint;
        (yyval.interm.type).setVector(2);
    }
#line 8019 "MachineIndependent/glslang_tab.cpp"
    break;

  case 241: /* type_specifier_nonarray: UVEC3  */
#line 1891 "MachineIndependent/glslang.y"
            {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "unsigned integer vector");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint;
        (yyval.interm.type).setVector(3);
    }
#line 8030 "MachineIndependent/glslang_tab.cpp"
    break;

  case 242: /* type_specifier_nonarray: UVEC4  */
#line 1897 "MachineIndependent/glslang.y"
            {
        parseContext.fullIntegerCheck((yyvsp[0].lex).loc, "unsigned integer vector");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint;
        (yyval.interm.type).setVector(4);
    }
#line 8041 "MachineIndependent/glslang_tab.cpp"
    break;

  case 243: /* type_specifier_nonarray: MAT2  */
#line 1903 "MachineIndependent/glslang.y"
           {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 8051 "MachineIndependent/glslang_tab.cpp"
    break;

  case 244: /* type_specifier_nonarray: MAT3  */
#line 1908 "MachineIndependent/glslang.y"
           {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 8061 "MachineIndependent/glslang_tab.cpp"
    break;

  case 245: /* type_specifier_nonarray: MAT4  */
#line 1913 "MachineIndependent/glslang.y"
           {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 8071 "MachineIndependent/glslang_tab.cpp"
    break;

  case 246: /* type_specifier_nonarray: MAT2X2  */
#line 1918 "MachineIndependent/glslang.y"
             {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 8081 "MachineIndependent/glslang_tab.cpp"
    break;

  case 247: /* type_specifier_nonarray: MAT2X3  */
#line 1923 "MachineIndependent/glslang.y"
             {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(2, 3);
    }
#line 8091 "MachineIndependent/glslang_tab.cpp"
    break;

  case 248: /* type_specifier_nonarray: MAT2X4  */
#line 1928 "MachineIndependent/glslang.y"
             {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(2, 4);
    }
#line 8101 "MachineIndependent/glslang_tab.cpp"
    break;

  case 249: /* type_specifier_nonarray: MAT3X2  */
#line 1933 "MachineIndependent/glslang.y"
             {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(3, 2);
    }
#line 8111 "MachineIndependent/glslang_tab.cpp"
    break;

  case 250: /* type_specifier_nonarray: MAT3X3  */
#line 1938 "MachineIndependent/glslang.y"
             {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 8121 "MachineIndependent/glslang_tab.cpp"
    break;

  case 251: /* type_specifier_nonarray: MAT3X4  */
#line 1943 "MachineIndependent/glslang.y"
             {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(3, 4);
    }
#line 8131 "MachineIndependent/glslang_tab.cpp"
    break;

  case 252: /* type_specifier_nonarray: MAT4X2  */
#line 1948 "MachineIndependent/glslang.y"
             {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(4, 2);
    }
#line 8141 "MachineIndependent/glslang_tab.cpp"
    break;

  case 253: /* type_specifier_nonarray: MAT4X3  */
#line 1953 "MachineIndependent/glslang.y"
             {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(4, 3);
    }
#line 8151 "MachineIndependent/glslang_tab.cpp"
    break;

  case 254: /* type_specifier_nonarray: MAT4X4  */
#line 1958 "MachineIndependent/glslang.y"
             {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 8161 "MachineIndependent/glslang_tab.cpp"
    break;

  case 255: /* type_specifier_nonarray: DOUBLE  */
#line 1963 "MachineIndependent/glslang.y"
             {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
    }
#line 8173 "MachineIndependent/glslang_tab.cpp"
    break;

  case 256: /* type_specifier_nonarray: BFLOAT16_T  */
#line 1970 "MachineIndependent/glslang.y"
                 {
        parseContext.bfloat16ScalarVectorCheck((yyvsp[0].lex).loc, "bfloat16_t", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtBFloat16;
    }
#line 8183 "MachineIndependent/glslang_tab.cpp"
    break;

  case 257: /* type_specifier_nonarray: FLOATE5M2_T  */
#line 1975 "MachineIndependent/glslang.y"
                  {
        parseContext.floate5m2ScalarVectorCheck((yyvsp[0].lex).loc, "floate5m2_t", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloatE5M2;
    }
#line 8193 "MachineIndependent/glslang_tab.cpp"
    break;

  case 258: /* type_specifier_nonarray: FLOATE4M3_T  */
#line 1980 "MachineIndependent/glslang.y"
                  {
        parseContext.floate4m3ScalarVectorCheck((yyvsp[0].lex).loc, "floate4m3_t", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloatE4M3;
    }
#line 8203 "MachineIndependent/glslang_tab.cpp"
    break;

  case 259: /* type_specifier_nonarray: FLOAT16_T  */
#line 1985 "MachineIndependent/glslang.y"
                {
        parseContext.float16ScalarVectorCheck((yyvsp[0].lex).loc, "float16_t", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
    }
#line 8213 "MachineIndependent/glslang_tab.cpp"
    break;

  case 260: /* type_specifier_nonarray: FLOAT32_T  */
#line 1990 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
    }
#line 8223 "MachineIndependent/glslang_tab.cpp"
    break;

  case 261: /* type_specifier_nonarray: FLOAT64_T  */
#line 1995 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
    }
#line 8233 "MachineIndependent/glslang_tab.cpp"
    break;

  case 262: /* type_specifier_nonarray: INT8_T  */
#line 2000 "MachineIndependent/glslang.y"
             {
        parseContext.int8ScalarVectorCheck((yyvsp[0].lex).loc, "8-bit signed integer", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt8;
    }
#line 8243 "MachineIndependent/glslang_tab.cpp"
    break;

  case 263: /* type_specifier_nonarray: UINT8_T  */
#line 2005 "MachineIndependent/glslang.y"
              {
        parseContext.int8ScalarVectorCheck((yyvsp[0].lex).loc, "8-bit unsigned integer", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint8;
    }
#line 8253 "MachineIndependent/glslang_tab.cpp"
    break;

  case 264: /* type_specifier_nonarray: INT16_T  */
#line 2010 "MachineIndependent/glslang.y"
              {
        parseContext.int16ScalarVectorCheck((yyvsp[0].lex).loc, "16-bit signed integer", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt16;
    }
#line 8263 "MachineIndependent/glslang_tab.cpp"
    break;

  case 265: /* type_specifier_nonarray: UINT16_T  */
#line 2015 "MachineIndependent/glslang.y"
               {
        parseContext.int16ScalarVectorCheck((yyvsp[0].lex).loc, "16-bit unsigned integer", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint16;
    }
#line 8273 "MachineIndependent/glslang_tab.cpp"
    break;

  case 266: /* type_specifier_nonarray: INT32_T  */
#line 2020 "MachineIndependent/glslang.y"
              {
        parseContext.explicitInt32Check((yyvsp[0].lex).loc, "32-bit signed integer", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt;
    }
#line 8283 "MachineIndependent/glslang_tab.cpp"
    break;

  case 267: /* type_specifier_nonarray: UINT32_T  */
#line 2025 "MachineIndependent/glslang.y"
               {
        parseContext.explicitInt32Check((yyvsp[0].lex).loc, "32-bit unsigned integer", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint;
    }
#line 8293 "MachineIndependent/glslang_tab.cpp"
    break;

  case 268: /* type_specifier_nonarray: INT64_T  */
#line 2030 "MachineIndependent/glslang.y"
              {
        parseContext.int64Check((yyvsp[0].lex).loc, "64-bit integer", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt64;
    }
#line 8303 "MachineIndependent/glslang_tab.cpp"
    break;

  case 269: /* type_specifier_nonarray: UINT64_T  */
#line 2035 "MachineIndependent/glslang.y"
               {
        parseContext.int64Check((yyvsp[0].lex).loc, "64-bit unsigned integer", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint64;
    }
#line 8313 "MachineIndependent/glslang_tab.cpp"
    break;

  case 270: /* type_specifier_nonarray: DVEC2  */
#line 2040 "MachineIndependent/glslang.y"
            {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double vector");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double vector");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setVector(2);
    }
#line 8326 "MachineIndependent/glslang_tab.cpp"
    break;

  case 271: /* type_specifier_nonarray: DVEC3  */
#line 2048 "MachineIndependent/glslang.y"
            {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double vector");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double vector");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setVector(3);
    }
#line 8339 "MachineIndependent/glslang_tab.cpp"
    break;

  case 272: /* type_specifier_nonarray: DVEC4  */
#line 2056 "MachineIndependent/glslang.y"
            {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double vector");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double vector");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setVector(4);
    }
#line 8352 "MachineIndependent/glslang_tab.cpp"
    break;

  case 273: /* type_specifier_nonarray: BF16VEC2  */
#line 2064 "MachineIndependent/glslang.y"
               {
        parseContext.bfloat16ScalarVectorCheck((yyvsp[0].lex).loc, "half float vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtBFloat16;
        (yyval.interm.type).setVector(2);
    }
#line 8363 "MachineIndependent/glslang_tab.cpp"
    break;

  case 274: /* type_specifier_nonarray: BF16VEC3  */
#line 2070 "MachineIndependent/glslang.y"
               {
        parseContext.bfloat16ScalarVectorCheck((yyvsp[0].lex).loc, "half float vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtBFloat16;
        (yyval.interm.type).setVector(3);
    }
#line 8374 "MachineIndependent/glslang_tab.cpp"
    break;

  case 275: /* type_specifier_nonarray: BF16VEC4  */
#line 2076 "MachineIndependent/glslang.y"
               {
        parseContext.bfloat16ScalarVectorCheck((yyvsp[0].lex).loc, "half float vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtBFloat16;
        (yyval.interm.type).setVector(4);
    }
#line 8385 "MachineIndependent/glslang_tab.cpp"
    break;

  case 276: /* type_specifier_nonarray: FE5M2VEC2  */
#line 2082 "MachineIndependent/glslang.y"
                {
        parseContext.floate5m2ScalarVectorCheck((yyvsp[0].lex).loc, "fe5m2 vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloatE5M2;
        (yyval.interm.type).setVector(2);
    }
#line 8396 "MachineIndependent/glslang_tab.cpp"
    break;

  case 277: /* type_specifier_nonarray: FE5M2VEC3  */
#line 2088 "MachineIndependent/glslang.y"
                {
        parseContext.floate5m2ScalarVectorCheck((yyvsp[0].lex).loc, "fe5m2 vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloatE5M2;
        (yyval.interm.type).setVector(3);
    }
#line 8407 "MachineIndependent/glslang_tab.cpp"
    break;

  case 278: /* type_specifier_nonarray: FE5M2VEC4  */
#line 2094 "MachineIndependent/glslang.y"
                {
        parseContext.floate5m2ScalarVectorCheck((yyvsp[0].lex).loc, "fe5m2 vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloatE5M2;
        (yyval.interm.type).setVector(4);
    }
#line 8418 "MachineIndependent/glslang_tab.cpp"
    break;

  case 279: /* type_specifier_nonarray: FE4M3VEC2  */
#line 2100 "MachineIndependent/glslang.y"
                {
        parseContext.floate4m3ScalarVectorCheck((yyvsp[0].lex).loc, "fe4m3 vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloatE4M3;
        (yyval.interm.type).setVector(2);
    }
#line 8429 "MachineIndependent/glslang_tab.cpp"
    break;

  case 280: /* type_specifier_nonarray: FE4M3VEC3  */
#line 2106 "MachineIndependent/glslang.y"
                {
        parseContext.floate4m3ScalarVectorCheck((yyvsp[0].lex).loc, "fe4m3 vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloatE4M3;
        (yyval.interm.type).setVector(3);
    }
#line 8440 "MachineIndependent/glslang_tab.cpp"
    break;

  case 281: /* type_specifier_nonarray: FE4M3VEC4  */
#line 2112 "MachineIndependent/glslang.y"
                {
        parseContext.floate4m3ScalarVectorCheck((yyvsp[0].lex).loc, "fe4m3 vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloatE4M3;
        (yyval.interm.type).setVector(4);
    }
#line 8451 "MachineIndependent/glslang_tab.cpp"
    break;

  case 282: /* type_specifier_nonarray: F16VEC2  */
#line 2118 "MachineIndependent/glslang.y"
              {
        parseContext.float16ScalarVectorCheck((yyvsp[0].lex).loc, "half float vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setVector(2);
    }
#line 8462 "MachineIndependent/glslang_tab.cpp"
    break;

  case 283: /* type_specifier_nonarray: F16VEC3  */
#line 2124 "MachineIndependent/glslang.y"
              {
        parseContext.float16ScalarVectorCheck((yyvsp[0].lex).loc, "half float vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setVector(3);
    }
#line 8473 "MachineIndependent/glslang_tab.cpp"
    break;

  case 284: /* type_specifier_nonarray: F16VEC4  */
#line 2130 "MachineIndependent/glslang.y"
              {
        parseContext.float16ScalarVectorCheck((yyvsp[0].lex).loc, "half float vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setVector(4);
    }
#line 8484 "MachineIndependent/glslang_tab.cpp"
    break;

  case 285: /* type_specifier_nonarray: F32VEC2  */
#line 2136 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setVector(2);
    }
#line 8495 "MachineIndependent/glslang_tab.cpp"
    break;

  case 286: /* type_specifier_nonarray: F32VEC3  */
#line 2142 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setVector(3);
    }
#line 8506 "MachineIndependent/glslang_tab.cpp"
    break;

  case 287: /* type_specifier_nonarray: F32VEC4  */
#line 2148 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setVector(4);
    }
#line 8517 "MachineIndependent/glslang_tab.cpp"
    break;

  case 288: /* type_specifier_nonarray: F64VEC2  */
#line 2154 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setVector(2);
    }
#line 8528 "MachineIndependent/glslang_tab.cpp"
    break;

  case 289: /* type_specifier_nonarray: F64VEC3  */
#line 2160 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setVector(3);
    }
#line 8539 "MachineIndependent/glslang_tab.cpp"
    break;

  case 290: /* type_specifier_nonarray: F64VEC4  */
#line 2166 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setVector(4);
    }
#line 8550 "MachineIndependent/glslang_tab.cpp"
    break;

  case 291: /* type_specifier_nonarray: I8VEC2  */
#line 2172 "MachineIndependent/glslang.y"
             {
        parseContext.int8ScalarVectorCheck((yyvsp[0].lex).loc, "8-bit signed integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt8;
        (yyval.interm.type).setVector(2);
    }
#line 8561 "MachineIndependent/glslang_tab.cpp"
    break;

  case 292: /* type_specifier_nonarray: I8VEC3  */
#line 2178 "MachineIndependent/glslang.y"
             {
        parseContext.int8ScalarVectorCheck((yyvsp[0].lex).loc, "8-bit signed integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt8;
        (yyval.interm.type).setVector(3);
    }
#line 8572 "MachineIndependent/glslang_tab.cpp"
    break;

  case 293: /* type_specifier_nonarray: I8VEC4  */
#line 2184 "MachineIndependent/glslang.y"
             {
        parseContext.int8ScalarVectorCheck((yyvsp[0].lex).loc, "8-bit signed integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt8;
        (yyval.interm.type).setVector(4);
    }
#line 8583 "MachineIndependent/glslang_tab.cpp"
    break;

  case 294: /* type_specifier_nonarray: I16VEC2  */
#line 2190 "MachineIndependent/glslang.y"
              {
        parseContext.int16ScalarVectorCheck((yyvsp[0].lex).loc, "16-bit signed integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt16;
        (yyval.interm.type).setVector(2);
    }
#line 8594 "MachineIndependent/glslang_tab.cpp"
    break;

  case 295: /* type_specifier_nonarray: I16VEC3  */
#line 2196 "MachineIndependent/glslang.y"
              {
        parseContext.int16ScalarVectorCheck((yyvsp[0].lex).loc, "16-bit signed integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt16;
        (yyval.interm.type).setVector(3);
    }
#line 8605 "MachineIndependent/glslang_tab.cpp"
    break;

  case 296: /* type_specifier_nonarray: I16VEC4  */
#line 2202 "MachineIndependent/glslang.y"
              {
        parseContext.int16ScalarVectorCheck((yyvsp[0].lex).loc, "16-bit signed integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt16;
        (yyval.interm.type).setVector(4);
    }
#line 8616 "MachineIndependent/glslang_tab.cpp"
    break;

  case 297: /* type_specifier_nonarray: I32VEC2  */
#line 2208 "MachineIndependent/glslang.y"
              {
        parseContext.explicitInt32Check((yyvsp[0].lex).loc, "32-bit signed integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt;
        (yyval.interm.type).setVector(2);
    }
#line 8627 "MachineIndependent/glslang_tab.cpp"
    break;

  case 298: /* type_specifier_nonarray: I32VEC3  */
#line 2214 "MachineIndependent/glslang.y"
              {
        parseContext.explicitInt32Check((yyvsp[0].lex).loc, "32-bit signed integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt;
        (yyval.interm.type).setVector(3);
    }
#line 8638 "MachineIndependent/glslang_tab.cpp"
    break;

  case 299: /* type_specifier_nonarray: I32VEC4  */
#line 2220 "MachineIndependent/glslang.y"
              {
        parseContext.explicitInt32Check((yyvsp[0].lex).loc, "32-bit signed integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt;
        (yyval.interm.type).setVector(4);
    }
#line 8649 "MachineIndependent/glslang_tab.cpp"
    break;

  case 300: /* type_specifier_nonarray: I64VEC2  */
#line 2226 "MachineIndependent/glslang.y"
              {
        parseContext.int64Check((yyvsp[0].lex).loc, "64-bit integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt64;
        (yyval.interm.type).setVector(2);
    }
#line 8660 "MachineIndependent/glslang_tab.cpp"
    break;

  case 301: /* type_specifier_nonarray: I64VEC3  */
#line 2232 "MachineIndependent/glslang.y"
              {
        parseContext.int64Check((yyvsp[0].lex).loc, "64-bit integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt64;
        (yyval.interm.type).setVector(3);
    }
#line 8671 "MachineIndependent/glslang_tab.cpp"
    break;

  case 302: /* type_specifier_nonarray: I64VEC4  */
#line 2238 "MachineIndependent/glslang.y"
              {
        parseContext.int64Check((yyvsp[0].lex).loc, "64-bit integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt64;
        (yyval.interm.type).setVector(4);
    }
#line 8682 "MachineIndependent/glslang_tab.cpp"
    break;

  case 303: /* type_specifier_nonarray: U8VEC2  */
#line 2244 "MachineIndependent/glslang.y"
             {
        parseContext.int8ScalarVectorCheck((yyvsp[0].lex).loc, "8-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint8;
        (yyval.interm.type).setVector(2);
    }
#line 8693 "MachineIndependent/glslang_tab.cpp"
    break;

  case 304: /* type_specifier_nonarray: U8VEC3  */
#line 2250 "MachineIndependent/glslang.y"
             {
        parseContext.int8ScalarVectorCheck((yyvsp[0].lex).loc, "8-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint8;
        (yyval.interm.type).setVector(3);
    }
#line 8704 "MachineIndependent/glslang_tab.cpp"
    break;

  case 305: /* type_specifier_nonarray: U8VEC4  */
#line 2256 "MachineIndependent/glslang.y"
             {
        parseContext.int8ScalarVectorCheck((yyvsp[0].lex).loc, "8-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint8;
        (yyval.interm.type).setVector(4);
    }
#line 8715 "MachineIndependent/glslang_tab.cpp"
    break;

  case 306: /* type_specifier_nonarray: U16VEC2  */
#line 2262 "MachineIndependent/glslang.y"
              {
        parseContext.int16ScalarVectorCheck((yyvsp[0].lex).loc, "16-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint16;
        (yyval.interm.type).setVector(2);
    }
#line 8726 "MachineIndependent/glslang_tab.cpp"
    break;

  case 307: /* type_specifier_nonarray: U16VEC3  */
#line 2268 "MachineIndependent/glslang.y"
              {
        parseContext.int16ScalarVectorCheck((yyvsp[0].lex).loc, "16-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint16;
        (yyval.interm.type).setVector(3);
    }
#line 8737 "MachineIndependent/glslang_tab.cpp"
    break;

  case 308: /* type_specifier_nonarray: U16VEC4  */
#line 2274 "MachineIndependent/glslang.y"
              {
        parseContext.int16ScalarVectorCheck((yyvsp[0].lex).loc, "16-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint16;
        (yyval.interm.type).setVector(4);
    }
#line 8748 "MachineIndependent/glslang_tab.cpp"
    break;

  case 309: /* type_specifier_nonarray: U32VEC2  */
#line 2280 "MachineIndependent/glslang.y"
              {
        parseContext.explicitInt32Check((yyvsp[0].lex).loc, "32-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint;
        (yyval.interm.type).setVector(2);
    }
#line 8759 "MachineIndependent/glslang_tab.cpp"
    break;

  case 310: /* type_specifier_nonarray: U32VEC3  */
#line 2286 "MachineIndependent/glslang.y"
              {
        parseContext.explicitInt32Check((yyvsp[0].lex).loc, "32-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint;
        (yyval.interm.type).setVector(3);
    }
#line 8770 "MachineIndependent/glslang_tab.cpp"
    break;

  case 311: /* type_specifier_nonarray: U32VEC4  */
#line 2292 "MachineIndependent/glslang.y"
              {
        parseContext.explicitInt32Check((yyvsp[0].lex).loc, "32-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint;
        (yyval.interm.type).setVector(4);
    }
#line 8781 "MachineIndependent/glslang_tab.cpp"
    break;

  case 312: /* type_specifier_nonarray: U64VEC2  */
#line 2298 "MachineIndependent/glslang.y"
              {
        parseContext.int64Check((yyvsp[0].lex).loc, "64-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint64;
        (yyval.interm.type).setVector(2);
    }
#line 8792 "MachineIndependent/glslang_tab.cpp"
    break;

  case 313: /* type_specifier_nonarray: U64VEC3  */
#line 2304 "MachineIndependent/glslang.y"
              {
        parseContext.int64Check((yyvsp[0].lex).loc, "64-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint64;
        (yyval.interm.type).setVector(3);
    }
#line 8803 "MachineIndependent/glslang_tab.cpp"
    break;

  case 314: /* type_specifier_nonarray: U64VEC4  */
#line 2310 "MachineIndependent/glslang.y"
              {
        parseContext.int64Check((yyvsp[0].lex).loc, "64-bit unsigned integer vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint64;
        (yyval.interm.type).setVector(4);
    }
#line 8814 "MachineIndependent/glslang_tab.cpp"
    break;

  case 315: /* type_specifier_nonarray: DMAT2  */
#line 2316 "MachineIndependent/glslang.y"
            {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 8827 "MachineIndependent/glslang_tab.cpp"
    break;

  case 316: /* type_specifier_nonarray: DMAT3  */
#line 2324 "MachineIndependent/glslang.y"
            {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 8840 "MachineIndependent/glslang_tab.cpp"
    break;

  case 317: /* type_specifier_nonarray: DMAT4  */
#line 2332 "MachineIndependent/glslang.y"
            {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 8853 "MachineIndependent/glslang_tab.cpp"
    break;

  case 318: /* type_specifier_nonarray: DMAT2X2  */
#line 2340 "MachineIndependent/glslang.y"
              {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 8866 "MachineIndependent/glslang_tab.cpp"
    break;

  case 319: /* type_specifier_nonarray: DMAT2X3  */
#line 2348 "MachineIndependent/glslang.y"
              {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(2, 3);
    }
#line 8879 "MachineIndependent/glslang_tab.cpp"
    break;

  case 320: /* type_specifier_nonarray: DMAT2X4  */
#line 2356 "MachineIndependent/glslang.y"
              {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(2, 4);
    }
#line 8892 "MachineIndependent/glslang_tab.cpp"
    break;

  case 321: /* type_specifier_nonarray: DMAT3X2  */
#line 2364 "MachineIndependent/glslang.y"
              {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(3, 2);
    }
#line 8905 "MachineIndependent/glslang_tab.cpp"
    break;

  case 322: /* type_specifier_nonarray: DMAT3X3  */
#line 2372 "MachineIndependent/glslang.y"
              {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 8918 "MachineIndependent/glslang_tab.cpp"
    break;

  case 323: /* type_specifier_nonarray: DMAT3X4  */
#line 2380 "MachineIndependent/glslang.y"
              {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(3, 4);
    }
#line 8931 "MachineIndependent/glslang_tab.cpp"
    break;

  case 324: /* type_specifier_nonarray: DMAT4X2  */
#line 2388 "MachineIndependent/glslang.y"
              {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(4, 2);
    }
#line 8944 "MachineIndependent/glslang_tab.cpp"
    break;

  case 325: /* type_specifier_nonarray: DMAT4X3  */
#line 2396 "MachineIndependent/glslang.y"
              {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(4, 3);
    }
#line 8957 "MachineIndependent/glslang_tab.cpp"
    break;

  case 326: /* type_specifier_nonarray: DMAT4X4  */
#line 2404 "MachineIndependent/glslang.y"
              {
        parseContext.requireProfile((yyvsp[0].lex).loc, ECoreProfile | ECompatibilityProfile, "double matrix");
        if (! parseContext.symbolTable.atBuiltInLevel())
            parseContext.doubleCheck((yyvsp[0].lex).loc, "double matrix");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 8970 "MachineIndependent/glslang_tab.cpp"
    break;

  case 327: /* type_specifier_nonarray: F16MAT2  */
#line 2412 "MachineIndependent/glslang.y"
              {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 8981 "MachineIndependent/glslang_tab.cpp"
    break;

  case 328: /* type_specifier_nonarray: F16MAT3  */
#line 2418 "MachineIndependent/glslang.y"
              {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 8992 "MachineIndependent/glslang_tab.cpp"
    break;

  case 329: /* type_specifier_nonarray: F16MAT4  */
#line 2424 "MachineIndependent/glslang.y"
              {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 9003 "MachineIndependent/glslang_tab.cpp"
    break;

  case 330: /* type_specifier_nonarray: F16MAT2X2  */
#line 2430 "MachineIndependent/glslang.y"
                {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 9014 "MachineIndependent/glslang_tab.cpp"
    break;

  case 331: /* type_specifier_nonarray: F16MAT2X3  */
#line 2436 "MachineIndependent/glslang.y"
                {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(2, 3);
    }
#line 9025 "MachineIndependent/glslang_tab.cpp"
    break;

  case 332: /* type_specifier_nonarray: F16MAT2X4  */
#line 2442 "MachineIndependent/glslang.y"
                {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(2, 4);
    }
#line 9036 "MachineIndependent/glslang_tab.cpp"
    break;

  case 333: /* type_specifier_nonarray: F16MAT3X2  */
#line 2448 "MachineIndependent/glslang.y"
                {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(3, 2);
    }
#line 9047 "MachineIndependent/glslang_tab.cpp"
    break;

  case 334: /* type_specifier_nonarray: F16MAT3X3  */
#line 2454 "MachineIndependent/glslang.y"
                {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 9058 "MachineIndependent/glslang_tab.cpp"
    break;

  case 335: /* type_specifier_nonarray: F16MAT3X4  */
#line 2460 "MachineIndependent/glslang.y"
                {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(3, 4);
    }
#line 9069 "MachineIndependent/glslang_tab.cpp"
    break;

  case 336: /* type_specifier_nonarray: F16MAT4X2  */
#line 2466 "MachineIndependent/glslang.y"
                {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(4, 2);
    }
#line 9080 "MachineIndependent/glslang_tab.cpp"
    break;

  case 337: /* type_specifier_nonarray: F16MAT4X3  */
#line 2472 "MachineIndependent/glslang.y"
                {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(4, 3);
    }
#line 9091 "MachineIndependent/glslang_tab.cpp"
    break;

  case 338: /* type_specifier_nonarray: F16MAT4X4  */
#line 2478 "MachineIndependent/glslang.y"
                {
        parseContext.float16Check((yyvsp[0].lex).loc, "half float matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat16;
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 9102 "MachineIndependent/glslang_tab.cpp"
    break;

  case 339: /* type_specifier_nonarray: F32MAT2  */
#line 2484 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 9113 "MachineIndependent/glslang_tab.cpp"
    break;

  case 340: /* type_specifier_nonarray: F32MAT3  */
#line 2490 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 9124 "MachineIndependent/glslang_tab.cpp"
    break;

  case 341: /* type_specifier_nonarray: F32MAT4  */
#line 2496 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 9135 "MachineIndependent/glslang_tab.cpp"
    break;

  case 342: /* type_specifier_nonarray: F32MAT2X2  */
#line 2502 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 9146 "MachineIndependent/glslang_tab.cpp"
    break;

  case 343: /* type_specifier_nonarray: F32MAT2X3  */
#line 2508 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(2, 3);
    }
#line 9157 "MachineIndependent/glslang_tab.cpp"
    break;

  case 344: /* type_specifier_nonarray: F32MAT2X4  */
#line 2514 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(2, 4);
    }
#line 9168 "MachineIndependent/glslang_tab.cpp"
    break;

  case 345: /* type_specifier_nonarray: F32MAT3X2  */
#line 2520 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(3, 2);
    }
#line 9179 "MachineIndependent/glslang_tab.cpp"
    break;

  case 346: /* type_specifier_nonarray: F32MAT3X3  */
#line 2526 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 9190 "MachineIndependent/glslang_tab.cpp"
    break;

  case 347: /* type_specifier_nonarray: F32MAT3X4  */
#line 2532 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(3, 4);
    }
#line 9201 "MachineIndependent/glslang_tab.cpp"
    break;

  case 348: /* type_specifier_nonarray: F32MAT4X2  */
#line 2538 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(4, 2);
    }
#line 9212 "MachineIndependent/glslang_tab.cpp"
    break;

  case 349: /* type_specifier_nonarray: F32MAT4X3  */
#line 2544 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(4, 3);
    }
#line 9223 "MachineIndependent/glslang_tab.cpp"
    break;

  case 350: /* type_specifier_nonarray: F32MAT4X4  */
#line 2550 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat32Check((yyvsp[0].lex).loc, "float32_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 9234 "MachineIndependent/glslang_tab.cpp"
    break;

  case 351: /* type_specifier_nonarray: F64MAT2  */
#line 2556 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 9245 "MachineIndependent/glslang_tab.cpp"
    break;

  case 352: /* type_specifier_nonarray: F64MAT3  */
#line 2562 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 9256 "MachineIndependent/glslang_tab.cpp"
    break;

  case 353: /* type_specifier_nonarray: F64MAT4  */
#line 2568 "MachineIndependent/glslang.y"
              {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 9267 "MachineIndependent/glslang_tab.cpp"
    break;

  case 354: /* type_specifier_nonarray: F64MAT2X2  */
#line 2574 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(2, 2);
    }
#line 9278 "MachineIndependent/glslang_tab.cpp"
    break;

  case 355: /* type_specifier_nonarray: F64MAT2X3  */
#line 2580 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(2, 3);
    }
#line 9289 "MachineIndependent/glslang_tab.cpp"
    break;

  case 356: /* type_specifier_nonarray: F64MAT2X4  */
#line 2586 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(2, 4);
    }
#line 9300 "MachineIndependent/glslang_tab.cpp"
    break;

  case 357: /* type_specifier_nonarray: F64MAT3X2  */
#line 2592 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(3, 2);
    }
#line 9311 "MachineIndependent/glslang_tab.cpp"
    break;

  case 358: /* type_specifier_nonarray: F64MAT3X3  */
#line 2598 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(3, 3);
    }
#line 9322 "MachineIndependent/glslang_tab.cpp"
    break;

  case 359: /* type_specifier_nonarray: F64MAT3X4  */
#line 2604 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(3, 4);
    }
#line 9333 "MachineIndependent/glslang_tab.cpp"
    break;

  case 360: /* type_specifier_nonarray: F64MAT4X2  */
#line 2610 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(4, 2);
    }
#line 9344 "MachineIndependent/glslang_tab.cpp"
    break;

  case 361: /* type_specifier_nonarray: F64MAT4X3  */
#line 2616 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(4, 3);
    }
#line 9355 "MachineIndependent/glslang_tab.cpp"
    break;

  case 362: /* type_specifier_nonarray: F64MAT4X4  */
#line 2622 "MachineIndependent/glslang.y"
                {
        parseContext.explicitFloat64Check((yyvsp[0].lex).loc, "float64_t matrix", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtDouble;
        (yyval.interm.type).setMatrix(4, 4);
    }
#line 9366 "MachineIndependent/glslang_tab.cpp"
    break;

  case 363: /* type_specifier_nonarray: ACCSTRUCTNV  */
#line 2628 "MachineIndependent/glslang.y"
                  {
       (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
       (yyval.interm.type).basicType = EbtAccStruct;
    }
#line 9375 "MachineIndependent/glslang_tab.cpp"
    break;

  case 364: /* type_specifier_nonarray: ACCSTRUCTEXT  */
#line 2632 "MachineIndependent/glslang.y"
                   {
       (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
       (yyval.interm.type).basicType = EbtAccStruct;
    }
#line 9384 "MachineIndependent/glslang_tab.cpp"
    break;

  case 365: /* type_specifier_nonarray: RAYQUERYEXT  */
#line 2636 "MachineIndependent/glslang.y"
                  {
       (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
       (yyval.interm.type).basicType = EbtRayQuery;
    }
#line 9393 "MachineIndependent/glslang_tab.cpp"
    break;

  case 366: /* type_specifier_nonarray: ATOMIC_UINT  */
#line 2640 "MachineIndependent/glslang.y"
                  {
        parseContext.vulkanRemoved((yyvsp[0].lex).loc, "atomic counter types");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtAtomicUint;
    }
#line 9403 "MachineIndependent/glslang_tab.cpp"
    break;

  case 367: /* type_specifier_nonarray: SAMPLER1D  */
#line 2645 "MachineIndependent/glslang.y"
                {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd1D);
    }
#line 9413 "MachineIndependent/glslang_tab.cpp"
    break;

  case 368: /* type_specifier_nonarray: SAMPLER2D  */
#line 2650 "MachineIndependent/glslang.y"
                {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd2D);
    }
#line 9423 "MachineIndependent/glslang_tab.cpp"
    break;

  case 369: /* type_specifier_nonarray: SAMPLER3D  */
#line 2655 "MachineIndependent/glslang.y"
                {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd3D);
    }
#line 9433 "MachineIndependent/glslang_tab.cpp"
    break;

  case 370: /* type_specifier_nonarray: SAMPLERCUBE  */
#line 2660 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, EsdCube);
    }
#line 9443 "MachineIndependent/glslang_tab.cpp"
    break;

  case 371: /* type_specifier_nonarray: SAMPLER2DSHADOW  */
#line 2665 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd2D, false, true);
    }
#line 9453 "MachineIndependent/glslang_tab.cpp"
    break;

  case 372: /* type_specifier_nonarray: SAMPLERCUBESHADOW  */
#line 2670 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, EsdCube, false, true);
    }
#line 9463 "MachineIndependent/glslang_tab.cpp"
    break;

  case 373: /* type_specifier_nonarray: SAMPLER2DARRAY  */
#line 2675 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd2D, true);
    }
#line 9473 "MachineIndependent/glslang_tab.cpp"
    break;

  case 374: /* type_specifier_nonarray: SAMPLER2DARRAYSHADOW  */
#line 2680 "MachineIndependent/glslang.y"
                           {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd2D, true, true);
    }
#line 9483 "MachineIndependent/glslang_tab.cpp"
    break;

  case 375: /* type_specifier_nonarray: SAMPLER1DSHADOW  */
#line 2685 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd1D, false, true);
    }
#line 9493 "MachineIndependent/glslang_tab.cpp"
    break;

  case 376: /* type_specifier_nonarray: SAMPLER1DARRAY  */
#line 2690 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd1D, true);
    }
#line 9503 "MachineIndependent/glslang_tab.cpp"
    break;

  case 377: /* type_specifier_nonarray: SAMPLER1DARRAYSHADOW  */
#line 2695 "MachineIndependent/glslang.y"
                           {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd1D, true, true);
    }
#line 9513 "MachineIndependent/glslang_tab.cpp"
    break;

  case 378: /* type_specifier_nonarray: SAMPLERCUBEARRAY  */
#line 2700 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, EsdCube, true);
    }
#line 9523 "MachineIndependent/glslang_tab.cpp"
    break;

  case 379: /* type_specifier_nonarray: SAMPLERCUBEARRAYSHADOW  */
#line 2705 "MachineIndependent/glslang.y"
                             {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, EsdCube, true, true);
    }
#line 9533 "MachineIndependent/glslang_tab.cpp"
    break;

  case 380: /* type_specifier_nonarray: F16SAMPLER1D  */
#line 2710 "MachineIndependent/glslang.y"
                   {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd1D);
    }
#line 9544 "MachineIndependent/glslang_tab.cpp"
    break;

  case 381: /* type_specifier_nonarray: F16SAMPLER2D  */
#line 2716 "MachineIndependent/glslang.y"
                   {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd2D);
    }
#line 9555 "MachineIndependent/glslang_tab.cpp"
    break;

  case 382: /* type_specifier_nonarray: F16SAMPLER3D  */
#line 2722 "MachineIndependent/glslang.y"
                   {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd3D);
    }
#line 9566 "MachineIndependent/glslang_tab.cpp"
    break;

  case 383: /* type_specifier_nonarray: F16SAMPLERCUBE  */
#line 2728 "MachineIndependent/glslang.y"
                     {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, EsdCube);
    }
#line 9577 "MachineIndependent/glslang_tab.cpp"
    break;

  case 384: /* type_specifier_nonarray: F16SAMPLER1DSHADOW  */
#line 2734 "MachineIndependent/glslang.y"
                         {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd1D, false, true);
    }
#line 9588 "MachineIndependent/glslang_tab.cpp"
    break;

  case 385: /* type_specifier_nonarray: F16SAMPLER2DSHADOW  */
#line 2740 "MachineIndependent/glslang.y"
                         {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd2D, false, true);
    }
#line 9599 "MachineIndependent/glslang_tab.cpp"
    break;

  case 386: /* type_specifier_nonarray: F16SAMPLERCUBESHADOW  */
#line 2746 "MachineIndependent/glslang.y"
                           {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, EsdCube, false, true);
    }
#line 9610 "MachineIndependent/glslang_tab.cpp"
    break;

  case 387: /* type_specifier_nonarray: F16SAMPLER1DARRAY  */
#line 2752 "MachineIndependent/glslang.y"
                        {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd1D, true);
    }
#line 9621 "MachineIndependent/glslang_tab.cpp"
    break;

  case 388: /* type_specifier_nonarray: F16SAMPLER2DARRAY  */
#line 2758 "MachineIndependent/glslang.y"
                        {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd2D, true);
    }
#line 9632 "MachineIndependent/glslang_tab.cpp"
    break;

  case 389: /* type_specifier_nonarray: F16SAMPLER1DARRAYSHADOW  */
#line 2764 "MachineIndependent/glslang.y"
                              {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd1D, true, true);
    }
#line 9643 "MachineIndependent/glslang_tab.cpp"
    break;

  case 390: /* type_specifier_nonarray: F16SAMPLER2DARRAYSHADOW  */
#line 2770 "MachineIndependent/glslang.y"
                              {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd2D, true, true);
    }
#line 9654 "MachineIndependent/glslang_tab.cpp"
    break;

  case 391: /* type_specifier_nonarray: F16SAMPLERCUBEARRAY  */
#line 2776 "MachineIndependent/glslang.y"
                          {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, EsdCube, true);
    }
#line 9665 "MachineIndependent/glslang_tab.cpp"
    break;

  case 392: /* type_specifier_nonarray: F16SAMPLERCUBEARRAYSHADOW  */
#line 2782 "MachineIndependent/glslang.y"
                                {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, EsdCube, true, true);
    }
#line 9676 "MachineIndependent/glslang_tab.cpp"
    break;

  case 393: /* type_specifier_nonarray: ISAMPLER1D  */
#line 2788 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, Esd1D);
    }
#line 9686 "MachineIndependent/glslang_tab.cpp"
    break;

  case 394: /* type_specifier_nonarray: ISAMPLER2D  */
#line 2793 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, Esd2D);
    }
#line 9696 "MachineIndependent/glslang_tab.cpp"
    break;

  case 395: /* type_specifier_nonarray: ISAMPLER3D  */
#line 2798 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, Esd3D);
    }
#line 9706 "MachineIndependent/glslang_tab.cpp"
    break;

  case 396: /* type_specifier_nonarray: ISAMPLERCUBE  */
#line 2803 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, EsdCube);
    }
#line 9716 "MachineIndependent/glslang_tab.cpp"
    break;

  case 397: /* type_specifier_nonarray: ISAMPLER2DARRAY  */
#line 2808 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, Esd2D, true);
    }
#line 9726 "MachineIndependent/glslang_tab.cpp"
    break;

  case 398: /* type_specifier_nonarray: USAMPLER2D  */
#line 2813 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, Esd2D);
    }
#line 9736 "MachineIndependent/glslang_tab.cpp"
    break;

  case 399: /* type_specifier_nonarray: USAMPLER3D  */
#line 2818 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, Esd3D);
    }
#line 9746 "MachineIndependent/glslang_tab.cpp"
    break;

  case 400: /* type_specifier_nonarray: USAMPLERCUBE  */
#line 2823 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, EsdCube);
    }
#line 9756 "MachineIndependent/glslang_tab.cpp"
    break;

  case 401: /* type_specifier_nonarray: ISAMPLER1DARRAY  */
#line 2828 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, Esd1D, true);
    }
#line 9766 "MachineIndependent/glslang_tab.cpp"
    break;

  case 402: /* type_specifier_nonarray: ISAMPLERCUBEARRAY  */
#line 2833 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, EsdCube, true);
    }
#line 9776 "MachineIndependent/glslang_tab.cpp"
    break;

  case 403: /* type_specifier_nonarray: USAMPLER1D  */
#line 2838 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, Esd1D);
    }
#line 9786 "MachineIndependent/glslang_tab.cpp"
    break;

  case 404: /* type_specifier_nonarray: USAMPLER1DARRAY  */
#line 2843 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, Esd1D, true);
    }
#line 9796 "MachineIndependent/glslang_tab.cpp"
    break;

  case 405: /* type_specifier_nonarray: USAMPLERCUBEARRAY  */
#line 2848 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, EsdCube, true);
    }
#line 9806 "MachineIndependent/glslang_tab.cpp"
    break;

  case 406: /* type_specifier_nonarray: TEXTURECUBEARRAY  */
#line 2853 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, EsdCube, true);
    }
#line 9816 "MachineIndependent/glslang_tab.cpp"
    break;

  case 407: /* type_specifier_nonarray: ITEXTURECUBEARRAY  */
#line 2858 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, EsdCube, true);
    }
#line 9826 "MachineIndependent/glslang_tab.cpp"
    break;

  case 408: /* type_specifier_nonarray: UTEXTURECUBEARRAY  */
#line 2863 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, EsdCube, true);
    }
#line 9836 "MachineIndependent/glslang_tab.cpp"
    break;

  case 409: /* type_specifier_nonarray: USAMPLER2DARRAY  */
#line 2868 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, Esd2D, true);
    }
#line 9846 "MachineIndependent/glslang_tab.cpp"
    break;

  case 410: /* type_specifier_nonarray: TEXTURE2D  */
#line 2873 "MachineIndependent/glslang.y"
                {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, Esd2D);
    }
#line 9856 "MachineIndependent/glslang_tab.cpp"
    break;

  case 411: /* type_specifier_nonarray: TEXTURE3D  */
#line 2878 "MachineIndependent/glslang.y"
                {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, Esd3D);
    }
#line 9866 "MachineIndependent/glslang_tab.cpp"
    break;

  case 412: /* type_specifier_nonarray: TEXTURE2DARRAY  */
#line 2883 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, Esd2D, true);
    }
#line 9876 "MachineIndependent/glslang_tab.cpp"
    break;

  case 413: /* type_specifier_nonarray: TEXTURECUBE  */
#line 2888 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, EsdCube);
    }
#line 9886 "MachineIndependent/glslang_tab.cpp"
    break;

  case 414: /* type_specifier_nonarray: ITEXTURE2D  */
#line 2893 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, Esd2D);
    }
#line 9896 "MachineIndependent/glslang_tab.cpp"
    break;

  case 415: /* type_specifier_nonarray: ITEXTURE3D  */
#line 2898 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, Esd3D);
    }
#line 9906 "MachineIndependent/glslang_tab.cpp"
    break;

  case 416: /* type_specifier_nonarray: ITEXTURECUBE  */
#line 2903 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, EsdCube);
    }
#line 9916 "MachineIndependent/glslang_tab.cpp"
    break;

  case 417: /* type_specifier_nonarray: ITEXTURE2DARRAY  */
#line 2908 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, Esd2D, true);
    }
#line 9926 "MachineIndependent/glslang_tab.cpp"
    break;

  case 418: /* type_specifier_nonarray: UTEXTURE2D  */
#line 2913 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, Esd2D);
    }
#line 9936 "MachineIndependent/glslang_tab.cpp"
    break;

  case 419: /* type_specifier_nonarray: UTEXTURE3D  */
#line 2918 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, Esd3D);
    }
#line 9946 "MachineIndependent/glslang_tab.cpp"
    break;

  case 420: /* type_specifier_nonarray: UTEXTURECUBE  */
#line 2923 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, EsdCube);
    }
#line 9956 "MachineIndependent/glslang_tab.cpp"
    break;

  case 421: /* type_specifier_nonarray: UTEXTURE2DARRAY  */
#line 2928 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, Esd2D, true);
    }
#line 9966 "MachineIndependent/glslang_tab.cpp"
    break;

  case 422: /* type_specifier_nonarray: SAMPLER  */
#line 2933 "MachineIndependent/glslang.y"
              {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setPureSampler(false);
    }
#line 9976 "MachineIndependent/glslang_tab.cpp"
    break;

  case 423: /* type_specifier_nonarray: SAMPLERSHADOW  */
#line 2938 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setPureSampler(true);
    }
#line 9986 "MachineIndependent/glslang_tab.cpp"
    break;

  case 424: /* type_specifier_nonarray: SAMPLER2DRECT  */
#line 2943 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, EsdRect);
    }
#line 9996 "MachineIndependent/glslang_tab.cpp"
    break;

  case 425: /* type_specifier_nonarray: SAMPLER2DRECTSHADOW  */
#line 2948 "MachineIndependent/glslang.y"
                          {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, EsdRect, false, true);
    }
#line 10006 "MachineIndependent/glslang_tab.cpp"
    break;

  case 426: /* type_specifier_nonarray: F16SAMPLER2DRECT  */
#line 2953 "MachineIndependent/glslang.y"
                       {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, EsdRect);
    }
#line 10017 "MachineIndependent/glslang_tab.cpp"
    break;

  case 427: /* type_specifier_nonarray: F16SAMPLER2DRECTSHADOW  */
#line 2959 "MachineIndependent/glslang.y"
                             {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, EsdRect, false, true);
    }
#line 10028 "MachineIndependent/glslang_tab.cpp"
    break;

  case 428: /* type_specifier_nonarray: ISAMPLER2DRECT  */
#line 2965 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, EsdRect);
    }
#line 10038 "MachineIndependent/glslang_tab.cpp"
    break;

  case 429: /* type_specifier_nonarray: USAMPLER2DRECT  */
#line 2970 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, EsdRect);
    }
#line 10048 "MachineIndependent/glslang_tab.cpp"
    break;

  case 430: /* type_specifier_nonarray: SAMPLERBUFFER  */
#line 2975 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, EsdBuffer);
    }
#line 10058 "MachineIndependent/glslang_tab.cpp"
    break;

  case 431: /* type_specifier_nonarray: F16SAMPLERBUFFER  */
#line 2980 "MachineIndependent/glslang.y"
                       {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, EsdBuffer);
    }
#line 10069 "MachineIndependent/glslang_tab.cpp"
    break;

  case 432: /* type_specifier_nonarray: ISAMPLERBUFFER  */
#line 2986 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, EsdBuffer);
    }
#line 10079 "MachineIndependent/glslang_tab.cpp"
    break;

  case 433: /* type_specifier_nonarray: USAMPLERBUFFER  */
#line 2991 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, EsdBuffer);
    }
#line 10089 "MachineIndependent/glslang_tab.cpp"
    break;

  case 434: /* type_specifier_nonarray: SAMPLER2DMS  */
#line 2996 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd2D, false, false, true);
    }
#line 10099 "MachineIndependent/glslang_tab.cpp"
    break;

  case 435: /* type_specifier_nonarray: F16SAMPLER2DMS  */
#line 3001 "MachineIndependent/glslang.y"
                     {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd2D, false, false, true);
    }
#line 10110 "MachineIndependent/glslang_tab.cpp"
    break;

  case 436: /* type_specifier_nonarray: ISAMPLER2DMS  */
#line 3007 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, Esd2D, false, false, true);
    }
#line 10120 "MachineIndependent/glslang_tab.cpp"
    break;

  case 437: /* type_specifier_nonarray: USAMPLER2DMS  */
#line 3012 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, Esd2D, false, false, true);
    }
#line 10130 "MachineIndependent/glslang_tab.cpp"
    break;

  case 438: /* type_specifier_nonarray: SAMPLER2DMSARRAY  */
#line 3017 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd2D, true, false, true);
    }
#line 10140 "MachineIndependent/glslang_tab.cpp"
    break;

  case 439: /* type_specifier_nonarray: F16SAMPLER2DMSARRAY  */
#line 3022 "MachineIndependent/glslang.y"
                          {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float sampler", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat16, Esd2D, true, false, true);
    }
#line 10151 "MachineIndependent/glslang_tab.cpp"
    break;

  case 440: /* type_specifier_nonarray: ISAMPLER2DMSARRAY  */
#line 3028 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtInt, Esd2D, true, false, true);
    }
#line 10161 "MachineIndependent/glslang_tab.cpp"
    break;

  case 441: /* type_specifier_nonarray: USAMPLER2DMSARRAY  */
#line 3033 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtUint, Esd2D, true, false, true);
    }
#line 10171 "MachineIndependent/glslang_tab.cpp"
    break;

  case 442: /* type_specifier_nonarray: TEXTURE1D  */
#line 3038 "MachineIndependent/glslang.y"
                {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, Esd1D);
    }
#line 10181 "MachineIndependent/glslang_tab.cpp"
    break;

  case 443: /* type_specifier_nonarray: F16TEXTURE1D  */
#line 3043 "MachineIndependent/glslang.y"
                   {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, Esd1D);
    }
#line 10192 "MachineIndependent/glslang_tab.cpp"
    break;

  case 444: /* type_specifier_nonarray: F16TEXTURE2D  */
#line 3049 "MachineIndependent/glslang.y"
                   {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, Esd2D);
    }
#line 10203 "MachineIndependent/glslang_tab.cpp"
    break;

  case 445: /* type_specifier_nonarray: F16TEXTURE3D  */
#line 3055 "MachineIndependent/glslang.y"
                   {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, Esd3D);
    }
#line 10214 "MachineIndependent/glslang_tab.cpp"
    break;

  case 446: /* type_specifier_nonarray: F16TEXTURECUBE  */
#line 3061 "MachineIndependent/glslang.y"
                     {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, EsdCube);
    }
#line 10225 "MachineIndependent/glslang_tab.cpp"
    break;

  case 447: /* type_specifier_nonarray: TEXTURE1DARRAY  */
#line 3067 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, Esd1D, true);
    }
#line 10235 "MachineIndependent/glslang_tab.cpp"
    break;

  case 448: /* type_specifier_nonarray: F16TEXTURE1DARRAY  */
#line 3072 "MachineIndependent/glslang.y"
                        {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, Esd1D, true);
    }
#line 10246 "MachineIndependent/glslang_tab.cpp"
    break;

  case 449: /* type_specifier_nonarray: F16TEXTURE2DARRAY  */
#line 3078 "MachineIndependent/glslang.y"
                        {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, Esd2D, true);
    }
#line 10257 "MachineIndependent/glslang_tab.cpp"
    break;

  case 450: /* type_specifier_nonarray: F16TEXTURECUBEARRAY  */
#line 3084 "MachineIndependent/glslang.y"
                          {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, EsdCube, true);
    }
#line 10268 "MachineIndependent/glslang_tab.cpp"
    break;

  case 451: /* type_specifier_nonarray: ITEXTURE1D  */
#line 3090 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, Esd1D);
    }
#line 10278 "MachineIndependent/glslang_tab.cpp"
    break;

  case 452: /* type_specifier_nonarray: ITEXTURE1DARRAY  */
#line 3095 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, Esd1D, true);
    }
#line 10288 "MachineIndependent/glslang_tab.cpp"
    break;

  case 453: /* type_specifier_nonarray: UTEXTURE1D  */
#line 3100 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, Esd1D);
    }
#line 10298 "MachineIndependent/glslang_tab.cpp"
    break;

  case 454: /* type_specifier_nonarray: UTEXTURE1DARRAY  */
#line 3105 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, Esd1D, true);
    }
#line 10308 "MachineIndependent/glslang_tab.cpp"
    break;

  case 455: /* type_specifier_nonarray: TEXTURE2DRECT  */
#line 3110 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, EsdRect);
    }
#line 10318 "MachineIndependent/glslang_tab.cpp"
    break;

  case 456: /* type_specifier_nonarray: F16TEXTURE2DRECT  */
#line 3115 "MachineIndependent/glslang.y"
                       {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, EsdRect);
    }
#line 10329 "MachineIndependent/glslang_tab.cpp"
    break;

  case 457: /* type_specifier_nonarray: ITEXTURE2DRECT  */
#line 3121 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, EsdRect);
    }
#line 10339 "MachineIndependent/glslang_tab.cpp"
    break;

  case 458: /* type_specifier_nonarray: UTEXTURE2DRECT  */
#line 3126 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, EsdRect);
    }
#line 10349 "MachineIndependent/glslang_tab.cpp"
    break;

  case 459: /* type_specifier_nonarray: TEXTUREBUFFER  */
#line 3131 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, EsdBuffer);
    }
#line 10359 "MachineIndependent/glslang_tab.cpp"
    break;

  case 460: /* type_specifier_nonarray: F16TEXTUREBUFFER  */
#line 3136 "MachineIndependent/glslang.y"
                       {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, EsdBuffer);
    }
#line 10370 "MachineIndependent/glslang_tab.cpp"
    break;

  case 461: /* type_specifier_nonarray: ITEXTUREBUFFER  */
#line 3142 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, EsdBuffer);
    }
#line 10380 "MachineIndependent/glslang_tab.cpp"
    break;

  case 462: /* type_specifier_nonarray: UTEXTUREBUFFER  */
#line 3147 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, EsdBuffer);
    }
#line 10390 "MachineIndependent/glslang_tab.cpp"
    break;

  case 463: /* type_specifier_nonarray: TEXTURE2DMS  */
#line 3152 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, Esd2D, false, false, true);
    }
#line 10400 "MachineIndependent/glslang_tab.cpp"
    break;

  case 464: /* type_specifier_nonarray: F16TEXTURE2DMS  */
#line 3157 "MachineIndependent/glslang.y"
                     {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, Esd2D, false, false, true);
    }
#line 10411 "MachineIndependent/glslang_tab.cpp"
    break;

  case 465: /* type_specifier_nonarray: ITEXTURE2DMS  */
#line 3163 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, Esd2D, false, false, true);
    }
#line 10421 "MachineIndependent/glslang_tab.cpp"
    break;

  case 466: /* type_specifier_nonarray: UTEXTURE2DMS  */
#line 3168 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, Esd2D, false, false, true);
    }
#line 10431 "MachineIndependent/glslang_tab.cpp"
    break;

  case 467: /* type_specifier_nonarray: TEXTURE2DMSARRAY  */
#line 3173 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat, Esd2D, true, false, true);
    }
#line 10441 "MachineIndependent/glslang_tab.cpp"
    break;

  case 468: /* type_specifier_nonarray: F16TEXTURE2DMSARRAY  */
#line 3178 "MachineIndependent/glslang.y"
                          {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float texture", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtFloat16, Esd2D, true, false, true);
    }
#line 10452 "MachineIndependent/glslang_tab.cpp"
    break;

  case 469: /* type_specifier_nonarray: ITEXTURE2DMSARRAY  */
#line 3184 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtInt, Esd2D, true, false, true);
    }
#line 10462 "MachineIndependent/glslang_tab.cpp"
    break;

  case 470: /* type_specifier_nonarray: UTEXTURE2DMSARRAY  */
#line 3189 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setTexture(EbtUint, Esd2D, true, false, true);
    }
#line 10472 "MachineIndependent/glslang_tab.cpp"
    break;

  case 471: /* type_specifier_nonarray: IMAGE1D  */
#line 3194 "MachineIndependent/glslang.y"
              {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, Esd1D);
    }
#line 10482 "MachineIndependent/glslang_tab.cpp"
    break;

  case 472: /* type_specifier_nonarray: F16IMAGE1D  */
#line 3199 "MachineIndependent/glslang.y"
                 {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, Esd1D);
    }
#line 10493 "MachineIndependent/glslang_tab.cpp"
    break;

  case 473: /* type_specifier_nonarray: IIMAGE1D  */
#line 3205 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, Esd1D);
    }
#line 10503 "MachineIndependent/glslang_tab.cpp"
    break;

  case 474: /* type_specifier_nonarray: UIMAGE1D  */
#line 3210 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, Esd1D);
    }
#line 10513 "MachineIndependent/glslang_tab.cpp"
    break;

  case 475: /* type_specifier_nonarray: IMAGE2D  */
#line 3215 "MachineIndependent/glslang.y"
              {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, Esd2D);
    }
#line 10523 "MachineIndependent/glslang_tab.cpp"
    break;

  case 476: /* type_specifier_nonarray: F16IMAGE2D  */
#line 3220 "MachineIndependent/glslang.y"
                 {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, Esd2D);
    }
#line 10534 "MachineIndependent/glslang_tab.cpp"
    break;

  case 477: /* type_specifier_nonarray: IIMAGE2D  */
#line 3226 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, Esd2D);
    }
#line 10544 "MachineIndependent/glslang_tab.cpp"
    break;

  case 478: /* type_specifier_nonarray: UIMAGE2D  */
#line 3231 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, Esd2D);
    }
#line 10554 "MachineIndependent/glslang_tab.cpp"
    break;

  case 479: /* type_specifier_nonarray: IMAGE3D  */
#line 3236 "MachineIndependent/glslang.y"
              {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, Esd3D);
    }
#line 10564 "MachineIndependent/glslang_tab.cpp"
    break;

  case 480: /* type_specifier_nonarray: F16IMAGE3D  */
#line 3241 "MachineIndependent/glslang.y"
                 {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, Esd3D);
    }
#line 10575 "MachineIndependent/glslang_tab.cpp"
    break;

  case 481: /* type_specifier_nonarray: IIMAGE3D  */
#line 3247 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, Esd3D);
    }
#line 10585 "MachineIndependent/glslang_tab.cpp"
    break;

  case 482: /* type_specifier_nonarray: UIMAGE3D  */
#line 3252 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, Esd3D);
    }
#line 10595 "MachineIndependent/glslang_tab.cpp"
    break;

  case 483: /* type_specifier_nonarray: IMAGE2DRECT  */
#line 3257 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, EsdRect);
    }
#line 10605 "MachineIndependent/glslang_tab.cpp"
    break;

  case 484: /* type_specifier_nonarray: F16IMAGE2DRECT  */
#line 3262 "MachineIndependent/glslang.y"
                     {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, EsdRect);
    }
#line 10616 "MachineIndependent/glslang_tab.cpp"
    break;

  case 485: /* type_specifier_nonarray: IIMAGE2DRECT  */
#line 3268 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, EsdRect);
    }
#line 10626 "MachineIndependent/glslang_tab.cpp"
    break;

  case 486: /* type_specifier_nonarray: UIMAGE2DRECT  */
#line 3273 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, EsdRect);
    }
#line 10636 "MachineIndependent/glslang_tab.cpp"
    break;

  case 487: /* type_specifier_nonarray: IMAGECUBE  */
#line 3278 "MachineIndependent/glslang.y"
                {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, EsdCube);
    }
#line 10646 "MachineIndependent/glslang_tab.cpp"
    break;

  case 488: /* type_specifier_nonarray: F16IMAGECUBE  */
#line 3283 "MachineIndependent/glslang.y"
                   {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, EsdCube);
    }
#line 10657 "MachineIndependent/glslang_tab.cpp"
    break;

  case 489: /* type_specifier_nonarray: IIMAGECUBE  */
#line 3289 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, EsdCube);
    }
#line 10667 "MachineIndependent/glslang_tab.cpp"
    break;

  case 490: /* type_specifier_nonarray: UIMAGECUBE  */
#line 3294 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, EsdCube);
    }
#line 10677 "MachineIndependent/glslang_tab.cpp"
    break;

  case 491: /* type_specifier_nonarray: IMAGEBUFFER  */
#line 3299 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, EsdBuffer);
    }
#line 10687 "MachineIndependent/glslang_tab.cpp"
    break;

  case 492: /* type_specifier_nonarray: F16IMAGEBUFFER  */
#line 3304 "MachineIndependent/glslang.y"
                     {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, EsdBuffer);
    }
#line 10698 "MachineIndependent/glslang_tab.cpp"
    break;

  case 493: /* type_specifier_nonarray: IIMAGEBUFFER  */
#line 3310 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, EsdBuffer);
    }
#line 10708 "MachineIndependent/glslang_tab.cpp"
    break;

  case 494: /* type_specifier_nonarray: UIMAGEBUFFER  */
#line 3315 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, EsdBuffer);
    }
#line 10718 "MachineIndependent/glslang_tab.cpp"
    break;

  case 495: /* type_specifier_nonarray: IMAGE1DARRAY  */
#line 3320 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, Esd1D, true);
    }
#line 10728 "MachineIndependent/glslang_tab.cpp"
    break;

  case 496: /* type_specifier_nonarray: F16IMAGE1DARRAY  */
#line 3325 "MachineIndependent/glslang.y"
                      {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, Esd1D, true);
    }
#line 10739 "MachineIndependent/glslang_tab.cpp"
    break;

  case 497: /* type_specifier_nonarray: IIMAGE1DARRAY  */
#line 3331 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, Esd1D, true);
    }
#line 10749 "MachineIndependent/glslang_tab.cpp"
    break;

  case 498: /* type_specifier_nonarray: UIMAGE1DARRAY  */
#line 3336 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, Esd1D, true);
    }
#line 10759 "MachineIndependent/glslang_tab.cpp"
    break;

  case 499: /* type_specifier_nonarray: IMAGE2DARRAY  */
#line 3341 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, Esd2D, true);
    }
#line 10769 "MachineIndependent/glslang_tab.cpp"
    break;

  case 500: /* type_specifier_nonarray: F16IMAGE2DARRAY  */
#line 3346 "MachineIndependent/glslang.y"
                      {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, Esd2D, true);
    }
#line 10780 "MachineIndependent/glslang_tab.cpp"
    break;

  case 501: /* type_specifier_nonarray: IIMAGE2DARRAY  */
#line 3352 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, Esd2D, true);
    }
#line 10790 "MachineIndependent/glslang_tab.cpp"
    break;

  case 502: /* type_specifier_nonarray: UIMAGE2DARRAY  */
#line 3357 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, Esd2D, true);
    }
#line 10800 "MachineIndependent/glslang_tab.cpp"
    break;

  case 503: /* type_specifier_nonarray: IMAGECUBEARRAY  */
#line 3362 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, EsdCube, true);
    }
#line 10810 "MachineIndependent/glslang_tab.cpp"
    break;

  case 504: /* type_specifier_nonarray: F16IMAGECUBEARRAY  */
#line 3367 "MachineIndependent/glslang.y"
                        {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, EsdCube, true);
    }
#line 10821 "MachineIndependent/glslang_tab.cpp"
    break;

  case 505: /* type_specifier_nonarray: IIMAGECUBEARRAY  */
#line 3373 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, EsdCube, true);
    }
#line 10831 "MachineIndependent/glslang_tab.cpp"
    break;

  case 506: /* type_specifier_nonarray: UIMAGECUBEARRAY  */
#line 3378 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, EsdCube, true);
    }
#line 10841 "MachineIndependent/glslang_tab.cpp"
    break;

  case 507: /* type_specifier_nonarray: IMAGE2DMS  */
#line 3383 "MachineIndependent/glslang.y"
                {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, Esd2D, false, false, true);
    }
#line 10851 "MachineIndependent/glslang_tab.cpp"
    break;

  case 508: /* type_specifier_nonarray: F16IMAGE2DMS  */
#line 3388 "MachineIndependent/glslang.y"
                   {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, Esd2D, false, false, true);
    }
#line 10862 "MachineIndependent/glslang_tab.cpp"
    break;

  case 509: /* type_specifier_nonarray: IIMAGE2DMS  */
#line 3394 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, Esd2D, false, false, true);
    }
#line 10872 "MachineIndependent/glslang_tab.cpp"
    break;

  case 510: /* type_specifier_nonarray: UIMAGE2DMS  */
#line 3399 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, Esd2D, false, false, true);
    }
#line 10882 "MachineIndependent/glslang_tab.cpp"
    break;

  case 511: /* type_specifier_nonarray: IMAGE2DMSARRAY  */
#line 3404 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat, Esd2D, true, false, true);
    }
#line 10892 "MachineIndependent/glslang_tab.cpp"
    break;

  case 512: /* type_specifier_nonarray: F16IMAGE2DMSARRAY  */
#line 3409 "MachineIndependent/glslang.y"
                        {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float image", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtFloat16, Esd2D, true, false, true);
    }
#line 10903 "MachineIndependent/glslang_tab.cpp"
    break;

  case 513: /* type_specifier_nonarray: IIMAGE2DMSARRAY  */
#line 3415 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt, Esd2D, true, false, true);
    }
#line 10913 "MachineIndependent/glslang_tab.cpp"
    break;

  case 514: /* type_specifier_nonarray: UIMAGE2DMSARRAY  */
#line 3420 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint, Esd2D, true, false, true);
    }
#line 10923 "MachineIndependent/glslang_tab.cpp"
    break;

  case 515: /* type_specifier_nonarray: I64IMAGE1D  */
#line 3425 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, Esd1D);
    }
#line 10933 "MachineIndependent/glslang_tab.cpp"
    break;

  case 516: /* type_specifier_nonarray: U64IMAGE1D  */
#line 3430 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, Esd1D);
    }
#line 10943 "MachineIndependent/glslang_tab.cpp"
    break;

  case 517: /* type_specifier_nonarray: I64IMAGE2D  */
#line 3435 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, Esd2D);
    }
#line 10953 "MachineIndependent/glslang_tab.cpp"
    break;

  case 518: /* type_specifier_nonarray: U64IMAGE2D  */
#line 3440 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, Esd2D);
    }
#line 10963 "MachineIndependent/glslang_tab.cpp"
    break;

  case 519: /* type_specifier_nonarray: I64IMAGE3D  */
#line 3445 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, Esd3D);
    }
#line 10973 "MachineIndependent/glslang_tab.cpp"
    break;

  case 520: /* type_specifier_nonarray: U64IMAGE3D  */
#line 3450 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, Esd3D);
    }
#line 10983 "MachineIndependent/glslang_tab.cpp"
    break;

  case 521: /* type_specifier_nonarray: I64IMAGE2DRECT  */
#line 3455 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, EsdRect);
    }
#line 10993 "MachineIndependent/glslang_tab.cpp"
    break;

  case 522: /* type_specifier_nonarray: U64IMAGE2DRECT  */
#line 3460 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, EsdRect);
    }
#line 11003 "MachineIndependent/glslang_tab.cpp"
    break;

  case 523: /* type_specifier_nonarray: I64IMAGECUBE  */
#line 3465 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, EsdCube);
    }
#line 11013 "MachineIndependent/glslang_tab.cpp"
    break;

  case 524: /* type_specifier_nonarray: U64IMAGECUBE  */
#line 3470 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, EsdCube);
    }
#line 11023 "MachineIndependent/glslang_tab.cpp"
    break;

  case 525: /* type_specifier_nonarray: I64IMAGEBUFFER  */
#line 3475 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, EsdBuffer);
    }
#line 11033 "MachineIndependent/glslang_tab.cpp"
    break;

  case 526: /* type_specifier_nonarray: U64IMAGEBUFFER  */
#line 3480 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, EsdBuffer);
    }
#line 11043 "MachineIndependent/glslang_tab.cpp"
    break;

  case 527: /* type_specifier_nonarray: I64IMAGE1DARRAY  */
#line 3485 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, Esd1D, true);
    }
#line 11053 "MachineIndependent/glslang_tab.cpp"
    break;

  case 528: /* type_specifier_nonarray: U64IMAGE1DARRAY  */
#line 3490 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, Esd1D, true);
    }
#line 11063 "MachineIndependent/glslang_tab.cpp"
    break;

  case 529: /* type_specifier_nonarray: I64IMAGE2DARRAY  */
#line 3495 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, Esd2D, true);
    }
#line 11073 "MachineIndependent/glslang_tab.cpp"
    break;

  case 530: /* type_specifier_nonarray: U64IMAGE2DARRAY  */
#line 3500 "MachineIndependent/glslang.y"
                      {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, Esd2D, true);
    }
#line 11083 "MachineIndependent/glslang_tab.cpp"
    break;

  case 531: /* type_specifier_nonarray: I64IMAGECUBEARRAY  */
#line 3505 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, EsdCube, true);
    }
#line 11093 "MachineIndependent/glslang_tab.cpp"
    break;

  case 532: /* type_specifier_nonarray: U64IMAGECUBEARRAY  */
#line 3510 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, EsdCube, true);
    }
#line 11103 "MachineIndependent/glslang_tab.cpp"
    break;

  case 533: /* type_specifier_nonarray: I64IMAGE2DMS  */
#line 3515 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, Esd2D, false, false, true);
    }
#line 11113 "MachineIndependent/glslang_tab.cpp"
    break;

  case 534: /* type_specifier_nonarray: U64IMAGE2DMS  */
#line 3520 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, Esd2D, false, false, true);
    }
#line 11123 "MachineIndependent/glslang_tab.cpp"
    break;

  case 535: /* type_specifier_nonarray: I64IMAGE2DMSARRAY  */
#line 3525 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtInt64, Esd2D, true, false, true);
    }
#line 11133 "MachineIndependent/glslang_tab.cpp"
    break;

  case 536: /* type_specifier_nonarray: U64IMAGE2DMSARRAY  */
#line 3530 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setImage(EbtUint64, Esd2D, true, false, true);
    }
#line 11143 "MachineIndependent/glslang_tab.cpp"
    break;

  case 537: /* type_specifier_nonarray: SAMPLEREXTERNALOES  */
#line 3535 "MachineIndependent/glslang.y"
                         {  // GL_OES_EGL_image_external
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd2D);
        (yyval.interm.type).sampler.external = true;
    }
#line 11154 "MachineIndependent/glslang_tab.cpp"
    break;

  case 538: /* type_specifier_nonarray: SAMPLEREXTERNAL2DY2YEXT  */
#line 3541 "MachineIndependent/glslang.y"
                              { // GL_EXT_YUV_target
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.set(EbtFloat, Esd2D);
        (yyval.interm.type).sampler.yuv = true;
    }
#line 11165 "MachineIndependent/glslang_tab.cpp"
    break;

  case 539: /* type_specifier_nonarray: ATTACHMENTEXT  */
#line 3547 "MachineIndependent/glslang.y"
                    {
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "attachmentEXT input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setAttachmentEXT(EbtFloat);
    }
#line 11176 "MachineIndependent/glslang_tab.cpp"
    break;

  case 540: /* type_specifier_nonarray: IATTACHMENTEXT  */
#line 3553 "MachineIndependent/glslang.y"
                     {
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "attachmentEXT input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setAttachmentEXT(EbtInt);
    }
#line 11187 "MachineIndependent/glslang_tab.cpp"
    break;

  case 541: /* type_specifier_nonarray: UATTACHMENTEXT  */
#line 3559 "MachineIndependent/glslang.y"
                     {
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "attachmentEXT input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setAttachmentEXT(EbtUint);
    }
#line 11198 "MachineIndependent/glslang_tab.cpp"
    break;

  case 542: /* type_specifier_nonarray: SUBPASSINPUT  */
#line 3565 "MachineIndependent/glslang.y"
                   {
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "subpass input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setSubpass(EbtFloat);
    }
#line 11209 "MachineIndependent/glslang_tab.cpp"
    break;

  case 543: /* type_specifier_nonarray: SUBPASSINPUTMS  */
#line 3571 "MachineIndependent/glslang.y"
                     {
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "subpass input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setSubpass(EbtFloat, true);
    }
#line 11220 "MachineIndependent/glslang_tab.cpp"
    break;

  case 544: /* type_specifier_nonarray: F16SUBPASSINPUT  */
#line 3577 "MachineIndependent/glslang.y"
                      {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float subpass input", parseContext.symbolTable.atBuiltInLevel());
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "subpass input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setSubpass(EbtFloat16);
    }
#line 11232 "MachineIndependent/glslang_tab.cpp"
    break;

  case 545: /* type_specifier_nonarray: F16SUBPASSINPUTMS  */
#line 3584 "MachineIndependent/glslang.y"
                        {
        parseContext.float16OpaqueCheck((yyvsp[0].lex).loc, "half float subpass input", parseContext.symbolTable.atBuiltInLevel());
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "subpass input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setSubpass(EbtFloat16, true);
    }
#line 11244 "MachineIndependent/glslang_tab.cpp"
    break;

  case 546: /* type_specifier_nonarray: ISUBPASSINPUT  */
#line 3591 "MachineIndependent/glslang.y"
                    {
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "subpass input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setSubpass(EbtInt);
    }
#line 11255 "MachineIndependent/glslang_tab.cpp"
    break;

  case 547: /* type_specifier_nonarray: ISUBPASSINPUTMS  */
#line 3597 "MachineIndependent/glslang.y"
                      {
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "subpass input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setSubpass(EbtInt, true);
    }
#line 11266 "MachineIndependent/glslang_tab.cpp"
    break;

  case 548: /* type_specifier_nonarray: USUBPASSINPUT  */
#line 3603 "MachineIndependent/glslang.y"
                    {
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "subpass input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setSubpass(EbtUint);
    }
#line 11277 "MachineIndependent/glslang_tab.cpp"
    break;

  case 549: /* type_specifier_nonarray: USUBPASSINPUTMS  */
#line 3609 "MachineIndependent/glslang.y"
                      {
        parseContext.requireStage((yyvsp[0].lex).loc, EShLangFragment, "subpass input");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtSampler;
        (yyval.interm.type).sampler.setSubpass(EbtUint, true);
    }
#line 11288 "MachineIndependent/glslang_tab.cpp"
    break;

  case 550: /* type_specifier_nonarray: FCOOPMATNV  */
#line 3615 "MachineIndependent/glslang.y"
                 {
        parseContext.fcoopmatCheckNV((yyvsp[0].lex).loc, "fcoopmatNV", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtFloat;
        (yyval.interm.type).coopmatNV = true;
        (yyval.interm.type).coopmatKHR = false;
    }
#line 11300 "MachineIndependent/glslang_tab.cpp"
    break;

  case 551: /* type_specifier_nonarray: ICOOPMATNV  */
#line 3622 "MachineIndependent/glslang.y"
                 {
        parseContext.intcoopmatCheckNV((yyvsp[0].lex).loc, "icoopmatNV", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtInt;
        (yyval.interm.type).coopmatNV = true;
        (yyval.interm.type).coopmatKHR = false;
    }
#line 11312 "MachineIndependent/glslang_tab.cpp"
    break;

  case 552: /* type_specifier_nonarray: UCOOPMATNV  */
#line 3629 "MachineIndependent/glslang.y"
                 {
        parseContext.intcoopmatCheckNV((yyvsp[0].lex).loc, "ucoopmatNV", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtUint;
        (yyval.interm.type).coopmatNV = true;
        (yyval.interm.type).coopmatKHR = false;
    }
#line 11324 "MachineIndependent/glslang_tab.cpp"
    break;

  case 553: /* type_specifier_nonarray: COOPMAT  */
#line 3636 "MachineIndependent/glslang.y"
              {
        parseContext.coopmatCheck((yyvsp[0].lex).loc, "coopmat", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtCoopmat;
        (yyval.interm.type).coopmatNV = false;
        (yyval.interm.type).coopmatKHR = true;
    }
#line 11336 "MachineIndependent/glslang_tab.cpp"
    break;

  case 554: /* type_specifier_nonarray: TENSORLAYOUTNV  */
#line 3643 "MachineIndependent/glslang.y"
                     {
        parseContext.tensorLayoutViewCheck((yyvsp[0].lex).loc, "tensorLayoutNV", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtTensorLayoutNV;
    }
#line 11346 "MachineIndependent/glslang_tab.cpp"
    break;

  case 555: /* type_specifier_nonarray: TENSORVIEWNV  */
#line 3648 "MachineIndependent/glslang.y"
                   {
        parseContext.tensorLayoutViewCheck((yyvsp[0].lex).loc, "tensorViewNV", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtTensorViewNV;
    }
#line 11356 "MachineIndependent/glslang_tab.cpp"
    break;

  case 556: /* type_specifier_nonarray: FUNCTION  */
#line 3653 "MachineIndependent/glslang.y"
               {
        (yyval.interm.type).init((yyvsp[0].lex).loc);
        (yyval.interm.type).basicType = EbtFunction;
    }
#line 11365 "MachineIndependent/glslang_tab.cpp"
    break;

  case 557: /* type_specifier_nonarray: COOPVECNV  */
#line 3657 "MachineIndependent/glslang.y"
                {
        parseContext.coopvecCheck((yyvsp[0].lex).loc, "coopvecNV", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtCoopvecNV;
        (yyval.interm.type).coopvecNV = true;
    }
#line 11376 "MachineIndependent/glslang_tab.cpp"
    break;

  case 558: /* type_specifier_nonarray: TENSORARM  */
#line 3663 "MachineIndependent/glslang.y"
                {
        parseContext.tensorCheckARM((yyvsp[0].lex).loc, "tensorARM", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).tensorRankARM = 1; // placeholder value
        (yyval.interm.type).basicType = EbtTensorARM;
    }
#line 11387 "MachineIndependent/glslang_tab.cpp"
    break;

  case 559: /* type_specifier_nonarray: VECTOR  */
#line 3669 "MachineIndependent/glslang.y"
             {
        parseContext.longVectorCheck((yyvsp[0].lex).loc, "vector", parseContext.symbolTable.atBuiltInLevel());
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).basicType = EbtLongVector;
        (yyval.interm.type).longVector = true;
    }
#line 11398 "MachineIndependent/glslang_tab.cpp"
    break;

  case 560: /* type_specifier_nonarray: spirv_type_specifier  */
#line 3675 "MachineIndependent/glslang.y"
                           {
        parseContext.requireExtensions((yyvsp[0].interm.type).loc, 1, &E_GL_EXT_spirv_intrinsics, "SPIR-V type specifier");
        (yyval.interm.type) = (yyvsp[0].interm.type);
    }
#line 11407 "MachineIndependent/glslang_tab.cpp"
    break;

  case 561: /* type_specifier_nonarray: HITOBJECTNV  */
#line 3679 "MachineIndependent/glslang.y"
                  {
       (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
       (yyval.interm.type).basicType = EbtHitObjectNV;
    }
#line 11416 "MachineIndependent/glslang_tab.cpp"
    break;

  case 562: /* type_specifier_nonarray: HITOBJECTEXT  */
#line 3683 "MachineIndependent/glslang.y"
                   {
       (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
       (yyval.interm.type).basicType = EbtHitObjectEXT;
    }
#line 11425 "MachineIndependent/glslang_tab.cpp"
    break;

  case 563: /* type_specifier_nonarray: struct_specifier  */
#line 3687 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.type) = (yyvsp[0].interm.type);
        (yyval.interm.type).qualifier.storage = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        parseContext.structTypeCheck((yyval.interm.type).loc, (yyval.interm.type));
    }
#line 11435 "MachineIndependent/glslang_tab.cpp"
    break;

  case 564: /* type_specifier_nonarray: TYPE_NAME  */
#line 3692 "MachineIndependent/glslang.y"
                {
        //
        // This is for user defined type names.  The lexical phase looked up the
        // type.
        //
        if (const TVariable* variable = ((yyvsp[0].lex).symbol)->getAsVariable()) {
            const TType& structure = variable->getType();
            (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
            (yyval.interm.type).basicType = EbtStruct;
            (yyval.interm.type).userDef = &structure;
        } else
            parseContext.error((yyvsp[0].lex).loc, "expected type name", (yyvsp[0].lex).string->c_str(), "");
    }
#line 11453 "MachineIndependent/glslang_tab.cpp"
    break;

  case 565: /* precision_qualifier: HIGH_PRECISION  */
#line 3708 "MachineIndependent/glslang.y"
                     {
        parseContext.profileRequires((yyvsp[0].lex).loc, ENoProfile, 130, 0, "highp precision qualifier");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        parseContext.handlePrecisionQualifier((yyvsp[0].lex).loc, (yyval.interm.type).qualifier, EpqHigh);
    }
#line 11463 "MachineIndependent/glslang_tab.cpp"
    break;

  case 566: /* precision_qualifier: MEDIUM_PRECISION  */
#line 3713 "MachineIndependent/glslang.y"
                       {
        parseContext.profileRequires((yyvsp[0].lex).loc, ENoProfile, 130, 0, "mediump precision qualifier");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        parseContext.handlePrecisionQualifier((yyvsp[0].lex).loc, (yyval.interm.type).qualifier, EpqMedium);
    }
#line 11473 "MachineIndependent/glslang_tab.cpp"
    break;

  case 567: /* precision_qualifier: LOW_PRECISION  */
#line 3718 "MachineIndependent/glslang.y"
                    {
        parseContext.profileRequires((yyvsp[0].lex).loc, ENoProfile, 130, 0, "lowp precision qualifier");
        (yyval.interm.type).init((yyvsp[0].lex).loc, parseContext.symbolTable.atGlobalLevel());
        parseContext.handlePrecisionQualifier((yyvsp[0].lex).loc, (yyval.interm.type).qualifier, EpqLow);
    }
#line 11483 "MachineIndependent/glslang_tab.cpp"
    break;

  case 568: /* $@3: %empty  */
#line 3726 "MachineIndependent/glslang.y"
                                   { parseContext.nestedStructCheck((yyvsp[-2].lex).loc); }
#line 11489 "MachineIndependent/glslang_tab.cpp"
    break;

  case 569: /* struct_specifier: STRUCT IDENTIFIER LEFT_BRACE $@3 struct_declaration_list RIGHT_BRACE  */
#line 3726 "MachineIndependent/glslang.y"
                                                                                                                   {
        TType* structure = new TType((yyvsp[-1].interm.typeList), *(yyvsp[-4].lex).string);
        parseContext.structArrayCheck((yyvsp[-4].lex).loc, *structure);

        TVariable* userTypeDef = new TVariable((yyvsp[-4].lex).string, *structure, true);
        if (! parseContext.symbolTable.insert(*userTypeDef))
            parseContext.error((yyvsp[-4].lex).loc, "redefinition", (yyvsp[-4].lex).string->c_str(), "struct");
        else if (parseContext.spvVersion.vulkanRelaxed
                 && structure->containsOpaque())
            parseContext.relaxedSymbols.push_back(structure->getTypeName());

        (yyval.interm.type).init((yyvsp[-5].lex).loc);
        (yyval.interm.type).basicType = EbtStruct;
        (yyval.interm.type).userDef = structure;
        --parseContext.structNestingLevel;
    }
#line 11510 "MachineIndependent/glslang_tab.cpp"
    break;

  case 570: /* $@4: %empty  */
#line 3742 "MachineIndependent/glslang.y"
                        { parseContext.nestedStructCheck((yyvsp[-1].lex).loc); }
#line 11516 "MachineIndependent/glslang_tab.cpp"
    break;

  case 571: /* struct_specifier: STRUCT LEFT_BRACE $@4 struct_declaration_list RIGHT_BRACE  */
#line 3742 "MachineIndependent/glslang.y"
                                                                                                        {
        TType* structure = new TType((yyvsp[-1].interm.typeList), TString(""));
        (yyval.interm.type).init((yyvsp[-4].lex).loc);
        (yyval.interm.type).basicType = EbtStruct;
        (yyval.interm.type).userDef = structure;
        --parseContext.structNestingLevel;
    }
#line 11528 "MachineIndependent/glslang_tab.cpp"
    break;

  case 572: /* struct_declaration_list: struct_declaration_without_heap  */
#line 3752 "MachineIndependent/glslang.y"
                                      {
        (yyval.interm.typeList) = (yyvsp[0].interm.typeList);
    }
#line 11536 "MachineIndependent/glslang_tab.cpp"
    break;

  case 573: /* struct_declaration_list: struct_declaration_with_heap  */
#line 3755 "MachineIndependent/glslang.y"
                                   {
        (yyval.interm.typeList) = (yyvsp[0].interm.typeList);
    }
#line 11544 "MachineIndependent/glslang_tab.cpp"
    break;

  case 574: /* struct_declaration_list: struct_declaration_with_heap struct_declaration_without_heap  */
#line 3758 "MachineIndependent/glslang.y"
                                                                   {
        (yyval.interm.typeList) = (yyvsp[-1].interm.typeList);
        for (unsigned int i = 0; i < (yyvsp[0].interm.typeList)->size(); ++i) {
            for (unsigned int j = 0; j < (yyval.interm.typeList)->size(); ++j) {
                if ((*(yyval.interm.typeList))[j].type->getFieldName() == (*(yyvsp[0].interm.typeList))[i].type->getFieldName())
                    parseContext.error((*(yyvsp[0].interm.typeList))[i].loc, "duplicate member name:", "", (*(yyvsp[0].interm.typeList))[i].type->getFieldName().c_str());
            }
            (yyval.interm.typeList)->push_back((*(yyvsp[0].interm.typeList))[i]);
        }
    }
#line 11559 "MachineIndependent/glslang_tab.cpp"
    break;

  case 575: /* struct_declaration_list: struct_declaration_without_heap struct_declaration_with_heap  */
#line 3768 "MachineIndependent/glslang.y"
                                                                   {
        (yyval.interm.typeList) = (yyvsp[-1].interm.typeList);
        for (unsigned int i = 0; i < (yyvsp[0].interm.typeList)->size(); ++i) {
            for (unsigned int j = 0; j < (yyval.interm.typeList)->size(); ++j) {
                if ((*(yyval.interm.typeList))[j].type->getFieldName() == (*(yyvsp[0].interm.typeList))[i].type->getFieldName())
                    parseContext.error((*(yyvsp[0].interm.typeList))[i].loc, "duplicate member name:", "", (*(yyvsp[0].interm.typeList))[i].type->getFieldName().c_str());
            }
            (yyval.interm.typeList)->push_back((*(yyvsp[0].interm.typeList))[i]);
        }
    }
#line 11574 "MachineIndependent/glslang_tab.cpp"
    break;

  case 576: /* struct_declaration_with_heap: block_heap_inner_structure struct_declarator_list SEMICOLON  */
#line 3781 "MachineIndependent/glslang.y"
                                                                  {
        (yyval.interm.typeList) = (yyvsp[-1].interm.typeList);
        parseContext.voidErrorCheck((yyvsp[-2].interm.type).loc, (*(yyvsp[-1].interm.typeList))[0].type->getFieldName(), (yyvsp[-2].interm.type).basicType);
        parseContext.precisionQualifierCheck((yyvsp[-2].interm.type).loc, (yyvsp[-2].interm.type).basicType, (yyvsp[-2].interm.type).qualifier, (yyvsp[-2].interm.type).hasTypeParameter());

        for (unsigned int i = 0; i < (yyval.interm.typeList)->size(); ++i) {
            TType type((yyvsp[-2].interm.type));
            type.setFieldName((*(yyval.interm.typeList))[i].type->getFieldName());
            type.transferArraySizes((*(yyval.interm.typeList))[i].type->getArraySizes());
            type.copyArrayInnerSizes((yyvsp[-2].interm.type).arraySizes);
            parseContext.arrayOfArrayVersionCheck((*(yyval.interm.typeList))[i].loc, type.getArraySizes());
            (*(yyval.interm.typeList))[i].type->shallowCopy(type);
        }
    }
#line 11593 "MachineIndependent/glslang_tab.cpp"
    break;

  case 577: /* $@5: %empty  */
#line 3798 "MachineIndependent/glslang.y"
                                { parseContext.nestedBlockCheck((yyvsp[-1].interm.type).loc, true); }
#line 11599 "MachineIndependent/glslang_tab.cpp"
    break;

  case 578: /* block_heap_inner_structure: type_qualifier LEFT_BRACE $@5 struct_declaration_without_heap RIGHT_BRACE  */
#line 3798 "MachineIndependent/glslang.y"
                                                                                                                             {
        --parseContext.blockNestingLevel;
        parseContext.globalQualifierFixCheck((yyvsp[-4].interm.type).loc, (yyvsp[-4].interm.type).qualifier);
        parseContext.checkNoShaderLayouts((yyvsp[-4].interm.type).loc, (yyvsp[-4].interm.type).shaderQualifiers);
        (yyval.interm.type).init((yyvsp[-4].interm.type).loc);
        TType* innerStructure = new TType((yyvsp[-1].interm.typeList), TString(""));
        (yyval.interm.type).basicType = EbtBlock;
        (yyval.interm.type).userDef = innerStructure;
        (yyval.interm.type).qualifier = (yyvsp[-4].interm.type).qualifier;
        (yyval.interm.type).qualifier.layoutDescriptorHeap = true;
        (yyval.interm.type).qualifier.layoutDescriptorInnerBlock = true;
    }
#line 11616 "MachineIndependent/glslang_tab.cpp"
    break;

  case 579: /* struct_declaration_without_heap: struct_declaration  */
#line 3813 "MachineIndependent/glslang.y"
                         {
        (yyval.interm.typeList) = (yyvsp[0].interm.typeList);
    }
#line 11624 "MachineIndependent/glslang_tab.cpp"
    break;

  case 580: /* struct_declaration_without_heap: struct_declaration_without_heap struct_declaration  */
#line 3816 "MachineIndependent/glslang.y"
                                                         {
        (yyval.interm.typeList) = (yyvsp[-1].interm.typeList);
        for (unsigned int i = 0; i < (yyvsp[0].interm.typeList)->size(); ++i) {
            for (unsigned int j = 0; j < (yyval.interm.typeList)->size(); ++j) {
                if ((*(yyval.interm.typeList))[j].type->getFieldName() == (*(yyvsp[0].interm.typeList))[i].type->getFieldName())
                    parseContext.error((*(yyvsp[0].interm.typeList))[i].loc, "duplicate member name:", "", (*(yyvsp[0].interm.typeList))[i].type->getFieldName().c_str());
            }
            (yyval.interm.typeList)->push_back((*(yyvsp[0].interm.typeList))[i]);
        }
    }
#line 11639 "MachineIndependent/glslang_tab.cpp"
    break;

  case 581: /* struct_declaration: type_specifier struct_declarator_list SEMICOLON  */
#line 3829 "MachineIndependent/glslang.y"
                                                      {
        if ((yyvsp[-2].interm.type).arraySizes) {
            parseContext.profileRequires((yyvsp[-2].interm.type).loc, ENoProfile, 120, E_GL_3DL_array_objects, "arrayed type");
            parseContext.profileRequires((yyvsp[-2].interm.type).loc, EEsProfile, 300, 0, "arrayed type");
            if (parseContext.isEsProfile())
                parseContext.arraySizeRequiredCheck((yyvsp[-2].interm.type).loc, *(yyvsp[-2].interm.type).arraySizes);
        }

        (yyval.interm.typeList) = (yyvsp[-1].interm.typeList);

        parseContext.voidErrorCheck((yyvsp[-2].interm.type).loc, (*(yyvsp[-1].interm.typeList))[0].type->getFieldName(), (yyvsp[-2].interm.type).basicType);
        parseContext.precisionQualifierCheck((yyvsp[-2].interm.type).loc, (yyvsp[-2].interm.type).basicType, (yyvsp[-2].interm.type).qualifier, (yyvsp[-2].interm.type).hasTypeParameter());

        for (unsigned int i = 0; i < (yyval.interm.typeList)->size(); ++i) {
            TType type((yyvsp[-2].interm.type));
            type.setFieldName((*(yyval.interm.typeList))[i].type->getFieldName());
            type.transferArraySizes((*(yyval.interm.typeList))[i].type->getArraySizes());
            type.copyArrayInnerSizes((yyvsp[-2].interm.type).arraySizes);
            parseContext.arrayOfArrayVersionCheck((*(yyval.interm.typeList))[i].loc, type.getArraySizes());
            (*(yyval.interm.typeList))[i].type->shallowCopy(type);
        }
    }
#line 11666 "MachineIndependent/glslang_tab.cpp"
    break;

  case 582: /* struct_declaration: type_qualifier type_specifier struct_declarator_list SEMICOLON  */
#line 3851 "MachineIndependent/glslang.y"
                                                                     {
        if ((yyvsp[-2].interm.type).arraySizes) {
            parseContext.profileRequires((yyvsp[-2].interm.type).loc, ENoProfile, 120, E_GL_3DL_array_objects, "arrayed type");
            parseContext.profileRequires((yyvsp[-2].interm.type).loc, EEsProfile, 300, 0, "arrayed type");
            if (parseContext.isEsProfile())
                parseContext.arraySizeRequiredCheck((yyvsp[-2].interm.type).loc, *(yyvsp[-2].interm.type).arraySizes);
        }

        (yyval.interm.typeList) = (yyvsp[-1].interm.typeList);

        parseContext.memberQualifierCheck((yyvsp[-3].interm.type));
        parseContext.voidErrorCheck((yyvsp[-2].interm.type).loc, (*(yyvsp[-1].interm.typeList))[0].type->getFieldName(), (yyvsp[-2].interm.type).basicType);
        parseContext.mergeQualifiers((yyvsp[-2].interm.type).loc, (yyvsp[-2].interm.type).qualifier, (yyvsp[-3].interm.type).qualifier, true);
        parseContext.precisionQualifierCheck((yyvsp[-2].interm.type).loc, (yyvsp[-2].interm.type).basicType, (yyvsp[-2].interm.type).qualifier, (yyvsp[-2].interm.type).hasTypeParameter());

        for (unsigned int i = 0; i < (yyval.interm.typeList)->size(); ++i) {
            TType type((yyvsp[-2].interm.type));
            type.setFieldName((*(yyval.interm.typeList))[i].type->getFieldName());
            type.transferArraySizes((*(yyval.interm.typeList))[i].type->getArraySizes());
            type.copyArrayInnerSizes((yyvsp[-2].interm.type).arraySizes);
            parseContext.arrayOfArrayVersionCheck((*(yyval.interm.typeList))[i].loc, type.getArraySizes());
            (*(yyval.interm.typeList))[i].type->shallowCopy(type);
        }
    }
#line 11695 "MachineIndependent/glslang_tab.cpp"
    break;

  case 583: /* struct_declarator_list: struct_declarator  */
#line 3878 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.typeList) = new TTypeList;
        (yyval.interm.typeList)->push_back((yyvsp[0].interm.typeLine));
    }
#line 11704 "MachineIndependent/glslang_tab.cpp"
    break;

  case 584: /* struct_declarator_list: struct_declarator_list COMMA struct_declarator  */
#line 3882 "MachineIndependent/glslang.y"
                                                     {
        (yyval.interm.typeList)->push_back((yyvsp[0].interm.typeLine));
    }
#line 11712 "MachineIndependent/glslang_tab.cpp"
    break;

  case 585: /* struct_declarator: IDENTIFIER  */
#line 3888 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.typeLine).type = new TType(EbtVoid);
        (yyval.interm.typeLine).loc = (yyvsp[0].lex).loc;
        (yyval.interm.typeLine).type->setFieldName(*(yyvsp[0].lex).string);
    }
#line 11722 "MachineIndependent/glslang_tab.cpp"
    break;

  case 586: /* struct_declarator: IDENTIFIER array_specifier  */
#line 3893 "MachineIndependent/glslang.y"
                                 {
        parseContext.arrayOfArrayVersionCheck((yyvsp[-1].lex).loc, (yyvsp[0].interm).arraySizes);

        (yyval.interm.typeLine).type = new TType(EbtVoid);
        (yyval.interm.typeLine).loc = (yyvsp[-1].lex).loc;
        (yyval.interm.typeLine).type->setFieldName(*(yyvsp[-1].lex).string);
        (yyval.interm.typeLine).type->transferArraySizes((yyvsp[0].interm).arraySizes);
    }
#line 11735 "MachineIndependent/glslang_tab.cpp"
    break;

  case 587: /* initializer: assignment_expression  */
#line 3904 "MachineIndependent/glslang.y"
                            {
        (yyval.interm.intermTypedNode) = (yyvsp[0].interm.intermTypedNode);
    }
#line 11743 "MachineIndependent/glslang_tab.cpp"
    break;

  case 588: /* initializer: LEFT_BRACE initializer_list RIGHT_BRACE  */
#line 3907 "MachineIndependent/glslang.y"
                                              {
        const char* initFeature = "{ } style initializers";
        parseContext.requireProfile((yyvsp[-2].lex).loc, ~EEsProfile, initFeature);
        parseContext.profileRequires((yyvsp[-2].lex).loc, ~EEsProfile, 420, E_GL_ARB_shading_language_420pack, initFeature);
        (yyval.interm.intermTypedNode) = (yyvsp[-1].interm.intermTypedNode);
    }
#line 11754 "MachineIndependent/glslang_tab.cpp"
    break;

  case 589: /* initializer: LEFT_BRACE initializer_list COMMA RIGHT_BRACE  */
#line 3913 "MachineIndependent/glslang.y"
                                                    {
        const char* initFeature = "{ } style initializers";
        parseContext.requireProfile((yyvsp[-3].lex).loc, ~EEsProfile, initFeature);
        parseContext.profileRequires((yyvsp[-3].lex).loc, ~EEsProfile, 420, E_GL_ARB_shading_language_420pack, initFeature);
        (yyval.interm.intermTypedNode) = (yyvsp[-2].interm.intermTypedNode);
    }
#line 11765 "MachineIndependent/glslang_tab.cpp"
    break;

  case 590: /* initializer: LEFT_BRACE RIGHT_BRACE  */
#line 3919 "MachineIndependent/glslang.y"
                             {
        const char* initFeature = "empty { } initializer";
        parseContext.profileRequires((yyvsp[-1].lex).loc, EEsProfile, 0, E_GL_EXT_null_initializer, initFeature);
        parseContext.profileRequires((yyvsp[-1].lex).loc, ~EEsProfile, 0, E_GL_EXT_null_initializer, initFeature);
        (yyval.interm.intermTypedNode) = parseContext.intermediate.makeAggregate((yyvsp[-1].lex).loc);
    }
#line 11776 "MachineIndependent/glslang_tab.cpp"
    break;

  case 591: /* initializer_list: initializer  */
#line 3928 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.intermTypedNode) = parseContext.intermediate.growAggregate(0, (yyvsp[0].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode)->getLoc());
    }
#line 11784 "MachineIndependent/glslang_tab.cpp"
    break;

  case 592: /* initializer_list: initializer_list COMMA initializer  */
#line 3931 "MachineIndependent/glslang.y"
                                         {
        (yyval.interm.intermTypedNode) = parseContext.intermediate.growAggregate((yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.intermTypedNode));
    }
#line 11792 "MachineIndependent/glslang_tab.cpp"
    break;

  case 593: /* declaration_statement: declaration  */
#line 3937 "MachineIndependent/glslang.y"
                  { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11798 "MachineIndependent/glslang_tab.cpp"
    break;

  case 594: /* statement: compound_statement  */
#line 3941 "MachineIndependent/glslang.y"
                          { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11804 "MachineIndependent/glslang_tab.cpp"
    break;

  case 595: /* statement: simple_statement  */
#line 3942 "MachineIndependent/glslang.y"
                          { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11810 "MachineIndependent/glslang_tab.cpp"
    break;

  case 596: /* simple_statement: declaration_statement  */
#line 3948 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11816 "MachineIndependent/glslang_tab.cpp"
    break;

  case 597: /* simple_statement: expression_statement  */
#line 3949 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11822 "MachineIndependent/glslang_tab.cpp"
    break;

  case 598: /* simple_statement: selection_statement  */
#line 3950 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11828 "MachineIndependent/glslang_tab.cpp"
    break;

  case 599: /* simple_statement: switch_statement  */
#line 3951 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11834 "MachineIndependent/glslang_tab.cpp"
    break;

  case 600: /* simple_statement: case_label  */
#line 3952 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11840 "MachineIndependent/glslang_tab.cpp"
    break;

  case 601: /* simple_statement: iteration_statement  */
#line 3953 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11846 "MachineIndependent/glslang_tab.cpp"
    break;

  case 602: /* simple_statement: jump_statement  */
#line 3954 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11852 "MachineIndependent/glslang_tab.cpp"
    break;

  case 603: /* simple_statement: demote_statement  */
#line 3955 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11858 "MachineIndependent/glslang_tab.cpp"
    break;

  case 604: /* demote_statement: DEMOTE SEMICOLON  */
#line 3959 "MachineIndependent/glslang.y"
                       {
        parseContext.requireStage((yyvsp[-1].lex).loc, EShLangFragment, "demote");
        parseContext.requireExtensions((yyvsp[-1].lex).loc, 1, &E_GL_EXT_demote_to_helper_invocation, "demote");
        (yyval.interm.intermNode) = parseContext.intermediate.addBranch(EOpDemote, (yyvsp[-1].lex).loc);
    }
#line 11868 "MachineIndependent/glslang_tab.cpp"
    break;

  case 605: /* compound_statement: LEFT_BRACE RIGHT_BRACE  */
#line 3967 "MachineIndependent/glslang.y"
                             { (yyval.interm.intermNode) = 0; }
#line 11874 "MachineIndependent/glslang_tab.cpp"
    break;

  case 606: /* $@6: %empty  */
#line 3968 "MachineIndependent/glslang.y"
                 {
        parseContext.symbolTable.push();
        ++parseContext.statementNestingLevel;
    }
#line 11883 "MachineIndependent/glslang_tab.cpp"
    break;

  case 607: /* $@7: %empty  */
#line 3972 "MachineIndependent/glslang.y"
                     {
        parseContext.symbolTable.pop(&parseContext.defaultPrecision[0]);
        --parseContext.statementNestingLevel;
    }
#line 11892 "MachineIndependent/glslang_tab.cpp"
    break;

  case 608: /* compound_statement: LEFT_BRACE $@6 statement_list $@7 RIGHT_BRACE  */
#line 3976 "MachineIndependent/glslang.y"
                  {
        if ((yyvsp[-2].interm.intermNode) && (yyvsp[-2].interm.intermNode)->getAsAggregate()) {
            (yyvsp[-2].interm.intermNode)->getAsAggregate()->setOperator(parseContext.intermediate.getDebugInfo() ? EOpScope : EOpSequence);
            (yyvsp[-2].interm.intermNode)->getAsAggregate()->setEndLoc((yyvsp[0].lex).loc);
        }
        (yyval.interm.intermNode) = (yyvsp[-2].interm.intermNode);
    }
#line 11904 "MachineIndependent/glslang_tab.cpp"
    break;

  case 609: /* statement_no_new_scope: compound_statement_no_new_scope  */
#line 3986 "MachineIndependent/glslang.y"
                                      { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11910 "MachineIndependent/glslang_tab.cpp"
    break;

  case 610: /* statement_no_new_scope: simple_statement  */
#line 3987 "MachineIndependent/glslang.y"
                                      { (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode); }
#line 11916 "MachineIndependent/glslang_tab.cpp"
    break;

  case 611: /* $@8: %empty  */
#line 3991 "MachineIndependent/glslang.y"
      {
        ++parseContext.controlFlowNestingLevel;
    }
#line 11924 "MachineIndependent/glslang_tab.cpp"
    break;

  case 612: /* statement_scoped: $@8 compound_statement  */
#line 3994 "MachineIndependent/glslang.y"
                          {
        --parseContext.controlFlowNestingLevel;
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 11933 "MachineIndependent/glslang_tab.cpp"
    break;

  case 613: /* $@9: %empty  */
#line 3998 "MachineIndependent/glslang.y"
      {
        parseContext.symbolTable.push();
        ++parseContext.statementNestingLevel;
        ++parseContext.controlFlowNestingLevel;
    }
#line 11943 "MachineIndependent/glslang_tab.cpp"
    break;

  case 614: /* statement_scoped: $@9 simple_statement  */
#line 4003 "MachineIndependent/glslang.y"
                       {
        parseContext.symbolTable.pop(&parseContext.defaultPrecision[0]);
        --parseContext.statementNestingLevel;
        --parseContext.controlFlowNestingLevel;
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 11954 "MachineIndependent/glslang_tab.cpp"
    break;

  case 615: /* compound_statement_no_new_scope: LEFT_BRACE RIGHT_BRACE  */
#line 4012 "MachineIndependent/glslang.y"
                             {
        (yyval.interm.intermNode) = 0;
    }
#line 11962 "MachineIndependent/glslang_tab.cpp"
    break;

  case 616: /* compound_statement_no_new_scope: LEFT_BRACE statement_list RIGHT_BRACE  */
#line 4015 "MachineIndependent/glslang.y"
                                            {
        if ((yyvsp[-1].interm.intermNode) && (yyvsp[-1].interm.intermNode)->getAsAggregate()) {
            (yyvsp[-1].interm.intermNode)->getAsAggregate()->setOperator(EOpSequence);
            (yyvsp[-1].interm.intermNode)->getAsAggregate()->setEndLoc((yyvsp[0].lex).loc);
        }
        (yyval.interm.intermNode) = (yyvsp[-1].interm.intermNode);
    }
#line 11974 "MachineIndependent/glslang_tab.cpp"
    break;

  case 617: /* statement_list: statement  */
#line 4025 "MachineIndependent/glslang.y"
                {
        (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate((yyvsp[0].interm.intermNode));
        if ((yyvsp[0].interm.intermNode) && (yyvsp[0].interm.intermNode)->getAsBranchNode() && ((yyvsp[0].interm.intermNode)->getAsBranchNode()->getFlowOp() == EOpCase ||
                                            (yyvsp[0].interm.intermNode)->getAsBranchNode()->getFlowOp() == EOpDefault)) {
            parseContext.wrapupSwitchSubsequence(0, (yyvsp[0].interm.intermNode));
            (yyval.interm.intermNode) = 0;  // start a fresh subsequence for what's after this case
        }
    }
#line 11987 "MachineIndependent/glslang_tab.cpp"
    break;

  case 618: /* statement_list: statement_list statement  */
#line 4033 "MachineIndependent/glslang.y"
                               {
        if ((yyvsp[0].interm.intermNode) && (yyvsp[0].interm.intermNode)->getAsBranchNode() && ((yyvsp[0].interm.intermNode)->getAsBranchNode()->getFlowOp() == EOpCase ||
                                            (yyvsp[0].interm.intermNode)->getAsBranchNode()->getFlowOp() == EOpDefault)) {
            parseContext.wrapupSwitchSubsequence((yyvsp[-1].interm.intermNode) ? (yyvsp[-1].interm.intermNode)->getAsAggregate() : 0, (yyvsp[0].interm.intermNode));
            (yyval.interm.intermNode) = 0;  // start a fresh subsequence for what's after this case
        } else
            (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyvsp[-1].interm.intermNode), (yyvsp[0].interm.intermNode));
    }
#line 12000 "MachineIndependent/glslang_tab.cpp"
    break;

  case 619: /* expression_statement: SEMICOLON  */
#line 4044 "MachineIndependent/glslang.y"
                 { (yyval.interm.intermNode) = 0; }
#line 12006 "MachineIndependent/glslang_tab.cpp"
    break;

  case 620: /* expression_statement: expression SEMICOLON  */
#line 4045 "MachineIndependent/glslang.y"
                            { (yyval.interm.intermNode) = static_cast<TIntermNode*>((yyvsp[-1].interm.intermTypedNode)); }
#line 12012 "MachineIndependent/glslang_tab.cpp"
    break;

  case 621: /* selection_statement: selection_statement_nonattributed  */
#line 4049 "MachineIndependent/glslang.y"
                                        {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12020 "MachineIndependent/glslang_tab.cpp"
    break;

  case 622: /* selection_statement: attribute selection_statement_nonattributed  */
#line 4052 "MachineIndependent/glslang.y"
                                                  {
        parseContext.requireExtensions((yyvsp[0].interm.intermNode)->getLoc(), 1, &E_GL_EXT_control_flow_attributes, "attribute");
        parseContext.handleSelectionAttributes(*(yyvsp[-1].interm.attributes), (yyvsp[0].interm.intermNode));
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12030 "MachineIndependent/glslang_tab.cpp"
    break;

  case 623: /* selection_statement_nonattributed: IF LEFT_PAREN expression RIGHT_PAREN selection_rest_statement  */
#line 4059 "MachineIndependent/glslang.y"
                                                                    {
        parseContext.boolCheck((yyvsp[-4].lex).loc, (yyvsp[-2].interm.intermTypedNode));
        (yyval.interm.intermNode) = parseContext.intermediate.addSelection((yyvsp[-2].interm.intermTypedNode), (yyvsp[0].interm.nodePair), (yyvsp[-4].lex).loc);
    }
#line 12039 "MachineIndependent/glslang_tab.cpp"
    break;

  case 624: /* selection_rest_statement: statement_scoped ELSE statement_scoped  */
#line 4066 "MachineIndependent/glslang.y"
                                             {
        (yyval.interm.nodePair).node1 = (yyvsp[-2].interm.intermNode);
        (yyval.interm.nodePair).node2 = (yyvsp[0].interm.intermNode);
    }
#line 12048 "MachineIndependent/glslang_tab.cpp"
    break;

  case 625: /* selection_rest_statement: statement_scoped  */
#line 4070 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.nodePair).node1 = (yyvsp[0].interm.intermNode);
        (yyval.interm.nodePair).node2 = 0;
    }
#line 12057 "MachineIndependent/glslang_tab.cpp"
    break;

  case 626: /* condition: expression  */
#line 4078 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermTypedNode);
        parseContext.boolCheck((yyvsp[0].interm.intermTypedNode)->getLoc(), (yyvsp[0].interm.intermTypedNode));
    }
#line 12066 "MachineIndependent/glslang_tab.cpp"
    break;

  case 627: /* condition: fully_specified_type IDENTIFIER EQUAL initializer  */
#line 4082 "MachineIndependent/glslang.y"
                                                        {
        parseContext.boolCheck((yyvsp[-2].lex).loc, (yyvsp[-3].interm.type));

        TType type((yyvsp[-3].interm.type));
        (yyval.interm.intermNode) = parseContext.declareVariable((yyvsp[-2].lex).loc, *(yyvsp[-2].lex).string, (yyvsp[-3].interm.type), 0, (yyvsp[0].interm.intermTypedNode));
    }
#line 12077 "MachineIndependent/glslang_tab.cpp"
    break;

  case 628: /* switch_statement: switch_statement_nonattributed  */
#line 4091 "MachineIndependent/glslang.y"
                                     {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12085 "MachineIndependent/glslang_tab.cpp"
    break;

  case 629: /* switch_statement: attribute switch_statement_nonattributed  */
#line 4094 "MachineIndependent/glslang.y"
                                               {
        parseContext.requireExtensions((yyvsp[0].interm.intermNode)->getLoc(), 1, &E_GL_EXT_control_flow_attributes, "attribute");
        parseContext.handleSwitchAttributes(*(yyvsp[-1].interm.attributes), (yyvsp[0].interm.intermNode));
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12095 "MachineIndependent/glslang_tab.cpp"
    break;

  case 630: /* $@10: %empty  */
#line 4101 "MachineIndependent/glslang.y"
                                               {
        // start new switch sequence on the switch stack
        ++parseContext.controlFlowNestingLevel;
        ++parseContext.statementNestingLevel;
        parseContext.switchSequenceStack.push_back(new TIntermSequence);
        parseContext.switchLevel.push_back(parseContext.statementNestingLevel);
        parseContext.symbolTable.push();
    }
#line 12108 "MachineIndependent/glslang_tab.cpp"
    break;

  case 631: /* switch_statement_nonattributed: SWITCH LEFT_PAREN expression RIGHT_PAREN $@10 LEFT_BRACE switch_statement_list RIGHT_BRACE  */
#line 4109 "MachineIndependent/glslang.y"
                                                 {
        (yyval.interm.intermNode) = parseContext.addSwitch((yyvsp[-7].lex).loc, (yyvsp[-5].interm.intermTypedNode), (yyvsp[-1].interm.intermNode) ? (yyvsp[-1].interm.intermNode)->getAsAggregate() : 0);
        delete parseContext.switchSequenceStack.back();
        parseContext.switchSequenceStack.pop_back();
        parseContext.switchLevel.pop_back();
        parseContext.symbolTable.pop(&parseContext.defaultPrecision[0]);
        --parseContext.statementNestingLevel;
        --parseContext.controlFlowNestingLevel;
    }
#line 12122 "MachineIndependent/glslang_tab.cpp"
    break;

  case 632: /* switch_statement_list: %empty  */
#line 4121 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.intermNode) = 0;
    }
#line 12130 "MachineIndependent/glslang_tab.cpp"
    break;

  case 633: /* switch_statement_list: statement_list  */
#line 4124 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12138 "MachineIndependent/glslang_tab.cpp"
    break;

  case 634: /* case_label: CASE expression COLON  */
#line 4130 "MachineIndependent/glslang.y"
                            {
        (yyval.interm.intermNode) = 0;
        if (parseContext.switchLevel.size() == 0)
            parseContext.error((yyvsp[-2].lex).loc, "cannot appear outside switch statement", "case", "");
        else if (parseContext.switchLevel.back() != parseContext.statementNestingLevel)
            parseContext.error((yyvsp[-2].lex).loc, "cannot be nested inside control flow", "case", "");
        else {
            parseContext.constantValueCheck((yyvsp[-1].interm.intermTypedNode), "case");
            parseContext.integerCheck((yyvsp[-1].interm.intermTypedNode), "case");
            (yyval.interm.intermNode) = parseContext.intermediate.addBranch(EOpCase, (yyvsp[-1].interm.intermTypedNode), (yyvsp[-2].lex).loc);
        }
    }
#line 12155 "MachineIndependent/glslang_tab.cpp"
    break;

  case 635: /* case_label: DEFAULT COLON  */
#line 4142 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.intermNode) = 0;
        if (parseContext.switchLevel.size() == 0)
            parseContext.error((yyvsp[-1].lex).loc, "cannot appear outside switch statement", "default", "");
        else if (parseContext.switchLevel.back() != parseContext.statementNestingLevel)
            parseContext.error((yyvsp[-1].lex).loc, "cannot be nested inside control flow", "default", "");
        else
            (yyval.interm.intermNode) = parseContext.intermediate.addBranch(EOpDefault, (yyvsp[-1].lex).loc);
    }
#line 12169 "MachineIndependent/glslang_tab.cpp"
    break;

  case 636: /* iteration_statement: iteration_statement_nonattributed  */
#line 4154 "MachineIndependent/glslang.y"
                                        {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12177 "MachineIndependent/glslang_tab.cpp"
    break;

  case 637: /* iteration_statement: attribute iteration_statement_nonattributed  */
#line 4157 "MachineIndependent/glslang.y"
                                                  {
        const char * extensions[2] = { E_GL_EXT_control_flow_attributes, E_GL_EXT_control_flow_attributes2 };
        parseContext.requireExtensions((yyvsp[0].interm.intermNode)->getLoc(), 2, extensions, "attribute");
        parseContext.handleLoopAttributes(*(yyvsp[-1].interm.attributes), (yyvsp[0].interm.intermNode));
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12188 "MachineIndependent/glslang_tab.cpp"
    break;

  case 638: /* $@11: %empty  */
#line 4165 "MachineIndependent/glslang.y"
                       {
        if (! parseContext.limits.whileLoops)
            parseContext.error((yyvsp[-1].lex).loc, "while loops not available", "limitation", "");
        parseContext.symbolTable.push();
        ++parseContext.loopNestingLevel;
        ++parseContext.statementNestingLevel;
        ++parseContext.controlFlowNestingLevel;
    }
#line 12201 "MachineIndependent/glslang_tab.cpp"
    break;

  case 639: /* iteration_statement_nonattributed: WHILE LEFT_PAREN $@11 condition RIGHT_PAREN statement_no_new_scope  */
#line 4173 "MachineIndependent/glslang.y"
                                                   {
        parseContext.symbolTable.pop(&parseContext.defaultPrecision[0]);
        (yyval.interm.intermNode) = parseContext.intermediate.addLoop((yyvsp[0].interm.intermNode), (yyvsp[-2].interm.intermNode), 0, true, (yyvsp[-5].lex).loc);
        if (parseContext.intermediate.getDebugInfo()) {
            (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate((yyval.interm.intermNode), (yyvsp[-5].lex).loc);
            (yyval.interm.intermNode)->getAsAggregate()->setOperator(EOpScope);
        }
        --parseContext.loopNestingLevel;
        --parseContext.statementNestingLevel;
        --parseContext.controlFlowNestingLevel;
    }
#line 12217 "MachineIndependent/glslang_tab.cpp"
    break;

  case 640: /* $@12: %empty  */
#line 4184 "MachineIndependent/glslang.y"
         {
        parseContext.symbolTable.push();
        ++parseContext.loopNestingLevel;
        ++parseContext.statementNestingLevel;
        ++parseContext.controlFlowNestingLevel;
    }
#line 12228 "MachineIndependent/glslang_tab.cpp"
    break;

  case 641: /* iteration_statement_nonattributed: DO $@12 statement WHILE LEFT_PAREN expression RIGHT_PAREN SEMICOLON  */
#line 4190 "MachineIndependent/glslang.y"
                                                                  {
        if (! parseContext.limits.whileLoops)
            parseContext.error((yyvsp[-7].lex).loc, "do-while loops not available", "limitation", "");

        parseContext.boolCheck((yyvsp[0].lex).loc, (yyvsp[-2].interm.intermTypedNode));

        (yyval.interm.intermNode) = parseContext.intermediate.addLoop((yyvsp[-5].interm.intermNode), (yyvsp[-2].interm.intermTypedNode), 0, false, (yyvsp[-4].lex).loc);
        if (parseContext.intermediate.getDebugInfo()) {
            (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate((yyval.interm.intermNode), (yyvsp[-4].lex).loc);
            (yyval.interm.intermNode)->getAsAggregate()->setOperator(EOpScope);
        }
        parseContext.symbolTable.pop(&parseContext.defaultPrecision[0]);
        --parseContext.loopNestingLevel;
        --parseContext.statementNestingLevel;
        --parseContext.controlFlowNestingLevel;
    }
#line 12249 "MachineIndependent/glslang_tab.cpp"
    break;

  case 642: /* $@13: %empty  */
#line 4206 "MachineIndependent/glslang.y"
                     {
        parseContext.symbolTable.push();
        ++parseContext.loopNestingLevel;
        ++parseContext.statementNestingLevel;
        ++parseContext.controlFlowNestingLevel;
    }
#line 12260 "MachineIndependent/glslang_tab.cpp"
    break;

  case 643: /* iteration_statement_nonattributed: FOR LEFT_PAREN $@13 for_init_statement for_rest_statement RIGHT_PAREN statement_no_new_scope  */
#line 4212 "MachineIndependent/glslang.y"
                                                                               {
        parseContext.symbolTable.pop(&parseContext.defaultPrecision[0]);
        (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate((yyvsp[-3].interm.intermNode), (yyvsp[-5].lex).loc);
        TIntermLoop* forLoop = parseContext.intermediate.addLoop((yyvsp[0].interm.intermNode), reinterpret_cast<TIntermTyped*>((yyvsp[-2].interm.nodePair).node1), reinterpret_cast<TIntermTyped*>((yyvsp[-2].interm.nodePair).node2), true, (yyvsp[-6].lex).loc);
        if (! parseContext.limits.nonInductiveForLoops)
            parseContext.inductiveLoopCheck((yyvsp[-6].lex).loc, (yyvsp[-3].interm.intermNode), forLoop);
        (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyval.interm.intermNode), forLoop, (yyvsp[-6].lex).loc);
        (yyval.interm.intermNode)->getAsAggregate()->setOperator(parseContext.intermediate.getDebugInfo() ? EOpScope : EOpSequence);
        --parseContext.loopNestingLevel;
        --parseContext.statementNestingLevel;
        --parseContext.controlFlowNestingLevel;
    }
#line 12277 "MachineIndependent/glslang_tab.cpp"
    break;

  case 644: /* for_init_statement: expression_statement  */
#line 4227 "MachineIndependent/glslang.y"
                           {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12285 "MachineIndependent/glslang_tab.cpp"
    break;

  case 645: /* for_init_statement: declaration_statement  */
#line 4230 "MachineIndependent/glslang.y"
                            {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12293 "MachineIndependent/glslang_tab.cpp"
    break;

  case 646: /* conditionopt: condition  */
#line 4236 "MachineIndependent/glslang.y"
                {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12301 "MachineIndependent/glslang_tab.cpp"
    break;

  case 647: /* conditionopt: %empty  */
#line 4239 "MachineIndependent/glslang.y"
                        {
        (yyval.interm.intermNode) = 0;
    }
#line 12309 "MachineIndependent/glslang_tab.cpp"
    break;

  case 648: /* for_rest_statement: conditionopt SEMICOLON  */
#line 4245 "MachineIndependent/glslang.y"
                             {
        (yyval.interm.nodePair).node1 = (yyvsp[-1].interm.intermNode);
        (yyval.interm.nodePair).node2 = 0;
    }
#line 12318 "MachineIndependent/glslang_tab.cpp"
    break;

  case 649: /* for_rest_statement: conditionopt SEMICOLON expression  */
#line 4249 "MachineIndependent/glslang.y"
                                         {
        (yyval.interm.nodePair).node1 = (yyvsp[-2].interm.intermNode);
        (yyval.interm.nodePair).node2 = (yyvsp[0].interm.intermTypedNode);
    }
#line 12327 "MachineIndependent/glslang_tab.cpp"
    break;

  case 650: /* jump_statement: CONTINUE SEMICOLON  */
#line 4256 "MachineIndependent/glslang.y"
                         {
        if (parseContext.loopNestingLevel <= 0)
            parseContext.error((yyvsp[-1].lex).loc, "continue statement only allowed in loops", "", "");
        (yyval.interm.intermNode) = parseContext.intermediate.addBranch(EOpContinue, (yyvsp[-1].lex).loc);
    }
#line 12337 "MachineIndependent/glslang_tab.cpp"
    break;

  case 651: /* jump_statement: BREAK SEMICOLON  */
#line 4261 "MachineIndependent/glslang.y"
                      {
        if (parseContext.loopNestingLevel + parseContext.switchSequenceStack.size() <= 0)
            parseContext.error((yyvsp[-1].lex).loc, "break statement only allowed in switch and loops", "", "");
        (yyval.interm.intermNode) = parseContext.intermediate.addBranch(EOpBreak, (yyvsp[-1].lex).loc);
    }
#line 12347 "MachineIndependent/glslang_tab.cpp"
    break;

  case 652: /* jump_statement: RETURN SEMICOLON  */
#line 4266 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.intermNode) = parseContext.intermediate.addBranch(EOpReturn, (yyvsp[-1].lex).loc);
        if (parseContext.currentFunctionType->getBasicType() != EbtVoid)
            parseContext.error((yyvsp[-1].lex).loc, "non-void function must return a value", "return", "");
        if (parseContext.inMain)
            parseContext.postEntryPointReturn = true;
    }
#line 12359 "MachineIndependent/glslang_tab.cpp"
    break;

  case 653: /* jump_statement: RETURN expression SEMICOLON  */
#line 4273 "MachineIndependent/glslang.y"
                                  {
        (yyval.interm.intermNode) = parseContext.handleReturnValue((yyvsp[-2].lex).loc, (yyvsp[-1].interm.intermTypedNode));
    }
#line 12367 "MachineIndependent/glslang_tab.cpp"
    break;

  case 654: /* jump_statement: DISCARD SEMICOLON  */
#line 4276 "MachineIndependent/glslang.y"
                        {
        parseContext.requireStage((yyvsp[-1].lex).loc, EShLangFragment, "discard");
        (yyval.interm.intermNode) = parseContext.intermediate.addBranch(EOpKill, (yyvsp[-1].lex).loc);
    }
#line 12376 "MachineIndependent/glslang_tab.cpp"
    break;

  case 655: /* jump_statement: TERMINATE_INVOCATION SEMICOLON  */
#line 4280 "MachineIndependent/glslang.y"
                                     {
        parseContext.requireStage((yyvsp[-1].lex).loc, EShLangFragment, "terminateInvocation");
        (yyval.interm.intermNode) = parseContext.intermediate.addBranch(EOpTerminateInvocation, (yyvsp[-1].lex).loc);
    }
#line 12385 "MachineIndependent/glslang_tab.cpp"
    break;

  case 656: /* jump_statement: TERMINATE_RAY SEMICOLON  */
#line 4284 "MachineIndependent/glslang.y"
                              {
        parseContext.requireStage((yyvsp[-1].lex).loc, EShLangAnyHit, "terminateRayEXT");
        (yyval.interm.intermNode) = parseContext.intermediate.addBranch(EOpTerminateRayKHR, (yyvsp[-1].lex).loc);
    }
#line 12394 "MachineIndependent/glslang_tab.cpp"
    break;

  case 657: /* jump_statement: IGNORE_INTERSECTION SEMICOLON  */
#line 4288 "MachineIndependent/glslang.y"
                                    {
        parseContext.requireStage((yyvsp[-1].lex).loc, EShLangAnyHit, "ignoreIntersectionEXT");
        (yyval.interm.intermNode) = parseContext.intermediate.addBranch(EOpIgnoreIntersectionKHR, (yyvsp[-1].lex).loc);
    }
#line 12403 "MachineIndependent/glslang_tab.cpp"
    break;

  case 658: /* translation_unit: external_declaration  */
#line 4297 "MachineIndependent/glslang.y"
                           {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
        parseContext.intermediate.setTreeRoot((yyval.interm.intermNode));
    }
#line 12412 "MachineIndependent/glslang_tab.cpp"
    break;

  case 659: /* translation_unit: translation_unit external_declaration  */
#line 4301 "MachineIndependent/glslang.y"
                                            {
        if ((yyvsp[0].interm.intermNode) != nullptr) {
            (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyvsp[-1].interm.intermNode), (yyvsp[0].interm.intermNode));
            parseContext.intermediate.setTreeRoot((yyval.interm.intermNode));
        }
    }
#line 12423 "MachineIndependent/glslang_tab.cpp"
    break;

  case 660: /* external_declaration: function_definition  */
#line 4310 "MachineIndependent/glslang.y"
                          {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12431 "MachineIndependent/glslang_tab.cpp"
    break;

  case 661: /* external_declaration: declaration  */
#line 4313 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.intermNode) = (yyvsp[0].interm.intermNode);
    }
#line 12439 "MachineIndependent/glslang_tab.cpp"
    break;

  case 662: /* external_declaration: SEMICOLON  */
#line 4316 "MachineIndependent/glslang.y"
                {
        parseContext.requireProfile((yyvsp[0].lex).loc, ~EEsProfile, "extraneous semicolon");
        parseContext.profileRequires((yyvsp[0].lex).loc, ~EEsProfile, 460, nullptr, "extraneous semicolon");
        (yyval.interm.intermNode) = nullptr;
    }
#line 12449 "MachineIndependent/glslang_tab.cpp"
    break;

  case 663: /* $@14: %empty  */
#line 4324 "MachineIndependent/glslang.y"
                         {
        (yyvsp[0].interm).function = parseContext.handleFunctionDeclarator((yyvsp[0].interm).loc, *(yyvsp[0].interm).function, false /* not prototype */);
        (yyvsp[0].interm).intermNode = parseContext.handleFunctionDefinition((yyvsp[0].interm).loc, *(yyvsp[0].interm).function);

        // For ES 100 only, according to ES shading language 100 spec: A function
        // body has a scope nested inside the function's definition.
        if (parseContext.profile == EEsProfile && parseContext.version == 100)
        {
            parseContext.symbolTable.push();
            ++parseContext.statementNestingLevel;
        }
    }
#line 12466 "MachineIndependent/glslang_tab.cpp"
    break;

  case 664: /* function_definition: function_prototype $@14 compound_statement_no_new_scope  */
#line 4336 "MachineIndependent/glslang.y"
                                    {
        //   May be best done as post process phase on intermediate code
        if (parseContext.currentFunctionType->getBasicType() != EbtVoid && ! parseContext.functionReturnsValue)
            parseContext.error((yyvsp[-2].interm).loc, "function does not return a value:", "", (yyvsp[-2].interm).function->getName().c_str());
        parseContext.symbolTable.pop(&parseContext.defaultPrecision[0]);
        (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyvsp[-2].interm).intermNode, (yyvsp[0].interm.intermNode));
        (yyval.interm.intermNode)->getAsAggregate()->setLinkType((yyvsp[-2].interm).function->getLinkType());
        parseContext.intermediate.setAggregateOperator((yyval.interm.intermNode), EOpFunction, (yyvsp[-2].interm).function->getType(), (yyvsp[-2].interm).loc);
        (yyval.interm.intermNode)->getAsAggregate()->setName((yyvsp[-2].interm).function->getMangledName().c_str());

        // store the pragma information for debug and optimize and other vendor specific
        // information. This information can be queried from the parse tree
        (yyval.interm.intermNode)->getAsAggregate()->setOptimize(parseContext.contextPragma.optimize);
        (yyval.interm.intermNode)->getAsAggregate()->setDebug(parseContext.contextPragma.debug);
        (yyval.interm.intermNode)->getAsAggregate()->setPragmaTable(parseContext.contextPragma.pragmaTable);

        // Set currentFunctionType to empty pointer when goes outside of the function
        parseContext.currentFunctionType = nullptr;

        // For ES 100 only, according to ES shading language 100 spec: A function
        // body has a scope nested inside the function's definition.
        if (parseContext.profile == EEsProfile && parseContext.version == 100)
        {
            parseContext.symbolTable.pop(&parseContext.defaultPrecision[0]);
            --parseContext.statementNestingLevel;
        }
    }
#line 12498 "MachineIndependent/glslang_tab.cpp"
    break;

  case 665: /* attribute: LEFT_BRACKET LEFT_BRACKET attribute_list RIGHT_BRACKET RIGHT_BRACKET  */
#line 4366 "MachineIndependent/glslang.y"
                                                                           {
        (yyval.interm.attributes) = (yyvsp[-2].interm.attributes);
    }
#line 12506 "MachineIndependent/glslang_tab.cpp"
    break;

  case 666: /* attribute_list: single_attribute  */
#line 4371 "MachineIndependent/glslang.y"
                       {
        (yyval.interm.attributes) = (yyvsp[0].interm.attributes);
    }
#line 12514 "MachineIndependent/glslang_tab.cpp"
    break;

  case 667: /* attribute_list: attribute_list COMMA single_attribute  */
#line 4374 "MachineIndependent/glslang.y"
                                            {
        (yyval.interm.attributes) = parseContext.mergeAttributes((yyvsp[-2].interm.attributes), (yyvsp[0].interm.attributes));
    }
#line 12522 "MachineIndependent/glslang_tab.cpp"
    break;

  case 668: /* single_attribute: IDENTIFIER  */
#line 4379 "MachineIndependent/glslang.y"
                 {
        (yyval.interm.attributes) = parseContext.makeAttributes(*(yyvsp[0].lex).string);
    }
#line 12530 "MachineIndependent/glslang_tab.cpp"
    break;

  case 669: /* single_attribute: IDENTIFIER LEFT_PAREN constant_expression RIGHT_PAREN  */
#line 4382 "MachineIndependent/glslang.y"
                                                            {
        (yyval.interm.attributes) = parseContext.makeAttributes(*(yyvsp[-3].lex).string, (yyvsp[-1].interm.intermTypedNode));
    }
#line 12538 "MachineIndependent/glslang_tab.cpp"
    break;

  case 670: /* spirv_requirements_list: spirv_requirements_parameter  */
#line 4387 "MachineIndependent/glslang.y"
                                   {
        (yyval.interm.spirvReq) = (yyvsp[0].interm.spirvReq);
    }
#line 12546 "MachineIndependent/glslang_tab.cpp"
    break;

  case 671: /* spirv_requirements_list: spirv_requirements_list COMMA spirv_requirements_parameter  */
#line 4390 "MachineIndependent/glslang.y"
                                                                 {
        (yyval.interm.spirvReq) = parseContext.mergeSpirvRequirements((yyvsp[-1].lex).loc, (yyvsp[-2].interm.spirvReq), (yyvsp[0].interm.spirvReq));
    }
#line 12554 "MachineIndependent/glslang_tab.cpp"
    break;

  case 672: /* spirv_requirements_parameter: IDENTIFIER EQUAL LEFT_BRACKET spirv_extension_list RIGHT_BRACKET  */
#line 4395 "MachineIndependent/glslang.y"
                                                                       {
        (yyval.interm.spirvReq) = parseContext.makeSpirvRequirement((yyvsp[-3].lex).loc, *(yyvsp[-4].lex).string, (yyvsp[-1].interm.intermNode)->getAsAggregate(), nullptr);
    }
#line 12562 "MachineIndependent/glslang_tab.cpp"
    break;

  case 673: /* spirv_requirements_parameter: IDENTIFIER EQUAL LEFT_BRACKET spirv_capability_list RIGHT_BRACKET  */
#line 4398 "MachineIndependent/glslang.y"
                                                                        {
        (yyval.interm.spirvReq) = parseContext.makeSpirvRequirement((yyvsp[-3].lex).loc, *(yyvsp[-4].lex).string, nullptr, (yyvsp[-1].interm.intermNode)->getAsAggregate());
    }
#line 12570 "MachineIndependent/glslang_tab.cpp"
    break;

  case 674: /* spirv_extension_list: STRING_LITERAL  */
#line 4403 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate(parseContext.intermediate.addConstantUnion((yyvsp[0].lex).string, (yyvsp[0].lex).loc, true));
    }
#line 12578 "MachineIndependent/glslang_tab.cpp"
    break;

  case 675: /* spirv_extension_list: spirv_extension_list COMMA STRING_LITERAL  */
#line 4406 "MachineIndependent/glslang.y"
                                                {
        (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyvsp[-2].interm.intermNode), parseContext.intermediate.addConstantUnion((yyvsp[0].lex).string, (yyvsp[0].lex).loc, true));
    }
#line 12586 "MachineIndependent/glslang_tab.cpp"
    break;

  case 676: /* spirv_capability_list: INTCONSTANT  */
#line 4411 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate(parseContext.intermediate.addConstantUnion((yyvsp[0].lex).i, (yyvsp[0].lex).loc, true));
    }
#line 12594 "MachineIndependent/glslang_tab.cpp"
    break;

  case 677: /* spirv_capability_list: spirv_capability_list COMMA INTCONSTANT  */
#line 4414 "MachineIndependent/glslang.y"
                                              {
        (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyvsp[-2].interm.intermNode), parseContext.intermediate.addConstantUnion((yyvsp[0].lex).i, (yyvsp[0].lex).loc, true));
    }
#line 12602 "MachineIndependent/glslang_tab.cpp"
    break;

  case 678: /* spirv_execution_mode_qualifier: SPIRV_EXECUTION_MODE LEFT_PAREN INTCONSTANT RIGHT_PAREN  */
#line 4419 "MachineIndependent/glslang.y"
                                                              {
        parseContext.intermediate.insertSpirvExecutionMode((yyvsp[-1].lex).i);
        (yyval.interm.intermNode) = 0;
    }
#line 12611 "MachineIndependent/glslang_tab.cpp"
    break;

  case 679: /* spirv_execution_mode_qualifier: SPIRV_EXECUTION_MODE LEFT_PAREN spirv_requirements_list COMMA INTCONSTANT RIGHT_PAREN  */
#line 4423 "MachineIndependent/glslang.y"
                                                                                            {
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-3].interm.spirvReq));
        parseContext.intermediate.insertSpirvExecutionMode((yyvsp[-1].lex).i);
        (yyval.interm.intermNode) = 0;
    }
#line 12621 "MachineIndependent/glslang_tab.cpp"
    break;

  case 680: /* spirv_execution_mode_qualifier: SPIRV_EXECUTION_MODE LEFT_PAREN INTCONSTANT COMMA spirv_execution_mode_parameter_list RIGHT_PAREN  */
#line 4428 "MachineIndependent/glslang.y"
                                                                                                        {
        parseContext.intermediate.insertSpirvExecutionMode((yyvsp[-3].lex).i, (yyvsp[-1].interm.intermNode)->getAsAggregate());
        (yyval.interm.intermNode) = 0;
    }
#line 12630 "MachineIndependent/glslang_tab.cpp"
    break;

  case 681: /* spirv_execution_mode_qualifier: SPIRV_EXECUTION_MODE LEFT_PAREN spirv_requirements_list COMMA INTCONSTANT COMMA spirv_execution_mode_parameter_list RIGHT_PAREN  */
#line 4432 "MachineIndependent/glslang.y"
                                                                                                                                      {
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-5].interm.spirvReq));
        parseContext.intermediate.insertSpirvExecutionMode((yyvsp[-3].lex).i, (yyvsp[-1].interm.intermNode)->getAsAggregate());
        (yyval.interm.intermNode) = 0;
    }
#line 12640 "MachineIndependent/glslang_tab.cpp"
    break;

  case 682: /* spirv_execution_mode_qualifier: SPIRV_EXECUTION_MODE_ID LEFT_PAREN INTCONSTANT COMMA spirv_execution_mode_id_parameter_list RIGHT_PAREN  */
#line 4437 "MachineIndependent/glslang.y"
                                                                                                              {
        parseContext.intermediate.insertSpirvExecutionModeId((yyvsp[-3].lex).i, (yyvsp[-1].interm.intermNode)->getAsAggregate());
        (yyval.interm.intermNode) = 0;
    }
#line 12649 "MachineIndependent/glslang_tab.cpp"
    break;

  case 683: /* spirv_execution_mode_qualifier: SPIRV_EXECUTION_MODE_ID LEFT_PAREN spirv_requirements_list COMMA INTCONSTANT COMMA spirv_execution_mode_id_parameter_list RIGHT_PAREN  */
#line 4441 "MachineIndependent/glslang.y"
                                                                                                                                            {
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-5].interm.spirvReq));
        parseContext.intermediate.insertSpirvExecutionModeId((yyvsp[-3].lex).i, (yyvsp[-1].interm.intermNode)->getAsAggregate());
        (yyval.interm.intermNode) = 0;
    }
#line 12659 "MachineIndependent/glslang_tab.cpp"
    break;

  case 684: /* spirv_execution_mode_parameter_list: spirv_execution_mode_parameter  */
#line 4448 "MachineIndependent/glslang.y"
                                     {
        (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate((yyvsp[0].interm.intermNode));
    }
#line 12667 "MachineIndependent/glslang_tab.cpp"
    break;

  case 685: /* spirv_execution_mode_parameter_list: spirv_execution_mode_parameter_list COMMA spirv_execution_mode_parameter  */
#line 4451 "MachineIndependent/glslang.y"
                                                                               {
        (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyvsp[-2].interm.intermNode), (yyvsp[0].interm.intermNode));
    }
#line 12675 "MachineIndependent/glslang_tab.cpp"
    break;

  case 686: /* spirv_execution_mode_parameter: FLOATCONSTANT  */
#line 4456 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).d, EbtFloat, (yyvsp[0].lex).loc, true);
    }
#line 12683 "MachineIndependent/glslang_tab.cpp"
    break;

  case 687: /* spirv_execution_mode_parameter: INTCONSTANT  */
#line 4459 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).i, (yyvsp[0].lex).loc, true);
    }
#line 12691 "MachineIndependent/glslang_tab.cpp"
    break;

  case 688: /* spirv_execution_mode_parameter: UINTCONSTANT  */
#line 4462 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).u, (yyvsp[0].lex).loc, true);
    }
#line 12699 "MachineIndependent/glslang_tab.cpp"
    break;

  case 689: /* spirv_execution_mode_parameter: BOOLCONSTANT  */
#line 4465 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).b, (yyvsp[0].lex).loc, true);
    }
#line 12707 "MachineIndependent/glslang_tab.cpp"
    break;

  case 690: /* spirv_execution_mode_parameter: STRING_LITERAL  */
#line 4468 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).string, (yyvsp[0].lex).loc, true);
    }
#line 12715 "MachineIndependent/glslang_tab.cpp"
    break;

  case 691: /* spirv_execution_mode_id_parameter_list: constant_expression  */
#line 4473 "MachineIndependent/glslang.y"
                          {
        if ((yyvsp[0].interm.intermTypedNode)->getBasicType() != EbtFloat &&
            (yyvsp[0].interm.intermTypedNode)->getBasicType() != EbtInt &&
            (yyvsp[0].interm.intermTypedNode)->getBasicType() != EbtUint &&
            (yyvsp[0].interm.intermTypedNode)->getBasicType() != EbtBool &&
            (yyvsp[0].interm.intermTypedNode)->getBasicType() != EbtString)
            parseContext.error((yyvsp[0].interm.intermTypedNode)->getLoc(), "this type not allowed", (yyvsp[0].interm.intermTypedNode)->getType().getBasicString(), "");
        (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate((yyvsp[0].interm.intermTypedNode));
    }
#line 12729 "MachineIndependent/glslang_tab.cpp"
    break;

  case 692: /* spirv_execution_mode_id_parameter_list: spirv_execution_mode_id_parameter_list COMMA constant_expression  */
#line 4482 "MachineIndependent/glslang.y"
                                                                       {
        if ((yyvsp[0].interm.intermTypedNode)->getBasicType() != EbtFloat &&
            (yyvsp[0].interm.intermTypedNode)->getBasicType() != EbtInt &&
            (yyvsp[0].interm.intermTypedNode)->getBasicType() != EbtUint &&
            (yyvsp[0].interm.intermTypedNode)->getBasicType() != EbtBool &&
            (yyvsp[0].interm.intermTypedNode)->getBasicType() != EbtString)
            parseContext.error((yyvsp[0].interm.intermTypedNode)->getLoc(), "this type not allowed", (yyvsp[0].interm.intermTypedNode)->getType().getBasicString(), "");
        (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyvsp[-2].interm.intermNode), (yyvsp[0].interm.intermTypedNode));
    }
#line 12743 "MachineIndependent/glslang_tab.cpp"
    break;

  case 693: /* spirv_storage_class_qualifier: SPIRV_STORAGE_CLASS LEFT_PAREN INTCONSTANT RIGHT_PAREN  */
#line 4493 "MachineIndependent/glslang.y"
                                                             {
        (yyval.interm.type).init((yyvsp[-3].lex).loc);
        (yyval.interm.type).qualifier.storage = EvqSpirvStorageClass;
        (yyval.interm.type).qualifier.spirvStorageClass = (yyvsp[-1].lex).i;
    }
#line 12753 "MachineIndependent/glslang_tab.cpp"
    break;

  case 694: /* spirv_storage_class_qualifier: SPIRV_STORAGE_CLASS LEFT_PAREN spirv_requirements_list COMMA INTCONSTANT RIGHT_PAREN  */
#line 4498 "MachineIndependent/glslang.y"
                                                                                           {
        (yyval.interm.type).init((yyvsp[-5].lex).loc);
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-3].interm.spirvReq));
        (yyval.interm.type).qualifier.storage = EvqSpirvStorageClass;
        (yyval.interm.type).qualifier.spirvStorageClass = (yyvsp[-1].lex).i;
    }
#line 12764 "MachineIndependent/glslang_tab.cpp"
    break;

  case 695: /* spirv_decorate_qualifier: SPIRV_DECORATE LEFT_PAREN INTCONSTANT RIGHT_PAREN  */
#line 4506 "MachineIndependent/glslang.y"
                                                       {
        (yyval.interm.type).init((yyvsp[-3].lex).loc);
        (yyval.interm.type).qualifier.setSpirvDecorate((yyvsp[-1].lex).i);
    }
#line 12773 "MachineIndependent/glslang_tab.cpp"
    break;

  case 696: /* spirv_decorate_qualifier: SPIRV_DECORATE LEFT_PAREN spirv_requirements_list COMMA INTCONSTANT RIGHT_PAREN  */
#line 4510 "MachineIndependent/glslang.y"
                                                                                     {
        (yyval.interm.type).init((yyvsp[-5].lex).loc);
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-3].interm.spirvReq));
        (yyval.interm.type).qualifier.setSpirvDecorate((yyvsp[-1].lex).i);
    }
#line 12783 "MachineIndependent/glslang_tab.cpp"
    break;

  case 697: /* spirv_decorate_qualifier: SPIRV_DECORATE LEFT_PAREN INTCONSTANT COMMA spirv_decorate_parameter_list RIGHT_PAREN  */
#line 4515 "MachineIndependent/glslang.y"
                                                                                            {
        (yyval.interm.type).init((yyvsp[-5].lex).loc);
        (yyval.interm.type).qualifier.setSpirvDecorate((yyvsp[-3].lex).i, (yyvsp[-1].interm.intermNode)->getAsAggregate());
    }
#line 12792 "MachineIndependent/glslang_tab.cpp"
    break;

  case 698: /* spirv_decorate_qualifier: SPIRV_DECORATE LEFT_PAREN spirv_requirements_list COMMA INTCONSTANT COMMA spirv_decorate_parameter_list RIGHT_PAREN  */
#line 4519 "MachineIndependent/glslang.y"
                                                                                                                          {
        (yyval.interm.type).init((yyvsp[-7].lex).loc);
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-5].interm.spirvReq));
        (yyval.interm.type).qualifier.setSpirvDecorate((yyvsp[-3].lex).i, (yyvsp[-1].interm.intermNode)->getAsAggregate());
    }
#line 12802 "MachineIndependent/glslang_tab.cpp"
    break;

  case 699: /* spirv_decorate_qualifier: SPIRV_DECORATE_ID LEFT_PAREN INTCONSTANT COMMA spirv_decorate_id_parameter_list RIGHT_PAREN  */
#line 4524 "MachineIndependent/glslang.y"
                                                                                                  {
        (yyval.interm.type).init((yyvsp[-5].lex).loc);
        (yyval.interm.type).qualifier.setSpirvDecorateId((yyvsp[-3].lex).i, (yyvsp[-1].interm.intermNode)->getAsAggregate());
    }
#line 12811 "MachineIndependent/glslang_tab.cpp"
    break;

  case 700: /* spirv_decorate_qualifier: SPIRV_DECORATE_ID LEFT_PAREN spirv_requirements_list COMMA INTCONSTANT COMMA spirv_decorate_id_parameter_list RIGHT_PAREN  */
#line 4528 "MachineIndependent/glslang.y"
                                                                                                                                {
        (yyval.interm.type).init((yyvsp[-7].lex).loc);
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-5].interm.spirvReq));
        (yyval.interm.type).qualifier.setSpirvDecorateId((yyvsp[-3].lex).i, (yyvsp[-1].interm.intermNode)->getAsAggregate());
    }
#line 12821 "MachineIndependent/glslang_tab.cpp"
    break;

  case 701: /* spirv_decorate_qualifier: SPIRV_DECORATE_STRING LEFT_PAREN INTCONSTANT COMMA spirv_decorate_string_parameter_list RIGHT_PAREN  */
#line 4533 "MachineIndependent/glslang.y"
                                                                                                          {
        (yyval.interm.type).init((yyvsp[-5].lex).loc);
        (yyval.interm.type).qualifier.setSpirvDecorateString((yyvsp[-3].lex).i, (yyvsp[-1].interm.intermNode)->getAsAggregate());
    }
#line 12830 "MachineIndependent/glslang_tab.cpp"
    break;

  case 702: /* spirv_decorate_qualifier: SPIRV_DECORATE_STRING LEFT_PAREN spirv_requirements_list COMMA INTCONSTANT COMMA spirv_decorate_string_parameter_list RIGHT_PAREN  */
#line 4537 "MachineIndependent/glslang.y"
                                                                                                                                        {
        (yyval.interm.type).init((yyvsp[-7].lex).loc);
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-5].interm.spirvReq));
        (yyval.interm.type).qualifier.setSpirvDecorateString((yyvsp[-3].lex).i, (yyvsp[-1].interm.intermNode)->getAsAggregate());
    }
#line 12840 "MachineIndependent/glslang_tab.cpp"
    break;

  case 703: /* spirv_decorate_parameter_list: spirv_decorate_parameter  */
#line 4544 "MachineIndependent/glslang.y"
                               {
        (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate((yyvsp[0].interm.intermNode));
    }
#line 12848 "MachineIndependent/glslang_tab.cpp"
    break;

  case 704: /* spirv_decorate_parameter_list: spirv_decorate_parameter_list COMMA spirv_decorate_parameter  */
#line 4547 "MachineIndependent/glslang.y"
                                                                   {
        (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyvsp[-2].interm.intermNode), (yyvsp[0].interm.intermNode));
    }
#line 12856 "MachineIndependent/glslang_tab.cpp"
    break;

  case 705: /* spirv_decorate_parameter: FLOATCONSTANT  */
#line 4552 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).d, EbtFloat, (yyvsp[0].lex).loc, true);
    }
#line 12864 "MachineIndependent/glslang_tab.cpp"
    break;

  case 706: /* spirv_decorate_parameter: INTCONSTANT  */
#line 4555 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).i, (yyvsp[0].lex).loc, true);
    }
#line 12872 "MachineIndependent/glslang_tab.cpp"
    break;

  case 707: /* spirv_decorate_parameter: UINTCONSTANT  */
#line 4558 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).u, (yyvsp[0].lex).loc, true);
    }
#line 12880 "MachineIndependent/glslang_tab.cpp"
    break;

  case 708: /* spirv_decorate_parameter: BOOLCONSTANT  */
#line 4561 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).b, (yyvsp[0].lex).loc, true);
    }
#line 12888 "MachineIndependent/glslang_tab.cpp"
    break;

  case 709: /* spirv_decorate_id_parameter_list: spirv_decorate_id_parameter  */
#line 4566 "MachineIndependent/glslang.y"
                                  {
        (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate((yyvsp[0].interm.intermNode));
    }
#line 12896 "MachineIndependent/glslang_tab.cpp"
    break;

  case 710: /* spirv_decorate_id_parameter_list: spirv_decorate_id_parameter_list COMMA spirv_decorate_id_parameter  */
#line 4569 "MachineIndependent/glslang.y"
                                                                         {
        (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyvsp[-2].interm.intermNode), (yyvsp[0].interm.intermNode));
    }
#line 12904 "MachineIndependent/glslang_tab.cpp"
    break;

  case 711: /* spirv_decorate_id_parameter: variable_identifier  */
#line 4574 "MachineIndependent/glslang.y"
                          {
        if ((yyvsp[0].interm.intermTypedNode)->getAsConstantUnion() || (yyvsp[0].interm.intermTypedNode)->getAsSymbolNode())
            (yyval.interm.intermNode) = (yyvsp[0].interm.intermTypedNode);
        else
            parseContext.error((yyvsp[0].interm.intermTypedNode)->getLoc(), "only allow constants or variables which are not elements of a composite", "", "");
    }
#line 12915 "MachineIndependent/glslang_tab.cpp"
    break;

  case 712: /* spirv_decorate_id_parameter: FLOATCONSTANT  */
#line 4580 "MachineIndependent/glslang.y"
                    {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).d, EbtFloat, (yyvsp[0].lex).loc, true);
    }
#line 12923 "MachineIndependent/glslang_tab.cpp"
    break;

  case 713: /* spirv_decorate_id_parameter: INTCONSTANT  */
#line 4583 "MachineIndependent/glslang.y"
                  {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).i, (yyvsp[0].lex).loc, true);
    }
#line 12931 "MachineIndependent/glslang_tab.cpp"
    break;

  case 714: /* spirv_decorate_id_parameter: UINTCONSTANT  */
#line 4586 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).u, (yyvsp[0].lex).loc, true);
    }
#line 12939 "MachineIndependent/glslang_tab.cpp"
    break;

  case 715: /* spirv_decorate_id_parameter: BOOLCONSTANT  */
#line 4589 "MachineIndependent/glslang.y"
                   {
        (yyval.interm.intermNode) = parseContext.intermediate.addConstantUnion((yyvsp[0].lex).b, (yyvsp[0].lex).loc, true);
    }
#line 12947 "MachineIndependent/glslang_tab.cpp"
    break;

  case 716: /* spirv_decorate_string_parameter_list: STRING_LITERAL  */
#line 4594 "MachineIndependent/glslang.y"
                     {
        (yyval.interm.intermNode) = parseContext.intermediate.makeAggregate(
            parseContext.intermediate.addConstantUnion((yyvsp[0].lex).string, (yyvsp[0].lex).loc, true));
    }
#line 12956 "MachineIndependent/glslang_tab.cpp"
    break;

  case 717: /* spirv_decorate_string_parameter_list: spirv_decorate_string_parameter_list COMMA STRING_LITERAL  */
#line 4598 "MachineIndependent/glslang.y"
                                                                {
        (yyval.interm.intermNode) = parseContext.intermediate.growAggregate((yyvsp[-2].interm.intermNode), parseContext.intermediate.addConstantUnion((yyvsp[0].lex).string, (yyvsp[0].lex).loc, true));
    }
#line 12964 "MachineIndependent/glslang_tab.cpp"
    break;

  case 718: /* spirv_type_specifier: SPIRV_TYPE LEFT_PAREN spirv_instruction_qualifier_list COMMA spirv_type_parameter_list RIGHT_PAREN  */
#line 4603 "MachineIndependent/glslang.y"
                                                                                                         {
        (yyval.interm.type).init((yyvsp[-5].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).setSpirvType(*(yyvsp[-3].interm.spirvInst), (yyvsp[-1].interm.spirvTypeParams));
    }
#line 12973 "MachineIndependent/glslang_tab.cpp"
    break;

  case 719: /* spirv_type_specifier: SPIRV_TYPE LEFT_PAREN spirv_requirements_list COMMA spirv_instruction_qualifier_list COMMA spirv_type_parameter_list RIGHT_PAREN  */
#line 4607 "MachineIndependent/glslang.y"
                                                                                                                                       {
        (yyval.interm.type).init((yyvsp[-7].lex).loc, parseContext.symbolTable.atGlobalLevel());
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-5].interm.spirvReq));
        (yyval.interm.type).setSpirvType(*(yyvsp[-3].interm.spirvInst), (yyvsp[-1].interm.spirvTypeParams));
    }
#line 12983 "MachineIndependent/glslang_tab.cpp"
    break;

  case 720: /* spirv_type_specifier: SPIRV_TYPE LEFT_PAREN spirv_instruction_qualifier_list RIGHT_PAREN  */
#line 4612 "MachineIndependent/glslang.y"
                                                                         {
        (yyval.interm.type).init((yyvsp[-3].lex).loc, parseContext.symbolTable.atGlobalLevel());
        (yyval.interm.type).setSpirvType(*(yyvsp[-1].interm.spirvInst));
    }
#line 12992 "MachineIndependent/glslang_tab.cpp"
    break;

  case 721: /* spirv_type_specifier: SPIRV_TYPE LEFT_PAREN spirv_requirements_list COMMA spirv_instruction_qualifier_list RIGHT_PAREN  */
#line 4616 "MachineIndependent/glslang.y"
                                                                                                       {
        (yyval.interm.type).init((yyvsp[-5].lex).loc, parseContext.symbolTable.atGlobalLevel());
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-3].interm.spirvReq));
        (yyval.interm.type).setSpirvType(*(yyvsp[-1].interm.spirvInst));
    }
#line 13002 "MachineIndependent/glslang_tab.cpp"
    break;

  case 722: /* spirv_type_parameter_list: spirv_type_parameter  */
#line 4623 "MachineIndependent/glslang.y"
                           {
        (yyval.interm.spirvTypeParams) = (yyvsp[0].interm.spirvTypeParams);
    }
#line 13010 "MachineIndependent/glslang_tab.cpp"
    break;

  case 723: /* spirv_type_parameter_list: spirv_type_parameter_list COMMA spirv_type_parameter  */
#line 4626 "MachineIndependent/glslang.y"
                                                           {
        (yyval.interm.spirvTypeParams) = parseContext.mergeSpirvTypeParameters((yyvsp[-2].interm.spirvTypeParams), (yyvsp[0].interm.spirvTypeParams));
    }
#line 13018 "MachineIndependent/glslang_tab.cpp"
    break;

  case 724: /* spirv_type_parameter: constant_expression  */
#line 4631 "MachineIndependent/glslang.y"
                          {
        (yyval.interm.spirvTypeParams) = parseContext.makeSpirvTypeParameters((yyvsp[0].interm.intermTypedNode)->getLoc(), (yyvsp[0].interm.intermTypedNode)->getAsConstantUnion());
    }
#line 13026 "MachineIndependent/glslang_tab.cpp"
    break;

  case 725: /* spirv_type_parameter: type_specifier_nonarray  */
#line 4634 "MachineIndependent/glslang.y"
                              {
        (yyval.interm.spirvTypeParams) = parseContext.makeSpirvTypeParameters((yyvsp[0].interm.type).loc, (yyvsp[0].interm.type));
    }
#line 13034 "MachineIndependent/glslang_tab.cpp"
    break;

  case 726: /* spirv_instruction_qualifier: SPIRV_INSTRUCTION LEFT_PAREN spirv_instruction_qualifier_list RIGHT_PAREN  */
#line 4639 "MachineIndependent/glslang.y"
                                                                                {
        (yyval.interm.spirvInst) = (yyvsp[-1].interm.spirvInst);
    }
#line 13042 "MachineIndependent/glslang_tab.cpp"
    break;

  case 727: /* spirv_instruction_qualifier: SPIRV_INSTRUCTION LEFT_PAREN spirv_requirements_list COMMA spirv_instruction_qualifier_list RIGHT_PAREN  */
#line 4642 "MachineIndependent/glslang.y"
                                                                                                              {
        parseContext.intermediate.insertSpirvRequirement((yyvsp[-3].interm.spirvReq));
        (yyval.interm.spirvInst) = (yyvsp[-1].interm.spirvInst);
    }
#line 13051 "MachineIndependent/glslang_tab.cpp"
    break;

  case 728: /* spirv_instruction_qualifier_list: spirv_instruction_qualifier_id  */
#line 4648 "MachineIndependent/glslang.y"
                                     {
        (yyval.interm.spirvInst) = (yyvsp[0].interm.spirvInst);
    }
#line 13059 "MachineIndependent/glslang_tab.cpp"
    break;

  case 729: /* spirv_instruction_qualifier_list: spirv_instruction_qualifier_list COMMA spirv_instruction_qualifier_id  */
#line 4651 "MachineIndependent/glslang.y"
                                                                            {
        (yyval.interm.spirvInst) = parseContext.mergeSpirvInstruction((yyvsp[-1].lex).loc, (yyvsp[-2].interm.spirvInst), (yyvsp[0].interm.spirvInst));
    }
#line 13067 "MachineIndependent/glslang_tab.cpp"
    break;

  case 730: /* spirv_instruction_qualifier_id: IDENTIFIER EQUAL STRING_LITERAL  */
#line 4656 "MachineIndependent/glslang.y"
                                      {
        (yyval.interm.spirvInst) = parseContext.makeSpirvInstruction((yyvsp[-1].lex).loc, *(yyvsp[-2].lex).string, *(yyvsp[0].lex).string);
    }
#line 13075 "MachineIndependent/glslang_tab.cpp"
    break;

  case 731: /* spirv_instruction_qualifier_id: IDENTIFIER EQUAL INTCONSTANT  */
#line 4659 "MachineIndependent/glslang.y"
                                   {
        (yyval.interm.spirvInst) = parseContext.makeSpirvInstruction((yyvsp[-1].lex).loc, *(yyvsp[-2].lex).string, (yyvsp[0].lex).i);
    }
#line 13083 "MachineIndependent/glslang_tab.cpp"
    break;


#line 13087 "MachineIndependent/glslang_tab.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (pParseContext, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, pParseContext);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, pParseContext);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (pParseContext, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, pParseContext);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, pParseContext);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 4663 "MachineIndependent/glslang.y"

