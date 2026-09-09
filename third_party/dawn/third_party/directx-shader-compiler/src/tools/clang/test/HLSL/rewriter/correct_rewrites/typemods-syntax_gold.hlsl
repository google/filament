// Rewrite unchanged result:
row_major float2x3 const g_row;
row_major snorm float2x3 const g_row_sno;
row_major unorm float2x3 const g_row_uno;
column_major float2x3 const g_col;
column_major snorm float2x3 const g_col_sno;
column_major unorm float2x3 const g_col_uno;
snorm float2x3 const g_sno;
unorm float2x3 const g_uno;
snorm float2x3 const g_rev_sno;
unorm float2x3 const g_rev_uno;
row_major float2x3 const g_rev_row;
column_major float2x3 const g_rev_col;
snorm float2 const g_vector_sno;
unorm float2 const g_vector_uno;
snorm float const g_scalar_sno;
unorm float const g_scalar_uno;
matrix<snorm float, 2, 3> g_mat_sno;
matrix<unorm float, 2, 3> g_mat_uno;
row_major matrix<snorm float, 2, 3> g_row_mat_sno;
row_major matrix<unorm float, 2, 3> g_row_mat_uno;
column_major matrix<snorm float, 2, 3> g_col_mat_sno;
column_major matrix<unorm float, 2, 3> g_col_mat_uno;
static row_major float2x3 const sc_row_init_scalar = 1.F;
static row_major snorm float2x3 const sc_row_sno_init_scalar = 1.F;
static row_major unorm float2x3 const sc_row_uno_init_scalar = 1.F;
static column_major float2x3 const sc_col_init_scalar = 1.F;
static column_major snorm float2x3 const sc_col_sno_init_scalar = 1.F;
static column_major unorm float2x3 const sc_col_uno_init_scalar = 1.F;
static snorm float2x3 const sc_sno_init_scalar = 1.F;
static unorm float2x3 const sc_uno_init_scalar = 1.F;
static const matrix<snorm float, 2, 3> sc_mat_sno_init_scalar = 1.F;
static const matrix<unorm float, 2, 3> sc_mat_uno_init_scalar = 1.F;
static row_major float2x3 s_row_init = sc_row_init_scalar;
static row_major snorm float2x3 s_row_sno_init = sc_row_sno_init_scalar;
static row_major unorm float2x3 s_row_uno_init = sc_row_uno_init_scalar;
static column_major float2x3 s_col_init = sc_col_init_scalar;
static column_major snorm float2x3 s_col_sno_init = sc_col_sno_init_scalar;
static column_major unorm float2x3 s_col_uno_init = sc_col_uno_init_scalar;
static snorm float2x3 s_sno_init = sc_sno_init_scalar;
static unorm float2x3 s_uno_init = sc_uno_init_scalar;
cbuffer CBInit {
  row_major float2x3 const g_row_init = sc_row_init_scalar;
  row_major snorm float2x3 const g_row_sno_init = sc_row_sno_init_scalar;
  row_major unorm float2x3 const g_row_uno_init = sc_row_uno_init_scalar;
  column_major float2x3 const g_col_init = sc_col_init_scalar;
  column_major snorm float2x3 const g_col_sno_init = sc_col_sno_init_scalar;
  column_major unorm float2x3 const g_col_uno_init = sc_col_uno_init_scalar;
  snorm float2x3 const g_sno_init = sc_sno_init_scalar;
  unorm float2x3 const g_uno_init = sc_uno_init_scalar;
  row_major float2x3 const g_row_init_scalar = 1.F;
  row_major snorm float2x3 const g_row_sno_init_scalar = 1.F;
  row_major unorm float2x3 const g_row_uno_init_scalar = 1.F;
  column_major float2x3 const g_col_init_scalar = 1.F;
  column_major snorm float2x3 const g_col_sno_init_scalar = 1.F;
  column_major unorm float2x3 const g_col_uno_init_scalar = 1.F;
  snorm float2x3 const g_sno_init_scalar = 1.F;
  unorm float2x3 const g_uno_init_scalar = 1.F;
}
typedef row_major float2x3 t_row;
typedef row_major snorm float2x3 t_row_sno;
typedef row_major unorm float2x3 t_row_uno;
typedef column_major float2x3 t_col;
typedef column_major snorm float2x3 t_col_sno;
typedef column_major unorm float2x3 t_col_uno;
typedef snorm float2x3 t_sno;
typedef unorm float2x3 t_uno;
typedef row_major matrix<float, 2, 3> t_row_mat;
typedef column_major matrix<float, 2, 3> t_col_mat;
typedef snorm float t_sno_f;
typedef unorm float t_uno_f;
typedef row_major matrix<t_sno_f, 2, 3> t_row_mat_sno;
typedef row_major matrix<t_uno_f, 2, 3> t_row_mat_uno;
typedef column_major matrix<t_sno_f, 2, 3> t_col_mat_sno;
typedef column_major matrix<t_uno_f, 2, 3> t_col_mat_uno;
typedef matrix<snorm float, 2, 3> t_mat_sno;
typedef matrix<unorm float, 2, 3> t_mat_uno;
struct s_fields {
  row_major float2x3 f_row;
  row_major snorm float2x3 f_row_sno;
  row_major unorm float2x3 f_row_uno;
  column_major float2x3 f_col;
  column_major snorm float2x3 f_col_sno;
  column_major unorm float2x3 f_col_uno;
  snorm float2x3 f_sno;
  unorm float2x3 f_uno;
};
float3 foo_row(row_major float2x3 val) {
  return val[0];
}


