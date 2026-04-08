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

    uint transfer_arr_size;
};

void main() {
    gl_Position = proj * view * vec4(position, 1.0);
}



#shader FRAGMENT
#version 430 core

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

    uint transfer_arr_size;
};

struct transfer_elem {
	vec3 col;
	float dens; // 16 bytes
};

layout(std430, binding = 3) buffer transfer_ssbo {
    transfer_elem transfer_data[2];
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

vec3 estimate_normal(vec3 p, float intensity) {
    vec3 tex_space = (p - aabb_min) / (aabb_max - aabb_min);
    float d = 0.01;
    float dx = texture(tex0, tex_space + vec3(d,0,0)).x - intensity;
    float dy = texture(tex0, tex_space + vec3(0,d,0)).x - intensity;
    float dz = texture(tex0, tex_space + vec3(0,0,d)).x - intensity;
    mat3 normal_matrix = mat3(model);
    return -normalize(normal_matrix * vec3(dx, dy, dz));
}

vec3 to_tex_space(vec3 pos) {
    return (pos - aabb_min) / (aabb_max - aabb_min);
}

float lookup(vec3 pos) {
    vec3 tex_space = to_tex_space(pos);
    return texture(tex0, tex_space).x;
}

vec3 transfer_func(float x) { // assumes x in texture space
    vec3 lerp_col;

    float min_d = transfer_data[0].dens;
    vec3 min_col = transfer_data[0].col;
    float max_d = transfer_data[transfer_arr_size - 1].dens;
    vec3 max_col = transfer_data[transfer_arr_size - 1].col;

    if (x < min_d) {
        lerp_col = min_col;
    } else if (x > max_d) {
        lerp_col = max_col;
    } else {
        int i = 0;
        while (x > transfer_data[i].dens) {
            i++;
        }
        vec3 col_1 = transfer_data[i - 1].col;
        float d_1 = transfer_data[i - 1].dens;

        vec3 col_2 = transfer_data[i].col;
        float d_2 = transfer_data[i].dens;

        lerp_col = mix(col_1, col_2, (x - d_1) / (d_2 - d_1)); // We're interpolating in linear rgb, for better results we could first translate to a space like OKLAB
    }

    return lerp_col;
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

    vec3 light_pos = vec3(0.0, 1.5, -1.5);
    
    float jitter = rand_pcg(rng_seed++) * step_size;
    vec3 pos = ray_start + step_vec * (jitter / step_size);
    ray_len -= jitter;

    float threshold = 0.3;
    vec3 color = vec3(0.09);

    float ka = 0.15;
    float kd = 0.5;
    float ks = 1.0;
    float spec_power = 8.0;
    vec3 mat_color = vec3(1.0); // Maybe use a transfer function for this
    vec3 spec_color = vec3(1.0);

    while (ray_len > 0) {
        float intensity = lookup(pos);

        if (intensity > threshold) {
            
            pos -= step_vec * 0.5;
            intensity = lookup(pos);
            
            if (intensity > threshold) {
                pos -= step_vec * 0.25;
            } else {
                pos += step_vec * 0.25;
            }

            intensity = lookup(pos);
            vec3 light_norm = normalize(light_pos - pos);
            vec3 ray_norm = -rd;
            vec3 vol_norm = estimate_normal(pos, intensity);
            vec3 half_norm = normalize(light_norm + ray_norm);

            float ambient = ka;
            float diffuse = kd * max(0, dot(vol_norm, light_norm));// add light color
            float specular = ks * pow(max(0, dot(vol_norm, half_norm)), spec_power);

            mat_color = transfer_func(lookup(pos));

            color = (ambient + diffuse) * mat_color + specular * spec_color;

            break;
        }

        ray_len -= step_size;
        pos += step_vec;
    }

    frag_color = vec4(color, 1.0);
}