// RUN: %dxc -Tlib_6_3 -verify %s

// The following is meant to be processed by the CodeTags extension in the "VS For Everything" Visual Studio extension:
/*<py>
import re
rxComments = re.compile(r'(//.*|/\*.*?\*\/)')
def strip_comments(line):
    line = rxComments.sub('', line)
    return line.strip()
def save_error_comments(lines):
    saved = {}
    for line in lines:
        key = strip_comments(line)
        if key and line.strip() != key:
            saved[key] = line
    return saved
def restore_error_comments(saved, lines):
    return [saved.get(line.strip(), line) for line in lines]
def modify(lines, newlines):
    return restore_error_comments(save_error_comments(lines), newlines)
def gen_combos(items, exclude=[]):
    exclude = map(set, exclude)
    for n, item in enumerate(items):
        s_item = set((item,))
        if s_item in exclude:
            continue
        yield (item,)
        for combo in gen_combos(items[n+1:], exclude=exclude):
            for excombo in exclude:
                if not set(excombo).difference(s_item.union(combo)):
                    break
            else:
                yield (item,) + combo
def gen_pairs(items):
    for n, item in enumerate(items):
        for item2 in items[n+1:]:
            yield (item, item2)
def make_trunc(num):
    def trunc(s):
        return s[:num]
    return trunc
def is_valid(combo):
    if 'snorm' in combo and 'unorm' in combo:
        return False
    if 'row_major' in combo and 'column_major' in combo:
        return False
    return True
norm_mods = ['snorm', 'unorm']
matrix_mods = ['row_major', 'column_major']
norm_combos = list(gen_combos(norm_mods))
matrix_combos = list(gen_combos(matrix_mods))
typemod_combos = list(gen_combos(matrix_mods + norm_mods))
def filter_combos(combos, val=False):
    newcombos = [c for c in combos if is_valid(c)]
    if not val:
        newcombos += [c for c in combos if not is_valid(c)]
    return newcombos
def gen_code(template, combos=typemod_combos, trunc=3, val=False):
    #return []  # uncomment to clear generated code
    trunc = make_trunc(trunc)
    combos = filter_combos(combos, val)
    return [
        template % {
            'mods': ' '.join(combo), 
            'id': '_'.join(map(trunc, combo))} 
        for combo in combos]
def gen_code2(template, combos1=typemod_combos, combos2=typemod_combos, trunc=3, val=False, val2=False):
    #return []  # uncomment to clear generated code
    trunc = make_trunc(trunc)
    combos1 = filter_combos(combos1, val)
    combos2 = filter_combos(combos2, val)
    combos = [(is_valid(combo1+combo2), (combo1, combo2)) 
              for combo1 in combos1
              for combo2 in combos2]
    invalid_combos = [c for v, c in combos if not v]
    combos = [c for v, c in combos if v]
    if not val2:
        combos += invalid_combos
    return [
        template % {
            'mods1': ' '.join(combo1), 
            'id1': '_'.join(map(trunc, combo1)),
            'mods2': ' '.join(combo2), 
            'id2': '_'.join(map(trunc, combo2))} 
        for combo1, combo2 in combos]
</py>*/

//////////////////////////////////////////////////////////////////////////////
// Global variables.

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2x3 g_%(id)s;'))</py>
// GENERATED_CODE:BEGIN
row_major float2x3 g_row;
row_major snorm float2x3 g_row_sno;
row_major unorm float2x3 g_row_uno;
column_major float2x3 g_col;
column_major snorm float2x3 g_col_sno;
column_major unorm float2x3 g_col_uno;
snorm float2x3 g_sno;
unorm float2x3 g_uno;
row_major column_major float2x3 g_row_col;                  /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
row_major column_major snorm float2x3 g_row_col_sno;        /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
row_major column_major snorm unorm float2x3 g_row_col_sno_uno;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
row_major column_major unorm float2x3 g_row_col_uno;        /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
row_major snorm unorm float2x3 g_row_sno_uno;               /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
column_major snorm unorm float2x3 g_col_sno_uno;            /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
snorm unorm float2x3 g_sno_uno;                             /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

