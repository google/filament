/*% cc -I. -I../api -I../../../../../registry/trunk/egl/api/ -c % ; rm testesext.o
 *
 * Test to make sure gl3.h builds properly.
 * Will also use SVN internal versions of gl3platform.h
 * and khrplatform.h (if needed) (via GLES and KHR symlinks into
 * appropriate parts of the tree).
 */
#define GL_GLEXT_PROTOTYPES
#include "GLES3/gl31.h"
#include "GLES2/gl2ext.h"

#if GL_NV_non_square_matrices != 1
#error "No extension GL_NV_non_square_matrices!"
#endif
