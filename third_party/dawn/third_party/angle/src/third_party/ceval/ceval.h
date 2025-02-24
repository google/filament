#ifndef CEVAL
#define CEVAL
//functions accessible from main() 
// - double ceval_result(char * inp) returns the result of valid math expression stored as a char array `inp`
// - void ceval_tree(char * inp) prints the parse tree for the input expression `inp`

#include<stdio.h>
#include<string.h>
#include<math.h>
#include<ctype.h>
#include<stdarg.h>
/****************************************** TOKENS ***********************************************/
typedef enum ceval_node_id {
    CEVAL_WHITESPACE, CEVAL_OPENPAR, CEVAL_CLOSEPAR, CEVAL_COMMA, 
    CEVAL_OR, CEVAL_AND, CEVAL_BIT_OR, CEVAL_BIT_XOR,
    CEVAL_BIT_AND, CEVAL_EQUAL, CEVAL_NOTEQUAL,CEVAL_LESSER,
    CEVAL_GREATER, CEVAL_LESSER_S, CEVAL_GREATER_S, CEVAL_BIT_LSHIFT, 
    CEVAL_BIT_RSHIFT, CEVAL_PLUS, CEVAL_MINUS, CEVAL_TIMES, 
    CEVAL_DIVIDE, CEVAL_MODULO, CEVAL_QUOTIENT, CEVAL_POW,
    CEVAL_GCD, CEVAL_HCF, CEVAL_LCM, CEVAL_LOG,
    CEVAL_ATAN2, CEVAL_SCI2DEC, CEVAL_POWFUN, 

    CEVAL_ABS, CEVAL_EXP, CEVAL_SQRT,CEVAL_CBRT, 
    CEVAL_LN, CEVAL_LOG10, CEVAL_CEIL, CEVAL_FLOOR, 
    CEVAL_SIGNUM, CEVAL_FACTORIAL, CEVAL_INT, CEVAL_FRAC, 
    CEVAL_DEG2RAD, CEVAL_RAD2DEG, CEVAL_SIN, CEVAL_COS, 
    CEVAL_TAN, CEVAL_ASIN, CEVAL_ACOS, CEVAL_ATAN, 
    CEVAL_SINH, CEVAL_COSH, CEVAL_TANH,CEVAL_NOT, 
    CEVAL_BIT_NOT,CEVAL_POSSIGN, CEVAL_NEGSIGN, 
    
    CEVAL_NUMBER, CEVAL_CONST_PI, CEVAL_CONST_E
} ceval_node_id;
typedef enum ceval_token_prec_specifiers {
// precedences :: <https://en.cppreference.com/w/cpp/language/operator_precedence>
// these precision specifiers are ordered in the ascending order of their precedences
// here, the higher precedence operators are evaluated first and end up at the bottom of the parse trees
    CEVAL_PREC_IGNORE, 
    // {' ', '\t', '\n', '\b', '\r'}
    CEVAL_PREC_PARANTHESES,
    // {'(', ')'}
    CEVAL_PREC_COMMA_OPR,
    // {','}
    CEVAL_PREC_LOGICAL_OR_OPR,
    // {'||'}
    CEVAL_PREC_LOGICAL_AND_OPR,
    // {'&&'}
    CEVAL_PREC_BIT_OR_OPR,
    // {'|'}
    CEVAL_PREC_BIT_XOR_OPR,
    // {'^'}
    CEVAL_PREC_BIT_AND_OPR,
    // {'&'}
    CEVAL_PREC_EQUALITY_OPRS,
    // {'==', '!='}
    CEVAL_PREC_RELATIONAL_OPRS,
    // {'<', '>', '<=', '>='}
    CEVAL_PREC_BIT_SHIFT_OPRS,
    // {'<<', '>>'}
    CEVAL_PREC_ADDITIVE_OPRS,
    // {'+', '-'}
    CEVAL_PREC_SIGN_OPRS,
    // {'+', '-'}
    CEVAL_PREC_MULTIPLICATIVE_OPRS,
    // {'*', '/', '%', '//'}
    CEVAL_PREC_EXPONENTIATION_OPR,
    // {'**'}
    CEVAL_PREC_FUNCTIONS,
    // {
    //     'exp()', 'sqrt()', 'cbrt()', 'sin()',
    //     'cos()', 'tan()', 'asin()', 'acos()', 
    //     'atan()', 'sinh()', 'cosh()', 'tanh()', 
    //     'abs()', 'ceil()', 'floor()', 'log10()', 
    //     'ln()', 'deg2rad()', 'rad2deg()', 'signum()',
    //     'int()', 'frac()', 'fact()', `pow()`, 
    //     `atan2()`, `gcd()`, `hcf()`, `lcm()`,
    //     `log()`
    // }
    CEVAL_PREC_NOT_OPRS,
    // {'!', '~'}}
    CEVAL_PREC_SCI2DEC_OPR,
    // {'e'},
    CEVAL_PREC_NUMERIC
    // {'_pi', '_e', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}
} ceval_token_prec_specifiers;
typedef enum ceval_token_type {
    CEVAL_UNARY_OPERATOR,
    CEVAL_BINARY_OPERATOR,
    CEVAL_UNARY_FUNCTION,
    CEVAL_BINARY_FUNCTION,
    CEVAL_OTHER
} ceval_token_type;
typedef struct ceval_token_info_ {
    ceval_node_id id;
    const char * symbol; 
    double prec;
    ceval_token_type token_type;
} ceval_token_info_; 
ceval_token_info_ ceval_token_info[] = {
    { CEVAL_WHITESPACE, " ", CEVAL_PREC_IGNORE, CEVAL_OTHER },
    { CEVAL_WHITESPACE, "\n", CEVAL_PREC_IGNORE, CEVAL_OTHER },
    { CEVAL_WHITESPACE, "\t", CEVAL_PREC_IGNORE, CEVAL_OTHER },
    { CEVAL_WHITESPACE, "\r", CEVAL_PREC_IGNORE, CEVAL_OTHER },
    { CEVAL_WHITESPACE, "\b", CEVAL_PREC_IGNORE, CEVAL_OTHER },

    { CEVAL_DEG2RAD, "deg2rad", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_RAD2DEG, "rad2deg", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },

    { CEVAL_SIGNUM, "signum", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },

    { CEVAL_ATAN2, "atan2", CEVAL_PREC_FUNCTIONS, CEVAL_BINARY_FUNCTION }, 
    { CEVAL_LOG10, "log10", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_FLOOR, "floor", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },

    { CEVAL_SQRT, "sqrt", CEVAL_PREC_FUNCTIONS , CEVAL_UNARY_FUNCTION },
    { CEVAL_CBRT, "cbrt", CEVAL_PREC_FUNCTIONS , CEVAL_UNARY_FUNCTION },
    { CEVAL_CEIL, "ceil", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_FRAC, "frac", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_FACTORIAL, "fact", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION }, 
    { CEVAL_SINH, "sinh", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION }, 
    { CEVAL_COSH, "cosh", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_TANH, "tanh", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_ASIN, "asin", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION }, 
    { CEVAL_ACOS, "acos", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_ATAN, "atan", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    
    { CEVAL_POWFUN, "pow", CEVAL_PREC_FUNCTIONS, CEVAL_BINARY_FUNCTION }, 
    { CEVAL_GCD, "gcd", CEVAL_PREC_FUNCTIONS, CEVAL_BINARY_FUNCTION }, 
    { CEVAL_HCF, "hcf", CEVAL_PREC_FUNCTIONS, CEVAL_BINARY_FUNCTION }, 
    { CEVAL_LCM, "lcm", CEVAL_PREC_FUNCTIONS, CEVAL_BINARY_FUNCTION }, 
    { CEVAL_LOG, "log", CEVAL_PREC_FUNCTIONS, CEVAL_BINARY_FUNCTION }, 
    { CEVAL_INT, "int", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_SIN, "sin", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_COS, "cos", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_TAN, "tan", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_ABS, "abs", CEVAL_PREC_FUNCTIONS , CEVAL_UNARY_FUNCTION },
    { CEVAL_EXP, "exp", CEVAL_PREC_FUNCTIONS , CEVAL_UNARY_FUNCTION },
    { CEVAL_CONST_PI, "_pi", CEVAL_PREC_NUMERIC, CEVAL_OTHER },

    { CEVAL_CONST_E, "_e", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
    { CEVAL_LN, "ln", CEVAL_PREC_FUNCTIONS, CEVAL_UNARY_FUNCTION },
    { CEVAL_OR, "||", CEVAL_PREC_LOGICAL_OR_OPR, CEVAL_BINARY_OPERATOR },
    { CEVAL_AND, "&&", CEVAL_PREC_LOGICAL_AND_OPR, CEVAL_BINARY_OPERATOR },
    { CEVAL_EQUAL, "==", CEVAL_PREC_EQUALITY_OPRS, CEVAL_BINARY_OPERATOR },
    { CEVAL_NOTEQUAL, "!=", CEVAL_PREC_EQUALITY_OPRS, CEVAL_BINARY_OPERATOR },
    { CEVAL_LESSER, "<=", CEVAL_PREC_RELATIONAL_OPRS , CEVAL_BINARY_OPERATOR },
    { CEVAL_GREATER, ">=", CEVAL_PREC_RELATIONAL_OPRS , CEVAL_BINARY_OPERATOR },
    { CEVAL_BIT_LSHIFT, "<<", CEVAL_PREC_BIT_SHIFT_OPRS, CEVAL_BINARY_OPERATOR},
    { CEVAL_BIT_RSHIFT, ">>", CEVAL_PREC_BIT_SHIFT_OPRS, CEVAL_BINARY_OPERATOR},
    { CEVAL_QUOTIENT, "//", CEVAL_PREC_MULTIPLICATIVE_OPRS , CEVAL_BINARY_OPERATOR }, 
    { CEVAL_POW, "**", CEVAL_PREC_EXPONENTIATION_OPR , CEVAL_BINARY_OPERATOR },

    { CEVAL_OPENPAR, "(", CEVAL_PREC_PARANTHESES, CEVAL_OTHER },
    { CEVAL_CLOSEPAR, ")", CEVAL_PREC_PARANTHESES, CEVAL_OTHER },
    { CEVAL_COMMA, ",", CEVAL_PREC_COMMA_OPR , CEVAL_BINARY_OPERATOR },
    { CEVAL_BIT_OR, "|", CEVAL_PREC_BIT_OR_OPR, CEVAL_BINARY_OPERATOR},
    { CEVAL_BIT_XOR, "^", CEVAL_PREC_BIT_XOR_OPR, CEVAL_BINARY_OPERATOR},
    { CEVAL_BIT_AND, "&", CEVAL_PREC_BIT_AND_OPR, CEVAL_BINARY_OPERATOR},
    { CEVAL_LESSER_S, "<", CEVAL_PREC_RELATIONAL_OPRS , CEVAL_BINARY_OPERATOR },
    { CEVAL_GREATER_S, ">", CEVAL_PREC_RELATIONAL_OPRS , CEVAL_BINARY_OPERATOR },
    { CEVAL_PLUS, "+", CEVAL_PREC_ADDITIVE_OPRS , CEVAL_BINARY_OPERATOR },
    { CEVAL_MINUS, "-", CEVAL_PREC_ADDITIVE_OPRS , CEVAL_BINARY_OPERATOR },
    { CEVAL_POSSIGN, "+", CEVAL_PREC_SIGN_OPRS, CEVAL_UNARY_OPERATOR }, 
    { CEVAL_NEGSIGN, "-", CEVAL_PREC_SIGN_OPRS, CEVAL_UNARY_OPERATOR }, 
    { CEVAL_TIMES, "*", CEVAL_PREC_MULTIPLICATIVE_OPRS , CEVAL_BINARY_OPERATOR },
    { CEVAL_DIVIDE, "/", CEVAL_PREC_MULTIPLICATIVE_OPRS , CEVAL_BINARY_OPERATOR },
    { CEVAL_MODULO, "%", CEVAL_PREC_MULTIPLICATIVE_OPRS , CEVAL_BINARY_OPERATOR },
    { CEVAL_NOT, "!", CEVAL_PREC_NOT_OPRS, CEVAL_UNARY_FUNCTION},
    { CEVAL_BIT_NOT, "~", CEVAL_PREC_NOT_OPRS, CEVAL_UNARY_OPERATOR},

    { CEVAL_SCI2DEC, "e", CEVAL_PREC_SCI2DEC_OPR , CEVAL_BINARY_OPERATOR },
    { CEVAL_NUMBER, "0", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
    { CEVAL_NUMBER, "1", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
    { CEVAL_NUMBER, "2", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
    { CEVAL_NUMBER, "3", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
    { CEVAL_NUMBER, "4", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
    { CEVAL_NUMBER, "5", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
    { CEVAL_NUMBER, "6", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
    { CEVAL_NUMBER, "7", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
    { CEVAL_NUMBER, "8", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
    { CEVAL_NUMBER, "9", CEVAL_PREC_NUMERIC, CEVAL_OTHER },
}; 
#ifndef CEVAL_TOKEN_TABLE_SIZE
#define CEVAL_TOKEN_TABLE_SIZE sizeof(ceval_token_info) / sizeof(ceval_token_info[0])
#endif
int ceval_is_binary_opr(ceval_node_id id) {
    for(unsigned int i = 0; i < CEVAL_TOKEN_TABLE_SIZE; i++) {
        if (ceval_token_info[i].id == id && ceval_token_info[i].token_type == CEVAL_BINARY_OPERATOR) {
            return 1;
        }
    }
    return 0;
}
int ceval_is_binary_fun(ceval_node_id id) {
    for(unsigned int i = 0; i < CEVAL_TOKEN_TABLE_SIZE; i++) {
        if (ceval_token_info[i].id == id && ceval_token_info[i].token_type == CEVAL_BINARY_FUNCTION) {
            return 1;
        }
    }
    return 0;
}
const char * ceval_token_symbol(ceval_node_id id) {
    for(unsigned int i = 0; i < CEVAL_TOKEN_TABLE_SIZE; i++) {
        if (id == ceval_token_info[i].id) {
            return ceval_token_info[i].symbol;
        }
    }
return "";
}
ceval_node_id ceval_token_id(char * symbol) {
    for(unsigned int i = 0; i < CEVAL_TOKEN_TABLE_SIZE; i++) {
        if (!strcmp(ceval_token_info[i].symbol, symbol)) {
            return ceval_token_info[i].id;
        }
    }
return CEVAL_WHITESPACE;
}
double ceval_token_prec(ceval_node_id id) {
    for(unsigned int i = 0; i < CEVAL_TOKEN_TABLE_SIZE; i++) {
        if (id == ceval_token_info[i].id) {
            return ceval_token_info[i].prec;
        }
    }
return 0;
}
typedef struct ceval_node {
    enum ceval_node_id id;
    double pre;
    double number;
    struct ceval_node * left, * right, * parent;
}
ceval_node;
#ifdef __cplusplus
  #define CEVAL_CXX
  #include<iostream>
  #include<string>
#endif
/***************************************** !TOKENS *******************************************/

/****************************************** FUNCTIONS ******************************************/
//constant definitions
#ifdef M_PI
#define CEVAL_PI M_PI
#else
#define CEVAL_PI 3.14159265358979323846
#endif
#ifdef M_E
#define CEVAL_E M_E
#else
#define CEVAL_E 2.71828182845904523536
#endif

#ifndef CEVAL_EPSILON
#define CEVAL_EPSILON 1e-2
#endif
#ifndef CEVAL_DELTA
#define CEVAL_DELTA 1e-6
#endif
#ifndef CEVAL_MAX_DIGITS
#define CEVAL_MAX_DIGITS 15
#endif
//these can be defined by the user before the include directive depending the desired level of precision

//helper function prototypes
void ceval_error(const char * , ...);
double ceval_gcd_binary(int, int);
char * ceval_shrink(char * );

//single argument funtion prototypes
double ceval_signum(double);
double ceval_asin(double);
double ceval_acos(double);
double ceval_atan(double);
double ceval_sin(double);
double ceval_cos(double);
double ceval_tan(double);
double ceval_sinh(double);
double ceval_cosh(double);
double ceval_tanh(double);
double ceval_rad2deg(double);
double ceval_deg2rad(double);
double ceval_int_part(double);
double ceval_frac_part(double);
double ceval_log10(double);
double ceval_ln(double);
double ceval_exp(double);
double ceval_factorial(double);
double ceval_positive_sign(double);
double ceval_negative_sign(double);
double ceval_abs(double);
double ceval_sqrt(double);
double ceval_sqrt(double);
double ceval_cbrt(double);
double ceval_ceil(double);
double ceval_floor(double);
double ceval_not(double);
double ceval_bit_not(double);

//double argument function prototypes
double ceval_sum(double, double, int);
double ceval_diff(double, double, int);
double ceval_prod(double, double, int);
double ceval_div(double, double, int);
double ceval_quotient(double, double, int);
double ceval_modulus(double, double, int);
double ceval_gcd(double, double, int);
double ceval_hcf(double, double, int);
double ceval_lcm(double, double, int);
double ceval_log(double, double, int);
double ceval_are_equal(double, double, int);
double ceval_not_equal(double, double, int);
double ceval_lesser(double, double, int);
double ceval_greater(double, double, int);
double ceval_lesser_s(double, double, int);
double ceval_greater_s(double, double, int);
double ceval_comma(double, double, int);
double ceval_power(double, double, int);
double ceval_atan2(double, double, int);
double ceval_sci2dec(double, double, int);
double ceval_and(double, double, int);
double ceval_or(double, double, int);
double ceval_bit_and(double, double, int);
double ceval_bit_xor(double, double, int);
double ceval_bit_or(double, double, int);
double ceval_bit_lshift(double, double, int);
double ceval_bit_rshift(double, double, int);

//helper function definitions
void ceval_error(const char* error_format_string, ...) {
    #ifndef CEVAL_STOICAL
        // start whining
        printf("\n[ceval]: ");
        va_list args;
        va_start(args, error_format_string);
        vprintf(error_format_string, args);
        va_end(args);
        printf("\n");
    #endif
}
double ceval_gcd_binary(int a, int b) {
    if (a == 0 && b == 0)
        return 0;
    while (b) {
        a %= b;
        b ^= a;
        a ^= b;
        b ^= a;
    }
    return a;
}
char * ceval_shrink(char * x) {
    char * y = x;
    unsigned int len = 0;
    for (unsigned int i = 0; i < strlen(x); i++) {
        if(x[i] == ' ' || x[i] == '\n' || x[i] == '\t' || x[i] == '\r') {
            continue;
        } else {
            if(x[i]=='(' && x[i+1]==')') {
                // empty pairs of parantheses are ignored
                // simlar to c lang where {} are ignored as empty blocks of code
                i++;
                continue;
            }
            *(y + len) = (char)tolower(x[i]);
            len++;
        }
    }
    y[len] = '\0';
    return y;
}
//single argument function definitions
double( * single_arg_fun[])(double) = {
    // double_arg_fun (first three tokens are whitespace and parantheses)
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL,
    // single_arg_fun
    ceval_abs, ceval_exp, ceval_sqrt, ceval_cbrt,
    ceval_ln, ceval_log10, ceval_ceil, ceval_floor,
    ceval_signum, ceval_factorial, ceval_int_part, ceval_frac_part,
    ceval_deg2rad, ceval_rad2deg, ceval_sin, ceval_cos,
    ceval_tan, ceval_asin, ceval_acos, ceval_atan,
    ceval_sinh, ceval_cosh, ceval_tanh, ceval_not,
    ceval_bit_not, ceval_positive_sign, ceval_negative_sign,
    // number and constant tokens
    NULL, NULL, NULL
};
double ceval_signum(double x) {
    return (x == 0) ? 0 :
        (x > 0) ? 1 :
        -1;
}
double ceval_asin(double x) {
    if (x > 1 || x < -1) {
        ceval_error("Numerical argument out of domain");
        return NAN;
    }
    return asin(x);
}
double ceval_acos(double x) {
    if (x > 1 || x < -1) {
        ceval_error("Numerical argument out of domain");
        return NAN;
    }
    return acos(x);
}
double ceval_atan(double x) {
    return atan(x);
}
double ceval_sin(double x) {
    double sin_val = sin(x);
    //sin(pi) == 0.000000, but sin(pi-CEVAL_EPSILON) == -0.00000* and sin(pi+CEVAL_EPSILON) == +0.00000*
    //since the precision of pi (approx) is limited, it often leads to -0.0000 printed out as a result
    //thus, we assumse 0.0000 value for all |sin(x)|<=CEVAL_EPSILON
    return (fabs(sin_val) <= CEVAL_EPSILON) ? 0 : sin_val;
}
double ceval_cos(double x) {
    double cos_val = cos(x);
    return (fabs(cos_val) <= CEVAL_EPSILON) ? 0 : cos_val;
}
double ceval_tan(double x) {
    double tan_val = tan(x);
    if (fabs(ceval_modulus(x - CEVAL_PI / 2, CEVAL_PI, 0)) <= CEVAL_DELTA) {
        ceval_error("tan() is not defined for odd-integral multiples of pi/2");
        return NAN;
    }
    return (fabs(tan_val) <= CEVAL_EPSILON) ? 0 : tan_val;
}
double ceval_rad2deg(double x) {
    return x / CEVAL_PI * 180;
}
double ceval_deg2rad(double x) {
    return x / 180 * CEVAL_PI;
}
double ceval_int_part(double x) {
    double x_i;
    modf(x, & x_i);
    return x_i;
}
double ceval_frac_part(double x) {
    double x_i, x_f;
    x_f = modf(x, & x_i);
    return x_f;
}
double ceval_log10(double x) {
    return ceval_log(10, x, 0);
}
double ceval_ln(double x) {
    return ceval_log(CEVAL_E, x, 0);
}
double ceval_exp(double x) {
    return ceval_power(CEVAL_E, x, 0);
}
double ceval_factorial(double x) {
    if (x < 0) {
        ceval_error("Numerical argument out of domain");
        return NAN;
    }
    return tgamma(x + 1);
}
double ceval_positive_sign(double x) {
    return x;
}
double ceval_negative_sign(double x) {
    return -x;
}
double ceval_abs(double x) {
    return fabs(x);
}
double ceval_sqrt(double x) {
    if (x >= 0) return sqrt(x);
    ceval_error("sqrt(): can't operate on negative numbers");
    return NAN;
}
double ceval_cbrt(double x) {
    return cbrt(x);
}
double ceval_ceil(double x) {
    return ceil(x);
}
double ceval_floor(double x) {
    return floor(x);
}
double ceval_sinh(double x) {
    return sinh(x);
}
double ceval_cosh(double x) {
    return cosh(x);
}
double ceval_tanh(double x) {
    return tanh(x);
}
double ceval_not(double x) {
    return (double) ! (int)x;
}
double ceval_bit_not(double x) {
    if(ceval_frac_part(x) == 0) {
        return ~(int)x;
    } else {
        ceval_error("bit_not(): operand must be of integral type");
        return NAN;
    }
}
//double argument function definitions
double( * double_arg_fun[])(double, double, int) = {
    // double_arg_fun (first three tokens are whitespace and parantheses)
    NULL, NULL, NULL, ceval_comma,
    ceval_or, ceval_and, ceval_bit_or, ceval_bit_xor,
    ceval_bit_and, ceval_are_equal, ceval_not_equal, ceval_lesser,
    ceval_greater, ceval_lesser_s, ceval_greater_s, ceval_bit_lshift,
    ceval_bit_rshift, ceval_sum, ceval_diff, ceval_prod,
    ceval_div, ceval_modulus, ceval_quotient, ceval_power, 
    ceval_gcd, ceval_hcf, ceval_lcm, ceval_log,
    ceval_atan2, ceval_sci2dec, ceval_power,
    // single_arg_fun
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL,
    // number and constant tokens
    NULL, NULL, NULL
};
double ceval_sum(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("sum(): function takes two arguments");
        return NAN;
    }
    return a + b;
}
double ceval_diff(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("diff(): function takes two arguments");
        return NAN;
    }
    return a - b;
}
double ceval_prod(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("prod(): function takes two arguments");
        return NAN;
    }
    return a * b;
}
double ceval_div(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("div(): function takes two arguments");
        return NAN;
    }
    if (b == 0 && a == 0) {
        ceval_error("0/0 is indeterminate...");
        ceval_error("Continuing evaluation with the assumption 0/0 = 1");
        return 1;
    } else if (b == 0) {
        ceval_error("Division by 0 is not defined...");
        ceval_error("Continuing evaluation with the assumption 1/0 = inf");
        return a * INFINITY;
    }
    return a / b;
}
double ceval_modulus(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("modulo(): function takes two arguments");
        return NAN;
    }
    if (b == 0) {
        ceval_error("Division by 0 is not defined...");
        ceval_error("Continuing evaluation with the assumption 1%0 = 0");
        return 0;
    }
    return fmod(a, b);
}
double ceval_quotient(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("quotient(): function takes two arguments");
        return NAN;
    }
    //a = b*q + r
    //q = (a - r)/b
    if (b == 0 && a == 0) {
        ceval_error("0/0 is indeterminate...");
        ceval_error("Continuing evaluation with the assumption 0/0 = 1");
        return 1;

    } else if (b == 0) {
        ceval_error("Division by 0 is not defined...");
        ceval_error("Continuing evaluation with the assumption 1/0 = inf");
        return a * INFINITY;
    }
    return (a - ceval_modulus(a, b, 0)) / b;
}
double ceval_gcd(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("gcd(): function takes two arguments");
        return NAN;
    }
    double a_f = ceval_frac_part(a),
        b_f = ceval_frac_part(b);
    int a_i = (int)ceval_int_part(a),
        b_i = (int)ceval_int_part(b);
    if (a_f == 0 && b_f == 0) {
        return (double) ceval_gcd_binary(a_i, b_i);
    } else {
        ceval_error("gcd() takes only integral parameters");
        return NAN;
    }
}
double ceval_hcf(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("hcf(): function takes two arguments");
        return NAN;
    }
    return ceval_gcd(a, b, 0);
}
double ceval_lcm(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("lcm(): function takes two arguments");
        return NAN;
    }
    return a * b / ceval_gcd(a, b, 0);
}
double ceval_log(double b, double x, int arg_check) {
    if (arg_check) {
        ceval_error("log(): function takes two arguments");
        return NAN;
    }
    if (b == 0) {
        if (x == 0) {
            ceval_error("log(0,0) is indeterminate");
            return NAN;
        } else {
            return 0;
        }
    }
    return log(x) / log(b);
}
double ceval_are_equal(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("==: function takes two arguments");
        return NAN;
    }
    if (fabs(a - b) <= CEVAL_EPSILON) {
        return 1;
    } else {
        return 0;
    }
}
double ceval_not_equal(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("!=: function takes two arguments");
        return NAN;
    }
    return (double)!(int)ceval_are_equal(a, b, 0);
}
double ceval_lesser(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("<=: function takes two arguments");
        return NAN;
    }
    return (double)!(int)ceval_greater_s(a, b, 0);
}
double ceval_greater(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error(">=: function takes two arguments");
        return NAN;
    }
    return (double)!(int)ceval_lesser_s(a, b, 0);
}
double ceval_lesser_s(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error("<: function takes two arguments");
        return NAN;
    }
    return (b - a) >= CEVAL_EPSILON;
}
double ceval_greater_s(double a, double b, int arg_check) {
    if (arg_check) {
        ceval_error(">: function takes two arguments");
        return NAN;
    }
    return (a - b) >= CEVAL_EPSILON;
}
double ceval_comma(double x, double y, int arg_check) {
    if (arg_check) {
        ceval_error(",: function takes two arguments");
        return NAN;
    }
    return y;
}
double ceval_power(double x, double y, int arg_check) {
    if (arg_check) {
        ceval_error("pow(): function takes two arguments");
        return NAN;
    }
    if(x<0 && ceval_frac_part(y)!=0) {
        ceval_error("pow(): negative numbers can only be raised to integral powers");
        return NAN;
    }
    return pow(x, y);
}
double ceval_atan2(double x, double y, int arg_check) {
    if (arg_check) {
        ceval_error("atan2(): function takes two arguments");
        return NAN;
    }
    return atan2(x, y);
}
double ceval_sci2dec(double m, double e, int arg_check) {
    if (arg_check) {
        ceval_error("sci2dec(): function takes two arguments");
        return NAN;
    }
    return (double) m * ceval_power(10, e, 0);
}
double ceval_and(double x, double y, int arg_check) {
    if (arg_check) {
        ceval_error("and(): function takes two arguments");
        return NAN;
    }
    return (double) ((int)x && (int)y);
}
double ceval_or(double x, double y, int arg_check) {
    if (arg_check) {
        ceval_error("or(): function takes two arguments");
        return NAN;
    }
    return (double) ((int)x || (int)y);
}
double ceval_bit_and(double x, double y, int arg_check) {
    if (arg_check) {
        ceval_error("bit_and(): function takes two arguments");
        return NAN;
    }
    if(ceval_frac_part(x) == 0 && ceval_frac_part(y) == 0) {
        return (int)x & (int)y;
    } else {
        ceval_error("bit_and(): operands must be of integral type");
        return NAN;
    }
}
double ceval_bit_xor(double x, double y, int arg_check) {
    if (arg_check) {
        ceval_error("bit_xor(): function takes two arguments");
        return NAN;
    }
    if(ceval_frac_part(x) == 0 && ceval_frac_part(y) == 0) {
        return (int)x ^ (int)y;
    } else {
        ceval_error("bit_xor(): operands must be of integral type");
        return NAN;
    }
}
double ceval_bit_or(double x, double y, int arg_check) {
    if (arg_check) {
        ceval_error("bit_or(): function takes two arguments");
        return NAN;
    }
    if(ceval_frac_part(x) == 0 && ceval_frac_part(y) == 0) {
        return (int)x | (int)y;
    } else {
        ceval_error("bit_or(): operands must be of integral type");
        return NAN;
    }
}
double ceval_bit_lshift(double x, double y, int arg_check) {
    if (arg_check) {
        ceval_error("bit_lshift(): function takes two arguments");
        return NAN;
    }
    if(ceval_frac_part(x) == 0 && ceval_frac_part(y) == 0) {
        return (int)x << (int)y;
    } else {
        ceval_error("bit_lshift(): operands must be of integral type");
        return NAN;
    }

}
double ceval_bit_rshift(double x, double y, int arg_check) {
    if (arg_check) {
        ceval_error("bit_rshift(): function takes two arguments");
        return NAN;
    }
    if(ceval_frac_part(x) == 0 && ceval_frac_part(y) == 0) {
        return (int)x >> (int)y;
    } else {
        ceval_error("bit_rshift(): operands must be of integral type");
        return NAN;
    }
}
/**************************************** !FUNCTIONS ********************************************/

