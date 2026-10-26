#version 330 core
out vec4 FragColor;

uniform bool u_render_complex;
uniform float u_radius_increase;
uniform vec3 u_camera_position;

in vec3 oColor;
in vec2 gTexCoord;
in vec3 vertex_position;

float smoothing_kernel(float  dist) {
    // NO Volume correction for visuals
    float value = max(0.0f, 1.0f - dist * dist);
    return (value * value * value);
}


void main()
{
    

    vec2 uv = gTexCoord;
	if(distance(uv,vec2(0.5f,0.5f))>=0.5f) discard;
    float alpha = 1.0f;
    if(u_render_complex){
        alpha = smoothing_kernel(distance(uv,vec2(0.5f,0.5f))*2);
        //alpha /= distance(vertex_position,u_camera_position);
        alpha /=4;
    }

    vec2 pos = (uv - 0.5) * 2.0;
    float r2 = dot(pos, pos);
    if (r2 > 1.0) discard;

    vec3 normal = vec3(pos, sqrt(1.0 - r2));

    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normal, lightDir), 0.0);
    float ambient = 0.15;

    FragColor = vec4(oColor * (ambient + 0.85 * diffuse), alpha);
}