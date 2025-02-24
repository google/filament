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

#ifndef YY_YY_GLSLANG_TAB_AUTOGEN_H_INCLUDED
#define YY_YY_GLSLANG_TAB_AUTOGEN_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
#    define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */

#define YYLTYPE TSourceLoc
#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE_IS_TRIVIAL 1

/* Token kinds.  */
#ifndef YYTOKENTYPE
#    define YYTOKENTYPE
enum yytokentype
{
    YYEMPTY                   = -2,
    YYEOF                     = 0,   /* "end of file"  */
    YYerror                   = 256, /* error  */
    YYUNDEF                   = 257, /* "invalid token"  */
    INVARIANT                 = 258, /* INVARIANT  */
    PRECISE                   = 259, /* PRECISE  */
    HIGH_PRECISION            = 260, /* HIGH_PRECISION  */
    MEDIUM_PRECISION          = 261, /* MEDIUM_PRECISION  */
    LOW_PRECISION             = 262, /* LOW_PRECISION  */
    PRECISION                 = 263, /* PRECISION  */
    ATTRIBUTE                 = 264, /* ATTRIBUTE  */
    CONST_QUAL                = 265, /* CONST_QUAL  */
    BOOL_TYPE                 = 266, /* BOOL_TYPE  */
    FLOAT_TYPE                = 267, /* FLOAT_TYPE  */
    INT_TYPE                  = 268, /* INT_TYPE  */
    UINT_TYPE                 = 269, /* UINT_TYPE  */
    BREAK                     = 270, /* BREAK  */
    CONTINUE                  = 271, /* CONTINUE  */
    DO                        = 272, /* DO  */
    ELSE                      = 273, /* ELSE  */
    FOR                       = 274, /* FOR  */
    IF                        = 275, /* IF  */
    DISCARD                   = 276, /* DISCARD  */
    RETURN                    = 277, /* RETURN  */
    SWITCH                    = 278, /* SWITCH  */
    CASE                      = 279, /* CASE  */
    DEFAULT                   = 280, /* DEFAULT  */
    BVEC2                     = 281, /* BVEC2  */
    BVEC3                     = 282, /* BVEC3  */
    BVEC4                     = 283, /* BVEC4  */
    IVEC2                     = 284, /* IVEC2  */
    IVEC3                     = 285, /* IVEC3  */
    IVEC4                     = 286, /* IVEC4  */
    VEC2                      = 287, /* VEC2  */
    VEC3                      = 288, /* VEC3  */
    VEC4                      = 289, /* VEC4  */
    UVEC2                     = 290, /* UVEC2  */
    UVEC3                     = 291, /* UVEC3  */
    UVEC4                     = 292, /* UVEC4  */
    MATRIX2                   = 293, /* MATRIX2  */
    MATRIX3                   = 294, /* MATRIX3  */
    MATRIX4                   = 295, /* MATRIX4  */
    IN_QUAL                   = 296, /* IN_QUAL  */
    OUT_QUAL                  = 297, /* OUT_QUAL  */
    INOUT_QUAL                = 298, /* INOUT_QUAL  */
    UNIFORM                   = 299, /* UNIFORM  */
    BUFFER                    = 300, /* BUFFER  */
    VARYING                   = 301, /* VARYING  */
    MATRIX2x3                 = 302, /* MATRIX2x3  */
    MATRIX3x2                 = 303, /* MATRIX3x2  */
    MATRIX2x4                 = 304, /* MATRIX2x4  */
    MATRIX4x2                 = 305, /* MATRIX4x2  */
    MATRIX3x4                 = 306, /* MATRIX3x4  */
    MATRIX4x3                 = 307, /* MATRIX4x3  */
    SAMPLE                    = 308, /* SAMPLE  */
    CENTROID                  = 309, /* CENTROID  */
    FLAT                      = 310, /* FLAT  */
    SMOOTH                    = 311, /* SMOOTH  */
    NOPERSPECTIVE             = 312, /* NOPERSPECTIVE  */
    PATCH                     = 313, /* PATCH  */
    READONLY                  = 314, /* READONLY  */
    WRITEONLY                 = 315, /* WRITEONLY  */
    COHERENT                  = 316, /* COHERENT  */
    RESTRICT                  = 317, /* RESTRICT  */
    VOLATILE                  = 318, /* VOLATILE  */
    SHARED                    = 319, /* SHARED  */
    STRUCT                    = 320, /* STRUCT  */
    VOID_TYPE                 = 321, /* VOID_TYPE  */
    WHILE                     = 322, /* WHILE  */
    SAMPLER2D                 = 323, /* SAMPLER2D  */
    SAMPLERCUBE               = 324, /* SAMPLERCUBE  */
    SAMPLER_EXTERNAL_OES      = 325, /* SAMPLER_EXTERNAL_OES  */
    SAMPLER2DRECT             = 326, /* SAMPLER2DRECT  */
    SAMPLER2DARRAY            = 327, /* SAMPLER2DARRAY  */
    ISAMPLER2D                = 328, /* ISAMPLER2D  */
    ISAMPLER3D                = 329, /* ISAMPLER3D  */
    ISAMPLERCUBE              = 330, /* ISAMPLERCUBE  */
    ISAMPLER2DARRAY           = 331, /* ISAMPLER2DARRAY  */
    USAMPLER2D                = 332, /* USAMPLER2D  */
    USAMPLER3D                = 333, /* USAMPLER3D  */
    USAMPLERCUBE              = 334, /* USAMPLERCUBE  */
    USAMPLER2DARRAY           = 335, /* USAMPLER2DARRAY  */
    SAMPLER2DMS               = 336, /* SAMPLER2DMS  */
    ISAMPLER2DMS              = 337, /* ISAMPLER2DMS  */
    USAMPLER2DMS              = 338, /* USAMPLER2DMS  */
    SAMPLER2DMSARRAY          = 339, /* SAMPLER2DMSARRAY  */
    ISAMPLER2DMSARRAY         = 340, /* ISAMPLER2DMSARRAY  */
    USAMPLER2DMSARRAY         = 341, /* USAMPLER2DMSARRAY  */
    SAMPLER3D                 = 342, /* SAMPLER3D  */
    SAMPLER3DRECT             = 343, /* SAMPLER3DRECT  */
    SAMPLER2DSHADOW           = 344, /* SAMPLER2DSHADOW  */
    SAMPLERCUBESHADOW         = 345, /* SAMPLERCUBESHADOW  */
    SAMPLER2DARRAYSHADOW      = 346, /* SAMPLER2DARRAYSHADOW  */
    SAMPLERVIDEOWEBGL         = 347, /* SAMPLERVIDEOWEBGL  */
    SAMPLERCUBEARRAYOES       = 348, /* SAMPLERCUBEARRAYOES  */
    SAMPLERCUBEARRAYSHADOWOES = 349, /* SAMPLERCUBEARRAYSHADOWOES  */
    ISAMPLERCUBEARRAYOES      = 350, /* ISAMPLERCUBEARRAYOES  */
    USAMPLERCUBEARRAYOES      = 351, /* USAMPLERCUBEARRAYOES  */
    SAMPLERCUBEARRAYEXT       = 352, /* SAMPLERCUBEARRAYEXT  */
    SAMPLERCUBEARRAYSHADOWEXT = 353, /* SAMPLERCUBEARRAYSHADOWEXT  */
    ISAMPLERCUBEARRAYEXT      = 354, /* ISAMPLERCUBEARRAYEXT  */
    USAMPLERCUBEARRAYEXT      = 355, /* USAMPLERCUBEARRAYEXT  */
    SAMPLERBUFFER             = 356, /* SAMPLERBUFFER  */
    ISAMPLERBUFFER            = 357, /* ISAMPLERBUFFER  */
    USAMPLERBUFFER            = 358, /* USAMPLERBUFFER  */
    SAMPLEREXTERNAL2DY2YEXT   = 359, /* SAMPLEREXTERNAL2DY2YEXT  */
    IMAGE2D                   = 360, /* IMAGE2D  */
    IIMAGE2D                  = 361, /* IIMAGE2D  */
    UIMAGE2D                  = 362, /* UIMAGE2D  */
    IMAGE3D                   = 363, /* IMAGE3D  */
    IIMAGE3D                  = 364, /* IIMAGE3D  */
    UIMAGE3D                  = 365, /* UIMAGE3D  */
    IMAGE2DARRAY              = 366, /* IMAGE2DARRAY  */
    IIMAGE2DARRAY             = 367, /* IIMAGE2DARRAY  */
    UIMAGE2DARRAY             = 368, /* UIMAGE2DARRAY  */
    IMAGECUBE                 = 369, /* IMAGECUBE  */
    IIMAGECUBE                = 370, /* IIMAGECUBE  */
    UIMAGECUBE                = 371, /* UIMAGECUBE  */
    IMAGECUBEARRAYOES         = 372, /* IMAGECUBEARRAYOES  */
    IIMAGECUBEARRAYOES        = 373, /* IIMAGECUBEARRAYOES  */
    UIMAGECUBEARRAYOES        = 374, /* UIMAGECUBEARRAYOES  */
    IMAGECUBEARRAYEXT         = 375, /* IMAGECUBEARRAYEXT  */
    IIMAGECUBEARRAYEXT        = 376, /* IIMAGECUBEARRAYEXT  */
    UIMAGECUBEARRAYEXT        = 377, /* UIMAGECUBEARRAYEXT  */
    IMAGEBUFFER               = 378, /* IMAGEBUFFER  */
    IIMAGEBUFFER              = 379, /* IIMAGEBUFFER  */
    UIMAGEBUFFER              = 380, /* UIMAGEBUFFER  */
    ATOMICUINT                = 381, /* ATOMICUINT  */
    PIXELLOCALANGLE           = 382, /* PIXELLOCALANGLE  */
    IPIXELLOCALANGLE          = 383, /* IPIXELLOCALANGLE  */
    UPIXELLOCALANGLE          = 384, /* UPIXELLOCALANGLE  */
    LAYOUT                    = 385, /* LAYOUT  */
    YUVCSCSTANDARDEXT         = 386, /* YUVCSCSTANDARDEXT  */
    YUVCSCSTANDARDEXTCONSTANT = 387, /* YUVCSCSTANDARDEXTCONSTANT  */
    IDENTIFIER                = 388, /* IDENTIFIER  */
    TYPE_NAME                 = 389, /* TYPE_NAME  */
    FLOATCONSTANT             = 390, /* FLOATCONSTANT  */
    INTCONSTANT               = 391, /* INTCONSTANT  */
    UINTCONSTANT              = 392, /* UINTCONSTANT  */
    BOOLCONSTANT              = 393, /* BOOLCONSTANT  */
    FIELD_SELECTION           = 394, /* FIELD_SELECTION  */
    LEFT_OP                   = 395, /* LEFT_OP  */
    RIGHT_OP                  = 396, /* RIGHT_OP  */
    INC_OP                    = 397, /* INC_OP  */
    DEC_OP                    = 398, /* DEC_OP  */
    LE_OP                     = 399, /* LE_OP  */
    GE_OP                     = 400, /* GE_OP  */
    EQ_OP                     = 401, /* EQ_OP  */
    NE_OP                     = 402, /* NE_OP  */
    AND_OP                    = 403, /* AND_OP  */
    OR_OP                     = 404, /* OR_OP  */
    XOR_OP                    = 405, /* XOR_OP  */
    MUL_ASSIGN                = 406, /* MUL_ASSIGN  */
    DIV_ASSIGN                = 407, /* DIV_ASSIGN  */
    ADD_ASSIGN                = 408, /* ADD_ASSIGN  */
    MOD_ASSIGN                = 409, /* MOD_ASSIGN  */
    LEFT_ASSIGN               = 410, /* LEFT_ASSIGN  */
    RIGHT_ASSIGN              = 411, /* RIGHT_ASSIGN  */
    AND_ASSIGN                = 412, /* AND_ASSIGN  */
    XOR_ASSIGN                = 413, /* XOR_ASSIGN  */
    OR_ASSIGN                 = 414, /* OR_ASSIGN  */
    SUB_ASSIGN                = 415, /* SUB_ASSIGN  */
    LEFT_PAREN                = 416, /* LEFT_PAREN  */
    RIGHT_PAREN               = 417, /* RIGHT_PAREN  */
    LEFT_BRACKET              = 418, /* LEFT_BRACKET  */
    RIGHT_BRACKET             = 419, /* RIGHT_BRACKET  */
    LEFT_BRACE                = 420, /* LEFT_BRACE  */
    RIGHT_BRACE               = 421, /* RIGHT_BRACE  */
    DOT                       = 422, /* DOT  */
    COMMA                     = 423, /* COMMA  */
    COLON                     = 424, /* COLON  */
    EQUAL                     = 425, /* EQUAL  */
    SEMICOLON                 = 426, /* SEMICOLON  */
    BANG                      = 427, /* BANG  */
    DASH                      = 428, /* DASH  */
    TILDE                     = 429, /* TILDE  */
    PLUS                      = 430, /* PLUS  */
    STAR                      = 431, /* STAR  */
    SLASH                     = 432, /* SLASH  */
    PERCENT                   = 433, /* PERCENT  */
    LEFT_ANGLE                = 434, /* LEFT_ANGLE  */
    RIGHT_ANGLE               = 435, /* RIGHT_ANGLE  */
    VERTICAL_BAR              = 436, /* VERTICAL_BAR  */
    CARET                     = 437, /* CARET  */
    AMPERSAND                 = 438, /* AMPERSAND  */
    QUESTION                  = 439  /* QUESTION  */
};
typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
union YYSTYPE
{

