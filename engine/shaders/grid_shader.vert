#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

const vec2 VERTICES_COOR[4] = vec2[](
  vec2(0.0, 0.0),
  vec2(1.0, 0.0),
  vec2(1.0, 1.0),
  vec2(0.0, 1.0)
);

struct PointLight {
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform UBO {
  mat4 projection;
  mat4 view;
  mat4 inverseView;
  vec4 ambientLightColor;
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(location = 0) out vec2 frag_position;

void main() {
    frag_position = VERTICES_COOR[gl_VertexIndex];

    vec4 positionWorld = vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;
}