#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 Color;

uniform vec2 u_resolution;
uniform float u_pixels_per_meter;

void main()
{  
    //*2 because from -1 to 1 so 9*100*2/1800 = -1 (negative nand posiive half)
    //bzw. width = 2(-1 to 1) in open GL so 2 to correct ratio
    vec3 pos = (aPos*u_pixels_per_meter*2)/vec3(u_resolution,1.0f)-vec3(1,1,0);

    gl_Position = vec4(pos.x,pos.y,aPos.z,1.0f);
    Color = aColor;
}