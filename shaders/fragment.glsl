#version 330 core

in vec3 fragNormal;
in vec3 fragPos;

uniform vec3 objcolor;

out vec4 fragColor;

void main() {
  vec3 lightDir = normalize(vec3(-0.4, 1.0, 0.6));
  vec3 normal = normalize(fragNormal);

  float ambient = 0.35;
  float diffuse = max(dot(normal, lightDir), 0.0) * 0.65;

  vec3 result = objcolor * (ambient + diffuse);
  fragColor = vec4(result, 1.0);
}
