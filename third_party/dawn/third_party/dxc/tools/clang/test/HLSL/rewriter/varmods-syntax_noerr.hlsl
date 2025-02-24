// RUN: %clang_cc1 -fsyntax-only -ffreestanding -verify %s

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
linear_mods = ['linear', 'sample', 'noperspective', 'centroid']
interp_combos = [('nointerpolation',)] + [('nointerpolation', mod) for mod in linear_mods] + list(gen_combos(linear_mods))
storage_mods = 'groupshared extern precise static uniform volatile const'.split()
bad_storage_combos = [('groupshared', 'extern'), 
                      ('extern', 'static'),
                      ('static', 'uniform')]
storage_combos = bad_storage_combos + list(gen_combos(storage_mods, exclude=bad_storage_combos))
def gen_code(template, combos=interp_combos, trunc=3):
    trunc = make_trunc(trunc)
    return [
        template % {
            'mods': ' '.join(combo), 
            'id': '_'.join(map(trunc, combo))} 
        for combo in combos]
</py>*/

//////////////////////////////////////////////////////////////////////////////
// Global variables.

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2 g_%(id)s;', storage_combos))</py>
// GENERATED_CODE:BEGIN
//groupshared extern float2 g_gro_ext;                        /* expected-error {{'extern' and 'groupshared' cannot be used together for a global variable}} fxc-error {{X3010: 'g_gro_ext': extern global variables cannot be declared 'groupshared'}} */
//extern static float2 g_ext_sta;                             /* expected-error {{cannot combine with previous 'extern' declaration specifier}} fxc-error {{X3007: 'g_ext_sta': extern global variables cannot be declared 'static'}} */
//static uniform float2 g_sta_uni;                            /* expected-error {{'static' and 'uniform' cannot be used together for a global variable}} fxc-error {{X3007: 'g_sta_uni': uniform global variables cannot be declared 'static'}} */
groupshared float2 g_gro;
groupshared precise float2 g_gro_pre;
groupshared precise static float2 g_gro_pre_sta;
//groupshared precise static volatile float2 g_gro_pre_sta_vol;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_sta_vol': global variables cannot be declared 'volatile'}} */
//groupshared precise static volatile const float2 g_gro_pre_sta_vol_con;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_sta_vol_con': global variables cannot be declared 'volatile'}} fxc-error {{X3012: 'g_gro_pre_sta_vol_con': missing initial value}} */
//groupshared precise static const float2 g_gro_pre_sta_con;                 /* fxc-error {{X3012: 'g_gro_pre_sta_con': missing initial value}} */
//groupshared precise uniform float2 g_gro_pre_uni;                          /* fxc-error {{X3010: 'g_gro_pre_uni': uniform global variables cannot be declared 'groupshared'}} */
//groupshared precise uniform volatile float2 g_gro_pre_uni_vol;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_uni_vol': global variables cannot be declared 'volatile'}} fxc-error {{X3010: 'g_gro_pre_uni_vol': uniform global variables cannot be declared 'groupshared'}} */
//groupshared precise uniform volatile const float2 g_gro_pre_uni_vol_con;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_uni_vol_con': global variables cannot be declared 'volatile'}} fxc-error {{X3010: 'g_gro_pre_uni_vol_con': uniform global variables cannot be declared 'groupshared'}} */
//groupshared precise uniform const float2 g_gro_pre_uni_con;                 /* fxc-error {{X3010: 'g_gro_pre_uni_con': uniform global variables cannot be declared 'groupshared'}} */
//groupshared precise volatile float2 g_gro_pre_vol;          /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_vol': global variables cannot be declared 'volatile'}} */
//groupshared precise volatile const float2 g_gro_pre_vol_con;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_vol_con': global variables cannot be declared 'volatile'}} fxc-error {{X3012: 'g_gro_pre_vol_con': missing initial value}} */
//groupshared precise const float2 g_gro_pre_con;                 /* fxc-error {{X3012: 'g_gro_pre_con': missing initial value}} */
groupshared static float2 g_gro_sta;
//groupshared static volatile float2 g_gro_sta_vol;           /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_sta_vol': global variables cannot be declared 'volatile'}} */
//groupshared static volatile const float2 g_gro_sta_vol_con; /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_sta_vol_con': global variables cannot be declared 'volatile'}} fxc-error {{X3012: 'g_gro_sta_vol_con': missing initial value}} */
//groupshared static const float2 g_gro_sta_con;              /* fxc-error {{X3012: 'g_gro_sta_con': missing initial value}} */
//groupshared uniform float2 g_gro_uni;                       /* fxc-error {{X3010: 'g_gro_uni': uniform global variables cannot be declared 'groupshared'}} */
//groupshared uniform volatile float2 g_gro_uni_vol;          /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_uni_vol': global variables cannot be declared 'volatile'}} fxc-error {{X3010: 'g_gro_uni_vol': uniform global variables cannot be declared 'groupshared'}} */
//groupshared uniform volatile const float2 g_gro_uni_vol_con;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_uni_vol_con': global variables cannot be declared 'volatile'}} fxc-error {{X3010: 'g_gro_uni_vol_con': uniform global variables cannot be declared 'groupshared'}} */
//groupshared uniform const float2 g_gro_uni_con;                 /* fxc-error {{X3010: 'g_gro_uni_con': uniform global variables cannot be declared 'groupshared'}} */
//groupshared volatile float2 g_gro_vol;                      /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_vol': global variables cannot be declared 'volatile'}} */
//groupshared volatile const float2 g_gro_vol_con;            /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_vol_con': global variables cannot be declared 'volatile'}} fxc-error {{X3012: 'g_gro_vol_con': missing initial value}} */
//groupshared const float2 g_gro_con;                         /* fxc-error {{X3012: 'g_gro_con': missing initial value}} */
extern float2 g_ext;
extern precise float2 g_ext_pre;
extern precise uniform float2 g_ext_pre_uni;
//extern precise uniform volatile float2 g_ext_pre_uni_vol;   /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_pre_uni_vol': global variables cannot be declared 'volatile'}} */
//extern precise uniform volatile const float2 g_ext_pre_uni_vol_con;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_pre_uni_vol_con': global variables cannot be declared 'volatile'}} */
extern precise uniform const float2 g_ext_pre_uni_con;
//extern precise volatile float2 g_ext_pre_vol;               /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_pre_vol': global variables cannot be declared 'volatile'}} */
//extern precise volatile const float2 g_ext_pre_vol_con;     /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_pre_vol_con': global variables cannot be declared 'volatile'}} */
extern precise const float2 g_ext_pre_con;
extern uniform float2 g_ext_uni;
//extern uniform volatile float2 g_ext_uni_vol;               /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_uni_vol': global variables cannot be declared 'volatile'}} */
//extern uniform volatile const float2 g_ext_uni_vol_con;     /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_uni_vol_con': global variables cannot be declared 'volatile'}} */
extern uniform const float2 g_ext_uni_con;
//extern volatile float2 g_ext_vol;                           /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_vol': global variables cannot be declared 'volatile'}} */
//extern volatile const float2 g_ext_vol_con;                 /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_vol_con': global variables cannot be declared 'volatile'}} */
extern const float2 g_ext_con;
precise float2 g_pre;
precise static float2 g_pre_sta;
//precise static volatile float2 g_pre_sta_vol;               /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_sta_vol': global variables cannot be declared 'volatile'}} */
//precise static volatile const float2 g_pre_sta_vol_con;     /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_sta_vol_con': global variables cannot be declared 'volatile'}} */
precise static const float2 g_pre_sta_con;
precise uniform float2 g_pre_uni;
//precise uniform volatile float2 g_pre_uni_vol;              /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_uni_vol': global variables cannot be declared 'volatile'}} */
//precise uniform volatile const float2 g_pre_uni_vol_con;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_uni_vol_con': global variables cannot be declared 'volatile'}} */
precise uniform const float2 g_pre_uni_con;
//precise volatile float2 g_pre_vol;                          /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_vol': global variables cannot be declared 'volatile'}} */
//precise volatile const float2 g_pre_vol_con;                /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_vol_con': global variables cannot be declared 'volatile'}} */
precise const float2 g_pre_con;
static float2 g_sta;
//static volatile float2 g_sta_vol;                           /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_sta_vol': global variables cannot be declared 'volatile'}} */
//static volatile const float2 g_sta_vol_con;                 /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_sta_vol_con': global variables cannot be declared 'volatile'}} */
static const float2 g_sta_con;
uniform float2 g_uni;
//uniform volatile float2 g_uni_vol;                          /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_uni_vol': global variables cannot be declared 'volatile'}} */
//uniform volatile const float2 g_uni_vol_con;                /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_uni_vol_con': global variables cannot be declared 'volatile'}} */
uniform const float2 g_uni_con;
//volatile float2 g_vol;                                      /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_vol': global variables cannot be declared 'volatile'}} */
//volatile const float2 g_vol_con;                            /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_vol_con': global variables cannot be declared 'volatile'}} */
const float2 g_con;
// GENERATED_CODE:END

