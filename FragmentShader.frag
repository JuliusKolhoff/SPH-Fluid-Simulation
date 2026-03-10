#version 330 core
out vec4 FragColor;

uniform vec2 u_resolution;
uniform vec2 u_mousePos;
  
in vec2 center;
in vec2 oRadius;
in vec2 resolution;

in vec3 oColor;


void main()
{
	vec2 st = gl_FragCoord.xy / resolution;
    vec2 diff = st - center;
    diff.x *= resolution.x / resolution.y;  // correct aspect on the difference, not the radius
    float r = oRadius.x / resolution.y;     // radius in the same corrected space
    if (length(diff) > r) discard;
    FragColor = vec4(oColor, 1.0);
}