float3 foo_row_sno(row_major snorm float2x3 val) {
  return val[0];
}


float3 foo_row_uno(row_major unorm float2x3 val) {
  return val[0];
}


float3 foo_col(column_major float2x3 val) {
  return val[0];
}


float3 foo_col_sno(column_major snorm float2x3 val) {
  return val[0];
}


float3 foo_col_uno(column_major unorm float2x3 val) {
  return val[0];
}


float3 foo_sno(snorm float2x3 val) {
  return val[0];
}


float3 foo_uno(unorm float2x3 val) {
  return val[0];
}


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
  float3 res = (float3)0;
  res += foo_col(val);
  res += foo_col(val_column);
  res += foo_col(val_row);
  return res;
}


void vain() {
  row_major float2x3 l_row;
  row_major snorm float2x3 l_row_sno;
  row_major unorm float2x3 l_row_uno;
  column_major float2x3 l_col;
  column_major snorm float2x3 l_col_sno;
  column_major unorm float2x3 l_col_uno;
  snorm float2x3 l_sno;
  unorm float2x3 l_uno;
}


row_major float2x3 fn_row() {
  return 1.F;
}


row_major snorm float2x3 fn_row_sno() {
  return 1.F;
}


row_major unorm float2x3 fn_row_uno() {
  return 1.F;
}


column_major float2x3 fn_col() {
  return 1.F;
}


column_major snorm float2x3 fn_col_sno() {
  return 1.F;
}


column_major unorm float2x3 fn_col_uno() {
  return 1.F;
}


snorm float2x3 fn_sno() {
  return 1.F;
}


unorm float2x3 fn_uno() {
  return 1.F;
}


class C {
  row_major float2x3 fn_row()   {
    return 1.F;
  }


  row_major snorm float2x3 fn_row_sno()   {
    return 1.F;
  }


  row_major unorm float2x3 fn_row_uno()   {
    return 1.F;
  }


  column_major float2x3 fn_col()   {
    return 1.F;
  }


  column_major snorm float2x3 fn_col_sno()   {
    return 1.F;
  }


  column_major unorm float2x3 fn_col_uno()   {
    return 1.F;
  }


  snorm float2x3 fn_sno()   {
    return 1.F;
  }


  unorm float2x3 fn_uno()   {
    return 1.F;
  }


};