// Initialized:
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float g_%(id)s_init = 1.0f;', storage_combos))</py>
// GENERATED_CODE:BEGIN
//groupshared extern float g_gro_ext_init = 1.0f;             /* expected-error {{'extern' and 'groupshared' cannot be used together for a global variable}} fxc-error {{X3010: 'g_gro_ext_init': extern global variables cannot be declared 'groupshared'}} */
//extern static float g_ext_sta_init = 1.0f;                  /* expected-error {{cannot combine with previous 'extern' declaration specifier}} expected-warning {{'extern' variable has an initializer}} fxc-error {{X3007: 'g_ext_sta_init': extern global variables cannot be declared 'static'}} */
//static uniform float g_sta_uni_init = 1.0f;                 /* expected-error {{'static' and 'uniform' cannot be used together for a global variable}} fxc-error {{X3007: 'g_sta_uni_init': uniform global variables cannot be declared 'static'}} */
groupshared float g_gro_init = 1.0f;
groupshared precise float g_gro_pre_init = 1.0f;
groupshared precise static float g_gro_pre_sta_init = 1.0f;
//groupshared precise static volatile float g_gro_pre_sta_vol_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_sta_vol_init': global variables cannot be declared 'volatile'}} */
//groupshared precise static volatile const float g_gro_pre_sta_vol_con_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_sta_vol_con_init': global variables cannot be declared 'volatile'}} */
groupshared precise static const float g_gro_pre_sta_con_init = 1.0f;
//groupshared precise uniform float g_gro_pre_uni_init = 1.0f;    /* fxc-error {{X3010: 'g_gro_pre_uni_init': uniform global variables cannot be declared 'groupshared'}} */
//groupshared precise uniform volatile float g_gro_pre_uni_vol_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_uni_vol_init': global variables cannot be declared 'volatile'}} fxc-error {{X3010: 'g_gro_pre_uni_vol_init': uniform global variables cannot be declared 'groupshared'}} */
//groupshared precise uniform volatile const float g_gro_pre_uni_vol_con_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_uni_vol_con_init': global variables cannot be declared 'volatile'}} fxc-error {{X3010: 'g_gro_pre_uni_vol_con_init': uniform global variables cannot be declared 'groupshared'}} */
//groupshared precise uniform const float g_gro_pre_uni_con_init = 1.0f;    /* fxc-error {{X3010: 'g_gro_pre_uni_con_init': uniform global variables cannot be declared 'groupshared'}} */
//groupshared precise volatile float g_gro_pre_vol_init = 1.0f;             /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_vol_init': global variables cannot be declared 'volatile'}} */
//groupshared precise volatile const float g_gro_pre_vol_con_init = 1.0f;   /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_pre_vol_con_init': global variables cannot be declared 'volatile'}} */
groupshared precise const float g_gro_pre_con_init = 1.0f;
groupshared static float g_gro_sta_init = 1.0f;
//groupshared static volatile float g_gro_sta_vol_init = 1.0f;              /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_sta_vol_init': global variables cannot be declared 'volatile'}} */
//groupshared static volatile const float g_gro_sta_vol_con_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_sta_vol_con_init': global variables cannot be declared 'volatile'}} */
groupshared static const float g_gro_sta_con_init = 1.0f;
//groupshared uniform float g_gro_uni_init = 1.0f;            /* fxc-error {{X3010: 'g_gro_uni_init': uniform global variables cannot be declared 'groupshared'}} */
//groupshared uniform volatile float g_gro_uni_vol_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_uni_vol_init': global variables cannot be declared 'volatile'}} fxc-error {{X3010: 'g_gro_uni_vol_init': uniform global variables cannot be declared 'groupshared'}} */
//groupshared uniform volatile const float g_gro_uni_vol_con_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_uni_vol_con_init': global variables cannot be declared 'volatile'}} fxc-error {{X3010: 'g_gro_uni_vol_con_init': uniform global variables cannot be declared 'groupshared'}} */
//groupshared uniform const float g_gro_uni_con_init = 1.0f;  /* fxc-error {{X3010: 'g_gro_uni_con_init': uniform global variables cannot be declared 'groupshared'}} */
//groupshared volatile float g_gro_vol_init = 1.0f;           /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_vol_init': global variables cannot be declared 'volatile'}} */
//groupshared volatile const float g_gro_vol_con_init = 1.0f; /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_gro_vol_con_init': global variables cannot be declared 'volatile'}} */
groupshared const float g_gro_con_init = 1.0f;
//extern float g_ext_init = 1.0f;                             /* expected-warning {{'extern' variable has an initializer}} fxc-pass {{}} */
//extern precise float g_ext_pre_init = 1.0f;                 /* expected-warning {{'extern' variable has an initializer}} fxc-pass {{}} */
//extern precise uniform float g_ext_pre_uni_init = 1.0f;     /* expected-warning {{'extern' variable has an initializer}} fxc-pass {{}} */
//extern precise uniform volatile float g_ext_pre_uni_vol_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_pre_uni_vol_init': global variables cannot be declared 'volatile'}} */
//extern precise uniform volatile const float g_ext_pre_uni_vol_con_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_pre_uni_vol_con_init': global variables cannot be declared 'volatile'}} */
//extern precise uniform const float g_ext_pre_uni_con_init = 1.0f;    /* expected-warning {{'extern' variable has an initializer}} fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
//extern precise volatile float g_ext_pre_vol_init = 1.0f;             /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_pre_vol_init': global variables cannot be declared 'volatile'}} */
//extern precise volatile const float g_ext_pre_vol_con_init = 1.0f;   /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_pre_vol_con_init': global variables cannot be declared 'volatile'}} */
//extern precise const float g_ext_pre_con_init = 1.0f;       /* expected-warning {{'extern' variable has an initializer}} fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
//extern uniform float g_ext_uni_init = 1.0f;                 /* expected-warning {{'extern' variable has an initializer}} fxc-pass {{}} */
//extern uniform volatile float g_ext_uni_vol_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_uni_vol_init': global variables cannot be declared 'volatile'}} */
//extern uniform volatile const float g_ext_uni_vol_con_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_uni_vol_con_init': global variables cannot be declared 'volatile'}} */
//extern uniform const float g_ext_uni_con_init = 1.0f;       /* expected-warning {{'extern' variable has an initializer}} fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
//extern volatile float g_ext_vol_init = 1.0f;                /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_vol_init': global variables cannot be declared 'volatile'}} */
//extern volatile const float g_ext_vol_con_init = 1.0f;      /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_ext_vol_con_init': global variables cannot be declared 'volatile'}} */
//extern const float g_ext_con_init = 1.0f;                   /* expected-warning {{'extern' variable has an initializer}} fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
precise float g_pre_init = 1.0f;
precise static float g_pre_sta_init = 1.0f;
//precise static volatile float g_pre_sta_vol_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_sta_vol_init': global variables cannot be declared 'volatile'}} */
//precise static volatile const float g_pre_sta_vol_con_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_sta_vol_con_init': global variables cannot be declared 'volatile'}} */
precise static const float g_pre_sta_con_init = 1.0f;
precise uniform float g_pre_uni_init = 1.0f;
//precise uniform volatile float g_pre_uni_vol_init = 1.0f;   /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_uni_vol_init': global variables cannot be declared 'volatile'}} */
//precise uniform volatile const float g_pre_uni_vol_con_init = 1.0f;    /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_uni_vol_con_init': global variables cannot be declared 'volatile'}} */
//precise uniform const float g_pre_uni_con_init = 1.0f;      /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
//precise volatile float g_pre_vol_init = 1.0f;               /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_vol_init': global variables cannot be declared 'volatile'}} */
//precise volatile const float g_pre_vol_con_init = 1.0f;     /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_pre_vol_con_init': global variables cannot be declared 'volatile'}} */
//precise const float g_pre_con_init = 1.0f;                  /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
static float g_sta_init = 1.0f;
//static volatile float g_sta_vol_init = 1.0f;                /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_sta_vol_init': global variables cannot be declared 'volatile'}} */
//static volatile const float g_sta_vol_con_init = 1.0f;      /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_sta_vol_con_init': global variables cannot be declared 'volatile'}} */
static const float g_sta_con_init = 1.0f;
uniform float g_uni_init = 1.0f;
//uniform volatile float g_uni_vol_init = 1.0f;               /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_uni_vol_init': global variables cannot be declared 'volatile'}} */
//uniform volatile const float g_uni_vol_con_init = 1.0f;     /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_uni_vol_con_init': global variables cannot be declared 'volatile'}} */
//uniform const float g_uni_con_init = 1.0f;                  /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
//volatile float g_vol_init = 1.0f;                           /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_vol_init': global variables cannot be declared 'volatile'}} */
//volatile const float g_vol_con_init = 1.0f;                 /* expected-error {{'volatile' is not a valid modifier for a global variable}} fxc-error {{X3008: 'g_vol_con_init': global variables cannot be declared 'volatile'}} */
//const float g_con_init = 1.0f;                              /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2 g_%(id)s;'))</py>
// GENERATED_CODE:BEGIN
//nointerpolation float2 g_noi;                               /* expected-error {{'nointerpolation' is not a valid modifier for a global variable}} fxc-pass {{}} */
//nointerpolation linear float2 g_noi_lin;                    /* expected-error {{'linear' is not a valid modifier for a global variable}} expected-error {{'nointerpolation' and 'linear' cannot be used together for a global variable}} expected-error {{'nointerpolation' is not a valid modifier for a global variable}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
//nointerpolation sample float2 g_noi_sam;                    /* expected-error {{'nointerpolation' and 'sample' cannot be used together for a global variable}} expected-error {{'nointerpolation' is not a valid modifier for a global variable}} expected-error {{'sample' is not a valid modifier for a global variable}} fxc-pass {{}} */
//nointerpolation noperspective float2 g_noi_nop;             /* expected-error {{'nointerpolation' and 'noperspective' cannot be used together for a global variable}} expected-error {{'nointerpolation' is not a valid modifier for a global variable}} expected-error {{'noperspective' is not a valid modifier for a global variable}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
//nointerpolation centroid float2 g_noi_cen;                  /* expected-error {{'centroid' is not a valid modifier for a global variable}} expected-error {{'nointerpolation' and 'centroid' cannot be used together for a global variable}} expected-error {{'nointerpolation' is not a valid modifier for a global variable}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
//linear float2 g_lin;                                        /* expected-error {{'linear' is not a valid modifier for a global variable}} fxc-pass {{}} */
//linear sample float2 g_lin_sam;                             /* expected-error {{'linear' is not a valid modifier for a global variable}} expected-error {{'sample' is not a valid modifier for a global variable}} fxc-pass {{}} */
//linear sample noperspective float2 g_lin_sam_nop;           /* expected-error {{'linear' is not a valid modifier for a global variable}} expected-error {{'noperspective' is not a valid modifier for a global variable}} expected-error {{'sample' is not a valid modifier for a global variable}} fxc-pass {{}} */
//linear sample noperspective centroid float2 g_lin_sam_nop_cen;    /* expected-error {{'centroid' is not a valid modifier for a global variable}} expected-error {{'linear' is not a valid modifier for a global variable}} expected-error {{'noperspective' is not a valid modifier for a global variable}} expected-error {{'sample' is not a valid modifier for a global variable}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
//linear sample centroid float2 g_lin_sam_cen;                /* expected-error {{'centroid' is not a valid modifier for a global variable}} expected-error {{'linear' is not a valid modifier for a global variable}} expected-error {{'sample' is not a valid modifier for a global variable}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
//linear noperspective float2 g_lin_nop;                      /* expected-error {{'linear' is not a valid modifier for a global variable}} expected-error {{'noperspective' is not a valid modifier for a global variable}} fxc-pass {{}} */
//linear noperspective centroid float2 g_lin_nop_cen;         /* expected-error {{'centroid' is not a valid modifier for a global variable}} expected-error {{'linear' is not a valid modifier for a global variable}} expected-error {{'noperspective' is not a valid modifier for a global variable}} fxc-pass {{}} */
//linear centroid float2 g_lin_cen;                           /* expected-error {{'centroid' is not a valid modifier for a global variable}} expected-error {{'linear' is not a valid modifier for a global variable}} fxc-pass {{}} */
//sample float2 g_sam;                                        /* expected-error {{'sample' is not a valid modifier for a global variable}} fxc-pass {{}} */
//sample noperspective float2 g_sam_nop;                      /* expected-error {{'noperspective' is not a valid modifier for a global variable}} expected-error {{'sample' is not a valid modifier for a global variable}} fxc-pass {{}} */
//sample noperspective centroid float2 g_sam_nop_cen;         /* expected-error {{'centroid' is not a valid modifier for a global variable}} expected-error {{'noperspective' is not a valid modifier for a global variable}} expected-error {{'sample' is not a valid modifier for a global variable}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
//sample centroid float2 g_sam_cen;                           /* expected-error {{'centroid' is not a valid modifier for a global variable}} expected-error {{'sample' is not a valid modifier for a global variable}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
//noperspective float2 g_nop;                                 /* expected-error {{'noperspective' is not a valid modifier for a global variable}} fxc-pass {{}} */
//noperspective centroid float2 g_nop_cen;                    /* expected-error {{'centroid' is not a valid modifier for a global variable}} expected-error {{'noperspective' is not a valid modifier for a global variable}} fxc-pass {{}} */
//centroid float2 g_cen;                                      /* expected-error {{'centroid' is not a valid modifier for a global variable}} fxc-pass {{}} */
// GENERATED_CODE:END

