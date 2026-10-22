
------------------------------------------------------------------------------------
Open Asset Import Library (Assimp) Viewer Installer
Release Notes
------------------------------------------------------------------------------------

http://assimp.sf.net

Known Bugs & Limitations
========================

Viewer

- Normals appear flipped from time to time when either of the normals-related menu items was hit.
- Alpha-sorting is implemented, but still causes artifacts when models are moved quickly.
- Several important texture file formats (such as GIF) are not supported.
- HUD is blurred on the right side. ATI/AMD hardware only.

Troubleshooting
===============

1. Missing d3dx9_(number).dll? 
Install the latest DirectX runtime or grab the file from somewhere (that's evil but mostly fine).

2. Application configuration not correct / missing msvcr***.dll?
Reinstall Microsoft Visual C++ 2005 SP1 Redistributable (x86 or x64, depending on your system)

3. assimp.exe not in PATH
Add it to PATH. That's not a bug, the installer does not alter the PATH.

4. Crashes immediately
You CPU lacks SSE2 support. Build Assimp from scratch to suit your CPU, sorry.