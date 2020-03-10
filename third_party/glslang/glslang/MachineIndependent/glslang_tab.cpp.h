/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED
# define YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    CONST = 258,
    BOOL = 259,
    INT = 260,
    UINT = 261,
    FLOAT = 262,
    BVEC2 = 263,
    BVEC3 = 264,
    BVEC4 = 265,
    IVEC2 = 266,
    IVEC3 = 267,
    IVEC4 = 268,
    UVEC2 = 269,
    UVEC3 = 270,
    UVEC4 = 271,
    VEC2 = 272,
    VEC3 = 273,
    VEC4 = 274,
    MAT2 = 275,
    MAT3 = 276,
    MAT4 = 277,
    MAT2X2 = 278,
    MAT2X3 = 279,
    MAT2X4 = 280,
    MAT3X2 = 281,
    MAT3X3 = 282,
    MAT3X4 = 283,
    MAT4X2 = 284,
    MAT4X3 = 285,
    MAT4X4 = 286,
    SAMPLER2D = 287,
    SAMPLER3D = 288,
    SAMPLERCUBE = 289,
    SAMPLER2DSHADOW = 290,
    SAMPLERCUBESHADOW = 291,
    SAMPLER2DARRAY = 292,
    SAMPLER2DARRAYSHADOW = 293,
    ISAMPLER2D = 294,
    ISAMPLER3D = 295,
    ISAMPLERCUBE = 296,
    ISAMPLER2DARRAY = 297,
    USAMPLER2D = 298,
    USAMPLER3D = 299,
    USAMPLERCUBE = 300,
    USAMPLER2DARRAY = 301,
    SAMPLER = 302,
    SAMPLERSHADOW = 303,
    TEXTURE2D = 304,
    TEXTURE3D = 305,
    TEXTURECUBE = 306,
    TEXTURE2DARRAY = 307,
    ITEXTURE2D = 308,
    ITEXTURE3D = 309,
    ITEXTURECUBE = 310,
    ITEXTURE2DARRAY = 311,
    UTEXTURE2D = 312,
    UTEXTURE3D = 313,
    UTEXTURECUBE = 314,
    UTEXTURE2DARRAY = 315,
    ATTRIBUTE = 316,
    VARYING = 317,
    FLOAT16_T = 318,
    FLOAT32_T = 319,
    DOUBLE = 320,
    FLOAT64_T = 321,
    INT64_T = 322,
    UINT64_T = 323,
    INT32_T = 324,
    UINT32_T = 325,
    INT16_T = 326,
    UINT16_T = 327,
    INT8_T = 328,
    UINT8_T = 329,
    I64VEC2 = 330,
    I64VEC3 = 331,
    I64VEC4 = 332,
    U64VEC2 = 333,
    U64VEC3 = 334,
    U64VEC4 = 335,
    I32VEC2 = 336,
    I32VEC3 = 337,
    I32VEC4 = 338,
    U32VEC2 = 339,
    U32VEC3 = 340,
    U32VEC4 = 341,
    I16VEC2 = 342,
    I16VEC3 = 343,
    I16VEC4 = 344,
    U16VEC2 = 345,
    U16VEC3 = 346,
    U16VEC4 = 347,
    I8VEC2 = 348,
    I8VEC3 = 349,
    I8VEC4 = 350,
    U8VEC2 = 351,
    U8VEC3 = 352,
    U8VEC4 = 353,
    DVEC2 = 354,
    DVEC3 = 355,
    DVEC4 = 356,
    DMAT2 = 357,
    DMAT3 = 358,
    DMAT4 = 359,
    F16VEC2 = 360,
    F16VEC3 = 361,
    F16VEC4 = 362,
    F16MAT2 = 363,
    F16MAT3 = 364,
    F16MAT4 = 365,
    F32VEC2 = 366,
    F32VEC3 = 367,
    F32VEC4 = 368,
    F32MAT2 = 369,
    F32MAT3 = 370,
    F32MAT4 = 371,
    F64VEC2 = 372,
    F64VEC3 = 373,
    F64VEC4 = 374,
    F64MAT2 = 375,
    F64MAT3 = 376,
    F64MAT4 = 377,
    DMAT2X2 = 378,
    DMAT2X3 = 379,
    DMAT2X4 = 380,
    DMAT3X2 = 381,
    DMAT3X3 = 382,
    DMAT3X4 = 383,
    DMAT4X2 = 384,
    DMAT4X3 = 385,
    DMAT4X4 = 386,
    F16MAT2X2 = 387,
    F16MAT2X3 = 388,
    F16MAT2X4 = 389,
    F16MAT3X2 = 390,
    F16MAT3X3 = 391,
    F16MAT3X4 = 392,
    F16MAT4X2 = 393,
    F16MAT4X3 = 394,
    F16MAT4X4 = 395,
    F32MAT2X2 = 396,
    F32MAT2X3 = 397,
    F32MAT2X4 = 398,
    F32MAT3X2 = 399,
    F32MAT3X3 = 400,
    F32MAT3X4 = 401,
    F32MAT4X2 = 402,
    F32MAT4X3 = 403,
    F32MAT4X4 = 404,
    F64MAT2X2 = 405,
    F64MAT2X3 = 406,
    F64MAT2X4 = 407,
    F64MAT3X2 = 408,
    F64MAT3X3 = 409,
    F64MAT3X4 = 410,
    F64MAT4X2 = 411,
    F64MAT4X3 = 412,
    F64MAT4X4 = 413,
    ATOMIC_UINT = 414,
    ACCSTRUCTNV = 415,
    FCOOPMATNV = 416,
    ICOOPMATNV = 417,
    UCOOPMATNV = 418,
    SAMPLERCUBEARRAY = 419,
    SAMPLERCUBEARRAYSHADOW = 420,
    ISAMPLERCUBEARRAY = 421,
    USAMPLERCUBEARRAY = 422,
    SAMPLER1D = 423,
    SAMPLER1DARRAY = 424,
    SAMPLER1DARRAYSHADOW = 425,
    ISAMPLER1D = 426,
    SAMPLER1DSHADOW = 427,
    SAMPLER2DRECT = 428,
    SAMPLER2DRECTSHADOW = 429,
    ISAMPLER2DRECT = 430,
    USAMPLER2DRECT = 431,
    SAMPLERBUFFER = 432,
    ISAMPLERBUFFER = 433,
    USAMPLERBUFFER = 434,
    SAMPLER2DMS = 435,
    ISAMPLER2DMS = 436,
    USAMPLER2DMS = 437,
    SAMPLER2DMSARRAY = 438,
    ISAMPLER2DMSARRAY = 439,
    USAMPLER2DMSARRAY = 440,
    SAMPLEREXTERNALOES = 441,
    SAMPLEREXTERNAL2DY2YEXT = 442,
    ISAMPLER1DARRAY = 443,
    USAMPLER1D = 444,
    USAMPLER1DARRAY = 445,
    F16SAMPLER1D = 446,
    F16SAMPLER2D = 447,
    F16SAMPLER3D = 448,
    F16SAMPLER2DRECT = 449,
    F16SAMPLERCUBE = 450,
    F16SAMPLER1DARRAY = 451,
    F16SAMPLER2DARRAY = 452,
    F16SAMPLERCUBEARRAY = 453,
    F16SAMPLERBUFFER = 454,
    F16SAMPLER2DMS = 455,
    F16SAMPLER2DMSARRAY = 456,
    F16SAMPLER1DSHADOW = 457,
    F16SAMPLER2DSHADOW = 458,
    F16SAMPLER1DARRAYSHADOW = 459,
    F16SAMPLER2DARRAYSHADOW = 460,
    F16SAMPLER2DRECTSHADOW = 461,
    F16SAMPLERCUBESHADOW = 462,
    F16SAMPLERCUBEARRAYSHADOW = 463,
    IMAGE1D = 464,
    IIMAGE1D = 465,
    UIMAGE1D = 466,
    IMAGE2D = 467,
    IIMAGE2D = 468,
    UIMAGE2D = 469,
    IMAGE3D = 470,
    IIMAGE3D = 471,
    UIMAGE3D = 472,
    IMAGE2DRECT = 473,
    IIMAGE2DRECT = 474,
    UIMAGE2DRECT = 475,
    IMAGECUBE = 476,
    IIMAGECUBE = 477,
    UIMAGECUBE = 478,
    IMAGEBUFFER = 479,
    IIMAGEBUFFER = 480,
    UIMAGEBUFFER = 481,
    IMAGE1DARRAY = 482,
    IIMAGE1DARRAY = 483,
    UIMAGE1DARRAY = 484,
    IMAGE2DARRAY = 485,
    IIMAGE2DARRAY = 486,
    UIMAGE2DARRAY = 487,
    IMAGECUBEARRAY = 488,
    IIMAGECUBEARRAY = 489,
    UIMAGECUBEARRAY = 490,
    IMAGE2DMS = 491,
    IIMAGE2DMS = 492,
    UIMAGE2DMS = 493,
    IMAGE2DMSARRAY = 494,
    IIMAGE2DMSARRAY = 495,
    UIMAGE2DMSARRAY = 496,
    F16IMAGE1D = 497,
    F16IMAGE2D = 498,
    F16IMAGE3D = 499,
    F16IMAGE2DRECT = 500,
    F16IMAGECUBE = 501,
    F16IMAGE1DARRAY = 502,
    F16IMAGE2DARRAY = 503,
    F16IMAGECUBEARRAY = 504,
    F16IMAGEBUFFER = 505,
    F16IMAGE2DMS = 506,
    F16IMAGE2DMSARRAY = 507,
    TEXTURECUBEARRAY = 508,
    ITEXTURECUBEARRAY = 509,
    UTEXTURECUBEARRAY = 510,
    TEXTURE1D = 511,
    ITEXTURE1D = 512,
    UTEXTURE1D = 513,
    TEXTURE1DARRAY = 514,
    ITEXTURE1DARRAY = 515,
    UTEXTURE1DARRAY = 516,
    TEXTURE2DRECT = 517,
    ITEXTURE2DRECT = 518,
    UTEXTURE2DRECT = 519,
    TEXTUREBUFFER = 520,
    ITEXTUREBUFFER = 521,
    UTEXTUREBUFFER = 522,
    TEXTURE2DMS = 523,
    ITEXTURE2DMS = 524,
    UTEXTURE2DMS = 525,
    TEXTURE2DMSARRAY = 526,
    ITEXTURE2DMSARRAY = 527,
    UTEXTURE2DMSARRAY = 528,
    F16TEXTURE1D = 529,
    F16TEXTURE2D = 530,
    F16TEXTURE3D = 531,
    F16TEXTURE2DRECT = 532,
    F16TEXTURECUBE = 533,
    F16TEXTURE1DARRAY = 534,
    F16TEXTURE2DARRAY = 535,
    F16TEXTURECUBEARRAY = 536,
    F16TEXTUREBUFFER = 537,
    F16TEXTURE2DMS = 538,
    F16TEXTURE2DMSARRAY = 539,
    SUBPASSINPUT = 540,
    SUBPASSINPUTMS = 541,
    ISUBPASSINPUT = 542,
    ISUBPASSINPUTMS = 543,
    USUBPASSINPUT = 544,
    USUBPASSINPUTMS = 545,
    F16SUBPASSINPUT = 546,
    F16SUBPASSINPUTMS = 547,
    LEFT_OP = 548,
    RIGHT_OP = 549,
    INC_OP = 550,
    DEC_OP = 551,
    LE_OP = 552,
    GE_OP = 553,
    EQ_OP = 554,
    NE_OP = 555,
    AND_OP = 556,
    OR_OP = 557,
    XOR_OP = 558,
    MUL_ASSIGN = 559,
    DIV_ASSIGN = 560,
    ADD_ASSIGN = 561,
    MOD_ASSIGN = 562,
    LEFT_ASSIGN = 563,
    RIGHT_ASSIGN = 564,
    AND_ASSIGN = 565,
    XOR_ASSIGN = 566,
    OR_ASSIGN = 567,
    SUB_ASSIGN = 568,
    STRING_LITERAL = 569,
    LEFT_PAREN = 570,
    RIGHT_PAREN = 571,
    LEFT_BRACKET = 572,
    RIGHT_BRACKET = 573,
    LEFT_BRACE = 574,
    RIGHT_BRACE = 575,
    DOT = 576,
    COMMA = 577,
    COLON = 578,
    EQUAL = 579,
    SEMICOLON = 580,
    BANG = 581,
    DASH = 582,
    TILDE = 583,
    PLUS = 584,
    STAR = 585,
    SLASH = 586,
    PERCENT = 587,
    LEFT_ANGLE = 588,
    RIGHT_ANGLE = 589,
    VERTICAL_BAR = 590,
    CARET = 591,
    AMPERSAND = 592,
    QUESTION = 593,
    INVARIANT = 594,
    HIGH_PRECISION = 595,
    MEDIUM_PRECISION = 596,
    LOW_PRECISION = 597,
    PRECISION = 598,
    PACKED = 599,
    RESOURCE = 600,
    SUPERP = 601,
    FLOATCONSTANT = 602,
    INTCONSTANT = 603,
    UINTCONSTANT = 604,
    BOOLCONSTANT = 605,
    IDENTIFIER = 606,
    TYPE_NAME = 607,
    CENTROID = 608,
    IN = 609,
    OUT = 610,
    INOUT = 611,
    STRUCT = 612,
    VOID = 613,
    WHILE = 614,
    BREAK = 615,
    CONTINUE = 616,
    DO = 617,
    ELSE = 618,
    FOR = 619,
    IF = 620,
    DISCARD = 621,
    RETURN = 622,
    SWITCH = 623,
    CASE = 624,
    DEFAULT = 625,
    UNIFORM = 626,
    SHARED = 627,
    BUFFER = 628,
    FLAT = 629,
    SMOOTH = 630,
    LAYOUT = 631,
    DOUBLECONSTANT = 632,
    INT16CONSTANT = 633,
    UINT16CONSTANT = 634,
    FLOAT16CONSTANT = 635,
    INT32CONSTANT = 636,
    UINT32CONSTANT = 637,
    INT64CONSTANT = 638,
    UINT64CONSTANT = 639,
    SUBROUTINE = 640,
    DEMOTE = 641,
    PAYLOADNV = 642,
    PAYLOADINNV = 643,
    HITATTRNV = 644,
    CALLDATANV = 645,
    CALLDATAINNV = 646,
    PATCH = 647,
    SAMPLE = 648,
    NONUNIFORM = 649,
    COHERENT = 650,
    VOLATILE = 651,
    RESTRICT = 652,
    READONLY = 653,
    WRITEONLY = 654,
    DEVICECOHERENT = 655,
    QUEUEFAMILYCOHERENT = 656,
    WORKGROUPCOHERENT = 657,
    SUBGROUPCOHERENT = 658,
    NONPRIVATE = 659,
    NOPERSPECTIVE = 660,
    EXPLICITINTERPAMD = 661,
    PERVERTEXNV = 662,
    PERPRIMITIVENV = 663,
    PERVIEWNV = 664,
    PERTASKNV = 665,
    PRECISE = 666
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 96 "MachineIndependent/glslang.y" /* yacc.c:1909  */

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
        glslang::TArraySizes* typeParameters;
    } interm;

#line 502 "MachineIndependent/glslang_tab.cpp.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (glslang::TParseContext* pParseContext);

#endif /* !YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED  */
