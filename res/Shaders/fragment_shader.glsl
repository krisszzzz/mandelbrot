#version 460

in vec4 color;
out vec4 frag_color;
uniform float u_time;
uniform vec2  resolution; 

vec2 rotate2D(vec2 uv, float a) {
    float s = sin(a);
    float c = cos(a);
    return mat2(c, -s, s, c) * uv;
}

void main() {

    vec2 uv = (gl_FragCoord.xy - 0.5 * resolution.xy) / resolution.y;
    vec3 col = vec3(0.0);

    uv = rotate2D(uv, 3.14 / 2);

    float r = 0.2;
    for (float i = 0.0; i < 60.0; i++) {

        float factor = (sin(u_time) * 0.5 + 0.5) + 0.3;
        i += factor;

        float a = i / 3;
        float dx = (20 * r * cos(a)) / 12 + r * cos(20*a / 12);
        float dy = (20 * r * sin(a)) / 12 - r * sin(20*a / 12);

        col += 0.01 * factor / length(uv - vec2(dx * 0.9, dy * 0.9));
    }
    col *= sin(vec3(0.2, 0.8, 0.9) * u_time) * 0.15 + 0.25;

    frag_color = vec4(col, 1.0);

}
