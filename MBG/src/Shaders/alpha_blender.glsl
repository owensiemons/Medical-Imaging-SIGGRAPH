#shader VERTEX
#version 430 core
layout(location = 0) in vec3 position;

layout(std140, binding = 0) uniform main_uniforms {
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

    uint rgb_transfer_arr_size;
    uint a_transfer_arr_size;

    vec2 x_bounds;
    vec2 y_bounds;
    vec2 z_bounds;

    vec3 bg_color;
    float pad2_;

    float step_size;
    float light_step_size;
    float light_t;
    float pad3_;
};

void main() {
    gl_Position = proj * view * vec4(position, 1.0);
}


#shader FRAGMENT
#version 430 core

layout(std140, binding = 0) uniform main_uniforms {
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

    uint rgb_transfer_arr_size;
    uint a_transfer_arr_size;

    vec2 x_bounds;
    vec2 y_bounds;
    vec2 z_bounds;
    
    vec3 bg_color;
    float pad2_;
    
    float step_size;
    float light_step_size;
    float light_t;
    float pad3_;
};

struct rgb_transfer_elem {
	vec3 col;
	float dens; // 16 bytes
};

struct a_transfer_elem {
	float opacity;
	float dens;
};

layout(std430, binding = 0) buffer rgb_transfer_ssbo {
    rgb_transfer_elem rgb_transfer_data[];
};

layout(std430, binding = 1) buffer a_transfer_ssbo {
    a_transfer_elem a_transfer_data[];
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

vec3 to_tex_space(vec3 pos) {
    return (pos - aabb_min) / (aabb_max - aabb_min);
}

vec3 rgb_transfer_func(float x) {
    vec3 lerp_col = vec3(0.0, 0.0, 0.0);

    float min_d = rgb_transfer_data[0].dens;
    vec3 min_col = rgb_transfer_data[0].col;
    float max_d = rgb_transfer_data[rgb_transfer_arr_size - 1].dens;
    vec3 max_col = rgb_transfer_data[rgb_transfer_arr_size - 1].col;

    if (x == 0.0) {// Not totally sure why this is needed, but otherwise it breaks if min_d == 0
        return min_col;
    }

    if (x < min_d) {
        lerp_col = min_col;
    } else if (x > max_d) {
        lerp_col = max_col;
    } else {
        int i = 0;
        while (x > rgb_transfer_data[i].dens) {
            i++;
        }
        vec3 col_1 = rgb_transfer_data[i - 1].col;
        float d_1 = rgb_transfer_data[i - 1].dens;

        vec3 col_2 = rgb_transfer_data[i].col;
        float d_2 = rgb_transfer_data[i].dens;

        lerp_col = mix(col_1, col_2, (x - d_1) / (d_2 - d_1)); // We're interpolating in linear rgb, for better results we could first translate to a space like OKLAB
    }

    return lerp_col;
}

float a_transfer_func(float x) {
    if (x == 0) {
        return 0.0;
    }

    float lerp_opacity = 0;

    float min_d = a_transfer_data[0].dens;
    float min_a = a_transfer_data[0].opacity;

    float max_d = a_transfer_data[a_transfer_arr_size - 1].dens;
    float max_a = a_transfer_data[a_transfer_arr_size - 1].opacity;

    if (x == 0.0) {
        return min_a;
    }

    if (x < min_d) {
        lerp_opacity = min_a;
    } else if (x > max_d) {
        lerp_opacity = max_a;
    } else {
        int i = 0;
        while (x > a_transfer_data[i].dens) {
            i++;
        }

        float a_1 = a_transfer_data[i - 1].opacity;
        float d_1 = a_transfer_data[i - 1].dens;

        float a_2 = a_transfer_data[i].opacity;
        float d_2 = a_transfer_data[i].dens;

        lerp_opacity = mix(a_1, a_2, (x - d_1) / (d_2 - d_1));
    }

    return lerp_opacity;
}

vec4 transfer_func(float x) { // assumes x in texture space
    return vec4(rgb_transfer_func(x), a_transfer_func(x));
}

vec4 lookup(vec3 pos) {
    return transfer_func(texture(tex0, to_tex_space(pos)).x);
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
        frag_color = vec4(bg_color, 0.0);
        return;
    }

    if (tnear < 0.0) { tnear = 0.0; }

    uint rng_seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * 1000u + frame_cnt * 1000000u;

    vec3 ray_start = main_ray.ro + main_ray.rd * tnear;
    vec3 ray_stop = main_ray.ro + main_ray.rd * tfar;

    vec3 ray = ray_stop - ray_start;
    float ray_len = length(ray);
    vec3 step_vec = step_size * ray / ray_len;

    float jitter = rand_pcg(rng_seed++) * step_size;
    vec3 pos = ray_start + step_vec * (jitter / step_size);
    ray_len -= jitter;


    vec4 dst = vec4(bg_color, 0);
    vec4 src = vec4(0, 0, 0, 0);

    float zcap = 0.4;
    while (ray_len > 0) {

        if (pos.z > z_bounds.x || pos.z < z_bounds.y || pos.y > y_bounds.x || pos.y < y_bounds.y || pos.x > x_bounds.x || pos.x < x_bounds.y) {
            ray_len -= step_size;
            pos += step_vec;
            continue;
        }

        src = lookup(pos);
        src.a *= 50 * step_size;
        // we multiply by scaled step size to ensure color isnt dependent on it

        src.xyz *= src.a;

        dst = (1.0 - dst.a) * src + dst;

        if (dst.a >= 0.95f) { break; }

        ray_len -= step_size;
        pos += step_vec;
    }

    frag_color = dst;
}