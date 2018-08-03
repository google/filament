#!/usr/bin/env python
# -*- coding: UTF-8 -*-

""" This program loads a model with PyASSIMP, and display it.

Based on:
- pygame code from http://3dengine.org/Spectator_%28PyOpenGL%29
- http://www.lighthouse3d.com/tutorials
- http://www.songho.ca/opengl/gl_transform.html
- http://code.activestate.com/recipes/325391/
- ASSIMP's C++ SimpleOpenGL viewer

Authors: Séverin Lemaignan, 2012-2016
"""
import sys
import logging

from functools import reduce

logger = logging.getLogger("pyassimp")
gllogger = logging.getLogger("OpenGL")
gllogger.setLevel(logging.WARNING)
logging.basicConfig(level=logging.INFO)

import OpenGL

OpenGL.ERROR_CHECKING = False
OpenGL.ERROR_LOGGING = False
# OpenGL.ERROR_ON_COPY = True
# OpenGL.FULL_LOGGING = True
from OpenGL.GL import *
from OpenGL.arrays import vbo
from OpenGL.GL import shaders

import pygame
import pygame.font
import pygame.image

import math, random
from numpy import linalg

import pyassimp
from pyassimp.postprocess import *
from pyassimp.helper import *
import transformations

ROTATION_180_X = numpy.array([[1, 0, 0, 0], [0, -1, 0, 0], [0, 0, -1, 0], [0, 0, 0, 1]], dtype=numpy.float32)

# rendering mode
BASE = "BASE"
COLORS = "COLORS"
SILHOUETTE = "SILHOUETTE"
HELPERS = "HELPERS"

# Entities type
ENTITY = "entity"
CAMERA = "camera"
MESH = "mesh"

FLAT_VERTEX_SHADER_120 = """
#version 120

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelMatrix;

uniform vec4 u_materialDiffuse;

attribute vec3 a_vertex;

varying vec4 v_color;

void main(void)
{
    v_color = u_materialDiffuse;
    gl_Position = u_viewProjectionMatrix * u_modelMatrix * vec4(a_vertex, 1.0);
}
"""

FLAT_VERTEX_SHADER_130 = """
#version 130

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelMatrix;

uniform vec4 u_materialDiffuse;

in vec3 a_vertex;

out vec4 v_color;

void main(void)
{
    v_color = u_materialDiffuse;
    gl_Position = u_viewProjectionMatrix * u_modelMatrix * vec4(a_vertex, 1.0);
}
"""

BASIC_VERTEX_SHADER_120 = """
#version 120

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;
uniform vec3 u_lightPos;

uniform vec4 u_materialDiffuse;

attribute vec3 a_vertex;
attribute vec3 a_normal;

varying vec4 v_color;

void main(void)
{
    // Now the normal is in world space, as we pass the light in world space.
    vec3 normal = u_normalMatrix * a_normal;

    float dist = distance(a_vertex, u_lightPos);

    // go to https://www.desmos.com/calculator/nmnaud1hrw to play with the parameters
    // att is not used for now
    float att=1.0/(1.0+0.8*dist*dist);

    vec3 surf2light = normalize(u_lightPos - a_vertex);
    vec3 norm = normalize(normal);
    float dcont=max(0.0,dot(norm,surf2light));

    float ambient = 0.3;
    float intensity = dcont + 0.3 + ambient;

    v_color = u_materialDiffuse  * intensity;

    gl_Position = u_viewProjectionMatrix * u_modelMatrix * vec4(a_vertex, 1.0);
}
"""

BASIC_VERTEX_SHADER_130 = """
#version 130

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;
uniform vec3 u_lightPos;

uniform vec4 u_materialDiffuse;

in vec3 a_vertex;
in vec3 a_normal;

out vec4 v_color;

void main(void)
{
    // Now the normal is in world space, as we pass the light in world space.
    vec3 normal = u_normalMatrix * a_normal;

    float dist = distance(a_vertex, u_lightPos);

    // go to https://www.desmos.com/calculator/nmnaud1hrw to play with the parameters
    // att is not used for now
    float att=1.0/(1.0+0.8*dist*dist);

    vec3 surf2light = normalize(u_lightPos - a_vertex);
    vec3 norm = normalize(normal);
    float dcont=max(0.0,dot(norm,surf2light));

    float ambient = 0.3;
    float intensity = dcont + 0.3 + ambient;

    v_color = u_materialDiffuse  * intensity;

    gl_Position = u_viewProjectionMatrix * u_modelMatrix * vec4(a_vertex, 1.0);
}
"""

BASIC_FRAGMENT_SHADER_120 = """
#version 120

varying vec4 v_color;

void main() {
    gl_FragColor = v_color;
}
"""

BASIC_FRAGMENT_SHADER_130 = """
#version 130

in vec4 v_color;

void main() {
    gl_FragColor = v_color;
}
"""