//in float g_in;                                              /* expected-error {{HLSL usage 'in' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'in'}} */
//inout float g_inout;                                        /* expected-error {{HLSL usage 'inout' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'inout'}} */
//out float g_out;                                            /* expected-error {{HLSL usage 'out' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'out'}} */
//float inout g_inoutAfterType;                               /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'inout'}} */
//in out float g_in;                                          /* expected-error {{HLSL usage 'in' is only valid on a parameter}} expected-error {{HLSL usage 'out' is only valid on a parameter}} expected-error {{only one usage is allowed on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'in'}} */
//
//static extern float g_staticExtern;                         /* expected-error {{cannot combine with previous 'static' declaration specifier}} fxc-error {{X3007: 'g_staticExtern': extern global variables cannot be declared 'static'}} */
//static uniform float g_staticUniform;                       /* expected-error {{'static' and 'uniform' cannot be used together for a global variable}} fxc-error {{X3007: 'g_staticUniform': uniform global variables cannot be declared 'static'}} */

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('typedef %(mods)s float2 t_%(id)s;', storage_combos))</py>
// GENERATED_CODE:BEGIN
//typedef groupshared extern float2 t_gro_ext;                /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef extern static float2 t_ext_sta;                     /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef static uniform float2 t_sta_uni;                    /* expected-error {{'uniform' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'static'}} */
//typedef groupshared float2 t_gro;                           /* expected-error {{'groupshared' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise float2 t_gro_pre;               /* expected-error {{'groupshared' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise static float2 t_gro_pre_sta;    /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise static volatile float2 t_gro_pre_sta_vol;    /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise static volatile const float2 t_gro_pre_sta_vol_con;    /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise static const float2 t_gro_pre_sta_con;    /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise uniform float2 t_gro_pre_uni;   /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise uniform volatile float2 t_gro_pre_uni_vol;    /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise uniform volatile const float2 t_gro_pre_uni_vol_con;    /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise uniform const float2 t_gro_pre_uni_con;    /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise volatile float2 t_gro_pre_vol;             /* expected-error {{'groupshared' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise volatile const float2 t_gro_pre_vol_con;   /* expected-error {{'groupshared' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared precise const float2 t_gro_pre_con;     /* expected-error {{'groupshared' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared static float2 t_gro_sta;                /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared static volatile float2 t_gro_sta_vol;   /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared static volatile const float2 t_gro_sta_vol_con;    /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared static const float2 t_gro_sta_con;      /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared uniform float2 t_gro_uni;               /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared uniform volatile float2 t_gro_uni_vol;  /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared uniform volatile const float2 t_gro_uni_vol_con;    /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared uniform const float2 t_gro_uni_con;     /* expected-error {{'groupshared' is not a valid modifier for a typedef}} expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared volatile float2 t_gro_vol;              /* expected-error {{'groupshared' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared volatile const float2 t_gro_vol_con;    /* expected-error {{'groupshared' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef groupshared const float2 t_gro_con;                 /* expected-error {{'groupshared' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} */
//typedef extern float2 t_ext;                                /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern precise float2 t_ext_pre;                    /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern precise uniform float2 t_ext_pre_uni;        /* expected-error {{'uniform' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern precise uniform volatile float2 t_ext_pre_uni_vol;    /* expected-error {{'uniform' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern precise uniform volatile const float2 t_ext_pre_uni_vol_con;    /* expected-error {{'uniform' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern precise uniform const float2 t_ext_pre_uni_con;    /* expected-error {{'uniform' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern precise volatile float2 t_ext_pre_vol;             /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern precise volatile const float2 t_ext_pre_vol_con;   /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern precise const float2 t_ext_pre_con;          /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern uniform float2 t_ext_uni;                    /* expected-error {{'uniform' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern uniform volatile float2 t_ext_uni_vol;       /* expected-error {{'uniform' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern uniform volatile const float2 t_ext_uni_vol_con;    /* expected-error {{'uniform' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern uniform const float2 t_ext_uni_con;          /* expected-error {{'uniform' is not a valid modifier for a typedef}} expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern volatile float2 t_ext_vol;                   /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern volatile const float2 t_ext_vol_con;         /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef extern const float2 t_ext_con;                      /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} */
//typedef precise float2 t_pre;
//typedef precise static float2 t_pre_sta;                    /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'static'}} */
//typedef precise static volatile float2 t_pre_sta_vol;       /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'static'}} */
//typedef precise static volatile const float2 t_pre_sta_vol_con;    /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'static'}} */
//typedef precise static const float2 t_pre_sta_con;          /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'static'}} */
//typedef precise uniform float2 t_pre_uni;                   /* expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'uniform'}} */
//typedef precise uniform volatile float2 t_pre_uni_vol;      /* expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'uniform'}} */
//typedef precise uniform volatile const float2 t_pre_uni_vol_con;    /* expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'uniform'}} */
//typedef precise uniform const float2 t_pre_uni_con;         /* expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'uniform'}} */
typedef precise volatile float2 t_pre_vol;
typedef precise volatile const float2 t_pre_vol_con;
typedef precise const float2 t_pre_con;
//typedef static float2 t_sta;                                /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'static'}} */
//typedef static volatile float2 t_sta_vol;                   /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'static'}} */
//typedef static volatile const float2 t_sta_vol_con;         /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'static'}} */
//typedef static const float2 t_sta_con;                      /* expected-error {{cannot combine with previous 'typedef' declaration specifier}} fxc-error {{X3000: syntax error: unexpected token 'static'}} */
//typedef uniform float2 t_uni;                               /* expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'uniform'}} */
//typedef uniform volatile float2 t_uni_vol;                  /* expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'uniform'}} */
//typedef uniform volatile const float2 t_uni_vol_con;        /* expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'uniform'}} */
//typedef uniform const float2 t_uni_con;                     /* expected-error {{'uniform' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'uniform'}} */
typedef volatile float2 t_vol;
typedef volatile const float2 t_vol_con;
typedef const float2 t_con;
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('typedef %(mods)s float2 t_%(id)s;'))</py>
// GENERATED_CODE:BEGIN
//typedef nointerpolation float2 t_noi;                       /* expected-error {{'nointerpolation' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'nointerpolation'}} */
//typedef nointerpolation linear float2 t_noi_lin;            /* expected-error {{'linear' is not a valid modifier for a typedef}} expected-error {{'nointerpolation' and 'linear' cannot be used together for a typedef}} expected-error {{'nointerpolation' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'nointerpolation'}} */
//typedef nointerpolation sample float2 t_noi_sam;            /* expected-error {{'nointerpolation' and 'sample' cannot be used together for a typedef}} expected-error {{'nointerpolation' is not a valid modifier for a typedef}} expected-error {{'sample' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'nointerpolation'}} */
//typedef nointerpolation noperspective float2 t_noi_nop;     /* expected-error {{'nointerpolation' and 'noperspective' cannot be used together for a typedef}} expected-error {{'nointerpolation' is not a valid modifier for a typedef}} expected-error {{'noperspective' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'nointerpolation'}} */
//typedef nointerpolation centroid float2 t_noi_cen;          /* expected-error {{'centroid' is not a valid modifier for a typedef}} expected-error {{'nointerpolation' and 'centroid' cannot be used together for a typedef}} expected-error {{'nointerpolation' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'nointerpolation'}} */
//typedef linear float2 t_lin;                                /* expected-error {{'linear' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'linear'}} */
//typedef linear sample float2 t_lin_sam;                     /* expected-error {{'linear' is not a valid modifier for a typedef}} expected-error {{'sample' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'linear'}} */
//typedef linear sample noperspective float2 t_lin_sam_nop;   /* expected-error {{'linear' is not a valid modifier for a typedef}} expected-error {{'noperspective' is not a valid modifier for a typedef}} expected-error {{'sample' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'linear'}} */
//typedef linear sample noperspective centroid float2 t_lin_sam_nop_cen;    /* expected-error {{'centroid' is not a valid modifier for a typedef}} expected-error {{'linear' is not a valid modifier for a typedef}} expected-error {{'noperspective' is not a valid modifier for a typedef}} expected-error {{'sample' is not a valid modifier for a typedef}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-error {{X3000: syntax error: unexpected token 'linear'}} */
//typedef linear sample centroid float2 t_lin_sam_cen;        /* expected-error {{'centroid' is not a valid modifier for a typedef}} expected-error {{'linear' is not a valid modifier for a typedef}} expected-error {{'sample' is not a valid modifier for a typedef}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-error {{X3000: syntax error: unexpected token 'linear'}} */
//typedef linear noperspective float2 t_lin_nop;              /* expected-error {{'linear' is not a valid modifier for a typedef}} expected-error {{'noperspective' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'linear'}} */
//typedef linear noperspective centroid float2 t_lin_nop_cen; /* expected-error {{'centroid' is not a valid modifier for a typedef}} expected-error {{'linear' is not a valid modifier for a typedef}} expected-error {{'noperspective' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'linear'}} */
//typedef linear centroid float2 t_lin_cen;                   /* expected-error {{'centroid' is not a valid modifier for a typedef}} expected-error {{'linear' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'linear'}} */
//typedef sample float2 t_sam;                                /* expected-error {{'sample' is not a valid modifier for a typedef}} fxc-pass {{}} */
//typedef sample noperspective float2 t_sam_nop;              /* expected-error {{'noperspective' is not a valid modifier for a typedef}} expected-error {{'sample' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'noperspective'}} */
//typedef sample noperspective centroid float2 t_sam_nop_cen; /* expected-error {{'centroid' is not a valid modifier for a typedef}} expected-error {{'noperspective' is not a valid modifier for a typedef}} expected-error {{'sample' is not a valid modifier for a typedef}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-error {{X3000: syntax error: unexpected token 'noperspective'}} */
//typedef sample centroid float2 t_sam_cen;                   /* expected-error {{'centroid' is not a valid modifier for a typedef}} expected-error {{'sample' is not a valid modifier for a typedef}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-error {{X3000: syntax error: unexpected token 'centroid'}} */
//typedef noperspective float2 t_nop;                         /* expected-error {{'noperspective' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'noperspective'}} */
//typedef noperspective centroid float2 t_nop_cen;            /* expected-error {{'centroid' is not a valid modifier for a typedef}} expected-error {{'noperspective' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'noperspective'}} */
//typedef centroid float2 t_cen;                              /* expected-error {{'centroid' is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token 'centroid'}} */
// GENERATED_CODE:END

//typedef in float2 t_in;                                     /* expected-error {{HLSL usage 'in' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'in'}} */
//typedef out float2 t_out;                                   /* expected-error {{HLSL usage 'out' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'out'}} */
//typedef inout float2 t_inout;                               /* expected-error {{HLSL usage 'inout' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'inout'}} */

//////////////////////////////////////////////////////////////////////////////
// Fields.
struct s_storage_mods {
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2 f_%(id)s;', storage_combos))</py>
    // GENERATED_CODE:BEGIN
    //groupshared extern float2 f_gro_ext;                    /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_gro_ext': struct/class members cannot be declared 'extern'}} fxc-error {{X3010: 'f_gro_ext': struct/class members cannot be declared 'groupshared'}} */
    //extern static float2 f_ext_sta;                         /* expected-error {{cannot combine with previous 'extern' declaration specifier}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_sta': struct/class members cannot be declared 'extern'}} */
    //static uniform float2 f_sta_uni;                        /* expected-error {{'static' and 'uniform' cannot be used together for a field}} expected-error {{'static' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} fxc-error {{X3047: 'f_sta_uni': struct/class members cannot be declared 'uniform'}} */
    //groupshared float2 f_gro;                               /* expected-error {{'groupshared' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro': struct/class members cannot be declared 'groupshared'}} */
    //groupshared precise float2 f_gro_pre;                   /* expected-error {{'groupshared' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_pre': struct/class members cannot be declared 'groupshared'}} */
    //groupshared precise static float2 f_gro_pre_sta;        /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_pre_sta': struct/class members cannot be declared 'groupshared'}} */
    //groupshared precise static volatile float2 f_gro_pre_sta_vol;    /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_pre_sta_vol': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_pre_sta_vol': struct/class members cannot be declared 'groupshared'}} */
    //groupshared precise static volatile const float2 f_gro_pre_sta_vol_con;    /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_pre_sta_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_pre_sta_vol_con': struct/class members cannot be declared 'groupshared'}} */
    //groupshared precise static const float2 f_gro_pre_sta_con;    /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_pre_sta_con': struct/class members cannot be declared 'groupshared'}} */
    //groupshared precise uniform float2 f_gro_pre_uni;       /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_pre_uni': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3047: 'f_gro_pre_uni': struct/class members cannot be declared 'uniform'}} */
    //groupshared precise uniform volatile float2 f_gro_pre_uni_vol;    /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_pre_uni_vol': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_pre_uni_vol': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3047: 'f_gro_pre_uni_vol': struct/class members cannot be declared 'uniform'}} */
    //groupshared precise uniform volatile const float2 f_gro_pre_uni_vol_con;    /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_pre_uni_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_pre_uni_vol_con': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3035: 'f_gro_pre_uni_vol_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_gro_pre_uni_vol_con': struct/class members cannot be declared 'uniform'}} */
    //groupshared precise uniform const float2 f_gro_pre_uni_con;    /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_pre_uni_con': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3035: 'f_gro_pre_uni_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_gro_pre_uni_con': struct/class members cannot be declared 'uniform'}} */
    //groupshared precise volatile float2 f_gro_pre_vol;             /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_pre_vol': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_pre_vol': struct/class members cannot be declared 'groupshared'}} */
    //groupshared precise volatile const float2 f_gro_pre_vol_con;   /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_pre_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_pre_vol_con': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3035: 'f_gro_pre_vol_con': struct/class members cannot be declared 'const'}} */
    //groupshared precise const float2 f_gro_pre_con;         /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_pre_con': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3035: 'f_gro_pre_con': struct/class members cannot be declared 'const'}} */
    //groupshared static float2 f_gro_sta;                    /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_sta': struct/class members cannot be declared 'groupshared'}} */
    //groupshared static volatile float2 f_gro_sta_vol;       /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_sta_vol': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_sta_vol': struct/class members cannot be declared 'groupshared'}} */
    //groupshared static volatile const float2 f_gro_sta_vol_con;    /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_sta_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_sta_vol_con': struct/class members cannot be declared 'groupshared'}} */
    //groupshared static const float2 f_gro_sta_con;          /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_sta_con': struct/class members cannot be declared 'groupshared'}} */
    //groupshared uniform float2 f_gro_uni;                   /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_uni': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3047: 'f_gro_uni': struct/class members cannot be declared 'uniform'}} */
    //groupshared uniform volatile float2 f_gro_uni_vol;      /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_uni_vol': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_uni_vol': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3047: 'f_gro_uni_vol': struct/class members cannot be declared 'uniform'}} */
    //groupshared uniform volatile const float2 f_gro_uni_vol_con;    /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_uni_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_uni_vol_con': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3035: 'f_gro_uni_vol_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_gro_uni_vol_con': struct/class members cannot be declared 'uniform'}} */
    //groupshared uniform const float2 f_gro_uni_con;         /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_uni_con': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3035: 'f_gro_uni_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_gro_uni_con': struct/class members cannot be declared 'uniform'}} */
    //groupshared volatile float2 f_gro_vol;                  /* expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_vol': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_vol': struct/class members cannot be declared 'groupshared'}} */
    //groupshared volatile const float2 f_gro_vol_con;        /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_gro_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3010: 'f_gro_vol_con': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3035: 'f_gro_vol_con': struct/class members cannot be declared 'const'}} */
    //groupshared const float2 f_gro_con;                     /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'groupshared' is not a valid modifier for a field}} fxc-error {{X3010: 'f_gro_con': struct/class members cannot be declared 'groupshared'}} fxc-error {{X3035: 'f_gro_con': struct/class members cannot be declared 'const'}} */
    //extern float2 f_ext;                                    /* expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext': struct/class members cannot be declared 'extern'}} */
    //extern precise float2 f_ext_pre;                        /* expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_pre': struct/class members cannot be declared 'extern'}} */
    //extern precise uniform float2 f_ext_pre_uni;            /* expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_pre_uni': struct/class members cannot be declared 'extern'}} fxc-error {{X3047: 'f_ext_pre_uni': struct/class members cannot be declared 'uniform'}} */
    //extern precise uniform volatile float2 f_ext_pre_uni_vol;    /* expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_pre_uni_vol': struct/class members cannot be declared 'extern'}} fxc-error {{X3008: 'f_ext_pre_uni_vol': struct/class members cannot be declared 'volatile'}} fxc-error {{X3047: 'f_ext_pre_uni_vol': struct/class members cannot be declared 'uniform'}} */
    //extern precise uniform volatile const float2 f_ext_pre_uni_vol_con;    /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_pre_uni_vol_con': struct/class members cannot be declared 'extern'}} fxc-error {{X3008: 'f_ext_pre_uni_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3035: 'f_ext_pre_uni_vol_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_ext_pre_uni_vol_con': struct/class members cannot be declared 'uniform'}} */
    //extern precise uniform const float2 f_ext_pre_uni_con;  /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_pre_uni_con': struct/class members cannot be declared 'extern'}} fxc-error {{X3035: 'f_ext_pre_uni_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_ext_pre_uni_con': struct/class members cannot be declared 'uniform'}} */
    //extern precise volatile float2 f_ext_pre_vol;           /* expected-error {{'volatile' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_pre_vol': struct/class members cannot be declared 'extern'}} fxc-error {{X3008: 'f_ext_pre_vol': struct/class members cannot be declared 'volatile'}} */
    //extern precise volatile const float2 f_ext_pre_vol_con; /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_pre_vol_con': struct/class members cannot be declared 'extern'}} fxc-error {{X3008: 'f_ext_pre_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3035: 'f_ext_pre_vol_con': struct/class members cannot be declared 'const'}} */
    //extern precise const float2 f_ext_pre_con;              /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_pre_con': struct/class members cannot be declared 'extern'}} fxc-error {{X3035: 'f_ext_pre_con': struct/class members cannot be declared 'const'}} */
    //extern uniform float2 f_ext_uni;                        /* expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_uni': struct/class members cannot be declared 'extern'}} fxc-error {{X3047: 'f_ext_uni': struct/class members cannot be declared 'uniform'}} */
    //extern uniform volatile float2 f_ext_uni_vol;           /* expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_uni_vol': struct/class members cannot be declared 'extern'}} fxc-error {{X3008: 'f_ext_uni_vol': struct/class members cannot be declared 'volatile'}} fxc-error {{X3047: 'f_ext_uni_vol': struct/class members cannot be declared 'uniform'}} */
    //extern uniform volatile const float2 f_ext_uni_vol_con; /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_uni_vol_con': struct/class members cannot be declared 'extern'}} fxc-error {{X3008: 'f_ext_uni_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3035: 'f_ext_uni_vol_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_ext_uni_vol_con': struct/class members cannot be declared 'uniform'}} */
    //extern uniform const float2 f_ext_uni_con;              /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_uni_con': struct/class members cannot be declared 'extern'}} fxc-error {{X3035: 'f_ext_uni_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_ext_uni_con': struct/class members cannot be declared 'uniform'}} */
    //extern volatile float2 f_ext_vol;                       /* expected-error {{'volatile' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_vol': struct/class members cannot be declared 'extern'}} fxc-error {{X3008: 'f_ext_vol': struct/class members cannot be declared 'volatile'}} */
    //extern volatile const float2 f_ext_vol_con;             /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_vol_con': struct/class members cannot be declared 'extern'}} fxc-error {{X3008: 'f_ext_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3035: 'f_ext_vol_con': struct/class members cannot be declared 'const'}} */
    //extern const float2 f_ext_con;                          /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'f_ext_con': struct/class members cannot be declared 'extern'}} fxc-error {{X3035: 'f_ext_con': struct/class members cannot be declared 'const'}} */
    precise float2 f_pre;
    //precise static float2 f_pre_sta;                        /* expected-error {{'static' is not a valid modifier for a field}} fxc-error {{X3103: 's_storage_mods::f_pre_sta': variable declared but not defined}} */
    //precise static volatile float2 f_pre_sta_vol;           /* expected-error {{'static' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_pre_sta_vol': struct/class members cannot be declared 'volatile'}} */
    //precise static volatile const float2 f_pre_sta_vol_con; /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_pre_sta_vol_con': struct/class members cannot be declared 'volatile'}} */
    //precise static const float2 f_pre_sta_con;              /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} fxc-error {{X3103: 's_storage_mods::f_pre_sta_con': variable declared but not defined}} */
    //precise uniform float2 f_pre_uni;                       /* expected-error {{'uniform' is not a valid modifier for a field}} fxc-error {{X3047: 'f_pre_uni': struct/class members cannot be declared 'uniform'}} */
    //precise uniform volatile float2 f_pre_uni_vol;          /* expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_pre_uni_vol': struct/class members cannot be declared 'volatile'}} fxc-error {{X3047: 'f_pre_uni_vol': struct/class members cannot be declared 'uniform'}} */
    //precise uniform volatile const float2 f_pre_uni_vol_con;    /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_pre_uni_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3035: 'f_pre_uni_vol_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_pre_uni_vol_con': struct/class members cannot be declared 'uniform'}} */
    //precise uniform const float2 f_pre_uni_con;             /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} fxc-error {{X3035: 'f_pre_uni_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_pre_uni_con': struct/class members cannot be declared 'uniform'}} */
    //precise volatile float2 f_pre_vol;                      /* expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_pre_vol': struct/class members cannot be declared 'volatile'}} */
    //precise volatile const float2 f_pre_vol_con;            /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_pre_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3035: 'f_pre_vol_con': struct/class members cannot be declared 'const'}} */
    //precise const float2 f_pre_con;                         /* expected-error {{'const' is not a valid modifier for a field}} fxc-error {{X3035: 'f_pre_con': struct/class members cannot be declared 'const'}} */
    //static float2 f_sta;                                    /* expected-error {{'static' is not a valid modifier for a field}} fxc-error {{X3103: 's_storage_mods::f_sta': variable declared but not defined}} */
    //static volatile float2 f_sta_vol;                       /* expected-error {{'static' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_sta_vol': struct/class members cannot be declared 'volatile'}} */
    //static volatile const float2 f_sta_vol_con;             /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_sta_vol_con': struct/class members cannot be declared 'volatile'}} */
    //static const float2 f_sta_con;                          /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'static' is not a valid modifier for a field}} fxc-error {{X3103: 's_storage_mods::f_sta_con': variable declared but not defined}} */
    //uniform float2 f_uni;                                   /* expected-error {{'uniform' is not a valid modifier for a field}} fxc-error {{X3047: 'f_uni': struct/class members cannot be declared 'uniform'}} */
    //uniform volatile float2 f_uni_vol;                      /* expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_uni_vol': struct/class members cannot be declared 'volatile'}} fxc-error {{X3047: 'f_uni_vol': struct/class members cannot be declared 'uniform'}} */
    //uniform volatile const float2 f_uni_vol_con;            /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_uni_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3035: 'f_uni_vol_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_uni_vol_con': struct/class members cannot be declared 'uniform'}} */
    //uniform const float2 f_uni_con;                         /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'uniform' is not a valid modifier for a field}} fxc-error {{X3035: 'f_uni_con': struct/class members cannot be declared 'const'}} fxc-error {{X3047: 'f_uni_con': struct/class members cannot be declared 'uniform'}} */
    //volatile float2 f_vol;                                  /* expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_vol': struct/class members cannot be declared 'volatile'}} */
    //volatile const float2 f_vol_con;                        /* expected-error {{'const' is not a valid modifier for a field}} expected-error {{'volatile' is not a valid modifier for a field}} fxc-error {{X3008: 'f_vol_con': struct/class members cannot be declared 'volatile'}} fxc-error {{X3035: 'f_vol_con': struct/class members cannot be declared 'const'}} */
    //const float2 f_con;                                     /* expected-error {{'const' is not a valid modifier for a field}} fxc-error {{X3035: 'f_con': struct/class members cannot be declared 'const'}} */
    // GENERATED_CODE:END
};

