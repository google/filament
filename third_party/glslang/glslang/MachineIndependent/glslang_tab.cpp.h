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
    VECTOR = 435,                  /* VECTOR  */
    HITOBJECTNV = 436,             /* HITOBJECTNV  */
    HITOBJECTATTRNV = 437,         /* HITOBJECTATTRNV  */
    HITOBJECTEXT = 438,            /* HITOBJECTEXT  */
    HITOBJECTATTREXT = 439,        /* HITOBJECTATTREXT  */
    TENSORLAYOUTNV = 440,          /* TENSORLAYOUTNV  */
    TENSORVIEWNV = 441,            /* TENSORVIEWNV  */
    TENSORARM = 442,               /* TENSORARM  */
    SAMPLERCUBEARRAY = 443,        /* SAMPLERCUBEARRAY  */
    SAMPLERCUBEARRAYSHADOW = 444,  /* SAMPLERCUBEARRAYSHADOW  */
    ISAMPLERCUBEARRAY = 445,       /* ISAMPLERCUBEARRAY  */
    USAMPLERCUBEARRAY = 446,       /* USAMPLERCUBEARRAY  */
    SAMPLER1D = 447,               /* SAMPLER1D  */
    SAMPLER1DARRAY = 448,          /* SAMPLER1DARRAY  */
    SAMPLER1DARRAYSHADOW = 449,    /* SAMPLER1DARRAYSHADOW  */
    ISAMPLER1D = 450,              /* ISAMPLER1D  */
    SAMPLER1DSHADOW = 451,         /* SAMPLER1DSHADOW  */
    SAMPLER2DRECT = 452,           /* SAMPLER2DRECT  */
    SAMPLER2DRECTSHADOW = 453,     /* SAMPLER2DRECTSHADOW  */
    ISAMPLER2DRECT = 454,          /* ISAMPLER2DRECT  */
    USAMPLER2DRECT = 455,          /* USAMPLER2DRECT  */
    SAMPLERBUFFER = 456,           /* SAMPLERBUFFER  */
    ISAMPLERBUFFER = 457,          /* ISAMPLERBUFFER  */
    USAMPLERBUFFER = 458,          /* USAMPLERBUFFER  */
    SAMPLER2DMS = 459,             /* SAMPLER2DMS  */
    ISAMPLER2DMS = 460,            /* ISAMPLER2DMS  */
    USAMPLER2DMS = 461,            /* USAMPLER2DMS  */
    SAMPLER2DMSARRAY = 462,        /* SAMPLER2DMSARRAY  */
    ISAMPLER2DMSARRAY = 463,       /* ISAMPLER2DMSARRAY  */
    USAMPLER2DMSARRAY = 464,       /* USAMPLER2DMSARRAY  */
    SAMPLEREXTERNALOES = 465,      /* SAMPLEREXTERNALOES  */
    SAMPLEREXTERNAL2DY2YEXT = 466, /* SAMPLEREXTERNAL2DY2YEXT  */
    ISAMPLER1DARRAY = 467,         /* ISAMPLER1DARRAY  */
    USAMPLER1D = 468,              /* USAMPLER1D  */
    USAMPLER1DARRAY = 469,         /* USAMPLER1DARRAY  */
    F16SAMPLER1D = 470,            /* F16SAMPLER1D  */
    F16SAMPLER2D = 471,            /* F16SAMPLER2D  */
    F16SAMPLER3D = 472,            /* F16SAMPLER3D  */
    F16SAMPLER2DRECT = 473,        /* F16SAMPLER2DRECT  */
    F16SAMPLERCUBE = 474,          /* F16SAMPLERCUBE  */
    F16SAMPLER1DARRAY = 475,       /* F16SAMPLER1DARRAY  */
    F16SAMPLER2DARRAY = 476,       /* F16SAMPLER2DARRAY  */
    F16SAMPLERCUBEARRAY = 477,     /* F16SAMPLERCUBEARRAY  */
    F16SAMPLERBUFFER = 478,        /* F16SAMPLERBUFFER  */
    F16SAMPLER2DMS = 479,          /* F16SAMPLER2DMS  */
    F16SAMPLER2DMSARRAY = 480,     /* F16SAMPLER2DMSARRAY  */
    F16SAMPLER1DSHADOW = 481,      /* F16SAMPLER1DSHADOW  */
    F16SAMPLER2DSHADOW = 482,      /* F16SAMPLER2DSHADOW  */
    F16SAMPLER1DARRAYSHADOW = 483, /* F16SAMPLER1DARRAYSHADOW  */
    F16SAMPLER2DARRAYSHADOW = 484, /* F16SAMPLER2DARRAYSHADOW  */
    F16SAMPLER2DRECTSHADOW = 485,  /* F16SAMPLER2DRECTSHADOW  */
    F16SAMPLERCUBESHADOW = 486,    /* F16SAMPLERCUBESHADOW  */
    F16SAMPLERCUBEARRAYSHADOW = 487, /* F16SAMPLERCUBEARRAYSHADOW  */
    IMAGE1D = 488,                 /* IMAGE1D  */
    IIMAGE1D = 489,                /* IIMAGE1D  */
    UIMAGE1D = 490,                /* UIMAGE1D  */
    IMAGE2D = 491,                 /* IMAGE2D  */
    IIMAGE2D = 492,                /* IIMAGE2D  */
    UIMAGE2D = 493,                /* UIMAGE2D  */
    IMAGE3D = 494,                 /* IMAGE3D  */
    IIMAGE3D = 495,                /* IIMAGE3D  */
    UIMAGE3D = 496,                /* UIMAGE3D  */
    IMAGE2DRECT = 497,             /* IMAGE2DRECT  */
    IIMAGE2DRECT = 498,            /* IIMAGE2DRECT  */
    UIMAGE2DRECT = 499,            /* UIMAGE2DRECT  */
    IMAGECUBE = 500,               /* IMAGECUBE  */
    IIMAGECUBE = 501,              /* IIMAGECUBE  */
    UIMAGECUBE = 502,              /* UIMAGECUBE  */
    IMAGEBUFFER = 503,             /* IMAGEBUFFER  */
    IIMAGEBUFFER = 504,            /* IIMAGEBUFFER  */
    UIMAGEBUFFER = 505,            /* UIMAGEBUFFER  */
    IMAGE1DARRAY = 506,            /* IMAGE1DARRAY  */
    IIMAGE1DARRAY = 507,           /* IIMAGE1DARRAY  */
    UIMAGE1DARRAY = 508,           /* UIMAGE1DARRAY  */
    IMAGE2DARRAY = 509,            /* IMAGE2DARRAY  */
    IIMAGE2DARRAY = 510,           /* IIMAGE2DARRAY  */
    UIMAGE2DARRAY = 511,           /* UIMAGE2DARRAY  */
    IMAGECUBEARRAY = 512,          /* IMAGECUBEARRAY  */
    IIMAGECUBEARRAY = 513,         /* IIMAGECUBEARRAY  */
    UIMAGECUBEARRAY = 514,         /* UIMAGECUBEARRAY  */
    IMAGE2DMS = 515,               /* IMAGE2DMS  */
    IIMAGE2DMS = 516,              /* IIMAGE2DMS  */
    UIMAGE2DMS = 517,              /* UIMAGE2DMS  */
    IMAGE2DMSARRAY = 518,          /* IMAGE2DMSARRAY  */
    IIMAGE2DMSARRAY = 519,         /* IIMAGE2DMSARRAY  */
    UIMAGE2DMSARRAY = 520,         /* UIMAGE2DMSARRAY  */
    F16IMAGE1D = 521,              /* F16IMAGE1D  */
    F16IMAGE2D = 522,              /* F16IMAGE2D  */
    F16IMAGE3D = 523,              /* F16IMAGE3D  */
    F16IMAGE2DRECT = 524,          /* F16IMAGE2DRECT  */
    F16IMAGECUBE = 525,            /* F16IMAGECUBE  */
    F16IMAGE1DARRAY = 526,         /* F16IMAGE1DARRAY  */
    F16IMAGE2DARRAY = 527,         /* F16IMAGE2DARRAY  */
    F16IMAGECUBEARRAY = 528,       /* F16IMAGECUBEARRAY  */
    F16IMAGEBUFFER = 529,          /* F16IMAGEBUFFER  */
    F16IMAGE2DMS = 530,            /* F16IMAGE2DMS  */
    F16IMAGE2DMSARRAY = 531,       /* F16IMAGE2DMSARRAY  */
    I64IMAGE1D = 532,              /* I64IMAGE1D  */
    U64IMAGE1D = 533,              /* U64IMAGE1D  */
    I64IMAGE2D = 534,              /* I64IMAGE2D  */
    U64IMAGE2D = 535,              /* U64IMAGE2D  */
    I64IMAGE3D = 536,              /* I64IMAGE3D  */
    U64IMAGE3D = 537,              /* U64IMAGE3D  */
    I64IMAGE2DRECT = 538,          /* I64IMAGE2DRECT  */
    U64IMAGE2DRECT = 539,          /* U64IMAGE2DRECT  */
    I64IMAGECUBE = 540,            /* I64IMAGECUBE  */
    U64IMAGECUBE = 541,            /* U64IMAGECUBE  */
    I64IMAGEBUFFER = 542,          /* I64IMAGEBUFFER  */
    U64IMAGEBUFFER = 543,          /* U64IMAGEBUFFER  */
    I64IMAGE1DARRAY = 544,         /* I64IMAGE1DARRAY  */
    U64IMAGE1DARRAY = 545,         /* U64IMAGE1DARRAY  */
    I64IMAGE2DARRAY = 546,         /* I64IMAGE2DARRAY  */
    U64IMAGE2DARRAY = 547,         /* U64IMAGE2DARRAY  */
    I64IMAGECUBEARRAY = 548,       /* I64IMAGECUBEARRAY  */
    U64IMAGECUBEARRAY = 549,       /* U64IMAGECUBEARRAY  */
    I64IMAGE2DMS = 550,            /* I64IMAGE2DMS  */
    U64IMAGE2DMS = 551,            /* U64IMAGE2DMS  */
    I64IMAGE2DMSARRAY = 552,       /* I64IMAGE2DMSARRAY  */
    U64IMAGE2DMSARRAY = 553,       /* U64IMAGE2DMSARRAY  */
    TEXTURECUBEARRAY = 554,        /* TEXTURECUBEARRAY  */
    ITEXTURECUBEARRAY = 555,       /* ITEXTURECUBEARRAY  */
    UTEXTURECUBEARRAY = 556,       /* UTEXTURECUBEARRAY  */
    TEXTURE1D = 557,               /* TEXTURE1D  */
    ITEXTURE1D = 558,              /* ITEXTURE1D  */
    UTEXTURE1D = 559,              /* UTEXTURE1D  */
    TEXTURE1DARRAY = 560,          /* TEXTURE1DARRAY  */
    ITEXTURE1DARRAY = 561,         /* ITEXTURE1DARRAY  */
    UTEXTURE1DARRAY = 562,         /* UTEXTURE1DARRAY  */
    TEXTURE2DRECT = 563,           /* TEXTURE2DRECT  */
    ITEXTURE2DRECT = 564,          /* ITEXTURE2DRECT  */
    UTEXTURE2DRECT = 565,          /* UTEXTURE2DRECT  */
    TEXTUREBUFFER = 566,           /* TEXTUREBUFFER  */
    ITEXTUREBUFFER = 567,          /* ITEXTUREBUFFER  */
    UTEXTUREBUFFER = 568,          /* UTEXTUREBUFFER  */
    TEXTURE2DMS = 569,             /* TEXTURE2DMS  */
    ITEXTURE2DMS = 570,            /* ITEXTURE2DMS  */
    UTEXTURE2DMS = 571,            /* UTEXTURE2DMS  */
    TEXTURE2DMSARRAY = 572,        /* TEXTURE2DMSARRAY  */
    ITEXTURE2DMSARRAY = 573,       /* ITEXTURE2DMSARRAY  */
    UTEXTURE2DMSARRAY = 574,       /* UTEXTURE2DMSARRAY  */
    F16TEXTURE1D = 575,            /* F16TEXTURE1D  */
    F16TEXTURE2D = 576,            /* F16TEXTURE2D  */
    F16TEXTURE3D = 577,            /* F16TEXTURE3D  */
    F16TEXTURE2DRECT = 578,        /* F16TEXTURE2DRECT  */
    F16TEXTURECUBE = 579,          /* F16TEXTURECUBE  */
    F16TEXTURE1DARRAY = 580,       /* F16TEXTURE1DARRAY  */
    F16TEXTURE2DARRAY = 581,       /* F16TEXTURE2DARRAY  */
    F16TEXTURECUBEARRAY = 582,     /* F16TEXTURECUBEARRAY  */
    F16TEXTUREBUFFER = 583,        /* F16TEXTUREBUFFER  */
    F16TEXTURE2DMS = 584,          /* F16TEXTURE2DMS  */
    F16TEXTURE2DMSARRAY = 585,     /* F16TEXTURE2DMSARRAY  */
    SUBPASSINPUT = 586,            /* SUBPASSINPUT  */
    SUBPASSINPUTMS = 587,          /* SUBPASSINPUTMS  */
    ISUBPASSINPUT = 588,           /* ISUBPASSINPUT  */
    ISUBPASSINPUTMS = 589,         /* ISUBPASSINPUTMS  */
    USUBPASSINPUT = 590,           /* USUBPASSINPUT  */
    USUBPASSINPUTMS = 591,         /* USUBPASSINPUTMS  */
    F16SUBPASSINPUT = 592,         /* F16SUBPASSINPUT  */
    F16SUBPASSINPUTMS = 593,       /* F16SUBPASSINPUTMS  */
    SPIRV_INSTRUCTION = 594,       /* SPIRV_INSTRUCTION  */
    SPIRV_EXECUTION_MODE = 595,    /* SPIRV_EXECUTION_MODE  */
    SPIRV_EXECUTION_MODE_ID = 596, /* SPIRV_EXECUTION_MODE_ID  */
    SPIRV_DECORATE = 597,          /* SPIRV_DECORATE  */
    SPIRV_DECORATE_ID = 598,       /* SPIRV_DECORATE_ID  */
    SPIRV_DECORATE_STRING = 599,   /* SPIRV_DECORATE_STRING  */
    SPIRV_TYPE = 600,              /* SPIRV_TYPE  */
    SPIRV_STORAGE_CLASS = 601,     /* SPIRV_STORAGE_CLASS  */
    SPIRV_BY_REFERENCE = 602,      /* SPIRV_BY_REFERENCE  */
    SPIRV_LITERAL = 603,           /* SPIRV_LITERAL  */
    ATTACHMENTEXT = 604,           /* ATTACHMENTEXT  */
    IATTACHMENTEXT = 605,          /* IATTACHMENTEXT  */
    UATTACHMENTEXT = 606,          /* UATTACHMENTEXT  */
    LEFT_OP = 607,                 /* LEFT_OP  */
    RIGHT_OP = 608,                /* RIGHT_OP  */
    INC_OP = 609,                  /* INC_OP  */
    DEC_OP = 610,                  /* DEC_OP  */
    LE_OP = 611,                   /* LE_OP  */
    GE_OP = 612,                   /* GE_OP  */
    EQ_OP = 613,                   /* EQ_OP  */
    NE_OP = 614,                   /* NE_OP  */
    AND_OP = 615,                  /* AND_OP  */
    OR_OP = 616,                   /* OR_OP  */
    XOR_OP = 617,                  /* XOR_OP  */
    MUL_ASSIGN = 618,              /* MUL_ASSIGN  */
    DIV_ASSIGN = 619,              /* DIV_ASSIGN  */
    ADD_ASSIGN = 620,              /* ADD_ASSIGN  */
    MOD_ASSIGN = 621,              /* MOD_ASSIGN  */
    LEFT_ASSIGN = 622,             /* LEFT_ASSIGN  */
    RIGHT_ASSIGN = 623,            /* RIGHT_ASSIGN  */
    AND_ASSIGN = 624,              /* AND_ASSIGN  */
    XOR_ASSIGN = 625,              /* XOR_ASSIGN  */
    OR_ASSIGN = 626,               /* OR_ASSIGN  */
    SUB_ASSIGN = 627,              /* SUB_ASSIGN  */
    STRING_LITERAL = 628,          /* STRING_LITERAL  */
    LEFT_PAREN = 629,              /* LEFT_PAREN  */
    RIGHT_PAREN = 630,             /* RIGHT_PAREN  */
    LEFT_BRACKET = 631,            /* LEFT_BRACKET  */
    RIGHT_BRACKET = 632,           /* RIGHT_BRACKET  */
    LEFT_BRACE = 633,              /* LEFT_BRACE  */
    RIGHT_BRACE = 634,             /* RIGHT_BRACE  */
    DOT = 635,                     /* DOT  */
    COMMA = 636,                   /* COMMA  */
    COLON = 637,                   /* COLON  */
    EQUAL = 638,                   /* EQUAL  */
    SEMICOLON = 639,               /* SEMICOLON  */
    BANG = 640,                    /* BANG  */
    DASH = 641,                    /* DASH  */
    TILDE = 642,                   /* TILDE  */
    PLUS = 643,                    /* PLUS  */
    STAR = 644,                    /* STAR  */
    SLASH = 645,                   /* SLASH  */
    PERCENT = 646,                 /* PERCENT  */
    LEFT_ANGLE = 647,              /* LEFT_ANGLE  */
    RIGHT_ANGLE = 648,             /* RIGHT_ANGLE  */
    VERTICAL_BAR = 649,            /* VERTICAL_BAR  */
    CARET = 650,                   /* CARET  */
    AMPERSAND = 651,               /* AMPERSAND  */
    QUESTION = 652,                /* QUESTION  */
    INVARIANT = 653,               /* INVARIANT  */
    HIGH_PRECISION = 654,          /* HIGH_PRECISION  */
    MEDIUM_PRECISION = 655,        /* MEDIUM_PRECISION  */
    LOW_PRECISION = 656,           /* LOW_PRECISION  */
    PRECISION = 657,               /* PRECISION  */
    PACKED = 658,                  /* PACKED  */
    RESOURCE = 659,                /* RESOURCE  */
    SUPERP = 660,                  /* SUPERP  */
    FLOATCONSTANT = 661,           /* FLOATCONSTANT  */
    INTCONSTANT = 662,             /* INTCONSTANT  */
    UINTCONSTANT = 663,            /* UINTCONSTANT  */
    BOOLCONSTANT = 664,            /* BOOLCONSTANT  */
    IDENTIFIER = 665,              /* IDENTIFIER  */
    TYPE_NAME = 666,               /* TYPE_NAME  */
    CENTROID = 667,                /* CENTROID  */
    IN = 668,                      /* IN  */
    OUT = 669,                     /* OUT  */
    INOUT = 670,                   /* INOUT  */
    STRUCT = 671,                  /* STRUCT  */
    VOID = 672,                    /* VOID  */
    WHILE = 673,                   /* WHILE  */
    BREAK = 674,                   /* BREAK  */
    CONTINUE = 675,                /* CONTINUE  */
    DO = 676,                      /* DO  */
    ELSE = 677,                    /* ELSE  */
    FOR = 678,                     /* FOR  */
    IF = 679,                      /* IF  */
    DISCARD = 680,                 /* DISCARD  */
    RETURN = 681,                  /* RETURN  */
    SWITCH = 682,                  /* SWITCH  */
    CASE = 683,                    /* CASE  */
    DEFAULT = 684,                 /* DEFAULT  */
    TERMINATE_INVOCATION = 685,    /* TERMINATE_INVOCATION  */
    TERMINATE_RAY = 686,           /* TERMINATE_RAY  */
    IGNORE_INTERSECTION = 687,     /* IGNORE_INTERSECTION  */
    UNIFORM = 688,                 /* UNIFORM  */
    SHARED = 689,                  /* SHARED  */
    BUFFER = 690,                  /* BUFFER  */
    TILEIMAGEEXT = 691,            /* TILEIMAGEEXT  */
    FLAT = 692,                    /* FLAT  */
    SMOOTH = 693,                  /* SMOOTH  */
    LAYOUT = 694,                  /* LAYOUT  */
    DOUBLECONSTANT = 695,          /* DOUBLECONSTANT  */
    INT16CONSTANT = 696,           /* INT16CONSTANT  */
    UINT16CONSTANT = 697,          /* UINT16CONSTANT  */
    FLOAT16CONSTANT = 698,         /* FLOAT16CONSTANT  */
    INT32CONSTANT = 699,           /* INT32CONSTANT  */
    UINT32CONSTANT = 700,          /* UINT32CONSTANT  */
    INT64CONSTANT = 701,           /* INT64CONSTANT  */
    UINT64CONSTANT = 702,          /* UINT64CONSTANT  */
    SUBROUTINE = 703,              /* SUBROUTINE  */
    DEMOTE = 704,                  /* DEMOTE  */
    FUNCTION = 705,                /* FUNCTION  */
    PAYLOADNV = 706,               /* PAYLOADNV  */
    PAYLOADINNV = 707,             /* PAYLOADINNV  */
    HITATTRNV = 708,               /* HITATTRNV  */
    CALLDATANV = 709,              /* CALLDATANV  */
    CALLDATAINNV = 710,            /* CALLDATAINNV  */
    PAYLOADEXT = 711,              /* PAYLOADEXT  */
    PAYLOADINEXT = 712,            /* PAYLOADINEXT  */
    HITATTREXT = 713,              /* HITATTREXT  */
    CALLDATAEXT = 714,             /* CALLDATAEXT  */
    CALLDATAINEXT = 715,           /* CALLDATAINEXT  */
    PATCH = 716,                   /* PATCH  */
    SAMPLE = 717,                  /* SAMPLE  */
    NONUNIFORM = 718,              /* NONUNIFORM  */
    RESOURCEHEAP = 719,            /* RESOURCEHEAP  */
    SAMPLERHEAP = 720,             /* SAMPLERHEAP  */
    COHERENT = 721,                /* COHERENT  */
    VOLATILE = 722,                /* VOLATILE  */
    RESTRICT = 723,                /* RESTRICT  */
    READONLY = 724,                /* READONLY  */
    WRITEONLY = 725,               /* WRITEONLY  */
    NONTEMPORAL = 726,             /* NONTEMPORAL  */
    DEVICECOHERENT = 727,          /* DEVICECOHERENT  */
    QUEUEFAMILYCOHERENT = 728,     /* QUEUEFAMILYCOHERENT  */
    WORKGROUPCOHERENT = 729,       /* WORKGROUPCOHERENT  */
    SUBGROUPCOHERENT = 730,        /* SUBGROUPCOHERENT  */
    NONPRIVATE = 731,              /* NONPRIVATE  */
    SHADERCALLCOHERENT = 732,      /* SHADERCALLCOHERENT  */
    NOPERSPECTIVE = 733,           /* NOPERSPECTIVE  */
    EXPLICITINTERPAMD = 734,       /* EXPLICITINTERPAMD  */
    PERVERTEXEXT = 735,            /* PERVERTEXEXT  */
    PERVERTEXNV = 736,             /* PERVERTEXNV  */
    PERPRIMITIVENV = 737,          /* PERPRIMITIVENV  */
    PERVIEWNV = 738,               /* PERVIEWNV  */
    PERTASKNV = 739,               /* PERTASKNV  */
    PERPRIMITIVEEXT = 740,         /* PERPRIMITIVEEXT  */
    TASKPAYLOADWORKGROUPEXT = 741, /* TASKPAYLOADWORKGROUPEXT  */
    PRECISE = 742                  /* PRECISE  */
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

#line 590 "MachineIndependent/glslang_tab.cpp.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int yyparse (glslang::TParseContext* pParseContext);


#endif /* !YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED  */