/***************************************** PARSE_TREE_CONSTRUCTION *******************************************/
void * ceval_make_tree(char * );
ceval_node * ceval_insert_node(ceval_node * , ceval_node, int);
void ceval_print_tree(const void * );
void ceval_print_node(const ceval_node * , int);
void ceval_delete_node(ceval_node * );
void ceval_delete_tree(void * );

void ceval_delete_node(ceval_node * node) {
    if (!node) return;
    ceval_delete_node(node -> left);
    ceval_delete_node(node -> right);
    free(node);
}
void ceval_delete_tree(void * tree) {
    ceval_delete_node((ceval_node * ) tree);
}
ceval_node * ceval_insert_node(ceval_node * current, ceval_node item, int isRightAssoc) {
    if (item.id != CEVAL_OPENPAR &&
        item.id != CEVAL_NEGSIGN &&
        item.id != CEVAL_POSSIGN) {
        if (isRightAssoc) {
            while (current -> pre > item.pre) {
                current = current -> parent;
            }
        } else {
            while (current -> pre >= item.pre) {
                current = current -> parent;
            }
        }
    }
    if (item.id == CEVAL_CLOSEPAR) {
        ceval_node * parent_of_openpar = current -> parent;
        parent_of_openpar -> right = current -> right;
        if (current -> right) current -> right -> parent = parent_of_openpar;
        free(current);
        current = parent_of_openpar;

        if (current -> right -> id == CEVAL_COMMA &&
            ceval_is_binary_fun(current -> id)) {
            ceval_node * address_of_comma = current -> right;
            parent_of_openpar -> left = address_of_comma -> left;
            address_of_comma -> left -> parent = parent_of_openpar;
            parent_of_openpar -> right = address_of_comma -> right;
            address_of_comma -> right -> parent = parent_of_openpar;
            free(address_of_comma);
        }
        return current;
    }
    ceval_node * newnode = (ceval_node * ) malloc(sizeof(ceval_node));
    * newnode = item;
    newnode -> right = NULL;

    newnode -> left = current -> right;
    if (current -> right) current -> right -> parent = newnode;
    current -> right = newnode;
    newnode -> parent = current;
    current = newnode;
    return current;
}

