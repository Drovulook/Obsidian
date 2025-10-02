#version 450

layout (location = 0) out vec4 outColor;

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

const float minCellSize = 0.02;
const float minCellPixelWidth = 0.5;
const float lineWidth = 4.0;
const vec3 thinColor = vec3(0.2, 0.2, 0.2);
const vec3 thickColor = vec3(1.0, 1.0, 1.0);

layout(location = 0) in vec2 frag_position;

float max2(vec2 v) {
  return max(v.x, v.y);
}

float log10(float x) {
  return log(x) / log(10.0);
}

float ease_inout_exp(float x) {
    return x <= 0.0 ? 0.0 : pow(2.0, 20.0 * (x - 1.0));
}

vec4 grid(vec2 uv) {
  vec2 dudv = vec2(
    length(vec2(dFdx(uv.x), dFdy(uv.x))),
    length(vec2(dFdx(uv.y), dFdy(uv.y)))
  );

  float lod = max(0.0, log10((max2(dudv) * minCellPixelWidth) / minCellSize) + 1.0);
  float fade = fract(lod);

  float lod0 = minCellSize * pow(10.0, floor(lod));
  float lod1 = lod0 * 10.0;
  float lod2 = lod1 * 10.0;

  float lod0a = max2(vec2(1.0) - abs(clamp(mod(uv, lod0) / dudv / lineWidth, 0.0, 1.0) * 2.0 - vec2(1.0)));
  float lod1a = max2(vec2(1.0) - abs(clamp(mod(uv, lod1) / dudv / lineWidth, 0.0, 1.0) * 2.0 - vec2(1.0)));
  float lod2a = max2(vec2(1.0) - abs(clamp(mod(uv, lod2) / dudv / lineWidth, 0.0, 1.0) * 2.0 - vec2(1.0)));

  return vec4(
    lod2a > 0.0 ? thickColor : lod1a > 0.0 ? mix(thickColor, thinColor, fade) : thinColor,
    0.8 + (lod2a > 0.0 ? lod2a : lod1a > 0.0 ? lod1a : lod1a * (1.0 - fade)) * 0.2
  );
}

void main() {
  vec3 cameraWorldPos = (ubo.inverseView * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
//   float scale = 1 + 40*pow(dot(cameraWorldPos, cameraWorldPos), 0.5);
float scale = 1 + 10*pow(abs(cameraWorldPos.y), 0.5);

  vec4 color = grid((frag_position - vec2(0.5, 0.5)) * scale);
  vec2 centeredPos = 0.5 - frag_position;

  color *= max(2.5 * ease_inout_exp(1.0 - length(centeredPos)), 0.0);
  outColor = color;
}