#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

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

void main() {
    vec4 positionWorld = vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;
}