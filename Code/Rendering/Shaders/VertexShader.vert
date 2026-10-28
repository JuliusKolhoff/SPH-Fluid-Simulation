#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;

out vec4 Color;

void main()
{
    gl_Position = aPos;
    Color = aColor;
}