// Reverse positions for norm and matrix orientation (fxc doesn't like this!)
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2x3 g_rev_%(id)s;', combos=list(gen_combos(norm_mods+matrix_mods)), val=True))</py>
// GENERATED_CODE:BEGIN
snorm float2x3 g_rev_sno;
snorm row_major float2x3 g_rev_sno_row;                     /* fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
snorm column_major float2x3 g_rev_sno_col;                  /* fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
unorm float2x3 g_rev_uno;
unorm row_major float2x3 g_rev_uno_row;                     /* fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
unorm column_major float2x3 g_rev_uno_col;                  /* fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
row_major float2x3 g_rev_row;
column_major float2x3 g_rev_col;
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2 g_vector_%(id)s;'))</py>
// GENERATED_CODE:BEGIN
row_major float2 g_vector_row;                              /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
row_major snorm float2 g_vector_row_sno;                    /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
row_major unorm float2 g_vector_row_uno;                    /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
column_major float2 g_vector_col;                           /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
column_major snorm float2 g_vector_col_sno;                 /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
column_major unorm float2 g_vector_col_uno;                 /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
snorm float2 g_vector_sno;
unorm float2 g_vector_uno;
row_major column_major float2 g_vector_row_col;             /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
row_major column_major snorm float2 g_vector_row_col_sno;   /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
row_major column_major snorm unorm float2 g_vector_row_col_sno_uno;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
row_major column_major unorm float2 g_vector_row_col_uno;   /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
row_major snorm unorm float2 g_vector_row_sno_uno;          /* expected-error {{'row_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
column_major snorm unorm float2 g_vector_col_sno_uno;       /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
snorm unorm float2 g_vector_sno_uno;                        /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float g_scalar_%(id)s;'))</py>
// GENERATED_CODE:BEGIN
row_major float g_scalar_row;                               /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
row_major snorm float g_scalar_row_sno;                     /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
row_major unorm float g_scalar_row_uno;                     /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
column_major float g_scalar_col;                            /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
column_major snorm float g_scalar_col_sno;                  /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
column_major unorm float g_scalar_col_uno;                  /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
snorm float g_scalar_sno;
unorm float g_scalar_uno;
row_major column_major float g_scalar_row_col;              /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
row_major column_major snorm float g_scalar_row_col_sno;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
row_major column_major snorm unorm float g_scalar_row_col_sno_uno;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
row_major column_major unorm float g_scalar_row_col_uno;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
row_major snorm unorm float g_scalar_row_sno_uno;           /* expected-error {{'row_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
column_major snorm unorm float g_scalar_col_sno_uno;        /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
snorm unorm float g_scalar_sno_uno;                         /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

// Matrix Template versions
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('matrix<%(mods)s float, 2, 3> g_mat_%(id)s;'))</py>
// GENERATED_CODE:BEGIN
matrix<row_major float, 2, 3> g_mat_row;                    /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
matrix<row_major snorm float, 2, 3> g_mat_row_sno;          /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
matrix<row_major unorm float, 2, 3> g_mat_row_uno;          /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
matrix<column_major float, 2, 3> g_mat_col;                 /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
matrix<column_major snorm float, 2, 3> g_mat_col_sno;       /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
matrix<column_major unorm float, 2, 3> g_mat_col_uno;       /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
matrix<snorm float, 2, 3> g_mat_sno;
matrix<unorm float, 2, 3> g_mat_uno;
matrix<row_major column_major float, 2, 3> g_mat_row_col;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
matrix<row_major column_major snorm float, 2, 3> g_mat_row_col_sno;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
matrix<row_major column_major snorm unorm float, 2, 3> g_mat_row_col_sno_uno;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
matrix<row_major column_major unorm float, 2, 3> g_mat_row_col_uno;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
matrix<row_major snorm unorm float, 2, 3> g_mat_row_sno_uno; /* expected-error {{'row_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
matrix<column_major snorm unorm float, 2, 3> g_mat_col_sno_uno;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
matrix<snorm unorm float, 2, 3> g_mat_sno_uno;              /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code2('%(mods1)s matrix<%(mods2)s float, 2, 3> g_%(id1)s_mat_%(id2)s;', combos2=norm_combos, val=True))</py>
// GENERATED_CODE:BEGIN
row_major matrix<snorm float, 2, 3> g_row_mat_sno;
row_major matrix<unorm float, 2, 3> g_row_mat_uno;
row_major snorm matrix<snorm float, 2, 3> g_row_sno_mat_sno;    /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
row_major unorm matrix<unorm float, 2, 3> g_row_uno_mat_uno;    /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
column_major matrix<snorm float, 2, 3> g_col_mat_sno;
column_major matrix<unorm float, 2, 3> g_col_mat_uno;
column_major snorm matrix<snorm float, 2, 3> g_col_sno_mat_sno;    /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
column_major unorm matrix<unorm float, 2, 3> g_col_uno_mat_uno;    /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
snorm matrix<snorm float, 2, 3> g_sno_mat_sno;              /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
unorm matrix<unorm float, 2, 3> g_uno_mat_uno;              /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
row_major snorm matrix<unorm float, 2, 3> g_row_sno_mat_uno;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
row_major unorm matrix<snorm float, 2, 3> g_row_uno_mat_sno;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
column_major snorm matrix<unorm float, 2, 3> g_col_sno_mat_uno;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
column_major unorm matrix<snorm float, 2, 3> g_col_uno_mat_sno;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
snorm matrix<unorm float, 2, 3> g_sno_mat_uno;              /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
unorm matrix<snorm float, 2, 3> g_uno_mat_sno;              /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
// GENERATED_CODE:END

// Initialized static const:
// static const initialized to scalar:
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('static const %(mods)s float2x3 sc_%(id)s_init_scalar = 1.0f;'))</py>
// GENERATED_CODE:BEGIN
static const row_major float2x3 sc_row_init_scalar = 1.0f;
static const row_major snorm float2x3 sc_row_sno_init_scalar = 1.0f;
static const row_major unorm float2x3 sc_row_uno_init_scalar = 1.0f;
static const column_major float2x3 sc_col_init_scalar = 1.0f;
static const column_major snorm float2x3 sc_col_sno_init_scalar = 1.0f;
static const column_major unorm float2x3 sc_col_uno_init_scalar = 1.0f;
static const snorm float2x3 sc_sno_init_scalar = 1.0f;
static const unorm float2x3 sc_uno_init_scalar = 1.0f;
static const row_major column_major float2x3 sc_row_col_init_scalar = 1.0f;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
static const row_major column_major snorm float2x3 sc_row_col_sno_init_scalar = 1.0f;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
static const row_major column_major snorm unorm float2x3 sc_row_col_sno_uno_init_scalar = 1.0f;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
static const row_major column_major unorm float2x3 sc_row_col_uno_init_scalar = 1.0f;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
static const row_major snorm unorm float2x3 sc_row_sno_uno_init_scalar = 1.0f; /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
static const column_major snorm unorm float2x3 sc_col_sno_uno_init_scalar = 1.0f;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
static const snorm unorm float2x3 sc_sno_uno_init_scalar = 1.0f;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

// matrix template versions:
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('static const matrix<%(mods)s float, 2, 3> sc_mat_%(id)s_init_scalar = 1.0f;'))</py>
// GENERATED_CODE:BEGIN
static const matrix<row_major float, 2, 3> sc_mat_row_init_scalar = 1.0f;      /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
static const matrix<row_major snorm float, 2, 3> sc_mat_row_sno_init_scalar = 1.0f;    /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
static const matrix<row_major unorm float, 2, 3> sc_mat_row_uno_init_scalar = 1.0f;    /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
static const matrix<column_major float, 2, 3> sc_mat_col_init_scalar = 1.0f;   /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
static const matrix<column_major snorm float, 2, 3> sc_mat_col_sno_init_scalar = 1.0f;    /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
static const matrix<column_major unorm float, 2, 3> sc_mat_col_uno_init_scalar = 1.0f;    /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
static const matrix<snorm float, 2, 3> sc_mat_sno_init_scalar = 1.0f;
static const matrix<unorm float, 2, 3> sc_mat_uno_init_scalar = 1.0f;
static const matrix<row_major column_major float, 2, 3> sc_mat_row_col_init_scalar = 1.0f;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
static const matrix<row_major column_major snorm float, 2, 3> sc_mat_row_col_sno_init_scalar = 1.0f;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
static const matrix<row_major column_major snorm unorm float, 2, 3> sc_mat_row_col_sno_uno_init_scalar = 1.0f;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
static const matrix<row_major column_major unorm float, 2, 3> sc_mat_row_col_uno_init_scalar = 1.0f;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
static const matrix<row_major snorm unorm float, 2, 3> sc_mat_row_sno_uno_init_scalar = 1.0f; /* expected-error {{'row_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
static const matrix<column_major snorm unorm float, 2, 3> sc_mat_col_sno_uno_init_scalar = 1.0f;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
static const matrix<snorm unorm float, 2, 3> sc_mat_sno_uno_init_scalar = 1.0f;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

// static initialized to static const versions:
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('static %(mods)s float2x3 s_%(id)s_init = sc_%(id)s_init_scalar;'))</py>
// GENERATED_CODE:BEGIN
static row_major float2x3 s_row_init = sc_row_init_scalar;
static row_major snorm float2x3 s_row_sno_init = sc_row_sno_init_scalar;
static row_major unorm float2x3 s_row_uno_init = sc_row_uno_init_scalar;
static column_major float2x3 s_col_init = sc_col_init_scalar;
static column_major snorm float2x3 s_col_sno_init = sc_col_sno_init_scalar;
static column_major unorm float2x3 s_col_uno_init = sc_col_uno_init_scalar;
static snorm float2x3 s_sno_init = sc_sno_init_scalar;
static unorm float2x3 s_uno_init = sc_uno_init_scalar;
static row_major column_major float2x3 s_row_col_init = sc_row_col_init_scalar;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
static row_major column_major snorm float2x3 s_row_col_sno_init = sc_row_col_sno_init_scalar;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
static row_major column_major snorm unorm float2x3 s_row_col_sno_uno_init = sc_row_col_sno_uno_init_scalar;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
static row_major column_major unorm float2x3 s_row_col_uno_init = sc_row_col_uno_init_scalar;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
static row_major snorm unorm float2x3 s_row_sno_uno_init = sc_row_sno_uno_init_scalar;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
static column_major snorm unorm float2x3 s_col_sno_uno_init = sc_col_sno_uno_init_scalar;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
static snorm unorm float2x3 s_sno_uno_init = sc_sno_uno_init_scalar;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

cbuffer CBInit
{
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2x3 g_%(id)s_init = sc_%(id)s_init_scalar;'))</py>
    // GENERATED_CODE:BEGIN
    row_major float2x3 g_row_init = sc_row_init_scalar;
    row_major snorm float2x3 g_row_sno_init = sc_row_sno_init_scalar;
    row_major unorm float2x3 g_row_uno_init = sc_row_uno_init_scalar;
    column_major float2x3 g_col_init = sc_col_init_scalar;
    column_major snorm float2x3 g_col_sno_init = sc_col_sno_init_scalar;
    column_major unorm float2x3 g_col_uno_init = sc_col_uno_init_scalar;
    snorm float2x3 g_sno_init = sc_sno_init_scalar;
    unorm float2x3 g_uno_init = sc_uno_init_scalar;
    row_major column_major float2x3 g_row_col_init = sc_row_col_init_scalar;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major column_major snorm float2x3 g_row_col_sno_init = sc_row_col_sno_init_scalar;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major column_major snorm unorm float2x3 g_row_col_sno_uno_init = sc_row_col_sno_uno_init_scalar;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    row_major column_major unorm float2x3 g_row_col_uno_init = sc_row_col_uno_init_scalar;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major snorm unorm float2x3 g_row_sno_uno_init = sc_row_sno_uno_init_scalar;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    column_major snorm unorm float2x3 g_col_sno_uno_init = sc_col_sno_uno_init_scalar;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    snorm unorm float2x3 g_sno_uno_init = sc_sno_uno_init_scalar;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    // GENERATED_CODE:END

    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2x3 g_%(id)s_init_scalar = 1.0f;'))</py>
    // GENERATED_CODE:BEGIN
    row_major float2x3 g_row_init_scalar = 1.0f;
    row_major snorm float2x3 g_row_sno_init_scalar = 1.0f;
    row_major unorm float2x3 g_row_uno_init_scalar = 1.0f;
    column_major float2x3 g_col_init_scalar = 1.0f;
    column_major snorm float2x3 g_col_sno_init_scalar = 1.0f;
    column_major unorm float2x3 g_col_uno_init_scalar = 1.0f;
    snorm float2x3 g_sno_init_scalar = 1.0f;
    unorm float2x3 g_uno_init_scalar = 1.0f;
    row_major column_major float2x3 g_row_col_init_scalar = 1.0f;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major column_major snorm float2x3 g_row_col_sno_init_scalar = 1.0f;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major column_major snorm unorm float2x3 g_row_col_sno_uno_init_scalar = 1.0f;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    row_major column_major unorm float2x3 g_row_col_uno_init_scalar = 1.0f;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major snorm unorm float2x3 g_row_sno_uno_init_scalar = 1.0f; /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    column_major snorm unorm float2x3 g_col_sno_uno_init_scalar = 1.0f;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    snorm unorm float2x3 g_sno_uno_init_scalar = 1.0f;      /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    // GENERATED_CODE:END
}

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('float2x3 %(mods)s g_%(id)s_after;'))</py>
// GENERATED_CODE:BEGIN
float2x3 row_major g_row_after;                             /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
float2x3 row_major snorm g_row_sno_after;                   /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
float2x3 row_major unorm g_row_uno_after;                   /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
float2x3 column_major g_col_after;                          /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
float2x3 column_major snorm g_col_sno_after;                /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
float2x3 column_major unorm g_col_uno_after;                /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
float2x3 snorm g_sno_after;                                 /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'snorm'}} */
float2x3 unorm g_uno_after;                                 /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
float2x3 row_major column_major g_row_col_after;            /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
float2x3 row_major column_major snorm g_row_col_sno_after;  /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
float2x3 row_major column_major snorm unorm g_row_col_sno_uno_after;    /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
float2x3 row_major column_major unorm g_row_col_uno_after;  /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
float2x3 row_major snorm unorm g_row_sno_uno_after;         /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
float2x3 column_major snorm unorm g_col_sno_uno_after;      /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
float2x3 snorm unorm g_sno_uno_after;                       /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'snorm'}} */
// GENERATED_CODE:END