// Interpolation modifiers
struct s_interp_mods {
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2 f_%(id)s;'))</py>
    // GENERATED_CODE:BEGIN
    nointerpolation float2 f_noi;
    //nointerpolation linear float2 f_noi_lin;                /* expected-error {{'nointerpolation' and 'linear' cannot be used together for a field}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
    //nointerpolation sample float2 f_noi_sam;                /* expected-error {{'nointerpolation' and 'sample' cannot be used together for a field}} fxc-pass {{}} */
    //nointerpolation noperspective float2 f_noi_nop;         /* expected-error {{'nointerpolation' and 'noperspective' cannot be used together for a field}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
    //nointerpolation centroid float2 f_noi_cen;              /* expected-error {{'nointerpolation' and 'centroid' cannot be used together for a field}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
    linear float2 f_lin;
    linear sample float2 f_lin_sam;
    linear sample noperspective float2 f_lin_sam_nop;
    //linear sample noperspective centroid float2 f_lin_sam_nop_cen;    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    //linear sample centroid float2 f_lin_sam_cen;            /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    linear noperspective float2 f_lin_nop;
    linear noperspective centroid float2 f_lin_nop_cen;
    linear centroid float2 f_lin_cen;
    sample float2 f_sam;
    sample noperspective float2 f_sam_nop;
    //sample noperspective centroid float2 f_sam_nop_cen;     /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    //sample centroid float2 f_sam_cen;                       /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    noperspective float2 f_nop;
    noperspective centroid float2 f_nop_cen;
    centroid float2 f_cen;
    // GENERATED_CODE:END
};

//struct s_inout {
//    in float2 f_in;                                         /* expected-error {{HLSL usage 'in' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'in'}} */
//    out float2 f_out;                                       /* expected-error {{HLSL usage 'out' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'out'}} */
//    inout float2 f_inout;                                   /* expected-error {{HLSL usage 'inout' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'inout'}} */
//};

//////////////////////////////////////////////////////////////////////////////
// Parameters
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('float4 foo_%(id)s(%(mods)s float4 val) { return val; }', storage_combos))</py>
// GENERATED_CODE:BEGIN
//float4 foo_gro_ext(groupshared extern float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_ext': function must return a value}} */
//float4 foo_ext_sta(extern static float4 val) { return val; }    /* expected-error {{cannot combine with previous 'extern' declaration specifier}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_sta': function must return a value}} */
//float4 foo_sta_uni(static uniform float4 val) { return val; }    /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'static'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_sta_uni': function must return a value}} */
//float4 foo_gro(groupshared float4 val) { return val; }      /* expected-error {{'groupshared' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro': function must return a value}} */
//float4 foo_gro_pre(groupshared precise float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre': function must return a value}} */
//float4 foo_gro_pre_sta(groupshared precise static float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_sta': function must return a value}} */
//float4 foo_gro_pre_sta_vol(groupshared precise static volatile float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_sta_vol': function must return a value}} */
//float4 foo_gro_pre_sta_vol_con(groupshared precise static volatile const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_sta_vol_con': function must return a value}} */
//float4 foo_gro_pre_sta_con(groupshared precise static const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_sta_con': function must return a value}} */
//float4 foo_gro_pre_uni(groupshared precise uniform float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_uni': function must return a value}} */
//float4 foo_gro_pre_uni_vol(groupshared precise uniform volatile float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_uni_vol': function must return a value}} */
//float4 foo_gro_pre_uni_vol_con(groupshared precise uniform volatile const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_uni_vol_con': function must return a value}} */
//float4 foo_gro_pre_uni_con(groupshared precise uniform const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_uni_con': function must return a value}} */
//float4 foo_gro_pre_vol(groupshared precise volatile float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_vol': function must return a value}} */
//float4 foo_gro_pre_vol_con(groupshared precise volatile const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_vol_con': function must return a value}} */
//float4 foo_gro_pre_con(groupshared precise const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_pre_con': function must return a value}} */
//float4 foo_gro_sta(groupshared static float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_sta': function must return a value}} */
//float4 foo_gro_sta_vol(groupshared static volatile float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_sta_vol': function must return a value}} */
//float4 foo_gro_sta_vol_con(groupshared static volatile const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_sta_vol_con': function must return a value}} */
//float4 foo_gro_sta_con(groupshared static const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_sta_con': function must return a value}} */
//float4 foo_gro_uni(groupshared uniform float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_uni': function must return a value}} */
//float4 foo_gro_uni_vol(groupshared uniform volatile float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_uni_vol': function must return a value}} */
//float4 foo_gro_uni_vol_con(groupshared uniform volatile const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_uni_vol_con': function must return a value}} */
//float4 foo_gro_uni_con(groupshared uniform const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_uni_con': function must return a value}} */
//float4 foo_gro_vol(groupshared volatile float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_vol': function must return a value}} */
//float4 foo_gro_vol_con(groupshared volatile const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_vol_con': function must return a value}} */
//float4 foo_gro_con(groupshared const float4 val) { return val; }    /* expected-error {{'groupshared' is not a valid modifier for a parameter}} fxc-error {{X3000: syntax error: unexpected token 'groupshared'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_gro_con': function must return a value}} */
//float4 foo_ext(extern float4 val) { return val; }           /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext': function must return a value}} */
//float4 foo_ext_pre(extern precise float4 val) { return val; }    /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_pre': function must return a value}} */
//float4 foo_ext_pre_uni(extern precise uniform float4 val) { return val; }    /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_pre_uni': function must return a value}} */
//float4 foo_ext_pre_uni_vol(extern precise uniform volatile float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_pre_uni_vol': function must return a value}} */
//float4 foo_ext_pre_uni_vol_con(extern precise uniform volatile const float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_pre_uni_vol_con': function must return a value}} */
//float4 foo_ext_pre_uni_con(extern precise uniform const float4 val) { return val; }    /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_pre_uni_con': function must return a value}} */
//float4 foo_ext_pre_vol(extern precise volatile float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_pre_vol': function must return a value}} */
//float4 foo_ext_pre_vol_con(extern precise volatile const float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_pre_vol_con': function must return a value}} */
//float4 foo_ext_pre_con(extern precise const float4 val) { return val; }    /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_pre_con': function must return a value}} */
//float4 foo_ext_uni(extern uniform float4 val) { return val; }    /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_uni': function must return a value}} */
//float4 foo_ext_uni_vol(extern uniform volatile float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_uni_vol': function must return a value}} */
//float4 foo_ext_uni_vol_con(extern uniform volatile const float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_uni_vol_con': function must return a value}} */
//float4 foo_ext_uni_con(extern uniform const float4 val) { return val; }    /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_uni_con': function must return a value}} */
//float4 foo_ext_vol(extern volatile float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_vol': function must return a value}} */
//float4 foo_ext_vol_con(extern volatile const float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_vol_con': function must return a value}} */
//float4 foo_ext_con(extern const float4 val) { return val; } /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'extern'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_ext_con': function must return a value}} */
float4 foo_pre(precise float4 val) { return val; }
//float4 foo_pre_sta(precise static float4 val) { return val; }    /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'static'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_pre_sta': function must return a value}} */
//float4 foo_pre_sta_vol(precise static volatile float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'static'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_pre_sta_vol': function must return a value}} */
//float4 foo_pre_sta_vol_con(precise static volatile const float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'static'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_pre_sta_vol_con': function must return a value}} */
//float4 foo_pre_sta_con(precise static const float4 val) { return val; }    /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'static'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_pre_sta_con': function must return a value}} */
float4 foo_pre_uni(precise uniform float4 val) { return val; }
//float4 foo_pre_uni_vol(precise uniform volatile float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-pass {{}} */
//float4 foo_pre_uni_vol_con(precise uniform volatile const float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-pass {{}} */
float4 foo_pre_uni_con(precise uniform const float4 val) { return val; }
//float4 foo_pre_vol(precise volatile float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-pass {{}} */
//float4 foo_pre_vol_con(precise volatile const float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-pass {{}} */
float4 foo_pre_con(precise const float4 val) { return val; }
//float4 foo_sta(static float4 val) { return val; }           /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'static'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_sta': function must return a value}} */
//float4 foo_sta_vol(static volatile float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'static'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_sta_vol': function must return a value}} */
//float4 foo_sta_vol_con(static volatile const float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'static'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_sta_vol_con': function must return a value}} */
//float4 foo_sta_con(static const float4 val) { return val; } /* expected-error {{invalid storage class specifier in function declarator}} fxc-error {{X3000: syntax error: unexpected token 'static'}} fxc-error {{X3004: undeclared identifier 'val'}} fxc-error {{X3080: 'foo_sta_con': function must return a value}} */
float4 foo_uni(uniform float4 val) { return val; }
//float4 foo_uni_vol(uniform volatile float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-pass {{}} */
//float4 foo_uni_vol_con(uniform volatile const float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-pass {{}} */
float4 foo_uni_con(uniform const float4 val) { return val; }
//float4 foo_vol(volatile float4 val) { return val; }         /* expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-pass {{}} */
//float4 foo_vol_con(volatile const float4 val) { return val; }    /* expected-error {{'volatile' is not a valid modifier for a parameter}} fxc-pass {{}} */
float4 foo_con(const float4 val) { return val; }
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('float4 foo_%(id)s(%(mods)s float4 val) { return val; }'))</py>
// GENERATED_CODE:BEGIN
float4 foo_noi(nointerpolation float4 val) { return val; }
//float4 foo_noi_lin(nointerpolation linear float4 val) { return val; }    /* expected-error {{'nointerpolation' and 'linear' cannot be used together for a parameter}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
//float4 foo_noi_sam(nointerpolation sample float4 val) { return val; }    /* expected-error {{'nointerpolation' and 'sample' cannot be used together for a parameter}} fxc-pass {{}} */
//float4 foo_noi_nop(nointerpolation noperspective float4 val) { return val; }    /* expected-error {{'nointerpolation' and 'noperspective' cannot be used together for a parameter}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
//float4 foo_noi_cen(nointerpolation centroid float4 val) { return val; }    /* expected-error {{'nointerpolation' and 'centroid' cannot be used together for a parameter}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
float4 foo_lin(linear float4 val) { return val; }
float4 foo_lin_sam(linear sample float4 val) { return val; }
float4 foo_lin_sam_nop(linear sample noperspective float4 val) { return val; }
//float4 foo_lin_sam_nop_cen(linear sample noperspective centroid float4 val) { return val; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
//float4 foo_lin_sam_cen(linear sample centroid float4 val) { return val; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
float4 foo_lin_nop(linear noperspective float4 val) { return val; }
float4 foo_lin_nop_cen(linear noperspective centroid float4 val) { return val; }
float4 foo_lin_cen(linear centroid float4 val) { return val; }
float4 foo_sam(sample float4 val) { return val; }
float4 foo_sam_nop(sample noperspective float4 val) { return val; }
//float4 foo_sam_nop_cen(sample noperspective centroid float4 val) { return val; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
//float4 foo_sam_cen(sample centroid float4 val) { return val; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
float4 foo_nop(noperspective float4 val) { return val; }
float4 foo_nop_cen(noperspective centroid float4 val) { return val; }
float4 foo_cen(centroid float4 val) { return val; }
// GENERATED_CODE:END

