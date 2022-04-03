#version 460
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec3 color;

out vec4 vertex_color;

void main() {
     vertex_color = vec4(color, 1.0);
     gl_Position = vec4(in_position, 0.0, 1.0);
}