//////////////////////////////////////////////////////////////////////////////
// Typedefs.
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('typedef %(mods)s float2x3 t_%(id)s;'))</py>
// GENERATED_CODE:BEGIN
typedef row_major float2x3 t_row;
typedef row_major snorm float2x3 t_row_sno;
typedef row_major unorm float2x3 t_row_uno;
typedef column_major float2x3 t_col;
typedef column_major snorm float2x3 t_col_sno;
typedef column_major unorm float2x3 t_col_uno;
typedef snorm float2x3 t_sno;
typedef unorm float2x3 t_uno;
typedef row_major column_major float2x3 t_row_col;          /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
typedef row_major column_major snorm float2x3 t_row_col_sno;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
typedef row_major column_major snorm unorm float2x3 t_row_col_sno_uno;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
typedef row_major column_major unorm float2x3 t_row_col_uno;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
typedef row_major snorm unorm float2x3 t_row_sno_uno;       /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
typedef column_major snorm unorm float2x3 t_col_sno_uno;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
typedef snorm unorm float2x3 t_sno_uno;                     /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('typedef %(mods)s matrix<float, 2, 3> t_%(id)s_mat;'))</py>
// GENERATED_CODE:BEGIN
typedef row_major matrix<float, 2, 3> t_row_mat;
typedef row_major snorm matrix<float, 2, 3> t_row_sno_mat;  /* fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef row_major unorm matrix<float, 2, 3> t_row_uno_mat;  /* fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef column_major matrix<float, 2, 3> t_col_mat;
typedef column_major snorm matrix<float, 2, 3> t_col_sno_mat;    /* fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef column_major unorm matrix<float, 2, 3> t_col_uno_mat;    /* fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef snorm matrix<float, 2, 3> t_sno_mat;                /* fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef unorm matrix<float, 2, 3> t_uno_mat;                /* fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef row_major column_major matrix<float, 2, 3> t_row_col_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
typedef row_major column_major snorm matrix<float, 2, 3> t_row_col_sno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef row_major column_major snorm unorm matrix<float, 2, 3> t_row_col_sno_uno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
typedef row_major column_major unorm matrix<float, 2, 3> t_row_col_uno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef row_major snorm unorm matrix<float, 2, 3> t_row_sno_uno_mat; /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
typedef column_major snorm unorm matrix<float, 2, 3> t_col_sno_uno_mat;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
typedef snorm unorm matrix<float, 2, 3> t_sno_uno_mat;      /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code2('typedef %(mods1)s t_%(id2)s_mat t_%(id1)s_t_%(id2)s_mat;', val=True))</py>
// GENERATED_CODE:BEGIN
typedef row_major t_row_mat t_row_t_row_mat;                /* expected-warning {{attribute 'row_major' is already applied}} fxc-pass {{}} */
typedef row_major t_row_sno_mat t_row_t_row_sno_mat;        /* expected-warning {{attribute 'row_major' is already applied}} fxc-error {{X3000: unrecognized identifier 't_row_sno_mat'}} */
typedef row_major t_row_uno_mat t_row_t_row_uno_mat;        /* expected-warning {{attribute 'row_major' is already applied}} fxc-error {{X3000: unrecognized identifier 't_row_uno_mat'}} */
typedef row_major t_sno_mat t_row_t_sno_mat;                /* fxc-error {{X3000: unrecognized identifier 't_sno_mat'}} */
typedef row_major t_uno_mat t_row_t_uno_mat;                /* fxc-error {{X3000: unrecognized identifier 't_uno_mat'}} */
typedef row_major snorm t_row_mat t_row_sno_t_row_mat;      /* expected-warning {{attribute 'row_major' is already applied}} fxc-error {{X3085: snorm can not be used with type}} */
typedef row_major snorm t_row_sno_mat t_row_sno_t_row_sno_mat;    /* expected-warning {{attribute 'row_major' is already applied}} expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_row_sno_mat'}} */
typedef row_major snorm t_sno_mat t_row_sno_t_sno_mat;      /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_sno_mat'}} */
typedef row_major unorm t_row_mat t_row_uno_t_row_mat;      /* expected-warning {{attribute 'row_major' is already applied}} fxc-error {{X3085: unorm can not be used with type}} */
typedef row_major unorm t_row_uno_mat t_row_uno_t_row_uno_mat;    /* expected-warning {{attribute 'row_major' is already applied}} expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_row_uno_mat'}} */
typedef row_major unorm t_uno_mat t_row_uno_t_uno_mat;      /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_uno_mat'}} */
typedef column_major t_col_mat t_col_t_col_mat;             /* expected-warning {{attribute 'column_major' is already applied}} fxc-pass {{}} */
typedef column_major t_col_sno_mat t_col_t_col_sno_mat;     /* expected-warning {{attribute 'column_major' is already applied}} fxc-error {{X3000: unrecognized identifier 't_col_sno_mat'}} */
typedef column_major t_col_uno_mat t_col_t_col_uno_mat;     /* expected-warning {{attribute 'column_major' is already applied}} fxc-error {{X3000: unrecognized identifier 't_col_uno_mat'}} */
typedef column_major t_sno_mat t_col_t_sno_mat;             /* fxc-error {{X3000: unrecognized identifier 't_sno_mat'}} */
typedef column_major t_uno_mat t_col_t_uno_mat;             /* fxc-error {{X3000: unrecognized identifier 't_uno_mat'}} */
typedef column_major snorm t_col_mat t_col_sno_t_col_mat;   /* expected-warning {{attribute 'column_major' is already applied}} fxc-error {{X3085: snorm can not be used with type}} */
typedef column_major snorm t_col_sno_mat t_col_sno_t_col_sno_mat;    /* expected-warning {{attribute 'column_major' is already applied}} expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_col_sno_mat'}} */
typedef column_major snorm t_sno_mat t_col_sno_t_sno_mat;   /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_sno_mat'}} */
typedef column_major unorm t_col_mat t_col_uno_t_col_mat;   /* expected-warning {{attribute 'column_major' is already applied}} fxc-error {{X3085: unorm can not be used with type}} */
typedef column_major unorm t_col_uno_mat t_col_uno_t_col_uno_mat;    /* expected-warning {{attribute 'column_major' is already applied}} expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_col_uno_mat'}} */
typedef column_major unorm t_uno_mat t_col_uno_t_uno_mat;   /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_uno_mat'}} */
typedef snorm t_row_mat t_sno_t_row_mat;                    /* fxc-error {{X3085: snorm can not be used with type}} */
typedef snorm t_row_sno_mat t_sno_t_row_sno_mat;            /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_row_sno_mat'}} */
typedef snorm t_col_mat t_sno_t_col_mat;                    /* fxc-error {{X3085: snorm can not be used with type}} */
typedef snorm t_col_sno_mat t_sno_t_col_sno_mat;            /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_col_sno_mat'}} */
typedef snorm t_sno_mat t_sno_t_sno_mat;                    /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_sno_mat'}} */
typedef unorm t_row_mat t_uno_t_row_mat;                    /* fxc-error {{X3085: unorm can not be used with type}} */
typedef unorm t_row_uno_mat t_uno_t_row_uno_mat;            /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_row_uno_mat'}} */
typedef unorm t_col_mat t_uno_t_col_mat;                    /* fxc-error {{X3085: unorm can not be used with type}} */
typedef unorm t_col_uno_mat t_uno_t_col_uno_mat;            /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_col_uno_mat'}} */
typedef unorm t_uno_mat t_uno_t_uno_mat;                    /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_uno_mat'}} */
typedef row_major t_col_mat t_row_t_col_mat;                /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
typedef row_major t_col_sno_mat t_row_t_col_sno_mat;        /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3000: unrecognized identifier 't_col_sno_mat'}} */
typedef row_major t_col_uno_mat t_row_t_col_uno_mat;        /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3000: unrecognized identifier 't_col_uno_mat'}} */
typedef row_major snorm t_row_uno_mat t_row_sno_t_row_uno_mat;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} expected-warning {{attribute 'row_major' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_row_uno_mat'}} */
typedef row_major snorm t_col_mat t_row_sno_t_col_mat;      /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} fxc-error {{X3085: snorm can not be used with type}} */
typedef row_major snorm t_col_sno_mat t_row_sno_t_col_sno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_col_sno_mat'}} */
typedef row_major snorm t_col_uno_mat t_row_sno_t_col_uno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_col_uno_mat'}} */
typedef row_major snorm t_uno_mat t_row_sno_t_uno_mat;      /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_uno_mat'}} */
typedef row_major unorm t_row_sno_mat t_row_uno_t_row_sno_mat;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} expected-warning {{attribute 'row_major' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_row_sno_mat'}} */
typedef row_major unorm t_col_mat t_row_uno_t_col_mat;      /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} fxc-error {{X3085: unorm can not be used with type}} */
typedef row_major unorm t_col_sno_mat t_row_uno_t_col_sno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_col_sno_mat'}} */
typedef row_major unorm t_col_uno_mat t_row_uno_t_col_uno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_col_uno_mat'}} */
typedef row_major unorm t_sno_mat t_row_uno_t_sno_mat;      /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_sno_mat'}} */
typedef column_major t_row_mat t_col_t_row_mat;             /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
typedef column_major t_row_sno_mat t_col_t_row_sno_mat;     /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3000: unrecognized identifier 't_row_sno_mat'}} */
typedef column_major t_row_uno_mat t_col_t_row_uno_mat;     /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3000: unrecognized identifier 't_row_uno_mat'}} */
typedef column_major snorm t_row_mat t_col_sno_t_row_mat;   /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} fxc-error {{X3085: snorm can not be used with type}} */
typedef column_major snorm t_row_sno_mat t_col_sno_t_row_sno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_row_sno_mat'}} */
typedef column_major snorm t_row_uno_mat t_col_sno_t_row_uno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_row_uno_mat'}} */
typedef column_major snorm t_col_uno_mat t_col_sno_t_col_uno_mat;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} expected-warning {{attribute 'column_major' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_col_uno_mat'}} */
typedef column_major snorm t_uno_mat t_col_sno_t_uno_mat;   /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_uno_mat'}} */
typedef column_major unorm t_row_mat t_col_uno_t_row_mat;   /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} fxc-error {{X3085: unorm can not be used with type}} */
typedef column_major unorm t_row_sno_mat t_col_uno_t_row_sno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_row_sno_mat'}} */
typedef column_major unorm t_row_uno_mat t_col_uno_t_row_uno_mat;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_row_uno_mat'}} */
typedef column_major unorm t_col_sno_mat t_col_uno_t_col_sno_mat;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} expected-warning {{attribute 'column_major' is already applied}} fxc-error {{X3000: syntax error: unexpected token 't_col_sno_mat'}} */
typedef column_major unorm t_sno_mat t_col_uno_t_sno_mat;   /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_sno_mat'}} */
typedef snorm t_row_uno_mat t_sno_t_row_uno_mat;            /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_row_uno_mat'}} */
typedef snorm t_col_uno_mat t_sno_t_col_uno_mat;            /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_col_uno_mat'}} */
typedef snorm t_uno_mat t_sno_t_uno_mat;                    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_uno_mat'}} */
typedef unorm t_row_sno_mat t_uno_t_row_sno_mat;            /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_row_sno_mat'}} */
typedef unorm t_col_sno_mat t_uno_t_col_sno_mat;            /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_col_sno_mat'}} */
typedef unorm t_sno_mat t_uno_t_sno_mat;                    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 't_sno_mat'}} */
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('typedef %(mods)s float t_%(id)s_f;'))</py>
// GENERATED_CODE:BEGIN
typedef row_major float t_row_f;                            /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
typedef row_major snorm float t_row_sno_f;                  /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
typedef row_major unorm float t_row_uno_f;                  /* expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
typedef column_major float t_col_f;                         /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
typedef column_major snorm float t_col_sno_f;               /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
typedef column_major unorm float t_col_uno_f;               /* expected-error {{'column_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
typedef snorm float t_sno_f;
typedef unorm float t_uno_f;
typedef row_major column_major float t_row_col_f;           /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
typedef row_major column_major snorm float t_row_col_sno_f; /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
typedef row_major column_major snorm unorm float t_row_col_sno_uno_f;    /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
typedef row_major column_major unorm float t_row_col_uno_f; /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'row_major' can only be used with a matrix type}} fxc-error {{X3077: non-matrix types cannot be declared 'row_major' or 'column_major'}} */
typedef row_major snorm unorm float t_row_sno_uno_f;        /* expected-error {{'row_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
typedef column_major snorm unorm float t_col_sno_uno_f;     /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
typedef snorm unorm float t_sno_uno_f;                      /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code2('typedef %(mods1)s matrix<t_%(id2)s_f, 2, 3> t_%(id1)s_mat_%(id2)s;', combos2=norm_combos, val=True))</py>
// GENERATED_CODE:BEGIN
typedef row_major matrix<t_sno_f, 2, 3> t_row_mat_sno;
typedef row_major matrix<t_uno_f, 2, 3> t_row_mat_uno;
typedef row_major snorm matrix<t_sno_f, 2, 3> t_row_sno_mat_sno;    /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef row_major unorm matrix<t_uno_f, 2, 3> t_row_uno_mat_uno;    /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef column_major matrix<t_sno_f, 2, 3> t_col_mat_sno;
typedef column_major matrix<t_uno_f, 2, 3> t_col_mat_uno;
typedef column_major snorm matrix<t_sno_f, 2, 3> t_col_sno_mat_sno;    /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef column_major unorm matrix<t_uno_f, 2, 3> t_col_uno_mat_uno;    /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef snorm matrix<t_sno_f, 2, 3> t_sno_mat_sno;          /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef unorm matrix<t_uno_f, 2, 3> t_uno_mat_uno;          /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef row_major snorm matrix<t_uno_f, 2, 3> t_row_sno_mat_uno;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef row_major unorm matrix<t_sno_f, 2, 3> t_row_uno_mat_sno;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef column_major snorm matrix<t_uno_f, 2, 3> t_col_sno_mat_uno;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef column_major unorm matrix<t_sno_f, 2, 3> t_col_uno_mat_sno;    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef snorm matrix<t_uno_f, 2, 3> t_sno_mat_uno;          /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
typedef unorm matrix<t_sno_f, 2, 3> t_uno_mat_sno;          /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'matrix'}} */
// GENERATED_CODE:END

