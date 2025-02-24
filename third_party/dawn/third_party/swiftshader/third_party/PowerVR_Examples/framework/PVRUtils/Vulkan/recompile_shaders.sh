#!/bin/bash
../../../bin/Linux_x86_64/Release/bin/glslangvalidator -S vert -V UIRendererVertShader.vsh -o UIRendererVertShader.vsh.spv
../../../bin/Linux_x86_64/Release/bin/glslangvalidator -S frag -V UIRendererFragShader.fsh -o UIRendererFragShader.fsh.spv
../../scripts/dumpSpv.sh UIRendererVertShader.vsh.spv UIRendererVertShader
../../scripts/dumpSpv.sh UIRendererFragShader.fsh.spv UIRendererFragShader

../../../bin/Linux_x86_64/Release/bin/glslangvalidator -S vert -V PBRUtilsVertShader.vsh -o PBRUtilsVertShader.vsh.spv
../../../bin/Linux_x86_64/Release/bin/glslangvalidator -S frag -V PBRUtilsIrradianceFragShader.fsh -o PBRUtilsIrradianceFragShader.fsh.spv
../../../bin/Linux_x86_64/Release/bin/glslangvalidator -S frag -V PBRUtilsPrefilteredFragShader.fsh -o PBRUtilsPrefilteredFragShader.fsh.spv
../../scripts/dumpSpv.sh PBRUtilsVertShader.vsh.spv PBRUtilsVertShader
../../scripts/dumpSpv.sh PBRUtilsIrradianceFragShader.fsh.spv PBRUtilsIrradianceFragShader
../../scripts/dumpSpv.sh PBRUtilsPrefilteredFragShader.fsh.spv PBRUtilsPrefilteredFragShader