#ifndef __gl_h_
#define __gl_h_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This document is licensed under the SGI Free Software B License Version
 * 2.0. For details, see http://oss.sgi.com/projects/FreeB/ .
 */

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef short GLshort;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef void GLvoid;
/* Internal convenience typedefs */
typedef void (*_GLfuncptr)(void);

/*************************************************************/

/* Extensions */
#define GL_OSC_VERSION_1_0                1
#define GL_EXT_paletted_texture           1
#define GL_OES_single_precision           1

/* ClearBufferMask */
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000

/* Boolean */
#define GL_FALSE                          0
#define GL_TRUE                           1

/* BeginMode */
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006

/* AlphaFunction */
/* #define GL_LEQUAL                         0x0203 */
/* #define GL_ALWAYS                         0x0207 */

/* BlendingFactorDest */
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_ONE_MINUS_SRC_ALPHA            0x0303

/* BlendingFactorSrc */
/* #define GL_ONE                            1 */
#define GL_SRC_ALPHA_SATURATE             0x0308
#define GL_SRC_ALPHA                      0x0302

/* ColorMaterialFace */
/* #define GL_FRONT_AND_BACK                 0x0408 */

/* ColorMaterialParameter */
/* #define GL_AMBIENT_AND_DIFFUSE            0x1602 */

/* ColorPointerType */
/* #define GL_FLOAT                          0x1406 */
/* #define GL_UNSIGNED_BYTE                  0x1401 */

/* CullFaceMode */
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_FRONT_AND_BACK                 0x0408

/* DepthFunction */
/* #define GL_LESS                           0x0201 */
/* #define GL_LEQUAL                         0x0203 */
/* #define GL_ALWAYS                         0x0207 */

/* EnableCap */
#define GL_LIGHTING                       0x0B50
#define GL_TEXTURE_2D                     0x0DE1
#define GL_CULL_FACE                      0x0B44
#define GL_ALPHA_TEST                     0x0BC0
#define GL_BLEND                          0x0BE2
#define GL_STENCIL_TEST                   0x0B90
#define GL_DEPTH_TEST                     0x0B71
#define GL_LIGHT0                         0x4000
#define GL_LIGHT1                         0x4001
#define GL_POINT_SMOOTH                   0x0B10
#define GL_LINE_STIPPLE                   0x0B24
#define GL_LINE_SMOOTH                    0x0B20
#define GL_SCISSOR_TEST                   0x0C11
#define GL_COLOR_MATERIAL                 0x0B57
#define GL_NORMALIZE                      0x0BA1
#define GL_RESCALE_NORMAL                 0x803A
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_POLYGON_STIPPLE                0x0B42
#define GL_VERTEX_ARRAY                   0x8074
#define GL_NORMAL_ARRAY                   0x8075
#define GL_COLOR_ARRAY                    0x8076
#define GL_TEXTURE_COORD_ARRAY            0x8078

/* ErrorCode */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_OUT_OF_MEMORY                  0x0505

/* FogMode */

/* FogParameter */

/* FrontFaceDirection */
#define GL_CW                             0x0900
#define GL_CCW                            0x0901

/* GetBooleanv */
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_COLOR_WRITEMASK                0x0C23

/* GetFloatv */
#define GL_CURRENT_COLOR                  0x0B00
#define GL_CURRENT_NORMAL                 0x0B02
#define GL_CURRENT_TEXTURE_COORDS         0x0B03
#define GL_CURRENT_RASTER_COLOR           0x0B04
#define GL_CURRENT_RASTER_TEXTURE_COORDS  0x0B06
#define GL_POINT_SIZE                     0x0B11
#define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY  0x0B13
#define GL_LINE_WIDTH                     0x0B21
#define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY  0x0B23
#define GL_LIGHT_MODEL_AMBIENT            0x0B53
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_ALPHA_TEST_REF                 0x0BC2
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_POLYGON_OFFSET_UNITS           0x2A00
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E

