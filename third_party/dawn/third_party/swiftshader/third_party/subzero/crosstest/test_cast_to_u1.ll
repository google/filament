define i32 @_Z4castIxbET0_T_(i64 %a) {
entry:
;  %tobool = icmp ne i64 %a, 0
  %tobool = trunc i64 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z4castIybET0_T_(i64 %a) {
entry:
;  %tobool = icmp ne i64 %a, 0
  %tobool = trunc i64 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z4castIibET0_T_(i32 %a) {
entry:
;  %tobool = icmp ne i32 %a, 0
  %tobool = trunc i32 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z4castIjbET0_T_(i32 %a) {
entry:
;  %tobool = icmp ne i32 %a, 0
  %tobool = trunc i32 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z4castIsbET0_T_(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
;  %tobool = icmp ne i16 %a.arg_trunc, 0
  %tobool = trunc i16 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z4castItbET0_T_(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
;  %tobool = icmp ne i16 %a.arg_trunc, 0
  %tobool = trunc i16 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z4castIabET0_T_(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
;  %tobool = icmp ne i8 %a.arg_trunc, 0
  %tobool = trunc i8 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z4castIhbET0_T_(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
;  %tobool = icmp ne i8 %a.arg_trunc, 0
  %tobool = trunc i8 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z4castIbbET0_T_(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i1
  %a.arg_trunc.ret_ext = zext i1 %a.arg_trunc to i32
  ret i32 %a.arg_trunc.ret_ext
}

define i32 @_Z4castIdbET0_T_(double %a) {
entry:
;  %tobool = fcmp une double %a, 0.000000e+00
  %tobool = fptoui double %a to i32
  %tobool.i1 = trunc i32 %tobool to i1
  %tobool.ret_ext = zext i1 %tobool.i1 to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z4castIfbET0_T_(float %a) {
entry:
;  %tobool = fcmp une float %a, 0.000000e+00
  %tobool = fptoui float %a to i32
  %tobool.i1 = trunc i32 %tobool to i1
  %tobool.ret_ext = zext i1 %tobool.i1 to i32
  ret i32 %tobool.ret_ext
}

define internal i32 @_Z4castIbbET0_iT_i(i32 %i, i32 %a, i32 %j) {
entry:
  %a.arg_trunc = trunc i32 %a to i1
  %a.arg_trunc.ret_ext = zext i1 %a.arg_trunc to i32
  ret i32 %a.arg_trunc.ret_ext
}

define internal i32 @_Z4castIabET0_iT_i(i32 %i, i32 %a, i32 %j) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
;  %tobool = icmp ne i8 %a.arg_trunc, 0
  %tobool = trunc i8 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define internal i32 @_Z4castIhbET0_iT_i(i32 %i, i32 %a, i32 %j) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
;  %tobool = icmp ne i8 %a.arg_trunc, 0
  %tobool = trunc i8 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define internal i32 @_Z4castIsbET0_iT_i(i32 %i, i32 %a, i32 %j) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
;  %tobool = icmp ne i16 %a.arg_trunc, 0
  %tobool = trunc i16 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define internal i32 @_Z4castItbET0_iT_i(i32 %i, i32 %a, i32 %j) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
;  %tobool = icmp ne i16 %a.arg_trunc, 0
  %tobool = trunc i16 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define internal i32 @_Z4castIibET0_iT_i(i32 %i, i32 %a, i32 %j) {
entry:
  %tobool = icmp ne i32 %a, 0
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define internal i32 @_Z4castIjbET0_iT_i(i32 %i, i32 %a, i32 %j) {
entry:
  %tobool = icmp ne i32 %a, 0
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define internal i32 @_Z4castIxbET0_iT_i(i32 %i, i64 %a, i32 %j) {
entry:
  %tobool = icmp ne i64 %a, 0
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define internal i32 @_Z4castIybET0_iT_i(i32 %i, i64 %a, i32 %j) {
entry:
  %tobool = icmp ne i64 %a, 0
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define internal i32 @_Z4castIfbET0_iT_i(i32 %i, float %a, i32 %j) {
entry:
;  %tobool = fcmp une float %a, 0.000000e+00
  %tobool = fptoui float %a to i32
  %tobool.i1 = trunc i32 %tobool to i1
  %tobool.ret_ext = zext i1 %tobool.i1 to i32
  ret i32 %tobool.ret_ext
}

define internal i32 @_Z4castIdbET0_iT_i(i32 %i, double %a, i32 %j) {
entry:
;  %tobool = fcmp une double %a, 0.000000e+00
  %tobool = fptoui double %a to i32
  %tobool.i1 = trunc i32 %tobool to i1
  %tobool.ret_ext = zext i1 %tobool.i1 to i32
  ret i32 %tobool.ret_ext
}
