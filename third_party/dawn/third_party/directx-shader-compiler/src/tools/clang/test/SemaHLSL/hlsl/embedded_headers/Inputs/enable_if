// Overlay copy of enable_if.h used by embedded_header_include_path.hlsl to
// verify that an explicit -I search path takes precedence over the
// compiled-in HLSL headers.  The unique HLSL_OVERLAY_ENABLE_IF macro lets
// the test confirm this overlay was the version actually included.

#define HLSL_OVERLAY_ENABLE_IF 1

namespace hlsl {

template <bool B, typename T>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
  using type = T;
};

} // namespace hlsl
