#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 Color;

uniform vec2 u_resolution;

void main()
{  
    vec3 pos = vec3(((aPos.xy/u_resolution) * vec2(2.0f) - vec2(1.0f)),aPos.z);
    gl_Position = vec4(pos,1.0f);
    Color = aColor;
}