/* GetIntegerv */
#define GL_MATRIX_MODE                    0x0BA0
#define GL_VIEWPORT                       0x0BA2
#define GL_MODELVIEW_STACK_DEPTH          0x0BA3
#define GL_PROJECTION_STACK_DEPTH         0x0BA4
#define GL_MODELVIEW_MATRIX               0x0BA6
#define GL_PROJECTION_MATRIX              0x0BA7
#define GL_LINE_STIPPLE_PATTERN           0x0B25
#define GL_LINE_STIPPLE_REPEAT            0x0B26
#define GL_MAX_LIST_NESTING               0x0B31
#define GL_LIST_BASE                      0x0B32
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_ALPHA_TEST_FUNC                0x0BC1
#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_SCISSOR_BOX                    0x0C10
#define GL_PERSPECTIVE_CORRECTION_HINT    0x0C50
#define GL_POINT_SMOOTH_HINT              0x0C51
#define GL_LINE_SMOOTH_HINT               0x0C52
#define GL_POLYGON_SMOOTH_HINT            0x0C53
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAX_LIGHTS                     0x0D31
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_MODELVIEW_STACK_DEPTH      0x0D36
#define GL_MAX_PROJECTION_STACK_DEPTH     0x0D38
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_RED_BITS                       0x0D52
#define GL_GREEN_BITS                     0x0D53
#define GL_BLUE_BITS                      0x0D54
#define GL_ALPHA_BITS                     0x0D55
#define GL_DEPTH_BITS                     0x0D56
#define GL_STENCIL_BITS                   0x0D57
#define GL_VERTEX_ARRAY_SIZE              0x807A 
#define GL_VERTEX_ARRAY_TYPE              0x807B 
#define GL_VERTEX_ARRAY_STRIDE            0x807C 
#define GL_NORMAL_ARRAY_TYPE              0x807E 
#define GL_NORMAL_ARRAY_STRIDE            0x807F 
#define GL_COLOR_ARRAY_SIZE               0x8081 
#define GL_COLOR_ARRAY_TYPE               0x8082 
#define GL_COLOR_ARRAY_STRIDE             0x8083 
#define GL_TEXTURE_COORD_ARRAY_SIZE       0x8088 
#define GL_TEXTURE_COORD_ARRAY_TYPE       0x8089 
#define GL_TEXTURE_COORD_ARRAY_STRIDE     0x808A 
#define GL_SHADE_MODEL                    0x0B54
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9
#define GL_ACTIVE_TEXTURE                 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE          0x84E1
#define GL_MAX_TEXTURE_UNITS              0x84E2

/* GetMaterialfv */
/* #define GL_AMBIENT                        0x1200 */
/* #define GL_DIFFUSE                        0x1201 */
/* #define GL_SPECULAR                       0x1202 */
/* #define GL_EMISSION                       0x1600 */
/* #define GL_SHININESS                      0x1601 */

/* GetLightfv */
/* #define GL_AMBIENT                        0x1200 */
/* #define GL_DIFFUSE                        0x1201 */
/* #define GL_SPECULAR                       0x1202 */
/* #define GL_POSITION                       0x1203 */

/* GetPointerv */
#define GL_VERTEX_ARRAY_POINTER              0x808E
#define GL_NORMAL_ARRAY_POINTER              0x808F
#define GL_COLOR_ARRAY_POINTER               0x8090
#define GL_TEXTURE_COORD_ARRAY_POINTER       0x8092

/* GetTexParameter */
/* #define GL_TEXTURE_MAG_FILTER             0x2800 */
/* #define GL_TEXTURE_MIN_FILTER             0x2801 */
/* #define GL_TEXTURE_WRAP_S                 0x2802 */
/* #define GL_TEXTURE_WRAP_T                 0x2803 */

/* GetTexEnvfv */
/* #define GL_TEXTURE_ENV_MODE               0x2200 */
/* #define GL_TEXTURE_ENV_COLOR              0x2201 */

/* HintMode */
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102

/* HintTarget */
#define GL_PERSPECTIVE_CORRECTION_HINT    0x0C50
#define GL_POINT_SMOOTH_HINT              0x0C51
#define GL_LINE_SMOOTH_HINT               0x0C52

/* IsEnabled */
/* #define GL_LIGHTING                       0x0B50 */
/* #define GL_TEXTURE_2D                     0x0DE1 */
/* #define GL_CULL_FACE                      0x0B44 */
/* #define GL_ALPHA_TEST                     0x0BC0 */
/* #define GL_BLEND                          0x0BE2 */
/* #define GL_STENCIL_TEST                   0x0B90 */
/* #define GL_DEPTH_TEST                     0x0B71 */
/* #define GL_LIGHT0                         0x4000 */
/* #define GL_LIGHT1                         0x4001 */
/* #define GL_POINT_SMOOTH                   0x0B10 */
/* #define GL_LINE_STIPPLE                   0x0B24 */
/* #define GL_LINE_SMOOTH                    0x0B20 */
/* #define GL_SCISSOR_TEST                   0x0C11 */
/* #define GL_COLOR_MATERIAL                 0x0B57 */
/* #define GL_NORMALIZE                      0x0BA1 */
/* #define GL_RESCALE_NORMAL                 0x803A */
/* #define GL_POLYGON_OFFSET_FILL            0x8037 */
/* #define GL_POLYGON_STIPPLE                0x0B42 */
/* #define GL_VERTEX_ARRAY                   0x8074 */
/* #define GL_NORMAL_ARRAY                   0x8075 */
/* #define GL_COLOR_ARRAY                    0x8076 */
/* #define GL_TEXTURE_COORD_ARRAY            0x8078 */

