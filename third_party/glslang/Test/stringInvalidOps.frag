#version 450
#extension GL_EXT_debug_printf : require

void main() {
    true ? "abd" : "def";
    "o" + "k";
    "o" + "k" * "l";
}