GOOCH_VERTEX_SHADER_120 = """
#version 120

// attributes
attribute vec3 a_vertex; // xyz - position
attribute vec3 a_normal; // xyz - normal

// uniforms
uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform mat3 u_normalMatrix;
uniform vec3 u_lightPos;
uniform vec3 u_camPos;

// output data from vertex to fragment shader
varying vec3 o_normal;
varying vec3 o_lightVector;

///////////////////////////////////////////////////////////////////

void main(void)
{
   // transform position and normal to world space
   vec4 positionWorld = u_modelMatrix * vec4(a_vertex, 1.0);
   vec3 normalWorld = u_normalMatrix * a_normal;

   // calculate and pass vectors required for lighting
   o_lightVector = u_lightPos - positionWorld.xyz;
   o_normal = normalWorld;

   // project world space position to the screen and output it
   gl_Position = u_viewProjectionMatrix * positionWorld;
}
"""

GOOCH_VERTEX_SHADER_130 = """
#version 130

// attributes
in vec3 a_vertex; // xyz - position
in vec3 a_normal; // xyz - normal

// uniforms
uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform mat3 u_normalMatrix;
uniform vec3 u_lightPos;
uniform vec3 u_camPos;

// output data from vertex to fragment shader
out vec3 o_normal;
out vec3 o_lightVector;

///////////////////////////////////////////////////////////////////

void main(void)
{
   // transform position and normal to world space
   vec4 positionWorld = u_modelMatrix * vec4(a_vertex, 1.0);
   vec3 normalWorld = u_normalMatrix * a_normal;

   // calculate and pass vectors required for lighting
   o_lightVector = u_lightPos - positionWorld.xyz;
   o_normal = normalWorld;

   // project world space position to the screen and output it
   gl_Position = u_viewProjectionMatrix * positionWorld;
}
"""

GOOCH_FRAGMENT_SHADER_120 = """
#version 120

// data from vertex shader
varying vec3 o_normal;
varying vec3 o_lightVector;

// diffuse color of the object
uniform vec4 u_materialDiffuse;
// cool color of gooch shading
uniform vec3 u_coolColor;
// warm color of gooch shading
uniform vec3 u_warmColor;
// how much to take from object color in final cool color
uniform float u_alpha;
// how much to take from object color in final warm color
uniform float u_beta;

///////////////////////////////////////////////////////////

void main(void)
{
   // normlize vectors for lighting
   vec3 normalVector = normalize(o_normal);
   vec3 lightVector = normalize(o_lightVector);
   // intensity of diffuse lighting [-1, 1]
   float diffuseLighting = dot(lightVector, normalVector);
   // map intensity of lighting from range [-1; 1] to [0, 1]
   float interpolationValue = (1.0 + diffuseLighting)/2;

   //////////////////////////////////////////////////////////////////

   // cool color mixed with color of the object
   vec3 coolColorMod = u_coolColor + vec3(u_materialDiffuse) * u_alpha;
   // warm color mixed with color of the object
   vec3 warmColorMod = u_warmColor + vec3(u_materialDiffuse) * u_beta;
   // interpolation of cool and warm colors according
   // to lighting intensity. The lower the light intensity,
   // the larger part of the cool color is used
   vec3 colorOut = mix(coolColorMod, warmColorMod, interpolationValue);

   //////////////////////////////////////////////////////////////////

   // save color
   gl_FragColor.rgb = colorOut;
   gl_FragColor.a = 1;
}
"""

GOOCH_FRAGMENT_SHADER_130 = """
#version 130

// data from vertex shader
in vec3 o_normal;
in vec3 o_lightVector;

// diffuse color of the object
uniform vec4 u_materialDiffuse;
// cool color of gooch shading
uniform vec3 u_coolColor;
// warm color of gooch shading
uniform vec3 u_warmColor;
// how much to take from object color in final cool color
uniform float u_alpha;
// how much to take from object color in final warm color
uniform float u_beta;

// output to framebuffer
out vec4 resultingColor;

///////////////////////////////////////////////////////////

void main(void)
{
   // normlize vectors for lighting
   vec3 normalVector = normalize(o_normal);
   vec3 lightVector = normalize(o_lightVector);
   // intensity of diffuse lighting [-1, 1]
   float diffuseLighting = dot(lightVector, normalVector);
   // map intensity of lighting from range [-1; 1] to [0, 1]
   float interpolationValue = (1.0 + diffuseLighting)/2;

   //////////////////////////////////////////////////////////////////

   // cool color mixed with color of the object
   vec3 coolColorMod = u_coolColor + vec3(u_materialDiffuse) * u_alpha;
   // warm color mixed with color of the object
   vec3 warmColorMod = u_warmColor + vec3(u_materialDiffuse) * u_beta;
   // interpolation of cool and warm colors according
   // to lighting intensity. The lower the light intensity,
   // the larger part of the cool color is used
   vec3 colorOut = mix(coolColorMod, warmColorMod, interpolationValue);

   //////////////////////////////////////////////////////////////////

   // save color
   resultingColor.rgb = colorOut;
   resultingColor.a = 1;
}
"""

SILHOUETTE_VERTEX_SHADER_120 = """
#version 120

attribute vec3 a_vertex; // xyz - position
attribute vec3 a_normal; // xyz - normal

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform vec4 u_materialDiffuse;
uniform float u_bordersize; // width of the border

varying vec4 v_color;

void main(void){
   v_color = u_materialDiffuse;
   float distToCamera = -(u_modelViewMatrix * vec4(a_vertex, 1.0)).z;
   vec4 tPos   = vec4(a_vertex + a_normal * u_bordersize * distToCamera, 1.0);
   gl_Position = u_viewProjectionMatrix * u_modelMatrix * tPos;
}
"""