/* LightModelParameter */
#define GL_LIGHT_MODEL_AMBIENT            0x0B53

/* LightParameter */
#define GL_AMBIENT                        0x1200
#define GL_DIFFUSE                        0x1201
#define GL_SPECULAR                       0x1202
#define GL_POSITION                       0x1203

/* ListMode */
#define GL_COMPILE						            0x1300
/* #define GL_COMPILE_AND_EXECUTE			   	  0x1301 */

/* DataType */
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
/* #define GL_SHORT                          0x1402 */
/* #define GL_UNSIGNED_SHORT                 0x1403 */
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406

/* LogicOp */

/* MaterialFace */
/* #define GL_FRONT_AND_BACK                 0x0408 */

/* MaterialParameter */
/* #define GL_AMBIENT                        0x1200 */
/* #define GL_DIFFUSE                        0x1201 */
/* #define GL_SPECULAR                       0x1202 */
#define GL_EMISSION                       0x1600
#define GL_SHININESS                      0x1601
#define GL_AMBIENT_AND_DIFFUSE            0x1602

/* MatrixMode */
#define GL_MODELVIEW                      0x1700
#define GL_PROJECTION                     0x1701

/* NormalPointerType */
/* #define GL_FLOAT                          0x1406 */

/* PixelFormat */
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A
#define GL_COLOR_INDEX                    0x1900

/* PixelStoreParameter */
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_ALIGNMENT                 0x0D05

/* PixelType */
/* #define GL_UNSIGNED_BYTE                  0x1401 */

/* ReadPixels */
#define GL_COLOR                          0x1800

/* ShadingModel */
#define GL_FLAT                           0x1D00
#define GL_SMOOTH                         0x1D01

/* StencilFunction */
#define GL_NEVER                          0x0200 
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

/* StencilOp */
/* #define GL_ZERO                           0 */
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
#define GL_INVERT                         0x150A

/* StringName */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

/* TexCoordPointerType */
/* #define GL_FLOAT                          0x1406 */

/* TextureEnvMode */
#define GL_MODULATE                       0x2100
#define GL_DECAL                          0x2101
/* #define GL_BLEND                          0x0BE2 */
#define GL_ADD                            0x0104
/* #define GL_REPLACE                        0x1E01 */

/* TextureEnvParameter */
#define GL_TEXTURE_ENV_MODE               0x2200
#define GL_TEXTURE_ENV_COLOR              0x2201

/* TextureEnvTarget */
#define GL_TEXTURE_ENV                    0x2300

/* TextureMagFilter */
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

/* TextureMinFilter */
/* #define GL_NEAREST                        0x2600 */
/* #define GL_LINEAR                         0x2601 */
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

/* TextureParameterName */
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803

/* TextureTarget */
/* #define GL_TEXTURE_2D                     0x0DE1 */

/* TextureUnit */
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF

/* TextureWrapMode */
#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F

/* PixelInternalFormat */
#define GL_COLOR_INDEX8_EXT               0x80E5

/* VertexPointerType */
/* #define GL_FLOAT                          0x1406 */

/* Paletted Textures Extension */
#define GL_COLOR_TABLE_FORMAT_EXT         0x80D8
#define GL_COLOR_TABLE_WIDTH_EXT          0x80D9
#define GL_COLOR_TABLE_RED_SIZE_EXT       0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE_EXT     0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE_EXT      0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE_EXT     0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE_EXT 0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE_EXT 0x80DF

/*************************************************************/

