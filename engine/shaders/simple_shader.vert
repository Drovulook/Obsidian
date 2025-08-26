#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform UBO {
  mat4 projectionViewMatrix;
  vec3 directionToLight;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix; 
  mat4 normalMatrix;
} push;

const float AMBIENT_LIGHT_INTENSITY = 0.02;

void main() {
  gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);

  // !! only works if scale is uniform
  // vec3 normalWorldSpace = normalize(mat3(push.modelMatrix) * normal);

  // expensive
  // mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
  // vec3 normalWorldSpace = normalize(normalMatrix * normal);

  vec3 normalWorldSpace =normalize(mat3(push.normalMatrix) * normal);

  float lightIntensity = AMBIENT_LIGHT_INTENSITY + max(dot(normalWorldSpace, normalize(ubo.directionToLight)), 0.0);

  fragColor = color * lightIntensity;
}