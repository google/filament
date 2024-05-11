#!/bin/sh
cd ../../code
grep -E 'Ifc([A-Z][a-z]*)+' -o IFCLoader.cpp IFCGeometry.cpp IFCCurve.cpp IFCProfile.cpp IFCMaterial.cpp | uniq | sed s/.*:// > ../scripts/IFCImporter/output.txt