void * ceval_make_tree(char * expression) {
    if (expression == NULL) return NULL;
    strcpy(expression, ceval_shrink(expression));
    ceval_node root = {
        CEVAL_OPENPAR,
        ceval_token_prec(CEVAL_OPENPAR),
        0,
        NULL,
        NULL,
        NULL
    };
    ceval_node_id previous_id = CEVAL_OPENPAR;
    ceval_node * current = & root;
    int isRightAssoc = 0;
    while (1) {
        ceval_node node;
        char c = * expression++;
        if (c == '\0') break;
        int token_found = -1;
        char token[50];
        unsigned int len = 0;
        for(unsigned int i = 0; i < CEVAL_TOKEN_TABLE_SIZE; i++) {
            strcpy(token, ceval_token_info[i].symbol);
            len = (unsigned int) strlen(token);
            if (!memcmp(expression - 1, token, len)) {
                token_found = ceval_token_info[i].id;
                isRightAssoc = (token_found == CEVAL_POW || token_found == CEVAL_CLOSEPAR ) ? 1 : 0;
                break;
            }
        }
        // if token is found
        if (token_found > -1) {
            // check if the token is a binary operator
            if (ceval_is_binary_opr((ceval_node_id)token_found)) {
                // a binary operator must be preceded by a number, a numerical constant, a clospar, or a factorial
                if (previous_id == CEVAL_NUMBER ||
                    previous_id == CEVAL_CONST_PI ||
                    previous_id == CEVAL_CONST_E ||
                    previous_id == CEVAL_CLOSEPAR) {
                    // other tokens (other than CEVAL_NUMBER, CEVAL_CLOSEPAR) are allowed only before '+'s or '-'s
                    expression = expression + (len - 1);
                    node.id = (ceval_node_id)token_found;
                    node.pre = ceval_token_prec(node.id);
                } else {
                    // if the operator is not preceded by a number, a numerical constant, a closepar, or a factorial, then check if the 
                    // character is a sign ('+' or '-')
                    if (c == '+') {
                        node.id = CEVAL_POSSIGN;
                        node.pre = ceval_token_prec(node.id);
                    } else if (c == '-') {
                        node.id = CEVAL_NEGSIGN;
                        node.pre = ceval_token_prec(node.id);
                    } else {
                        // if it is not a sign, then it must be a misplaced character
                        ceval_error("Misplaced '%c'.", c);
                        ceval_delete_tree(root.right);
                        root.right = NULL;
                        return NULL;
                    }
                }
            } else if (token_found == CEVAL_NUMBER){
                // if the token is a number, then store it in an array
                node.pre = ceval_token_prec(CEVAL_NUMBER);
                unsigned int i;
                char number[CEVAL_MAX_DIGITS];
                for (i = 0; i + 1 < sizeof(number);) {
                    number[i++] = c;
                    c = * expression;
                    if (('0' <= c && c <= '9') || 
                                          c == '.')
                        expression++;
                    else 
                        break;
                }
                number[i] = '\0';
                //copy the contents of the number array at &node.number
                sscanf(number, "%lf", & node.number);
                node.id = CEVAL_NUMBER;
                goto END;
            } else if (token_found == CEVAL_WHITESPACE) {
                // skip whitespace
                continue;
            } else {
                // for any other token
                expression = expression + (len - 1);
                node.id = (ceval_node_id)token_found;
                node.pre = ceval_token_prec(node.id);
                if (node.id == CEVAL_CONST_PI || node.id == CEVAL_CONST_E) {
                    node.number = (node.id == CEVAL_CONST_PI) ? CEVAL_PI : CEVAL_E;
                }
            }
        } else {
            // if the token is not found in the token table
            ceval_error("Unknown token '%c'.", c);
            ceval_delete_tree(root.right);
            root.right = NULL;
            return NULL;
        }
        END: ;
        previous_id = node.id;
        current = ceval_insert_node(current, node, isRightAssoc);
    }
    if (root.right) root.right -> parent = NULL;
    return root.right;
}
void ceval_print_node(const ceval_node * node, int indent) {
    int i;
    char number[CEVAL_MAX_DIGITS];
    const char * str;
    if (!node) return;
    ceval_print_node(node -> right, indent + 4);
    if (node -> id == CEVAL_NUMBER) {
        if ((long) node -> number == node -> number) //for integers, skip the trailing zeroes
            snprintf(number, sizeof(number), "%.0f", node -> number);
        else snprintf(number, sizeof(number), "%.2f", node -> number);
        str = number;
    } else {
        str = ceval_token_symbol(node -> id);
    }
    for (i = 0; i < indent; i++) {
        putchar(' ');
        putchar(' ');
    }
    printf("%s\n", str);
    ceval_print_node(node -> left, indent + 4);
}
void ceval_print_tree(const void * tree) {
    ceval_print_node((const ceval_node * ) tree, 0);
}
/***************************************** !PARSE_TREE_CONSTRUCTION *******************************************/

