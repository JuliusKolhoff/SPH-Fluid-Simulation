#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;

uniform mat4  view;
uniform mat4  projection;
uniform float u_radius;
uniform float u_viewport_h;

out vec4 oColor;

void main() {
    gl_Position  = projection * view * vec4(aPos.xyz, 1.0);
    gl_PointSize = u_viewport_h * projection[1][1] * u_radius / gl_Position.w; // diameter in px
    oColor = aColor;
}