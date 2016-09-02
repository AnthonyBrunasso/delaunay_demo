#version 430 
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_texcoord;

uniform mat4 view, proj, model;

out vec3 tex_coord;

void main() {
  tex_coord = vertex_texcoord;
  gl_Position = proj * view * model * vec4(vertex_position, 1.0);
}