GLAPI void APIENTRY glActiveTexture (GLenum texture);
GLAPI void APIENTRY glAlphaFunc (GLenum func, GLclampf ref);
GLAPI void APIENTRY glBegin(GLenum mode);
GLAPI void APIENTRY glBindTexture (GLenum target, GLuint texture);
GLAPI void APIENTRY glBitmap (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
GLAPI void APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor);
GLAPI void APIENTRY glCallLists (GLsizei n, GLenum type, const GLvoid *lists);
GLAPI void APIENTRY glClear (GLbitfield mask);
GLAPI void APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
GLAPI void APIENTRY glClearDepthf (GLclampf depth);
GLAPI void APIENTRY glClearStencil (GLint s);
GLAPI void APIENTRY glClientActiveTexture (GLenum texture);
GLAPI void APIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GLAPI void APIENTRY glColor4fv (const GLfloat *v);
GLAPI void APIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
GLAPI void APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
GLAPI void APIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void APIENTRY glColorSubTableEXT (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *table);
GLAPI void APIENTRY glColorTableEXT (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
GLAPI void APIENTRY glCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
GLAPI void APIENTRY glCullFace (GLenum mode);
GLAPI void APIENTRY glDepthFunc (GLenum func);
GLAPI void APIENTRY glDepthMask (GLboolean flag);
GLAPI void APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar);
GLAPI void APIENTRY glDisable (GLenum cap);
GLAPI void APIENTRY glDisableClientState (GLenum array);
GLAPI void APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count);
GLAPI void APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
GLAPI void APIENTRY glDrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void APIENTRY glEnable (GLenum cap);
GLAPI void APIENTRY glEnableClientState (GLenum array);
GLAPI void APIENTRY glEnd (void);
GLAPI void APIENTRY glEndList (void);
GLAPI void APIENTRY glFinish (void);
GLAPI void APIENTRY glFlush (void);
GLAPI void APIENTRY glFrontFace (GLenum mode);
GLAPI void APIENTRY glFrustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
GLAPI GLuint APIENTRY glGenLists (GLsizei range);
GLAPI void APIENTRY glGenTextures (GLsizei n, GLuint *textures);
GLAPI GLenum APIENTRY glGetError (void);
GLAPI void APIENTRY glGetBooleanv (GLenum pname, GLboolean *params);
GLAPI void APIENTRY glGetColorTableEXT (GLenum target, GLenum format, GLenum type, GLvoid *table);
GLAPI void APIENTRY glGetColorTableParameterivEXT (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY glGetFloatv (GLenum pname, GLfloat *params);
GLAPI void APIENTRY glGetIntegerv (GLenum pname, GLint *params);
GLAPI void APIENTRY glGetLightfv (GLenum light, GLenum pname, GLfloat *params);
GLAPI void APIENTRY glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params);
GLAPI void APIENTRY glGetPointerv (GLenum pname, GLvoid * *params);
GLAPI void APIENTRY glGetPolygonStipple (GLubyte *mask);
GLAPI void APIENTRY glGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params);
GLAPI void APIENTRY glGetTexEnviv (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
GLAPI const GLubyte * APIENTRY glGetString (GLenum name);
GLAPI void APIENTRY glHint (GLenum target, GLenum mode);
GLAPI GLboolean APIENTRY glIsEnabled (GLenum cap);
GLAPI void APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params);
GLAPI void APIENTRY glLightModelfv (GLenum pname, const GLfloat *params);
GLAPI void APIENTRY glLineStipple (GLint factor, GLushort pattern);
GLAPI void APIENTRY glLineWidth (GLfloat width);
GLAPI void APIENTRY glListBase (GLuint base);
GLAPI void APIENTRY glLoadIdentity (void);
GLAPI void APIENTRY glLoadMatrixf (const GLfloat *m);
GLAPI void APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param);
GLAPI void APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params);
GLAPI void APIENTRY glMatrixMode (GLenum mode);
GLAPI void APIENTRY glMultMatrixf (const GLfloat *m);
GLAPI void APIENTRY glMultiTexCoord2f (GLenum target, GLfloat s, GLfloat t);
GLAPI void APIENTRY glMultiTexCoord2fv (GLenum target, const GLfloat *v);
GLAPI void APIENTRY glNewList (GLuint list, GLenum mode);
GLAPI void APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz);
GLAPI void APIENTRY glNormal3fv (const GLfloat *v);
GLAPI void APIENTRY glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void APIENTRY glOrthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
GLAPI void APIENTRY glPixelStorei (GLenum pname, GLint param);
GLAPI void APIENTRY glPointSize (GLfloat size);
GLAPI void APIENTRY glPolygonOffset (GLfloat factor, GLfloat units);
GLAPI void APIENTRY glPolygonStipple (const GLubyte *mask);
GLAPI void APIENTRY glPopMatrix (void);
GLAPI void APIENTRY glPushMatrix (void);
GLAPI void APIENTRY glRasterPos3f (GLfloat x, GLfloat y, GLfloat z);
GLAPI void APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
GLAPI void APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
GLAPI void APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z);
GLAPI void APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
GLAPI void APIENTRY glShadeModel (GLenum mode);
GLAPI void APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask);
GLAPI void APIENTRY glStencilMask (GLuint mask);
GLAPI void APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass);
GLAPI void APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void APIENTRY glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params);
GLAPI void APIENTRY glTexEnvi (GLenum target, GLenum pname, GLint param);
GLAPI void APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param);
GLAPI void APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z);
GLAPI void APIENTRY glVertex2f (GLfloat x, GLfloat y);
GLAPI void APIENTRY glVertex2fv (const GLfloat *v);
GLAPI void APIENTRY glVertex3f (GLfloat x, GLfloat y, GLfloat z);
GLAPI void APIENTRY glVertex3fv (const GLfloat *v);
GLAPI void APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height);

#ifdef __cplusplus
}
#endif

#endif /* __gl_h_ */


