# About

Because ANGLE can only generate full HLSL programs after we known the signatures
between the vertex and pixel stages, we can not immediately call the D3D
compiler at GL shader compile time. Moreover, we can insert additional
optimization code right at draw-time.

ESSL 1.00 shaders treat all vertex inputs as floating point. We insert a
conversion routine to transform un-normalized integer vertex attributes in the
shader preamble to floating point, saving CPU conversion time.

At draw-time, we also optimize out any unused render target outputs. This
improved draw call performance significantly on lower spec and integrated
devices. Changing render target setups may trigger a shader recompile at draw
time.

# Addendum

ANGLE is not the only program to do this kind of draw-time optimization. A
common complaint from application developers is that draw calls sometimes
perform very slowly due to dynamic shader re-compilation. A future design
direction for ANGLE, when targeting a more modern API, is to perform the vertex
conversion in a separate shader pass, which would then be linked with another
compiled shader.
