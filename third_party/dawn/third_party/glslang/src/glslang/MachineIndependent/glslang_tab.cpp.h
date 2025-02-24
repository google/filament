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
    FLOAT16_T = 318,               /* FLOAT16_T  */
    FLOAT32_T = 319,               /* FLOAT32_T  */
    DOUBLE = 320,                  /* DOUBLE  */
    FLOAT64_T = 321,               /* FLOAT64_T  */
    INT64_T = 322,                 /* INT64_T  */
    UINT64_T = 323,                /* UINT64_T  */
    INT32_T = 324,                 /* INT32_T  */
    UINT32_T = 325,                /* UINT32_T  */
    INT16_T = 326,                 /* INT16_T  */
    UINT16_T = 327,                /* UINT16_T  */
    INT8_T = 328,                  /* INT8_T  */
    UINT8_T = 329,                 /* UINT8_T  */
    I64VEC2 = 330,                 /* I64VEC2  */
    I64VEC3 = 331,                 /* I64VEC3  */
    I64VEC4 = 332,                 /* I64VEC4  */
    U64VEC2 = 333,                 /* U64VEC2  */
    U64VEC3 = 334,                 /* U64VEC3  */
    U64VEC4 = 335,                 /* U64VEC4  */
    I32VEC2 = 336,                 /* I32VEC2  */
    I32VEC3 = 337,                 /* I32VEC3  */
    I32VEC4 = 338,                 /* I32VEC4  */
    U32VEC2 = 339,                 /* U32VEC2  */
    U32VEC3 = 340,                 /* U32VEC3  */
    U32VEC4 = 341,                 /* U32VEC4  */
    I16VEC2 = 342,                 /* I16VEC2  */
    I16VEC3 = 343,                 /* I16VEC3  */
    I16VEC4 = 344,                 /* I16VEC4  */
    U16VEC2 = 345,                 /* U16VEC2  */
    U16VEC3 = 346,                 /* U16VEC3  */
    U16VEC4 = 347,                 /* U16VEC4  */
    I8VEC2 = 348,                  /* I8VEC2  */
    I8VEC3 = 349,                  /* I8VEC3  */
    I8VEC4 = 350,                  /* I8VEC4  */
    U8VEC2 = 351,                  /* U8VEC2  */
    U8VEC3 = 352,                  /* U8VEC3  */
    U8VEC4 = 353,                  /* U8VEC4  */
    DVEC2 = 354,                   /* DVEC2  */
    DVEC3 = 355,                   /* DVEC3  */
    DVEC4 = 356,                   /* DVEC4  */
    DMAT2 = 357,                   /* DMAT2  */
    DMAT3 = 358,                   /* DMAT3  */
    DMAT4 = 359,                   /* DMAT4  */
    F16VEC2 = 360,                 /* F16VEC2  */
    F16VEC3 = 361,                 /* F16VEC3  */
    F16VEC4 = 362,                 /* F16VEC4  */
    F16MAT2 = 363,                 /* F16MAT2  */
    F16MAT3 = 364,                 /* F16MAT3  */
    F16MAT4 = 365,                 /* F16MAT4  */
    F32VEC2 = 366,                 /* F32VEC2  */
    F32VEC3 = 367,                 /* F32VEC3  */
    F32VEC4 = 368,                 /* F32VEC4  */
    F32MAT2 = 369,                 /* F32MAT2  */
    F32MAT3 = 370,                 /* F32MAT3  */
    F32MAT4 = 371,                 /* F32MAT4  */
    F64VEC2 = 372,                 /* F64VEC2  */
    F64VEC3 = 373,                 /* F64VEC3  */
    F64VEC4 = 374,                 /* F64VEC4  */
    F64MAT2 = 375,                 /* F64MAT2  */
    F64MAT3 = 376,                 /* F64MAT3  */
    F64MAT4 = 377,                 /* F64MAT4  */
    DMAT2X2 = 378,                 /* DMAT2X2  */
    DMAT2X3 = 379,                 /* DMAT2X3  */
    DMAT2X4 = 380,                 /* DMAT2X4  */
    DMAT3X2 = 381,                 /* DMAT3X2  */
    DMAT3X3 = 382,                 /* DMAT3X3  */
    DMAT3X4 = 383,                 /* DMAT3X4  */
    DMAT4X2 = 384,                 /* DMAT4X2  */
    DMAT4X3 = 385,                 /* DMAT4X3  */
    DMAT4X4 = 386,                 /* DMAT4X4  */
    F16MAT2X2 = 387,               /* F16MAT2X2  */
    F16MAT2X3 = 388,               /* F16MAT2X3  */
    F16MAT2X4 = 389,               /* F16MAT2X4  */
    F16MAT3X2 = 390,               /* F16MAT3X2  */
    F16MAT3X3 = 391,               /* F16MAT3X3  */
    F16MAT3X4 = 392,               /* F16MAT3X4  */
    F16MAT4X2 = 393,               /* F16MAT4X2  */
    F16MAT4X3 = 394,               /* F16MAT4X3  */
    F16MAT4X4 = 395,               /* F16MAT4X4  */
    F32MAT2X2 = 396,               /* F32MAT2X2  */
    F32MAT2X3 = 397,               /* F32MAT2X3  */
    F32MAT2X4 = 398,               /* F32MAT2X4  */
    F32MAT3X2 = 399,               /* F32MAT3X2  */
    F32MAT3X3 = 400,               /* F32MAT3X3  */
    F32MAT3X4 = 401,               /* F32MAT3X4  */
    F32MAT4X2 = 402,               /* F32MAT4X2  */
    F32MAT4X3 = 403,               /* F32MAT4X3  */
    F32MAT4X4 = 404,               /* F32MAT4X4  */
    F64MAT2X2 = 405,               /* F64MAT2X2  */
    F64MAT2X3 = 406,               /* F64MAT2X3  */
    F64MAT2X4 = 407,               /* F64MAT2X4  */
    F64MAT3X2 = 408,               /* F64MAT3X2  */
    F64MAT3X3 = 409,               /* F64MAT3X3  */
    F64MAT3X4 = 410,               /* F64MAT3X4  */
    F64MAT4X2 = 411,               /* F64MAT4X2  */
    F64MAT4X3 = 412,               /* F64MAT4X3  */
    F64MAT4X4 = 413,               /* F64MAT4X4  */
    ATOMIC_UINT = 414,             /* ATOMIC_UINT  */
    ACCSTRUCTNV = 415,             /* ACCSTRUCTNV  */
    ACCSTRUCTEXT = 416,            /* ACCSTRUCTEXT  */
    RAYQUERYEXT = 417,             /* RAYQUERYEXT  */
    FCOOPMATNV = 418,              /* FCOOPMATNV  */
    ICOOPMATNV = 419,              /* ICOOPMATNV  */
    UCOOPMATNV = 420,              /* UCOOPMATNV  */
    COOPMAT = 421,                 /* COOPMAT  */
    COOPVECNV = 422,               /* COOPVECNV  */
    HITOBJECTNV = 423,             /* HITOBJECTNV  */
    HITOBJECTATTRNV = 424,         /* HITOBJECTATTRNV  */
    TENSORLAYOUTNV = 425,          /* TENSORLAYOUTNV  */
    TENSORVIEWNV = 426,            /* TENSORVIEWNV  */
    SAMPLERCUBEARRAY = 427,        /* SAMPLERCUBEARRAY  */
    SAMPLERCUBEARRAYSHADOW = 428,  /* SAMPLERCUBEARRAYSHADOW  */
    ISAMPLERCUBEARRAY = 429,       /* ISAMPLERCUBEARRAY  */
    USAMPLERCUBEARRAY = 430,       /* USAMPLERCUBEARRAY  */
    SAMPLER1D = 431,               /* SAMPLER1D  */
    SAMPLER1DARRAY = 432,          /* SAMPLER1DARRAY  */
    SAMPLER1DARRAYSHADOW = 433,    /* SAMPLER1DARRAYSHADOW  */
    ISAMPLER1D = 434,              /* ISAMPLER1D  */
    SAMPLER1DSHADOW = 435,         /* SAMPLER1DSHADOW  */
    SAMPLER2DRECT = 436,           /* SAMPLER2DRECT  */
    SAMPLER2DRECTSHADOW = 437,     /* SAMPLER2DRECTSHADOW  */
    ISAMPLER2DRECT = 438,          /* ISAMPLER2DRECT  */
    USAMPLER2DRECT = 439,          /* USAMPLER2DRECT  */
    SAMPLERBUFFER = 440,           /* SAMPLERBUFFER  */
    ISAMPLERBUFFER = 441,          /* ISAMPLERBUFFER  */
    USAMPLERBUFFER = 442,          /* USAMPLERBUFFER  */
    SAMPLER2DMS = 443,             /* SAMPLER2DMS  */
    ISAMPLER2DMS = 444,            /* ISAMPLER2DMS  */
    USAMPLER2DMS = 445,            /* USAMPLER2DMS  */
    SAMPLER2DMSARRAY = 446,        /* SAMPLER2DMSARRAY  */
    ISAMPLER2DMSARRAY = 447,       /* ISAMPLER2DMSARRAY  */
    USAMPLER2DMSARRAY = 448,       /* USAMPLER2DMSARRAY  */
    SAMPLEREXTERNALOES = 449,      /* SAMPLEREXTERNALOES  */
    SAMPLEREXTERNAL2DY2YEXT = 450, /* SAMPLEREXTERNAL2DY2YEXT  */
    ISAMPLER1DARRAY = 451,         /* ISAMPLER1DARRAY  */
    USAMPLER1D = 452,              /* USAMPLER1D  */
    USAMPLER1DARRAY = 453,         /* USAMPLER1DARRAY  */
    F16SAMPLER1D = 454,            /* F16SAMPLER1D  */
    F16SAMPLER2D = 455,            /* F16SAMPLER2D  */
    F16SAMPLER3D = 456,            /* F16SAMPLER3D  */
    F16SAMPLER2DRECT = 457,        /* F16SAMPLER2DRECT  */
    F16SAMPLERCUBE = 458,          /* F16SAMPLERCUBE  */
    F16SAMPLER1DARRAY = 459,       /* F16SAMPLER1DARRAY  */
    F16SAMPLER2DARRAY = 460,       /* F16SAMPLER2DARRAY  */
    F16SAMPLERCUBEARRAY = 461,     /* F16SAMPLERCUBEARRAY  */
    F16SAMPLERBUFFER = 462,        /* F16SAMPLERBUFFER  */
    F16SAMPLER2DMS = 463,          /* F16SAMPLER2DMS  */
    F16SAMPLER2DMSARRAY = 464,     /* F16SAMPLER2DMSARRAY  */
    F16SAMPLER1DSHADOW = 465,      /* F16SAMPLER1DSHADOW  */
    F16SAMPLER2DSHADOW = 466,      /* F16SAMPLER2DSHADOW  */
    F16SAMPLER1DARRAYSHADOW = 467, /* F16SAMPLER1DARRAYSHADOW  */
    F16SAMPLER2DARRAYSHADOW = 468, /* F16SAMPLER2DARRAYSHADOW  */
    F16SAMPLER2DRECTSHADOW = 469,  /* F16SAMPLER2DRECTSHADOW  */
    F16SAMPLERCUBESHADOW = 470,    /* F16SAMPLERCUBESHADOW  */
    F16SAMPLERCUBEARRAYSHADOW = 471, /* F16SAMPLERCUBEARRAYSHADOW  */
    IMAGE1D = 472,                 /* IMAGE1D  */
    IIMAGE1D = 473,                /* IIMAGE1D  */
    UIMAGE1D = 474,                /* UIMAGE1D  */
    IMAGE2D = 475,                 /* IMAGE2D  */
    IIMAGE2D = 476,                /* IIMAGE2D  */
    UIMAGE2D = 477,                /* UIMAGE2D  */
    IMAGE3D = 478,                 /* IMAGE3D  */
    IIMAGE3D = 479,                /* IIMAGE3D  */
    UIMAGE3D = 480,                /* UIMAGE3D  */
    IMAGE2DRECT = 481,             /* IMAGE2DRECT  */
    IIMAGE2DRECT = 482,            /* IIMAGE2DRECT  */
    UIMAGE2DRECT = 483,            /* UIMAGE2DRECT  */
    IMAGECUBE = 484,               /* IMAGECUBE  */
    IIMAGECUBE = 485,              /* IIMAGECUBE  */
    UIMAGECUBE = 486,              /* UIMAGECUBE  */
    IMAGEBUFFER = 487,             /* IMAGEBUFFER  */
    IIMAGEBUFFER = 488,            /* IIMAGEBUFFER  */
    UIMAGEBUFFER = 489,            /* UIMAGEBUFFER  */
    IMAGE1DARRAY = 490,            /* IMAGE1DARRAY  */
    IIMAGE1DARRAY = 491,           /* IIMAGE1DARRAY  */
    UIMAGE1DARRAY = 492,           /* UIMAGE1DARRAY  */
    IMAGE2DARRAY = 493,            /* IMAGE2DARRAY  */
    IIMAGE2DARRAY = 494,           /* IIMAGE2DARRAY  */
    UIMAGE2DARRAY = 495,           /* UIMAGE2DARRAY  */
    IMAGECUBEARRAY = 496,          /* IMAGECUBEARRAY  */
    IIMAGECUBEARRAY = 497,         /* IIMAGECUBEARRAY  */
    UIMAGECUBEARRAY = 498,         /* UIMAGECUBEARRAY  */
    IMAGE2DMS = 499,               /* IMAGE2DMS  */
    IIMAGE2DMS = 500,              /* IIMAGE2DMS  */
    UIMAGE2DMS = 501,              /* UIMAGE2DMS  */
    IMAGE2DMSARRAY = 502,          /* IMAGE2DMSARRAY  */
    IIMAGE2DMSARRAY = 503,         /* IIMAGE2DMSARRAY  */
    UIMAGE2DMSARRAY = 504,         /* UIMAGE2DMSARRAY  */
    F16IMAGE1D = 505,              /* F16IMAGE1D  */
    F16IMAGE2D = 506,              /* F16IMAGE2D  */
    F16IMAGE3D = 507,              /* F16IMAGE3D  */
    F16IMAGE2DRECT = 508,          /* F16IMAGE2DRECT  */
    F16IMAGECUBE = 509,            /* F16IMAGECUBE  */
    F16IMAGE1DARRAY = 510,         /* F16IMAGE1DARRAY  */
    F16IMAGE2DARRAY = 511,         /* F16IMAGE2DARRAY  */
    F16IMAGECUBEARRAY = 512,       /* F16IMAGECUBEARRAY  */
    F16IMAGEBUFFER = 513,          /* F16IMAGEBUFFER  */
    F16IMAGE2DMS = 514,            /* F16IMAGE2DMS  */
    F16IMAGE2DMSARRAY = 515,       /* F16IMAGE2DMSARRAY  */
    I64IMAGE1D = 516,              /* I64IMAGE1D  */
    U64IMAGE1D = 517,              /* U64IMAGE1D  */
    I64IMAGE2D = 518,              /* I64IMAGE2D  */
    U64IMAGE2D = 519,              /* U64IMAGE2D  */
    I64IMAGE3D = 520,              /* I64IMAGE3D  */
    U64IMAGE3D = 521,              /* U64IMAGE3D  */
    I64IMAGE2DRECT = 522,          /* I64IMAGE2DRECT  */
    U64IMAGE2DRECT = 523,          /* U64IMAGE2DRECT  */
    I64IMAGECUBE = 524,            /* I64IMAGECUBE  */
    U64IMAGECUBE = 525,            /* U64IMAGECUBE  */
    I64IMAGEBUFFER = 526,          /* I64IMAGEBUFFER  */
    U64IMAGEBUFFER = 527,          /* U64IMAGEBUFFER  */
    I64IMAGE1DARRAY = 528,         /* I64IMAGE1DARRAY  */
    U64IMAGE1DARRAY = 529,         /* U64IMAGE1DARRAY  */
    I64IMAGE2DARRAY = 530,         /* I64IMAGE2DARRAY  */
    U64IMAGE2DARRAY = 531,         /* U64IMAGE2DARRAY  */
    I64IMAGECUBEARRAY = 532,       /* I64IMAGECUBEARRAY  */
    U64IMAGECUBEARRAY = 533,       /* U64IMAGECUBEARRAY  */
    I64IMAGE2DMS = 534,            /* I64IMAGE2DMS  */
    U64IMAGE2DMS = 535,            /* U64IMAGE2DMS  */
    I64IMAGE2DMSARRAY = 536,       /* I64IMAGE2DMSARRAY  */
    U64IMAGE2DMSARRAY = 537,       /* U64IMAGE2DMSARRAY  */
    TEXTURECUBEARRAY = 538,        /* TEXTURECUBEARRAY  */
    ITEXTURECUBEARRAY = 539,       /* ITEXTURECUBEARRAY  */
    UTEXTURECUBEARRAY = 540,       /* UTEXTURECUBEARRAY  */
    TEXTURE1D = 541,               /* TEXTURE1D  */
    ITEXTURE1D = 542,              /* ITEXTURE1D  */
    UTEXTURE1D = 543,              /* UTEXTURE1D  */
    TEXTURE1DARRAY = 544,          /* TEXTURE1DARRAY  */
    ITEXTURE1DARRAY = 545,         /* ITEXTURE1DARRAY  */
    UTEXTURE1DARRAY = 546,         /* UTEXTURE1DARRAY  */
    TEXTURE2DRECT = 547,           /* TEXTURE2DRECT  */
    ITEXTURE2DRECT = 548,          /* ITEXTURE2DRECT  */
    UTEXTURE2DRECT = 549,          /* UTEXTURE2DRECT  */
    TEXTUREBUFFER = 550,           /* TEXTUREBUFFER  */
    ITEXTUREBUFFER = 551,          /* ITEXTUREBUFFER  */
    UTEXTUREBUFFER = 552,          /* UTEXTUREBUFFER  */
    TEXTURE2DMS = 553,             /* TEXTURE2DMS  */
    ITEXTURE2DMS = 554,            /* ITEXTURE2DMS  */
    UTEXTURE2DMS = 555,            /* UTEXTURE2DMS  */
    TEXTURE2DMSARRAY = 556,        /* TEXTURE2DMSARRAY  */
    ITEXTURE2DMSARRAY = 557,       /* ITEXTURE2DMSARRAY  */
    UTEXTURE2DMSARRAY = 558,       /* UTEXTURE2DMSARRAY  */
    F16TEXTURE1D = 559,            /* F16TEXTURE1D  */
    F16TEXTURE2D = 560,            /* F16TEXTURE2D  */
    F16TEXTURE3D = 561,            /* F16TEXTURE3D  */
    F16TEXTURE2DRECT = 562,        /* F16TEXTURE2DRECT  */
    F16TEXTURECUBE = 563,          /* F16TEXTURECUBE  */
    F16TEXTURE1DARRAY = 564,       /* F16TEXTURE1DARRAY  */
    F16TEXTURE2DARRAY = 565,       /* F16TEXTURE2DARRAY  */
    F16TEXTURECUBEARRAY = 566,     /* F16TEXTURECUBEARRAY  */
    F16TEXTUREBUFFER = 567,        /* F16TEXTUREBUFFER  */
    F16TEXTURE2DMS = 568,          /* F16TEXTURE2DMS  */
    F16TEXTURE2DMSARRAY = 569,     /* F16TEXTURE2DMSARRAY  */
    SUBPASSINPUT = 570,            /* SUBPASSINPUT  */
    SUBPASSINPUTMS = 571,          /* SUBPASSINPUTMS  */
    ISUBPASSINPUT = 572,           /* ISUBPASSINPUT  */
    ISUBPASSINPUTMS = 573,         /* ISUBPASSINPUTMS  */
    USUBPASSINPUT = 574,           /* USUBPASSINPUT  */
    USUBPASSINPUTMS = 575,         /* USUBPASSINPUTMS  */
    F16SUBPASSINPUT = 576,         /* F16SUBPASSINPUT  */
    F16SUBPASSINPUTMS = 577,       /* F16SUBPASSINPUTMS  */
    SPIRV_INSTRUCTION = 578,       /* SPIRV_INSTRUCTION  */
    SPIRV_EXECUTION_MODE = 579,    /* SPIRV_EXECUTION_MODE  */
    SPIRV_EXECUTION_MODE_ID = 580, /* SPIRV_EXECUTION_MODE_ID  */
    SPIRV_DECORATE = 581,          /* SPIRV_DECORATE  */
    SPIRV_DECORATE_ID = 582,       /* SPIRV_DECORATE_ID  */
    SPIRV_DECORATE_STRING = 583,   /* SPIRV_DECORATE_STRING  */
    SPIRV_TYPE = 584,              /* SPIRV_TYPE  */
    SPIRV_STORAGE_CLASS = 585,     /* SPIRV_STORAGE_CLASS  */
    SPIRV_BY_REFERENCE = 586,      /* SPIRV_BY_REFERENCE  */
    SPIRV_LITERAL = 587,           /* SPIRV_LITERAL  */
    ATTACHMENTEXT = 588,           /* ATTACHMENTEXT  */
    IATTACHMENTEXT = 589,          /* IATTACHMENTEXT  */
    UATTACHMENTEXT = 590,          /* UATTACHMENTEXT  */
    LEFT_OP = 591,                 /* LEFT_OP  */
    RIGHT_OP = 592,                /* RIGHT_OP  */
    INC_OP = 593,                  /* INC_OP  */
    DEC_OP = 594,                  /* DEC_OP  */
    LE_OP = 595,                   /* LE_OP  */
    GE_OP = 596,                   /* GE_OP  */
    EQ_OP = 597,                   /* EQ_OP  */
    NE_OP = 598,                   /* NE_OP  */
    AND_OP = 599,                  /* AND_OP  */
    OR_OP = 600,                   /* OR_OP  */
    XOR_OP = 601,                  /* XOR_OP  */
    MUL_ASSIGN = 602,              /* MUL_ASSIGN  */
    DIV_ASSIGN = 603,              /* DIV_ASSIGN  */
    ADD_ASSIGN = 604,              /* ADD_ASSIGN  */
    MOD_ASSIGN = 605,              /* MOD_ASSIGN  */
    LEFT_ASSIGN = 606,             /* LEFT_ASSIGN  */
    RIGHT_ASSIGN = 607,            /* RIGHT_ASSIGN  */
    AND_ASSIGN = 608,              /* AND_ASSIGN  */
    XOR_ASSIGN = 609,              /* XOR_ASSIGN  */
    OR_ASSIGN = 610,               /* OR_ASSIGN  */
    SUB_ASSIGN = 611,              /* SUB_ASSIGN  */
    STRING_LITERAL = 612,          /* STRING_LITERAL  */
    LEFT_PAREN = 613,              /* LEFT_PAREN  */
    RIGHT_PAREN = 614,             /* RIGHT_PAREN  */
    LEFT_BRACKET = 615,            /* LEFT_BRACKET  */
    RIGHT_BRACKET = 616,           /* RIGHT_BRACKET  */
    LEFT_BRACE = 617,              /* LEFT_BRACE  */
    RIGHT_BRACE = 618,             /* RIGHT_BRACE  */
    DOT = 619,                     /* DOT  */
    COMMA = 620,                   /* COMMA  */
    COLON = 621,                   /* COLON  */
    EQUAL = 622,                   /* EQUAL  */
    SEMICOLON = 623,               /* SEMICOLON  */
    BANG = 624,                    /* BANG  */
    DASH = 625,                    /* DASH  */
    TILDE = 626,                   /* TILDE  */
    PLUS = 627,                    /* PLUS  */
    STAR = 628,                    /* STAR  */
    SLASH = 629,                   /* SLASH  */
    PERCENT = 630,                 /* PERCENT  */
    LEFT_ANGLE = 631,              /* LEFT_ANGLE  */
    RIGHT_ANGLE = 632,             /* RIGHT_ANGLE  */
    VERTICAL_BAR = 633,            /* VERTICAL_BAR  */
    CARET = 634,                   /* CARET  */
    AMPERSAND = 635,               /* AMPERSAND  */
    QUESTION = 636,                /* QUESTION  */
    INVARIANT = 637,               /* INVARIANT  */
    HIGH_PRECISION = 638,          /* HIGH_PRECISION  */
    MEDIUM_PRECISION = 639,        /* MEDIUM_PRECISION  */
    LOW_PRECISION = 640,           /* LOW_PRECISION  */
    PRECISION = 641,               /* PRECISION  */
    PACKED = 642,                  /* PACKED  */
    RESOURCE = 643,                /* RESOURCE  */
    SUPERP = 644,                  /* SUPERP  */
    FLOATCONSTANT = 645,           /* FLOATCONSTANT  */
    INTCONSTANT = 646,             /* INTCONSTANT  */
    UINTCONSTANT = 647,            /* UINTCONSTANT  */
    BOOLCONSTANT = 648,            /* BOOLCONSTANT  */
    IDENTIFIER = 649,              /* IDENTIFIER  */
    TYPE_NAME = 650,               /* TYPE_NAME  */
    CENTROID = 651,                /* CENTROID  */
    IN = 652,                      /* IN  */
    OUT = 653,                     /* OUT  */
    INOUT = 654,                   /* INOUT  */
    STRUCT = 655,                  /* STRUCT  */
    VOID = 656,                    /* VOID  */
    WHILE = 657,                   /* WHILE  */
    BREAK = 658,                   /* BREAK  */
    CONTINUE = 659,                /* CONTINUE  */
    DO = 660,                      /* DO  */
    ELSE = 661,                    /* ELSE  */
    FOR = 662,                     /* FOR  */
    IF = 663,                      /* IF  */
    DISCARD = 664,                 /* DISCARD  */
    RETURN = 665,                  /* RETURN  */
    SWITCH = 666,                  /* SWITCH  */
    CASE = 667,                    /* CASE  */
    DEFAULT = 668,                 /* DEFAULT  */
    TERMINATE_INVOCATION = 669,    /* TERMINATE_INVOCATION  */
    TERMINATE_RAY = 670,           /* TERMINATE_RAY  */
    IGNORE_INTERSECTION = 671,     /* IGNORE_INTERSECTION  */
    UNIFORM = 672,                 /* UNIFORM  */
    SHARED = 673,                  /* SHARED  */
    BUFFER = 674,                  /* BUFFER  */
    TILEIMAGEEXT = 675,            /* TILEIMAGEEXT  */
    FLAT = 676,                    /* FLAT  */
    SMOOTH = 677,                  /* SMOOTH  */
    LAYOUT = 678,                  /* LAYOUT  */
    DOUBLECONSTANT = 679,          /* DOUBLECONSTANT  */
    INT16CONSTANT = 680,           /* INT16CONSTANT  */
    UINT16CONSTANT = 681,          /* UINT16CONSTANT  */
    FLOAT16CONSTANT = 682,         /* FLOAT16CONSTANT  */
    INT32CONSTANT = 683,           /* INT32CONSTANT  */
    UINT32CONSTANT = 684,          /* UINT32CONSTANT  */
    INT64CONSTANT = 685,           /* INT64CONSTANT  */
    UINT64CONSTANT = 686,          /* UINT64CONSTANT  */
    SUBROUTINE = 687,              /* SUBROUTINE  */
    DEMOTE = 688,                  /* DEMOTE  */
    FUNCTION = 689,                /* FUNCTION  */
    PAYLOADNV = 690,               /* PAYLOADNV  */
    PAYLOADINNV = 691,             /* PAYLOADINNV  */
    HITATTRNV = 692,               /* HITATTRNV  */
    CALLDATANV = 693,              /* CALLDATANV  */
    CALLDATAINNV = 694,            /* CALLDATAINNV  */
    PAYLOADEXT = 695,              /* PAYLOADEXT  */
    PAYLOADINEXT = 696,            /* PAYLOADINEXT  */
    HITATTREXT = 697,              /* HITATTREXT  */
    CALLDATAEXT = 698,             /* CALLDATAEXT  */
    CALLDATAINEXT = 699,           /* CALLDATAINEXT  */
    PATCH = 700,                   /* PATCH  */
    SAMPLE = 701,                  /* SAMPLE  */
    NONUNIFORM = 702,              /* NONUNIFORM  */
    COHERENT = 703,                /* COHERENT  */
    VOLATILE = 704,                /* VOLATILE  */
    RESTRICT = 705,                /* RESTRICT  */
    READONLY = 706,                /* READONLY  */
    WRITEONLY = 707,               /* WRITEONLY  */
    NONTEMPORAL = 708,             /* NONTEMPORAL  */
    DEVICECOHERENT = 709,          /* DEVICECOHERENT  */
    QUEUEFAMILYCOHERENT = 710,     /* QUEUEFAMILYCOHERENT  */
    WORKGROUPCOHERENT = 711,       /* WORKGROUPCOHERENT  */
    SUBGROUPCOHERENT = 712,        /* SUBGROUPCOHERENT  */
    NONPRIVATE = 713,              /* NONPRIVATE  */
    SHADERCALLCOHERENT = 714,      /* SHADERCALLCOHERENT  */
    NOPERSPECTIVE = 715,           /* NOPERSPECTIVE  */
    EXPLICITINTERPAMD = 716,       /* EXPLICITINTERPAMD  */
    PERVERTEXEXT = 717,            /* PERVERTEXEXT  */
    PERVERTEXNV = 718,             /* PERVERTEXNV  */
    PERPRIMITIVENV = 719,          /* PERPRIMITIVENV  */
    PERVIEWNV = 720,               /* PERVIEWNV  */
    PERTASKNV = 721,               /* PERTASKNV  */
    PERPRIMITIVEEXT = 722,         /* PERPRIMITIVEEXT  */
    TASKPAYLOADWORKGROUPEXT = 723, /* TASKPAYLOADWORKGROUPEXT  */
    PRECISE = 724                  /* PRECISE  */
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

#line 572 "MachineIndependent/glslang_tab.cpp.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int yyparse (glslang::TParseContext* pParseContext);


#endif /* !YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED  */
