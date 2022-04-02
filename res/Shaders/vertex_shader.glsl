#version 460

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 vert_color;
out vec4 color;

void main() {
     color       = vec4(vert_color, 1.0);
     gl_Position = vec4(in_position, 1.0);
}
