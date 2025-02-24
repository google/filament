/*% /lib/cpp -I../api testglext.c > testglext.i ; cc -c % ; rm testglext.[oi]
*/
#define GL_GLEXT_LEGACY
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include "GL/glext.h"

#define GLX_GLXEXT_LEGACY
#include <GL/glx.h>

/* Necessary to avoid token redefinition warnings */
#undef GLX_SAMPLE_BUFFERS
#undef GLX_SAMPLES
#undef GLX_FRONT_EXT
#undef GLX_BACK_EXT

#define GLX_GLXEXT_PROTOTYPES
#include "GL/glxext.h"

int VERSION_4_3 = GL_VERSION_4_3;
/* These should be the same */
int mcsor = GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES;
int mciufo = GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS;

/* See if latest extension is included */
int latest_ext = GL_NV_blend_equation_advanced;
GLenum latest_enum = GL_CONJOINT_NV;
