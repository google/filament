// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: @main

// The following is meant to be processed by the CodeTags extension in the "VS For Everything" Visual Studio extension:
/*<py>
import re
rxComments = re.compile(r'(//.*|/\*.*?\*\/)')
def strip_comments(line):
    line = rxComments.sub('', line)
    return line.strip()
    saved = {}
    for line in lines:
        key = strip_comments(line)
        if key and line.strip() != key:
            saved[key] = line
    return saved
    return [saved.get(line.strip(), line) for line in lines]
def modify(lines, newlines):
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
groupshared float2 g_gro;
groupshared precise float2 g_gro_pre;
groupshared precise static float2 g_gro_pre_sta;
groupshared static float2 g_gro_sta;
extern float2 g_ext;
extern precise float2 g_ext_pre;
extern precise uniform float2 g_ext_pre_uni;
extern precise uniform const float2 g_ext_pre_uni_con;
extern precise const float2 g_ext_pre_con;
extern uniform float2 g_ext_uni;
extern uniform const float2 g_ext_uni_con;
extern const float2 g_ext_con;
precise float2 g_pre;
precise static float2 g_pre_sta;
precise static const float2 g_pre_sta_con;
precise uniform float2 g_pre_uni;
precise uniform const float2 g_pre_uni_con;
precise const float2 g_pre_con;
static float2 g_sta;
static const float2 g_sta_con;
uniform float2 g_uni;
uniform const float2 g_uni_con;
const float2 g_con;
// GENERATED_CODE:END

// Initialized:
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float g_%(id)s_init = 1.0f;', storage_combos))</py>
// GENERATED_CODE:BEGIN
groupshared float g_gro_init = 1.0f;
groupshared precise float g_gro_pre_init = 1.0f;
groupshared precise static float g_gro_pre_sta_init = 1.0f;
groupshared precise static const float g_gro_pre_sta_con_init = 1.0f;
groupshared precise const float g_gro_pre_con_init = 1.0f;
groupshared static float g_gro_sta_init = 1.0f;
groupshared static const float g_gro_sta_con_init = 1.0f;
groupshared const float g_gro_con_init = 1.0f;
extern float g_ext_init = 1.0f;                             /* expected-warning {{'extern' variable has an initializer}} fxc-pass {{}} */
extern precise float g_ext_pre_init = 1.0f;                 /* expected-warning {{'extern' variable has an initializer}} fxc-pass {{}} */
extern precise uniform float g_ext_pre_uni_init = 1.0f;     /* expected-warning {{'extern' variable has an initializer}} fxc-pass {{}} */
extern precise uniform const float g_ext_pre_uni_con_init = 1.0f;    /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
extern precise const float g_ext_pre_con_init = 1.0f;       /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
extern uniform float g_ext_uni_init = 1.0f;                 /* expected-warning {{'extern' variable has an initializer}} fxc-pass {{}} */
extern uniform const float g_ext_uni_con_init = 1.0f;       /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
extern const float g_ext_con_init = 1.0f;                   /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
precise float g_pre_init = 1.0f;
precise static float g_pre_sta_init = 1.0f;
precise static const float g_pre_sta_con_init = 1.0f;
precise uniform float g_pre_uni_init = 1.0f;
precise uniform const float g_pre_uni_con_init = 1.0f;      /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
precise const float g_pre_con_init = 1.0f;                  /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
static float g_sta_init = 1.0f;
static const float g_sta_con_init = 1.0f;
uniform float g_uni_init = 1.0f;
uniform const float g_uni_con_init = 1.0f;                  /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
const float g_con_init = 1.0f;                              /* fxc-warning {{X3207: Initializer used on a global 'const' variable. This requires setting an external constant. If a literal is desired, use 'static const' instead.}} */
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2 g_%(id)s;'))</py>
// GENERATED_CODE:BEGIN
// GENERATED_CODE:END



//////////////////////////////////////////////////////////////////////////////
// Typedefs.
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('typedef %(mods)s float2 t_%(id)s;', storage_combos))</py>
// GENERATED_CODE:BEGIN
typedef precise float2 t_pre;
typedef precise volatile float2 t_pre_vol;
typedef precise volatile const float2 t_pre_vol_con;
typedef precise const float2 t_pre_con;
typedef volatile float2 t_vol;
typedef volatile const float2 t_vol_con;
typedef const float2 t_con;
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('typedef %(mods)s float2 t_%(id)s;'))</py>
// GENERATED_CODE:BEGIN
// GENERATED_CODE:END


