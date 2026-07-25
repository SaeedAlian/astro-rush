#version 330 core

in vec3 fragNormal;
in vec3 fragPos;
in vec2 fragUV;

uniform vec3 objColor;
uniform sampler2D diffuseTex;
uniform int hasTexture;

out vec4 fragColor;

void main() {
  vec3 lightDir = normalize(vec3(-0.4, 1.0, 0.6));
  vec3 normal = normalize(fragNormal);

  float ambient = 0.35;
  float diffuse = max(dot(normal, lightDir), 0.0) * 0.65;

  vec3 base = (hasTexture == 1) ? texture(diffuseTex, fragUV).rgb : objColor;
  vec3 result = base * (ambient + diffuse);
  fragColor = vec4(result, 1.0);
}
