#!/usr/bin/env python

import os, sys
from sys import platform

num_tested = 0
num_errors = 0

def collect_files(path, type, exe):
    global num_tested
    global num_errors
    for the_file in os.listdir(path):
        file_path = os.path.join(os.path.normpath(path), the_file)
        if os.path.isfile(file_path):
            if the_file.endswith(type):
                num_tested = num_tested +1
                print("### Testing: " + file_path)
                result = 0
                if platform == "win32":
                    result = os.system("build\\Debug\\{0} \"{1}\"".format(exe, file_path))
                else:
                    result = os.system("build/{0} \"{1}\"".format(exe, file_path))
                print("### Result: " + str(result) + "\n")
                if result != 0:
                    num_errors = num_errors + 1
                    print("Error.")
                    sys.exit(1)
        elif os.path.isdir(file_path):
            collect_files(file_path, type, exe)

if __name__ == "__main__":
    if not os.path.exists("build/"):
        os.makedirs("build/")
    os.chdir("build/")
    os.system("cmake ..")
    os.system("cmake --build .")
    os.chdir("..")
    if not os.path.exists("glTF-Sample-Models/"):
        os.system("git init glTF-Sample-Models")
        os.chdir("glTF-Sample-Models")
        os.system("git remote add origin https://github.com/KhronosGroup/glTF-Sample-Models.git")
        os.system("git config core.sparsecheckout true")
        f = open(".git/info/sparse-checkout", "w+")
        f.write("2.0/*\n");
        f.close();
        os.system("git pull --depth=1 origin master")
        os.chdir("..")
    collect_files("glTF-Sample-Models/2.0/", ".glb", "cgltf_test")
    collect_files("glTF-Sample-Models/2.0/", ".gltf", "cgltf_test")
    collect_files("glTF-Sample-Models/2.0/", ".glb", "test_conversion")
    collect_files("glTF-Sample-Models/2.0/", ".gltf", "test_conversion")
    print("Tested files: " + str(num_tested))
    print("Errors: " + str(num_errors))

