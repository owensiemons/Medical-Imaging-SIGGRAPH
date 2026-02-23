#shader VERTEX
#version 420 core

layout(location = 0) in vec3 position;

void main() {
    gl_Position = vec4(position, 1.0);
}



#shader FRAGMENT
#version 420 core

layout(std140, binding = 0) uniform uniforms {
    float screen_width;
    float screen_height;
};

out vec4 frag_color;

float sdfSphere(vec3 p, vec3 center, float rad) {
    return length(center - p) - rad;
}

float map(vec3 p) {
    return min(sdfSphere(p, vec3(-0.5, 0, 0), 0.4), sdfSphere(p, vec3(0.5, 0, 0), 0.4));
}

float raymarch(vec3 ro, vec3 rd) {
    float t = 0.0;

    for (int i = 0; i < 128; i++) {
        vec3 p = ro + (rd * t);
        float d = map(p);
        if (d < 0.0005) {
            return d;
        }
        t += d;
        if (t > 100.0) {
            break;
        }
    }
    return -1.0;
}

void main() {
    vec2 resolution = vec2(screen_width, screen_height);
    vec2 uv = (gl_FragCoord.xy / resolution) * 2.0 - 1.0;
    vec3 ro = vec3(0, 0, -2.75);
    float aspect = screen_width / screen_height;
    vec3 rd = normalize(vec3(uv.x * aspect, uv.y, 1.0));


    vec3 color = vec3(1.0, 0.0, 1.0);
    float t = raymarch(ro, rd);
    if (t > 0.0) {
        color = vec3(1.0, 1.0, 1.0);
    } else {
        color = vec3(0.2, 0.2, 0.2);
    }
    frag_color = vec4(color, 1.0);
}