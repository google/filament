pyassimp examples
=================

- `sample.py`: shows how to load a model with pyassimp, and display some statistics.
- `3d_viewer.py`: an OpenGL 3D viewer that requires shaders
- `fixed_pipeline_3d_viewer`: an OpenGL 3D viewer using the old fixed-pipeline.
  Only for illustration example. Base new projects on `3d_viewer.py`.


Requirements for the 3D viewers:

- `pyopengl` (on Ubuntu/Debian, `sudo apt-get install python-opengl`)
- `pygame` (on Ubuntu/Debian, `sudo apt-get install python-pygame`)
