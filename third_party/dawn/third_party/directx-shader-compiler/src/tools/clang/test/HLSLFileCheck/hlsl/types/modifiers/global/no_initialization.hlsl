// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that no variable initializers are emitted, especially for cbuffers globals.

// CHECK-NOT: {{.*}} = constant
// CHECK: define void @main()

int var;
int var_init = 1;
const int const_var;
const int const_var_init = 1;
extern int extern_var;
extern int extern_var_init = 1;
extern const int extern_const_var;
extern const int extern_const_var_init = 1;

// Those get optimized away
static int static_var;
static int static_var_init = 1;
static const int static_const_var;
static const int static_const_var_init = 1;

struct s
{
  // Those get optimized away
  static int struct_static_var;
  // static int struct_static_var_init = 1; // error: struct/class members cannot have default values
  static const int struct_static_const_var;
  static const int struct_static_const_var_init = 1;
};

int s::struct_static_var = 1;
const int s::struct_static_const_var = 1;

int main() : OUT {
  static int func_static_var;
  static int func_static_var_init = 1;
  static const int func_static_const_var;
  static const int func_static_const_var_init = 1;
  return var + var_init
    + const_var + const_var_init
    + extern_var + extern_var_init
    + extern_const_var + extern_const_var_init
    + static_var + static_var_init
    + static_const_var + static_const_var_init
    + s::struct_static_var + /*s::struct_static_var_init*/
    + s::struct_static_const_var + s::struct_static_const_var_init
    + func_static_var + func_static_var_init
    + func_static_const_var + func_static_const_var_init;
}