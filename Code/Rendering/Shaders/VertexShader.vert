#version 330 core
layout (location = 0) in vec4 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y - 1.0, aPos.z, 1.0); // y in [0,2] -> [-1,1], floor at the bottom
    gl_PointSize = 2.0;
}