SILHOUETTE_VERTEX_SHADER_130 = """
#version 130

in vec3 a_vertex; // xyz - position
in vec3 a_normal; // xyz - normal

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform vec4 u_materialDiffuse;
uniform float u_bordersize; // width of the border

out vec4 v_color;

void main(void){
   v_color = u_materialDiffuse;
   float distToCamera = -(u_modelViewMatrix * vec4(a_vertex, 1.0)).z;
   vec4 tPos   = vec4(a_vertex + a_normal * u_bordersize * distToCamera, 1.0);
   gl_Position = u_viewProjectionMatrix * u_modelMatrix * tPos;
}
"""
DEFAULT_CLIP_PLANE_NEAR = 0.001
DEFAULT_CLIP_PLANE_FAR = 1000.0


def get_world_transform(scene, node):
    if node == scene.rootnode:
        return numpy.identity(4, dtype=numpy.float32)

    parents = reversed(_get_parent_chain(scene, node, []))
    parent_transform = reduce(numpy.dot, [p.transformation for p in parents])
    return numpy.dot(parent_transform, node.transformation)


def _get_parent_chain(scene, node, parents):
    parent = node.parent

    parents.append(parent)

    if parent == scene.rootnode:
        return parents

    return _get_parent_chain(scene, parent, parents)


class DefaultCamera:
    def __init__(self, w, h, fov):
        self.name = "default camera"
        self.type = CAMERA
        self.clipplanenear = DEFAULT_CLIP_PLANE_NEAR
        self.clipplanefar = DEFAULT_CLIP_PLANE_FAR
        self.aspect = w / h
        self.horizontalfov = fov * math.pi / 180
        self.transformation = numpy.array([[0.68, -0.32, 0.65, 7.48],
                                           [0.73, 0.31, -0.61, -6.51],
                                           [-0.01, 0.89, 0.44, 5.34],
                                           [0., 0., 0., 1.]], dtype=numpy.float32)

        self.transformation = numpy.dot(self.transformation, ROTATION_180_X)

    def __str__(self):
        return self.name


