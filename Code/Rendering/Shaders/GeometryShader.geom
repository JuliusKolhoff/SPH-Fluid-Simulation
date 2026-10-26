#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;


uniform float u_radius;
uniform bool u_render_complex;
uniform float u_radius_increase;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 u_camera_up;
uniform vec3 u_camera_right;
   

in vec3 Color[];

out vec3 oColor;
out vec2 gTexCoord;
out vec3 vertex_position;

//out vec2 resolution;

void main(){

	oColor = Color[0];

	float r = u_radius;
    if (u_render_complex) r *= u_radius_increase;

    vec3 right = normalize(u_camera_right) * r;
    vec3 up    = normalize(u_camera_up)    * r;

    vec3 c = gl_in[0].gl_Position.xyz;
    mat4 mvp = projection * view * model;

    gl_Position = mvp * vec4(c - right - up, 1.0); gTexCoord = vec2(0,0); vertex_position = c; EmitVertex();
    gl_Position = mvp * vec4(c - right + up, 1.0); gTexCoord = vec2(0,1); vertex_position = c; EmitVertex();
    gl_Position = mvp * vec4(c + right - up, 1.0); gTexCoord = vec2(1,0); vertex_position = c; EmitVertex();
    gl_Position = mvp * vec4(c + right + up, 1.0); gTexCoord = vec2(1,1); vertex_position = c; EmitVertex();
}