float4 foo_in(in float4 val) {
    return val;
}
float4 foo_out(out float4 val) {
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}
float4 foo_inout(inout float4 val) {
    float4 result = val;
    val = val + float4(1.0f,2.0f,3.0f,4.0f);
    return result;
}
//float4 foo_out2(float4 out val) {                           /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'out'}} */
//    val = float4(1.0f,2.0f,3.0f,4.0f);
//    return val;
//}
float4 foo_out3(out linear float4 val) {
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}
float4 foo_out4(linear out float4 val) {
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}
//float4 foo_out5(linear float4 out val) {                    /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'out'}} */
//    val = float4(1.0f,2.0f,3.0f,4.0f);
//    return val;
//}
float4 foo_out6(noperspective out linear float4 val) {
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}

// In is default, so this shouldn't matter:
float4 foo_in_missing_in_decl(float4 val);
float4 foo_in_missing_in_decl(in float4 val) {
    return val;
}
float4 foo_in_missing_in_def(in float4 val);
float4 foo_in_missing_in_def(float4 val) {
    return val;
}
//float4 foo_out_missing_in_decl(float4 val);                 /* expected-note {{candidate function}} fxc-pass {{}} */
//float4 foo_out_missing_in_decl(out float4 val) {            /* expected-note {{candidate function}} fxc-pass {{}} */
//    val = float4(1.0f,2.0f,3.0f,4.0f);
//    return val;
//}
//float4 foo_out_missing_in_def(out float4 val);              /* expected-note {{candidate function}} fxc-pass {{}} */
//float4 foo_out_missing_in_def(float4 val) {                 /* expected-note {{candidate function}} fxc-pass {{}} */
//    val = float4(1.0f,2.0f,3.0f,4.0f);
//    return val;
//}
//float4 foo_inout_missing_in_decl(float4 val);               /* expected-note {{candidate function}} fxc-pass {{}} */
//float4 foo_inout_missing_in_decl(inout float4 val) {        /* expected-note {{candidate function}} fxc-pass {{}} */
//    val = float4(1.0f,2.0f,3.0f,4.0f);
//    return val;
//}
//float4 foo_inout_missing_in_def(inout float4 val);          /* expected-note {{candidate function}} fxc-pass {{}} */
//float4 foo_inout_missing_in_def(float4 val) {               /* expected-note {{candidate function}} fxc-pass {{}} */
//    val = float4(1.0f,2.0f,3.0f,4.0f);
//    return val;
//}

float4 use_conflicting_inout(float4 val) {
    float4 out1, inout1, in1, res1;
    float4 out2, inout2, in2, res2;
    //inout1 = foo_out_missing_in_def(out1);                  /* expected-error {{call to 'foo_out_missing_in_def' is ambiguous}} fxc-error {{X3067: 'foo_out_missing_in_def': ambiguous function call}} */
    //inout2 = foo_out_missing_in_decl(out2);                 /* expected-error {{call to 'foo_out_missing_in_decl' is ambiguous}} fxc-error {{X3067: 'foo_out_missing_in_decl': ambiguous function call}} */
    //in1 = foo_inout_missing_in_def(out1);                   /* expected-error {{call to 'foo_inout_missing_in_def' is ambiguous}} fxc-error {{X3067: 'foo_inout_missing_in_def': ambiguous function call}} */
    //in2 = foo_inout_missing_in_decl(out2);                  /* expected-error {{call to 'foo_inout_missing_in_decl' is ambiguous}} fxc-error {{X3067: 'foo_inout_missing_in_decl': ambiguous function call}} */
    res1 = foo_in_missing_in_def(in1);
    res2 = foo_in_missing_in_decl(in2);
    return res1 + res2 + inout1 + inout2;
}

// Try interpolation modifiers in function decl, valid, invalid, and conflicting modifiers:
float4 foo_noi_decl(nointerpolation float4 val);
//float4 foo_noi_sam_decl(nointerpolation sample float4 val); /* expected-error {{'nointerpolation' and 'sample' cannot be used together for a parameter}} fxc-pass {{}} */
float4 foo_interpolation_different_decl(nointerpolation float4 val);
float4 foo_interpolation_different_decl(sample float4 val) {
    return val;
}

