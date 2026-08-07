#version 330 core
out vec4 FragColor;

uniform vec2 u_resolution;
uniform vec2 u_mousePos;
uniform bool u_render_complex;
uniform float u_radius_increase;
  
in vec2 center;
in vec2 oRadius;
in vec2 resolution;

in vec3 oColor;

float smoothing_kernel(float  dist) {
    float volume = 3.141f/ 4.0f;
    volume = 1; // NO Volume correction for visuals
    float value = max(0.0f, 1.0f - dist * dist);
    return (value * value * value / volume);
}


void main()
{
	vec2 st = gl_FragCoord.xy / resolution;
    vec2 diff = st - center;
    diff.x *= resolution.x / resolution.y;  // correct aspect on the difference, not the radius
    float r = oRadius.x / resolution.y;     // radius in the same corrected space
    if(u_render_complex){
        if(length(diff)/(u_radius_increase*r) > 1.0f) discard;
        //float alpha = 1.0 - length(diff)/(u_radius_increase*r);
        float alpha = smoothing_kernel(length(diff)/(u_radius_increase*r));
        FragColor = vec4(oColor,alpha);
    }else{
        if (length(diff) > r) discard;
        FragColor = vec4(oColor,1.0f);
    }
}