/***************************************** EVALUATION *******************************************/
double ceval_evaluate_tree_(const ceval_node * );
double ceval_evaluate_tree(const void * );

double ceval_evaluate_tree_(const ceval_node * node) {
    if (!node) 
        return 0;
    
    double left, right;
    left = ceval_evaluate_tree_(node -> left);
    right = ceval_evaluate_tree_(node -> right);
    switch (node -> id) {

        //unary-right operators/functions (operate on the expression to their right)
        case CEVAL_ABS: case CEVAL_EXP: case CEVAL_SQRT: case CEVAL_CBRT: 
        case CEVAL_LN: case CEVAL_LOG10: case CEVAL_CEIL: case CEVAL_FLOOR: 
        case CEVAL_SIGNUM: case CEVAL_FACTORIAL: case CEVAL_INT: case CEVAL_FRAC: 
        case CEVAL_DEG2RAD: case CEVAL_RAD2DEG: case CEVAL_SIN: case CEVAL_COS: 
        case CEVAL_TAN: case CEVAL_ASIN: case CEVAL_ACOS: case CEVAL_ATAN: 
        case CEVAL_SINH: case CEVAL_COSH: case CEVAL_TANH: case CEVAL_NOT: 
        case CEVAL_BIT_NOT: case CEVAL_POSSIGN: case CEVAL_NEGSIGN: 
            if (node -> right != NULL) {
                //operate on right operand
                return ( * single_arg_fun[node -> id])(right);
            } else {
                ceval_error("Missing operand(s)");
                return NAN;
            }
        //binary operators/functions
        case CEVAL_COMMA:  
        case CEVAL_OR:  case CEVAL_AND:  case CEVAL_BIT_OR:  case CEVAL_BIT_XOR: 
        case CEVAL_BIT_AND:  case CEVAL_EQUAL:  case CEVAL_NOTEQUAL: case CEVAL_LESSER: 
        case CEVAL_GREATER:  case CEVAL_LESSER_S:  case CEVAL_GREATER_S:  case CEVAL_BIT_LSHIFT:  
        case CEVAL_BIT_RSHIFT:  case CEVAL_PLUS:  case CEVAL_MINUS:  case CEVAL_TIMES:  
        case CEVAL_DIVIDE:  case CEVAL_MODULO:  case CEVAL_QUOTIENT:  case CEVAL_POW: 
        case CEVAL_GCD:  case CEVAL_HCF:  case CEVAL_LCM:  case CEVAL_LOG: 
        case CEVAL_ATAN2:  case CEVAL_SCI2DEC: case CEVAL_POWFUN:
            if (node -> left == NULL) {
                return ( * double_arg_fun[node -> id])(left, right, -1);
            } else if (node -> right == NULL) {
                return ( * double_arg_fun[node -> id])(left, right, 1);
            } else {
                return ( * double_arg_fun[node -> id])(left, right, 0);
            }
            default:
                return node -> number;
    }
}
double ceval_evaluate_tree(const void * node) {
    return (node == NULL)? NAN :
            ceval_evaluate_tree_((ceval_node * ) node);
}
/***************************************** !EVALUATION *******************************************/

/***************************************** MAIN FUNCTIONS *******************************************/
// functions accessible from main() 
// - double ceval_result(char * inp) returns the result of valid math expression stored as a char array `inp`
// - void ceval_tree(char * inp) prints the parse tree for the input expression `inp`
// - define CEVAL_EPSILON (default value : 1e-2), CEVAL_DELTA (default value : 1e-6) and CEVAL_MAX_DIGITS (default value : 15) manually before the include directive
// - define CEVAL_STOICAL before the #include directive to use the parser/evaluator in stoical (non-complaining) mode. It suppresses all the error messages from [ceval]. 

double ceval_result(char * expr) {
    void * tree = ceval_make_tree(expr);
    double result = ceval_evaluate_tree(tree);
    ceval_delete_tree(tree);
    return result;
}
void ceval_tree(char * expr) {
    void * tree = ceval_make_tree(expr);
    ceval_print_tree(tree);
    ceval_delete_tree(tree);
}

#ifdef CEVAL_CXX
    double ceval_result(std::string expr) {
        return ceval_result((char * ) expr.c_str());
    }
    void ceval_tree(std::string expr) {
        ceval_tree((char * ) expr.c_str());
    }
#endif
/***************************************** !MAIN FUNCTIONS *******************************************/
#endif