//////////////////////////////////////////////////////////////////////////////
// Locals.
void vain() {
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float l_%(id)s;', storage_combos))</py>
    // GENERATED_CODE:BEGIN
    //groupshared extern float l_gro_ext;                     /* expected-error {{'extern' and 'groupshared' cannot be used together for a local variable}} expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_gro_ext': local variables cannot be declared 'extern'}} fxc-error {{X3010: 'l_gro_ext': local variables cannot be declared 'groupshared'}} */
    //extern static float l_ext_sta;                          /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{cannot combine with previous 'extern' declaration specifier}} fxc-error {{X3006: 'l_ext_sta': local variables cannot be declared 'extern'}} */
    //static uniform float l_sta_uni;                         /* expected-error {{'static' and 'uniform' cannot be used together for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3047: 'l_sta_uni': local variables cannot be declared 'uniform'}} */
    //groupshared float l_gro;                                /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro': local variables cannot be declared 'groupshared'}} */
    //groupshared precise float l_gro_pre;                    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre': local variables cannot be declared 'groupshared'}} */
    //groupshared precise static float l_gro_pre_sta;         /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_sta': local variables cannot be declared 'groupshared'}} */
    //groupshared precise static volatile float l_gro_pre_sta_vol;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_sta_vol': local variables cannot be declared 'groupshared'}} */
    //groupshared precise static volatile const float l_gro_pre_sta_vol_con;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_sta_vol_con': local variables cannot be declared 'groupshared'}} */
    //groupshared precise static const float l_gro_pre_sta_con;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_sta_con': local variables cannot be declared 'groupshared'}} */
    //groupshared precise uniform float l_gro_pre_uni;        /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_uni': local variables cannot be declared 'groupshared'}} fxc-error {{X3047: 'l_gro_pre_uni': local variables cannot be declared 'uniform'}} */
    //groupshared precise uniform volatile float l_gro_pre_uni_vol;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_uni_vol': local variables cannot be declared 'groupshared'}} fxc-error {{X3047: 'l_gro_pre_uni_vol': local variables cannot be declared 'uniform'}} */
    //groupshared precise uniform volatile const float l_gro_pre_uni_vol_con;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_uni_vol_con': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_pre_uni_vol_con': missing initial value}} fxc-error {{X3047: 'l_gro_pre_uni_vol_con': local variables cannot be declared 'uniform'}} */
    //groupshared precise uniform const float l_gro_pre_uni_con;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_uni_con': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_pre_uni_con': missing initial value}} fxc-error {{X3047: 'l_gro_pre_uni_con': local variables cannot be declared 'uniform'}} */
    //groupshared precise volatile float l_gro_pre_vol;             /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_vol': local variables cannot be declared 'groupshared'}} */
    //groupshared precise volatile const float l_gro_pre_vol_con;   /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_vol_con': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_pre_vol_con': missing initial value}} */
    //groupshared precise const float l_gro_pre_con;          /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_con': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_pre_con': missing initial value}} */
    //groupshared static float l_gro_sta;                     /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_sta': local variables cannot be declared 'groupshared'}} */
    //groupshared static volatile float l_gro_sta_vol;        /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_sta_vol': local variables cannot be declared 'groupshared'}} */
    //groupshared static volatile const float l_gro_sta_vol_con;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_sta_vol_con': local variables cannot be declared 'groupshared'}} */
    //groupshared static const float l_gro_sta_con;           /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_sta_con': local variables cannot be declared 'groupshared'}} */
    //groupshared uniform float l_gro_uni;                    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_uni': local variables cannot be declared 'groupshared'}} fxc-error {{X3047: 'l_gro_uni': local variables cannot be declared 'uniform'}} */
    //groupshared uniform volatile float l_gro_uni_vol;       /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_uni_vol': local variables cannot be declared 'groupshared'}} fxc-error {{X3047: 'l_gro_uni_vol': local variables cannot be declared 'uniform'}} */
    //groupshared uniform volatile const float l_gro_uni_vol_con;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_uni_vol_con': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_uni_vol_con': missing initial value}} fxc-error {{X3047: 'l_gro_uni_vol_con': local variables cannot be declared 'uniform'}} */
    //groupshared uniform const float l_gro_uni_con;          /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_uni_con': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_uni_con': missing initial value}} fxc-error {{X3047: 'l_gro_uni_con': local variables cannot be declared 'uniform'}} */
    //groupshared volatile float l_gro_vol;                   /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_vol': local variables cannot be declared 'groupshared'}} */
    //groupshared volatile const float l_gro_vol_con;         /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_vol_con': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_vol_con': missing initial value}} */
    //groupshared const float l_gro_con;                      /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_con': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_con': missing initial value}} */
    //extern float l_ext;                                     /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext': local variables cannot be declared 'extern'}} */
    //extern precise float l_ext_pre;                         /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre': local variables cannot be declared 'extern'}} */
    //extern precise uniform float l_ext_pre_uni;             /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_uni': local variables cannot be declared 'extern'}} fxc-error {{X3047: 'l_ext_pre_uni': local variables cannot be declared 'uniform'}} */
    //extern precise uniform volatile float l_ext_pre_uni_vol;    /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_uni_vol': local variables cannot be declared 'extern'}} fxc-error {{X3047: 'l_ext_pre_uni_vol': local variables cannot be declared 'uniform'}} */
    //extern precise uniform volatile const float l_ext_pre_uni_vol_con;    /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_uni_vol_con': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_pre_uni_vol_con': missing initial value}} fxc-error {{X3047: 'l_ext_pre_uni_vol_con': local variables cannot be declared 'uniform'}} */
    //extern precise uniform const float l_ext_pre_uni_con;   /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_uni_con': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_pre_uni_con': missing initial value}} fxc-error {{X3047: 'l_ext_pre_uni_con': local variables cannot be declared 'uniform'}} */
    //extern precise volatile float l_ext_pre_vol;            /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_vol': local variables cannot be declared 'extern'}} */
    //extern precise volatile const float l_ext_pre_vol_con;  /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_vol_con': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_pre_vol_con': missing initial value}} */
    //extern precise const float l_ext_pre_con;               /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_con': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_pre_con': missing initial value}} */
    //extern uniform float l_ext_uni;                         /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_uni': local variables cannot be declared 'extern'}} fxc-error {{X3047: 'l_ext_uni': local variables cannot be declared 'uniform'}} */
    //extern uniform volatile float l_ext_uni_vol;            /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_uni_vol': local variables cannot be declared 'extern'}} fxc-error {{X3047: 'l_ext_uni_vol': local variables cannot be declared 'uniform'}} */
    //extern uniform volatile const float l_ext_uni_vol_con;  /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_uni_vol_con': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_uni_vol_con': missing initial value}} fxc-error {{X3047: 'l_ext_uni_vol_con': local variables cannot be declared 'uniform'}} */
    //extern uniform const float l_ext_uni_con;               /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_uni_con': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_uni_con': missing initial value}} fxc-error {{X3047: 'l_ext_uni_con': local variables cannot be declared 'uniform'}} */
    //extern volatile float l_ext_vol;                        /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_vol': local variables cannot be declared 'extern'}} */
    //extern volatile const float l_ext_vol_con;              /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_vol_con': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_vol_con': missing initial value}} */
    //extern const float l_ext_con;                           /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_con': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_con': missing initial value}} */
    precise float l_pre;
    precise static float l_pre_sta;
    precise static volatile float l_pre_sta_vol;
    precise static volatile const float l_pre_sta_vol_con;
    precise static const float l_pre_sta_con;
    //precise uniform float l_pre_uni;                        /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3047: 'l_pre_uni': local variables cannot be declared 'uniform'}} */
    //precise uniform volatile float l_pre_uni_vol;           /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3047: 'l_pre_uni_vol': local variables cannot be declared 'uniform'}} */
    //precise uniform volatile const float l_pre_uni_vol_con; /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3012: 'l_pre_uni_vol_con': missing initial value}} fxc-error {{X3047: 'l_pre_uni_vol_con': local variables cannot be declared 'uniform'}} */
    //precise uniform const float l_pre_uni_con;              /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3012: 'l_pre_uni_con': missing initial value}} fxc-error {{X3047: 'l_pre_uni_con': local variables cannot be declared 'uniform'}} */
    precise volatile float l_pre_vol;
    //precise volatile const float l_pre_vol_con;             /* fxc-error {{X3012: 'l_pre_vol_con': missing initial value}} */
    //precise const float l_pre_con;                          /* fxc-error {{X3012: 'l_pre_con': missing initial value}} */
    static float l_sta;
    static volatile float l_sta_vol;
    static volatile const float l_sta_vol_con;
    static const float l_sta_con;
    //uniform float l_uni;                                    /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3047: 'l_uni': local variables cannot be declared 'uniform'}} */
    //uniform volatile float l_uni_vol;                       /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3047: 'l_uni_vol': local variables cannot be declared 'uniform'}} */
    //uniform volatile const float l_uni_vol_con;             /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3012: 'l_uni_vol_con': missing initial value}} fxc-error {{X3047: 'l_uni_vol_con': local variables cannot be declared 'uniform'}} */
    //uniform const float l_uni_con;                          /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3012: 'l_uni_con': missing initial value}} fxc-error {{X3047: 'l_uni_con': local variables cannot be declared 'uniform'}} */
    volatile float l_vol;
    //volatile const float l_vol_con;                         /* fxc-error {{X3012: 'l_vol_con': missing initial value}} */
    //const float l_con;                                      /* fxc-error {{X3012: 'l_con': missing initial value}} */
    // GENERATED_CODE:END
    // Now with const vars initialized:
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float l_%(id)s_init = 0.0;', filter(lambda combo: 'const' in combo, storage_combos)))</py>
    // GENERATED_CODE:BEGIN
    //groupshared precise static volatile const float l_gro_pre_sta_vol_con_init = 0.0;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_sta_vol_con_init': local variables cannot be declared 'groupshared'}} */
    //groupshared precise static const float l_gro_pre_sta_con_init = 0.0;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_sta_con_init': local variables cannot be declared 'groupshared'}} */
    //groupshared precise uniform volatile const float l_gro_pre_uni_vol_con_init = 0.0;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_uni_vol_con_init': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_pre_uni_vol_con_init': missing initial value}} fxc-error {{X3047: 'l_gro_pre_uni_vol_con_init': local variables cannot be declared 'uniform'}} */
    //groupshared precise uniform const float l_gro_pre_uni_con_init = 0.0;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_uni_con_init': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_pre_uni_con_init': missing initial value}} fxc-error {{X3047: 'l_gro_pre_uni_con_init': local variables cannot be declared 'uniform'}} */
    //groupshared precise volatile const float l_gro_pre_vol_con_init = 0.0;   /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_vol_con_init': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_pre_vol_con_init': missing initial value}} */
    //groupshared precise const float l_gro_pre_con_init = 0.0;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_pre_con_init': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_pre_con_init': missing initial value}} */
    //groupshared static volatile const float l_gro_sta_vol_con_init = 0.0;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_sta_vol_con_init': local variables cannot be declared 'groupshared'}} */
    //groupshared static const float l_gro_sta_con_init = 0.0;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_sta_con_init': local variables cannot be declared 'groupshared'}} */
    //groupshared uniform volatile const float l_gro_uni_vol_con_init = 0.0;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_uni_vol_con_init': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_uni_vol_con_init': missing initial value}} fxc-error {{X3047: 'l_gro_uni_vol_con_init': local variables cannot be declared 'uniform'}} */
    //groupshared uniform const float l_gro_uni_con_init = 0.0;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_uni_con_init': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_uni_con_init': missing initial value}} fxc-error {{X3047: 'l_gro_uni_con_init': local variables cannot be declared 'uniform'}} */
    //groupshared volatile const float l_gro_vol_con_init = 0.0;   /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_vol_con_init': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_vol_con_init': missing initial value}} */
    //groupshared const float l_gro_con_init = 0.0;           /* expected-error {{'groupshared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'l_gro_con_init': local variables cannot be declared 'groupshared'}} fxc-error {{X3012: 'l_gro_con_init': missing initial value}} */
    //extern precise uniform volatile const float l_ext_pre_uni_vol_con_init = 0.0;    /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_uni_vol_con_init': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_pre_uni_vol_con_init': missing initial value}} fxc-error {{X3047: 'l_ext_pre_uni_vol_con_init': local variables cannot be declared 'uniform'}} */
    //extern precise uniform const float l_ext_pre_uni_con_init = 0.0;    /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_uni_con_init': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_pre_uni_con_init': missing initial value}} fxc-error {{X3047: 'l_ext_pre_uni_con_init': local variables cannot be declared 'uniform'}} */
    //extern precise volatile const float l_ext_pre_vol_con_init = 0.0;   /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_vol_con_init': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_pre_vol_con_init': missing initial value}} */
    //extern precise const float l_ext_pre_con_init = 0.0;    /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_pre_con_init': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_pre_con_init': missing initial value}} */
    //extern uniform volatile const float l_ext_uni_vol_con_init = 0.0;    /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_uni_vol_con_init': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_uni_vol_con_init': missing initial value}} fxc-error {{X3047: 'l_ext_uni_vol_con_init': local variables cannot be declared 'uniform'}} */
    //extern uniform const float l_ext_uni_con_init = 0.0;    /* expected-error {{'extern' is not a valid modifier for a local variable}} expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_uni_con_init': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_uni_con_init': missing initial value}} fxc-error {{X3047: 'l_ext_uni_con_init': local variables cannot be declared 'uniform'}} */
    //extern volatile const float l_ext_vol_con_init = 0.0;   /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_vol_con_init': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_vol_con_init': missing initial value}} */
    //extern const float l_ext_con_init = 0.0;                /* expected-error {{'extern' is not a valid modifier for a local variable}} fxc-error {{X3006: 'l_ext_con_init': local variables cannot be declared 'extern'}} fxc-error {{X3012: 'l_ext_con_init': missing initial value}} */
    precise static volatile const float l_pre_sta_vol_con_init = 0.0;
    precise static const float l_pre_sta_con_init = 0.0;
    //precise uniform volatile const float l_pre_uni_vol_con_init = 0.0;    /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3012: 'l_pre_uni_vol_con_init': missing initial value}} fxc-error {{X3047: 'l_pre_uni_vol_con_init': local variables cannot be declared 'uniform'}} */
    //precise uniform const float l_pre_uni_con_init = 0.0;   /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3012: 'l_pre_uni_con_init': missing initial value}} fxc-error {{X3047: 'l_pre_uni_con_init': local variables cannot be declared 'uniform'}} */
    precise volatile const float l_pre_vol_con_init = 0.0;
    precise const float l_pre_con_init = 0.0;
    static volatile const float l_sta_vol_con_init = 0.0;
    static const float l_sta_con_init = 0.0;
    //uniform volatile const float l_uni_vol_con_init = 0.0;  /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3012: 'l_uni_vol_con_init': missing initial value}} fxc-error {{X3047: 'l_uni_vol_con_init': local variables cannot be declared 'uniform'}} */
    //uniform const float l_uni_con_init = 0.0;               /* expected-error {{'uniform' is not a valid modifier for a local variable}} fxc-error {{X3012: 'l_uni_con_init': missing initial value}} fxc-error {{X3047: 'l_uni_con_init': local variables cannot be declared 'uniform'}} */
    volatile const float l_vol_con_init = 0.0;
    const float l_con_init = 0.0;
    // GENERATED_CODE:END

    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float3 l_%(id)s;'))</py>
    // GENERATED_CODE:BEGIN
    //nointerpolation float3 l_noi;                           /* expected-error {{'nointerpolation' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //nointerpolation linear float3 l_noi_lin;                /* expected-error {{'linear' is not a valid modifier for a local variable}} expected-error {{'nointerpolation' and 'linear' cannot be used together for a local variable}} expected-error {{'nointerpolation' is not a valid modifier for a local variable}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
    //nointerpolation sample float3 l_noi_sam;                /* expected-error {{'nointerpolation' and 'sample' cannot be used together for a local variable}} expected-error {{'nointerpolation' is not a valid modifier for a local variable}} expected-error {{'sample' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //nointerpolation noperspective float3 l_noi_nop;         /* expected-error {{'nointerpolation' and 'noperspective' cannot be used together for a local variable}} expected-error {{'nointerpolation' is not a valid modifier for a local variable}} expected-error {{'noperspective' is not a valid modifier for a local variable}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
    //nointerpolation centroid float3 l_noi_cen;              /* expected-error {{'centroid' is not a valid modifier for a local variable}} expected-error {{'nointerpolation' and 'centroid' cannot be used together for a local variable}} expected-error {{'nointerpolation' is not a valid modifier for a local variable}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
    //linear float3 l_lin;                                    /* expected-error {{'linear' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //linear sample float3 l_lin_sam;                         /* expected-error {{'linear' is not a valid modifier for a local variable}} expected-error {{'sample' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //linear sample noperspective float3 l_lin_sam_nop;       /* expected-error {{'linear' is not a valid modifier for a local variable}} expected-error {{'noperspective' is not a valid modifier for a local variable}} expected-error {{'sample' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //linear sample noperspective centroid float3 l_lin_sam_nop_cen;    /* expected-error {{'centroid' is not a valid modifier for a local variable}} expected-error {{'linear' is not a valid modifier for a local variable}} expected-error {{'noperspective' is not a valid modifier for a local variable}} expected-error {{'sample' is not a valid modifier for a local variable}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    //linear sample centroid float3 l_lin_sam_cen;            /* expected-error {{'centroid' is not a valid modifier for a local variable}} expected-error {{'linear' is not a valid modifier for a local variable}} expected-error {{'sample' is not a valid modifier for a local variable}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    //linear noperspective float3 l_lin_nop;                  /* expected-error {{'linear' is not a valid modifier for a local variable}} expected-error {{'noperspective' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //linear noperspective centroid float3 l_lin_nop_cen;     /* expected-error {{'centroid' is not a valid modifier for a local variable}} expected-error {{'linear' is not a valid modifier for a local variable}} expected-error {{'noperspective' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //linear centroid float3 l_lin_cen;                       /* expected-error {{'centroid' is not a valid modifier for a local variable}} expected-error {{'linear' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //sample float3 l_sam;                                    /* expected-error {{'sample' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //sample noperspective float3 l_sam_nop;                  /* expected-error {{'noperspective' is not a valid modifier for a local variable}} expected-error {{'sample' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //sample noperspective centroid float3 l_sam_nop_cen;     /* expected-error {{'centroid' is not a valid modifier for a local variable}} expected-error {{'noperspective' is not a valid modifier for a local variable}} expected-error {{'sample' is not a valid modifier for a local variable}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    //sample centroid float3 l_sam_cen;                       /* expected-error {{'centroid' is not a valid modifier for a local variable}} expected-error {{'sample' is not a valid modifier for a local variable}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    //noperspective float3 l_nop;                             /* expected-error {{'noperspective' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //noperspective centroid float3 l_nop_cen;                /* expected-error {{'centroid' is not a valid modifier for a local variable}} expected-error {{'noperspective' is not a valid modifier for a local variable}} fxc-pass {{}} */
    //centroid float3 l_cen;                                  /* expected-error {{'centroid' is not a valid modifier for a local variable}} fxc-pass {{}} */
    // GENERATED_CODE:END

    //nointerpolation precise shared groupshared float3 f8;   /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'nointerpolation' is not a valid modifier for a local variable}} expected-error {{'shared' is not a valid modifier for a local variable}} fxc-error {{X3010: 'f8': local variables cannot be declared 'groupshared'}} fxc-error {{X3054: 'f8': local variables cannot be declared 'shared'}} */

    //// no type:
    //static precise shared groupshared nointerpolation f9;   /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'nointerpolation' is not a valid modifier for a local variable}} expected-error {{'shared' is not a valid modifier for a local variable}} expected-error {{'static' and 'shared' cannot be used together for a local variable}} expected-error {{HLSL requires a type specifier for all declarations}} fxc-error {{X3000: unrecognized identifier 'f9'}} */
    
    //static precise shared groupshared nointerpolation float2 f9_no_init;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'nointerpolation' is not a valid modifier for a local variable}} expected-error {{'shared' is not a valid modifier for a local variable}} expected-error {{'static' and 'shared' cannot be used together for a local variable}} fxc-error {{X3010: 'f9_no_init': local variables cannot be declared 'groupshared'}} fxc-error {{X3054: 'f9_no_init': local variables cannot be declared 'shared'}} */
    //static precise shared groupshared nointerpolation float f9_init = 0.1;    /* expected-error {{'groupshared' is not a valid modifier for a local variable}} expected-error {{'nointerpolation' is not a valid modifier for a local variable}} expected-error {{'shared' is not a valid modifier for a local variable}} expected-error {{'static' and 'shared' cannot be used together for a local variable}} fxc-error {{X3010: 'f9_init': local variables cannot be declared 'groupshared'}} fxc-error {{X3054: 'f9_init': local variables cannot be declared 'shared'}} */

    //in float g_in;                                              /* expected-error {{HLSL usage 'in' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'in'}} */
    //inout float g_inout;                                        /* expected-error {{HLSL usage 'inout' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'inout'}} */
    //out float g_out;                                            /* expected-error {{HLSL usage 'out' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'out'}} */
    //float inout g_inoutAfterType;                               /* expected-error {{modifiers must appear before type}} fxc-error {{X3000: syntax error: unexpected token 'inout'}} */
    //in out float g_in;                                          /* expected-error {{HLSL usage 'in' is only valid on a parameter}} expected-error {{HLSL usage 'out' is only valid on a parameter}} expected-error {{only one usage is allowed on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'in'}} */

}

