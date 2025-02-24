#!/bin/bash
glslangValidator -V VertShader.vsh -o VertShader.vsh.spv -S vert
glslangValidator -V FragShader.fsh -o FragShader.fsh.spv -S frag
dumpSpv.sh VertShader.vsh.spv VertShader
dumpSpv.sh FragShader.fsh.spv FragShader