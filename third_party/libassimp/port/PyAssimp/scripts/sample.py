#!/usr/bin/env python
#-*- coding: UTF-8 -*-

"""
This module demonstrates the functionality of PyAssimp.
"""

import os, sys
import logging
logging.basicConfig(level=logging.INFO)

import pyassimp
import pyassimp.postprocess

def recur_node(node,level = 0):
    print("  " + "\t" * level + "- " + str(node))
    for child in node.children:
        recur_node(child, level + 1)


def main(filename=None):

    scene = pyassimp.load(filename, processing=pyassimp.postprocess.aiProcess_Triangulate)
    
    #the model we load
    print("MODEL:" + filename)
    print
    
    #write some statistics
    print("SCENE:")
    print("  meshes:" + str(len(scene.meshes)))
    print("  materials:" + str(len(scene.materials)))
    print("  textures:" + str(len(scene.textures)))
    print
    
    print("NODES:")
    recur_node(scene.rootnode)

    print
    print("MESHES:")
    for index, mesh in enumerate(scene.meshes):
        print("  MESH" + str(index+1))
        print("    material id:" + str(mesh.materialindex+1))
        print("    vertices:" + str(len(mesh.vertices)))
        print("    first 3 verts:\n" + str(mesh.vertices[:3]))
        if mesh.normals.any():
                print("    first 3 normals:\n" + str(mesh.normals[:3]))
        else:
                print("    no normals")
        print("    colors:" + str(len(mesh.colors)))
        tcs = mesh.texturecoords
        if tcs.any():
            for index, tc in enumerate(tcs):
                print("    texture-coords "+ str(index) + ":" + str(len(tcs[index])) + "first3:" + str(tcs[index][:3]))

        else:
            print("    no texture coordinates")
        print("    uv-component-count:" + str(len(mesh.numuvcomponents)))
        print("    faces:" + str(len(mesh.faces)) + " -> first:\n" + str(mesh.faces[:3]))
        print("    bones:" + str(len(mesh.bones)) + " -> first:" + str([str(b) for b in mesh.bones[:3]]))
        print

    print("MATERIALS:")
    for index, material in enumerate(scene.materials):
        print("  MATERIAL (id:" + str(index+1) + ")")
        for key, value in material.properties.items():
            print("    %s: %s" % (key, value))
    print
    
    print("TEXTURES:")
    for index, texture in enumerate(scene.textures):
        print("  TEXTURE" + str(index+1))
        print("    width:" + str(texture.width))
        print("    height:" + str(texture.height))
        print("    hint:" + str(texture.achformathint))
        print("    data (size):" + str(len(texture.data)))
   
    # Finally release the model
    pyassimp.release(scene)

def usage():
    print("Usage: sample.py <3d model>")

if __name__ == "__main__":

    if len(sys.argv) != 2:
        usage()
    else:
        main(sys.argv[1])
