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

const float PI = 3.14159265359;

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

bool intersectBox(vec3 ro, vec3 rd, AABB aabb, out float t0, out float t1) {
    vec3 invR = 1.0 / rd;
    vec3 tbot = invR * (aabb.min-ro);
    vec3 ttop = invR * (aabb.max-ro);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    t1 = min(t.x, t.y);
    return t0 <= t1;
}

// Henyey-Greenstein phase function
// g: asymmetry parameter (-1 to 1)
// g = 0: isotropic scattering
// g > 0: forward scattering
// g < 0: backward scattering
float phaseHG(vec3 rd, vec3 light_dir, float g) {
    float cos_theta = dot(rd, light_dir);
    float denom = 1.0 + g * g - 2.0 * g * cos_theta;
    return (1.0 / (4.0 * PI)) * (1.0 - g * g) / (denom * sqrt(denom));
}

// handles changing space
float lookup(vec3 world_pos) {
    vec3 tex = 0.5 * (world_pos + 1.0);
    return texture(tex0, tex).x;
}

vec3 traceScene(vec3 ro, vec3 rd, AABB aabb, uint rng_seed) {
    // absorption coeff
    float sigma_a = 0.5;

    // scattering coeff
    float sigma_s = 0.5;

    // extinction coeff
    float sigma_t = sigma_a + sigma_s;

    // phase function asymmetry (-1, 1)
    float g = 0.0;

    vec3 background_color = vec3(0.09);

    vec3 light_color = vec3(50);
    vec3 light_pos = vec3(0.0, 2.0, -2.0);

    int n_steps = 48;
    int n_light_steps = 12;

    float tnear, tfar;

    if (!intersectBox(ro, rd, aabb, tnear, tfar)) {
        return background_color;
    }

    if (tnear < 0.0) { tnear = 0.0; } //NOTE: all the marching is done in world space, lookup() changes to tex space
    vec3 ray_start = ro + rd * tnear;
    vec3 ray_stop  = ro + rd * tfar;

    vec3 ray = ray_stop - ray_start;
    float ray_len = length(ray);
    vec3 step_vec = ray / float(n_steps);
    float step_size = ray_len / float(n_steps);

    float jitter = rand_pcg(rng_seed++) * step_size;
    vec3 sample_pos = ray_start + step_vec * (jitter / step_size);
    ray_len -= jitter;

    float transparency = 1.0;
    vec3 result = vec3(0.0);

    while (ray_len > 0) {
        float density = lookup(sample_pos);

        // attenuate by beer-lambert
        float sample_attenuation = exp(-step_size * density * sigma_t);
        // attenuate volume object transparency by current sample transmission value
        transparency *= sample_attenuation;

        // In-Scattering. Find the distance traveled by light through 
        // the volume to our sample point. Then apply Beer's law.
        // NOTE: doesnt not currently work as intented with varied density
        vec3 light_dir = normalize(light_pos - sample_pos);
        float ltnear, ltfar;
        
        if (density > 0 && intersectBox(sample_pos, light_dir, aabb, ltnear, ltfar)) {
            if (tnear < 0.0) { tnear = 0.0; }
            vec3 light_ray = light_dir * ltfar;
            float light_ray_len = length(light_ray);
            vec3 light_sample_pos = sample_pos;
            vec3 light_step_vec = light_ray / float(n_light_steps);
            float light_step_size = light_ray_len / float(n_light_steps);
             
            float tau = 0;

            while (light_ray_len > 0) {
                float inscatter_density = lookup(light_sample_pos);
                tau += inscatter_density;
                light_sample_pos += light_step_vec;
                light_ray_len -= light_step_size;
            }
            
            float light_attenuation = exp(-tau * light_step_size * sigma_t);
            
            float phase = phaseHG(rd, light_dir, g);
            
            // attenuate in-scattering contrib. by the transmission of all samples accumulated so far
            result += transparency * light_color * light_attenuation * density * sigma_s * step_size * phase;
            rng_seed++;
        }

        // russian roulette
        if (transparency < 0.01) {
            float rr_prob = 0.5;
            if (rand_pcg(rng_seed) > rr_prob) {
                break;
            } else {
                transparency /= rr_prob; // compensate for surviving paths
            }
        }
        ray_len -= step_size;
        sample_pos += step_vec;
    }

    return background_color * transparency + result;
}

void main() {
    vec2 resolution = vec2(screen_width, screen_height);
    vec2 uv = (gl_FragCoord.xy / resolution) * 2.0 - 1.0;
    vec3 ro = vec3(inv_view[3]);
    vec4 ray_clip = vec4(uv, -1.0, 1.0);
    vec4 ray_view = inv_proj * ray_clip;
    ray_view = vec4(ray_view.xy, -1.0, 0.0);
    vec3 rd = normalize(vec3(inv_view * ray_view));
    AABB aabb = AABB(vec3(-1.0), vec3(+1.0));

    uint seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * 1000u + frame_cnt * 1000000u;
    vec3 color = traceScene(ro, rd, aabb, seed);
    frag_color = vec4(color, 1.0);
}