#version 330 core

out vec4 FragColor;

uniform vec3 objcolor;

void main() {
  FragColor = vec4(
      objcolor,
      1.0
  );
}