//////////////////////////////////////////////////////////////////////////////
// Fields.
struct s_storage_mods {
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2 f_%(id)s;', storage_combos))</py>
    // GENERATED_CODE:BEGIN
    precise float2 f_pre;
    // GENERATED_CODE:END
};

// Interpolation modifiers
struct s_interp_mods {
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float2 f_%(id)s;'))</py>
    // GENERATED_CODE:BEGIN
    nointerpolation float2 f_noi;
    linear float2 f_lin;
    linear sample float2 f_lin_sam;
    linear sample noperspective float2 f_lin_sam_nop;
    linear sample noperspective centroid float2 f_lin_sam_nop_cen;    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    linear sample centroid float2 f_lin_sam_cen;            /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    linear noperspective float2 f_lin_nop;
    linear noperspective centroid float2 f_lin_nop_cen;
    linear centroid float2 f_lin_cen;
    sample float2 f_sam;
    sample noperspective float2 f_sam_nop;
    sample noperspective centroid float2 f_sam_nop_cen;     /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    sample centroid float2 f_sam_cen;                       /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
    noperspective float2 f_nop;
    noperspective centroid float2 f_nop_cen;
    centroid float2 f_cen;
    // GENERATED_CODE:END
};

struct s_inout {
};

//////////////////////////////////////////////////////////////////////////////
// Parameters
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('float4 foo_%(id)s(%(mods)s float4 val) { return val; }', storage_combos))</py>
// GENERATED_CODE:BEGIN
float4 foo_pre(precise float4 val) { return val; }
float4 foo_pre_uni(precise uniform float4 val) { return val; }
float4 foo_pre_uni_con(precise uniform const float4 val) { return val; }
float4 foo_pre_con(precise const float4 val) { return val; }
float4 foo_uni(uniform float4 val) { return val; }
float4 foo_uni_con(uniform const float4 val) { return val; }
float4 foo_con(const float4 val) { return val; }
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('float4 foo_%(id)s(%(mods)s float4 val) { return val; }'))</py>
// GENERATED_CODE:BEGIN
float4 foo_noi(nointerpolation float4 val) { return val; }
float4 foo_lin(linear float4 val) { return val; }
float4 foo_lin_sam(linear sample float4 val) { return val; }
float4 foo_lin_sam_nop(linear sample noperspective float4 val) { return val; }
float4 foo_lin_sam_nop_cen(linear sample noperspective centroid float4 val) { return val; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
float4 foo_lin_sam_cen(linear sample centroid float4 val) { return val; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
float4 foo_lin_nop(linear noperspective float4 val) { return val; }
float4 foo_lin_nop_cen(linear noperspective centroid float4 val) { return val; }
float4 foo_lin_cen(linear centroid float4 val) { return val; }
float4 foo_sam(sample float4 val) { return val; }
float4 foo_sam_nop(sample noperspective float4 val) { return val; }
float4 foo_sam_nop_cen(sample noperspective centroid float4 val) { return val; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
float4 foo_sam_cen(sample centroid float4 val) { return val; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
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
float4 foo_out2(out float4 val) { 
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}
float4 foo_out3(out linear float4 val) {
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}
float4 foo_out4(linear out float4 val) {
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}
float4 foo_out5(linear out float4 val) { 
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}
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
float4 foo_out_missing_in_decl(float4 val);                 /* expected-note {{candidate function}} fxc-pass {{}} */
float4 foo_out_missing_in_decl(out float4 val) {            /* expected-note {{candidate function}} fxc-pass {{}} */
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}
float4 foo_out_missing_in_def(out float4 val);              /* expected-note {{candidate function}} fxc-pass {{}} */
float4 foo_out_missing_in_def(float4 val) {                 /* expected-note {{candidate function}} fxc-pass {{}} */
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}
float4 foo_inout_missing_in_decl(float4 val);               /* expected-note {{candidate function}} fxc-pass {{}} */
float4 foo_inout_missing_in_decl(inout float4 val) {        /* expected-note {{candidate function}} fxc-pass {{}} */
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}
float4 foo_inout_missing_in_def(inout float4 val);          /* expected-note {{candidate function}} fxc-pass {{}} */
float4 foo_inout_missing_in_def(float4 val) {               /* expected-note {{candidate function}} fxc-pass {{}} */
    val = float4(1.0f,2.0f,3.0f,4.0f);
    return val;
}

float4 use_conflicting_inout(float4 val) {
    float4 out1, inout1, in1, res1;
    float4 out2, inout2, in2, res2;
    res1 = foo_in_missing_in_def(in1);
    res2 = foo_in_missing_in_decl(in2);
    return res1 + res2 + inout1 + inout2;
}

// Try interpolation modifiers in function decl, valid, invalid, and conflicting modifiers:
float4 foo_noi_decl(nointerpolation float4 val);
float4 foo_interpolation_different_decl(nointerpolation float4 val);
float4 foo_interpolation_different_decl(sample float4 val) {
    return val;
}

//////////////////////////////////////////////////////////////////////////////
// Locals.
[numthreads(1,1,1)]
void main() {
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float l_%(id)s;', storage_combos))</py>
    // GENERATED_CODE:BEGIN
    precise float l_pre;
    precise static float l_pre_sta;
    precise static volatile float l_pre_sta_vol;
    precise static volatile const float l_pre_sta_vol_con;
    precise static const float l_pre_sta_con;
    precise volatile float l_pre_vol;
    static float l_sta;
    static volatile float l_sta_vol;
    static volatile const float l_sta_vol_con;
    static const float l_sta_con;
    volatile float l_vol;
    // GENERATED_CODE:END
    // Now with const vars initialized:
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float l_%(id)s_init = 0.0;', filter(lambda combo: 'const' in combo, storage_combos)))</py>
    // GENERATED_CODE:BEGIN
    precise static volatile const float l_pre_sta_vol_con_init = 0.0;
    precise static const float l_pre_sta_con_init = 0.0;
    precise volatile const float l_pre_vol_con_init = 0.0;
    precise const float l_pre_con_init = 0.0;
    static volatile const float l_sta_vol_con_init = 0.0;
    static const float l_sta_con_init = 0.0;
    volatile const float l_vol_con_init = 0.0;
    const float l_con_init = 0.0;
    // GENERATED_CODE:END

    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float3 l_%(id)s;'))</py>
    // GENERATED_CODE:BEGIN
    // GENERATED_CODE:END


    // no type:
    


}

//////////////////////////////////////////////////////////////////////////////
// Functions.
// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float fn_%(id)s() { return 1.0f; }', storage_combos))</py>
// GENERATED_CODE:BEGIN
precise float fn_pre() { return 1.0f; }
precise static float fn_pre_sta() { return 1.0f; }
static float fn_sta() { return 1.0f; }
static const float fn_sta_con() { return 1.0f; }
const float fn_con() { return 1.0f; }
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float fn_%(id)s() { return 1.0f; }'))</py>
// GENERATED_CODE:BEGIN
nointerpolation float fn_noi() { return 1.0f; }
linear float fn_lin() { return 1.0f; }
linear sample float fn_lin_sam() { return 1.0f; }
linear sample noperspective float fn_lin_sam_nop() { return 1.0f; }
linear sample noperspective centroid float fn_lin_sam_nop_cen() { return 1.0f; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
linear sample centroid float fn_lin_sam_cen() { return 1.0f; }  /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
linear noperspective float fn_lin_nop() { return 1.0f; }
linear noperspective centroid float fn_lin_nop_cen() { return 1.0f; }
linear centroid float fn_lin_cen() { return 1.0f; }
sample float fn_sam() { return 1.0f; }
sample noperspective float fn_sam_nop() { return 1.0f; }
sample noperspective centroid float fn_sam_nop_cen() { return 1.0f; }    /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
sample centroid float fn_sam_cen() { return 1.0f; }             /* expected-warning {{'centroid' will be overridden by 'sample'}} fxc-pass {{}} */
noperspective float fn_nop() { return 1.0f; }
noperspective centroid float fn_nop_cen() { return 1.0f; }
centroid float fn_cen() { return 1.0f; }
// GENERATED_CODE:END


//////////////////////////////////////////////////////////////////////////////
// Methods.
class C
{
    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float fn_%(id)s() { return 1.0f; }', storage_combos))</py>
    // GENERATED_CODE:BEGIN
    precise float fn_pre() { return 1.0f; }
    precise static float fn_pre_sta() { return 1.0f; }
    static float fn_sta() { return 1.0f; }
    static const float fn_sta_con() { return 1.0f; }
    const float fn_con() { return 1.0f; }
    // GENERATED_CODE:END

    // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(mods)s float fn_%(id)s() { return 1.0f; }'))</py>
    // GENERATED_CODE:BEGIN
    // GENERATED_CODE:END


};