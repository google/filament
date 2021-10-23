Pandora 
=====================================================================

( http://openpandora.org/ )
- A pandora specific video driver was written to allow SDL 2.0 with OpenGL ES
support to work on the pandora under the framebuffer. This driver do not have
input support for now, so if you use it you will have to add your own control code.
The video driver name is "pandora" so if you have problem running it from
the framebuffer, try to set the following variable before starting your application :
"export SDL_VIDEODRIVER=pandora"

- OpenGL ES support was added to the x11 driver, so it's working like the normal
x11 driver one with OpenGLX support, with SDL input event's etc..


David Carré (Cpasjuste)
cpasjuste@gmail.com