    struct
    {
        union
        {
            const char *string;  // pool allocated.
            float f;
            int i;
            unsigned int u;
            bool b;
        };
        const TSymbol *symbol;
    } lex;
    struct
    {
        TOperator op;
        union
        {
            TIntermNode *intermNode;
            TIntermNodePair nodePair;
            TIntermTyped *intermTypedNode;
            TIntermAggregate *intermAggregate;
            TIntermBlock *intermBlock;
            TIntermDeclaration *intermDeclaration;
            TIntermFunctionPrototype *intermFunctionPrototype;
            TIntermSwitch *intermSwitch;
            TIntermCase *intermCase;
        };
        union
        {
            TVector<unsigned int> *arraySizes;
            TTypeSpecifierNonArray typeSpecifierNonArray;
            TPublicType type;
            TPrecision precision;
            TLayoutQualifier layoutQualifier;
            TQualifier qualifier;
            TFunction *function;
            TFunctionLookup *functionLookup;
            TParameter param;
            TDeclarator *declarator;
            TDeclaratorList *declaratorList;
            TFieldList *fieldList;
            TQualifierWrapperBase *qualifierWrapper;
            TTypeQualifierBuilder *typeQualifierBuilder;
        };
    } interm;
};
typedef union YYSTYPE YYSTYPE;
#    define YYSTYPE_IS_TRIVIAL 1
#    define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if !defined YYLTYPE && !defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
    int first_line;
    int first_column;
    int last_line;
    int last_column;
};
#    define YYLTYPE_IS_DECLARED 1
#    define YYLTYPE_IS_TRIVIAL 1
#endif

int yyparse(TParseContext *context, void *scanner);

#endif /* !YY_YY_GLSLANG_TAB_AUTOGEN_H_INCLUDED  */