class PyAssimp3DViewer:
    base_name = "PyASSIMP 3D viewer"

    def __init__(self, model, w=1024, h=768):

        self.w = w
        self.h = h

        pygame.init()
        pygame.display.set_caption(self.base_name)
        pygame.display.set_mode((w, h), pygame.OPENGL | pygame.DOUBLEBUF)

        glClearColor(0.18, 0.18, 0.18, 1.0)

        shader_compilation_succeeded = False
        try:
            self.set_shaders_v130()
            self.prepare_shaders()
        except RuntimeError, message:
            sys.stderr.write("%s\n" % message)
            sys.stdout.write("Could not compile shaders in version 1.30, trying version 1.20\n")

        if not shader_compilation_succeeded:
            self.set_shaders_v120()
            self.prepare_shaders()

        self.scene = None
        self.meshes = {}  # stores the OpenGL vertex/faces/normals buffers pointers

        self.node2colorid = {}  # stores a color ID for each node. Useful for mouse picking and visibility checking
        self.colorid2node = {}  # reverse dict of node2colorid

        self.currently_selected = None
        self.moving = False
        self.moving_situation = None

        self.default_camera = DefaultCamera(self.w, self.h, fov=70)
        self.cameras = [self.default_camera]

        self.current_cam_index = 0
        self.current_cam = self.default_camera
        self.set_camera_projection()

        self.load_model(model)

        # user interactions
        self.focal_point = [0, 0, 0]
        self.is_rotating = False
        self.is_panning = False
        self.is_zooming = False

    def set_shaders_v120(self):
      self.BASIC_VERTEX_SHADER = BASIC_VERTEX_SHADER_120
      self.FLAT_VERTEX_SHADER = FLAT_VERTEX_SHADER_120
      self.SILHOUETTE_VERTEX_SHADER = SILHOUETTE_VERTEX_SHADER_120
      self.GOOCH_VERTEX_SHADER = GOOCH_VERTEX_SHADER_120

      self.BASIC_FRAGMENT_SHADER = BASIC_FRAGMENT_SHADER_120
      self.GOOCH_FRAGMENT_SHADER = GOOCH_FRAGMENT_SHADER_120

    def set_shaders_v130(self):
      self.BASIC_VERTEX_SHADER = BASIC_VERTEX_SHADER_130
      self.FLAT_VERTEX_SHADER = FLAT_VERTEX_SHADER_130
      self.SILHOUETTE_VERTEX_SHADER = SILHOUETTE_VERTEX_SHADER_130
      self.GOOCH_VERTEX_SHADER = GOOCH_VERTEX_SHADER_130

      self.BASIC_FRAGMENT_SHADER = BASIC_FRAGMENT_SHADER_130
      self.GOOCH_FRAGMENT_SHADER = GOOCH_FRAGMENT_SHADER_130

    def prepare_shaders(self):

        ### Base shader
        vertex = shaders.compileShader(self.BASIC_VERTEX_SHADER, GL_VERTEX_SHADER)
        fragment = shaders.compileShader(self.BASIC_FRAGMENT_SHADER, GL_FRAGMENT_SHADER)

        self.shader = shaders.compileProgram(vertex, fragment)

        self.set_shader_accessors(('u_modelMatrix',
                                   'u_viewProjectionMatrix',
                                   'u_normalMatrix',
                                   'u_lightPos',
                                   'u_materialDiffuse'),
                                  ('a_vertex',
                                   'a_normal'), self.shader)

        ### Flat shader
        flatvertex = shaders.compileShader(self.FLAT_VERTEX_SHADER, GL_VERTEX_SHADER)
        self.flatshader = shaders.compileProgram(flatvertex, fragment)

        self.set_shader_accessors(('u_modelMatrix',
                                   'u_viewProjectionMatrix',
                                   'u_materialDiffuse',),
                                  ('a_vertex',), self.flatshader)

        ### Silhouette shader
        silh_vertex = shaders.compileShader(self.SILHOUETTE_VERTEX_SHADER, GL_VERTEX_SHADER)
        self.silhouette_shader = shaders.compileProgram(silh_vertex, fragment)

        self.set_shader_accessors(('u_modelMatrix',
                                   'u_viewProjectionMatrix',
                                   'u_modelViewMatrix',
                                   'u_materialDiffuse',
                                   'u_bordersize'  # width of the silhouette
                                   ),
                                  ('a_vertex',
                                   'a_normal'), self.silhouette_shader)

        ### Gooch shader
        gooch_vertex = shaders.compileShader(self.GOOCH_VERTEX_SHADER, GL_VERTEX_SHADER)
        gooch_fragment = shaders.compileShader(self.GOOCH_FRAGMENT_SHADER, GL_FRAGMENT_SHADER)
        self.gooch_shader = shaders.compileProgram(gooch_vertex, gooch_fragment)

        self.set_shader_accessors(('u_modelMatrix',
                                   'u_viewProjectionMatrix',
                                   'u_normalMatrix',
                                   'u_lightPos',
                                   'u_materialDiffuse',
                                   'u_coolColor',
                                   'u_warmColor',
                                   'u_alpha',
                                   'u_beta'
                                   ),
                                  ('a_vertex',
                                   'a_normal'), self.gooch_shader)

    @staticmethod
    def set_shader_accessors(uniforms, attributes, shader):
        # add accessors to the shaders uniforms and attributes
        for uniform in uniforms:
            location = glGetUniformLocation(shader, uniform)
            if location in (None, -1):
                raise RuntimeError('No uniform: %s (maybe it is not used '
                                   'anymore and has been optimized out by'
                                   ' the shader compiler)' % uniform)
            setattr(shader, uniform, location)

        for attribute in attributes:
            location = glGetAttribLocation(shader, attribute)
            if location in (None, -1):
                raise RuntimeError('No attribute: %s' % attribute)
            setattr(shader, attribute, location)

    @staticmethod
    def prepare_gl_buffers(mesh):

        mesh.gl = {}

        # Fill the buffer for vertex and normals positions
        v = numpy.array(mesh.vertices, 'f')
        n = numpy.array(mesh.normals, 'f')

        mesh.gl["vbo"] = vbo.VBO(numpy.hstack((v, n)))

        # Fill the buffer for vertex positions
        mesh.gl["faces"] = glGenBuffers(1)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.gl["faces"])
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     numpy.array(mesh.faces, dtype=numpy.int32),
                     GL_STATIC_DRAW)

        mesh.gl["nbfaces"] = len(mesh.faces)

        # Unbind buffers
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)

    @staticmethod
    def get_rgb_from_colorid(colorid):
        r = (colorid >> 0) & 0xff
        g = (colorid >> 8) & 0xff
        b = (colorid >> 16) & 0xff

        return r, g, b

    def get_color_id(self):
        id = random.randint(0, 256 * 256 * 256)
        if id not in self.colorid2node:
            return id
        else:
            return self.get_color_id()

    def glize(self, scene, node):

        logger.info("Loading node <%s>" % node)
        node.selected = True if self.currently_selected and self.currently_selected == node else False

        node.transformation = node.transformation.astype(numpy.float32)

        if node.meshes:
            node.type = MESH
            colorid = self.get_color_id()
            self.colorid2node[colorid] = node
            self.node2colorid[node.name] = colorid

        elif node.name in [c.name for c in scene.cameras]:

            # retrieve the ASSIMP camera object
            [cam] = [c for c in scene.cameras if c.name == node.name]
            node.type = CAMERA
            logger.info("Added camera <%s>" % node.name)
            logger.info("Camera position: %.3f, %.3f, %.3f" % tuple(node.transformation[:, 3][:3].tolist()))
            self.cameras.append(node)
            node.clipplanenear = cam.clipplanenear
            node.clipplanefar = cam.clipplanefar

            if numpy.allclose(cam.lookat, [0, 0, -1]) and numpy.allclose(cam.up, [0, 1, 0]):  # Cameras in .blend files

                # Rotate by 180deg around X to have Z pointing forward
                node.transformation = numpy.dot(node.transformation, ROTATION_180_X)
            else:
                raise RuntimeError(
                    "I do not know how to normalize this camera orientation: lookat=%s, up=%s" % (cam.lookat, cam.up))

            if cam.aspect == 0.0:
                logger.warning("Camera aspect not set. Setting to default 4:3")
                node.aspect = 1.333
            else:
                node.aspect = cam.aspect

            node.horizontalfov = cam.horizontalfov

        else:
            node.type = ENTITY

        for child in node.children:
            self.glize(scene, child)

    def load_model(self, path, postprocess=aiProcessPreset_TargetRealtime_MaxQuality):
        logger.info("Loading model:" + path + "...")

        if postprocess:
            self.scene = pyassimp.load(path, processing=postprocess)
        else:
            self.scene = pyassimp.load(path)
        logger.info("Done.")

        scene = self.scene
        # log some statistics
        logger.info("  meshes: %d" % len(scene.meshes))
        logger.info("  total faces: %d" % sum([len(mesh.faces) for mesh in scene.meshes]))
        logger.info("  materials: %d" % len(scene.materials))
        self.bb_min, self.bb_max = get_bounding_box(self.scene)
        logger.info("  bounding box:" + str(self.bb_min) + " - " + str(self.bb_max))

        self.scene_center = [(a + b) / 2. for a, b in zip(self.bb_min, self.bb_max)]

        for index, mesh in enumerate(scene.meshes):
            self.prepare_gl_buffers(mesh)

        self.glize(scene, scene.rootnode)

        # Finally release the model
        pyassimp.release(scene)
        logger.info("Ready for 3D rendering!")

    def cycle_cameras(self):

        self.current_cam_index = (self.current_cam_index + 1) % len(self.cameras)
        self.current_cam = self.cameras[self.current_cam_index]
        self.set_camera_projection(self.current_cam)
        logger.info("Switched to camera <%s>" % self.current_cam)

    def set_overlay_projection(self):
        glViewport(0, 0, self.w, self.h)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(0.0, self.w - 1.0, 0.0, self.h - 1.0, -1.0, 1.0)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def set_camera_projection(self, camera=None):

        if not camera:
            camera = self.current_cam

        znear = camera.clipplanenear or DEFAULT_CLIP_PLANE_NEAR
        zfar = camera.clipplanefar or DEFAULT_CLIP_PLANE_FAR
        aspect = camera.aspect
        fov = camera.horizontalfov

        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()

        # Compute gl frustrum
        tangent = math.tan(fov / 2.)
        h = znear * tangent
        w = h * aspect

        # params: left, right, bottom, top, near, far
        glFrustum(-w, w, -h, h, znear, zfar)
        # equivalent to:
        # gluPerspective(fov * 180/math.pi, aspect, znear, zfar)

        self.projection_matrix = glGetFloatv(GL_PROJECTION_MATRIX).transpose()

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def render_colors(self):

        glEnable(GL_DEPTH_TEST)
        glDepthFunc(GL_LEQUAL)

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)
        glEnable(GL_CULL_FACE)

        glUseProgram(self.flatshader)

        glUniformMatrix4fv(self.flatshader.u_viewProjectionMatrix, 1, GL_TRUE,
                           numpy.dot(self.projection_matrix, self.view_matrix))

        self.recursive_render(self.scene.rootnode, self.flatshader, mode=COLORS)

        glUseProgram(0)

    def get_hovered_node(self, mousex, mousey):
        """
        Attention: The performances of this method relies heavily on the size of the display!
        """

        # mouse out of the window?
        if mousex < 0 or mousex >= self.w or mousey < 0 or mousey >= self.h:
            return None

        self.render_colors()
        # Capture image from the OpenGL buffer
        buf = (GLubyte * (3 * self.w * self.h))(0)
        glReadPixels(0, 0, self.w, self.h, GL_RGB, GL_UNSIGNED_BYTE, buf)

        # Reinterpret the RGB pixel buffer as a 1-D array of 24bits colors
        a = numpy.ndarray(len(buf), numpy.dtype('>u1'), buf)
        colors = numpy.zeros(len(buf) // 3, numpy.dtype('<u4'))
        for i in range(3):
            colors.view(dtype='>u1')[i::4] = a.view(dtype='>u1')[i::3]

        colorid = colors[mousex + mousey * self.w]

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        if colorid in self.colorid2node:
            return self.colorid2node[colorid]

    def render(self, wireframe=False, twosided=False):

        glEnable(GL_DEPTH_TEST)
        glDepthFunc(GL_LEQUAL)

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE if wireframe else GL_FILL)
        glDisable(GL_CULL_FACE) if twosided else glEnable(GL_CULL_FACE)

        self.render_grid()

        self.recursive_render(self.scene.rootnode, None, mode=HELPERS)

        ### First, the silhouette

        if False:
            shader = self.silhouette_shader

            # glDepthMask(GL_FALSE)
            glCullFace(GL_FRONT)  # cull front faces

            glUseProgram(shader)
            glUniform1f(shader.u_bordersize, 0.01)

            glUniformMatrix4fv(shader.u_viewProjectionMatrix, 1, GL_TRUE,
                               numpy.dot(self.projection_matrix, self.view_matrix))

            self.recursive_render(self.scene.rootnode, shader, mode=SILHOUETTE)

            glUseProgram(0)

        ### Then, inner shading
        # glDepthMask(GL_TRUE)
        glCullFace(GL_BACK)

        use_gooch = False
        if use_gooch:
            shader = self.gooch_shader

            glUseProgram(shader)
            glUniform3f(shader.u_lightPos, -.5, -.5, .5)

            ##### GOOCH specific
            glUniform3f(shader.u_coolColor, 159.0 / 255, 148.0 / 255, 255.0 / 255)
            glUniform3f(shader.u_warmColor, 255.0 / 255, 75.0 / 255, 75.0 / 255)
            glUniform1f(shader.u_alpha, .25)
            glUniform1f(shader.u_beta, .25)
            #########
        else:
            shader = self.shader
            glUseProgram(shader)
            glUniform3f(shader.u_lightPos, -.5, -.5, .5)

        glUniformMatrix4fv(shader.u_viewProjectionMatrix, 1, GL_TRUE,
                           numpy.dot(self.projection_matrix, self.view_matrix))

        self.recursive_render(self.scene.rootnode, shader)

        glUseProgram(0)

    def render_axis(self,
                    transformation=numpy.identity(4, dtype=numpy.float32),
                    label=None,
                    size=0.2,
                    selected=False):
        m = transformation.transpose()  # OpenGL row major

        glPushMatrix()
        glMultMatrixf(m)

        glLineWidth(3 if selected else 1)

        size = 2 * size if selected else size

        glBegin(GL_LINES)

        # draw line for x axis
        glColor3f(1.0, 0.0, 0.0)
        glVertex3f(0.0, 0.0, 0.0)
        glVertex3f(size, 0.0, 0.0)

        # draw line for y axis
        glColor3f(0.0, 1.0, 0.0)
        glVertex3f(0.0, 0.0, 0.0)
        glVertex3f(0.0, size, 0.0)

        # draw line for Z axis
        glColor3f(0.0, 0.0, 1.0)
        glVertex3f(0.0, 0.0, 0.0)
        glVertex3f(0.0, 0.0, size)

        glEnd()

        if label:
            self.showtext(label)

        glPopMatrix()

    @staticmethod
    def render_camera(camera, transformation):

        m = transformation.transpose()  # OpenGL row major

        aspect = camera.aspect

        u = 0.1  # unit size (in m)
        l = 3 * u  # length of the camera cone
        f = 3 * u  # aperture of the camera cone

        glPushMatrix()
        glMultMatrixf(m)

        glLineWidth(2)
        glBegin(GL_LINE_STRIP)

        glColor3f(.2, .2, .2)

        glVertex3f(u, u, -u)
        glVertex3f(u, -u, -u)
        glVertex3f(-u, -u, -u)
        glVertex3f(-u, u, -u)
        glVertex3f(u, u, -u)

        glVertex3f(u, u, 0.0)
        glVertex3f(u, -u, 0.0)
        glVertex3f(-u, -u, 0.0)
        glVertex3f(-u, u, 0.0)
        glVertex3f(u, u, 0.0)

        glVertex3f(f * aspect, f, l)
        glVertex3f(f * aspect, -f, l)
        glVertex3f(-f * aspect, -f, l)
        glVertex3f(-f * aspect, f, l)
        glVertex3f(f * aspect, f, l)

        glEnd()

        glBegin(GL_LINE_STRIP)
        glVertex3f(u, -u, -u)
        glVertex3f(u, -u, 0.0)
        glVertex3f(f * aspect, -f, l)
        glEnd()

        glBegin(GL_LINE_STRIP)
        glVertex3f(-u, -u, -u)
        glVertex3f(-u, -u, 0.0)
        glVertex3f(-f * aspect, -f, l)
        glEnd()

        glBegin(GL_LINE_STRIP)
        glVertex3f(-u, u, -u)
        glVertex3f(-u, u, 0.0)
        glVertex3f(-f * aspect, f, l)
        glEnd()

        glPopMatrix()

    @staticmethod
    def render_grid():

        glLineWidth(1)
        glColor3f(0.5, 0.5, 0.5)
        glBegin(GL_LINES)
        for i in range(-10, 11):
            glVertex3f(i, -10.0, 0.0)
            glVertex3f(i, 10.0, 0.0)

        for i in range(-10, 11):
            glVertex3f(-10.0, i, 0.0)
            glVertex3f(10.0, i, 0.0)
        glEnd()

    def recursive_render(self, node, shader, mode=BASE, with_normals=True):
        """ Main recursive rendering method.
        """

        normals = with_normals

        if mode == COLORS:
            normals = False


        if not hasattr(node, "selected"):
            node.selected = False

        m = get_world_transform(self.scene, node)

        # HELPERS mode
        ###
        if mode == HELPERS:
            # if node.type == ENTITY:
            self.render_axis(m,
                             label=node.name if node != self.scene.rootnode else None,
                             selected=node.selected if hasattr(node, "selected") else False)

            if node.type == CAMERA:
                self.render_camera(node, m)

            for child in node.children:
                    self.recursive_render(child, shader, mode)

            return

        # Mesh rendering modes
        ###
        if node.type == MESH:

            for mesh in node.meshes:

                stride = 24  # 6 * 4 bytes

                if node.selected and mode == SILHOUETTE:
                    glUniform4f(shader.u_materialDiffuse, 1.0, 0.0, 0.0, 1.0)
                    glUniformMatrix4fv(shader.u_modelViewMatrix, 1, GL_TRUE,
                                       numpy.dot(self.view_matrix, m))

                else:
                    if mode == COLORS:
                        colorid = self.node2colorid[node.name]
                        r, g, b = self.get_rgb_from_colorid(colorid)
                        glUniform4f(shader.u_materialDiffuse, r / 255.0, g / 255.0, b / 255.0, 1.0)
                    elif mode == SILHOUETTE:
                        glUniform4f(shader.u_materialDiffuse, .0, .0, .0, 1.0)
                    else:
                        if node.selected:
                            diffuse = (1.0, 0.0, 0.0, 1.0)  # selected nodes in red
                        else:
                            diffuse = mesh.material.properties["diffuse"]
                        if len(diffuse) == 3:  # RGB instead of expected RGBA
                            diffuse.append(1.0)
                        glUniform4f(shader.u_materialDiffuse, *diffuse)
                        # if ambient:
                        #    glUniform4f( shader.Material_ambient, *mat["ambient"] )

                if mode == BASE:  # not in COLORS or SILHOUETTE
                    normal_matrix = linalg.inv(numpy.dot(self.view_matrix, m)[0:3, 0:3]).transpose()
                    glUniformMatrix3fv(shader.u_normalMatrix, 1, GL_TRUE, normal_matrix)

                glUniformMatrix4fv(shader.u_modelMatrix, 1, GL_TRUE, m)

                vbo = mesh.gl["vbo"]
                vbo.bind()

                glEnableVertexAttribArray(shader.a_vertex)
                if normals:
                    glEnableVertexAttribArray(shader.a_normal)

                glVertexAttribPointer(
                    shader.a_vertex,
                    3, GL_FLOAT, False, stride, vbo
                )

                if normals:
                    glVertexAttribPointer(
                        shader.a_normal,
                        3, GL_FLOAT, False, stride, vbo + 12
                    )

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.gl["faces"])
                glDrawElements(GL_TRIANGLES, mesh.gl["nbfaces"] * 3, GL_UNSIGNED_INT, None)

                vbo.unbind()
                glDisableVertexAttribArray(shader.a_vertex)

                if normals:
                    glDisableVertexAttribArray(shader.a_normal)

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)

        for child in node.children:
            self.recursive_render(child, shader, mode)


    def switch_to_overlay(self):
        glPushMatrix()
        self.set_overlay_projection()

    def switch_from_overlay(self):
        self.set_camera_projection()
        glPopMatrix()

    def select_node(self, node):
        self.currently_selected = node
        self.update_node_select(self.scene.rootnode)

    def update_node_select(self, node):
        if node is self.currently_selected:
            node.selected = True
        else:
            node.selected = False

        for child in node.children:
            self.update_node_select(child)

    def loop(self):

        pygame.display.flip()

        if not self.process_events():
            return False  # ESC has been pressed

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        return True

    def process_events(self):

        LEFT_BUTTON = 1
        MIDDLE_BUTTON = 2
        RIGHT_BUTTON = 3
        WHEEL_UP = 4
        WHEEL_DOWN = 5

        dx, dy = pygame.mouse.get_rel()
        mousex, mousey = pygame.mouse.get_pos()

        zooming_one_shot = False

        ok = True

        for evt in pygame.event.get():
            if evt.type == pygame.MOUSEBUTTONDOWN and evt.button == LEFT_BUTTON:
                hovered = self.get_hovered_node(mousex, self.h - mousey)
                if hovered:
                    if self.currently_selected and self.currently_selected == hovered:
                        self.select_node(None)
                    else:
                        logger.info("Node %s selected" % hovered)
                        self.select_node(hovered)
                else:
                    self.is_rotating = True
            if evt.type == pygame.MOUSEBUTTONUP and evt.button == LEFT_BUTTON:
                self.is_rotating = False

            if evt.type == pygame.MOUSEBUTTONDOWN and evt.button == MIDDLE_BUTTON:
                self.is_panning = True
            if evt.type == pygame.MOUSEBUTTONUP and evt.button == MIDDLE_BUTTON:
                self.is_panning = False

            if evt.type == pygame.MOUSEBUTTONDOWN and evt.button == RIGHT_BUTTON:
                self.is_zooming = True
            if evt.type == pygame.MOUSEBUTTONUP and evt.button == RIGHT_BUTTON:
                self.is_zooming = False

            if evt.type == pygame.MOUSEBUTTONDOWN and evt.button in [WHEEL_UP, WHEEL_DOWN]:
                zooming_one_shot = True
                self.is_zooming = True
                dy = -10 if evt.button == WHEEL_UP else 10

            if evt.type == pygame.KEYDOWN:
                ok = (ok and self.process_keystroke(evt.key, evt.mod))

        self.controls_3d(dx, dy, zooming_one_shot)

        return ok

    def process_keystroke(self, key, mod):

        # process arrow keys if an object is selected
        if self.currently_selected:
            up = 0
            strafe = 0

            if key == pygame.K_UP:
                up = 1
            if key == pygame.K_DOWN:
                up = -1
            if key == pygame.K_LEFT:
                strafe = -1
            if key == pygame.K_RIGHT:
                strafe = 1

            self.move_selected_node(up, strafe)

        if key == pygame.K_f:
            pygame.display.toggle_fullscreen()

        if key == pygame.K_TAB:
            self.cycle_cameras()

        if key in [pygame.K_ESCAPE, pygame.K_q]:
            return False

        return True

    def controls_3d(self, dx, dy, zooming_one_shot=False):

        CAMERA_TRANSLATION_FACTOR = 0.01
        CAMERA_ROTATION_FACTOR = 0.01

        if not (self.is_rotating or self.is_panning or self.is_zooming):
            return

        current_pos = self.current_cam.transformation[:3, 3].copy()
        distance = numpy.linalg.norm(self.focal_point - current_pos)

        if self.is_rotating:
            """ Orbiting the camera is implemented the following way:

            - the rotation is split into a rotation around the *world* Z axis
              (controlled by the horizontal mouse motion along X) and a
              rotation around the *X* axis of the camera (pitch) *shifted to
              the focal origin* (the world origin for now). This is controlled
              by the vertical motion of the mouse (Y axis).

            - as a result, the resulting transformation of the camera in the
              world frame C' is:
                C' = (T · Rx · T⁻¹ · (Rz · C)⁻¹)⁻¹

              where:
                - C is the original camera transformation in the world frame,
                - Rz is the rotation along the Z axis (in the world frame)
                - T is the translation camera -> world (ie, the inverse of the
                  translation part of C
                - Rx is the rotation around X in the (translated) camera frame
            """

            rotation_camera_x = dy * CAMERA_ROTATION_FACTOR
            rotation_world_z = dx * CAMERA_ROTATION_FACTOR
            world_z_rotation = transformations.euler_matrix(0, 0, rotation_world_z)
            cam_x_rotation = transformations.euler_matrix(rotation_camera_x, 0, 0)

            after_world_z_rotation = numpy.dot(world_z_rotation, self.current_cam.transformation)

            inverse_transformation = transformations.inverse_matrix(after_world_z_rotation)

            translation = transformations.translation_matrix(
                transformations.decompose_matrix(inverse_transformation)[3])
            inverse_translation = transformations.inverse_matrix(translation)

            new_inverse = numpy.dot(inverse_translation, inverse_transformation)
            new_inverse = numpy.dot(cam_x_rotation, new_inverse)
            new_inverse = numpy.dot(translation, new_inverse)

            self.current_cam.transformation = transformations.inverse_matrix(new_inverse).astype(numpy.float32)

        if self.is_panning:
            tx = -dx * CAMERA_TRANSLATION_FACTOR * distance
            ty = dy * CAMERA_TRANSLATION_FACTOR * distance
            cam_transform = transformations.translation_matrix((tx, ty, 0)).astype(numpy.float32)
            self.current_cam.transformation = numpy.dot(self.current_cam.transformation, cam_transform)

        if self.is_zooming:
            tz = dy * CAMERA_TRANSLATION_FACTOR * distance
            cam_transform = transformations.translation_matrix((0, 0, tz)).astype(numpy.float32)
            self.current_cam.transformation = numpy.dot(self.current_cam.transformation, cam_transform)

        if zooming_one_shot:
            self.is_zooming = False

        self.update_view_camera()

    def update_view_camera(self):

        self.view_matrix = linalg.inv(self.current_cam.transformation)

        # Rotate by 180deg around X to have Z pointing backward (OpenGL convention)
        self.view_matrix = numpy.dot(ROTATION_180_X, self.view_matrix)

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        glMultMatrixf(self.view_matrix.transpose())

    def move_selected_node(self, up, strafe):
        self.currently_selected.transformation[0][3] += strafe
        self.currently_selected.transformation[2][3] += up

    @staticmethod
    def showtext(text, x=0, y=0, z=0, size=20):

        # TODO: alpha blending does not work...
        # glEnable(GL_BLEND)
        # glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

        font = pygame.font.Font(None, size)
        text_surface = font.render(text, True, (10, 10, 10, 255),
                                  (255 * 0.18, 255 * 0.18, 255 * 0.18, 0))
        text_data = pygame.image.tostring(text_surface, "RGBA", True)
        glRasterPos3d(x, y, z)
        glDrawPixels(text_surface.get_width(),
                     text_surface.get_height(),
                     GL_RGBA, GL_UNSIGNED_BYTE,
                     text_data)

        # glDisable(GL_BLEND)


def main(model, width, height):
    app = PyAssimp3DViewer(model, w=width, h=height)

    clock = pygame.time.Clock()

    while app.loop():

        app.update_view_camera()

        ## Main rendering
        app.render()

        ## GUI text display
        app.switch_to_overlay()
        app.showtext("Active camera: %s" % str(app.current_cam), 10, app.h - 30)
        if app.currently_selected:
            app.showtext("Selected node: %s" % app.currently_selected, 10, app.h - 50)
            pos = app.h - 70

            app.showtext("(%sm, %sm, %sm)" % (app.currently_selected.transformation[0, 3],
                                              app.currently_selected.transformation[1, 3],
                                              app.currently_selected.transformation[2, 3]), 30, pos)

        app.switch_from_overlay()

        # Make sure we do not go over 30fps
        clock.tick(30)

    logger.info("Quitting! Bye bye!")


#########################################################################
#########################################################################

if __name__ == '__main__':
    if not len(sys.argv) > 1:
        print("Usage: " + __file__ + " <model>")
        sys.exit(2)

    main(model=sys.argv[1], width=1024, height=768)
