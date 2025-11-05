#version 330 core
out vec4 FragColor;

uniform vec2 u_resolution;
uniform vec2 u_mousePos;
  
in vec2 center;
in float oRadius;

in vec3 oColor;


void main()
{
	vec3 color = oColor;
	vec2 st = gl_FragCoord.xy/u_resolution;
	if(distance(st,center) > oRadius) discard;
	FragColor = vec4(color,1.0f);
}