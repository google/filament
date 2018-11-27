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
    ATTRIBUTE = 258,
    VARYING = 259,
    FLOAT16_T = 260,
    FLOAT = 261,
    FLOAT32_T = 262,
    DOUBLE = 263,
    FLOAT64_T = 264,
    CONST = 265,
    BOOL = 266,
    INT = 267,
    UINT = 268,
    INT64_T = 269,
    UINT64_T = 270,
    INT32_T = 271,
    UINT32_T = 272,
    INT16_T = 273,
    UINT16_T = 274,
    INT8_T = 275,
    UINT8_T = 276,
    BREAK = 277,
    CONTINUE = 278,
    DO = 279,
    ELSE = 280,
    FOR = 281,
    IF = 282,
    DISCARD = 283,
    RETURN = 284,
    SWITCH = 285,
    CASE = 286,
    DEFAULT = 287,
    SUBROUTINE = 288,
    BVEC2 = 289,
    BVEC3 = 290,
    BVEC4 = 291,
    IVEC2 = 292,
    IVEC3 = 293,
    IVEC4 = 294,
    UVEC2 = 295,
    UVEC3 = 296,
    UVEC4 = 297,
    I64VEC2 = 298,
    I64VEC3 = 299,
    I64VEC4 = 300,
    U64VEC2 = 301,
    U64VEC3 = 302,
    U64VEC4 = 303,
    I32VEC2 = 304,
    I32VEC3 = 305,
    I32VEC4 = 306,
    U32VEC2 = 307,
    U32VEC3 = 308,
    U32VEC4 = 309,
    I16VEC2 = 310,
    I16VEC3 = 311,
    I16VEC4 = 312,
    U16VEC2 = 313,
    U16VEC3 = 314,
    U16VEC4 = 315,
    I8VEC2 = 316,
    I8VEC3 = 317,
    I8VEC4 = 318,
    U8VEC2 = 319,
    U8VEC3 = 320,
    U8VEC4 = 321,
    VEC2 = 322,
    VEC3 = 323,
    VEC4 = 324,
    MAT2 = 325,
    MAT3 = 326,
    MAT4 = 327,
    CENTROID = 328,
    IN = 329,
    OUT = 330,
    INOUT = 331,
    UNIFORM = 332,
    PATCH = 333,
    SAMPLE = 334,
    BUFFER = 335,
    SHARED = 336,
    NONUNIFORM = 337,
    PAYLOADNV = 338,
    PAYLOADINNV = 339,
    HITATTRNV = 340,
    CALLDATANV = 341,
    CALLDATAINNV = 342,
    COHERENT = 343,
    VOLATILE = 344,
    RESTRICT = 345,
    READONLY = 346,
    WRITEONLY = 347,
    DEVICECOHERENT = 348,
    QUEUEFAMILYCOHERENT = 349,
    WORKGROUPCOHERENT = 350,
    SUBGROUPCOHERENT = 351,
    NONPRIVATE = 352,
    DVEC2 = 353,
    DVEC3 = 354,
    DVEC4 = 355,
    DMAT2 = 356,
    DMAT3 = 357,
    DMAT4 = 358,
    F16VEC2 = 359,
    F16VEC3 = 360,
    F16VEC4 = 361,
    F16MAT2 = 362,
    F16MAT3 = 363,
    F16MAT4 = 364,
    F32VEC2 = 365,
    F32VEC3 = 366,
    F32VEC4 = 367,
    F32MAT2 = 368,
    F32MAT3 = 369,
    F32MAT4 = 370,
    F64VEC2 = 371,
    F64VEC3 = 372,
    F64VEC4 = 373,
    F64MAT2 = 374,
    F64MAT3 = 375,
    F64MAT4 = 376,
    NOPERSPECTIVE = 377,
    FLAT = 378,
    SMOOTH = 379,
    LAYOUT = 380,
    EXPLICITINTERPAMD = 381,
    PERVERTEXNV = 382,
    PERPRIMITIVENV = 383,
    PERVIEWNV = 384,
    PERTASKNV = 385,
    MAT2X2 = 386,
    MAT2X3 = 387,
    MAT2X4 = 388,
    MAT3X2 = 389,
    MAT3X3 = 390,
    MAT3X4 = 391,
    MAT4X2 = 392,
    MAT4X3 = 393,
    MAT4X4 = 394,
    DMAT2X2 = 395,
    DMAT2X3 = 396,
    DMAT2X4 = 397,
    DMAT3X2 = 398,
    DMAT3X3 = 399,
    DMAT3X4 = 400,
    DMAT4X2 = 401,
    DMAT4X3 = 402,
    DMAT4X4 = 403,
    F16MAT2X2 = 404,
    F16MAT2X3 = 405,
    F16MAT2X4 = 406,
    F16MAT3X2 = 407,
    F16MAT3X3 = 408,
    F16MAT3X4 = 409,
    F16MAT4X2 = 410,
    F16MAT4X3 = 411,
    F16MAT4X4 = 412,
    F32MAT2X2 = 413,
    F32MAT2X3 = 414,
    F32MAT2X4 = 415,
    F32MAT3X2 = 416,
    F32MAT3X3 = 417,
    F32MAT3X4 = 418,
    F32MAT4X2 = 419,
    F32MAT4X3 = 420,
    F32MAT4X4 = 421,
    F64MAT2X2 = 422,
    F64MAT2X3 = 423,
    F64MAT2X4 = 424,
    F64MAT3X2 = 425,
    F64MAT3X3 = 426,
    F64MAT3X4 = 427,
    F64MAT4X2 = 428,
    F64MAT4X3 = 429,
    F64MAT4X4 = 430,
    ATOMIC_UINT = 431,
    ACCSTRUCTNV = 432,
    SAMPLER1D = 433,
    SAMPLER2D = 434,
    SAMPLER3D = 435,
    SAMPLERCUBE = 436,
    SAMPLER1DSHADOW = 437,
    SAMPLER2DSHADOW = 438,
    SAMPLERCUBESHADOW = 439,
    SAMPLER1DARRAY = 440,
    SAMPLER2DARRAY = 441,
    SAMPLER1DARRAYSHADOW = 442,
    SAMPLER2DARRAYSHADOW = 443,
    ISAMPLER1D = 444,
    ISAMPLER2D = 445,
    ISAMPLER3D = 446,
    ISAMPLERCUBE = 447,
    ISAMPLER1DARRAY = 448,
    ISAMPLER2DARRAY = 449,
    USAMPLER1D = 450,
    USAMPLER2D = 451,
    USAMPLER3D = 452,
    USAMPLERCUBE = 453,
    USAMPLER1DARRAY = 454,
    USAMPLER2DARRAY = 455,
    SAMPLER2DRECT = 456,
    SAMPLER2DRECTSHADOW = 457,
    ISAMPLER2DRECT = 458,
    USAMPLER2DRECT = 459,
    SAMPLERBUFFER = 460,
    ISAMPLERBUFFER = 461,
    USAMPLERBUFFER = 462,
    SAMPLERCUBEARRAY = 463,
    SAMPLERCUBEARRAYSHADOW = 464,
    ISAMPLERCUBEARRAY = 465,
    USAMPLERCUBEARRAY = 466,
    SAMPLER2DMS = 467,
    ISAMPLER2DMS = 468,
    USAMPLER2DMS = 469,
    SAMPLER2DMSARRAY = 470,
    ISAMPLER2DMSARRAY = 471,
    USAMPLER2DMSARRAY = 472,
    SAMPLEREXTERNALOES = 473,
    F16SAMPLER1D = 474,
    F16SAMPLER2D = 475,
    F16SAMPLER3D = 476,
    F16SAMPLER2DRECT = 477,
    F16SAMPLERCUBE = 478,
    F16SAMPLER1DARRAY = 479,
    F16SAMPLER2DARRAY = 480,
    F16SAMPLERCUBEARRAY = 481,
    F16SAMPLERBUFFER = 482,
    F16SAMPLER2DMS = 483,
    F16SAMPLER2DMSARRAY = 484,
    F16SAMPLER1DSHADOW = 485,
    F16SAMPLER2DSHADOW = 486,
    F16SAMPLER1DARRAYSHADOW = 487,
    F16SAMPLER2DARRAYSHADOW = 488,
    F16SAMPLER2DRECTSHADOW = 489,
    F16SAMPLERCUBESHADOW = 490,
    F16SAMPLERCUBEARRAYSHADOW = 491,
    SAMPLER = 492,
    SAMPLERSHADOW = 493,
    TEXTURE1D = 494,
    TEXTURE2D = 495,
    TEXTURE3D = 496,
    TEXTURECUBE = 497,
    TEXTURE1DARRAY = 498,
    TEXTURE2DARRAY = 499,
    ITEXTURE1D = 500,
    ITEXTURE2D = 501,
    ITEXTURE3D = 502,
    ITEXTURECUBE = 503,
    ITEXTURE1DARRAY = 504,
    ITEXTURE2DARRAY = 505,
    UTEXTURE1D = 506,
    UTEXTURE2D = 507,
    UTEXTURE3D = 508,
    UTEXTURECUBE = 509,
    UTEXTURE1DARRAY = 510,
    UTEXTURE2DARRAY = 511,
    TEXTURE2DRECT = 512,
    ITEXTURE2DRECT = 513,
    UTEXTURE2DRECT = 514,
    TEXTUREBUFFER = 515,
    ITEXTUREBUFFER = 516,
    UTEXTUREBUFFER = 517,
    TEXTURECUBEARRAY = 518,
    ITEXTURECUBEARRAY = 519,
    UTEXTURECUBEARRAY = 520,
    TEXTURE2DMS = 521,
    ITEXTURE2DMS = 522,
    UTEXTURE2DMS = 523,
    TEXTURE2DMSARRAY = 524,
    ITEXTURE2DMSARRAY = 525,
    UTEXTURE2DMSARRAY = 526,
    F16TEXTURE1D = 527,
    F16TEXTURE2D = 528,
    F16TEXTURE3D = 529,
    F16TEXTURE2DRECT = 530,
    F16TEXTURECUBE = 531,
    F16TEXTURE1DARRAY = 532,
    F16TEXTURE2DARRAY = 533,
    F16TEXTURECUBEARRAY = 534,
    F16TEXTUREBUFFER = 535,
    F16TEXTURE2DMS = 536,
    F16TEXTURE2DMSARRAY = 537,
    SUBPASSINPUT = 538,
    SUBPASSINPUTMS = 539,
    ISUBPASSINPUT = 540,
    ISUBPASSINPUTMS = 541,
    USUBPASSINPUT = 542,
    USUBPASSINPUTMS = 543,
    F16SUBPASSINPUT = 544,
    F16SUBPASSINPUTMS = 545,
    IMAGE1D = 546,
    IIMAGE1D = 547,
    UIMAGE1D = 548,
    IMAGE2D = 549,
    IIMAGE2D = 550,
    UIMAGE2D = 551,
    IMAGE3D = 552,
    IIMAGE3D = 553,
    UIMAGE3D = 554,
    IMAGE2DRECT = 555,
    IIMAGE2DRECT = 556,
    UIMAGE2DRECT = 557,
    IMAGECUBE = 558,
    IIMAGECUBE = 559,
    UIMAGECUBE = 560,
    IMAGEBUFFER = 561,
    IIMAGEBUFFER = 562,
    UIMAGEBUFFER = 563,
    IMAGE1DARRAY = 564,
    IIMAGE1DARRAY = 565,
    UIMAGE1DARRAY = 566,
    IMAGE2DARRAY = 567,
    IIMAGE2DARRAY = 568,
    UIMAGE2DARRAY = 569,
    IMAGECUBEARRAY = 570,
    IIMAGECUBEARRAY = 571,
    UIMAGECUBEARRAY = 572,
    IMAGE2DMS = 573,
    IIMAGE2DMS = 574,
    UIMAGE2DMS = 575,
    IMAGE2DMSARRAY = 576,
    IIMAGE2DMSARRAY = 577,
    UIMAGE2DMSARRAY = 578,
    F16IMAGE1D = 579,
    F16IMAGE2D = 580,
    F16IMAGE3D = 581,
    F16IMAGE2DRECT = 582,
    F16IMAGECUBE = 583,
    F16IMAGE1DARRAY = 584,
    F16IMAGE2DARRAY = 585,
    F16IMAGECUBEARRAY = 586,
    F16IMAGEBUFFER = 587,
    F16IMAGE2DMS = 588,
    F16IMAGE2DMSARRAY = 589,
    STRUCT = 590,
    VOID = 591,
    WHILE = 592,
    IDENTIFIER = 593,
    TYPE_NAME = 594,
    FLOATCONSTANT = 595,
    DOUBLECONSTANT = 596,
    INT16CONSTANT = 597,
    UINT16CONSTANT = 598,
    INT32CONSTANT = 599,
    UINT32CONSTANT = 600,
    INTCONSTANT = 601,
    UINTCONSTANT = 602,
    INT64CONSTANT = 603,
    UINT64CONSTANT = 604,
    BOOLCONSTANT = 605,
    FLOAT16CONSTANT = 606,
    LEFT_OP = 607,
    RIGHT_OP = 608,
    INC_OP = 609,
    DEC_OP = 610,
    LE_OP = 611,
    GE_OP = 612,
    EQ_OP = 613,
    NE_OP = 614,
    AND_OP = 615,
    OR_OP = 616,
    XOR_OP = 617,
    MUL_ASSIGN = 618,
    DIV_ASSIGN = 619,
    ADD_ASSIGN = 620,
    MOD_ASSIGN = 621,
    LEFT_ASSIGN = 622,
    RIGHT_ASSIGN = 623,
    AND_ASSIGN = 624,
    XOR_ASSIGN = 625,
    OR_ASSIGN = 626,
    SUB_ASSIGN = 627,
    LEFT_PAREN = 628,
    RIGHT_PAREN = 629,
    LEFT_BRACKET = 630,
    RIGHT_BRACKET = 631,
    LEFT_BRACE = 632,
    RIGHT_BRACE = 633,
    DOT = 634,
    COMMA = 635,
    COLON = 636,
    EQUAL = 637,
    SEMICOLON = 638,
    BANG = 639,
    DASH = 640,
    TILDE = 641,
    PLUS = 642,
    STAR = 643,
    SLASH = 644,
    PERCENT = 645,
    LEFT_ANGLE = 646,
    RIGHT_ANGLE = 647,
    VERTICAL_BAR = 648,
    CARET = 649,
    AMPERSAND = 650,
    QUESTION = 651,
    INVARIANT = 652,
    PRECISE = 653,
    HIGH_PRECISION = 654,
    MEDIUM_PRECISION = 655,
    LOW_PRECISION = 656,
    PRECISION = 657,
    PACKED = 658,
    RESOURCE = 659,
    SUPERP = 660
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 70 "MachineIndependent/glslang.y" /* yacc.c:1909  */

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
    } interm;

#line 495 "MachineIndependent/glslang_tab.cpp.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (glslang::TParseContext* pParseContext);

#endif /* !YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED  */