//////////////////////////////////////////////////////////////////////////////
// Functions.
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float fn_%(id)s() { return 1.0f; }', storage_combos))</py>
// GENERATED_CODE:BEGIN
//groupshared extern float fn_gro_ext() { return 1.0f; }          /* expected-error {{'extern' and 'groupshared' cannot be used together for a function}} expected-error {{'groupshared' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_gro_ext': functions cannot be declared 'extern'}} */
//extern static float fn_ext_sta() { return 1.0f; }               /* expected-error {{'extern' is not a valid modifier for a function}} expected-error {{cannot combine with previous 'extern' declaration specifier}} fxc-error {{X3006: 'fn_ext_sta': functions cannot be declared 'extern'}} */
//static uniform float fn_sta_uni() { return 1.0f; }              /* expected-error {{'static' and 'uniform' cannot be used together for a function}} expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_sta_uni': functions cannot be declared 'uniform'}} */
//groupshared float fn_gro() { return 1.0f; }                     /* expected-error {{'groupshared' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared precise float fn_gro_pre() { return 1.0f; }         /* expected-error {{'groupshared' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared precise static float fn_gro_pre_sta() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared precise static volatile float fn_gro_pre_sta_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared precise static volatile const float fn_gro_pre_sta_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3025: l-value specifies const object}} */
//groupshared precise static const float fn_gro_pre_sta_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} fxc-error {{X3025: l-value specifies const object}} */
//groupshared precise uniform float fn_gro_pre_uni() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_gro_pre_uni': functions cannot be declared 'uniform'}} */
//groupshared precise uniform volatile float fn_gro_pre_uni_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_gro_pre_uni_vol': functions cannot be declared 'uniform'}} */
//groupshared precise uniform volatile const float fn_gro_pre_uni_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_gro_pre_uni_vol_con': functions cannot be declared 'uniform'}} */
//groupshared precise uniform const float fn_gro_pre_uni_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_gro_pre_uni_con': functions cannot be declared 'uniform'}} */
//groupshared precise volatile float fn_gro_pre_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared precise volatile const float fn_gro_pre_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3025: l-value specifies const object}} */
//groupshared precise const float fn_gro_pre_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} fxc-error {{X3025: l-value specifies const object}} */
//groupshared static float fn_gro_sta() { return 1.0f; }          /* expected-error {{'groupshared' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared static volatile float fn_gro_sta_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared static volatile const float fn_gro_sta_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared static const float fn_gro_sta_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared uniform float fn_gro_uni() { return 1.0f; }         /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_gro_uni': functions cannot be declared 'uniform'}} */
//groupshared uniform volatile float fn_gro_uni_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_gro_uni_vol': functions cannot be declared 'uniform'}} */
//groupshared uniform volatile const float fn_gro_uni_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_gro_uni_vol_con': functions cannot be declared 'uniform'}} */
//groupshared uniform const float fn_gro_uni_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_gro_uni_con': functions cannot be declared 'uniform'}} */
//groupshared volatile float fn_gro_vol() { return 1.0f; }        /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared volatile const float fn_gro_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
//groupshared const float fn_gro_con() { return 1.0f; }           /* expected-error {{'groupshared' is not a valid modifier for a function}} fxc-pass {{}} */
//extern float fn_ext() { return 1.0f; }                          /* expected-error {{'extern' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext': functions cannot be declared 'extern'}} */
//extern precise float fn_ext_pre() { return 1.0f; }              /* expected-error {{'extern' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_pre': functions cannot be declared 'extern'}} */
//extern precise uniform float fn_ext_pre_uni() { return 1.0f; }  /* expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_pre_uni': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_pre_uni': functions cannot be declared 'uniform'}} */
//extern precise uniform volatile float fn_ext_pre_uni_vol() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_pre_uni_vol': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_pre_uni_vol': functions cannot be declared 'uniform'}} */
//extern precise uniform volatile const float fn_ext_pre_uni_vol_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_pre_uni_vol_con': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_pre_uni_vol_con': functions cannot be declared 'uniform'}} */
//extern precise uniform const float fn_ext_pre_uni_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_pre_uni_con': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_pre_uni_con': functions cannot be declared 'uniform'}} */
//extern precise volatile float fn_ext_pre_vol() { return 1.0f; } /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_pre_vol': functions cannot be declared 'extern'}} */
//extern precise volatile const float fn_ext_pre_vol_con() { return 1.0f; }    /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_pre_vol_con': functions cannot be declared 'extern'}} */
//extern precise const float fn_ext_pre_con() { return 1.0f; }    /* expected-error {{'extern' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_pre_con': functions cannot be declared 'extern'}} */
//extern uniform float fn_ext_uni() { return 1.0f; }              /* expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_uni': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_uni': functions cannot be declared 'uniform'}} */
//extern uniform volatile float fn_ext_uni_vol() { return 1.0f; } /* expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_uni_vol': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_uni_vol': functions cannot be declared 'uniform'}} */
//extern uniform volatile const float fn_ext_uni_vol_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_uni_vol_con': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_uni_vol_con': functions cannot be declared 'uniform'}} */
//extern uniform const float fn_ext_uni_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_uni_con': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_uni_con': functions cannot be declared 'uniform'}} */
//extern volatile float fn_ext_vol() { return 1.0f; }             /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_vol': functions cannot be declared 'extern'}} */
//extern volatile const float fn_ext_vol_con() { return 1.0f; }   /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_vol_con': functions cannot be declared 'extern'}} */
//extern const float fn_ext_con() { return 1.0f; }                /* expected-error {{'extern' is not a valid modifier for a function}} fxc-error {{X3006: 'fn_ext_con': functions cannot be declared 'extern'}} */
precise float fn_pre() { return 1.0f; }
//precise static float fn_pre_sta() { return 1.0f; }
//precise static volatile float fn_pre_sta_vol() { return 1.0f; } /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
//precise static volatile const float fn_pre_sta_vol_con() { return 1.0f; }    /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3025: l-value specifies const object}} */
//precise static const float fn_pre_sta_con() { return 1.0f; }                 /* fxc-error {{X3025: l-value specifies const object}} */
//precise uniform float fn_pre_uni() { return 1.0f; }             /* expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_pre_uni': functions cannot be declared 'uniform'}} */
//precise uniform volatile float fn_pre_uni_vol() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_pre_uni_vol': functions cannot be declared 'uniform'}} */
//precise uniform volatile const float fn_pre_uni_vol_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_pre_uni_vol_con': functions cannot be declared 'uniform'}} */
//precise uniform const float fn_pre_uni_con() { return 1.0f; }   /* expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_pre_uni_con': functions cannot be declared 'uniform'}} */
//precise volatile float fn_pre_vol() { return 1.0f; }            /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
//precise volatile const float fn_pre_vol_con() { return 1.0f; }  /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3025: l-value specifies const object}} */
//precise const float fn_pre_con() { return 1.0f; }               /* fxc-error {{X3025: l-value specifies const object}} */
static float fn_sta() { return 1.0f; }
//static volatile float fn_sta_vol() { return 1.0f; }             /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
//static volatile const float fn_sta_vol_con() { return 1.0f; }   /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
static const float fn_sta_con() { return 1.0f; }
//uniform float fn_uni() { return 1.0f; }                         /* expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_uni': functions cannot be declared 'uniform'}} */
//uniform volatile float fn_uni_vol() { return 1.0f; }            /* expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_uni_vol': functions cannot be declared 'uniform'}} */
//uniform volatile const float fn_uni_vol_con() { return 1.0f; }  /* expected-error {{'uniform' is not a valid modifier for a function}} expected-error {{'volatile' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_uni_vol_con': functions cannot be declared 'uniform'}} */
//uniform const float fn_uni_con() { return 1.0f; }               /* expected-error {{'uniform' is not a valid modifier for a function}} fxc-error {{X3047: 'fn_uni_con': functions cannot be declared 'uniform'}} */
//volatile float fn_vol() { return 1.0f; }                        /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
//volatile const float fn_vol_con() { return 1.0f; }              /* expected-error {{'volatile' is not a valid modifier for a function}} fxc-pass {{}} */
const float fn_con() { return 1.0f; }
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float fn_%(id)s() { return 1.0f; }'))</py>
// GENERATED_CODE:BEGIN
nointerpolation float fn_noi() { return 1.0f; }
//nointerpolation linear float fn_noi_lin() { return 1.0f; }      /* expected-error {{'nointerpolation' and 'linear' cannot be used together for a function}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
//nointerpolation sample float fn_noi_sam() { return 1.0f; }      /* expected-error {{'nointerpolation' and 'sample' cannot be used together for a function}} fxc-pass {{}} */
//nointerpolation noperspective float fn_noi_nop() { return 1.0f; }    /* expected-error {{'nointerpolation' and 'noperspective' cannot be used together for a function}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
//nointerpolation centroid float fn_noi_cen() { return 1.0f; }    /* expected-error {{'nointerpolation' and 'centroid' cannot be used together for a function}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
linear float fn_lin() { return 1.0f; }
linear sample float fn_lin_sam() { return 1.0f; }
linear sample noperspective float fn_lin_sam_nop() { return 1.0f; }
//linear sample noperspective centroid float fn_lin_sam_nop_cen() { return 1.0f; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
//linear sample centroid float fn_lin_sam_cen() { return 1.0f; }  /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
linear noperspective float fn_lin_nop() { return 1.0f; }
linear noperspective centroid float fn_lin_nop_cen() { return 1.0f; }
linear centroid float fn_lin_cen() { return 1.0f; }
sample float fn_sam() { return 1.0f; }
sample noperspective float fn_sam_nop() { return 1.0f; }
//sample noperspective centroid float fn_sam_nop_cen() { return 1.0f; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
//sample centroid float fn_sam_cen() { return 1.0f; }             /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
noperspective float fn_nop() { return 1.0f; }
noperspective centroid float fn_nop_cen() { return 1.0f; }
centroid float fn_cen() { return 1.0f; }
// GENERATED_CODE:END

//in float fn_in() { return 1.0f; }                           /* expected-error {{HLSL usage 'in' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'in'}} */
//out float fn_out() { return 1.0f; }                         /* expected-error {{HLSL usage 'out' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'out'}} */
//inout float fn_inout() { return 1.0f; }                     /* expected-error {{HLSL usage 'inout' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'inout'}} */