// Typedefs defining matrix specializations with modified component type defined inline
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('typedef matrix<%(mods)s float, 2, 3> t_mat_%(id)s;', combos=norm_combos, val=True))</py>
// GENERATED_CODE:BEGIN
typedef matrix<snorm float, 2, 3> t_mat_sno;
typedef matrix<unorm float, 2, 3> t_mat_uno;
// GENERATED_CODE:END

// Typedefs adding modifiers to matrix typedef where component type is modified in inline matrix specialization
// <py::lines('GENERATED_CODE')>modify(lines, gen_code2('typedef %(mods1)s t_mat_%(id2)s t_%(id1)s_mat_%(id2)s;', combos2=norm_combos, val=True))</py>
// GENERATED_CODE:BEGIN
typedef row_major t_mat_sno t_row_mat_sno;                  /* fxc-error {{X3003: redefinition of 't_row_mat_sno'}} */
typedef row_major t_mat_uno t_row_mat_uno;                  /* fxc-error {{X3003: redefinition of 't_row_mat_uno'}} */
typedef row_major snorm t_mat_sno t_row_sno_mat_sno;        /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3085: snorm can not be used with type}} */
typedef row_major unorm t_mat_uno t_row_uno_mat_uno;        /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3085: unorm can not be used with type}} */
typedef column_major t_mat_sno t_col_mat_sno;               /* fxc-error {{X3003: redefinition of 't_col_mat_sno'}} */
typedef column_major t_mat_uno t_col_mat_uno;               /* fxc-error {{X3003: redefinition of 't_col_mat_uno'}} */
typedef column_major snorm t_mat_sno t_col_sno_mat_sno;     /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3085: snorm can not be used with type}} */
typedef column_major unorm t_mat_uno t_col_uno_mat_uno;     /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3085: unorm can not be used with type}} */
typedef snorm t_mat_sno t_sno_mat_sno;                                 /* expected-warning {{attribute 'snorm' is already applied}} fxc-error {{X3085: snorm can not be used with type}} */
typedef unorm t_mat_uno t_uno_mat_uno;                                 /* expected-warning {{attribute 'unorm' is already applied}} fxc-error {{X3085: unorm can not be used with type}} */
typedef row_major snorm t_mat_uno t_row_sno_mat_uno;                   /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3085: snorm can not be used with type}} */
typedef row_major unorm t_mat_sno t_row_uno_mat_sno;                   /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3085: unorm can not be used with type}} */
typedef column_major snorm t_mat_uno t_col_sno_mat_uno;                /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3085: snorm can not be used with type}} */
typedef column_major unorm t_mat_sno t_col_uno_mat_sno;                /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3085: unorm can not be used with type}} */
typedef snorm t_mat_uno t_sno_mat_uno;                                 /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3085: snorm can not be used with type}} */
typedef unorm t_mat_sno t_uno_mat_sno;                                 /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3085: unorm can not be used with type}} */
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('typedef float2x3 %(mods)s t_%(id)s_after;'))</py>
// GENERATED_CODE:BEGIN
typedef float2x3 row_major t_row_after;                     /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
typedef float2x3 row_major snorm t_row_sno_after;           /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
typedef float2x3 row_major unorm t_row_uno_after;           /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
typedef float2x3 column_major t_col_after;                  /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
typedef float2x3 column_major snorm t_col_sno_after;        /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
typedef float2x3 column_major unorm t_col_uno_after;        /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
typedef float2x3 snorm t_sno_after;                         /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'snorm'}} */
typedef float2x3 unorm t_uno_after;                         /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
typedef float2x3 row_major column_major t_row_col_after;    /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
typedef float2x3 row_major column_major snorm t_row_col_sno_after;    /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
typedef float2x3 row_major column_major snorm unorm t_row_col_sno_uno_after;    /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
typedef float2x3 row_major column_major unorm t_row_col_uno_after;    /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
typedef float2x3 row_major snorm unorm t_row_sno_uno_after; /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
typedef float2x3 column_major snorm unorm t_col_sno_uno_after;    /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
typedef float2x3 snorm unorm t_sno_uno_after;               /* expected-error {{modifiers must appear before type}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'snorm'}} */
// GENERATED_CODE:END

