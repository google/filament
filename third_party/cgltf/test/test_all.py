#!/usr/bin/env python

import os, sys
from sys import platform

num_tested = 0
num_errors = 0

def get_executable_path(name):
    if platform == "win32":
        return "build\\Debug\\" + name
    else:
        return "build/" + name

def is_ascii(s):
    return all(ord(c) < 128 for c in s)

def collect_files(path, type, name):
    global num_tested
    global num_errors
    exe = get_executable_path(name)
    for the_file in os.listdir(path):
        file_path = os.path.join(os.path.normpath(path), the_file)
        if os.path.isfile(file_path):
            if the_file.endswith(type):
                num_tested = num_tested +1
                if is_ascii(file_path):
                    print("### " + name + " " + file_path)
                    result = os.system("{0} \"{1}\"".format(exe, file_path))
                    print("### Result: " + str(result) + "\n")
                    if result != 0:
                        num_errors = num_errors + 1
                        print("Error.")
                        sys.exit(1)
        elif os.path.isdir(file_path):
            collect_files(file_path, type, name)

if __name__ == "__main__":
    if not os.path.exists("build/"):
        os.makedirs("build/")
    os.chdir("build/")
    os.system("cmake ..")
    if os.system("cmake --build .") != 0:
        print("Unable to build.")
        exit(1)
    os.chdir("..")
    if not os.path.exists("glTF-Sample-Models/"):
        os.system("git init glTF-Sample-Models")
        os.chdir("glTF-Sample-Models")
        os.system("git remote add origin https://github.com/KhronosGroup/glTF-Sample-Models.git")
        os.system("git config core.sparsecheckout true")
        f = open(".git/info/sparse-checkout", "w+")
        f.write("2.0/*\n")
        f.close()
        os.system("git pull --depth=1 origin main")
        os.chdir("..")
    collect_files("glTF-Sample-Models/2.0/", ".glb", "cgltf_test")
    collect_files("glTF-Sample-Models/2.0/", ".gltf", "cgltf_test")
    collect_files("glTF-Sample-Models/2.0/", ".glb", "test_conversion")
    collect_files("glTF-Sample-Models/2.0/", ".gltf", "test_conversion")
    collect_files("glTF-Sample-Models/2.0/", ".gltf", "test_write")
    collect_files("glTF-Sample-Models/2.0/", ".glb", "test_write_glb")

    result = os.system(get_executable_path("test_math"))
    if result != 0:
        num_errors = num_errors + 1
        print("Error.")
        sys.exit(1)

    result = os.system(get_executable_path("test_strings"))
    if result != 0:
        num_errors = num_errors + 1
        print("Error.")
        sys.exit(1)

    print("Tested files: " + str(num_tested))
    print("Errors: " + str(num_errors))
