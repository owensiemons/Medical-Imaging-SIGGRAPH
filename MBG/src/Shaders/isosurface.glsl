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
    float pad0, pad1;

    mat4 proj;
    mat4 view;
    mat4 model;

    mat4 inv_proj;
    mat4 inv_view;

    uint frame_cnt;
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
    float d = 0.01;
    float dx = texture(tex0, p + vec3(d,0,0)).x - intensity;
    float dy = texture(tex0, p + vec3(0,d,0)).x - intensity;
    float dz = texture(tex0, p + vec3(0,0,d)).x - intensity;
    mat3 normal_matrix = mat3(model);
    return -normalize(normal_matrix * vec3(dx, dy, dz));
}

void main() {
    vec2 resolution = vec2(screen_width, screen_height);
    vec2 uv = (gl_FragCoord.xy / resolution) * 2.0 - 1.0;
    vec3 ro = vec3(inv_view[3]);

    vec4 ray_clip = vec4(uv, -1.0, 1.0);
    vec4 ray_view = inv_proj * ray_clip;
    ray_view = vec4(ray_view.xy, -1.0, 0.0);

    vec3 rd = normalize(vec3(inv_view * ray_view));

    Ray main_ray = Ray(ro, rd);

    AABB aabb = AABB(vec3(-1.0), vec3(+1.0));

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

    ray_start = 0.5 * (ray_start + 1.0);
    ray_stop = 0.5 * (ray_stop + 1.0);

    vec3 ray = ray_stop - ray_start;
    float ray_len = length(ray);
    vec3 step_vec = step_size * ray / ray_len;

    vec3 light_pos = vec3(0.0, 0.35, -0.35);
    
    float jitter = rand_pcg(rng_seed++) * step_size;
    vec3 pos = ray_start + step_vec * (jitter / step_size);
    ray_len -= jitter;

    float threshold = 0.1;
    vec3 color = vec3(0.09);

    float ka = 0.15;
    float kd = 0.5;
    float ks = 1.0;
    float spec_power = 8.0;
    vec3 mat_color = vec3(1.0);
    vec3 spec_color = vec3(1.0);

    while (ray_len > 0) {
        float intensity = texture(tex0, pos).x;

        if (intensity > threshold) {
            
            pos -= step_vec * 0.5;
            intensity = texture(tex0, pos).x;
            
            if (intensity > threshold) {
                pos -= step_vec * 0.25;
            } else {
                pos += step_vec * 0.25;
            }

            intensity = texture(tex0, pos).x;
            vec3 world_pos = 2.0 * pos - 1.0;
            vec3 light_norm = normalize(light_pos - world_pos);
            vec3 ray_norm = -rd;
            vec3 vol_norm = estimate_normal(pos, intensity);
            vec3 half_norm = normalize(light_norm + ray_norm);

            float ambient = ka;
            float diffuse = kd * max(0, dot(vol_norm, light_norm));
            float specular = ks * pow(max(0, dot(vol_norm, half_norm)), spec_power);

            color = (ambient + diffuse) * mat_color + specular * spec_color;

            break;
        }

        ray_len -= step_size;
        pos += step_vec;
    }

    frag_color = vec4(color, 1.0);
}