//////////////////////////////////////////////////////////////////////////////
// Fields.
struct s_fields {
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2x3 f_%(id)s;'))</py>
    // GENERATED_CODE:BEGIN
    row_major float2x3 f_row;
    row_major snorm float2x3 f_row_sno;
    row_major unorm float2x3 f_row_uno;
    column_major float2x3 f_col;
    column_major snorm float2x3 f_col_sno;
    column_major unorm float2x3 f_col_uno;
    snorm float2x3 f_sno;
    unorm float2x3 f_uno;
    row_major column_major float2x3 f_row_col;              /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major column_major snorm float2x3 f_row_col_sno;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major column_major snorm unorm float2x3 f_row_col_sno_uno;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    row_major column_major unorm float2x3 f_row_col_uno;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major snorm unorm float2x3 f_row_sno_uno;           /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    column_major snorm unorm float2x3 f_col_sno_uno;        /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    snorm unorm float2x3 f_sno_uno;                         /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    // GENERATED_CODE:END
};

//////////////////////////////////////////////////////////////////////////////
// Parameters
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('float3 foo_%(id)s(%(mods)s float2x3 val) { return val[0]; }'))</py>
// GENERATED_CODE:BEGIN
float3 foo_row(row_major float2x3 val) { return val[0]; }
float3 foo_row_sno(row_major snorm float2x3 val) { return val[0]; }
float3 foo_row_uno(row_major unorm float2x3 val) { return val[0]; }
float3 foo_col(column_major float2x3 val) { return val[0]; }
float3 foo_col_sno(column_major snorm float2x3 val) { return val[0]; }
float3 foo_col_uno(column_major unorm float2x3 val) { return val[0]; }
float3 foo_sno(snorm float2x3 val) { return val[0]; }
float3 foo_uno(unorm float2x3 val) { return val[0]; }
float3 foo_row_col(row_major column_major float2x3 val) { return val[0]; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
float3 foo_row_col_sno(row_major column_major snorm float2x3 val) { return val[0]; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
float3 foo_row_col_sno_uno(row_major column_major snorm unorm float2x3 val) { return val[0]; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} fxc-error {{X3004: undeclared identifier 'val'}} */
float3 foo_row_col_uno(row_major column_major unorm float2x3 val) { return val[0]; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
float3 foo_row_sno_uno(row_major snorm unorm float2x3 val) { return val[0]; } /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} fxc-error {{X3004: undeclared identifier 'val'}} */
float3 foo_col_sno_uno(column_major snorm unorm float2x3 val) { return val[0]; }    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} fxc-error {{X3004: undeclared identifier 'val'}} */
float3 foo_sno_uno(snorm unorm float2x3 val) { return val[0]; }    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} fxc-error {{X3004: undeclared identifier 'val'}} */
// GENERATED_CODE:END

