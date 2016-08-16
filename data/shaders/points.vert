#version 400

layout(location = 0) in vec3 vpos;

uniform mat4 view, proj, model;

void main() {
  gl_PointSize = 5.0;

  gl_Position = proj * view * model * vec4(vpos, 1.0);
}