//////////////////////////////////////////////////////////////////////////////
// Methods.
class C
{
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float fn_%(id)s() { return 1.0f; }', storage_combos))</py>
    // GENERATED_CODE:BEGIN
    //groupshared extern float fn_gro_ext() { return 1.0f; }  /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_gro_ext': functions cannot be declared 'extern'}} */
    //extern static float fn_ext_sta() { return 1.0f; }       /* expected-error {{cannot combine with previous 'extern' declaration specifier}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_sta': functions cannot be declared 'extern'}} */
    //static uniform float fn_sta_uni() { return 1.0f; }      /* expected-error {{'static' and 'uniform' cannot be used together for a method}} expected-error {{'uniform' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_sta_uni': functions cannot be declared 'uniform'}} */
    //groupshared float fn_gro() { return 1.0f; }             /* expected-error {{'groupshared' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared precise float fn_gro_pre() { return 1.0f; } /* expected-error {{'groupshared' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared precise static float fn_gro_pre_sta() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared precise static volatile float fn_gro_pre_sta_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared precise static volatile const float fn_gro_pre_sta_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3025: l-value specifies const object}} */
    //groupshared precise static const float fn_gro_pre_sta_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} fxc-error {{X3025: l-value specifies const object}} */
    //groupshared precise uniform float fn_gro_pre_uni() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'uniform' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_gro_pre_uni': functions cannot be declared 'uniform'}} */
    //groupshared precise uniform volatile float fn_gro_pre_uni_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_gro_pre_uni_vol': functions cannot be declared 'uniform'}} */
    //groupshared precise uniform volatile const float fn_gro_pre_uni_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_gro_pre_uni_vol_con': functions cannot be declared 'uniform'}} */
    //groupshared precise uniform const float fn_gro_pre_uni_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'uniform' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_gro_pre_uni_con': functions cannot be declared 'uniform'}} */
    //groupshared precise volatile float fn_gro_pre_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared precise volatile const float fn_gro_pre_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3025: l-value specifies const object}} */
    //groupshared precise const float fn_gro_pre_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} fxc-error {{X3025: l-value specifies const object}} */
    //groupshared static float fn_gro_sta() { return 1.0f; }  /* expected-error {{'groupshared' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared static volatile float fn_gro_sta_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared static volatile const float fn_gro_sta_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared static const float fn_gro_sta_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared uniform float fn_gro_uni() { return 1.0f; } /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'uniform' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_gro_uni': functions cannot be declared 'uniform'}} */
    //groupshared uniform volatile float fn_gro_uni_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_gro_uni_vol': functions cannot be declared 'uniform'}} */
    //groupshared uniform volatile const float fn_gro_uni_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_gro_uni_vol_con': functions cannot be declared 'uniform'}} */
    //groupshared uniform const float fn_gro_uni_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'uniform' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_gro_uni_con': functions cannot be declared 'uniform'}} */
    //groupshared volatile float fn_gro_vol() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared volatile const float fn_gro_vol_con() { return 1.0f; }    /* expected-error {{'groupshared' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    //groupshared const float fn_gro_con() { return 1.0f; }   /* expected-error {{'groupshared' is not a valid modifier for a method}} fxc-pass {{}} */
    //extern float fn_ext() { return 1.0f; }                  /* expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext': functions cannot be declared 'extern'}} */
    //extern precise float fn_ext_pre() { return 1.0f; }      /* expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_pre': functions cannot be declared 'extern'}} */
    //extern precise uniform float fn_ext_pre_uni() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_pre_uni': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_pre_uni': functions cannot be declared 'uniform'}} */
    //extern precise uniform volatile float fn_ext_pre_uni_vol() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_pre_uni_vol': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_pre_uni_vol': functions cannot be declared 'uniform'}} */
    //extern precise uniform volatile const float fn_ext_pre_uni_vol_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_pre_uni_vol_con': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_pre_uni_vol_con': functions cannot be declared 'uniform'}} */
    //extern precise uniform const float fn_ext_pre_uni_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_pre_uni_con': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_pre_uni_con': functions cannot be declared 'uniform'}} */
    //extern precise volatile float fn_ext_pre_vol() { return 1.0f; }    /* expected-error {{'volatile' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_pre_vol': functions cannot be declared 'extern'}} */
    //extern precise volatile const float fn_ext_pre_vol_con() { return 1.0f; }    /* expected-error {{'volatile' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_pre_vol_con': functions cannot be declared 'extern'}} */
    //extern precise const float fn_ext_pre_con() { return 1.0f; }    /* expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_pre_con': functions cannot be declared 'extern'}} */
    //extern uniform float fn_ext_uni() { return 1.0f; }      /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_uni': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_uni': functions cannot be declared 'uniform'}} */
    //extern uniform volatile float fn_ext_uni_vol() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_uni_vol': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_uni_vol': functions cannot be declared 'uniform'}} */
    //extern uniform volatile const float fn_ext_uni_vol_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_uni_vol_con': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_uni_vol_con': functions cannot be declared 'uniform'}} */
    //extern uniform const float fn_ext_uni_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_uni_con': functions cannot be declared 'extern'}} fxc-error {{X3047: 'fn_ext_uni_con': functions cannot be declared 'uniform'}} */
    //extern volatile float fn_ext_vol() { return 1.0f; }     /* expected-error {{'volatile' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_vol': functions cannot be declared 'extern'}} */
    //extern volatile const float fn_ext_vol_con() { return 1.0f; }    /* expected-error {{'volatile' is not a valid modifier for a method}} expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_vol_con': functions cannot be declared 'extern'}} */
    //extern const float fn_ext_con() { return 1.0f; }        /* expected-error {{storage class specified for a member declaration}} fxc-error {{X3006: 'fn_ext_con': functions cannot be declared 'extern'}} */
    precise float fn_pre() { return 1.0f; }
    precise static float fn_pre_sta() { return 1.0f; }
    //precise static volatile float fn_pre_sta_vol() { return 1.0f; }    /* expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    //precise static volatile const float fn_pre_sta_vol_con() { return 1.0f; }    /* expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3025: l-value specifies const object}} */
    //precise static const float fn_pre_sta_con() { return 1.0f; }                 /* fxc-error {{X3025: l-value specifies const object}} */
    //precise uniform float fn_pre_uni() { return 1.0f; }     /* expected-error {{'uniform' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_pre_uni': functions cannot be declared 'uniform'}} */
    //precise uniform volatile float fn_pre_uni_vol() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_pre_uni_vol': functions cannot be declared 'uniform'}} */
    //precise uniform volatile const float fn_pre_uni_vol_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_pre_uni_vol_con': functions cannot be declared 'uniform'}} */
    //precise uniform const float fn_pre_uni_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_pre_uni_con': functions cannot be declared 'uniform'}} */
    //precise volatile float fn_pre_vol() { return 1.0f; }    /* expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    //precise volatile const float fn_pre_vol_con() { return 1.0f; }    /* expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3025: l-value specifies const object}} */
    //precise const float fn_pre_con() { return 1.0f; }                 /* fxc-error {{X3025: l-value specifies const object}} */
    static float fn_sta() { return 1.0f; }
    //static volatile float fn_sta_vol() { return 1.0f; }     /* expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    //static volatile const float fn_sta_vol_con() { return 1.0f; }    /* expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    static const float fn_sta_con() { return 1.0f; }
    //uniform float fn_uni() { return 1.0f; }                 /* expected-error {{'uniform' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_uni': functions cannot be declared 'uniform'}} */
    //uniform volatile float fn_uni_vol() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_uni_vol': functions cannot be declared 'uniform'}} */
    //uniform volatile const float fn_uni_vol_con() { return 1.0f; }    /* expected-error {{'uniform' is not a valid modifier for a method}} expected-error {{'volatile' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_uni_vol_con': functions cannot be declared 'uniform'}} */
    //uniform const float fn_uni_con() { return 1.0f; }       /* expected-error {{'uniform' is not a valid modifier for a method}} fxc-error {{X3047: 'fn_uni_con': functions cannot be declared 'uniform'}} */
    //volatile float fn_vol() { return 1.0f; }                /* expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    //volatile const float fn_vol_con() { return 1.0f; }      /* expected-error {{'volatile' is not a valid modifier for a method}} fxc-pass {{}} */
    const float fn_con() { return 1.0f; }
    // GENERATED_CODE:END

    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float fn_%(id)s() { return 1.0f; }'))</py>
    // GENERATED_CODE:BEGIN
    //nointerpolation float fn_noi() { return 1.0f; }         /* expected-error {{'nointerpolation' is not a valid modifier for a method}} fxc-pass {{}} */
    //nointerpolation linear float fn_noi_lin() { return 1.0f; }    /* expected-error {{'linear' is not a valid modifier for a method}} expected-error {{'nointerpolation' and 'linear' cannot be used together for a method}} expected-error {{'nointerpolation' is not a valid modifier for a method}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
    //nointerpolation sample float fn_noi_sam() { return 1.0f; }    /* expected-error {{'nointerpolation' and 'sample' cannot be used together for a method}} expected-error {{'nointerpolation' is not a valid modifier for a method}} expected-error {{'sample' is not a valid modifier for a method}} fxc-pass {{}} */
    //nointerpolation noperspective float fn_noi_nop() { return 1.0f; }    /* expected-error {{'nointerpolation' and 'noperspective' cannot be used together for a method}} expected-error {{'nointerpolation' is not a valid modifier for a method}} expected-error {{'noperspective' is not a valid modifier for a method}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
    //nointerpolation centroid float fn_noi_cen() { return 1.0f; }    /* expected-error {{'centroid' is not a valid modifier for a method}} expected-error {{'nointerpolation' and 'centroid' cannot be used together for a method}} expected-error {{'nointerpolation' is not a valid modifier for a method}} fxc-error {{X3048: constinterp usage cannot be used with linear, noperspective, or centroid usage}} */
    //linear float fn_lin() { return 1.0f; }                  /* expected-error {{'linear' is not a valid modifier for a method}} fxc-pass {{}} */
    //linear sample float fn_lin_sam() { return 1.0f; }       /* expected-error {{'linear' is not a valid modifier for a method}} expected-error {{'sample' is not a valid modifier for a method}} fxc-pass {{}} */
    //linear sample noperspective float fn_lin_sam_nop() { return 1.0f; }    /* expected-error {{'linear' is not a valid modifier for a method}} expected-error {{'noperspective' is not a valid modifier for a method}} expected-error {{'sample' is not a valid modifier for a method}} fxc-pass {{}} */
    //linear sample noperspective centroid float fn_lin_sam_nop_cen() { return 1.0f; }    /* expected-error {{'centroid' is not a valid modifier for a method}} expected-error {{'linear' is not a valid modifier for a method}} expected-error {{'noperspective' is not a valid modifier for a method}} expected-error {{'sample' is not a valid modifier for a method}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    //linear sample centroid float fn_lin_sam_cen() { return 1.0f; }    /* expected-error {{'centroid' is not a valid modifier for a method}} expected-error {{'linear' is not a valid modifier for a method}} expected-error {{'sample' is not a valid modifier for a method}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    //linear noperspective float fn_lin_nop() { return 1.0f; }    /* expected-error {{'linear' is not a valid modifier for a method}} expected-error {{'noperspective' is not a valid modifier for a method}} fxc-pass {{}} */
    //linear noperspective centroid float fn_lin_nop_cen() { return 1.0f; }    /* expected-error {{'centroid' is not a valid modifier for a method}} expected-error {{'linear' is not a valid modifier for a method}} expected-error {{'noperspective' is not a valid modifier for a method}} fxc-pass {{}} */
    //linear centroid float fn_lin_cen() { return 1.0f; }     /* expected-error {{'centroid' is not a valid modifier for a method}} expected-error {{'linear' is not a valid modifier for a method}} fxc-pass {{}} */
    //sample float fn_sam() { return 1.0f; }                  /* expected-error {{'sample' is not a valid modifier for a method}} fxc-pass {{}} */
    //sample noperspective float fn_sam_nop() { return 1.0f; }    /* expected-error {{'noperspective' is not a valid modifier for a method}} expected-error {{'sample' is not a valid modifier for a method}} fxc-pass {{}} */
    //sample noperspective centroid float fn_sam_nop_cen() { return 1.0f; }    /* expected-error {{'centroid' is not a valid modifier for a method}} expected-error {{'noperspective' is not a valid modifier for a method}} expected-error {{'sample' is not a valid modifier for a method}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    //sample centroid float fn_sam_cen() { return 1.0f; }     /* expected-error {{'centroid' is not a valid modifier for a method}} expected-error {{'sample' is not a valid modifier for a method}} expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    //noperspective float fn_nop() { return 1.0f; }           /* expected-error {{'noperspective' is not a valid modifier for a method}} fxc-pass {{}} */
    //noperspective centroid float fn_nop_cen() { return 1.0f; }    /* expected-error {{'centroid' is not a valid modifier for a method}} expected-error {{'noperspective' is not a valid modifier for a method}} fxc-pass {{}} */
    //centroid float fn_cen() { return 1.0f; }                /* expected-error {{'centroid' is not a valid modifier for a method}} fxc-pass {{}} */
    // GENERATED_CODE:END

    //in float fn_in() { return 1.0f; }                       /* expected-error {{HLSL usage 'in' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'in'}} */
    //out float fn_out() { return 1.0f; }                     /* expected-error {{HLSL usage 'out' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'out'}} */
    //inout float fn_inout() { return 1.0f; }                 /* expected-error {{HLSL usage 'inout' is only valid on a parameter}} fxc-error {{X3000: syntax error: unexpected token 'inout'}} */

};