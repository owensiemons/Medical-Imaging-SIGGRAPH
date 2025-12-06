#shader VERTEX
#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out vec2 uv_v;

void main() {
    gl_Position = vec4(position, 1.0);
    uv_v = uv;
}

#shader FRAGMENT
#version 410 core

uniform sampler2D tex0; // We use a default naming scheme of all our textures

in vec2 uv_v;

out vec4 frag_color;

void main() {
    frag_color = texture(tex0, uv_v);
}