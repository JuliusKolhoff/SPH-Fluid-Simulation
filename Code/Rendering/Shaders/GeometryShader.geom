#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out vec2 center;
out vec2 oRadius;

uniform float u_radius;
uniform vec2 u_resolution;
uniform bool u_render_complex;
uniform float u_radius_increase;

uniform float u_pixels_per_meter;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 u_camera_up;
uniform vec3 u_camera_right;
   

in vec3 Color[];
out vec3 oColor;
out vec2 gTexCoord;

//out vec2 resolution;

void main(){

	oColor = Color[0];

    float perspective_correction = 5.0f;

	float r = perspective_correction * u_radius * u_pixels_per_meter * 2.0 / u_resolution.y;
    if (u_render_complex) r *= u_radius_increase;

    vec3 right = normalize(u_camera_right) * r;
    vec3 up    = normalize(u_camera_up)    * r;

    vec3 c = gl_in[0].gl_Position.xyz;
    mat4 mvp = projection * view * model;

    gl_Position = mvp * vec4(c - right - up, 1.0); gTexCoord = vec2(0,0); EmitVertex();
    gl_Position = mvp * vec4(c - right + up, 1.0); gTexCoord = vec2(0,1); EmitVertex();
    gl_Position = mvp * vec4(c + right - up, 1.0); gTexCoord = vec2(1,0); EmitVertex();
    gl_Position = mvp * vec4(c + right + up, 1.0); gTexCoord = vec2(1,1); EmitVertex();
}
