/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED
# define YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    CONST = 258,                   /* CONST  */
    BOOL = 259,                    /* BOOL  */
    INT = 260,                     /* INT  */
    UINT = 261,                    /* UINT  */
    FLOAT = 262,                   /* FLOAT  */
    BVEC2 = 263,                   /* BVEC2  */
    BVEC3 = 264,                   /* BVEC3  */
    BVEC4 = 265,                   /* BVEC4  */
    IVEC2 = 266,                   /* IVEC2  */
    IVEC3 = 267,                   /* IVEC3  */
    IVEC4 = 268,                   /* IVEC4  */
    UVEC2 = 269,                   /* UVEC2  */
    UVEC3 = 270,                   /* UVEC3  */
    UVEC4 = 271,                   /* UVEC4  */
    VEC2 = 272,                    /* VEC2  */
    VEC3 = 273,                    /* VEC3  */
    VEC4 = 274,                    /* VEC4  */
    MAT2 = 275,                    /* MAT2  */
    MAT3 = 276,                    /* MAT3  */
    MAT4 = 277,                    /* MAT4  */
    MAT2X2 = 278,                  /* MAT2X2  */
    MAT2X3 = 279,                  /* MAT2X3  */
    MAT2X4 = 280,                  /* MAT2X4  */
    MAT3X2 = 281,                  /* MAT3X2  */
    MAT3X3 = 282,                  /* MAT3X3  */
    MAT3X4 = 283,                  /* MAT3X4  */
    MAT4X2 = 284,                  /* MAT4X2  */
    MAT4X3 = 285,                  /* MAT4X3  */
    MAT4X4 = 286,                  /* MAT4X4  */
    SAMPLER2D = 287,               /* SAMPLER2D  */
    SAMPLER3D = 288,               /* SAMPLER3D  */
    SAMPLERCUBE = 289,             /* SAMPLERCUBE  */
    SAMPLER2DSHADOW = 290,         /* SAMPLER2DSHADOW  */
    SAMPLERCUBESHADOW = 291,       /* SAMPLERCUBESHADOW  */
    SAMPLER2DARRAY = 292,          /* SAMPLER2DARRAY  */
    SAMPLER2DARRAYSHADOW = 293,    /* SAMPLER2DARRAYSHADOW  */
    ISAMPLER2D = 294,              /* ISAMPLER2D  */
    ISAMPLER3D = 295,              /* ISAMPLER3D  */
    ISAMPLERCUBE = 296,            /* ISAMPLERCUBE  */
    ISAMPLER2DARRAY = 297,         /* ISAMPLER2DARRAY  */
    USAMPLER2D = 298,              /* USAMPLER2D  */
    USAMPLER3D = 299,              /* USAMPLER3D  */
    USAMPLERCUBE = 300,            /* USAMPLERCUBE  */
    USAMPLER2DARRAY = 301,         /* USAMPLER2DARRAY  */
    SAMPLER = 302,                 /* SAMPLER  */
    SAMPLERSHADOW = 303,           /* SAMPLERSHADOW  */
    TEXTURE2D = 304,               /* TEXTURE2D  */
    TEXTURE3D = 305,               /* TEXTURE3D  */
    TEXTURECUBE = 306,             /* TEXTURECUBE  */
    TEXTURE2DARRAY = 307,          /* TEXTURE2DARRAY  */
    ITEXTURE2D = 308,              /* ITEXTURE2D  */
    ITEXTURE3D = 309,              /* ITEXTURE3D  */
    ITEXTURECUBE = 310,            /* ITEXTURECUBE  */
    ITEXTURE2DARRAY = 311,         /* ITEXTURE2DARRAY  */
    UTEXTURE2D = 312,              /* UTEXTURE2D  */
    UTEXTURE3D = 313,              /* UTEXTURE3D  */
    UTEXTURECUBE = 314,            /* UTEXTURECUBE  */
    UTEXTURE2DARRAY = 315,         /* UTEXTURE2DARRAY  */
    ATTRIBUTE = 316,               /* ATTRIBUTE  */
    VARYING = 317,                 /* VARYING  */
    FLOATE5M2_T = 318,             /* FLOATE5M2_T  */
    FLOATE4M3_T = 319,             /* FLOATE4M3_T  */
    BFLOAT16_T = 320,              /* BFLOAT16_T  */
    FLOAT16_T = 321,               /* FLOAT16_T  */
    FLOAT32_T = 322,               /* FLOAT32_T  */
    DOUBLE = 323,                  /* DOUBLE  */
    FLOAT64_T = 324,               /* FLOAT64_T  */
    INT64_T = 325,                 /* INT64_T  */
    UINT64_T = 326,                /* UINT64_T  */
    INT32_T = 327,                 /* INT32_T  */
    UINT32_T = 328,                /* UINT32_T  */
    INT16_T = 329,                 /* INT16_T  */
    UINT16_T = 330,                /* UINT16_T  */
    INT8_T = 331,                  /* INT8_T  */
    UINT8_T = 332,                 /* UINT8_T  */
    I64VEC2 = 333,                 /* I64VEC2  */
    I64VEC3 = 334,                 /* I64VEC3  */
    I64VEC4 = 335,                 /* I64VEC4  */
    U64VEC2 = 336,                 /* U64VEC2  */
    U64VEC3 = 337,                 /* U64VEC3  */
    U64VEC4 = 338,                 /* U64VEC4  */
    I32VEC2 = 339,                 /* I32VEC2  */
    I32VEC3 = 340,                 /* I32VEC3  */
    I32VEC4 = 341,                 /* I32VEC4  */
    U32VEC2 = 342,                 /* U32VEC2  */
    U32VEC3 = 343,                 /* U32VEC3  */
    U32VEC4 = 344,                 /* U32VEC4  */
    I16VEC2 = 345,                 /* I16VEC2  */
    I16VEC3 = 346,                 /* I16VEC3  */
    I16VEC4 = 347,                 /* I16VEC4  */
    U16VEC2 = 348,                 /* U16VEC2  */
    U16VEC3 = 349,                 /* U16VEC3  */
    U16VEC4 = 350,                 /* U16VEC4  */
    I8VEC2 = 351,                  /* I8VEC2  */
    I8VEC3 = 352,                  /* I8VEC3  */
    I8VEC4 = 353,                  /* I8VEC4  */
    U8VEC2 = 354,                  /* U8VEC2  */
    U8VEC3 = 355,                  /* U8VEC3  */
    U8VEC4 = 356,                  /* U8VEC4  */
    DVEC2 = 357,                   /* DVEC2  */
    DVEC3 = 358,                   /* DVEC3  */
    DVEC4 = 359,                   /* DVEC4  */
    DMAT2 = 360,                   /* DMAT2  */
    DMAT3 = 361,                   /* DMAT3  */
    DMAT4 = 362,                   /* DMAT4  */
    BF16VEC2 = 363,                /* BF16VEC2  */
    BF16VEC3 = 364,                /* BF16VEC3  */
    BF16VEC4 = 365,                /* BF16VEC4  */
    FE5M2VEC2 = 366,               /* FE5M2VEC2  */
    FE5M2VEC3 = 367,               /* FE5M2VEC3  */
    FE5M2VEC4 = 368,               /* FE5M2VEC4  */
    FE4M3VEC2 = 369,               /* FE4M3VEC2  */
    FE4M3VEC3 = 370,               /* FE4M3VEC3  */
    FE4M3VEC4 = 371,               /* FE4M3VEC4  */
    F16VEC2 = 372,                 /* F16VEC2  */
    F16VEC3 = 373,                 /* F16VEC3  */
    F16VEC4 = 374,                 /* F16VEC4  */
    F16MAT2 = 375,                 /* F16MAT2  */
    F16MAT3 = 376,                 /* F16MAT3  */
    F16MAT4 = 377,                 /* F16MAT4  */
    F32VEC2 = 378,                 /* F32VEC2  */
    F32VEC3 = 379,                 /* F32VEC3  */
    F32VEC4 = 380,                 /* F32VEC4  */
    F32MAT2 = 381,                 /* F32MAT2  */
    F32MAT3 = 382,                 /* F32MAT3  */
    F32MAT4 = 383,                 /* F32MAT4  */
    F64VEC2 = 384,                 /* F64VEC2  */
    F64VEC3 = 385,                 /* F64VEC3  */
    F64VEC4 = 386,                 /* F64VEC4  */
    F64MAT2 = 387,                 /* F64MAT2  */
    F64MAT3 = 388,                 /* F64MAT3  */
    F64MAT4 = 389,                 /* F64MAT4  */
    DMAT2X2 = 390,                 /* DMAT2X2  */
    DMAT2X3 = 391,                 /* DMAT2X3  */
    DMAT2X4 = 392,                 /* DMAT2X4  */
    DMAT3X2 = 393,                 /* DMAT3X2  */
    DMAT3X3 = 394,                 /* DMAT3X3  */
    DMAT3X4 = 395,                 /* DMAT3X4  */
    DMAT4X2 = 396,                 /* DMAT4X2  */
    DMAT4X3 = 397,                 /* DMAT4X3  */
    DMAT4X4 = 398,                 /* DMAT4X4  */
    F16MAT2X2 = 399,               /* F16MAT2X2  */
    F16MAT2X3 = 400,               /* F16MAT2X3  */
    F16MAT2X4 = 401,               /* F16MAT2X4  */
    F16MAT3X2 = 402,               /* F16MAT3X2  */
    F16MAT3X3 = 403,               /* F16MAT3X3  */
    F16MAT3X4 = 404,               /* F16MAT3X4  */
    F16MAT4X2 = 405,               /* F16MAT4X2  */
    F16MAT4X3 = 406,               /* F16MAT4X3  */
    F16MAT4X4 = 407,               /* F16MAT4X4  */
    F32MAT2X2 = 408,               /* F32MAT2X2  */
    F32MAT2X3 = 409,               /* F32MAT2X3  */
    F32MAT2X4 = 410,               /* F32MAT2X4  */
    F32MAT3X2 = 411,               /* F32MAT3X2  */
    F32MAT3X3 = 412,               /* F32MAT3X3  */
    F32MAT3X4 = 413,               /* F32MAT3X4  */
    F32MAT4X2 = 414,               /* F32MAT4X2  */
    F32MAT4X3 = 415,               /* F32MAT4X3  */
    F32MAT4X4 = 416,               /* F32MAT4X4  */
    F64MAT2X2 = 417,               /* F64MAT2X2  */
    F64MAT2X3 = 418,               /* F64MAT2X3  */
    F64MAT2X4 = 419,               /* F64MAT2X4  */
    F64MAT3X2 = 420,               /* F64MAT3X2  */
    F64MAT3X3 = 421,               /* F64MAT3X3  */
    F64MAT3X4 = 422,               /* F64MAT3X4  */
    F64MAT4X2 = 423,               /* F64MAT4X2  */
    F64MAT4X3 = 424,               /* F64MAT4X3  */
    F64MAT4X4 = 425,               /* F64MAT4X4  */
    ATOMIC_UINT = 426,             /* ATOMIC_UINT  */
    ACCSTRUCTNV = 427,             /* ACCSTRUCTNV  */
    ACCSTRUCTEXT = 428,            /* ACCSTRUCTEXT  */
    RAYQUERYEXT = 429,             /* RAYQUERYEXT  */
    FCOOPMATNV = 430,              /* FCOOPMATNV  */
    ICOOPMATNV = 431,              /* ICOOPMATNV  */
    UCOOPMATNV = 432,              /* UCOOPMATNV  */
    COOPMAT = 433,                 /* COOPMAT  */
    COOPVECNV = 434,               /* COOPVECNV  */
    HITOBJECTNV = 435,             /* HITOBJECTNV  */
    HITOBJECTATTRNV = 436,         /* HITOBJECTATTRNV  */
    TENSORLAYOUTNV = 437,          /* TENSORLAYOUTNV  */
    TENSORVIEWNV = 438,            /* TENSORVIEWNV  */
    TENSORARM = 439,               /* TENSORARM  */
    SAMPLERCUBEARRAY = 440,        /* SAMPLERCUBEARRAY  */
    SAMPLERCUBEARRAYSHADOW = 441,  /* SAMPLERCUBEARRAYSHADOW  */
    ISAMPLERCUBEARRAY = 442,       /* ISAMPLERCUBEARRAY  */
    USAMPLERCUBEARRAY = 443,       /* USAMPLERCUBEARRAY  */
    SAMPLER1D = 444,               /* SAMPLER1D  */
    SAMPLER1DARRAY = 445,          /* SAMPLER1DARRAY  */
    SAMPLER1DARRAYSHADOW = 446,    /* SAMPLER1DARRAYSHADOW  */
    ISAMPLER1D = 447,              /* ISAMPLER1D  */
    SAMPLER1DSHADOW = 448,         /* SAMPLER1DSHADOW  */
    SAMPLER2DRECT = 449,           /* SAMPLER2DRECT  */
    SAMPLER2DRECTSHADOW = 450,     /* SAMPLER2DRECTSHADOW  */
    ISAMPLER2DRECT = 451,          /* ISAMPLER2DRECT  */
    USAMPLER2DRECT = 452,          /* USAMPLER2DRECT  */
    SAMPLERBUFFER = 453,           /* SAMPLERBUFFER  */
    ISAMPLERBUFFER = 454,          /* ISAMPLERBUFFER  */
    USAMPLERBUFFER = 455,          /* USAMPLERBUFFER  */
    SAMPLER2DMS = 456,             /* SAMPLER2DMS  */
    ISAMPLER2DMS = 457,            /* ISAMPLER2DMS  */
    USAMPLER2DMS = 458,            /* USAMPLER2DMS  */
    SAMPLER2DMSARRAY = 459,        /* SAMPLER2DMSARRAY  */
    ISAMPLER2DMSARRAY = 460,       /* ISAMPLER2DMSARRAY  */
    USAMPLER2DMSARRAY = 461,       /* USAMPLER2DMSARRAY  */
    SAMPLEREXTERNALOES = 462,      /* SAMPLEREXTERNALOES  */
    SAMPLEREXTERNAL2DY2YEXT = 463, /* SAMPLEREXTERNAL2DY2YEXT  */
    ISAMPLER1DARRAY = 464,         /* ISAMPLER1DARRAY  */
    USAMPLER1D = 465,              /* USAMPLER1D  */
    USAMPLER1DARRAY = 466,         /* USAMPLER1DARRAY  */
    F16SAMPLER1D = 467,            /* F16SAMPLER1D  */
    F16SAMPLER2D = 468,            /* F16SAMPLER2D  */
    F16SAMPLER3D = 469,            /* F16SAMPLER3D  */
    F16SAMPLER2DRECT = 470,        /* F16SAMPLER2DRECT  */
    F16SAMPLERCUBE = 471,          /* F16SAMPLERCUBE  */
    F16SAMPLER1DARRAY = 472,       /* F16SAMPLER1DARRAY  */
    F16SAMPLER2DARRAY = 473,       /* F16SAMPLER2DARRAY  */
    F16SAMPLERCUBEARRAY = 474,     /* F16SAMPLERCUBEARRAY  */
    F16SAMPLERBUFFER = 475,        /* F16SAMPLERBUFFER  */
    F16SAMPLER2DMS = 476,          /* F16SAMPLER2DMS  */
    F16SAMPLER2DMSARRAY = 477,     /* F16SAMPLER2DMSARRAY  */
    F16SAMPLER1DSHADOW = 478,      /* F16SAMPLER1DSHADOW  */
    F16SAMPLER2DSHADOW = 479,      /* F16SAMPLER2DSHADOW  */
    F16SAMPLER1DARRAYSHADOW = 480, /* F16SAMPLER1DARRAYSHADOW  */
    F16SAMPLER2DARRAYSHADOW = 481, /* F16SAMPLER2DARRAYSHADOW  */
    F16SAMPLER2DRECTSHADOW = 482,  /* F16SAMPLER2DRECTSHADOW  */
    F16SAMPLERCUBESHADOW = 483,    /* F16SAMPLERCUBESHADOW  */
    F16SAMPLERCUBEARRAYSHADOW = 484, /* F16SAMPLERCUBEARRAYSHADOW  */
    IMAGE1D = 485,                 /* IMAGE1D  */
    IIMAGE1D = 486,                /* IIMAGE1D  */
    UIMAGE1D = 487,                /* UIMAGE1D  */
    IMAGE2D = 488,                 /* IMAGE2D  */
    IIMAGE2D = 489,                /* IIMAGE2D  */
    UIMAGE2D = 490,                /* UIMAGE2D  */
    IMAGE3D = 491,                 /* IMAGE3D  */
    IIMAGE3D = 492,                /* IIMAGE3D  */
    UIMAGE3D = 493,                /* UIMAGE3D  */
    IMAGE2DRECT = 494,             /* IMAGE2DRECT  */
    IIMAGE2DRECT = 495,            /* IIMAGE2DRECT  */
    UIMAGE2DRECT = 496,            /* UIMAGE2DRECT  */
    IMAGECUBE = 497,               /* IMAGECUBE  */
    IIMAGECUBE = 498,              /* IIMAGECUBE  */
    UIMAGECUBE = 499,              /* UIMAGECUBE  */
    IMAGEBUFFER = 500,             /* IMAGEBUFFER  */
    IIMAGEBUFFER = 501,            /* IIMAGEBUFFER  */
    UIMAGEBUFFER = 502,            /* UIMAGEBUFFER  */
    IMAGE1DARRAY = 503,            /* IMAGE1DARRAY  */
    IIMAGE1DARRAY = 504,           /* IIMAGE1DARRAY  */
    UIMAGE1DARRAY = 505,           /* UIMAGE1DARRAY  */
    IMAGE2DARRAY = 506,            /* IMAGE2DARRAY  */
    IIMAGE2DARRAY = 507,           /* IIMAGE2DARRAY  */
    UIMAGE2DARRAY = 508,           /* UIMAGE2DARRAY  */
    IMAGECUBEARRAY = 509,          /* IMAGECUBEARRAY  */
    IIMAGECUBEARRAY = 510,         /* IIMAGECUBEARRAY  */
    UIMAGECUBEARRAY = 511,         /* UIMAGECUBEARRAY  */
    IMAGE2DMS = 512,               /* IMAGE2DMS  */
    IIMAGE2DMS = 513,              /* IIMAGE2DMS  */
    UIMAGE2DMS = 514,              /* UIMAGE2DMS  */
    IMAGE2DMSARRAY = 515,          /* IMAGE2DMSARRAY  */
    IIMAGE2DMSARRAY = 516,         /* IIMAGE2DMSARRAY  */
    UIMAGE2DMSARRAY = 517,         /* UIMAGE2DMSARRAY  */
    F16IMAGE1D = 518,              /* F16IMAGE1D  */
    F16IMAGE2D = 519,              /* F16IMAGE2D  */
    F16IMAGE3D = 520,              /* F16IMAGE3D  */
    F16IMAGE2DRECT = 521,          /* F16IMAGE2DRECT  */
    F16IMAGECUBE = 522,            /* F16IMAGECUBE  */
    F16IMAGE1DARRAY = 523,         /* F16IMAGE1DARRAY  */
    F16IMAGE2DARRAY = 524,         /* F16IMAGE2DARRAY  */
    F16IMAGECUBEARRAY = 525,       /* F16IMAGECUBEARRAY  */
    F16IMAGEBUFFER = 526,          /* F16IMAGEBUFFER  */
    F16IMAGE2DMS = 527,            /* F16IMAGE2DMS  */
    F16IMAGE2DMSARRAY = 528,       /* F16IMAGE2DMSARRAY  */
    I64IMAGE1D = 529,              /* I64IMAGE1D  */
    U64IMAGE1D = 530,              /* U64IMAGE1D  */
    I64IMAGE2D = 531,              /* I64IMAGE2D  */
    U64IMAGE2D = 532,              /* U64IMAGE2D  */
    I64IMAGE3D = 533,              /* I64IMAGE3D  */
    U64IMAGE3D = 534,              /* U64IMAGE3D  */
    I64IMAGE2DRECT = 535,          /* I64IMAGE2DRECT  */
    U64IMAGE2DRECT = 536,          /* U64IMAGE2DRECT  */
    I64IMAGECUBE = 537,            /* I64IMAGECUBE  */
    U64IMAGECUBE = 538,            /* U64IMAGECUBE  */
    I64IMAGEBUFFER = 539,          /* I64IMAGEBUFFER  */
    U64IMAGEBUFFER = 540,          /* U64IMAGEBUFFER  */
    I64IMAGE1DARRAY = 541,         /* I64IMAGE1DARRAY  */
    U64IMAGE1DARRAY = 542,         /* U64IMAGE1DARRAY  */
    I64IMAGE2DARRAY = 543,         /* I64IMAGE2DARRAY  */
    U64IMAGE2DARRAY = 544,         /* U64IMAGE2DARRAY  */
    I64IMAGECUBEARRAY = 545,       /* I64IMAGECUBEARRAY  */
    U64IMAGECUBEARRAY = 546,       /* U64IMAGECUBEARRAY  */
    I64IMAGE2DMS = 547,            /* I64IMAGE2DMS  */
    U64IMAGE2DMS = 548,            /* U64IMAGE2DMS  */
    I64IMAGE2DMSARRAY = 549,       /* I64IMAGE2DMSARRAY  */
    U64IMAGE2DMSARRAY = 550,       /* U64IMAGE2DMSARRAY  */
    TEXTURECUBEARRAY = 551,        /* TEXTURECUBEARRAY  */
    ITEXTURECUBEARRAY = 552,       /* ITEXTURECUBEARRAY  */
    UTEXTURECUBEARRAY = 553,       /* UTEXTURECUBEARRAY  */
    TEXTURE1D = 554,               /* TEXTURE1D  */
    ITEXTURE1D = 555,              /* ITEXTURE1D  */
    UTEXTURE1D = 556,              /* UTEXTURE1D  */
    TEXTURE1DARRAY = 557,          /* TEXTURE1DARRAY  */
    ITEXTURE1DARRAY = 558,         /* ITEXTURE1DARRAY  */
    UTEXTURE1DARRAY = 559,         /* UTEXTURE1DARRAY  */
    TEXTURE2DRECT = 560,           /* TEXTURE2DRECT  */
    ITEXTURE2DRECT = 561,          /* ITEXTURE2DRECT  */
    UTEXTURE2DRECT = 562,          /* UTEXTURE2DRECT  */
    TEXTUREBUFFER = 563,           /* TEXTUREBUFFER  */
    ITEXTUREBUFFER = 564,          /* ITEXTUREBUFFER  */
    UTEXTUREBUFFER = 565,          /* UTEXTUREBUFFER  */
    TEXTURE2DMS = 566,             /* TEXTURE2DMS  */
    ITEXTURE2DMS = 567,            /* ITEXTURE2DMS  */
    UTEXTURE2DMS = 568,            /* UTEXTURE2DMS  */
    TEXTURE2DMSARRAY = 569,        /* TEXTURE2DMSARRAY  */
    ITEXTURE2DMSARRAY = 570,       /* ITEXTURE2DMSARRAY  */
    UTEXTURE2DMSARRAY = 571,       /* UTEXTURE2DMSARRAY  */
    F16TEXTURE1D = 572,            /* F16TEXTURE1D  */
    F16TEXTURE2D = 573,            /* F16TEXTURE2D  */
    F16TEXTURE3D = 574,            /* F16TEXTURE3D  */
    F16TEXTURE2DRECT = 575,        /* F16TEXTURE2DRECT  */
    F16TEXTURECUBE = 576,          /* F16TEXTURECUBE  */
    F16TEXTURE1DARRAY = 577,       /* F16TEXTURE1DARRAY  */
    F16TEXTURE2DARRAY = 578,       /* F16TEXTURE2DARRAY  */
    F16TEXTURECUBEARRAY = 579,     /* F16TEXTURECUBEARRAY  */
    F16TEXTUREBUFFER = 580,        /* F16TEXTUREBUFFER  */
    F16TEXTURE2DMS = 581,          /* F16TEXTURE2DMS  */
    F16TEXTURE2DMSARRAY = 582,     /* F16TEXTURE2DMSARRAY  */
    SUBPASSINPUT = 583,            /* SUBPASSINPUT  */
    SUBPASSINPUTMS = 584,          /* SUBPASSINPUTMS  */
    ISUBPASSINPUT = 585,           /* ISUBPASSINPUT  */
    ISUBPASSINPUTMS = 586,         /* ISUBPASSINPUTMS  */
    USUBPASSINPUT = 587,           /* USUBPASSINPUT  */
    USUBPASSINPUTMS = 588,         /* USUBPASSINPUTMS  */
    F16SUBPASSINPUT = 589,         /* F16SUBPASSINPUT  */
    F16SUBPASSINPUTMS = 590,       /* F16SUBPASSINPUTMS  */
    SPIRV_INSTRUCTION = 591,       /* SPIRV_INSTRUCTION  */
    SPIRV_EXECUTION_MODE = 592,    /* SPIRV_EXECUTION_MODE  */
    SPIRV_EXECUTION_MODE_ID = 593, /* SPIRV_EXECUTION_MODE_ID  */
    SPIRV_DECORATE = 594,          /* SPIRV_DECORATE  */
    SPIRV_DECORATE_ID = 595,       /* SPIRV_DECORATE_ID  */
    SPIRV_DECORATE_STRING = 596,   /* SPIRV_DECORATE_STRING  */
    SPIRV_TYPE = 597,              /* SPIRV_TYPE  */
    SPIRV_STORAGE_CLASS = 598,     /* SPIRV_STORAGE_CLASS  */
    SPIRV_BY_REFERENCE = 599,      /* SPIRV_BY_REFERENCE  */
    SPIRV_LITERAL = 600,           /* SPIRV_LITERAL  */
    ATTACHMENTEXT = 601,           /* ATTACHMENTEXT  */
    IATTACHMENTEXT = 602,          /* IATTACHMENTEXT  */
    UATTACHMENTEXT = 603,          /* UATTACHMENTEXT  */
    LEFT_OP = 604,                 /* LEFT_OP  */
    RIGHT_OP = 605,                /* RIGHT_OP  */
    INC_OP = 606,                  /* INC_OP  */
    DEC_OP = 607,                  /* DEC_OP  */
    LE_OP = 608,                   /* LE_OP  */
    GE_OP = 609,                   /* GE_OP  */
    EQ_OP = 610,                   /* EQ_OP  */
    NE_OP = 611,                   /* NE_OP  */
    AND_OP = 612,                  /* AND_OP  */
    OR_OP = 613,                   /* OR_OP  */
    XOR_OP = 614,                  /* XOR_OP  */
    MUL_ASSIGN = 615,              /* MUL_ASSIGN  */
    DIV_ASSIGN = 616,              /* DIV_ASSIGN  */
    ADD_ASSIGN = 617,              /* ADD_ASSIGN  */
    MOD_ASSIGN = 618,              /* MOD_ASSIGN  */
    LEFT_ASSIGN = 619,             /* LEFT_ASSIGN  */
    RIGHT_ASSIGN = 620,            /* RIGHT_ASSIGN  */
    AND_ASSIGN = 621,              /* AND_ASSIGN  */
    XOR_ASSIGN = 622,              /* XOR_ASSIGN  */
    OR_ASSIGN = 623,               /* OR_ASSIGN  */
    SUB_ASSIGN = 624,              /* SUB_ASSIGN  */
    STRING_LITERAL = 625,          /* STRING_LITERAL  */
    LEFT_PAREN = 626,              /* LEFT_PAREN  */
    RIGHT_PAREN = 627,             /* RIGHT_PAREN  */
    LEFT_BRACKET = 628,            /* LEFT_BRACKET  */
    RIGHT_BRACKET = 629,           /* RIGHT_BRACKET  */
    LEFT_BRACE = 630,              /* LEFT_BRACE  */
    RIGHT_BRACE = 631,             /* RIGHT_BRACE  */
    DOT = 632,                     /* DOT  */
    COMMA = 633,                   /* COMMA  */
    COLON = 634,                   /* COLON  */
    EQUAL = 635,                   /* EQUAL  */
    SEMICOLON = 636,               /* SEMICOLON  */
    BANG = 637,                    /* BANG  */
    DASH = 638,                    /* DASH  */
    TILDE = 639,                   /* TILDE  */
    PLUS = 640,                    /* PLUS  */
    STAR = 641,                    /* STAR  */
    SLASH = 642,                   /* SLASH  */
    PERCENT = 643,                 /* PERCENT  */
    LEFT_ANGLE = 644,              /* LEFT_ANGLE  */
    RIGHT_ANGLE = 645,             /* RIGHT_ANGLE  */
    VERTICAL_BAR = 646,            /* VERTICAL_BAR  */
    CARET = 647,                   /* CARET  */
    AMPERSAND = 648,               /* AMPERSAND  */
    QUESTION = 649,                /* QUESTION  */
    INVARIANT = 650,               /* INVARIANT  */
    HIGH_PRECISION = 651,          /* HIGH_PRECISION  */
    MEDIUM_PRECISION = 652,        /* MEDIUM_PRECISION  */
    LOW_PRECISION = 653,           /* LOW_PRECISION  */
    PRECISION = 654,               /* PRECISION  */
    PACKED = 655,                  /* PACKED  */
    RESOURCE = 656,                /* RESOURCE  */
    SUPERP = 657,                  /* SUPERP  */
    FLOATCONSTANT = 658,           /* FLOATCONSTANT  */
    INTCONSTANT = 659,             /* INTCONSTANT  */
    UINTCONSTANT = 660,            /* UINTCONSTANT  */
    BOOLCONSTANT = 661,            /* BOOLCONSTANT  */
    IDENTIFIER = 662,              /* IDENTIFIER  */
    TYPE_NAME = 663,               /* TYPE_NAME  */
    CENTROID = 664,                /* CENTROID  */
    IN = 665,                      /* IN  */
    OUT = 666,                     /* OUT  */
    INOUT = 667,                   /* INOUT  */
    STRUCT = 668,                  /* STRUCT  */
    VOID = 669,                    /* VOID  */
    WHILE = 670,                   /* WHILE  */
    BREAK = 671,                   /* BREAK  */
    CONTINUE = 672,                /* CONTINUE  */
    DO = 673,                      /* DO  */
    ELSE = 674,                    /* ELSE  */
    FOR = 675,                     /* FOR  */
    IF = 676,                      /* IF  */
    DISCARD = 677,                 /* DISCARD  */
    RETURN = 678,                  /* RETURN  */
    SWITCH = 679,                  /* SWITCH  */
    CASE = 680,                    /* CASE  */
    DEFAULT = 681,                 /* DEFAULT  */
    TERMINATE_INVOCATION = 682,    /* TERMINATE_INVOCATION  */
    TERMINATE_RAY = 683,           /* TERMINATE_RAY  */
    IGNORE_INTERSECTION = 684,     /* IGNORE_INTERSECTION  */
    UNIFORM = 685,                 /* UNIFORM  */
    SHARED = 686,                  /* SHARED  */
    BUFFER = 687,                  /* BUFFER  */
    TILEIMAGEEXT = 688,            /* TILEIMAGEEXT  */
    FLAT = 689,                    /* FLAT  */
    SMOOTH = 690,                  /* SMOOTH  */
    LAYOUT = 691,                  /* LAYOUT  */
    DOUBLECONSTANT = 692,          /* DOUBLECONSTANT  */
    INT16CONSTANT = 693,           /* INT16CONSTANT  */
    UINT16CONSTANT = 694,          /* UINT16CONSTANT  */
    FLOAT16CONSTANT = 695,         /* FLOAT16CONSTANT  */
    INT32CONSTANT = 696,           /* INT32CONSTANT  */
    UINT32CONSTANT = 697,          /* UINT32CONSTANT  */
    INT64CONSTANT = 698,           /* INT64CONSTANT  */
    UINT64CONSTANT = 699,          /* UINT64CONSTANT  */
    SUBROUTINE = 700,              /* SUBROUTINE  */
    DEMOTE = 701,                  /* DEMOTE  */
    FUNCTION = 702,                /* FUNCTION  */
    PAYLOADNV = 703,               /* PAYLOADNV  */
    PAYLOADINNV = 704,             /* PAYLOADINNV  */
    HITATTRNV = 705,               /* HITATTRNV  */
    CALLDATANV = 706,              /* CALLDATANV  */
    CALLDATAINNV = 707,            /* CALLDATAINNV  */
    PAYLOADEXT = 708,              /* PAYLOADEXT  */
    PAYLOADINEXT = 709,            /* PAYLOADINEXT  */
    HITATTREXT = 710,              /* HITATTREXT  */
    CALLDATAEXT = 711,             /* CALLDATAEXT  */
    CALLDATAINEXT = 712,           /* CALLDATAINEXT  */
    PATCH = 713,                   /* PATCH  */
    SAMPLE = 714,                  /* SAMPLE  */
    NONUNIFORM = 715,              /* NONUNIFORM  */
    COHERENT = 716,                /* COHERENT  */
    VOLATILE = 717,                /* VOLATILE  */
    RESTRICT = 718,                /* RESTRICT  */
    READONLY = 719,                /* READONLY  */
    WRITEONLY = 720,               /* WRITEONLY  */
    NONTEMPORAL = 721,             /* NONTEMPORAL  */
    DEVICECOHERENT = 722,          /* DEVICECOHERENT  */
    QUEUEFAMILYCOHERENT = 723,     /* QUEUEFAMILYCOHERENT  */
    WORKGROUPCOHERENT = 724,       /* WORKGROUPCOHERENT  */
    SUBGROUPCOHERENT = 725,        /* SUBGROUPCOHERENT  */
    NONPRIVATE = 726,              /* NONPRIVATE  */
    SHADERCALLCOHERENT = 727,      /* SHADERCALLCOHERENT  */
    NOPERSPECTIVE = 728,           /* NOPERSPECTIVE  */
    EXPLICITINTERPAMD = 729,       /* EXPLICITINTERPAMD  */
    PERVERTEXEXT = 730,            /* PERVERTEXEXT  */
    PERVERTEXNV = 731,             /* PERVERTEXNV  */
    PERPRIMITIVENV = 732,          /* PERPRIMITIVENV  */
    PERVIEWNV = 733,               /* PERVIEWNV  */
    PERTASKNV = 734,               /* PERTASKNV  */
    PERPRIMITIVEEXT = 735,         /* PERPRIMITIVEEXT  */
    TASKPAYLOADWORKGROUPEXT = 736, /* TASKPAYLOADWORKGROUPEXT  */
    PRECISE = 737                  /* PRECISE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 72 "MachineIndependent/glslang.y"

    struct {
        glslang::TSourceLoc loc;
        union {
            glslang::TString *string;
            int i;
            unsigned int u;
            long long i64;
            unsigned long long u64;
            bool b;
            double d;
        };
        glslang::TSymbol* symbol;
    } lex;
    struct {
        glslang::TSourceLoc loc;
        glslang::TOperator op;
        union {
            TIntermNode* intermNode;
            glslang::TIntermNodePair nodePair;
            glslang::TIntermTyped* intermTypedNode;
            glslang::TAttributes* attributes;
            glslang::TSpirvRequirement* spirvReq;
            glslang::TSpirvInstruction* spirvInst;
            glslang::TSpirvTypeParameters* spirvTypeParams;
        };
        union {
            glslang::TPublicType type;
            glslang::TFunction* function;
            glslang::TParameter param;
            glslang::TTypeLoc typeLine;
            glslang::TTypeList* typeList;
            glslang::TArraySizes* arraySizes;
            glslang::TIdentifierList* identifierList;
        };
        glslang::TTypeParameters* typeParameters;
    } interm;

#line 585 "MachineIndependent/glslang_tab.cpp.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int yyparse (glslang::TParseContext* pParseContext);


#endif /* !YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED  */