float3 foo_col_after(float2x3 column_major val) {        /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
    return val[0];
}

// Try conflicting column_major/row_major between function declarations and definitions:
// TODO: Figure out what to do with these cases
float3 foo_col_missing_in_decl(float2x3 val);
float3 foo_col_missing_in_decl(column_major float2x3 val) {
    return val[0];
}
float3 foo_col_missing_in_def(column_major float2x3 val);
float3 foo_col_missing_in_def(float2x3 val) {
    return val[0];
}
float3 foo_col_in_decl_row_in_def(column_major float2x3 val);
float3 foo_col_in_decl_row_in_def(row_major float2x3 val) {
    return val[0];
}
float3 use_conflicting_column_row(float2x3 val, column_major float2x3 val_column, row_major float2x3 val_row) {
    float3 res = (float3) 0;
    res += foo_col_missing_in_decl(val);                 /* fxc-error {{X3067: 'foo_col_missing_in_decl': ambiguous function call}} */
    res += foo_col_missing_in_def(val);                  /* fxc-error {{X3067: 'foo_col_missing_in_def': ambiguous function call}} */
    res += foo_col_in_decl_row_in_def(val);              /* fxc-error {{X3067: 'foo_col_in_decl_row_in_def': ambiguous function call}} */
    res += foo_col(val);
    res += foo_col_missing_in_decl(val_column);          /* fxc-error {{X3067: 'foo_col_missing_in_decl': ambiguous function call}} */
    res += foo_col_missing_in_def(val_column);           /* fxc-error {{X3067: 'foo_col_missing_in_def': ambiguous function call}} */
    res += foo_col_in_decl_row_in_def(val_column);       /* fxc-error {{X3067: 'foo_col_in_decl_row_in_def': ambiguous function call}} */
    res += foo_col(val_column);
    res += foo_col_missing_in_decl(val_row);             /* fxc-error {{X3067: 'foo_col_missing_in_decl': ambiguous function call}} */
    res += foo_col_missing_in_def(val_row);              /* fxc-error {{X3067: 'foo_col_missing_in_def': ambiguous function call}} */
    res += foo_col_in_decl_row_in_def(val_row);          /* fxc-error {{X3067: 'foo_col_in_decl_row_in_def': ambiguous function call}} */
    res += foo_col(val_row);
    return res;
}

