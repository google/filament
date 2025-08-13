#version 460

layout(location = 0) out vec4 o;

void main()
{
    // 非 ASCII 文字をテストするコメント (a comment to test non-ASCII characters)
    o = vec4(1.0);
}
