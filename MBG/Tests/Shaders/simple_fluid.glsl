#shader VERTEX
#version 410 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 last_position;
layout(location = 2) in vec2 velocity;
layout(location = 3) in float C;
layout(location = 4) in float lamdab;

out vec2 velocity_v;

void main() {
    gl_PointSize = 20.0;
    gl_Position = vec4(position, 0.0, 1.0);
    velocity_v = velocity;
}

#shader FRAGMENT
#version 410 core

out vec4 frag_color;
in vec2 velocity_v;

void main() {
    frag_color = vec4(velocity_v * 1000.0, 1.0, 1.0);
}