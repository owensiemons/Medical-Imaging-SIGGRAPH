#shader VERTEX
#version 420 core
layout(location = 0) in vec3 position;

layout(std140, binding = 0) uniform uniforms {
    vec2 screen_size;
    uint frame_cnt;
    float pad_;

    mat4 proj;
    mat4 view;
    mat4 model;

    mat4 inv_proj;
    mat4 inv_view;

    vec3 aabb_max;
    float pad0_;
    vec3 aabb_min;
    float pad1_;
};

void main() {
    gl_Position = proj * view * vec4(position, 1.0);
}



#shader FRAGMENT
#version 420 core

layout(std140, binding = 0) uniform uniforms {
    vec2 screen_size;
    uint frame_cnt;
    float pad_;

    mat4 proj;
    mat4 view;
    mat4 model;

    mat4 inv_proj;
    mat4 inv_view;

    vec3 aabb_max;
    float pad0_;
    vec3 aabb_min;
    float pad1_;
};

uniform sampler3D tex0;

out vec4 frag_color;

struct Ray {
    vec3 ro;
    vec3 rd;
};

struct AABB {
    vec3 min;
    vec3 max;
};

uint pcg(uint v) {
    uint state = v * 747796405u + 2891336453u;
    uint word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float rand_pcg(uint seed) {
    return float(pcg(seed)) / float(0xFFFFFFFFu);
}

bool intersectBox(Ray r, AABB aabb, out float t0, out float t1) {
    vec3 invR = 1.0 / r.rd;
    vec3 tbot = invR * (aabb.min-r.ro);
    vec3 ttop = invR * (aabb.max-r.ro);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    t1 = min(t.x, t.y);
    return t0 <= t1;
}

float lookup(vec3 pos) {
    vec3 tex_space = (pos - aabb_min) / (aabb_max - aabb_min);
    return texture(tex0, tex_space).x;
}

void main() {
    vec2 resolution = screen_size;
    vec2 uv = (gl_FragCoord.xy / resolution) * 2.0 - 1.0;
    vec3 ro = vec3(inv_view[3]);

    vec4 ray_clip = vec4(uv, -1.0, 1.0);
    vec4 ray_view = inv_proj * ray_clip;
    ray_view = vec4(ray_view.xy, -1.0, 0.0);

    vec3 rd = normalize(vec3(inv_view * ray_view));

    Ray main_ray = Ray(ro, rd);

    AABB aabb = AABB(aabb_min, aabb_max);

    float tnear, tfar;

    if (!intersectBox(main_ray, aabb, tnear, tfar)) {
        frag_color = vec4(vec3(0.09), 0.0);
        return;
    }
    if (tnear < 0.0) { tnear = 0.0; }

    uint rng_seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * 1000u + frame_cnt * 1000000u;

    float step_size = 0.01;
    vec3 ray_start = main_ray.ro + main_ray.rd * tnear;
    vec3 ray_stop = main_ray.ro + main_ray.rd * tfar;

    vec3 ray = ray_stop - ray_start;
    float ray_len = length(ray);
    vec3 step_vec = step_size * ray / ray_len;

    float jitter = rand_pcg(rng_seed++) * step_size;
    vec3 pos = ray_start + step_vec * (jitter / step_size);
    ray_len -= jitter;

    vec4 dst = vec4(0.09, 0.09, 0.09, 0);
    vec4 src = vec4(0, 0, 0, 0);

    float value = 0;

    float max_intensity = 0.09;

    while (ray_len > 0) {
        value = lookup(pos);

        if (value > max_intensity) {
            max_intensity = value;
        }

        ray_len -= step_size;
        pos += step_vec;
    }

    frag_color = vec4(max_intensity, max_intensity, max_intensity, 1.0);
}