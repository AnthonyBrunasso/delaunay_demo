#version 430

in vec3 tex_coord;
layout (location = 0) out vec4 frag_color;

layout (binding = 0) uniform blob_settings {
  vec4 inner_color;
  vec4 outer_color;
  float radius_inner;
  float radius_outer;
} blob;

void main() {
  float dx = tex_coord.x - 0.5;
  float dy = tex_coord.y - 0.5;
  float dist = sqrt(dx * dx + dy * dy);
  if (dist < blob.radius_inner) discard;
  frag_color = mix(blob.inner_color, blob.outer_color, smoothstep(blob.radius_inner, blob.radius_outer, dist));
}