//////////////////////////////////////////////////////////////////////////////
// Locals.
void vain() {
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2x3 l_%(id)s;'))</py>
    // GENERATED_CODE:BEGIN
    row_major float2x3 l_row;
    row_major snorm float2x3 l_row_sno;
    row_major unorm float2x3 l_row_uno;
    column_major float2x3 l_col;
    column_major snorm float2x3 l_col_sno;
    column_major unorm float2x3 l_col_uno;
    snorm float2x3 l_sno;
    unorm float2x3 l_uno;
    row_major column_major float2x3 l_row_col;           /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major column_major snorm float2x3 l_row_col_sno; /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major column_major snorm unorm float2x3 l_row_col_sno_uno;    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    row_major column_major unorm float2x3 l_row_col_uno; /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major snorm unorm float2x3 l_row_sno_uno;        /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    column_major snorm unorm float2x3 l_col_sno_uno;     /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    snorm unorm float2x3 l_sno_uno;                      /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    // GENERATED_CODE:END

    // no type:
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s no_type_%(id)s;', val=True))</py>
    // GENERATED_CODE:BEGIN
    row_major no_type_row;                               /* expected-error {{'row_major' can only be used with a matrix type}} expected-error {{HLSL requires a type specifier for all declarations}} fxc-error {{X3000: unrecognized identifier 'no_type_row'}} */
    row_major snorm no_type_row_sno;                     /* expected-error {{'row_major' can only be used with a matrix type}} expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{snorm and unorm qualifier can only be used on floating-point types}} fxc-error {{X3000: syntax error: unexpected token 'no_type_row_sno'}} */
    row_major unorm no_type_row_uno;                     /* expected-error {{'row_major' can only be used with a matrix type}} expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{snorm and unorm qualifier can only be used on floating-point types}} fxc-error {{X3000: syntax error: unexpected token 'no_type_row_uno'}} */
    column_major no_type_col;                            /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{HLSL requires a type specifier for all declarations}} fxc-error {{X3000: unrecognized identifier 'no_type_col'}} */
    column_major snorm no_type_col_sno;                  /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{snorm and unorm qualifier can only be used on floating-point types}} fxc-error {{X3000: syntax error: unexpected token 'no_type_col_sno'}} */
    column_major unorm no_type_col_uno;                  /* expected-error {{'column_major' can only be used with a matrix type}} expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{snorm and unorm qualifier can only be used on floating-point types}} fxc-error {{X3000: syntax error: unexpected token 'no_type_col_uno'}} */
    snorm no_type_sno;                                   /* expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{snorm and unorm qualifier can only be used on floating-point types}} fxc-error {{X3000: syntax error: unexpected token 'no_type_sno'}} */
    unorm no_type_uno;                                   /* expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{snorm and unorm qualifier can only be used on floating-point types}} fxc-error {{X3000: syntax error: unexpected token 'no_type_uno'}} */
    // GENERATED_CODE:END

    float3x4 column_major l_column_after;                   /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */

    // Various random cases that previously caused some internal errors from unreachable:
    float4 unorm = float4(1,2,3,4);                      /* expected-error {{expected unqualified-id}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    float4 f;
    f = unorm;                                           /* expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    f = unorm(f);                                        /* expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    f = (unorm)f;                                        /* expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{snorm and unorm qualifier can only be used on floating-point types}} fxc-error {{X3000: syntax error: unexpected token ')'}} fxc-error {{X3000: syntax error: unexpected token 'f'}} */
    float4 row_major = float4(1,2,3,4);                  /* expected-error {{expected unqualified-id}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
    f = row_major(1);                                    /* expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token 'row_major'}} */
    row_major float4x4 foobar = {unorm.x};               /* expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    row_major float4x4 foo;
    float4 unorm = foo[0];                               /* expected-error {{expected unqualified-id}} expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    foo[0] = unorm;                                      /* expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    float4 f2 = unorm col_major;                         /* expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    f = ((row_major unorm float4x4)foo)[0];

}

//////////////////////////////////////////////////////////////////////////////
// Functions.
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2x3 fn_%(id)s() { return 1.0f; }'))</py>
// GENERATED_CODE:BEGIN
row_major float2x3 fn_row() { return 1.0f; }
row_major snorm float2x3 fn_row_sno() { return 1.0f; }
row_major unorm float2x3 fn_row_uno() { return 1.0f; }
column_major float2x3 fn_col() { return 1.0f; }
column_major snorm float2x3 fn_col_sno() { return 1.0f; }
column_major unorm float2x3 fn_col_uno() { return 1.0f; }
snorm float2x3 fn_sno() { return 1.0f; }
unorm float2x3 fn_uno() { return 1.0f; }
row_major column_major float2x3 fn_row_col() { return 1.0f; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
row_major column_major snorm float2x3 fn_row_col_sno() { return 1.0f; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
row_major column_major snorm unorm float2x3 fn_row_col_sno_uno() { return 1.0f; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
row_major column_major unorm float2x3 fn_row_col_uno() { return 1.0f; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
row_major snorm unorm float2x3 fn_row_sno_uno() { return 1.0f; } /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
column_major snorm unorm float2x3 fn_col_sno_uno() { return 1.0f; }    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
snorm unorm float2x3 fn_sno_uno() { return 1.0f; }          /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
// GENERATED_CODE:END

float3x4 column_major fn_column_after() {                   /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
  return (float3x4) 1.0;
}
//////////////////////////////////////////////////////////////////////////////
// Methods.
class C
{
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2x3 fn_%(id)s() { return 1.0f; }'))</py>
    // GENERATED_CODE:BEGIN
    row_major float2x3 fn_row() { return 1.0f; }
    row_major snorm float2x3 fn_row_sno() { return 1.0f; }
    row_major unorm float2x3 fn_row_uno() { return 1.0f; }
    column_major float2x3 fn_col() { return 1.0f; }
    column_major snorm float2x3 fn_col_sno() { return 1.0f; }
    column_major unorm float2x3 fn_col_uno() { return 1.0f; }
    snorm float2x3 fn_sno() { return 1.0f; }
    unorm float2x3 fn_uno() { return 1.0f; }
    row_major column_major float2x3 fn_row_col() { return 1.0f; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major column_major snorm float2x3 fn_row_col_sno() { return 1.0f; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major column_major snorm unorm float2x3 fn_row_col_sno_uno() { return 1.0f; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    row_major column_major unorm float2x3 fn_row_col_uno() { return 1.0f; }    /* expected-error {{'row_major' and 'column_major' attributes are not compatible}} fxc-error {{X3048: matrix types cannot be both column_major and row_major}} */
    row_major snorm unorm float2x3 fn_row_sno_uno() { return 1.0f; } /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    column_major snorm unorm float2x3 fn_col_sno_uno() { return 1.0f; }    /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    snorm unorm float2x3 fn_sno_uno() { return 1.0f; }      /* expected-error {{'unorm' and 'snorm' attributes are not compatible}} fxc-error {{X3000: syntax error: unexpected token 'unorm'}} */
    // GENERATED_CODE:END

    float3x4 column_major fn_column_after() {               /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'column_major'}} */
      return (float3x4) 1.0;
    }

};